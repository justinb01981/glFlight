#ifndef HTTPFS_UTILS
#define HTTPFS_UTILS

#include <stdio.h>
#include "simplesocket.h"

#define MIN(x,y) ((x) < (y)? (x): (y))

#define TIMEOUT_SECS 30

static const char* gIndexPageHead = "<HTML><META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\"><BODY BGCOLOR=\"CBCBCB\">";
static const char* gUploadFileBuffer1 = "<BR><BR><HR><P>Upload file:<FORM name='theform' enctype='multipart/form-data' method='POST' action='./upload_file'><INPUT type='hidden' name='path' value='";
static const char* gUploadFileBuffer2 = "'><INPUT type='file' name='upload_file' size=30><BR><INPUT type='button' value='Upload' onclick='this.form.submit()'></FORM><HR>\n \
                                        <P>Admin: <A HREF=\"./auth\">(Click here to login)</A>, or <A HREF=\"./auth_logout\">(here to logout)</A><FORM name='theform2' enctype='multipart/form-data' method='POST' action='./admin_command'><SELECT name='cmd'><OPTION value='del'>Delete\n<OPTION value='mkdir'>Create Directory<OPTION value='exec'>exec\n</SELECT>&nbsp;&nbsp; name:<INPUT type='text' name='admin_filename'><BR>optional arguments:<INPUT type='text' name='admin_args'><BR><INPUT type='button' value='Go' onclick='this.form.submit()'></FORM>";
static const char* gAuthPageBuf = "<HTML><META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\"><BODY BGCOLOR=\"CBCBCB\"><P>Password:<FORM name='theform2' enctype='multipart/form-data' method='POST' action='./auth'><INPUT type='password' name='auth_password'> <INPUT type='button' value='Login' onclick='this.form.submit()'></FORM></HTML>";

#ifdef _BWINDOWS_
typedef HANDLE MutexType;
#endif
#ifdef _UNIX_
typedef pthread_mutex_t MutexType;
#endif

typedef struct {
    MutexType mutex1;
} Semaphore;


void ConvertSlashes(char* str);
void CleanString(char* str);
void EscapeHttp(char* str, int len);
bool isWhiteSpc(char c);
char ContainsAnyChar(char* haystack, char* needles);
void Send404(SimpleSocket* s);
void Send404WithMsg(SimpleSocket* s, char *msg);
void SendRedirect(SimpleSocket* s, char *location);
void SendAuth(SimpleSocket* s, char* cookie, bool success);
void SendAuthPage(SimpleSocket* s);
void SendContinue(SimpleSocket* s);
int ReadAndBufferUntil(SimpleSocket* s, char* str, bool matchCase, char* destbuf, long destbuf_size, unsigned long* count);
void LookupContentTypeByExt(char* str, /* at most 64 chars in length */ char* dest);
int MkDir(char* pathFileName);
int DelLocalFile(char* pathFileName);
int SystemExec(char* pathFileName, char* args);
void MutexSemInit(Semaphore* sem);
void MutexSemDestroy(Semaphore* sem);
void MutexSemTake(Semaphore* sem);
void MutexSemGive(Semaphore* sem);

void CreateDirFile(char* path);
int SendDirectory(SimpleSocket* s, char* path);
int TestDirectory(char *localpath);
int getFileModTime(char *path, char *strdest);
int getDateTime(char *strdest);
void bwm_sleep(int msec);
int simple_parse(char *buf, char *beginkey, char *endkey, char* dest, int dest_len);
void pipe_to_devnull(FILE *fp);
#endif /* HTTPFS_UTILS */
