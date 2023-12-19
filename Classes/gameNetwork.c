//
//  gameNetwork.c
//  gl_flight
//
//  Created by jbrady on 11/21/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#ifndef _NOT_POSIX
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#define ADDRINFO struct addrinfo
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
//#include <ws2ipdef.h>
//#include <socketapi.h>
//#include <WinInet.h>
//#include <in6addr.h>

#define close closesocket
#endif



//#ifdef __cplusplus
//extern "C" {
//#endif
    
#include "gameNetwork.h"
#include "gameNetworkByteOrder.h"
#include "gameMotionInterp.h"
#include "world.h"
#include "gameUtils.h"
#include "gameGlobals.h"
#include "gameAudio.h"
#include "world_file.h"
#include "action.h"
#include "textures.h"
#include "gamePlay.h"
#include "gameAI.h"
#include "glFlight.h"
#include "gameIncludes.h"
#include "gameLock.h"
#include "collision.h"
#include "sounds.h"

#define REMAINING_TIME_STR "remaining time: "

gameNetworkState_t gameNetworkState;

unsigned long update_frequency_ms = 1000 / (GAME_TICK_RATE / 2);
static unsigned long update_time_last = 0;
const unsigned long GAME_NETWORK_TIMEOUT_MS = (30*1000);
const static char *map_eom = "\nmap_eom\n";

const long GAME_NETWORK_PORT_DEFAULT = 52000;
unsigned long GAME_NETWORK_PORT = 52000;

static char *gameKillVerbs[] = {"slaughtered", "evicerated", "de-rezzed", "shot-down", "ended", "terminated"};

extern int GameNetworkBonjourManagerHost(const char* name, int* sock_out);
extern int GameNetworkBonjourManagerBrowseBegin(void);
extern int GameNetworkBonjourManagerBrowseEnd(gameNetworkAddress* address_returned);
extern int GameNetworkBonjourManagerDisconnect(void);
extern int GameNetworkBonjourManagerDisconnectPeer(int peer_id);
void GameNetworkBonjourManagerSendMessageToPeer(uint8_t* msg_, int peer_id);
extern int gameNetwork_onBonjourConnecting1(gameNetworkMessage*, gameNetworkAddress*);
extern int gameNetwork_onDirectoryRegister1(gameNetworkMessage*, gameNetworkAddress*);
int gameNetwork_onBonjourConnecting3(gameNetworkMessage* msg, gameNetworkAddress* srcAddr);
extern float get_time_ms_wall(void);

static char* do_game_map_render(void);
void send_bonjour_beacon_callback(void);


////////////////////////////////

static int
socket_read_ready(int sock, unsigned int timeout_ms)
{
    fd_set r;
    struct timeval tv;
    int retries = 5;
    
    while(retries > 0)
    {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        
        FD_ZERO(&r);
        FD_SET(sock, &r);
        
        if(select(sock+1, &r, NULL, NULL, &tv) <= 0)
        {
            return 0;
        }
        else
        {
            if(!FD_ISSET(sock, &r))
            {
                // some other socket may have caused us to return, retry
                retries--;
                continue;
            }
        }
        break;
    }
    
    return 1;
}

static int
prepare_listen_socket(int stream, unsigned int port, unsigned int do_bind)
{
    int sock;
    struct sockaddr_in6 addr;
    struct sockaddr_in6* addr6 = (void*)& addr;
    int proto = 0;
    ADDRINFO hints, * addrResult = NULL;
    char portnum[255];

    memset(&addr, 0, sizeof(addr));
    addr6->sin6_port = htons(port);
    addr6->sin6_family = AF_INET6;

    if( ! inet_pton(AF_INET6, "::", &(addr6->sin6_addr)) ) {
        return GAME_NETWORK_ERR_FAIL;
    }

    sock = socket(AF_INET6, (stream ? SOCK_STREAM : SOCK_DGRAM), proto);
    if (sock < 0)
    {
        console_write("%s:%d %s\n", __func__, __LINE__, "socket failure");
        return -1;
    }

//    memset(&hints, 0, sizeof(hints));
//    hints.ai_family = AF_INET6;
//    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_V4MAPPED;
//    hints.ai_socktype = SOCK_DGRAM;
//    hints.ai_addrlen = sizeof(struct sockaddr_in6);
//    hints.ai_protocol = 0;
//
//    getaddrinfo("0.0.0.0", "52000/udp", &hints, &addrResult);
//    if(!addrResult)
//    {
//        console_write("%s:%d %s (%s)\n", __func__, __LINE__, "getaddrinfo failure", strerror(errno));
//        return -1;
//    }
//
//    memcpy(addr6, addrResult->ai_addr, addrResult->ai_addrlen);

#ifdef BSD_SOCKETS
    addr6->sin6_len = sizeof(struct sockaddr_in6);
#endif

    int bcast_enabled = 1;
    int so_arg = 1;
    int backlog = 10;

    if (!stream)
    {
        setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &bcast_enabled, sizeof(bcast_enabled));
    }
    else
    {
        /*
        int blocking = 0;
        setsockopt(sock, sol_socket, o_nonblock, &blocking, sizeof(blocking));
        */
    }

#ifdef BSD_SOCKETS
    setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &so_arg, sizeof(so_arg));
#endif

    //setsockopt(sock, sol_socket, so_reuseaddr, &so_arg, sizeof(so_arg));

//    so_arg = 1;
//    setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &so_arg, sizeof(so_arg));
    
    /* bind */
    if(do_bind)
    if(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
        console_write("%s:%d %s\n", __func__, __LINE__, "socket failure (bind)");
        close(sock);
        gameDialogError("bind failed (kill/restart app)");
        return -1;
    }
    
    if(do_bind && stream)
    {
        if(listen(sock, backlog))
        {
            console_write("%s:%d %s\n", __func__, __LINE__, "socket failure (listen)");
            // listen failed
            close(sock);
            gameDialogError("listen() failed (kill/restart app)");
            return -1;
        }
    }
    
    return sock;
}

static void
send_lan_broadcast(gameNetworkMessage* msg)
{
    struct sockaddr_in6 sa_bc6;
    long r;
    
    memset(&sa_bc6, 0, sizeof(sa_bc6));
    
    // resend with ipv6 link-local
    
    sa_bc6.sin6_family = AF_INET6;
#ifdef BSD_SOCKETS
    sa_bc6.sin6_len = sizeof(sa_bc6);
#endif
    sa_bc6.sin6_port = htons(gameNetworkState.hostInfo.port);
#ifdef BSD_SOCKETS
    sa_bc6.sin6_addr = in6addr_linklocal_allnodes;
#endif
    
    r = sendto(gameNetworkState.hostInfo.socket.s, msg, sizeof(*msg),
               0, (struct sockaddr*) &sa_bc6, sizeof(sa_bc6));
}

static void
 send_to_address_udp(gameNetworkMessage* msg, gameNetworkAddress* address)
{
    struct sockaddr_storage* sa;
    int r;

    sa = address->storage;

    gameMessage_to_nbo(msg);
    
    if(gameNetworkState.hostInfo.bonjour_lan)
    {
        GameNetworkBonjourManagerSendMessageToPeer((void*) msg, ((struct bonjour_addr_stuffed_in_sockaddr_in *) address->storage)->peer_id);
        gameMessage_from_nbo(msg);
        return;
    }
    
    r = sendto(gameNetworkState.hostInfo.socket.s, msg, sizeof(*msg),
               0, (struct sockaddr*) sa, address->len);
    if(r < 0)
    {
        DBPRINTF(("sendto: %ld - errno:%s\n", r, strerror(errno)));
    }

    gameMessage_from_nbo(msg);
}

static void gameNetworkAddress_incrementPort(gameNetworkAddress* address)
{
    struct sockaddr_in6 sa;
    
    memcpy(&sa, address->storage, address->len);
    
    sa.sin6_port = htons(ntohs(sa.sin6_port)+1);
    
    memcpy(address, &sa, address->len);
}
    
static unsigned long strcksum(const char* str)
{
    size_t cksumlen = strlen(str);
    unsigned long cksum = 0, *pcksum = (unsigned long*) str;
    unsigned char *pcksumc;
    while(cksumlen >= sizeof(cksum))
    {
        cksum ^= *pcksum;
        pcksum ++;
        cksumlen -= sizeof(cksum);
    }
    pcksumc = (unsigned char*) pcksum;
    while(cksumlen > 0)
    {
        cksum = cksum << 8;
        cksum ^= *pcksumc;
        pcksumc++;
        cksumlen--;
    }
    return cksum;
}
    
void
send_beacon(gameNetworkAddress *addr, const char* game_name)
{
    gameNetworkMessage msg;
    
    memset(&msg, 0, sizeof(msg));
    msg.cmd = GAME_NETWORK_MSG_BEACON;
    strcpy(msg.params.c, game_name);
    msg.player_id = gameNetworkState.my_player_id;
    
    send_to_address_udp(&msg, addr);
}
    
void
send_connect(gameNetworkAddress *addr)
{
    gameNetworkMessage msg;
    
    memset(&msg, 0, sizeof(msg));
    msg.cmd = GAME_NETWORK_MSG_CONNECT;
    strncpy(msg.params.c, gameNetworkState.my_player_name, sizeof(msg.params.c));
    msg.player_id = gameNetworkState.my_player_id;
    
    send_to_address_udp(&msg, addr);
}
    
void
send_startgame()
{
    gameNetworkMessage msg;
    
    memset(&msg, 0, sizeof(msg));
    msg.cmd = GAME_NETWORK_MSG_STARTGAME;
    msg.params.f[0] = game_time_remaining();
    msg.player_id = gameNetworkState.my_player_id;
    
    gameNetwork_send(&msg);
}
    
void
send_endgame()
{
    gameNetworkMessage msg;
    gameNetworkAddress fakeAddress;
    
    memset(&msg, 0, sizeof(msg));
    msg.cmd = GAME_NETWORK_MSG_ENDGAME;
    strncpy(msg.params.c, gameNetworkState.my_player_name, sizeof(msg.params.c));
    msg.player_id = gameNetworkState.my_player_id;
    
    gameNetwork_send(&msg);
    
    do_game_network_handle_msg(&msg, &fakeAddress, get_time_ms_wall());
}


static int
gameNetwork_getDNSAddress(char *name, gameNetworkAddress* addr)
{
    int ret = GAME_NETWORK_ERR_NONE;
    char buf[128], portnum[255];
    
    if(!strchr(name, '.')) return GAME_NETWORK_ERR_FAIL;
    ADDRINFO *addrInfoResultp = NULL, hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET6;

    sprintf(portnum, "%u", GAME_NETWORK_PORT_DEFAULT);

    addrInfoResultp = NULL;
    
#ifdef BSD_SOCKETS
    int aret =    getaddrinfo(name, portnum, &hints, &addrInfoResultp);
    if(aret != 0) {
        DBPRINTF(("getaddrinfo failed: %s\n", gai_strerror(aret)));
        return GAME_NETWORK_ERR_FAIL;
    }
#else
    int aret =    getaddrinfo(name, portnum, NULL, &addrInfoResultp);
    if(aret != 0) {
        DBPRINTF(("getaddrinfo failed: %s\n", gai_strerror(aret)));
        return GAME_NETWORK_ERR_FAIL;
    }
#endif

    if(addrInfoResultp->ai_family == AF_INET)
    {
        // convert to ipv4
        struct sockaddr_in* sa = addrInfoResultp->ai_addr;
        sa->sin_port = htons(gameNetworkState.hostInfo.port);
        inet_ntop(AF_INET, &sa->sin_addr, buf, sizeof(buf));
        console_write("server DNS resolved: %s (ipv4)\n", buf);
    }
    else
    if(addrInfoResultp->ai_family == AF_INET6)
    {
        // convert to ipv4
        struct sockaddr_in6 *sa = addrInfoResultp->ai_addr;
        sa->sin6_port = htons(gameNetworkState.hostInfo.port);
        inet_ntop(AF_INET6, &sa->sin6_addr, buf, sizeof(buf));
        console_write("server DNS resolved: %s (ipv6)\n", buf);
    } else {
        return GAME_NETWORK_ERR_FAIL;
    }

    memcpy(addr->storage, addrInfoResultp->ai_addr, addrInfoResultp->ai_addrlen);
    addr->len = addrInfoResultp->ai_addrlen;

    freeaddrinfo(addrInfoResultp);
    return ret;
}

static void
gameNetwork_initsockets()
{
    //if(gameNetworkState.hostInfo.socket.s != -1) close_socket(gameNetworkState.hostInfo.socket.s);
    if(gameNetworkState.hostInfo.socket.s == -1)
    gameNetworkState.hostInfo.socket.s = prepare_listen_socket(0, gameNetworkState.hostInfo.port, 1);
    
    //if(gameNetworkState.hostInfo.map_socket.s != -1) close_socket(gameNetworkState.hostInfo.map_socket.s);
    if(gameNetworkState.hostInfo.map_socket.s == -1)
    gameNetworkState.hostInfo.map_socket.s = prepare_listen_socket(1, gameNetworkState.hostInfo.port+1, gameNetworkState.hostInfo.hosting);

    gameNetworkState.hostInfo.map_socket_sending.s = -1;

    //if(gameNetworkState.hostInfo.stream_socket.s != -1) close_socket(gameNetworkState.hostInfo.stream_socket.s);
    if(gameNetworkState.hostInfo.stream_socket.s == -1)
    gameNetworkState.hostInfo.stream_socket.s = prepare_listen_socket(1, gameNetworkState.hostInfo.port+2, gameNetworkState.hostInfo.hosting);

    if (gameNetworkState.hostInfo.socket.s < 0 || gameNetworkState.hostInfo.map_socket.s < 0 || gameNetworkState.hostInfo.stream_socket.s < 0)
    {
        console_append("incrementing GAME_NETWORK_PORT");
        GAME_NETWORK_PORT += 1;
    }
}

gameNetworkError
gameNetwork_test()
{
    gameNetworkAddress addr;
    gameNetwork_getDNSAddress("d0gf1ght.domain17.net", &addr);

    return addr.len >= sizeof(struct sockaddr_in);
}

gameNetworkError
gameNetwork_init(int broadcast_mode, const char* server_name,
                 const char* player_name, int update_ms,
                 unsigned short host_port,
                 const char* local_inet_addr)
{
    const char *directory_name = GAME_NETWORK_DIRECTORY_HOSTNAME_DEFAULT;
    
    if(gameNetworkState.inited)
    {
        gameNetwork_disconnectSignal();
        //assert(!gameNetworkState.connected);
    }
    
    update_frequency_ms = update_ms;
    
    memset(&gameNetworkState, 0, sizeof(gameNetworkState));
    
    strcpy(gameNetworkState.hostInfo.name, server_name);
    gameNetworkState.hostInfo.port = host_port;
    
    gameNetworkState.hostInfo.hosting = 0;
    gameNetworkState.broadcast_mode = broadcast_mode;
    gameNetworkState.my_player_id = rand_in_range(1, GAME_NETWORK_PLAYER_ID_HOST-1);
    strcpy(gameNetworkState.my_player_name, player_name);
    strcpy(gameNetworkState.gameDirectory.directory_name, directory_name);
 
    gameNetworkState.game_object_id_next = GAME_NETWORK_OBJECT_ID_MIN;

    GAME_NETWORK_ADDRESS_INADDR(&gameNetworkState.hostInfo.local_inet_addr) = in6addr_any;

    if(strlen(local_inet_addr) > 0 && inet_addr(local_inet_addr) != INADDR_NONE && inet_addr(local_inet_addr) != INADDR_ANY)
    {
        inet_pton(AF_INET6, local_inet_addr, &(GAME_NETWORK_ADDRESS_INADDR(&gameNetworkState.hostInfo.local_inet_addr)));
    }
        
    gameNetworkState.hostInfo.socket.s = -1;
    gameNetworkState.hostInfo.map_socket.s = -1;
    gameNetworkState.hostInfo.stream_socket.s = -1;
    gameNetworkState.hostInfo.map_socket_sending.s = -1;
    
    gameNetworkState.inited = 1;
    
    gameNetworkState.msg_seq_acked_last = 0;
    gameNetworkState.msg_seq_next = rand();
    
    game_lock_init(&gameNetworkState.msgQueue.lock);

    if(!gameNetwork_test()) assert(0);
    
    return GAME_NETWORK_ERR_NONE;
}

gameNetworkError
gameNetwork_resume()
{
    gameNetwork_initsockets();
    return GAME_NETWORK_ERR_NONE;
}

int
gameNetwork_connect_core(int hosting, char* server_name, void (*callback_becamehost)(void))
{
    int retries = 0;
    gameNetworkAddress gameAddress;
    struct sockaddr_in6* addr6 = (struct sockaddr_in6*) gameAddress.storage;
    struct sockaddr* sockaddr_p = (struct sockaddr*) gameAddress.storage;
    gameNetworkPlayerInfo* playerInfo = NULL;
    int host_player_id = GAME_NETWORK_PLAYER_ID_HOST;
    int directory_search_retries = 1;
    unsigned short server_port_resolved = gameNetworkState.hostInfo.port;
    int retries_udp = 1;
    int server_ports = 10;
    char strtmp[256];
    
    gameAddress.len = 0;

    memset(&gameAddress, 0, sizeof(gameAddress));
    
    gameNetwork_disconnectSignal();
    
    game_lock_lock(&gameNetworkState.msgQueue.lock);
    /*
    while(gameNetworkState.player_list_head.next_connected != NULL)
    {
        gameNetwork_removePlayer(gameNetworkState.player_list_head.next->player_id);
    }
     */
    
    assert(sizeof(gameAddress.storage) >= sizeof(struct sockaddr_storage));
    
    gameNetwork_initsockets();
    
    if(hosting && !gameNetworkState.hostInfo.bonjour_lan)
    {
        strcpy(gameNetworkState.hostInfo.name, "0.0.0.0");
        directory_search_retries = 0;
        gameNetworkState.hostInfo.hosting = 1;
        
        gameNetworkState.my_player_id = GAME_NETWORK_PLAYER_ID_HOST;
        if(server_name != gameNetworkState.hostInfo.name)
        {
            strcpy(gameNetworkState.hostInfo.name, server_name);
        }
        
        // add self to player list
        gameNetwork_getPlayerInfo(gameNetworkState.my_player_id, &playerInfo, 1);
        strcpy(playerInfo->name, gameNetworkState.my_player_name);
        
        callback_becamehost();

        goto gameNetwork_connect_done;
    }
    
    // if its an IP, treat as ip address
    sprintf(strtmp, "::FFFF:%s", server_name);
    if(inet_pton(AF_INET6, server_name, &(addr6->sin6_addr)) == 1)
    {
        gameAddress.len = sizeof(*addr6);
        
        addr6->sin6_port = htons(server_port_resolved);
        addr6->sin6_family = AF_INET6;

#ifdef BSD_SOCKETS
        addr6->sin6_len = gameAddress.len;
#endif
        gameNetworkState.hostInfo.hosting = 0;
    }
    else if(!gameNetworkState.hostInfo.bonjour_lan)
    {
        // try directory
        memset(&gameNetworkState.gameDirectory.directory_address, 0, sizeof(gameNetworkState.gameDirectory.directory_address));

        if(gameNetwork_getDNSAddress(gameNetworkState.gameDirectory.directory_name,
                                     &gameNetworkState.gameDirectory.directory_address) == GAME_NETWORK_ERR_NONE)
        {
            gameAddress = gameNetworkState.gameDirectory.directory_address;
        }
        else
        {
            if(!gameNetworkState.hostInfo.bonjour_lan)
            {
                console_write("\ninvalid ipv4 address/hostname errno: (%s)", strerror(errno));
                game_lock_unlock(&gameNetworkState.msgQueue.lock);
                return GAME_NETWORK_ERR_FAIL;
            }
        }
    }

    if(gameNetworkState.hostInfo.hosting)
    {
        // registering with directory happens after gameNetwork_connect
    }
    else
    {
        // moving to gameNetwork_init
        if(gameNetworkState.my_player_id == 0 || gameNetworkState.my_player_id == GAME_NETWORK_PLAYER_ID_HOST)
        { 
            gameNetworkState.my_player_id = rand_in_range(1, GAME_NETWORK_PLAYER_ID_HOST-1);
        }
        
        // attempt to find via apple "bonjour" (bluetooth + wifi)
        if(gameNetworkState.hostInfo.bonjour_lan)
        {
            memset(&gameAddress, 0, sizeof(gameAddress));
            gameAddress.len = sizeof(struct sockaddr_in6);
            
            if(GameNetworkBonjourManagerBrowseEnd(&gameAddress) != 0)
            {
                if(sockaddr_p->sa_family != GAME_NETWORK_BONJOUR_ADDRFAMILY_HACK)
                {
                    game_lock_unlock(&gameNetworkState.msgQueue.lock);
                    return GAME_NETWORK_ERR_FAIL;
                }
                
                gameNetworkState.gameNetworkHookOnMessage = gameNetwork_onBonjourConnecting1;

                send_beacon(&gameAddress, server_name);
                goto gameNetwork_connect_done;
            }
            else
            {
                console_write("\nNo local games found named \"%s\" creating...\n", server_name);
                
                gameNetworkState.hostInfo.hosting = 1;
                
                // add self to player list
                gameNetworkState.my_player_id = GAME_NETWORK_PLAYER_ID_HOST;
                gameNetwork_getPlayerInfo(gameNetworkState.my_player_id, &playerInfo, 1);
                strcpy(playerInfo->name, gameNetworkState.my_player_name);
                GameNetworkBonjourManagerHost(server_name, &gameNetworkState.hostInfo.socket.s);
                
                callback_becamehost();
                
                goto gameNetwork_connect_done;
            }
        }

        if(gameAddress.len >= sizeof(struct sockaddr_in))
        {
            if(gameNetwork_getPlayerInfo(host_player_id, &playerInfo, 1) == GAME_NETWORK_ERR_NONE)
            {
                strcpy(playerInfo->name, "host");
                
                if(playerInfo->address.len == 0)
                {
                    memcpy(&playerInfo->address, &gameAddress, sizeof(gameAddress));
                }
            
                retries = retries_udp;
                while(retries > 0)
                {
                    strcpy(gameNetworkState.hostInfo.name, server_name);
                    
                    while(server_ports > 0)
                    {
                        send_beacon(&gameAddress, server_name);
                        gameNetworkAddress_incrementPort(&gameAddress);
                        server_ports--;
                    }
                    
                    //send_connect(&gameAddress);
                    
                    console_write("searching for game...");
                    
                    // TODO: time out and remove callback
                    gameNetworkState.gameNetworkHookOnMessage = gameNetwork_onBonjourConnecting1;
                    
                    retries--;
                }
            }

        } else {
            DBPRINTF(("illegal gameAddress.len during game network connect\n"));
        }
    }
    
gameNetwork_connect_done:
    gameNetworkState.connected_signal = 1;
    game_lock_unlock(&gameNetworkState.msgQueue.lock);
    while(gameNetworkState.connected_signal)
    {
        usleep(GAME_NETWORK_READ_THREAD_IDLE_USLEEP_INTERVAL);
    }
    return GAME_NETWORK_ERR_NONE;
}

int
gameNetwork_host(char* server_name, void (*callback_becamehost)(void))
{
    return gameNetwork_connect_core(1, server_name, callback_becamehost);
}

int
gameNetwork_connect(char* server_name, void (*callback_becamehost)(void))
{
    return gameNetwork_connect_core(0, server_name, callback_becamehost);
}

int
gameNetwork_directoryRegister(const char* roomName)
{
    // register with directory
    if(gameNetworkState.hostInfo.hosting &&
       gameNetwork_getDNSAddress(gameNetworkState.gameDirectory.directory_name,
                                 &gameNetworkState.gameDirectory.directory_address) == GAME_NETWORK_ERR_NONE)
    {
        strcpy(gameNetworkState.hostInfo.name, roomName);
        send_beacon(&gameNetworkState.gameDirectory.directory_address, roomName);
        
        gameNetworkState.gameNetworkHookOnMessage = gameNetwork_onDirectoryRegister1;
        return GAME_NETWORK_ERR_NONE;
    }
    return GAME_NETWORK_ERR_FAIL;
}

int
gameNetwork_eagerConnectInit()
{
    if(gameNetwork_getDNSAddress(gameNetworkState.gameDirectory.directory_name,
                                 &gameNetworkState.gameDirectory.directory_address) == GAME_NETWORK_ERR_NONE)
    {
        return GAME_NETWORK_ERR_NONE;
    }
    
    return GAME_NETWORK_ERR_FAIL;
}

void
gameNetwork_worldInit()
{
    gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next_connected;
    
    while(pInfo)
    {
        pInfo->elem_id = WORLD_ELEM_ID_INVALID;
        
        pInfo = pInfo->next_connected;
    }
}
    
void
gameNetwork_sendPlayersDisconnect()
{
    gameNetworkMessage msg;
    
    memset(&msg, 0, sizeof(msg));
    
    msg.cmd = GAME_NETWORK_MSG_DISCONNECT;
    gameNetwork_send(&msg);
}

void
gameNetwork_disconnectSignal()
{
    if(!gameNetworkState.connected) return;
        
    gameNetworkState.connected_signal = 1;
    int waiting = 1;
    
    do
    {
        usleep(GAME_NETWORK_READ_THREAD_IDLE_USLEEP_INTERVAL);
        
        game_lock_lock(&gameNetworkState.msgQueue.lock);
        if(!gameNetworkState.connected_signal) waiting = 0;
        game_lock_unlock(&gameNetworkState.msgQueue.lock);
    } while (waiting);
}

void
gameNetwork_disconnect()
{
    if(gameNetworkState.connected)
    {
        gameNetwork_sendPlayersDisconnect();
        
        while(gameNetworkState.player_list_head.next != NULL)
        {
            gameNetworkPlayerInfo* pFree = gameNetworkState.player_list_head.next;
            gameNetworkState.player_list_head.next = gameNetworkState.player_list_head.next->next;
            gameNetworkState.player_list_head.next_connected = gameNetworkState.player_list_head.next;
            free(pFree);
        }
    }

    int* sock_set[] = {&gameNetworkState.hostInfo.socket.s,
        &gameNetworkState.hostInfo.map_socket.s,
        &gameNetworkState.hostInfo.map_socket_sending.s,
        &gameNetworkState.hostInfo.stream_socket.s};
    int s = 0;
    for(s = 0; s < sizeof(sock_set)/sizeof(int*); s++)
    {
        if(*(sock_set[s]) != -1)
        {
            int so_arg = 0;
            setsockopt(*(sock_set[s]), SOL_SOCKET, SO_LINGER, &so_arg, sizeof(so_arg));

            if (*(sock_set[s]) > 0)
            {
                close(*(sock_set[s]));
                *(sock_set[s]) = -1;
            }
        }
    }
    
    GameNetworkBonjourManagerDisconnect();
    gameNetworkState.hostInfo.bonjour_lan = 0;
    gameNetworkState.hostInfo.hosting = 0;
    gameNetworkState.map_downloaded = 0;
    
    gameNetworkState.gameNetworkHookOnMessage = NULL;
    gameNetworkState.gameNetworkHookGameDiscovered = NULL;
    
    // synchronize with network background-thread
    game_lock_lock(&gameNetworkState.msgQueue.lock);
    
    while(gameNetworkState.msgQueue.head.next)
    {
        struct gameNetworkMessageQueued* pFree = gameNetworkState.msgQueue.head.next;
        
        gameNetworkState.msgQueue.head.next = gameNetworkState.msgQueue.head.next->next;
        free(pFree);
    }
    
    gameNetworkState.connected = 0;
    
    game_lock_unlock(&gameNetworkState.msgQueue.lock);
}

int
gameNetwork_countPlayers()
{
    gameNetworkPlayerInfo* p = gameNetworkState.player_list_head.next_connected;
    int count = 0;
    
    while(p)
    {
        count++;
        p = p->next_connected;
    }
    return count;
}

void
gameNetwork_startGame(unsigned int sec)
{
    gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next_connected;
    
    while(pInfo)
    {
        memset(&pInfo->stats, 0, sizeof(pInfo->stats));
        
        pInfo = pInfo->next_connected;
    }
    
    send_startgame();
    
    world_remove_object(my_ship_id);
    my_ship_id = WORLD_ELEM_ID_INVALID;
}
    
gameNetworkError
gameNetwork_send_player_core(gameNetworkMessage* msg, gameNetworkPlayerInfo* playerInfoTarget)
{
    long s = 1;
    int needs_stream;
    
    needs_stream = msg->needs_stream;
    
    gameNetworkPlayerInfo* playerInfo = gameNetworkState.player_list_head.next_connected;
    while(playerInfo)
    {
        if(playerInfoTarget == playerInfo ||
           (playerInfoTarget == NULL && playerInfo->player_id != gameNetworkState.my_player_id))
        {
            if(playerInfo->address.len > 0)
            if(playerInfo->player_id != msg->player_id)
            {
                if(needs_stream)
                {
                    long r = send(playerInfo->stream_socket.s, msg, sizeof(*msg), 0);
                    
                    if(r <= 0)
                    {
                        // do nothing for now
                    }
                }
                else
                {
                    if(gameNetworkState.hostInfo.bonjour_lan)
                    {
                        GameNetworkBonjourManagerSendMessageToPeer((uint8_t*)msg, ((struct bonjour_addr_stuffed_in_sockaddr_in *) playerInfo->address.storage)->peer_id);
                        s = sizeof(*msg);
                    }
                    else
                    {
                        s = sendto(gameNetworkState.hostInfo.socket.s, msg, sizeof(*msg), 0,
                                   (struct sockaddr*) (playerInfo->address.storage), playerInfo->address.len);
                        if(s <= 0)
                        {
                            DBPRINTF(("sendto: %d (errno: %s)\n", (int) s, strerror(errno)));
                        }
                    }
                }
                
                // only host has to send to everyone, since host will re-bcast packets
                if(!gameNetworkState.hostInfo.hosting) break;
            }
        }
        playerInfo = playerInfo->next_connected;
    }
    
    return s > 0? GAME_NETWORK_ERR_NONE: GAME_NETWORK_ERR_FAIL;
}
    
gameNetworkError
gameNetwork_forward_to_player(gameNetworkMessage* msg, gameNetworkPlayerInfo* playerInfoTarget)
{
    return gameNetwork_send_player_core(msg, playerInfoTarget);
}
    
gameNetworkError
gameNetwork_send_player(gameNetworkMessage* msg, gameNetworkPlayerInfo* playerInfoTarget)
{
    int r;
    msg->player_id = gameNetworkState.my_player_id;
    
    msg->needs_stream = 0;
    
    msg->timestamp = get_time_ms_wall();
    
    gameMessage_to_nbo(msg);
    
    r = gameNetwork_send_player_core(msg, playerInfoTarget);
    
    gameMessage_from_nbo(msg);
    
    return r;
}

gameNetworkError
gameNetwork_send(gameNetworkMessage* msg)
{
    return gameNetwork_send_player(msg, NULL);
}

gameNetworkError
gameNetwork_receive(gameNetworkMessage* msg, gameNetworkAddress* src_addr, unsigned int timeout_ms)
{
    long r = 0;
    struct sockaddr_in6 from_addr;
    socklen_t from_addr_len;
    struct timeval block_time;
    fd_set rd_set;
    int *pSock;
    int found = 0;
    int sock_max;
    
    memset(&block_time, 0, sizeof(block_time));
    if(timeout_ms > 0)
    {
        block_time.tv_sec = timeout_ms / 1000;
        block_time.tv_usec = (timeout_ms % 1000) * 1000;
    }
    
    while(gameNetworkState.hostInfo.socket.s != -1 &&
          gameNetworkState.hostInfo.stream_socket.s != -1 &&
          gameNetworkState.hostInfo.map_socket.s != -1)
    {
        pSock = NULL;
        FD_ZERO(&rd_set);
        
        FD_SET(gameNetworkState.hostInfo.socket.s, &rd_set);
        sock_max = gameNetworkState.hostInfo.socket.s;
        
        gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next_connected;
        while(pInfo)
        {
            if(pInfo->stream_socket.s > 0)
            {
                if(pInfo->stream_socket.s > sock_max)
                {
                    sock_max = pInfo->stream_socket.s;
                }
                
                FD_SET(pInfo->stream_socket.s, &rd_set);
            }
            pInfo = pInfo->next_connected;
        }
        
        if(select(sock_max+1, &rd_set, NULL, NULL, &block_time) <= 0)
        {   
            return GAME_NETWORK_ERR_FAIL;
        }

        // find socket available for reading
        if(FD_ISSET(gameNetworkState.hostInfo.socket.s, &rd_set))
        {
            pSock = &gameNetworkState.hostInfo.socket.s;
        }
        else
        {
            pInfo = gameNetworkState.player_list_head.next_connected;
            while(pInfo)
            {
                if(FD_ISSET(pInfo->stream_socket.s, &rd_set))
                {
                    pSock = &pInfo->stream_socket.s;
                    break;
                }
                pInfo = pInfo->next_connected;
            }
        }
        
        if(!pSock) return GAME_NETWORK_ERR_FAIL;
        
        memset(msg, 0, sizeof(*msg));
        
        from_addr_len = sizeof(from_addr);
        errno = 0;
        r = (int) recvfrom(*pSock,
                     (void*) msg, sizeof(*msg),
                     0, (struct sockaddr*) &from_addr, &from_addr_len);
        if(r <= 0)
        {
#ifdef _NOT_POSIX
            errno = WSAGetLastError();
            if (errno == WSAEMSGSIZE)
            {
                DBPRINTF(("errno=WSAEMSGSIZE: (sizeof(gameNetworkMessage):%u)\n", sizeof(gameNetworkMessage)));
                continue;
            }
            if (errno == WSAECONNRESET)
            {
                continue;
            }
#endif
            //close(*pSock);
            //*pSock = -1;
            return GAME_NETWORK_ERR_FAIL;
        }
        
        memset(src_addr, 0, sizeof(*src_addr));
        memcpy(src_addr->storage, &from_addr, from_addr_len);
        src_addr->len = from_addr_len;

        // TODO: endian swap all fields on little-endian platforms
        gameMessage_from_nbo(msg);
        
        if(gameNetworkState.hostInfo.hosting)
        {
            if(msg->cmd >= GAME_NETWORK_MSG_BEACON &&
                msg->cmd <= GAME_NETWORK_MSG_BEACON_RESP)
            {
            }
            else if(msg->player_id == gameNetworkState.my_player_id)
            {
                // ignore packets from self
                continue;
            }
            else if(!msg->rebroadcasted)
            {
                int needs_stream = msg->needs_stream;
                msg->rebroadcasted = 1;
                
                gameMessage_to_nbo(msg);
                
                // resend to all players
                gameNetworkPlayerInfo* playerInfo = gameNetworkState.player_list_head.next_connected;
                while(playerInfo)
                {
                    if(playerInfo->player_id == msg->player_id)
                    {
                        // sending player
                        found = 1;
                    }
                    else if(playerInfo->player_id == gameNetworkState.my_player_id)
                    {
                        // dont resend to self
                    }
                    else
                    {
                        int* rb_sock = needs_stream? &playerInfo->stream_socket.s:
                                      &gameNetworkState.hostInfo.socket.s;
                        
                        if(gameNetwork_forward_to_player(msg, playerInfo) == GAME_NETWORK_ERR_FAIL)
                        {
                            *rb_sock = -1;
                        }
                           
                    }
                    playerInfo = playerInfo->next_connected;
                }
                
                gameMessage_from_nbo(msg);
            }
        }
        else
        {
        }
        break;
    }

    if (r != sizeof(*msg)) DBPRINTF((" ERROR: msg size wrong (%u/%u)", r, sizeof(*msg)));
    
    return r == sizeof(*msg)? GAME_NETWORK_ERR_NONE: GAME_NETWORK_ERR_FAIL;
}

gameNetworkError
gameNetwork_accept_map_download(gameNetworkAddress* src_addr, char* (*mapRenderCallback)(void))
{
    struct sockaddr_storage from_addr;
    socklen_t from_addr_len;
    struct timeval block_time;
    fd_set rd_set;
    int s = gameNetworkState.hostInfo.map_socket.s;
    
    if(s < 0) return GAME_NETWORK_ERR_FAIL;
    
    memset(&block_time, 0, sizeof(block_time));
    
    // any new incoming connections?
    FD_ZERO(&rd_set);
    FD_SET(s, &rd_set);
    
    if(select(s+1, &rd_set, NULL, NULL, &block_time) > 0)
    {
        if(FD_ISSET(s, &rd_set))
        {
            from_addr_len = sizeof(from_addr);
            
            int a = accept(s, (struct sockaddr*) &from_addr, &from_addr_len);

            int so_arg = 0;
            setsockopt(a, SOL_SOCKET, SO_LINGER, &so_arg, sizeof(so_arg));
            
            if(a > 0)
            {
                long w;
                char inbuf;

                gameNetworkState.hostInfo.map_socket_sending.s = a;
                
                // wait for a newline to be received
                while(1)
                {
                    w = recv(a, &inbuf, sizeof(inbuf), 0);

                    char *map_data = mapRenderCallback();
                    
                    if(w > 0 && map_data && (inbuf == '\n' || inbuf == '\r'))
                    {
                        do
                        {
                            long send_len = 128;
                            if(strlen(map_data) < send_len) send_len = strlen(map_data);
                            w = send(a, map_data, send_len, 0);
                            map_data += w;
                            
                        } while(w > 0 && strlen(map_data) > 0);
                        
                        send(a, map_eom, strlen(map_eom), 0);
                    }
                    else break;
                }
                
                close(a);
            }
        }
    }
    
    return GAME_NETWORK_ERR_FAIL;
}

gameNetworkError
gameNetwork_accept_stream_connection(gameNetworkAddress* src_addr)
{
    struct sockaddr from_addr;
    socklen_t from_addr_len;
    struct timeval block_time;
    fd_set rd_set;
    int s = gameNetworkState.hostInfo.stream_socket.s;
    gameNetworkPlayerInfo* playerInfo;
    
    if(s < 0) return GAME_NETWORK_ERR_FAIL;
    
    memset(&block_time, 0, sizeof(block_time));
    
    // any new incoming connections?
    FD_ZERO(&rd_set);
    FD_SET(s, &rd_set);
    
    if(select(s+1, &rd_set, NULL, NULL, &block_time) > 0)
    {
        if(FD_ISSET(s, &rd_set))
        {
            from_addr_len = sizeof(from_addr);
            
            int a = accept(s, &from_addr, &from_addr_len);

            int so_arg = 0;
            setsockopt(a, SOL_SOCKET, SO_LINGER, &so_arg, sizeof(so_arg));
            
            if(a > 0)
            {
                // receive player-id
                // TODO: timeout
                gameNetworkMessage connectMsg;
                long r = recv(a, &connectMsg, sizeof(connectMsg), 0);
                
                gameMessage_from_nbo(&connectMsg);
                
                if(r == sizeof(connectMsg))
                {
                    if(gameNetwork_getPlayerInfo(connectMsg.player_id, &playerInfo, 1) == GAME_NETWORK_ERR_NONE)
                    {
                        // store socket
                        playerInfo->stream_socket.s = a;
                    }
                }
                else
                {
                    close(a);
                }
            }
        }
    }
    
    return GAME_NETWORK_ERR_FAIL;
}

gameNetworkError
gameNetwork_addPlayerInfo(int player_id)
{
    gameNetworkPlayerInfo* pInfoTail = &gameNetworkState.player_list_head;
    gameNetworkPlayerInfo* pInfo;
    
    pInfo = (gameNetworkPlayerInfo*) malloc(sizeof(gameNetworkPlayerInfo));
    if(!pInfo) return GAME_NETWORK_ERR_FAIL;
    
    memset(pInfo, 0, sizeof(gameNetworkPlayerInfo));
    pInfo->time_last_update = get_time_ms_wall();
    pInfo->elem_id = WORLD_ELEM_ID_INVALID;
    
    pInfo->player_id = player_id;
    
    while(pInfoTail->next_connected)
    {
        pInfoTail = pInfoTail->next_connected;
    }
    pInfoTail->next_connected = pInfo;
    pInfoTail->next = pInfo;
    
    return GAME_NETWORK_ERR_NONE;
}

gameNetworkError
gameNetwork_getPlayerInfo(int player_id, gameNetworkPlayerInfo** info_out, int add_if_not_found)
{
    gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next_connected;
    
    while(pInfo)
    {
        if(pInfo->player_id == player_id)
        {
            *info_out = pInfo;
            return GAME_NETWORK_ERR_NONE;
        }
        pInfo = pInfo->next_connected;
    }
    
    // not found, add
    if(add_if_not_found)
    {
        if(gameNetwork_addPlayerInfo(player_id) == GAME_NETWORK_ERR_NONE)
        {
            int r = gameNetwork_getPlayerInfo(player_id, info_out, 0);
            
            return r;
        }
    }
    
    return GAME_NETWORK_ERR_FAIL;
}

gameNetworkError
gameNetwork_getPlayerInfoForElemID(int player_elem_id, gameNetworkPlayerInfo** info_out)
{
    gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next_connected;
    
    while(pInfo)
    {
        if(pInfo->elem_id == player_elem_id)
        {
            *info_out = pInfo;
            return GAME_NETWORK_ERR_NONE;
        }
        pInfo = pInfo->next_connected;
    }
    
    return GAME_NETWORK_ERR_FAIL;
}

gameNetworkError
gameNetwork_removePlayerBonjour(int player_id)
{
    gameNetworkPlayerInfo* pInfo = &gameNetworkState.player_list_head;
    while(pInfo->next_connected)
    {
        if(pInfo->next_connected->player_id == player_id)
        {
            pInfo->next_connected->time_last_update = get_time_ms_wall() - GAME_NETWORK_TIMEOUT_MS;
        }
        pInfo = pInfo->next_connected;
    }
    return GAME_NETWORK_ERR_NONE;
}

gameNetworkError
gameNetwork_removePlayer(int player_id)
{
    game_timeval_t network_time_ms = get_time_ms_wall();
    gameNetworkPlayerInfo* pInfo = &gameNetworkState.player_list_head;
    while(pInfo->next_connected)
    {
        if(pInfo->next_connected->player_id != player_id)
        {
            goto removePlayer_retry;
        }
        
        if(pInfo->next_connected->player_id == GAME_NETWORK_PLAYER_ID_HOST)
        {
            console_write("Lost connection to %s (host)", pInfo->name);
            
            if(!gameNetworkState.connected_signal)
            {
                // we are in main thread, signal background thread to disconnect and stop
                gameNetwork_disconnectSignal();
                break;
            }
        }
        
        console_write("Player %s disconnected (%lu)", pInfo->next_connected->name,
                      network_time_ms - pInfo->next_connected->time_last_update);
        
        world_remove_object(pInfo->next_connected->elem_id);
        
        gameNetworkPlayerInfo* pFree = pInfo->next_connected;
        pInfo->next_connected = pInfo->next_connected->next_connected;
        if(pFree->stream_socket.s > 0)
        {
            close(pFree->stream_socket.s);
        }
        
        if(gameNetworkState.hostInfo.bonjour_lan)
        {
            GameNetworkBonjourManagerDisconnectPeer(pFree->player_id);
        }
        
        if(pFree->map_data)
        {
            char* freeptr = pFree->map_data;
            pFree->map_data = NULL;
            free(freeptr);
        }
        
        // only freeing at disconnect-time
        //free(pFree);
        break;

    removePlayer_retry:
        pInfo = pInfo->next_connected;
    }
    
    return GAME_NETWORK_ERR_NONE;
}

gameNetworkError
gameNetwork_updatePlayerObject(gameNetworkPlayerInfo* playerInfo,
                               float x, float y, float z,
                               float alpha, float beta, float gamma,
                               float velx, float vely, float velz)
{
    WorldElemListNode* pElemNode = world_elem_list_find(playerInfo->elem_id, &gWorld->elements_list);
    
    WorldElem* pElem = NULL;
    if(pElemNode) pElem = pElemNode->elem;
    
    if(pElem)
    {
        int obj_id =
        world_replace_object(pElem->elem_id, pElem->type,
                             x, y, z,
                             alpha, beta, gamma,
                             pElem->scale, pElem->texture_id);
        if(obj_id != WORLD_ELEM_ID_INVALID)
        {
            pElem->object_type = OBJ_PLAYER;
        
            update_object_velocity(pElem->elem_id, velx, vely, velz, 0);
        }
        
        return GAME_NETWORK_ERR_NONE;
    }
    return GAME_NETWORK_ERR_FAIL;
}

static int
get_world_elem_info(int elem_id, gameNetworkMessage* msg)
{
    WorldElemListNode* pNode = world_elem_list_find(elem_id, &gWorld->elements_list);
    if(pNode)
    {
        WorldElem* pElem = pNode->elem;
        
        memset(&(msg->params), 0, sizeof(msg->params));
        
        int i = 0;
        msg->params.f[i++] = pElem->type;
        msg->params.f[i++] = pElem->physics.ptr->x;
        msg->params.f[i++] = pElem->physics.ptr->y;
        msg->params.f[i++] = pElem->physics.ptr->z;
        msg->params.f[i++] = pElem->physics.ptr->alpha;
        msg->params.f[i++] = pElem->physics.ptr->beta;
        msg->params.f[i++] = pElem->physics.ptr->gamma;
        msg->params.f[i++] = pElem->physics.ptr->vx; // [7]
        msg->params.f[i++] = pElem->physics.ptr->vy;
        msg->params.f[i++] = pElem->physics.ptr->vz;
        msg->params.f[i++] = pElem->lifetime;
        msg->params.f[i++] = pElem->destructible;
        msg->params.f[i++] = pElem->scale;
        msg->params.f[i++] = pElem->texture_id;
        msg->params.f[i++] = pElem->stuff.bullet.action; // action f[14]
        msg->params.f[i++] = pElem->object_type; // f[15]

        return GAME_NETWORK_ERR_NONE;
    }
    
    return GAME_NETWORK_ERR_FAIL;
}

void
gameNetwork_alert(char* alert)
{
    gameNetworkMessage msg;
    gameNetworkAddress fakeAddress;

    memset(&msg, 0, sizeof(msg));
    msg.cmd = GAME_NETWORK_MSG_ALERT_BEGIN;
    msg.rebroadcasted = 1;
    
    char* p = alert;
    do{
        size_t l = strlen(p);
        
        if(l >= sizeof(msg.params.c)-1) l = sizeof(msg.params.c)-1;
        
        memset(msg.params.c, 0, sizeof(msg.params.c));
        strncpy(msg.params.c, p, l);
        
        gameNetwork_send(&msg);
        
        do_game_network_handle_msg(&msg, &fakeAddress, get_time_ms_wall());
        
        msg.cmd = GAME_NETWORK_MSG_ALERT_CONTINUE;
        p += l;
    } while(*p != '\0');
    
    msg.cmd = GAME_NETWORK_MSG_ALERT_END;
    gameNetwork_send(&msg);
    do_game_network_handle_msg(&msg, &fakeAddress, get_time_ms_wall());
}

static void
game_network_periodic_check()
{
    const game_timeval_t time_out_ms = GAME_NETWORK_TIMEOUT_MS;
    gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next_connected;
    game_timeval_t network_time_ms = get_time_ms_wall();
    int map_retransmit = 0;
    
    static game_timeval_t time_retransmit_map_last = 0;
    static game_timeval_t time_game_remaining_dec_last = 0;
    static game_timeval_t time_game_start_alert_last = 0;
    
    if(network_time_ms - time_retransmit_map_last > 1000*2 && !gameNetworkState.hostInfo.bonjour_lan)
    {
        time_retransmit_map_last = network_time_ms;
        map_retransmit = 1;
    }
    
    if(gameNetworkState.connected && network_time_ms - time_game_remaining_dec_last > 1000)
    {
        time_game_remaining_dec_last = network_time_ms;
        gameNetworkState.time_game_remaining -= 1;
    }
    
    if(gameNetworkState.connected && gameNetworkState.hostInfo.hosting && !gameStateSinglePlayer.started
       && network_time_ms - time_game_start_alert_last > 10000)
    {
        time_game_start_alert_last = network_time_ms;
        gameDialogStartNetworkGameWait();
        console_append("waiting for players...\n");
    }
    
    while(pInfo)
    {
        game_timeval_t network_time_ms = get_time_ms_wall();
        
        if(pInfo->player_id != gameNetworkState.my_player_id)
        {
            if(network_time_ms - pInfo->time_last_update > time_out_ms)
            {
                gameNetwork_removePlayer(pInfo->player_id);
                break;
            }
        }
        
        if(gameNetworkState.hostInfo.hosting)
        {
            if(network_time_ms - pInfo->time_status_last > 1000)
            {
                gameNetworkMessage statusMsg;

                pInfo->time_status_last = network_time_ms;
                
                memset(&statusMsg, 0, sizeof(statusMsg));
                
                statusMsg.cmd = GAME_NETWORK_MSG_PLAYER_STATUS;
                statusMsg.params.playerStatus.is_towing = 0;
                statusMsg.params.playerStatus.ping_ms = pInfo->time_ping;
                gameNetwork_send_player(&statusMsg, pInfo);
            }
        }
        
        // periodically call gameNetworkHookOnMessage if its set (to allow retransmitting)
        if(gameNetworkState.gameNetworkHookOnMessage && map_retransmit)
        {
            gameNetworkMessage retryMsg;
            
            memset(&retryMsg, 0, sizeof(retryMsg));
            retryMsg.cmd = GAME_NETWORK_MSG_PING;

            gameNetworkState.gameNetworkHookOnMessage(&retryMsg, &pInfo->address);
        }
        
        pInfo = pInfo->next_connected;
    }

    if(!gameNetworkState.hostInfo.hosting)
    {
        gameNetworkObjectInfo* pObjectInfo = &gameNetworkState.game_object_list_head;
        while(pObjectInfo->next)
        {
            if(network_time_ms - pObjectInfo->next->update_time > 1000)
            {
                world_remove_object(pObjectInfo->next->elem_id_local);
                
                gameNetworkObjectInfo* pFree = pObjectInfo->next;
                
                pObjectInfo->next = pObjectInfo->next->next;
                free(pFree);
                continue;
            }
            pObjectInfo = pObjectInfo->next;
        }
    }
    
    if(gameNetworkState.hostInfo.hosting)
    {
        if(network_time_ms - gameNetworkState.hostInfo.time_last_stats_alert > 10000)
        {
            if(gameNetwork_getPlayerInfo(GAME_NETWORK_PLAYER_ID_HOST, &pInfo, 0) == GAME_NETWORK_ERR_NONE)
            {
                pInfo->elem_id = my_ship_id;
            }
            
            gameNetworkState.hostInfo.time_last_stats_alert = network_time_ms;
            gameNetwork_sendStatsAlert();
            
            gameNetwork_sendPing();
            
            // register game with directory
            send_beacon(&gameNetworkState.gameDirectory.directory_address, gameNetworkState.hostInfo.name);
        }
    }
    
    if(network_time_ms - gameNetworkState.time_last_name_send > 5000)
    {
        gameNetworkState.time_last_name_send = network_time_ms;
        
        gameNetworkMessage nameMsg;
        
        memset(&nameMsg, 0, sizeof(nameMsg));
        
        nameMsg.cmd = GAME_NETWORK_MSG_MYINFO;
        strncpy(nameMsg.params.c, gameNetworkState.my_player_name, sizeof(nameMsg.params.c)-1);
        gameNetwork_send(&nameMsg);
    }
    
    if(network_time_ms - gameNetworkState.time_last_unacked_send > 200 &&
       gameNetworkState.msgUnacked.cmd != GAME_NETWORK_MSG_NONE &&
       gameNetworkState.msgUnackedRetransmitCount < 10)
    {
        gameNetworkState.time_last_unacked_send = network_time_ms;
        gameNetwork_send(&gameNetworkState.msgUnacked);
        gameNetworkState.msgUnackedRetransmitCount++;
    }
    
    gameNetworkState.time_last_periodic_check = network_time_ms;
}

void
do_game_network_write()
{
    gameNetworkMessage netMsg;
    WorldElemListNode* pNode;
    static game_timeval_t network_objects_update_time_last = 0;
    static float net_obj_update_vel_ignore_min = 0.1;
    float network_time_ms = get_time_ms_wall();
    
    if(!gameNetworkState.connected)
    {
        return;
    }
    
    game_network_periodic_check();
    
    if(network_time_ms - update_time_last >= update_frequency_ms)
    {
        update_time_last = network_time_ms;

        memset(&netMsg, 0, sizeof(netMsg));
        netMsg.cmd = GAME_NETWORK_MSG_PLAYER_INFO;
        if(get_world_elem_info(my_ship_id, &netMsg) == GAME_NETWORK_ERR_NONE)
        {
            netMsg.params.f[16] = update_time_last;
        
            gameNetwork_send(&netMsg);
        }
    }
    
    // send info about new bullets/blocks here
    // JB: moved to do_game_network_world_update

    // send info about AI/missles state here
    if(gameNetworkState.hostInfo.hosting && network_time_ms - network_objects_update_time_last >= update_frequency_ms)
    {
        network_objects_update_time_last = network_time_ms;
        
        pNode = gWorld->elements_moving.next;
        while(pNode)
        {
            if(pNode->elem->stuff.intelligent ||
               pNode->elem->object_type == OBJ_POWERUP_GENERIC ||
               pNode->elem->object_type == OBJ_SPAWNPOINT ||
               pNode->elem->object_type == OBJ_SPAWNPOINT_ENEMY ||
               pNode->elem->object_type == OBJ_TOUCHCONTROLBALL)
            {
                int net_obj_update = 0;
                
                // find network-object in list
                gameNetworkObjectInfo* pObjectInfo = gameNetworkState.game_object_list_head.next;
                while(pObjectInfo)
                {
                    if(pObjectInfo->elem_id_local == pNode->elem->elem_id) break;
                    
                    pObjectInfo = pObjectInfo->next;
                }
                
                // not found, add to network-list and assign an ID
                if(!pObjectInfo)
                {
                    pObjectInfo = malloc(sizeof(*pObjectInfo));
                    if(!pObjectInfo) break;
                    
                    pObjectInfo->next = gameNetworkState.game_object_list_head.next;
                    gameNetworkState.game_object_list_head.next = pObjectInfo;
                    
                    pObjectInfo->elem_id_local = pNode->elem->elem_id;
                    
                    gameNetworkState.game_object_id_next++;
                    if(gameNetworkState.game_object_id_next <= GAME_NETWORK_OBJECT_ID_MIN)
                    {
                        gameNetworkState.game_object_id_next = GAME_NETWORK_OBJECT_ID_MIN+1;
                    }
                    pObjectInfo->elem_id_net = gameNetworkState.game_object_id_next;
                    pNode->elem->stuff.game_object_id = gameNetworkState.game_object_id_next;
                    net_obj_update = 1;
                }
                
                // object velocity sufficient
                if(pNode->elem->physics.ptr->velocity > net_obj_update_vel_ignore_min &&
                   network_time_ms - pObjectInfo->update_time >= update_frequency_ms)
                    net_obj_update = 1;
                
                // or time interval sufficient
                if(network_time_ms - pObjectInfo->update_time >= 500)
                    net_obj_update = 1;
                
                if(net_obj_update)
                {
                    memset(&netMsg, 0, sizeof(netMsg));
                    netMsg.cmd = GAME_NETWORK_MSG_REPLACE_SERVER_OBJECT_WITH_ID;
                    get_world_elem_info(pNode->elem->elem_id, &netMsg);
                    netMsg.params.f[16] = network_time_ms;
                    netMsg.params.f[17] = pNode->elem->durability;
                    netMsg.params.f[18] = pNode->elem->stuff.affiliation;
                    netMsg.params.f[19] = pNode->elem->stuff.subtype;
                    netMsg.elem_id_net = pObjectInfo->elem_id_net;
                    
                    gameNetwork_send(&netMsg);
                    
                    pObjectInfo->update_time = network_time_ms;
                }
            }
            pNode = pNode->next;
        }
        
        // remove network-objects that have disappeared
        gameNetworkObjectInfo* pObjectInfo = &gameNetworkState.game_object_list_head;
        while(pObjectInfo->next)
        {
            if(!world_elem_list_find(pObjectInfo->next->elem_id_local, &gWorld->elements_list))
            {
                memset(&netMsg, 0, sizeof(netMsg));
                netMsg.cmd = GAME_NETWORK_MSG_REMOVE_SERVER_OBJECT_WITH_ID;
                netMsg.elem_id_net = pObjectInfo->elem_id_net;
                gameNetwork_send(&netMsg);
                
                gameNetworkObjectInfo* pFree = pObjectInfo->next;
                pObjectInfo->next = pObjectInfo->next->next;
                free(pFree);
                continue;
            }
            pObjectInfo = pObjectInfo->next;
        }
    }
}

void
do_game_network_world_update()
{
    gameNetworkMessage netMsg;
    WorldElemListNode* pNode;
    float network_time_ms = get_time_ms_wall();

    // HACK: also calling game_ai_run here
    if(!gameNetworkState.connected || gameNetworkState.hostInfo.hosting) game_ai_run();
    
    if(!gameNetworkState.connected)
    {
        return;
    }
    
    //get_time_ms_wall();
    
    /*
     * walk list of newly pending-added objects, do not do world_elem_remove here
     */
    pNode = gWorld->elements_to_be_added.next;
    while(pNode)
    {
        memset(&netMsg, 0, sizeof(netMsg));
        
        // for additions not spawned by other players
        //if(pNode->elem->stuff.player.player_id <= 0)
        if(!pNode->elem->stuff.network_created)
        {
            switch(pNode->elem->object_type)
            {
                case OBJ_MISSLE:
                {                    
                    netMsg.cmd = GAME_NETWORK_MSG_FIRE_MISSLE;
                    get_world_elem_info(pNode->elem->elem_id, &netMsg);
                    
                    // add target information
                    if(pNode->elem->stuff.u.enemy.target_id >= 0)
                    {
                        WorldElemListNode* pMisTgtNode = world_elem_list_find(pNode->elem->stuff.u.enemy.target_id,
                                                                              &gWorld->elements_moving);
                        if(pMisTgtNode)
                        {
                            if(pMisTgtNode->elem->object_type != OBJ_PLAYER)
                            {
                                // find network-id
                                gameNetworkObjectInfo* objectInfo = gameNetworkState.game_object_list_head.next;
                                while(objectInfo)
                                {
                                    if(objectInfo->elem_id_local == pMisTgtNode->elem->elem_id)
                                    {
                                        netMsg.params.f[16] = objectInfo->elem_id_net;
                                    }
                                    objectInfo = objectInfo->next;
                                }
                            }
                            else
                            {
                                netMsg.params.f[16] = pMisTgtNode->elem->stuff.affiliation;
                            }
                        }
                    }
                    
                    gameNetwork_send(&netMsg);
                    
                    if(gameNetworkState.hostInfo.hosting)
                    {
                        do_game_network_handle_msg(&netMsg, &gameNetworkState.hostInfo.addr, network_time_ms);
                    }
                    
                    // make the original inert
                    world_object_set_lifetime(pNode->elem->elem_id, 1);
                    pNode->elem->object_type = OBJ_DISPLAYONLY;
                }
                break;
                    
                case OBJ_BULLET:
                {
                    netMsg.cmd = GAME_NETWORK_MSG_FIRE_BULLET;
                    get_world_elem_info(pNode->elem->elem_id, &netMsg);
                    
                    // add bullet-action
                    netMsg.params.f[14] = pNode->elem->stuff.bullet.action;
                    
                    gameNetwork_send(&netMsg);
                    
                    if(!gameNetworkState.hostInfo.hosting &&
                       pNode->elem->stuff.bullet.action >= ACTION_SHOOT_BLOCK &&
                       pNode->elem->stuff.bullet.action <= ACTION_PLACE_TURRET)
                    {
                        // clients don't automatically spawn objects
                        // cause local copy of bullet to be harmless
                        world_get_last_object()->stuff.bullet.action = ACTION_FIRE_BULLET;
                        world_get_last_object()->durability = 0;
                    }
                    
                    if(gameNetworkState.hostInfo.hosting && gameNetworkState.player_list_head.next_connected)
                    {
                        gameNetworkState.player_list_head.next_connected->stats.shots_fired++;
                    }
                }
                break;
                    
                    // TODO: generate these locally ?
                case OBJ_POOPEDCUBE:
                {
                    /*
                    netMsg.cmd = GAME_NETWORK_MSG_ADD_OBJECT;
                    netMsg.rebroadcasted = 0;
                    get_world_elem_info(pNode->elem->elem_id, &netMsg);
                    gameNetwork_send(&netMsg);
                     */
                }
                break;
                    
                case OBJ_WRECKAGE:
                {
                    netMsg.cmd = GAME_NETWORK_MSG_KILLED;
                    get_world_elem_info(pNode->elem->elem_id, &netMsg);
                    gameNetwork_send(&netMsg);
                }
                break;
                    
                case OBJ_BLOCK:
                if(gameNetworkState.hostInfo.hosting)
                {
                    netMsg.cmd = GAME_NETWORK_MSG_ADD_OBJECT;
                    netMsg.player_id = gameNetworkState.my_player_id;
                    get_world_elem_info(pNode->elem->elem_id, &netMsg);
                    gameNetwork_send(&netMsg);
                }
                break;
                    
                default:
                    break;
            }
        }
        pNode = pNode->next;
    }
    
    // interpolate euler-velocity
    gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next_connected;
    while(pInfo)
    {
        if(pInfo->player_id == my_ship_id)
        {
        }
        else
        {
            WorldElemListNode* pElemNode = world_elem_list_find(pInfo->elem_id, &gWorld->elements_moving);
            WorldElem* pElem = NULL;
            if(pElemNode) pElem = pElemNode->elem;
            
            motion_interpolate_euler(network_time_ms, &pInfo->motion, pInfo, pElem);
        }
        pInfo = pInfo->next_connected;
    }
}

void
do_game_network_handle_msg(gameNetworkMessage* msg, gameNetworkAddress* srcAddr, game_timeval_t network_time_ms)
{
    gameNetworkPlayerInfo* playerInfo, *playerInfoFound, *playerInfoTarget;
    WorldElemListNode* pNode;
    gameNetworkObjectInfo* objectInfo;
    game_timeval_t t;
    int net_obj_id;
    int interp_velo = 1;
    int object_type = OBJ_BLOCK;
    char* ptrStr;
    
    //DBPRINTF(("msg.cmd:%d (ntohl:%d", msg->cmd, ntohl(msg->cmd)));
    
    if(!gameNetworkState.connected) return;
    
    if(gameNetworkState.gameNetworkHookOnMessage != NULL)
    {
        if(gameNetworkState.gameNetworkHookOnMessage(msg, srcAddr))
        {
            return;
        }
    }
    
    if(msg->is_ack && msg->msg_seq == gameNetworkState.msgUnacked.msg_seq)
    {
        memset(&gameNetworkState.msgUnacked, 0, sizeof(gameNetworkState.msgUnacked));
        return;
    }
    
    if(gameNetworkState.hostInfo.hosting &&
       msg->needs_ack &&
       !msg->is_ack)
    {
        gameNetworkMessage msgAck;
        
        memcpy(&msgAck, msg, sizeof(*msg));
        
        msgAck.is_ack = 1;
        gameNetwork_send(&msgAck);
        
        /* already processed this msg */
        if(msg->msg_seq == gameNetworkState.msg_seq_acked_last) return;
        
        gameNetworkState.msg_seq_acked_last = msg->msg_seq;
    }
    
    if(msg->cmd == GAME_NETWORK_MSG_BEACON && gameNetworkState.hostInfo.hosting)
    {
        memset(msg, 0, sizeof(*msg));
        
        msg->cmd = GAME_NETWORK_MSG_BEACON_RESP;
        msg->player_id = gameNetworkState.my_player_id;
        strncpy(msg->params.c, gameNetworkState.hostInfo.name, sizeof(msg->params.c));
        //send_lan_broadcast(&msg);

        send_to_address_udp(msg, srcAddr);
        console_write("GAME_NETWORK_MSG_BEACON_RESP->");
        return;
    }
    
    // find player info
    // TODO: check game-id matches
    if(gameNetwork_getPlayerInfo(msg->player_id, &playerInfo, 0) == GAME_NETWORK_ERR_FAIL)
    {
        int add_player = 0;
        int add_name = 0;
        
        if(gameNetworkState.hostInfo.hosting && msg->cmd == GAME_NETWORK_MSG_CONNECT)
        {
            add_player = 1;
            add_name = 1;
        }
        else if(msg->cmd == GAME_NETWORK_MSG_PLAYER_INFO)
        {
            //add_player = 1;
        }
        
        if(add_player)
        {
            // add new player
            if(gameNetwork_addPlayerInfo(msg->player_id) != GAME_NETWORK_ERR_NONE)
            {
                return;
            }
            
            if(gameNetwork_getPlayerInfo(msg->player_id, &playerInfo, 0) == GAME_NETWORK_ERR_FAIL) return;
            
            if(add_name) strncpy(playerInfo->name, msg->params.c, GAME_NETWORK_MAX_STRING_LEN-1);
            
            console_write("player %s joined", playerInfo->name);
            
            memcpy(&playerInfo->address, srcAddr, sizeof(*srcAddr));
            playerInfo->elem_id = WORLD_ELEM_ID_INVALID;
            
            // send back message with our player-name
            send_connect(&playerInfo->address);
            
            gameDialogStartNetworkGameWait();
        }
        else
        {
            return;
        }
        
        interp_velo = 0;
    }
    
    while(1)
    {
        //game_timeval_t network_time_ms = get_time_ms_wall();
        int object_target = WORLD_ELEM_ID_INVALID;
        
        switch(msg->cmd)
        {
            case GAME_NETWORK_MSG_CONNECT:
                strncpy(playerInfo->name, msg->params.c, sizeof(msg->params.c));
                // send back message with our player-name
                send_connect(&playerInfo->address);
                break;
                
            case GAME_NETWORK_MSG_DISCONNECT:
                playerInfo->time_last_update = 0;
                break;
                
            case GAME_NETWORK_MSG_PING:
                msg->cmd = GAME_NETWORK_MSG_PONG;
                gameNetwork_send(msg);
                break;
                
            case GAME_NETWORK_MSG_PONG:
                playerInfo->network_latency = network_time_ms - playerInfo->time_ping;
                break;
                
            case GAME_NETWORK_MSG_PLAYER_INFO:
                {
                    WorldElem *pPlayerElem = NULL;
                    
                    pNode = world_elem_list_find(playerInfo->elem_id, &gWorld->elements_list);
                    
                    if(pNode)
                    {
                        // happens when the map was rebuilt?
                        if(pNode->elem->type != msg->params.f[0] ||
                           pNode->elem->object_type != msg->params.f[15])
                        {
                            DBPRINTF(("playerInfo->elem_id object type mismatch\n"));
                            world_remove_object(pNode->elem->elem_id);
                            
                            playerInfo->elem_id = WORLD_ELEM_ID_INVALID;
                        }
                    }
                    
                    if(!pNode ||
                       playerInfo->elem_id == WORLD_ELEM_ID_INVALID)
                    {
                        //respawn
                        playerInfo->elem_id = world_add_object(msg->params.f[0], 0, 0, 0, 0, 0, 0,
                                                               msg->params.f[12], msg->params.f[13]);
                        pNode = world_elem_list_find(playerInfo->elem_id, &gWorld->elements_list);
                        
                        pNode->elem->object_type = OBJ_PLAYER;
                        pNode->elem->durability = DURABILITY_PLAYER;
                        pNode->elem->stuff.affiliation = playerInfo->player_id;
                        pNode->elem->stuff.game_object_id = playerInfo->player_id;
                        pNode->elem->bounding_remain = 1;
                        pNode->elem->stuff.network_created = 1;
                        world_object_set_nametag(playerInfo->elem_id, playerInfo->name);
                        
                        interp_velo = 0;
                    }
                    
                    pPlayerElem = pNode->elem;
                    
                    motion_interp_st* motion = &playerInfo->motion;
                    
                    t = msg->params.f[16];

                    float velDetected[3] =
                    {
                        (msg->params.f[1] - pPlayerElem->physics.ptr->x) * (1000.0 / (t - motion->timestamp_last[0])),
                        (msg->params.f[2] - pPlayerElem->physics.ptr->y) * (1000.0 / (t - motion->timestamp_last[0])),
                        (msg->params.f[3] - pPlayerElem->physics.ptr->z) * (1000.0 / (t - motion->timestamp_last[0]))
                    };
                    
                    motion_interpolate_velocity(network_time_ms, motion, pPlayerElem, msg, interp_velo);

                    //pPlayerElem->stuff.player.player_id = playerInfo->player_id;
                    pPlayerElem->stuff.affiliation = pPlayerElem->stuff.game_object_id = playerInfo->player_id;
                    
                    // handle shot fired since last update
                    if(playerInfo->shot_fired)
                    {
                        pPlayerElem->stuff.towed_elem_id = WORLD_ELEM_ID_INVALID;
                    }
                    playerInfo->shot_fired = 0;
                    
                    if(network_time_ms - playerInfo->time_cube_pooped_last >= pooped_cube_interval_ms)
                    {
                        playerInfo->time_cube_pooped_last = network_time_ms;
                        
                        if(firePoopedCube(pPlayerElem) != WORLD_ELEM_ID_INVALID)
                        {
                            WorldElem *pCube = world_get_last_object();
                            pCube->physics.ptr->vx = pCube->physics.ptr->vy = pCube->physics.ptr->vz = 0;
                        }
                    }
                    
                    playerInfo->time_last_update = network_time_ms;
                }
                break;
                
            case GAME_NETWORK_MSG_FIRE_MISSLE:
                {
                    if(!gameNetworkState.hostInfo.hosting)
                    {
                        break;
                    }
                    
                    playerInfo->stats.shots_fired++;
                    
                    playerInfo->shot_fired = 1;
                    
                    if(gameNetwork_getPlayerInfo(msg->params.f[16], &playerInfoTarget, 0) == GAME_NETWORK_ERR_NONE)
                    {
                        object_target = playerInfoTarget->elem_id;
                    }
                    else
                    {
                        objectInfo = gameNetworkState.game_object_list_head.next;
                        while(objectInfo)
                        {
                            if(objectInfo->elem_id_net == msg->params.f[16])
                            {
                                object_target = objectInfo->elem_id_local;
                                break;
                            }
                            objectInfo = objectInfo->next;
                        }
                    }
                    
                    world_add_object(msg->params.f[0],
                                     msg->params.f[1], msg->params.f[2], msg->params.f[3],
                                     msg->params.f[4], msg->params.f[5], msg->params.f[6],
                                     msg->params.f[12], msg->params.f[13]);
                    
                    //world_get_last_object()->stuff.player.player_id = playerInfo->player_id;
                    world_get_last_object()->stuff.affiliation = playerInfo->player_id;
                    world_get_last_object()->object_type = msg->params.f[15];
                    game_elem_setup_missle(world_get_last_object());
                    world_get_last_object()->stuff.u.enemy.target_id = object_target;
                    world_object_set_lifetime(world_get_last_object()->elem_id, msg->params.f[10]);
                    world_get_last_object()->destructible = msg->params.f[11];
                    world_get_last_object()->stuff.network_created = 1;

                    if(msg->params.f[7] != 0 || msg->params.f[8] != 0 || msg->params.f[9] != 0)
                    {
                        update_object_velocity(world_get_last_object()->elem_id,
                                               msg->params.f[7], msg->params.f[8], msg->params.f[9],
                                               0);
                    }
                    
                    gameAudioPlaySoundAtLocation("missle", msg->params.f[1], msg->params.f[2], msg->params.f[3]);
                }
                break;
                
            case GAME_NETWORK_MSG_FIRE_BULLET:
                playerInfo->stats.shots_fired++;
                playerInfo->shot_fired = 1;
            case GAME_NETWORK_MSG_ADD_OBJECT:
            case GAME_NETWORK_MSG_KILLED:
                object_type = msg->params.f[15];
                
                world_add_object(msg->params.f[0],
                                 msg->params.f[1], msg->params.f[2], msg->params.f[3],
                                 msg->params.f[4], msg->params.f[5], msg->params.f[6],
                                 msg->params.f[12], msg->params.f[13]);
                
                //world_get_last_object()->stuff.player.player_id = playerInfo->player_id;
                world_get_last_object()->stuff.affiliation = playerInfo->player_id;
                world_get_last_object()->object_type = object_type;
                
                if(msg->params.f[7] != 0 || msg->params.f[8] != 0 || msg->params.f[9] != 0)
                {
                    update_object_velocity(world_get_last_object()->elem_id,
                                           msg->params.f[7], msg->params.f[8], msg->params.f[9],
                                           0);
                }
                
                if(msg->params.f[10])
                {
                    //world_get_last_object()->stuff.player.player_id = playerInfo->player_id;
                    world_get_last_object()->stuff.affiliation = playerInfo->player_id;
                    world_object_set_lifetime(world_get_last_object()->elem_id, msg->params.f[10]);
                }
                
                world_get_last_object()->destructible = msg->params.f[11];
                
                world_get_last_object()->stuff.network_created = 1;
                
                if(msg->params.f[0] == MODEL_BULLET)
                {
                    gameAudioPlaySoundAtLocation("shoot", msg->params.f[1], msg->params.f[2], msg->params.f[3]);
                    
                    world_get_last_object()->stuff.sound.emit_sound_id = GAME_SOUND_ID_BULLET_FLYBY;
                    world_get_last_object()->stuff.sound.emit_sound_duration = GAME_SOUND_DURATION_BULLET_FLYBY;
                }
                
                if(msg->cmd == GAME_NETWORK_MSG_KILLED)
                {
                    gameAudioPlaySoundAtLocation("dead", msg->params.f[1], msg->params.f[2], msg->params.f[3]);
                }
                
                if(msg->cmd == GAME_NETWORK_MSG_FIRE_BULLET)
                {
                    world_get_last_object()->stuff.bullet.action = msg->params.f[14];
                }
                break;
                
            case GAME_NETWORK_MSG_REMOVE_OBJECT: // collision / object removed
                break;
                
            case GAME_NETWORK_MSG_REPLACE_SERVER_OBJECT_WITH_ID:
                net_obj_id = msg->elem_id_net;
                objectInfo = gameNetworkState.game_object_list_head.next;
                while(objectInfo)
                {
                    if(objectInfo->elem_id_net == net_obj_id)
                    {
                        break;
                    }
                    objectInfo = objectInfo->next;
                }
                
                if(!objectInfo)
                {
                    // add new object
                    objectInfo = (gameNetworkObjectInfo*) malloc(sizeof(gameNetworkObjectInfo));
                    if(objectInfo)
                    {
                        objectInfo->next = gameNetworkState.game_object_list_head.next;
                        gameNetworkState.game_object_list_head.next = objectInfo;
                        objectInfo->elem_id_net = net_obj_id;
                        objectInfo->elem_id_local = WORLD_ELEM_ID_INVALID;
                    }
                }
                
                if(objectInfo)
                {
                    float obj_v[3] = {
                        msg->params.f[7],
                        msg->params.f[8],
                        msg->params.f[9]
                    };
                    
                    t = msg->params.f[16];
                    WorldElemListNode* last_found = NULL;
                    
                    if(objectInfo->elem_id_local != WORLD_ELEM_ID_INVALID)
                    {
                        last_found = world_elem_list_find(objectInfo->elem_id_local, &gWorld->elements_list);
                    }
                    
                    if(!last_found)
                    {
                        objectInfo->elem_id_local =
                            world_add_object(msg->params.f[0],
                                             msg->params.f[1], msg->params.f[2], msg->params.f[3],
                                             msg->params.f[4], msg->params.f[5], msg->params.f[6],
                                             msg->params.f[12], msg->params.f[13]);
                        //world_get_last_object()->stuff.player.player_id = msg->player_id;
                        // hack to map model-id to an object_type
                        // TODO: what about POWERUP?
                        world_get_last_object()->object_type = msg->params.f[15];
                        world_get_last_object()->destructible = msg->params.f[11];
                        world_get_last_object()->durability = msg->params.f[17];
                        world_get_last_object()->stuff.affiliation = msg->params.f[18];
                        world_get_last_object()->stuff.subtype = msg->params.f[19];
                        world_get_last_object()->stuff.network_created = 1;
                        
                        last_found = world_elem_list_find(objectInfo->elem_id_local, &gWorld->elements_list);
                    }
                    else
                    {
                        world_replace_object(objectInfo->elem_id_local,
                                             msg->params.f[0],
                                             msg->params.f[1], msg->params.f[2], msg->params.f[3],
                                             msg->params.f[4], msg->params.f[5], msg->params.f[6],
                                             msg->params.f[12], msg->params.f[13]);
                        last_found->elem->object_type = msg->params.f[15];
                    }

                    if(last_found)
                    {
                        motion_interpolate_velocity(network_time_ms, &objectInfo->motion, last_found->elem, msg, 1);
                        
                        /*
                        // log position information
                        for(int i = 2; i > 0; i--)
                        {
                            for(int d = 0; d < 3; d++)
                            {
                                objectInfo->loc_last[d][i] = objectInfo->loc_last[d][i-1];
                            }
                            objectInfo->timestamp_last[i] = objectInfo->timestamp_last[i-1];
                        }
                        
                        objectInfo->loc_last[0][0] = last_found->elem->physics.ptr->x;
                        objectInfo->loc_last[1][0] = last_found->elem->physics.ptr->y;
                        objectInfo->loc_last[2][0] = last_found->elem->physics.ptr->z;
                        objectInfo->timestamp_last[0] = t;
                        
                        float time_win = objectInfo->timestamp_last[0] - objectInfo->timestamp_last[2];
                        
                        if(time_win < 1000 && time_win > 0)
                        {
                            // predict velocity
                            float v1[3];
                            float v[3] =
                            {
                                last_found->elem->physics.ptr->x,
                                last_found->elem->physics.ptr->y,
                                last_found->elem->physics.ptr->z
                            };
                            
                            for(int d = 0; d < 3; d++)
                            {
                                v1[d] = predict_a1_at_b1_for_a_over_b(objectInfo->loc_last[d],
                                                                      objectInfo->timestamp_last,
                                                                      t + 1000);
                                obj_v[d] = v1[d] - v[d];
                            }
                        }
                        
                        if(object_is_static(last_found->elem->object_type))
                        {
                            printf("WARN: GAME_NETWORK_MSG_REPLACE_SERVER_OBJECT_WITH_ID object_is_static!\n");
                        }
                        else
                        {
                            update_object_velocity(objectInfo->elem_id_local,
                                                   obj_v[0], obj_v[1], obj_v[2], 0);
                        }
                         */
                    }
                    
                    objectInfo->update_time = network_time_ms;
                }
                break;
                
            case GAME_NETWORK_MSG_ALERT_BEGIN:
                msg->params.c[sizeof(msg->params.c)-1] = '\0';
                gameNetworkState.gameStatsMessage[0] = '\0';
                sprintf(gameNetworkState.gameStatsMessage, "%s:\n%s", playerInfo->name, msg->params.c);
                break;
                
            case GAME_NETWORK_MSG_ALERT_CONTINUE:
                {
                    char* p = msg->params.c;
                    p[sizeof(msg->params.c)-1] = '\0';
                    if(strlen(gameNetworkState.gameStatsMessage) + strlen(p) < sizeof(gameNetworkState.gameStatsMessage))
                    {
                        DBPRINTF(("ALERT_CONTINUE: appending\n%s", p));
                        strcat(gameNetworkState.gameStatsMessage, p);
                    }
                    
                    DBPRINTF(("recv alert continue:\n%s", msg->params.c));
                    
                }
                break;
                
            case GAME_NETWORK_MSG_ALERT_END:
                DBPRINTF(("GAME_NETWORK_MSG_ALERT_END: completed\n%s", gameNetworkState.gameStatsMessage));
                if((ptrStr = strstr(gameNetworkState.gameStatsMessage, REMAINING_TIME_STR)))
                {
                    sscanf(ptrStr + strlen(REMAINING_TIME_STR), "%f", &gameNetworkState.time_game_remaining);
                }
                
                gameDialogNetworkGameStatus();
                break;
                
            case GAME_NETWORK_MSG_HITME:
                break;
                
            case GAME_NETWORK_MSG_KILLEDME:
                {
                    char deathMsg[255];
                    
                    if(gameNetwork_getPlayerInfo(msg->params.i[0], &playerInfoFound, 0) == GAME_NETWORK_ERR_NONE)
                    {
                        if(msg->params.i[0] == gameNetworkState.my_player_id)
                        {
                            // no special handling needed here?
                        }
                        
                        playerInfo->stats.killed++;
                        playerInfoFound->stats.killer++;
                        
                        // choose a random "death-verb" ;-)
                        int v = 0;
                        v = rand() % (sizeof(gameKillVerbs) / sizeof(char*) );
                        
                        sprintf(deathMsg, "\n%s %s %s (%s)", playerInfoFound->name, gameKillVerbs[v],
                                playerInfo->name,
                                msg->params.i[1] == OBJ_MISSLE? "with a missle": "with a laser");
                    }
                    else
                    {
                        sprintf(deathMsg, "\n%s was killed", playerInfo->name);
                    }
                    
                    pNode = world_elem_list_find(playerInfo->elem_id, &gWorld->elements_list);
                    if(pNode)
                    {
                        game_handle_destruction(pNode->elem);
                    }
                    
                    gameNetwork_alert(deathMsg);
                }
                break;
                
            case GAME_NETWORK_MSG_MYINFO:
                strncpy(playerInfo->name, msg->params.c, sizeof(msg->params.c)-1);
                if(playerInfo->elem_id != WORLD_ELEM_ID_INVALID)
                {
                    world_object_set_nametag(playerInfo->elem_id, playerInfo->name);
                }
                break;
                
            case GAME_NETWORK_MSG_TOW_DROP:
                {
                    if(gameNetworkState.hostInfo.hosting)
                    {
                        t = msg->params.f[16];
                        pNode = world_elem_list_find(playerInfo->elem_id, &gWorld->elements_list);
                        if(pNode)
                        {
                            pNode->elem->stuff.towed_elem_id = WORLD_ELEM_ID_INVALID;
                        }
                    }
                }
                break;
                
            case GAME_NETWORK_MSG_STARTGAME:
                console_write("netgame started!");
                if(my_ship_id != WORLD_ELEM_ID_INVALID)
                {
                    WorldElemListNode* pMyShipNode = world_elem_list_find(my_ship_id, &gWorld->elements_moving);
                    
                    if(pMyShipNode)
                    {
                        world_remove_object(my_ship_id);
                        my_ship_id = WORLD_ELEM_ID_INVALID;
                    }
                }
                break;
                
            case GAME_NETWORK_MSG_ENDGAME:
                {
                    gameDialogNetworkGameEnded();
                }
                break;
                
            case GAME_NETWORK_MSG_GET_MAP_REQUEST:
                {
                    unsigned long offset = msg->params.mapData.offset;
                    gameNetworkMessage msgOut;
                    char *map_data_ptr;
                    
                    memset(&msgOut, 0, sizeof(msgOut));
                    
                    if(offset == 0)
                    {
                        char* map_data = strdup(do_game_map_render());
                        
                        if(playerInfo->map_data)
                        {
                            free(playerInfo->map_data);
                        }
                        playerInfo->map_data = map_data;
                        map_data_ptr = map_data;
                        
                        msgOut.cmd = GAME_NETWORK_MSG_GET_MAP_BEGIN;
                        msgOut.player_id = gameNetworkState.my_player_id;
                        
                        send_to_address_udp(&msgOut, srcAddr);
                    }
                    else if(offset > 0 && playerInfo->map_data && offset < strlen(playerInfo->map_data))
                    {
                        map_data_ptr = playerInfo->map_data + offset;
                    }
                    else
                    {
                        break;
                    }
                    
                    playerInfo->time_last_update = get_time_ms_wall();
                    
                    msgOut.cmd = GAME_NETWORK_MSG_GET_MAP_SOME;
                    msgOut.player_id = gameNetworkState.my_player_id;
                    
                    int block_cur = 0;
                    msgOut.params.mapData.offset = map_data_ptr - playerInfo->map_data;
                    
                    while(*map_data_ptr && block_cur < GAME_NETWORK_MAX_MAP_STRING_LEN)
                    {
                        msgOut.params.mapData.s[block_cur] = *map_data_ptr;
                        map_data_ptr++;
                        block_cur++;
                    }

                    send_to_address_udp(&msgOut, srcAddr);
                    
                    if(!*map_data_ptr)
                    {
                        msgOut.cmd = GAME_NETWORK_MSG_GET_MAP_END;
                        msgOut.params.mapData.offset = strcksum(playerInfo->map_data);
                        console_write("sent map cksum:\n%lu\nbytes:%lu\n", msgOut.params.mapData.offset, strlen(playerInfo->map_data));
                        send_to_address_udp(&msgOut, srcAddr);
                    }
                }
                break;
                
            default:
                break;
        }
        
        break;
    }
}
    
static char*
do_game_map_render()
{
    char* map = gameMapReadRendered();
    
    return map;
}
    
void
do_game_network_read_core()
{
    gameNetworkMessage msg;
    gameNetworkAddress srcAddr;
    int receive_block_ms = 5;
    int retries = 10;
    gameNetworkMessageQueued* pMsgNew;
    
    if(!gameNetworkState.connected)
    {
        usleep(GAME_NETWORK_READ_THREAD_IDLE_USLEEP_INTERVAL);
        return;
    }
    
    while(retries > 0)
    {
        pMsgNew = NULL;
        
        if(gameNetworkState.hostInfo.bonjour_lan)
        {
            retries = 1;
        }
        else if(gameNetwork_receive(&msg, &srcAddr, receive_block_ms) == GAME_NETWORK_ERR_NONE)
        {
            printf("DEBUG: gameNetwork_receive cmd:%d\n", msg.cmd);
            
            // messages that don't require synchronizing with rendering/game thread
            if(msg.cmd == GAME_NETWORK_MSG_BEACON && gameNetworkState.hostInfo.hosting)
            {
                memset(&msg, 0, sizeof(msg));
                
                msg.cmd = GAME_NETWORK_MSG_BEACON_RESP;
                msg.player_id = gameNetworkState.my_player_id;
                strncpy(msg.params.c, gameNetworkState.hostInfo.name, sizeof(msg.params.c));

                send_to_address_udp(&msg, &srcAddr);
                DBPRINTF(("GAME_NETWORK_MSG_BEACON\n"));
                continue;
            }
            /*
            else if(msg.cmd == GAME_NETWORK_MSG_GET_MAP_REQUEST && msg.params.mapData.offset > 0)
            {
                do_game_network_handle_msg(&msg, &srcAddr);
                continue;
            }
            else if(msg.cmd == GAME_NETWORK_MSG_GET_MAP_SOME)
            {
                do_game_network_handle_msg(&msg, &srcAddr);
                continue;
            }
             */
            
            // add to queue
            pMsgNew = (gameNetworkMessageQueued*) malloc(sizeof(gameNetworkMessageQueued));
        }

        // clean up processed messages
        game_lock_lock(&gameNetworkState.msgQueue.lock);
        
        while(gameNetworkState.msgQueue.head.next && gameNetworkState.msgQueue.head.next->processed)
        {
            gameNetworkMessageQueued* pMsgFree = gameNetworkState.msgQueue.head.next;
            gameNetworkState.msgQueue.head.next = pMsgFree->next;
            free(pMsgFree);
        }
        
        if(pMsgNew)
        {
            pMsgNew->next = NULL;
            pMsgNew->msg = msg;
            pMsgNew->srcAddr = srcAddr;
            pMsgNew->receive_time = get_time_ms_wall();
            pMsgNew->processed = 0;
            
            gameNetworkMessageQueued* pTail = &gameNetworkState.msgQueue.head;
            while(pTail->next) pTail = pTail->next;
            pTail->next = pMsgNew;
        }
        
        game_lock_unlock(&gameNetworkState.msgQueue.lock);
        
        retries--;
    }
    
    if(retries <= 0)
    {
        //printf("Warning: network thread lagging\n");
    }
}
    
void
do_game_network_read()
{
    if(gameNetworkState.connected_signal)
    {
        if(gameNetworkState.connected)
        {
            gameNetwork_disconnect();
        }
        else
        {
            gameNetworkState.connected = 1;
        }
        gameNetworkState.connected_signal = 0;
        return;
    }

    if(!gameNetworkState.hostInfo.bonjour_lan)
    {
        do_game_network_read_core();
    }
    else
    {
        usleep(GAME_NETWORK_READ_THREAD_IDLE_USLEEP_INTERVAL); // 100ms
    }
}

void
do_game_network_read_bonjour()
{
    if(gameNetworkState.hostInfo.bonjour_lan)
    {
        do_game_network_read_core();
    }
}

void
gameNetwork_sendHitBy(gameNetworkPlayerID killer, int object_type)
{
    gameNetworkMessage msg;
    gameNetworkAddress fakeAddress;
    
    memset(&msg, 0, sizeof(msg));
    
    msg.cmd = GAME_NETWORK_MSG_HITME;
    msg.player_id = gameNetworkState.my_player_id;
    msg.params.i[0] = killer;
    msg.params.i[1] = object_type;
    msg.msg_seq = gameNetworkState.msg_seq_next;
    msg.rebroadcasted = 1;
    gameNetworkState.msg_seq_next++;
    
    if(gameNetworkState.hostInfo.hosting)
    {
        do_game_network_handle_msg(&msg, &fakeAddress, get_time_ms_wall());
    }
    else
    {
        gameNetwork_send(&msg);
    }
}
    
static void
gameNetwork_sendMsgWithAck(gameNetworkMessage* msg)
{
    msg->needs_ack = 1;
    msg->msg_seq = gameNetworkState.msg_seq_next;
    gameNetworkState.msg_seq_next++;
    
    memcpy(&gameNetworkState.msgUnacked, msg, sizeof(*msg));
    gameNetworkState.msgUnackedRetransmitCount = 0;
    
    gameNetwork_send(msg);
}
    
void
gameNetwork_sendKilledBy(gameNetworkPlayerID killer, int object_type)
{
    gameNetworkMessage msg;
    gameNetworkAddress fakeAddress;
    
    memset(&msg, 0, sizeof(msg));
    
    msg.cmd = GAME_NETWORK_MSG_KILLEDME;
    msg.player_id = gameNetworkState.my_player_id;
    msg.params.i[0] = killer;
    msg.params.i[1] = object_type;
    msg.rebroadcasted = 1; /* do not rebroadcast, for server only */
    
    if(gameNetworkState.hostInfo.hosting)
    {
        do_game_network_handle_msg(&msg, &fakeAddress, get_time_ms_wall());
    }
    else
    {
        gameNetwork_sendMsgWithAck(&msg);
    }
}

void
gameNetwork_sendStatsAlert()
{
    gameNetworkPlayerInfo* pInfo;
    char str[4096];
    char tmp[128];
    int clear_stats = 0;
    const char* decor = "^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L\n";
    char *rows[] = {"NAME   ","", "K^N   ", "D^M   ", "SHOTS", "PTS  ", "PING "};
    char *cSep = "|";
    char *pRowTop = NULL;
    static unsigned NAME_BREAK = 6;
    
    memset(str, 0, sizeof(str));
    
    if(gameStateSinglePlayer.started)
    {
        if(game_time_remaining() <= 0) clear_stats = 1;
    }
    else
    {
        strncat(str, "ALERT:\nWaiting for host\n", sizeof(str)-1);
    }
    
    strncat(str, "ALERT:\n", sizeof(str)-1);
    
    strncat(str, decor, sizeof(str)-1);
    sprintf(tmp, "%s%lu\n", REMAINING_TIME_STR, (unsigned long) game_time_remaining());
    if(gameStateSinglePlayer.started)
    {
        strncat(str, tmp, sizeof(str)-1);
        strncat(str, decor, sizeof(str)-1);
    }
    
    int row = 0;
    while(row < sizeof(rows)/sizeof(char*))
    {
        int col = 0;
        pInfo = gameNetworkState.player_list_head.next;
        
        if(!pRowTop) pRowTop = str + strlen(str);
        
        char* pLastSep = pRowTop;
        
        strncat(str, rows[row], sizeof(str)-1);
        
        while(pInfo)
        {
            int maxPad = 8;
            while(row > 0 && maxPad > 0 && pLastSep && *(pLastSep) && *pLastSep != cSep[0])
            {
                // TODO: bug here where this loop never exits - hack is to cap with maxPad
                strncat(str, " ", sizeof(str)-1);
                pLastSep++;
                maxPad--;
            }
            
            if(row == 1) pLastSep++;
            
            switch(row)
            {
                case 0:
                    strncat(str, " ", sizeof(str)-1);
                    strncat(str, cSep, sizeof(str)-1);
                    strncat(str, pInfo->name, NAME_BREAK);
                    break;
                    
                case 1:
                    if(strlen(pInfo->name) > NAME_BREAK)
                    {
                        char* p = str+strlen(str), *rp = pInfo->name + NAME_BREAK;
                        *p = ' ';
                        p++;
                        while(*(pLastSep) != cSep[0] && *(pLastSep) != '\n')
                        {
                            if(*rp != '\0') *p = *rp;
                            else *p = ' ';
                            p++;
                            pLastSep++;
                            rp++;
                        }
                        *p = '\0';
                    }
                    break;
                    
                case 2:
                    sprintf(tmp, "%03d", pInfo->stats.killer);
                    strncat(str, tmp, sizeof(str)-1);
                    if(clear_stats) pInfo->stats.killer = 0;
                    break;
                    
                case 3:
                    sprintf(tmp, "%03d", pInfo->stats.killed);
                    strncat(str, tmp, sizeof(str)-1);
                    if(clear_stats) pInfo->stats.killed = 0;
                    break;
                    
                case 4:
                    sprintf(tmp, "%03d", pInfo->stats.shots_fired);
                    strncat(str, tmp, sizeof(str)-1);
                    if(clear_stats) pInfo->stats.shots_fired = 0;
                    break;
                    
                case 5:
                    sprintf(tmp, "%03d", pInfo->stats.points);
                    strncat(str, tmp, sizeof(str)-1);
                    if(clear_stats) pInfo->stats.points = 0;
                    break;
                    
                case 6:
                    sprintf(tmp, "%03d", (int) pInfo->network_latency);
                    strncat(str, tmp, sizeof(str)-1);
                    if(clear_stats) pInfo->stats.score_calculated = 0;
                    break;
            }
            
            if(row > 1) pLastSep += 3;
            
            pInfo = pInfo->next;
            col++;
        }
        strncat(str, "\n", sizeof(str)-1);
        row++;
    }
    
    strncat(str, decor, sizeof(str)-1);
    
    gameNetwork_alert(str);
    
    if(clear_stats)
    {
        send_endgame();
    }
    
    //console_clear();
    //console_write(str);
}

void
gameNetwork_sendPing()
{
    gameNetworkPlayerInfo* pInfo;
    gameNetworkMessage msg;
    
    memset(&msg, 0, sizeof(msg));
    msg.cmd = GAME_NETWORK_MSG_PING;
    
    pInfo = gameNetworkState.player_list_head.next_connected;
    while(pInfo)
    {
        pInfo->time_ping = get_time_ms_wall();
        pInfo = pInfo->next_connected;
    }
    
    gameNetwork_send(&msg);
}

void
gameNetwork_handle_collision(WorldElem* elemA, WorldElem* elemB, int collision_action)
{
    gameNetworkPlayerInfo *pInfo = NULL;
    WorldElem* pElemMyShip = NULL, *pElemC = NULL;
    
    if(!elemB) return;
    
    if(elemA->elem_id == my_ship_id)
    {
        pElemMyShip = elemA;
        pElemC = elemB;
    }
    else if(elemB->elem_id == my_ship_id)
    {
        pElemMyShip = elemB;
        pElemC = elemA;
    }
    
    if(gameNetworkState.hostInfo.hosting)
    {
        if(collision_action == COLLISION_ACTION_POWERUP_GRAB_OR_TOW)
        {
            gameNetwork_getPlayerInfo(elemB->stuff.affiliation, &pInfo, 0);
            
            if(pInfo)
            {
                pInfo->stats.points += 100;
            }
        }
    }
    
    if(collision_action != COLLISION_ACTION_DAMAGE) return;
    
    if(pElemMyShip && (pElemC->object_type == OBJ_BULLET || pElemC->object_type == OBJ_MISSLE))
    {
        gameNetworkState.collision.affiliation_hit_last = pElemC->stuff.affiliation;
        gameNetworkState.collision.obj_type_hit_last = pElemC->object_type;
        /*
        gameNetwork_sendHitBy(gameNetworkState.collision.affiliation_hit_last,
                              gameNetworkState.collision.obj_type_hit_last);
         */
    }
    
    // network client, special case some collisions
    if(gameNetworkState.connected && !gameStateSinglePlayer.started)
    {
        if(pElemMyShip && pElemC->object_type == OBJ_POWERUP_GENERIC)
        {
            game_handle_collision_powerup(pElemMyShip, pElemC);
        }
    }
}

void
gameNetwork_handle_destruction(WorldElem* elem)
{
    gameNetworkPlayerInfo *pInfo = NULL;
    
    if(!gameNetworkState.connected) return;
    
    if(elem->object_type == OBJ_PLAYER)
    {
        gameNetwork_getPlayerInfo(elem->stuff.affiliation, &pInfo, 0);
        
        if(pInfo)
        {
            pInfo->elem_id = WORLD_ELEM_ID_INVALID;
        }
        
        if(elem->elem_id == my_ship_id)
        {
            gameNetwork_sendKilledBy(gameNetworkState.collision.affiliation_hit_last,
                                     gameNetworkState.collision.obj_type_hit_last);
        }
    }
}

void
gameNetwork_action_handle(int action)
{
    gameNetworkMessage msg;
    
    memset(&msg, 0, sizeof(msg));
    
    if(action == ACTION_DROP_TOW)
    {
        msg.cmd = GAME_NETWORK_MSG_TOW_DROP;
        gameNetwork_send(&msg);
    }
}
    
// mark -- callbacks used during connection negotiating
int gameNetwork_onBonjourConnecting3(gameNetworkMessage* msg, gameNetworkAddress* srcAddr)
{
    gameNetworkMessage downloadMsg;
    unsigned long l = gameNetworkState.server_map_data_end - gameNetworkState.server_map_data;
    int request_more = 0;
    
    gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next_connected;
    while(pInfo)
    {
        if(pInfo->player_id == GAME_NETWORK_PLAYER_ID_HOST)
        {
            pInfo->time_last_update = get_time_ms_wall();
            break;
        }
        pInfo = pInfo->next_connected;
    }
    
    if(!gameNetworkState.server_map_data) return 0;
    
    if(msg->cmd == GAME_NETWORK_MSG_GET_MAP_BEGIN)
    {
        return 1;
    }
    else if(msg->cmd == GAME_NETWORK_MSG_GET_MAP_END && gameNetworkState.server_map_data)
    {
        gameNetworkState.gameNetworkHookOnMessage = NULL;
        
        unsigned long mapcksum = strcksum(gameNetworkState.server_map_data);
        if(mapcksum == msg->params.mapData.offset)
        {
            gameDialogConnectToGameSuccessful2();
        
            gameMapSetMap(gameNetworkState.server_map_data);
            
            game_start_network_guest();
        }
        else
        {
            console_write("map download failed\ncksum:\n%lu\nbytes:%lu\n", mapcksum, strlen(gameNetworkState.server_map_data));
        }
        
        return 1;
    }
    else if(msg->cmd == GAME_NETWORK_MSG_GET_MAP_SOME && gameNetworkState.server_map_data)
    {
        if(msg->params.mapData.offset != l)
        {
            return 1;
        }
        
        request_more = 1;
        
        char* catbuf = malloc(l + GAME_NETWORK_MAX_MAP_STRING_LEN + 1);
        
        memcpy(catbuf, gameNetworkState.server_map_data, l);
        memset(catbuf + l, 0, GAME_NETWORK_MAX_MAP_STRING_LEN+1);
        
        free(gameNetworkState.server_map_data);
        
        gameNetworkState.server_map_data = catbuf;
        gameNetworkState.server_map_data_end = catbuf + l;
        
        char *p = msg->params.mapData.s;
        
        while(*p && p - msg->params.mapData.s < GAME_NETWORK_MAX_MAP_STRING_LEN)
        {
            *gameNetworkState.server_map_data_end = *p;
            gameNetworkState.server_map_data_end++;
            p++;
            l++;
        }
        
        console_clear();
        console_write("downloaded map: %lu\n", l);
    }
    else if(msg->cmd == GAME_NETWORK_MSG_PING)
    {
        if(!gameNetworkState.hostInfo.bonjour_lan) request_more = 1;
    }
    
gameNetwork_onBonjourConnecting3_retry:
    
    if(request_more > 0)
    {
        memset(&downloadMsg, 0, sizeof(downloadMsg));
        downloadMsg.cmd = GAME_NETWORK_MSG_GET_MAP_REQUEST;
        downloadMsg.rebroadcasted = 1;
        downloadMsg.params.mapData.offset = l;
        downloadMsg.player_id = gameNetworkState.my_player_id;
        
        send_to_address_udp(&downloadMsg, srcAddr);
        return 1;
    }

    return 1;
}
    
int gameNetwork_onBonjourConnecting2(gameNetworkMessage* msg, gameNetworkAddress* srcAddr)
{
    gameNetworkMessage downloadMsg;
    gameNetworkPlayerInfo *playerInfo;
    
    if(msg->cmd == GAME_NETWORK_MSG_CONNECT)
    {
        if(gameNetworkState.server_map_data) free(gameNetworkState.server_map_data);
        
        gameNetworkState.server_map_data = gameNetworkState.server_map_data_end = malloc(GAME_NETWORK_MAP_REQUEST_WINDOW_LEN);
        *gameNetworkState.server_map_data = '\0';
        
        memset(&downloadMsg, 0, sizeof(downloadMsg));
        downloadMsg.cmd = GAME_NETWORK_MSG_GET_MAP_REQUEST;
        downloadMsg.rebroadcasted = 1;
        downloadMsg.params.mapData.offset = 0;
        downloadMsg.player_id = gameNetworkState.my_player_id;
        
        if(gameNetwork_getPlayerInfo(msg->player_id, &playerInfo, 1) == GAME_NETWORK_ERR_NONE)
        {
            memcpy(&playerInfo->address, srcAddr, sizeof(gameNetworkAddress));

            send_to_address_udp(&downloadMsg, srcAddr);
        }
        
        return 1;
    }
    else if(msg->cmd == GAME_NETWORK_MSG_GET_MAP_BEGIN)
    {
        gameNetworkState.gameNetworkHookOnMessage = gameNetwork_onBonjourConnecting3;
        gameNetworkState.map_downloaded = 1; // ??
        
        return gameNetwork_onBonjourConnecting3(msg, srcAddr);
    }
    
    return 0;
}

int gameNetwork_onBonjourConnecting1(gameNetworkMessage* msg, gameNetworkAddress* srcAddr)
{
    if(msg->cmd == GAME_NETWORK_MSG_BEACON_RESP)
    {
        if(gameNetworkState.gameNetworkHookGameDiscovered)
        {
            console_write("GAME FOUND\nOpening portal to %s\n", msg->params.c);
            gameNetworkState.gameNetworkHookGameDiscovered(msg->params.c);
            return 1;
        }
        
        if(strncmp(msg->params.c, gameNetworkState.hostInfo.name, sizeof(msg->params.c)) == 0 ||
           gameNetworkState.hostInfo.bonjour_lan)
        {
            gameNetworkState.gameNetworkHookOnMessage = gameNetwork_onBonjourConnecting2;
            send_connect(srcAddr);
        }
        else
        {
            gameNetworkAddress_incrementPort(&gameNetworkState.gameDirectory.directory_address);
            send_beacon(&gameNetworkState.gameDirectory.directory_address, gameNetworkState.hostInfo.name);
        }
        
        return 1;
    }
    return 0;
}

int gameNetwork_onDirectoryRegister1(gameNetworkMessage* msg, gameNetworkAddress* srcAddr)
{
    if(msg->cmd == GAME_NETWORK_MSG_BEACON_RESP && strncmp(msg->params.c, gameNetworkState.hostInfo.name, sizeof(msg->params.c)) != 0)
    {
        // port in use, increment and try again
        console_append("server port %d in use by game\n%s, retrying\n",
                       ntohs(GAME_NETWORK_ADDRESS_PORT(&gameNetworkState.gameDirectory.directory_address)),
                       msg->params.c);
        gameNetworkAddress_incrementPort(&gameNetworkState.gameDirectory.directory_address);
        send_beacon(&gameNetworkState.gameDirectory.directory_address, gameNetworkState.hostInfo.name);
        return 1;
    }
    
    return 0;
}
    
void load_map_and_host_game()
{
    actions_menu_reset();
    save_map = 0;
    console_write("Hosting game: %s\nWaiting for guests...",
                  gameNetworkState.hostInfo.name);
    
    gameStateSinglePlayer.game_type = GAME_TYPE_NONE;
    
    if(!game_map_custom_loaded) gameMapSetMap(initial_map_deathmatch);
    
    gameNetworkState.gameNetworkHookGameDiscovered = NULL;
    
    if(!gameNetworkState.hostInfo.bonjour_lan)
    {
        gameNetwork_directoryRegister(gameSettingGameTitle);
    }
    
    gameDialogStartNetworkGameWait();
}
    
#ifdef __cplusplus
}
#endif
