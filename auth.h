#ifndef __AUTH__
#define __AUTH__

#include <time.h>

/* an authorization API - user logs in and if correct password is sent, he's given a cookie
 * this cookie is stored for up to 60 minutes 
 */

#define MAX_COOKIES 5
#define COOKIE_LEN 64
#define AUTH_TIMEOUT_SECS (60 * 60)
#define AUTH_COOKIE_TAG "authcookie="
#define SECURE_PATH_LEN 2048
#define SECURE_PATH_NUM 128

typedef struct
{
    char data[COOKIE_LEN];
    int id_secure_path;
} cookie_t;

class Auth {
private:
    cookie_t cookie[MAX_COOKIES];
    char secure_path[SECURE_PATH_NUM][SECURE_PATH_LEN];
    char secure_path_passwd[SECURE_PATH_NUM][254];
    time_t timer[MAX_COOKIES];

    bool newAuthHelper(char* cookiedest, int secureDirID);
    bool checkHeadersHelper(char* headersbuf, char* path, bool admin_only);

public:
    /* constructor - set auth table to NULL */
    Auth();

    /* search for the Cookie: abcdefg\r\n header and compare with auth table */
    bool checkHeaders(char* headersbuf, char *path);

    /* like checkHeaders, but only TRUE if securedirID == 0 */
    bool checkHeadersAdmin(char *headersbuf, char *path);
    
    /* create a new entry in the auth table, and copy the new cookie back to cookiedest */
    bool newAuth(char* cookiedest);

    /* find cookie in auth list and remove it */
    bool logoutSession(char* headersbuf);

    /* add a secure-path with this password */
    bool addSecurePath(char *path, char *password);

    /* is this path secure? */
    bool isSecurePath(char *path);

    /* compare path to list of secure-paths, if a match is found compare password
     * and copy a cookie back */
    bool authSecurePath(char *path, char *password, char *cookiedest);
};


#endif /* __AUTH__ */
