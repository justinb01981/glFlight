/* httpfileserver.cpp - a simple HTTP file server with web based file management
 * author: Justin Brady */

/* TO DO:
 * - allow setting passwords for certain paths, manage multiple logins via seperate cookies
 */
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>

#include "simplesocket.h"
#include "httpfsutils.h"
#include "auth.h"
#include "cpthread.h"
#include "res/images.h"

#include "boundary_helper.h"

#ifdef _UNIX_
#include <signal.h>
#endif

using namespace std;

#define VERSIONSTR "0.6.1"

#define PORT 80
#define BUFFER_SIZE 5000

#define MAX_BOUNDARY_SIZE 255
#define MAX_FILENAME_SIZE 1024
#define MAX_PATH_SIZE 4096
#define MAX_HEADERS_SIZE 10000

#define MAX_CONNECTIONS 2048

#define COMMAND_GET 1
#define COMMAND_POST 2
#define COMMAND_HEAD 3
#define COMMAND_OPTIONS 4

#define CRLF "\r\n"
#define CRLFCRLF "\r\n\r\n"
#define FILE_UPLOAD_TAG "uploaded_file"
#define FILE_NAME_TAG "filename="
#define PATH_NAME_TAG "path="
#define BOUNDARY_TAG "boundary="
#define CONTENT_LENGTH_TAG "Content-Length:"

#define AUTH_SEM_TAKE MutexSemTake(&authsem)
#define AUTH_SEM_GIVE MutexSemGive(&authsem)

void HandleConnectionMT(void* params);
int HandleCommand(SimpleSocket* s, char* buf, int len);
int HandlePOST(SimpleSocket* s, char* path, char* headersbuf);
int ParseCommand(char* cmd, char* args, int args_len);
unsigned int GetFileSize(fstream* file);
bool endsWith(char* str, char* end);
void UnescapeHttp(char* str);
void SendUploadSuccessful(SimpleSocket* s);
void SendDeleteSuccessful(SimpleSocket* s);
int WriteUntilBoundaryFound(fstream* file, SimpleSocket* s,
                            char* endBoundaryStr);
void BuildFinalBoundary(char* boundary, char* finalBoundary, int fb_len);
char* BuildUploadFilename(char* str);
char * stristr(char *haystack, char *needle);
bool strEndsWith(char* haystack, char* needle);
int GetLocalFile(char* pathFileName, char* localPathOut, int localPathSize,
                 fstream** file, bool create, bool replace);
void Log(char* msg, char* clientCommand, char* localCmdFile);
fstream* fstreamWrapper(char* pathfile, bool in, bool out,
                        bool create, bool replace);
int validate_ascii(char* buf, int len);
int sendBuf(SimpleSocket* s, char* buf);
int HandleSpecialFile(char *args, SimpleSocket* s, char *req_headers);
int LogoutSession(char *headers);
int GrabPasswordFromFile(char *filename, char *dest, int dest_len);
void handle_sigpipe(int s);


static char* crlf = "\r\n";
static char* crlfcrlf = "\r\n\r\n";

static char* ggifs[] = {(char*) gopher_unknown_gif, (char*) gopher_menu_gif, 0};

/* globals */
bool done = false;
char gAuthPass[255];
char gPermissions[255];    /* r=read, u=upload, d=delete */
class Auth gAuth;

Semaphore authsem;

char gLogPart2[4096];

size_t total_bytes_per_sec = 2500000; /* per connection */
int bwm_sleep_ms = 100;
int send_buf_size = 250000; /* the size of the read-buffer for sending files */
bool adminPasswordSet = false;

int main(int argc, char* argv[])
{
    int i, port;
    SimpleSocket *listen_sock = NULL;
    CPThread* cpthreads[MAX_CONNECTIONS];

#ifdef _UNIX_
    signal(SIGPIPE, handle_sigpipe);
#endif

    pipe_to_devnull(stderr);

    cout << "Web File Share (by justin@domain17.net) - version " 
         << VERSIONSTR << endl
         << "\t(run with --help for options)" << endl;

    memset(cpthreads, 0, sizeof(cpthreads));

    /* setup semaphores */
    MutexSemInit(&authsem);

    port = PORT;
    /* default (unspecified) is to allow read/upload, but no admin access */
    strcpy(gAuthPass, "password");
    strcpy(gPermissions, "ru-a-e");

    for(i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-port") == 0 && i+1 < argc)
        {
            port = atoi(argv[i+1]);
            i++;
        }
        else if(strcmp(argv[i], "-perm") == 0 && i+1 < argc)
        {
            strcpy(gPermissions, argv[i+1]);
            cout << endl;
            i++;
        }
        else if(strcmp(argv[i], "-pass") == 0 && i+1 < argc)
        {
            strcpy(gAuthPass, argv[i+1]);
            adminPasswordSet = true;
            i++;
        }
        else if(strcmp(argv[i], "-passfile") == 0 && i+1 < argc)
        {
            if(!GrabPasswordFromFile(argv[i+1], gAuthPass, sizeof(gAuthPass)))
            {
                cout << "unable to get password from file " << argv[i+1] << endl;
                goto quit;
            }
            adminPasswordSet = true;
            i++;
        }
        else if(strcmp(argv[i], "-rate") == 0 && i+1 < argc)
        {
            total_bytes_per_sec = atoi(argv[i+1]);
            i++;
        }
        else if(strcmp(argv[i], "-securepath") == 0 && i+1 < argc)
        {
            char *s = strchr(argv[i+1], ':');
            int l = strlen(argv[i+1]);
            if(s) *s = '\0';
            if(!s || *(s-1) != '/' || argv[i+1][0] != '/')
            {
                cout << "invalid value for securepath, use /path/to/protect/:password (must begin/end with /)" << endl;
                goto quit;
            }
            s++;
            gAuth.addSecurePath(argv[i+1], s);
            i++;
        }
        else
        {
            if(!strstr(argv[i], "help")) cout << "unknown option: " << argv[i] << endl;
            cout << " usage: httpfsd [options]" << endl
                 << "\t-pass [admin_password]" << endl
                 << "\t-passfile [password_filename]" << endl
                 << "\t-port [port_num]" << endl
                 << "\t-perm [r (read) | u (upload) | a (admin)]" << endl
                 << "\t-rate [send_rate]" << endl
                 << "\t-securepath [/pw_protected_folder:password]" << endl
                 << endl;
            cout << endl << "-pass sets the admin password" << endl
                 << "-passfile sets the admin password from a file" << endl
                 << "-port sets the listening port number (default 80)" << endl
                 << "-perm sets anonymous user permissions (any combination of r, u, a)" << endl
                 << "-rate sets the maximum rate in bytes per second to send data" << endl
                 << "-securepath allows you to set a password on a path before it can be read" << endl;
            goto quit;
        }
    }

    cout << "anonymous user permissions: (" << (strrchr(gPermissions, '-')? "default": gPermissions) << ") ";
    if(strstr(gPermissions, "-r") == NULL &&
       strstr(gPermissions, "r") != NULL) cout << "read";
    if(strstr(gPermissions, "-u") == NULL &&
       strstr(gPermissions, "u") != NULL) cout << "/upload_file";
    if(strstr(gPermissions, "-a") == NULL &&
       strstr(gPermissions, "a") != NULL) cout << "/delete_file_or_folder/create_folder" << " - dangerous!";    cout << endl;
    if(strstr(gPermissions, "-e") == NULL &&
       strstr(gPermissions, "e") != NULL) cout << "/exec_cmd [args]" << " - really dangerous!";    cout << endl;

    listen_sock = new SimpleSocket(port, "0.0.0.0", MAX_CONNECTIONS);
    if(!listen_sock) goto quit;

    done = false;

    while(!done)
    {
        /* accept a connection */
        SimpleSocket* s = listen_sock->accept();
        /* TODO: if the listening socket is no longer valid, should
         * quit... right now just assuming that accept() failed because
         * of EINTR or some reason that wants a retry... */
        if(!s) continue;

        for(i = 0; i < MAX_CONNECTIONS; i++)
        {
            if(cpthreads[i] != NULL)
            {
                if(cpthreads[i]->done())
                {
                    delete cpthreads[i];
                    cpthreads[i] = NULL;
                }
            }
        }

        i = 0;
        while(i < MAX_CONNECTIONS)
        {
            if(cpthreads[i] == NULL) break;
            i++;
        }

        if(i >= MAX_CONNECTIONS)
        {
            delete s;
            continue;
        }

        /* kick off a new thread to serve this connection */
        cpthreads[i] = new CPThread((void*) HandleConnectionMT, s);
        cpthreads[i]->start();
    }

    quit:

    if(listen_sock) delete listen_sock;
    
    MutexSemDestroy(&authsem);

    return 0;
}

void HandleConnectionMT(void* params)
{
    SimpleSocket* sock = (SimpleSocket*) params;
    char buf[BUFFER_SIZE];
    int buf_max = sizeof(buf)-1;
    int r;
    time_t curtime;

    memset(buf, 0, BUFFER_SIZE);

    memset(gLogPart2, 0, sizeof(gLogPart2));

    while(sock->ready(TIMEOUT_SECS, 0) > 0)
    {
        r = ReadAndBufferUntil(sock, "\r\n", false,
                               buf, sizeof(buf)-1, NULL);
        if(r < 0) break;
        buf[r] = 0;

        if(!validate_ascii(buf, r))
            break;

        curtime = time(NULL);
        sprintf(gLogPart2, "--------------------------------------\n%s: %s--------------------------------------\n%s", sock->getRemoteHostStr(),
                ctime(&curtime), buf);
        Log(NULL, gLogPart2, NULL);

        if(HandleCommand(sock, buf, strlen(buf)) < 0)
            break;
    }

    /* wait for disconnect up to 2 seconds */
    sock->ready(2, 0);

    sock->close();
    delete sock;

    return;    /* thread will die here */
}

int HandleCommand(SimpleSocket* s, char* cmdbuf, int len)
{
    char buf[BUFFER_SIZE];
    char args[BUFFER_SIZE];
    char headersbuf[4096];
    char filepath[BUFFER_SIZE];
    char localPath[BUFFER_SIZE];
    char contentType[64];
    char lm_str[255];

    char* workptr = NULL;
    char* ptr = NULL;
    char finalBoundary[1024];

    char tmp[BUFFER_SIZE];
    fstream* file = NULL;
    
    char *send_buf = NULL;
    bool authenticated = false;

    int err;
    int cmd = ParseCommand(cmdbuf, args, BUFFER_SIZE);
    size_t path_len_reserve = 20;

    memset(filepath, 0, sizeof(filepath));

    /* pull out headers */
    memset(headersbuf, 0, sizeof(headersbuf));
    err = ReadAndBufferUntil(s, "\r\n\r\n", false, headersbuf,
                             sizeof(headersbuf)-3, NULL);
    if(err < 0) return err;
    strcat(headersbuf, "\r\n");
    cout << headersbuf << endl << endl; /* logging hack */

    switch(cmd)
    {
    /****************************************************************/
    /********************HANDLE GET COMMAND *************************/
    case COMMAND_GET:

        /* special files */
        if(HandleSpecialFile(args, s, headersbuf))
        {
            err = -1; /* disconnect */
            break;
        }

        AUTH_SEM_TAKE;
        authenticated = gAuth.checkHeaders(headersbuf, args);
        AUTH_SEM_GIVE;

        if(!strchr(gPermissions, 'r') || gAuth.isSecurePath(args))
        {
            /*
            AUTH_SEM_TAKE;
            authenticated = gAuth.checkHeaders(headersbuf, args);
            AUTH_SEM_GIVE;
            */
            
            if(!authenticated)
            {
                SendAuth(s, NULL, false);
                break;
            }
        }

        strcpy(buf, "HTTP/1.0 200 OK");
        strcat(buf, crlf);

        strncpy(filepath, args, sizeof(filepath) - path_len_reserve);
        err = GetLocalFile(filepath, localPath, sizeof(localPath), &file, false, false);
        if(err < 0)
        {
            Send404(s);    /* 404 file not found */
            break;
        }
        else if (err == 1)
        {
            /* asking to list dir, redirect to 'index.html' if exists */
            sprintf(tmp, "%s/index.html", filepath);
            GetLocalFile(tmp, localPath, sizeof(localPath), &file, false, false);
            if(file && !authenticated)
            {
                /* fall through */
                strcpy(filepath, tmp);
            }
            else
            {
                CreateDirFile(filepath);
                SendDirectory(s, filepath);
                err = -1; /* disconnect */
                break;
            }
        }
        else if (err == 2)
        {
            strcat(filepath, "/");
            SendRedirect(s, filepath);
            err = -1;
            break;
        }

        getDateTime(lm_str);
        sprintf(tmp, "Date: %s\r\n", lm_str);
        strcat(buf, tmp);
        getFileModTime(localPath, lm_str);
        sprintf(tmp, "Last-Modified: %s\r\n", lm_str);
        strcat(buf, tmp);
        LookupContentTypeByExt(filepath, contentType);
        sprintf(tmp, "Content-Length: %d\r\nContent-Type: %s\r\n", GetFileSize(file), contentType);
        strcat(buf, tmp);
        strcat(buf, "\r\n");

        s->sSend(buf, strlen(buf));

        send_buf = new char[send_buf_size];
        while(send_buf && !file->eof())    /* read and send file */
        {
            size_t readsome = MIN(send_buf_size, ((total_bytes_per_sec * bwm_sleep_ms)/1000));
            bwm_sleep(bwm_sleep_ms);
            
            file->read(send_buf, readsome);
            unsigned long r = file->gcount();
            if(r <= 0)
                break;

            int ns = s->sSend(send_buf, r);
            if(ns <= 0)
                break;
        }
        
        if(send_buf) delete send_buf;
        delete file;

        err = -1; /* disconnect */
        break;
    /*****************************************************************/
    /********************HANDLE POST COMMAND *************************/
    case COMMAND_POST:

        err = HandlePOST(s, args, headersbuf);
        if(err != 0)
        {
            Send404(s);
            return err;
        }

        break;
    /*****************************************************************/
    /********************HANDLE HEAD COMMAND *************************/
    case COMMAND_HEAD:

        if(!strchr(gPermissions, 'r') || gAuth.isSecurePath(args))
        {
            AUTH_SEM_TAKE;
            bool b = gAuth.checkHeaders(headersbuf, args);
            AUTH_SEM_GIVE;
            if(!b)
            {
                SendAuth(s, NULL, false);
                break;
            }
        }

        /* find file */
        err = GetLocalFile(args, localPath, sizeof(localPath), &file, false, false);
        if(err != 0)
        {
            Send404(s);    /* 404 file not found */
            break;
        }

        strcpy(buf, "HTTP/1.0 200 OK");
        strcat(buf, crlf);
        getDateTime(lm_str);
        sprintf(tmp, "Date: %s\r\n", lm_str);
        strcat(buf, tmp);
        getFileModTime(localPath, lm_str);
        sprintf(tmp, "Last-Modified: %s\r\n", lm_str);
        strcat(buf, tmp);
        strcat(buf, "Connection: Keep-Alive\r\n");
        LookupContentTypeByExt(args, contentType);
        sprintf(tmp, "Content-Length: %d\r\nContent-Type: %s\r\n",
                GetFileSize(file), contentType);
        strcat(buf, tmp);

        strcat(buf, crlf);

        s->sSend(buf, strlen(buf));

        delete file;

        break;

    /*****************************************************************/
    /********************HANDLE OPTIONS COMMAND **********************/
    case COMMAND_OPTIONS:

        sprintf(buf, "HTTP/1.0 200 OK\r\n\r\n");
        s->sSend(buf, strlen(buf));
        err = 0;
        break;

    /*****************************************************************/
    /*****************UNKNOWN COMMAND ********************************/
    default:
        sprintf(buf, "HTTP/1.0 500 ERROR\r\n\r\n");
        s->sSend(buf, strlen(buf));

        err = -1;    /* disconnect after responding */
        break;
    }

    return err;
}

int HandlePOST(SimpleSocket* s, char* path, char *headersbuf)
{
    char filename[MAX_FILENAME_SIZE];
    char admin_args[MAX_FILENAME_SIZE];
    char admin_command[64];
    char* filePath = NULL;
    char* workptr;
    char* ptr, *ptr2;
    char finalBoundary[1024];
    char* boundaryptr;
    char buf[BUFFER_SIZE], pathbuf[BUFFER_SIZE], localPath[BUFFER_SIZE];
    int buf_remain = sizeof(buf)-1;
    char authpassword[255];
    char c;
    fstream *outfile;
    int i;
    int r;
    long contentLen;
    unsigned long total;
    bool postFailCleaner = true, b;

    memset(buf, 0, sizeof(buf));

    /* find content-length header */
    ptr = stristr(headersbuf, CONTENT_LENGTH_TAG);
    if(!ptr) return -1;
    ptr += strlen(CONTENT_LENGTH_TAG);
    ptr2 = strchr(ptr, '\r');
    if(!ptr2) return -1;
    *ptr2 = 0;
    contentLen = atoi(ptr);
    if(contentLen < 0) return -1;
    *ptr2 = '\r';

    /* get end-boundary - all forms should have this */
    boundaryptr = stristr(headersbuf, BOUNDARY_TAG);
    if(!boundaryptr)
    {
        return -1;
    }
    boundaryptr += strlen(BOUNDARY_TAG);

    memset(finalBoundary, 0, sizeof(finalBoundary));
    BuildFinalBoundary(boundaryptr, finalBoundary, sizeof(finalBoundary));
    if(finalBoundary[0] == 0)
    {
        delete filePath;
        return -1;
    }

    total = 0;

    AUTH_SEM_TAKE;
    b = gAuth.checkHeaders(headersbuf, path);
    AUTH_SEM_GIVE;

    if(strEndsWith(path, "/upload_file"))
    {
        if(strstr(gPermissions, "-u"))
        {
            /* don't allow upload */
            Send404WithMsg(s, "Operation not permitted");
            return 0;
        }

        if(!strchr(gPermissions, 'u') || gAuth.isSecurePath(path))
        {
            if(!b)
            {
                if(postFailCleaner)
                {
                    ReadAndBufferUntil(s, finalBoundary, false, NULL, 0, &total);
                }

                SendAuth(s, NULL, false);
                return 0;
            }
        }

        /* find the filename */
        if(ReadAndBufferUntil(s, FILE_NAME_TAG, false, NULL, 0, &total) < 0)
            return -1;

        r = ReadAndBufferUntil(s, "\r", false, filename, sizeof(filename)-1, &total);
        if(r < 0) return -1;
        filename[r] = 0;

        /* strip off leading dirs in the filename */
        ptr = filename + strlen(filename);
        while(ptr > filename)
        {
            if(*ptr == '\\' || *ptr == '/')
            {
                ptr++;
                break;
            }
            ptr--;
        }
        strcpy(filename, ptr);

        if(endsWith(filename, "index.html") || endsWith(filename, "index.html\"") || endsWith(filename, "index.html\'"))
        {
            /* must be logged in to upload index.html */
            if(!b)
            {
                Send404WithMsg(s, "Must be logged in to upload index.html file for security reasons");
                return 0;
            }
        }

        /* found the correct mimepart, now find the header-end */
        if(ReadAndBufferUntil(s, "\n\r\n", false, NULL, 0, &total) < 0)
            return -1;

        /* buld path to new local file */
        filePath = new char[strlen(path) + strlen(filename) + 2];
        if(!filePath) return -1;

        memset(filePath, 0, strlen(path) + strlen(filename) + 2);

        ptr = strrchr(path, '/');
        if(ptr == NULL)
        {
            delete filePath;
            return -1;
        }
        strncpy(filePath, path, ptr - path);
        filePath[ptr - path] = '/';

        CleanString(filename);
        ptr = strrchr(filename, '/');
        if(ptr == NULL)
        {
            ptr = filename;
        }
        strcat(filePath, ptr);
        /* build file path */
        r = GetLocalFile(filePath, localPath, sizeof(localPath), &outfile, true, b);
        if(r != 0)
        {
            delete filePath;
            if(postFailCleaner)
            {
              ReadAndBufferUntil(s, finalBoundary, false, NULL, 0, &total);
              Send404(s);
              return 0;
            }
            else
            {
              /* if they're uploading a big file, this'll fail quicker... */
              return -1;
            }
        }

        WriteUntilBoundaryFound(outfile, s, finalBoundary);

        delete outfile;
        SendUploadSuccessful(s);

        delete filePath;

        return 0;
    }
    else if(strEndsWith(path, "/admin_command"))
    {
        if(strstr(gPermissions, "-a"))
        {
            /* don't allow delete */
            Send404WithMsg(s, "Operation not permitted");
            return 0;
        }

        if(!strchr(gPermissions, 'a') || gAuth.isSecurePath(path))
        {
            AUTH_SEM_TAKE;
            bool b = gAuth.checkHeaders(headersbuf, path);
            AUTH_SEM_GIVE;
            if(!b)
            {
                SendAuth(s, NULL, false);
                return 0;
            }
        }

        while(total < contentLen)
        {
            if(s->ready(TIMEOUT_SECS, 0) <= 0) return -1;
            r = s->sRecv(buf + total, buf_remain);
            if(r <= 0) return -1;
            
            total += r;
            buf_remain -= r;
        }
        buf[total] = 0;

        /* find the command */
        if(simple_parse(buf, "name=\"cmd\"\r\n\r\n", "\r\n",
                        admin_command, sizeof(admin_command)) < 0)
        {
            return -1;
        }

        /* find the filename */
        if(simple_parse(buf, "name=\"admin_filename\"\r\n\r\n", "\r\n",
                        filename, sizeof(filename)) < 0)
        {
            return -1;
        }

        /* find other arguments */
        if(simple_parse(buf, "name=\"admin_args\"\r\n\r\n", "\r\n",
                        admin_args, sizeof(admin_args)) < 0)
        {
            return -1;
        }

        if(ContainsAnyChar(filename, "\r\n\"/\\")) return -1;

        ptr = path + (strlen(path) - strlen("/admin_command"));

        if(sizeof(pathbuf) <= ((ptr - path) + strlen(filename) + 10)) return -1;

        /* build delete path/file */
        *ptr = 0;
        if((ptr2 = strchr(path, '/')) == NULL)
        {
            sprintf(pathbuf, "./%s", filename);
        }
        else
        {
            sprintf(pathbuf, ".%s/%s", ptr2, filename);
        }
        *ptr = '/';

        if(strcmp(admin_command, "del") == 0)
        {
            if(DelLocalFile(pathbuf) < 0) return -1;
        }
        else if(strcmp(admin_command, "mkdir") == 0)
        {
            if(MkDir(pathbuf) < 0) return -1;
        }
        else if(strcmp(admin_command, "exec") == 0)
        {
            if(!strstr(gPermissions, "-e") && strstr(gPermissions, "e"))
            {
                int exec_r = SystemExec(pathbuf, admin_args); 
            }
            else
            {
                /* don't allow delete */
                Send404WithMsg(s, "Operation not permitted");
                return 0;
            }
        }
        else
        {
            return -1;
        }

        SendDeleteSuccessful(s);

        return 0;
    }
    else if(strEndsWith(path, "/auth"))
    {
        char tmp[COOKIE_LEN];

        if(!adminPasswordSet)
        {
            Send404WithMsg(s, "Login not allowed: admin password not set");
            return 0;
        }
        
        while(total < contentLen)
        {
            if(s->ready(TIMEOUT_SECS, 0) <= 0) return -1;
            r = s->sRecv(buf + total, buf_remain);
            if(r <= 0) return -1;
            
            total += r;
            buf_remain -= r;
        }
        buf[total] = 0;

        if(!(ptr = strstr(buf, "auth_password"))) return -1;

        if(!(ptr = strstr(ptr, "\r\n\r\n"))) return -1;
        ptr += 4;

        if(!(ptr2 = strstr(ptr, "\r\n"))) return -1;

        if((ptr2 - ptr)+1 > sizeof(authpassword)) return -1;
        memcpy(authpassword, ptr, ptr2 - ptr);
        authpassword[ptr2 - ptr] = 0;

        /* fork off here and first attempt to auth for secure-path */
        AUTH_SEM_TAKE;
        if(gAuth.authSecurePath(path, authpassword, tmp))
        {
            SendAuth(s, tmp, true);
            AUTH_SEM_GIVE;
            return 0;
        }
        AUTH_SEM_GIVE;

        if(strcmp(authpassword, gAuthPass) == 0)
        {
            char tmp[COOKIE_LEN];
            AUTH_SEM_TAKE;
            bool b = gAuth.newAuth(tmp);
            AUTH_SEM_GIVE;

            if(b)
            {
                SendAuth(s, tmp, true);
            }
            else
            {
                SendAuth(s, NULL, false);
            }
            return 0;
        }
        else
        {
            SendAuth(s, NULL, false);
            return 0;
        }
    }

    return -1;
}

/* USER INPUT SANITIZED HERE */
int ParseCommand(char* cmd, char* args, int args_len)
{
    int ret = -1;

    if(strncmp(cmd, "GET", 3) == 0)
    {
        int len = 0;
        char* farg = cmd + 4;
        while(!isWhiteSpc(*farg) && len < (args_len-1)) {
            args[len] = *farg;
            len++;
            farg++;
        }
        args[len] = 0;

        ret = COMMAND_GET;
    }
    else if(strncmp(cmd, "POST", 4) == 0)
    {
        int len = 0;
        char* farg = cmd + 5;
        while(!isWhiteSpc(*farg) && len < (args_len-1)) {
            args[len] = *farg;
            len++;
            farg++;
        }
        args[len] = 0;

        ret = COMMAND_POST;
    }
    else if(strncmp(cmd, "HEAD", 4) == 0)
    {
        int len = 0;
        char* farg = cmd + 5;
        while(!isWhiteSpc(*farg) && len < (args_len-1)) {
            args[len] = *farg;
            len++;
            farg++;
        }
        args[len] = 0;

        ret = COMMAND_HEAD;
    }
    else if(strncmp(cmd, "OPTIONS", 7) == 0)
    {
        int len = 0;
        char* farg = cmd + 8;
        while(!isWhiteSpc(*farg) && len < (args_len-1)) {
            args[len] = *farg;
            len++;
            farg++;
        }
        args[len] = 0;

        ret = COMMAND_OPTIONS;
    }
    else
    {
        return -1;
    }

    /* parse path of get-line */
    UnescapeHttp(args);

    if(args[0] != '/')
    {
        return -1;
    }

    /* check for /../ traversal */
    if(strstr(args, "/..") != NULL)
    {
        return -1;
    }

    /* hokey check to limit the args len to BUFFER_SIZE-50
     * so that we can append things to make the path local */
    if(BUFFER_SIZE - strlen(args) < 50)
    {
        return -1;
    }

    /* AT THIS POINT, args IS SANITIZED */

    return ret;
}

void SendUploadSuccessful(SimpleSocket* s)
{
    int r;
    char* contentStr =
           "<HTML><META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\">"
           "<BODY BGCOLOR=\"CBCBCB\"><P>Upload Successful!</P>"
           "<INPUT type='button' value='OK' "
           "onclick=\"javascript:window.location='./'\"></HTML>";
    char buf[BUFFER_SIZE];

    sprintf(buf, "HTTP/1.0 200 OK\r\nConnection: close\r\n"
                 "Content-Type: text/html\r\nContent-Length: %d\r\n\r\n"
                 "%s",
                 strlen(contentStr), contentStr);
    r = 0;
    while(r < strlen(buf))
    {
        int t;
        t = s->sSend(buf, strlen(buf));
        if(t <= 0) break;
        r += t;
    }

    return;
}

void SendDeleteSuccessful(SimpleSocket* s)
{
    int r;
    char* contentStr = 
        "<HTML><META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\">"
        "<BODY BGCOLOR=\"CBCBCB\"><P>Command Successful!</P>"
        "<INPUT type='button' value='OK' "
        "onclick=\"javascript:window.location='./'\"></HTML>";
    char buf[BUFFER_SIZE];

    strcat(buf, contentStr);
    sprintf(buf, "HTTP/1.0 200 OK\r\nConnection: close\r\n"
                 "Content-Type: text/html\r\nContent-Length: %d\r\n\r\n"
                 "%s",
                 strlen(contentStr), contentStr);
    r = 0;
    while(r < strlen(buf))
    {
        int t;
        t = s->sSend(buf, strlen(buf));
        if(t <= 0) break;
        r += t;
    }

    return;
}

unsigned int GetFileSize(fstream* file)
{
    file->seekg(0, ios_base::end);
    unsigned int eof = file->tellg();
    file->seekg(0, ios_base::beg);

    return eof;
}

bool endsWith(char* str, char* end)
{
    char* ptr = str + ((strlen(str)-1)-strlen(end));
    if(ptr < str)
        return false;

    if(strcmp(str, end) == 0)
        return true;

    return false;
}

void UnescapeHttp(char* str)
{
    char escapecode[10];
    int eclen, r;
    char* end = str + strlen(str);
    char* dest = str;
    char* src = str;

    eclen = -1;
    while(src < end)
    {
        char c = *src;

        if(c == '%' && eclen < 0)
        {
            eclen = 0;
            src++;
            continue;
        }

        if(eclen >= 0)
        {
            escapecode[eclen] = c;
            eclen++;
            if(eclen == 2)
            {
                int code;

                escapecode[2] = 0;
                r = sscanf(escapecode, "%x", &code);
                if(r < 1
                   || code == 0
                   || code == '.') code = ' ';
                eclen = -1;
                *dest = (char) code;
                dest++;
            }

            src++;
            continue;
        }

        *dest = *src;

        src++;
        dest++;
    }

    *dest = 0;
}

struct write_helper_fn_struct
{
    fstream *file;
    size_t count;
};

static void
write_fn(void *ptr, char *buf, size_t l)
{
    struct write_helper_fn_struct *data = (struct write_helper_fn_struct *) ptr;
    data->file->write(buf, l);
    data->count += l;
}

int WriteUntilBoundaryFound2(fstream* file, SimpleSocket* s,
                            char* endBoundaryStr)
{
    struct write_helper_fn_struct write_data;
    boundary_search_state search_state;
    int l = strlen(endBoundaryStr);
    char buf[512];

    write_data.file = file;
    write_data.count = 0;

    read_until_boundary_init(&search_state, endBoundaryStr, write_fn, &write_data);
    while(1)
    {
        if(s->ready(TIMEOUT_SECS, 0) <= 0)
        {
            break;
        }

        int r = s->sRecv(buf, sizeof(buf));
        if(r <= 0) break;

        if(read_until_boundary(buf, r, &search_state) == 1)
        {
            break;
        }
    }
    read_until_boundary_uninit(&search_state);

    return write_data.count;
}

int WriteUntilBoundaryFound(fstream* file, SimpleSocket* s,
                            char* endBoundaryStr)
{
    // crash somewhere in this function, for now use slow-dump version
    return WriteUntilBoundaryFound2(file, s, endBoundaryStr);

    char* buf1;
    char* buf2;
    char* end;
    int r;
    int boundaryLen;
    int count = 0;
    char *buf;
    int buf_sz = 8000;

    buf = new char[buf_sz];
    if(!buf) return -1;

    boundaryLen = strlen(endBoundaryStr);

    memset(buf, 0, buf_sz);

    buf1 = buf;

    /* fill buf1 */
    if(s->ready(TIMEOUT_SECS, 0) <= 0)    return -1;
    r = s->sRecv(buf1, buf_sz/2);
    if(r <= 0) return -1;

    buf2 = buf + r;

    end = mymemsearch(buf, buf_sz, endBoundaryStr, boundaryLen);
    if(!end)
    {
        while(1)
        {
            /* fill buffer 2 */
            memset(buf2, 0, buf_sz - (buf2 - buf));

            if(s->ready(TIMEOUT_SECS, 0) <= 0)    return -1;
            r = s->sRecv(buf2, buf_sz/2);
            if(r <= 0) break;

            /* buffer length must be at least the boundary-length, so we dont miss it */
            while(((buf2 - buf) + r) < boundaryLen)
            {
                int m;
                int sl = buf_sz - ((buf2 - buf) + r);
                m = s->sRecv(buf2 + r, sl);
                if(m <= 0) return -1;

                r += m;
            }
            
            end = mymemsearch(buf, buf_sz, endBoundaryStr, boundaryLen);
            if(end)
            {
                break;
            }

            /* write out buf1 */
            count += buf2 - buf1;
            file->write(buf1, buf2 - buf1);

            /* move buf2 to buf1 */
            memcpy(buf1, buf2, r);
            buf2 = buf1 + r;
        }
    }

    file->write(buf, end - buf);
    count += end - buf;

    delete buf;


    return count;
}

void BuildFinalBoundary(char* boundary, char* finalBoundary, int fb_len)
{
    char* rptr;

    memset(finalBoundary, 0, fb_len);
    int newlen = strlen(boundary) + 6;

    if(newlen > fb_len) return;

    rptr = boundary;
    while(*rptr != '\r' && *rptr != '\n' && *rptr != 0)
        rptr++;

    int bl = rptr - boundary;

    strcpy(finalBoundary, "\r\n--");
    strncat(finalBoundary, boundary, bl);
    strcat(finalBoundary, "--");
}

char * stristr(char *haystack, char *needle)
{
    char *np = needle, tmph, tmpn;

    while( (tmph = *haystack) ){
        if( (tmpn = *np) && toupper(tmpn) == toupper(tmph) )
            np++;
        else{
            if(!tmpn)
            return (haystack - strlen(needle));
            np = needle;
        }
        haystack++;
    }

    return NULL;
}

bool strEndsWith(char* haystack, char* needle)
{
    char* ptr;
    ptr = haystack + (strlen(haystack)-1);
    ptr -= strlen(needle)-1;

    if(ptr < haystack) return false;

    if(strcmp(ptr, needle) == 0) return true;

    return false;
}

/* take the path/file that was in request and convert to local file pointer
 * returns:
 * -1 if file not found
 * 0 for success (fileHdl valid)
 * 1 if this is a dir
 * 2 if this is a dir (append '/' and redirect) */
int GetLocalFile(char* pathFileName, char *localPath, int localPathSize, fstream** file, bool create, bool replace)
{
    fstream* newFile;
    unsigned pathFileNameLen;

    pathFileNameLen = strlen(pathFileName);
    if(pathFileNameLen == 0) return -1;
    /* must have 10 byte reserve to convert to a local path */
    if(pathFileNameLen > localPathSize-10) return -1;
    
    memset(localPath, 0, sizeof(localPath));
    strcat(localPath, "./");
    strcat(localPath, pathFileName);

    if(TestDirectory(localPath) > 0)
    {
        if(pathFileName[pathFileNameLen-1] != '/')
        {
            return 2;
        }
        return 1;
    }

    ConvertSlashes(localPath);

    if(create)
    {
        newFile = fstreamWrapper(localPath, false, true, true, replace);
    }
    else
    {
        newFile = fstreamWrapper(localPath, true, false, false, replace);
    }

    if(newFile == NULL)
    {
        return -1;
    }

    if(!newFile->is_open()) 
    {
        return -1; /* 404 file not found */
    }

    if(!newFile->good())
    {
        return -1;
    }

    *file = newFile;

    return 0;
}

void Log(char* msg, char* clientCommand, char* localCmdFile)
{
    if(msg != NULL)
        cout << msg << msg << endl;

    if(clientCommand != NULL)
        cout << clientCommand << endl;

    if(localCmdFile != NULL)
        cout << localCmdFile << endl;
}

fstream* fstreamWrapper(char* pathfile, bool in, bool out,
                        bool create, bool replace)
{
    bool exists = false;
    fstream fs(pathfile, ios_base::in);
    if(!fs)
    {
        exists = false;
    }
    else
    {
        exists = true;
    }
    fs.close();

    if(exists)
    {
        if(replace)
        {
            return new fstream(pathfile, (in ? ios_base::in : ios_base::binary) 
                               | (out ? ios_base::out : ios_base::binary)
                               | ios_base::binary);
        }
        else
        {
            if(out)
                return NULL;
            else
                return new fstream(pathfile,
                                   (in ? ios_base::in : ios_base::binary)
                                   | ios_base::binary);
        }
    }
    else
    {
        if(!create)
        {
            return NULL;
        }
        else
        {
            return new fstream(pathfile,
                               (in ? ios_base::in : ios_base::binary)
                               | (out ? ios_base::out : ios_base::binary)
                               | ios_base::binary);
        }
    }
}

int validate_ascii(char* buf, int len)
{
    char* ptr = buf;
    char* end = ptr + len;

    while(ptr < end)
    {
        if(*ptr < 0x20 || *ptr > 0x7e)
            return 0;
        ptr++;
    }

    return 1;
}

int sendBuf(SimpleSocket* s, char* buf, int len)
{
    return s->sSend(buf, len);
}

int HandleSpecialFile(char *args, SimpleSocket* s, char *req_headers)
{
    char headers[1024];

    if(strEndsWith(args, "/auth"))
    {
        SendAuthPage(s);
        return 1;
    }
    else if(strEndsWith(args, "/auth_logout"))
    {
        gAuth.logoutSession(req_headers);
        SendAuthPage(s);
        return 1;
    }
    else if(strEndsWith(args, "/http_icon0.gif"))
    {
        sprintf(headers, 
            "HTTP/1.0 200 OK\r\nConnection: Close\r\n"
            "Content-Length:%d\r\nContent-Type:image/gif\r\n\r\n",
            sizeof(gopher_unknown_gif));
        sendBuf(s, headers, strlen(headers));
        sendBuf(s, (char*) gopher_unknown_gif, sizeof(gopher_unknown_gif));
        return 1;
    }
    else if(strEndsWith(args, "/http_icon1.gif"))
    {
        sprintf(headers, 
            "HTTP/1.0 200 OK\r\nConnection: Close\r\n"
            "Content-Length:%d\r\nContent-Type:image/gif\r\n\r\n",
            sizeof(gopher_menu_gif));
        sendBuf(s, headers, strlen(headers));
        sendBuf(s, (char*) gopher_menu_gif, sizeof(gopher_menu_gif));
        return 1;
    }

    return 0;
}

int GrabPasswordFromFile(char *filename, char *dest, int dest_len)
{
    char *ptr = dest;
    fstream f(filename, ios_base::in);
    if(!f.good() || !f.is_open()) return 0;
    f.read(dest, dest_len-1);
    dest[f.gcount()] = '\0';
    while(*ptr != ' ' && *ptr != '\r' && *ptr != '\n' && *ptr != '\0')
        ptr++;
    *ptr = '\0';
    return 1;
}

void handle_sigpipe(int s)
{
}
