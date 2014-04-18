#ifndef __WINDOWSDEFS__
#define __WINDOWSDEFS__

#define socklen_t int
#define CLOSE closesocket
#define ERROR_CODE WSAGetLastError()
#define WINSOCK_VERS 0x202

#endif