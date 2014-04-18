/* session-cookie based authentication */

#include "auth.h"
#include <time.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>

#define ADMIN_FAKE_ID 0

using namespace std;


static char* cookiechars = "abcdefghijklmnopqrstuvwxyz"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/* returns 1 if prefix is a prefix of path, case insensitive */
static int comparePath(char *prefix, char *path)
{
    int i;

    for(i = 0; i < strlen(prefix); i++)
    {
        if(i >= strlen(path)) return 0;
        else if(tolower(prefix[i]) != tolower(path[i])) return 0;
    }
    return 1;
}

Auth::Auth()
{
    memset(cookie, 0, sizeof(cookie));
    memset(secure_path, 0, sizeof(secure_path));
    memset(timer, 0, sizeof(timer));
}

bool Auth::checkHeaders(char* headersbuf, char* path)
{
    return checkHeadersHelper(headersbuf, path, false);
}

bool Auth::checkHeadersAdmin(char* headersbuf, char* path)
{
    return checkHeadersHelper(headersbuf, path, true);
}

bool Auth::checkHeadersHelper(char* headersbuf, char* path, bool admin_only)
{
    char* ptr;
    char* end;
    char* cookieheader = AUTH_COOKIE_TAG;
    int i;
    char t;
    time_t current_t;
    int securePathID = 0;

    /* check secure-dir list */
    for(i = 0; i < SECURE_PATH_NUM && secure_path[i][0] != '\0'; i++)
    {
        if(comparePath(secure_path[i], path))
        {
            securePathID = i+1;
        }
    }
    
    ptr = strstr(headersbuf, cookieheader);
    if(!ptr) return false;

    ptr += strlen(cookieheader);

    end = ptr;
    while(strrchr(cookiechars, *end) && (end - ptr) < COOKIE_LEN)
        end++;

    t = *end;
    *end = 0;

    current_t = time(NULL);
    for(i = 0; i < MAX_COOKIES; i++)
    {
        if(strcmp(cookie[i].data, ptr) == 0)
        {
            if(cookie[i].id_secure_path == securePathID || cookie[i].id_secure_path == ADMIN_FAKE_ID)
            {
                /* renew the timeout */
                if((current_t - timer[i]) < AUTH_TIMEOUT_SECS)
                {
                    timer[i] = current_t;
                    *end = t;
                    return true;
                }
            }
        }
    }

    *end = t;

    return false;
}

bool Auth::newAuth(char *destbuf)
{
    return newAuthHelper(destbuf, 0);
}

bool Auth::newAuthHelper(char* destbuf, int secureDirID)
{
    int i, oldest;
    time_t current_t;

    current_t = time(NULL);
    srand(current_t);

    /* find oldest slot, or an empty one */
    oldest = 0;
    for(i = 0; i < MAX_COOKIES; i++)
    {
        if(timer[i] == 0)
        {
            oldest = i;
            break;
        }
        else if((current_t - timer[i]) > 
                (current_t - timer[oldest]))
        {
            oldest = i;
        }
    }
        
    timer[oldest] = current_t;
    for(i = 0; i < COOKIE_LEN-1; i++)
    {
        int r = rand();
        cookie[oldest].data[i] = cookiechars[r%(strlen(cookiechars)-1)];
    }
    cookie[oldest].data[COOKIE_LEN-1] = 0;
    cookie[oldest].id_secure_path = secureDirID;

    strcpy(destbuf, cookie[oldest].data);

    return true;
}

bool Auth::logoutSession(char* headersbuf)
{
    char *ptr, *end;
    int i;

    ptr = strstr(headersbuf, AUTH_COOKIE_TAG);
    if(ptr)
    {
        ptr += strlen(AUTH_COOKIE_TAG);
        end = ptr;
        while(strrchr(cookiechars, *end) && (end - ptr) < COOKIE_LEN)
            end++;
        
        for(i = 0; i < MAX_COOKIES; i++)
        {
            if(strncmp(cookie[i].data, ptr, COOKIE_LEN-1) == 0)
            {
                timer[i] = 0;
                memset(cookie[i].data, 0, sizeof(cookie[i].data));
                return true;
            }
        }
    }

    return false;
}

bool Auth::isSecurePath(char *path)
{
    int i;
    
    for(i = 0; i < SECURE_PATH_NUM && secure_path[i][0] != '\0'; i++)
    {
        if(comparePath(secure_path[i], path))
        {
            return true;
        }
    }
    return false;
}

bool Auth::addSecurePath(char *path, char *password)
{
    int i = 0;
    while(i < SECURE_PATH_NUM)
    {
        if(secure_path[i][0] == '\0')
        {
            strcpy(secure_path[i], path);
            strcpy(secure_path_passwd[i], password);
            return true;
        }
        i++;
    }
    return false;
}

bool Auth::authSecurePath(char *path, char *password, char *cookiedest)
{
    int i;

    /* find if this path is a suffix of any secure-paths */
    for(i = 0; i < SECURE_PATH_NUM; i++)
    {
        if(comparePath(secure_path[i], path))
        {
            if(strcmp(secure_path_passwd[i], password) == 0)
            {
                newAuthHelper(cookiedest, i+1);
                return true;
            }
        }
    }
    return false;
}
