#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using std::cout;
using std::endl;

#ifdef _BWINDOWS_
#include "windowsdefs.h"
#include <winsock2.h>
#endif

#ifdef _UNIX_
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#endif

#include "simplesocket.h"


SimpleSocket::SimpleSocket()
{
#ifdef _BWINDOWS_
    WSADATA startupData;
    WSAStartup(WINSOCK_VERS, &startupData);
#endif /* _BWINDOWS_ */

    listensock = remotesock = -1;

    isready = false;
}

SimpleSocket::~SimpleSocket()
{
    close();
}

SimpleSocket::SimpleSocket(short localport, char* localip, int nbacklog)
{
    int err;

    backlog = nbacklog;

    listensock = remotesock = -1;

#ifdef _BWINDOWS_
    WSADATA startupData;
    WSAStartup(WINSOCK_VERS, &startupData);
#endif /* _BWINDOWS_ */

    isready = false;

    listenport = localport;
    listensock = socket(AF_INET, SOCK_STREAM, 0);

    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(localport);
    
    if(localip == NULL)
        local.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        local.sin_addr.s_addr = htonl(inet_addr(localip));

    err = bind(listensock, (struct sockaddr*) &local, sizeof(local));
    if(err < 0)
    {
      cout << "couldn't bind(), got errno " << ERROR_CODE << endl;
    }

    isready = true;
}
    
int SimpleSocket::doConnect(char* host, short port)
{
    int err;
    struct hostent* mhost;


    remotesock = socket(AF_INET, SOCK_STREAM, 0);

    mhost = gethostbyname(host);
    if(mhost == NULL)
    {
        cout << "couldn't gethostbyname(), got " << ERROR_CODE << endl;
    }

    remote.sin_family = AF_INET;
    remote.sin_port = htons(port);
    memcpy(&remote.sin_addr.s_addr, mhost->h_addr_list[0], sizeof(in_addr));
    
    err = connect(remotesock, (struct sockaddr*) &remote, sizeof(remote));

    if(err == 0)
        isready = true;

    return err;
}

int SimpleSocket::doConnectWithTimeout(char* host, short port, int wait_secs)
{
    int err;
    struct hostent* mhost;
    unsigned long nonblock = 1;


    remotesock = socket(AF_INET, SOCK_STREAM, 0);

    mhost = gethostbyname(host);
    if(mhost == NULL)
    {
        cout << "couldn't gethostbyname(), got " << ERROR_CODE << endl;
        return -1;
    }

    remote.sin_family = AF_INET;
    remote.sin_port = htons(port);
    memcpy(&remote.sin_addr.s_addr, mhost->h_addr_list[0], sizeof(in_addr));

    //set non-blocking
    IOCTL(remotesock, FIONBIO, &nonblock);
    
    err = connect(remotesock, (struct sockaddr*) &remote, sizeof(remote));

    if(err == 0)
    {
        isready = true;
        return err;
    }
    else if(err == SOCKET_ERROR)
    {
        err = getError();

#ifdef _BWINDOWS_
        if(err == WSAEWOULDBLOCK)
#else
        if(err == EAGAIN)
#endif
        {
            fd_set write, error;
            TIMEVAL timeout;

            FD_ZERO(&error);
            FD_ZERO(&write);
            FD_SET(remotesock, &error);
            FD_SET(remotesock, &write);
            
            timeout.tv_usec = 0;
            timeout.tv_sec = wait_secs;

            int v = select(0, NULL, &write, &error, &timeout);

            if(v == 0)
                return -1;    //connect timeout
            else
            {
                if(FD_ISSET(remotesock, &write))
                {
                    nonblock = 0;
                    IOCTL(remotesock, FIONBIO, &nonblock);
                    isready = true;
                    return 0;
                }
            }
        }
    }

    nonblock = 0;
    IOCTL(remotesock, FIONBIO, &nonblock);
    return -1;
}

SimpleSocket* SimpleSocket::accept()
{
    int err, addrlen = sizeof(sockaddr_in);

    SimpleSocket* newsock = new SimpleSocket();

    if(!newsock) return NULL;

    err = listen(listensock, backlog);
    if(err < 0)
    {
        cout << "couldn't listen(), got " << ERROR_CODE << endl;
        return NULL;
    }

    newsock->remotesock = ::accept(listensock, (struct sockaddr*) &remote, (socklen_t*) &addrlen);
    if(newsock->remotesock > 0)
    {
        memcpy(&newsock->remote, &remote, sizeof(remote));
        newsock->isready = true;
    }
    else return NULL;

    return newsock;    
}

/* expects dest to be 255 bytes! */
static
void remoteSockToStr(struct sockaddr_in *addr, char *dest)
{
    memset(dest, 0, 255);
    sprintf(dest, "%s:%d",
            inet_ntoa(addr->sin_addr),
            ntohs(addr->sin_port));
}

int SimpleSocket::waitConnect()
{
    int err, addrlen = sizeof(sockaddr_in);

    err = listen(listensock, backlog);
    if(err < 0)
    {
        cout << "couldn't listen(), got " << ERROR_CODE << endl;
    }

    remotesock = ::accept(listensock, (struct sockaddr*) &remote, (socklen_t*) &addrlen);
    if(remotesock > 0)
        isready = true;

    return err;
}

int SimpleSocket::ready(int waitsec, int waitusec)
{
    fd_set recvset, exceptset;
    struct timeval timeout;

    FD_ZERO(&recvset);
    FD_SET(remotesock, &recvset);
    FD_ZERO(&exceptset);
    FD_SET(remotesock, &exceptset);

    timeout.tv_sec = waitsec;
    timeout.tv_usec = waitusec;

    int nsocks = select(remotesock+1, &recvset, NULL, &exceptset, &timeout);
    if(isready && (nsocks > 0))
    {
        if(FD_ISSET(remotesock, &exceptset))
            return -1;

        else
            return 1;
    }

    else
        return 0;
}

int SimpleSocket::sSend(char* buffer, int len)
{
    int flags = 0;
#ifdef _UNIX_
    flags |= /*MSG_NOSIGNAL*/ 0;
#endif
    return send(remotesock, buffer, len, flags); 
}

int SimpleSocket::sRecv(char *buffer, int maxlen)
{
    int flags = 0;
#ifdef _UNIX_
    flags |= /*MSG_NOSIGNAL*/ 0;
#endif

    int cnt = recv(remotesock, buffer, maxlen, flags);

    return cnt;
}

SOCKET SimpleSocket::getRemoteSocket()
{
    return remotesock;
}

char * SimpleSocket::getRemoteHostStr()
{
    remoteSockToStr(&remote, remoteHostStr);
    return remoteHostStr;
}

void SimpleSocket::close()
{
    if(remotesock != -1)
        ::CLOSE(remotesock);
    if(listensock != -1)
        ::CLOSE(listensock);
}

void SimpleSocket::closeRemote()
{
    if(remotesock != -1)
        ::CLOSE(remotesock);
}

int SimpleSocket::getError()
{
#ifdef _BWINDOWS_
    return WSAGetLastError();
#endif
#ifdef _UNIX_
    return errno;
#endif
}
