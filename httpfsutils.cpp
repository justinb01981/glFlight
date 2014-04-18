/* Windows specific functions */

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "simplesocket.h"
#include "httpfsutils.h"
#include "mime_types.h"
#include "auth.h"
#include "aho.h"

#ifdef _UNIX_
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef _BWINDOWS_

#define LIST_FILE_COMMAND "dir "
#define LIST_FILE_NAME ".httpfs_dir.txt"
#define DEL_LIST_FILE_COMMAND "del /Q /F "
#define MAKE_DIR_COMMAND "mkdir "
#define DEL_DIR_COMMAND "rmdir /Q /S "
#define DIR_SLASH '\\'

#else

#define LIST_FILE_COMMAND "ls -1p "
#define LIST_FILE_NAME ".httpfs_dir.txt"
#define DEL_LIST_FILE_COMMAND "rm -rf "
#define MAKE_DIR_COMMAND "mkdir "
#define DIR_SLASH '/'
#endif 

using namespace std;

#define BUFFER_SIZE 2000

unsigned long GetMyThreadId();


int SystemCall(char* str)
{
    int r = 0;

    if(strstr(str, ".."))
    {
        return -1;
    }

    r = system(str);
    return r;
}

int SystemParamSafe(char* param)
{
    /* if the command contains a char that is unsafe, return 0 */
    /* for now, all system params should be wrapped in "" - only that char is unsafe */
    char* ptr = param;
    while(*ptr)
    {
        if(*ptr == '\"')
            return 0;
        ptr++;
    }
    return 1;
}

bool isWhiteSpc(char c)
{
    char* validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_[]()/-+'.%!";

    for(unsigned int i = 0; i < strlen(validChars); i++)
        if(c == validChars[i])
            return false;

    return true;
}

char ContainsAnyChar(char* haystack, char* needles)
{
    int i = 0, nlen;
    
    nlen = strlen(needles);

    for(i = 0; i < nlen; i++)
    {
        if(strchr(haystack, needles[i]))
        {
            return needles[i];
        }
    }
    return 0;
}

void ConvertSlashes(char* str)
{
#ifdef _BWINDOWS_
    for(unsigned int i=0; i < strlen(str); i++)
        if(str[i] == '/')
            str[i] = '\\';
#else
#endif
}

void CleanString(char* str)
{
    /* remove whitespace, and quotes.. */
    char* begin = str;
    char* end = str + strlen(str);

    while(*begin == '\"' || *begin == ' ' || *begin == '\t' && *begin != 0)
    {
        begin++;
    }

    end = str + strlen(str);
    while((*end == '\"' || *end == ' ' || *end == '\t' || *end == '\r' || *end == '\n' || *end == 0) && end > begin)
    {
        end--;
    }
    end++;

    memmove(str, begin, end - begin);
    str[end - begin] = 0;
}

void EscapeHttp(char* str, int len)
{
    int dest, i;
    char escapecode[64];
    char* newbuf = new char[len+1];
    char* allowedChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-=./";

    if(!newbuf) return;

    newbuf[0] = 0;

    dest = 0;
    for(i = 0; i < (int) strlen(str); i++)
    {
        if((len - dest) <= 3)
        {
            delete newbuf;
            return;
        }

        if(strchr(allowedChars, str[i]))
        {
            newbuf[dest] = str[i];
            dest++;
            newbuf[dest] = 0;
        }
        else
        {
            sprintf(escapecode, "%%%x", (int) str[i]);
            strcat(newbuf, escapecode);
            dest += strlen(escapecode);
        }
    }

    strcpy(str, newbuf);

    delete newbuf;
}

void readLine(fstream* file, char* line, int maxlen)
{
    int r = 0;

    while(r < maxlen && !file->eof())
    {
        file->read(&line[r], 1);
        
        if(file->gcount() <= 0)
          break;

        if(line[r] == '\n' || line[r] == '\r')
            break;

        r++;
    }
    line[r] = 0;

    return;
}

void Send404WithMsg(SimpleSocket* s, char *msg)
{
    char buf[512];

    sprintf(buf, "HTTP/1.0 404 Not Found\r\n" \
        "Connection: Close\r\n" \
        "Content-Length: %d\r\n" \
        "Content-Type: text/html\r\n\r\n", strlen(msg));
    s->sSend(buf, strlen(buf));
    s->sSend(msg, strlen(msg));

    return;
}

void Send404(SimpleSocket *s)
{
    char* fnf = "404 - File not found";

    Send404WithMsg(s, fnf);
}

void SendRedirect(SimpleSocket* s, char *location)
{
    char *p1 =
        "HTTP/1.0 302 Object moved\r\n" \
        "Location: ";
    char *p2 = "\r\n" \
        "Content-Length: 0\r\n\r\n";

    s->sSend(p1, strlen(p1));
    s->sSend(location, strlen(location));
    s->sSend(p2, strlen(p2));

    return;
}

void SendAuth(SimpleSocket* s, char* cookie, bool success)
{
    char* str;
    char buf[2048];

    if(success)
    {
        str = "<HTML><META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\"><BODY BGCOLOR=\"CBCBCB\"><P>Login successful! This browser session is now logged in. <FORM name='theForm'><INPUT type='button' value='OK' onclick=\"javascript:window.location='./'\"></FORM></HTML>";
        sprintf(buf, "HTTP/1.0 200 OK\r\nConnection: keep-alive\r\nContent-Length: %d\r\nContent-Type: text/html\r\nSet-Cookie: %s%s; path=/\r\n\r\n%s", strlen(str), AUTH_COOKIE_TAG, cookie, str);
    }
    else
    {
        /* changed the response code to 401 - unauthorized */
        str = "<HTML><META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\"><BODY BGCOLOR=\"CBCBCB\">Must log in before you can do that, or your password was incorrect!<BR>Click <A HREF=\"./auth\">here</A>...</HTML>";
        sprintf(buf, "HTTP/1.0 401 OK\r\nConnection: keep-alive\r\nContent-Length: %d\r\nContent-Type: text/html\r\n\r\n%s", strlen(str), str);
    }

    s->sSend(buf, strlen(buf));
}

void SendAuthPage(SimpleSocket* s)
{
    char* str;
    char buf[2048];

    str = (char*) gAuthPageBuf;
    sprintf(buf, "HTTP/1.0 200 OK\r\nConnection: keep-alive\r\nContent-Length:%d\r\nContent-Type:text/html\r\n\r\n%s", strlen(str), str);

    s->sSend(buf, strlen(buf));
}

int ReadAndBufferUntil(SimpleSocket* s, char* str, bool matchCase, char* destbuf, long destbuf_size, unsigned long* count)
{
    long i = 0;
    int k;
    int sl = strlen(str);
    char *buf = new char[sl];
    int o = 0;
    int r;

    if(!buf) return -1;

    if(s->ready(TIMEOUT_SECS, 0) <= 0) return -1;

    /* fill buffer first */
    r = 0;
    while(r < sl)
    {
        int t;

        if(s->ready(TIMEOUT_SECS, 0) <= 0){ delete buf; return -1; }
        t = s->sRecv(buf+r, sl - r);
        if(t <= 0) { delete buf; return -1; }
        r += t;
    }
    o = (o+r)%sl;

    if(count)*count += r;

    while(1)
    {
        /* copy char falling out of buffer to destbuf */
        if(destbuf_size > 0 && (destbuf_size - i) > 0)
        {
            destbuf[i] = buf[o];
            i++;
        }

        if(s->ready(TIMEOUT_SECS, 0) <= 0) break;

        if((r = s->sRecv(buf + o, 1)) <= 0) break;

        if(count)*count += r;

        /* compare buffer with str */
        k = sl;
        while(1)
        {
            /*if(buf[(o + k)%sl] == str[k-1])*/
            if( (matchCase? buf[(o + k)%sl] == str[k-1] : tolower(buf[(o + k)%sl]) == tolower(str[k-1])) )
            {
                k--;
                if(k == 0){ delete buf; return i;}
                continue;
            }
            else
                break;
        }
        /* end comparison loop */

        o = (o+1)%sl;
    }

    delete buf;

    return -1;
}

void LookupContentTypeByExt(char* str, /* at most 64 chars in length */ char* dest)
{
    char* extension, *buf, *ptr;
    int i = 0;
    char* mime_type_unknown = "application/octet-stream";

    extension = strrchr(str, '.');
    if(extension == NULL)
    {
        strcpy(dest, mime_type_unknown);
        return;
    }
    extension++;

    buf = new char[strlen(extension)+1];
    if(!buf) {
        strcpy(dest, mime_type_unknown);
        return;
    }
    ptr = buf;
    for(;*extension != 0;extension++) {
        *ptr = tolower(*extension);
        ptr++;
    }
    *ptr = 0;

    i = 0;
    while(mime_table[i])    {
        if(strcmp(buf, mime_table[i]) == 0 && mime_table[i+1]) {
            strcpy(dest, mime_table[i+1]);
            delete buf;
            return;
        }
        i++;
    }

    strcpy(dest, mime_type_unknown);

    delete buf;
}

int MkDir(char* pathFileName)
{
    char* str;

    if(strstr("..", pathFileName) != NULL
       || !SystemParamSafe(pathFileName)) return -1;

    str = new char[strlen(pathFileName) + 64];
    if(!str) return -1;

    sprintf(str,"%s \"%s\"", MAKE_DIR_COMMAND, pathFileName);

    ConvertSlashes(str + strlen(MAKE_DIR_COMMAND));

    SystemCall(str);

    delete str;

    return 0;
}

int SystemExec(char* pathFileName, char* args)
{
    int r;
    char* str;

    if(strstr("..", pathFileName) != NULL
       || !SystemParamSafe(pathFileName)) return -1;

    str = new char[strlen(pathFileName) + 64];
    if(!str) return -1;

    sprintf(str,"\"%s\" %s", pathFileName, args);

    ConvertSlashes(str);

    r = SystemCall(str);

    delete str;

    return r;
}


int TestDirectory(char *localpath)
{
    char testcmd[2048];
    int success;

    if(strlen(localpath) >= 2000) return -1;

#ifdef _UNIX_
    success = 0;
    sprintf(testcmd, "ls \"%s\"/ 1>/dev/null 2>/dev/null", localpath);
#else
    success = 0;    /* ? */
    /* todo - pipe to /dev/NUL */
    sprintf(testcmd, "dir \"%s\"\\ > NUL", localpath);
#endif
    return (SystemCall(testcmd) == success? 1: 0);
}

int DelLocalFile(char* pathFileName)
{
    char* str;

    if(strstr("..", pathFileName) != NULL
       || !SystemParamSafe(pathFileName)) return -1;

    str = new char[strlen(pathFileName) + 64];
    if(!str) return -1;

    sprintf(str,"%s \"%s\"", DEL_LIST_FILE_COMMAND, pathFileName);

    ConvertSlashes(str + strlen(DEL_LIST_FILE_COMMAND));

    SystemCall(str);

#ifdef _BWINDOWS_
    sprintf(str, "%s \"%s\"", DEL_DIR_COMMAND, pathFileName);
    ConvertSlashes(str + strlen(DEL_DIR_COMMAND));

    SystemCall(str);
#endif

    delete str;

    return 0;
}

int SendDirectoryHelper(SimpleSocket* s, char* path, int size_only)
{
    char htmlbuffer[BUFFER_SIZE];
    char file_path[BUFFER_SIZE];
    char list_file_path[BUFFER_SIZE];
    char deletecmd[BUFFER_SIZE];
    int r = 0, size = 0, headerslen = 0;

    if(!SystemParamSafe(path))
    {
        Send404(s);
        return -1;
    }

    sprintf(list_file_path, ".%s%s%d",
         path, LIST_FILE_NAME, GetMyThreadId());

    fstream* file = new fstream(list_file_path, ios_base::in|ios_base::binary);
    if(!file->good())
    {
        Send404(s);
        delete file;
        return -1;
    }

    if(!size_only)
    {
        size = SendDirectoryHelper(s, path, 1);
        if(size < 0)
        {
            delete file;
            return -1;
        }
    }
    sprintf(htmlbuffer, "HTTP/1.0 200 OK\r\n");
    sprintf(htmlbuffer + strlen(htmlbuffer), 
            "Content-Type: text/html\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",
            size);

    headerslen = strlen(htmlbuffer);
    
    sprintf(htmlbuffer + strlen(htmlbuffer), "%s <H2>%s</H2>", gIndexPageHead, path);

    /* append "Up one directory" link */
    strcat(htmlbuffer, "<A HREF='../'>Up one level</A><BR>");

    while(!file->eof())
    {
        char line[1024];
        char filename[300];
        bool isdir = false;
        char* ptr;


        /* read a line from the file */
        readLine(file, line, sizeof(line));    /* read a line */
        if(strlen(line) == 0 || isWhiteSpc(line[0]))    /* if it starts with whitespace, skip this line */
        {
            continue;
        }
        
#if _BWINDOWS_
        /* find actual file/dir name */
        ptr = line + strlen(line)-1;
        while(ptr > line
            && (ptr - line >= 40)    /* in dir command (win2k/XP DOS), filename starts at 40th byte of line */
            && (*(ptr-1) != 13))
        {
            ptr--;
        }
#else        
        ptr = line;
#endif

        if(strlen(ptr) > (sizeof(filename)-1)) 
            continue;

        strcpy(filename, ptr);

        /* skip these dirs */
        if((strcmp(filename, "..") == 0) 
           || (strcmp(filename, ".") == 0)
           || (strncmp(filename, LIST_FILE_NAME, strlen(LIST_FILE_NAME)) == 0)
           || isWhiteSpc(filename[0]))
        {
            continue;
        }

        /* is this a directory? */
#if _BWINDOWS_
        if(strstr(line, "<DIR>") != NULL) isdir = true;
#else
        if(filename[strlen(filename)-1] == '/') isdir = true;
#endif    

        strcpy(file_path, filename);

        EscapeHttp(file_path, sizeof(file_path)-1);


        /* append icon.gif */
        if(isdir) strcat(htmlbuffer, "<IMG SRC=\"./http_icon1.gif\">");
        else strcat(htmlbuffer, "<IMG SRC=\"./http_icon0.gif\">");

        /* append a link to the HTML */
        strcat(htmlbuffer, "<A HREF='./");
        strcat(htmlbuffer, file_path);
        strcat(htmlbuffer, "'>");
        strcat(htmlbuffer, filename);
        strcat(htmlbuffer, "</A><BR>");

        if(size_only) {
            size += strlen(htmlbuffer);
        }
        else {
            s->sSend(htmlbuffer, strlen(htmlbuffer));
        }

        memset(htmlbuffer, 0, sizeof(htmlbuffer));
    }

    strcat(htmlbuffer, gUploadFileBuffer1);
    strcat(htmlbuffer, path);
    strcat(htmlbuffer, gUploadFileBuffer2);
    strcat(htmlbuffer, "</HTML>");

    if(size_only) {
        size += strlen(htmlbuffer);
    }
    else {
        s->sSend(htmlbuffer, strlen(htmlbuffer));
    }

    if(size_only) {
        delete file;
        return size - headerslen;
    }

    /* now what? dont create the file twice! */
    delete file;
    ConvertSlashes(list_file_path);
    sprintf(deletecmd, "%s \"%s\"", DEL_LIST_FILE_COMMAND, list_file_path);
    SystemCall(deletecmd);

    return 0;
}

int SendDirectory(SimpleSocket* s, char* path)
{
    return SendDirectoryHelper(s, path, 0);
}

void MutexSemInit(Semaphore* sem)
{
#ifdef _BWINDOWS_
    sem->mutex1 = CreateMutex(NULL, false, NULL);
#endif
#ifdef _UNIX_
    pthread_mutex_init(&sem->mutex1, NULL);
#endif
}

void MutexSemDestroy(Semaphore* sem)
{
#ifdef _BWINDOWS_
    CloseHandle(sem->mutex1);
#endif
#ifdef _UNIX_
    pthread_mutex_destroy(&sem->mutex1);
#endif
}

unsigned long GetMyThreadId()
{
#if _BWINDOWS_
    return (unsigned long) GetCurrentThread();
#else
    return (unsigned long) pthread_self();
#endif
}

/* on windows, use GetCurrentThread(), on linux use pthread_self */
/* the semaphore should contain the ID of the current thread that holds the sem */
void MutexSemTake(Semaphore* sem)
{
    int i = 0;

#ifdef _BWINDOWS_
    while(true)
    {
        if(WaitForSingleObject(sem->mutex1, INFINITE) == 0)
            break;
    }
#endif

#ifdef _UNIX_
    pthread_mutex_lock(&sem->mutex1);
#endif
}

void MutexSemGive(Semaphore* sem)
{
/*    MutexSemTake(sem);*/    /* we're not using counting semaphores */

#ifdef _BWINDOWS_
    ReleaseMutex(sem->mutex1);
#endif
#ifdef _UNIX_
    pthread_mutex_unlock(&sem->mutex1);
#endif
}

void CreateDirFile(char* path)
{
    char systemcmd[4096];

    if(strlen(path) > (sizeof(systemcmd)/2 - 64))
        return;

    if(!SystemParamSafe(path))
        return;

    /* generate directory list file */
    sprintf(systemcmd, "%s \".%s\" > \".%s%s%d\"",LIST_FILE_COMMAND, path, path, LIST_FILE_NAME, GetMyThreadId());
    ConvertSlashes(systemcmd + strlen(LIST_FILE_COMMAND));

    SystemCall(systemcmd);
}

/* strdest must be >= 64 chars */
int getFileModTime(char *path, char *strdest)
{
#ifdef _UNIX_
    struct stat file_stat;
    struct tm ts;
    int destlen;

    strdest[0] = 0;
    if(stat(path, &file_stat) == 0)
    {
        /* get the time in UTC */
        gmtime_r(&file_stat.st_mtime, &ts);
        asctime_r(&ts, strdest);
        destlen = strlen(strdest);
        if(destlen > 0 && strdest[destlen-1] == '\n')
        {
            strdest[destlen-1] = 0;
        }
        strcat(strdest, " GMT");
 
        return 0;
    }
    return -1;
#else
    /* TODO: implement for windows */
    sprintf(strdest, "");
    return 0;
#endif
}

int getDateTime(char *strdest)
{
    int len;
    time_t tm;
    struct tm ts;

    strdest[0] = 0;

    tm = time(NULL);
#ifdef _BWINDOWS_
    gmtime_s(&ts, &tm);
    strcpy(strdest, asctime(&ts));
#else
    gmtime_r(&tm, &ts);
    asctime_r(&ts, strdest);
#endif
    len = strlen(strdest);
    if(len > 0 && strdest[len-1] == '\n')
    {
        strdest[len-1] = 0;
    }
    strcat(strdest, " GMT");
    return 0;
}

void bwm_sleep(int msec)
{
#ifdef _UNIX_
    usleep(msec*1000);
#else
    Sleep(msec);
#endif
}

/* returns: num of bytes copied to dest, -1 on fail */
int simple_parse(char *buf, char *beginkey, char *endkey, char* dest, int dest_len)
{
    int copied;
    char *ptr, *eptr;

    ptr = strstr(buf, beginkey);
    if(!ptr) return -1;

    ptr += strlen(beginkey);

    if(!endkey)
    {
        /* no end-tag specified */
        eptr = ptr + strlen(ptr);
    }
    else
    {
        /* if end-tag not found, return an error */
        eptr = strstr(ptr, endkey);
        if(!eptr) return -1;
    }
    
    copied = 0;
    while(ptr != eptr && copied < dest_len-1)
    {
        dest[copied] = *ptr;
        copied++;
        ptr++;
    }
    if(ptr != eptr) return -1;
    
    dest[copied] = '\0';
    return copied;
}

void pipe_to_devnull(FILE *fp)
{
#if _BWINDOWS_
    freopen("NUL", "w", fp);
#else
    freopen("/dev/null", "w", fp);
#endif
}
