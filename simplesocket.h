
//include files
#include <memory.h>
#include <stdlib.h>

#ifdef _BWINDOWS_
#include "windowsdefs.h"
#include <winsock2.h>

#define IOCTL ioctlsocket

#endif

#ifdef _UNIX_
#include "unixdefs.h"
#include <sys/socket.h>
#include <netinet/in.h>

#define SOCKET_ERROR -1

#define IOCTL ioctl

#define TIMEVAL struct timeval

#endif    /* _UNIX_ */

#ifndef _simplesocket
#define _simplesocket


class SimpleSocket
{
public:

    SimpleSocket();

    ~SimpleSocket();

    SimpleSocket(short localport, char* localip);

    SimpleSocket(short localport, char* localip, int backlog = 1);

    int doConnect(char* host, short port);

    int doConnectWithTimeout(char* host, short port, int wait_secs);

    SimpleSocket* accept();    /* wait for an incoming connection attempt, and create a new socket */

    int waitConnect();

    int sSend(char* buffer, int len);

    int sRecv(char* buffer, int maxlen);

    int ready(int waitsec, int waitusec);

    SOCKET getRemoteSocket();

    // get the IP/port of the host we are connected with
    char *getRemoteHostStr();

    void close();                                    //closes everything

    void closeRemote();                                //closes down current (remote) connection to listening socket

    int getError();

private:
    short listenport;
    SOCKET listensock, remotesock;
    struct sockaddr_in remote, local;
    bool isready;
    int backlog;
    char remoteHostStr[255];
};

#endif
