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
#include <unistd.h>
#include <assert.h>
#include <unistd.h>
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

#ifdef __cplusplus
extern "C" {
#endif
    
#include "gameNetwork.h"
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
    
//#define GAMENET_TRACE() {console_append("%s:%d",__func__,__LINE__);}
#define GAMENET_TRACE()

gameNetworkState_t gameNetworkState;

static unsigned long update_frequency_ms = 20;
static unsigned long update_time_last = 0;
static int gameNetwork_inited = 0;
static int read_in_render_thread = 0;
const static float euler_interp_range_max = (M_PI/8);
const static float velo_interp_update_mult = 8;
const static char *map_eom = "\nmap_eom\n";

static char *gameKillVerbs[] = {"slaughtered", "evicerated", "de-rezzed", "shot-down", "ended", "terminated"};

static int
gameNetwork_directoryListOffset(int offset);

extern int GameNetworkBonjourManagerHost(const char* name, int* sock_out);
extern int GameNetworkBonjourManagerBrowseBegin();
extern int GameNetworkBonjourManagerBrowseEnd(gameNetworkAddress* address_returned);
extern int GameNetworkBonjourManagerDisconnect();
void GameNetworkBonjourManagerSendMessageToPeer(uint8_t* msg_, int peer_id);
extern int gameNetwork_onBonjourConnecting1(gameNetworkMessage*, gameNetworkAddress*);
static char* do_game_map_render();


////////////////////////////////

static void
socket_set_blocking(int sock, int block)
{
    int blocking = block;
    
    setsockopt(sock, SOL_SOCKET, O_NONBLOCK, &blocking, sizeof(blocking));
}

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
    struct sockaddr_in6* addr6 = (void*) &addr;
    
    sock = socket(AF_INET6, (stream? SOCK_STREAM: SOCK_DGRAM), 0);
    if(sock < 0)
    {
        console_append("%s:%d %s\n", __func__, __LINE__, "socket failure");
        return -1;
    }
    
    memset(&addr, 0, sizeof(addr));
    
    addr6->sin6_len = sizeof(struct sockaddr_in6);
    addr6->sin6_family = AF_INET6;
    addr6->sin6_addr = in6addr_any;
	addr6->sin6_port = htons(port);
    
	int bcast_enabled = 1;
	int so_arg = 1;
    int backlog = 10;
    if(!stream)
    {
        setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &bcast_enabled, sizeof(bcast_enabled));
    }
    else
    {
        /*
        int blocking = 0;
        setsockopt(sock, SOL_SOCKET, O_NONBLOCK, &blocking, sizeof(blocking));
        */
    }
    
#ifdef BSD_SOCKETS
	setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &so_arg, sizeof(so_arg));
#endif
    
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &so_arg, sizeof(so_arg));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &so_arg, sizeof(so_arg));
    
    /* bind */
    if(do_bind)
    if(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
        console_append("%s:%d %s\n", __func__, __LINE__, "socket failure (bind)");
        close(sock);
        gameDialogError("bind failed (kill/restart app)");
        return -1;
    }
    
    if(stream)
    {
        if(listen(sock, backlog))
        {
            console_append("%s:%d %s\n", __func__, __LINE__, "socket failure (listen)");
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
    sa_bc6.sin6_addr = in6addr_linklocal_allnodes;
    
    r = sendto(gameNetworkState.hostInfo.socket.s, msg, sizeof(*msg),
               0, (struct sockaddr*) &sa_bc6, sizeof(sa_bc6));
}

static void
send_to_address_udp(gameNetworkMessage* msg, gameNetworkAddress* address)
{
    struct sockaddr_storage sa;
    long r;
    
    memcpy(&sa, address->storage, address->len);
    
    if(gameNetworkState.hostInfo.bonjour_lan)
    {
        GameNetworkBonjourManagerSendMessageToPeer((void*) msg, ((struct bonjour_addr_stuffed_in_sockaddr_in *) address->storage)->peer_id);
        return;
    }
    
    r = sendto(gameNetworkState.hostInfo.socket.s, msg, sizeof(*msg),
               0, (struct sockaddr*) &sa, address->len);
}

static int
gameNetwork_getDNSAddress(char *name, gameNetworkAddress* addr)
{
    // find directory server
    struct hostent* he = gethostbyname(name);
    if(he && he->h_length > 0)
    {
        addr->len = sizeof(struct sockaddr_in);
        struct sockaddr_in* sa = (struct sockaddr_in*) addr->storage;
        sa->sin_family = AF_INET;
        sa->sin_port = htons(GAME_NETWORK_PORT);
#ifdef BSD_SOCKETS
        sa->sin_len = sizeof(struct sockaddr_in);
#endif
        memcpy(&sa->sin_addr, he->h_addr_list[0],
               sizeof(sa->sin_addr));
        console_append("DNS resolved: %s", inet_ntoa(sa->sin_addr));
        return GAME_NETWORK_ERR_NONE;
    }
    
    console_append("DNS lookup failed for %s\n", name);
    
    return GAME_NETWORK_ERR_FAIL;
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
}

gameNetworkError
gameNetwork_init(int broadcast_mode, const char* server_name,
                 const char* player_name, int update_ms,
                 unsigned short host_port,
                 const char* local_inet_addr)
{
    const char *directory_name = "d0gf1ght.domain17.net";
    
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
    
    if(!gameNetwork_inited)
    {
        gameNetwork_inited = 1;
        
        gameNetworkState.hostInfo.socket.s = -1;
        gameNetworkState.hostInfo.map_socket.s = -1;
        gameNetworkState.hostInfo.stream_socket.s = -1;
        gameNetworkState.hostInfo.map_socket_sending.s = -1;
    }
    
    gameNetworkState.inited = 1;
    
    gameNetworkState.msg_seq_acked_last = 0;
    gameNetworkState.msg_seq_next = rand();
    
    return GAME_NETWORK_ERR_NONE;
}

gameNetworkError
gameNetwork_resume()
{
    gameNetwork_initsockets();
    return GAME_NETWORK_ERR_NONE;
}

int
gameNetwork_connect(char* server_name, int host, int lan_only)
{
    int retries = 0;
    //struct sockaddr_storage addr;
    gameNetworkAddress gameAddress;
    struct sockaddr_in6* addr6 = (struct sockaddr_in6*) gameAddress.storage;
    struct sockaddr* sockaddr_p = (struct sockaddr*) gameAddress.storage;
    gameNetworkMessage connectMsg, netMsg, msgBeacon;
    gameNetworkPlayerInfo* playerInfo = NULL;
    int host_player_id = GAME_NETWORK_PLAYER_ID_HOST;
    char server_ip_resolved[64];
    int lan_bcast_retries = 0, directory_search_retries = 3, bonjour_search_retries = 8, search_retries = 0;
    unsigned short server_port_resolved = gameNetworkState.hostInfo.port;
    int retries_udp = 10;
    int delay_udp = 100;
    
    gameAddress.len = sizeof(struct sockaddr_in);
    memset(&gameAddress, 0, sizeof(gameAddress));
    
    assert(sizeof(gameAddress.storage) >= sizeof(struct sockaddr_storage));
    
    gameNetworkState.hostInfo.lan_only = lan_only;
    
    gameNetworkState.connected = 0;
    
    gameNetworkState.hostInfo.hosting = host;
    
    gameNetwork_initsockets();
    
    playerInfo = gameNetworkState.player_list_head.next;
    while(playerInfo)
    {
        gameNetworkPlayerInfo* del = playerInfo;
        playerInfo = playerInfo->next;
        free(del);
    }
    gameNetworkState.player_list_head.next = NULL;
    
    // if starts with a number, server name is ip address
    if(server_name[0] >= '0' && server_name[0] <= '9')
    {
        directory_search_retries = lan_bcast_retries = bonjour_search_retries = 0;
        strcpy(server_ip_resolved, server_name);
    }
    else
    {
        memset(server_ip_resolved, 0, sizeof(server_ip_resolved));
        
        if(!lan_only)
        {
            lan_bcast_retries = 0;
            
            gameNetwork_getDNSAddress(gameNetworkState.gameDirectory.directory_name,
                                      &gameNetworkState.gameDirectory.directory_address);
        }
        else
        {
            directory_search_retries = 0;
        }
    }
    
    if(host)
    {
        // register game with directory server
        gameNetworkState.my_player_id = GAME_NETWORK_PLAYER_ID_HOST;
        if(server_name != gameNetworkState.hostInfo.name)
        {
            strcpy(gameNetworkState.hostInfo.name, server_name);
        }
        
        console_clear();
        console_write("Hosting game %s\non port %d\n",
                       gameNetworkState.hostInfo.name,
                       gameNetworkState.hostInfo.port);
        
        gameNetwork_getPlayerInfo(gameNetworkState.my_player_id, &playerInfo, 1);
        strcpy(playerInfo->name, gameNetworkState.my_player_name);
        
        if(lan_only)
        {
            GameNetworkBonjourManagerHost(GAME_NETWORK_LAN_GAME_NAME, &gameNetworkState.hostInfo.socket.s);
        }
    }
    else
    {
        gameNetworkState.hostInfo.hosting = 0;
        
        // moving to gameNetwork_init
        if(gameNetworkState.my_player_id == 0 || gameNetworkState.my_player_id == GAME_NETWORK_PLAYER_ID_HOST)
        { 
            gameNetworkState.my_player_id = rand_in_range(1, GAME_NETWORK_PLAYER_ID_HOST-1);
        }
        
        // attempt to find via apple "bonjour" (bluetooth + wifi)
        search_retries = lan_only ? bonjour_search_retries: 0;
        gameAddress.len = sizeof(struct sockaddr_in6);
        
        while(search_retries > 0)
        {
            memset(&gameAddress, 0, sizeof(gameAddress));
            gameAddress.len = sizeof(struct sockaddr_in6);
            
            if(GameNetworkBonjourManagerBrowseEnd(&gameAddress) != 0)
            {
                gameNetworkState.hostInfo.bonjour_lan = 1;
                
                memset(&msgBeacon, 0, sizeof(msgBeacon));
                msgBeacon.cmd = GAME_NETWORK_MSG_BEACON;
                strcpy(msgBeacon.params.c, GAME_NETWORK_LAN_GAME_NAME);
                msgBeacon.player_id = gameNetworkState.my_player_id;
                
                if(sockaddr_p->sa_family != GAME_NETWORK_BONJOUR_ADDRFAMILY_HACK)
                {
                    return GAME_NETWORK_ERR_FAIL;
                }
                
                gameNetworkState.gameNetworkHookOnMessage = gameNetwork_onBonjourConnecting1;

                send_to_address_udp(&msgBeacon, &gameAddress);
                gameNetworkState.connected = 1;
                return GAME_NETWORK_ERR_NONE;
            }
            
            search_retries--;
        }
        
        // attempt to find on lan
        if(search_retries == 0)
        {
            search_retries = lan_bcast_retries;
            while(search_retries > 0)
            {
                gameNetworkMessage msgBeacon;
                
                memset(&msgBeacon, 0, sizeof(msgBeacon));
                msgBeacon.cmd = GAME_NETWORK_MSG_BEACON;
                strcpy(msgBeacon.params.c, GAME_NETWORK_LAN_GAME_NAME);
                msgBeacon.player_id = gameNetworkState.my_player_id;
                
                send_lan_broadcast(&msgBeacon);
                
                if(gameNetwork_receive(&msgBeacon, &gameAddress, 1000) == GAME_NETWORK_ERR_NONE)
                {
                    GAMENET_TRACE();
                    
                    if(msgBeacon.cmd == GAME_NETWORK_MSG_BEACON_RESP)
                    {
                        struct sockaddr_in6* pBeaconAddr6 = (struct sockaddr_in6*) gameAddress.storage;
                        struct sockaddr_in* pBeaconAddr4 = (struct sockaddr_in*) gameAddress.storage;
                        
                        if(gameAddress.len == sizeof(*pBeaconAddr6)) inet_ntop(AF_INET6, &pBeaconAddr6->sin6_addr, server_ip_resolved, sizeof(server_ip_resolved));
                        if(gameAddress.len == sizeof(*pBeaconAddr4)) inet_ntop(AF_INET, &pBeaconAddr4->sin_addr, server_ip_resolved, sizeof(server_ip_resolved));
                        server_port_resolved = ntohs(GAME_NETWORK_ADDRESS_PORT(&gameAddress));
                        GAMENET_TRACE();
                        break;
                    }
                    else continue;
                }
                search_retries--;
            }
        }
    
        if(search_retries == 0) // failed
        {
            GAMENET_TRACE();
            // attempt to find via directory
            search_retries = directory_search_retries;
            while(search_retries > 0 && server_ip_resolved[0] == '\0')
            {
                gameNetworkMessage msgSearch;
                gameNetworkMessage msgSearchResponse;
                gameNetworkAddress responseAddr;
                
                console_append("\n...trying directory server\n");
                
                memset(&msgSearch, 0, sizeof(msgSearch));
                msgSearch.cmd = GAME_NETWORK_MSG_DIRECTORY_QUERY;
                strcpy(msgSearch.params.c, gameNetworkState.hostInfo.name);
                
                GAMENET_TRACE();
                
                send_to_address_udp(&msgSearch, &gameNetworkState.gameDirectory.directory_address);
                
                int recv_retries = retries_udp;
                while(recv_retries > 0 &&
                      gameNetwork_receive(&msgSearchResponse, &responseAddr, delay_udp) == GAME_NETWORK_ERR_NONE)
                {
                    GAMENET_TRACE();
                    if(msgSearchResponse.cmd == GAME_NETWORK_MSG_DIRECTORY_RESPONSE)
                    {
                        console_append("\n...game found in directory\n");
                        memset(server_ip_resolved, 0, sizeof(server_ip_resolved));
                        strncpy(server_ip_resolved, msgSearchResponse.params.c,
                                sizeof(msgSearchResponse.params.c));
                        server_port_resolved = gameNetworkState.hostInfo.port;
                        break;
                    }
                    recv_retries--;
                }
                
                search_retries--;
            }
        }
        
        if(search_retries == 0)
        {
            // attempt to treat "hostname" as an ipv4 address
            sprintf(server_ip_resolved, "::FFFF:%s", gameNetworkState.hostInfo.name);
            gameAddress.len = sizeof(struct sockaddr_in6);
        }
        
        GAMENET_TRACE();

        if(gameAddress.len == sizeof(struct sockaddr_in6))
        {
            memset(gameAddress.storage, 0, sizeof(gameAddress.storage));
            
            if(inet_pton(AF_INET6, server_ip_resolved, &(addr6->sin6_addr)) != 1)
            {
                console_write("inet_pton(%s) failed",server_ip_resolved);
            }
            addr6->sin6_port = htons(server_port_resolved);
            addr6->sin6_family = AF_INET6;
            
#ifdef BSD_SOCKETS
            addr6->sin6_len = gameAddress.len;
#endif
        }

        memset(&msgBeacon, 0, sizeof(msgBeacon));
        msgBeacon.cmd = GAME_NETWORK_MSG_BEACON;
        strcpy(msgBeacon.params.c, GAME_NETWORK_LAN_GAME_NAME);
        msgBeacon.player_id = gameNetworkState.my_player_id;
        send_to_address_udp(&msgBeacon, &gameAddress);
        
        memset(&connectMsg, 0, sizeof(connectMsg));
        connectMsg.cmd = GAME_NETWORK_MSG_CONNECT;
        strncpy(connectMsg.params.c, gameNetworkState.my_player_name, sizeof(connectMsg.params.c));
        
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
                gameNetwork_send(&connectMsg);
                
                if(gameNetwork_receive(&netMsg, &gameAddress, 100) == GAME_NETWORK_ERR_NONE)
                {
                    if(gameNetwork_getPlayerInfo(netMsg.player_id, &playerInfo, 1) == GAME_NETWORK_ERR_NONE)
                    {
                        console_write("\rconnecting to game...");
                        
                        gameNetworkState.gameNetworkHookOnMessage = gameNetwork_onBonjourConnecting1;
                        break;
                    }
                }
                
                retries--;
            }
        }
        
        if(retries <= 0)
        {
            console_write("connect failed\n");
            console_append("(server %s:%d)\n",
                           server_name,
                           server_port_resolved);
            gameNetwork_removePlayer(host_player_id);
            return GAME_NETWORK_ERR_FAIL;
        }
    }
    
    gameNetworkState.connected = 1;
    return GAME_NETWORK_ERR_NONE;
}

void
gameNetwork_worldInit()
{
    gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next;
    
    while(pInfo)
    {
        pInfo->elem_id = WORLD_ELEM_ID_INVALID;
        
        pInfo = pInfo->next;
    }
}
    
void
gameNetwork_sendPlayersDisconnect()
{
    gameNetworkMessage msg;
    
    msg.cmd = GAME_NETWORK_MSG_DISCONNECT;
    gameNetwork_send(&msg);
}

void
gameNetwork_disconnect()
{
    if(gameNetworkState.connected)
    {
        gameNetwork_sendPlayersDisconnect();
        
        gameNetworkState.connected = 0;
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

            close(*(sock_set[s]));
            *(sock_set[s]) = -1;
        }
    }
    
    GameNetworkBonjourManagerDisconnect();
}

int
gameNetwork_countPlayers()
{
    gameNetworkPlayerInfo* p = gameNetworkState.player_list_head.next;
    int count = 0;
    
    while(p)
    {
        count++;
        p = p->next;
    }
    return count;
}

void
gameNetwork_startGame(unsigned int sec)
{
    gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next;
    
    while(pInfo)
    {
        memset(&pInfo->stats, 0, sizeof(pInfo->stats));
        
        pInfo = pInfo->next;
    }
}
    
gameNetworkError
gameNetwork_send_player_core(gameNetworkMessage* msg, gameNetworkPlayerInfo* playerInfoTarget)
{
    long s;
    
    gameNetworkPlayerInfo* playerInfo = gameNetworkState.player_list_head.next;
    while(playerInfo)
    {
        if(playerInfoTarget == playerInfo ||
           (playerInfoTarget == NULL && playerInfo->player_id != gameNetworkState.my_player_id))
        {
            if(playerInfo->player_id != msg->player_id)
            {
                if(msg->needs_stream)
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
                    }
                }
                
                // only host has to send to everyone, since host will re-bcast packets
                if(!gameNetworkState.hostInfo.hosting) break;
            }
        }
        playerInfo = playerInfo->next;
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
    msg->player_id = gameNetworkState.my_player_id;
    
    msg->needs_stream = 0;
    
    msg->timestamp = get_time_ms();
    
    return gameNetwork_send_player_core(msg, playerInfoTarget);
}

gameNetworkError
gameNetwork_send(gameNetworkMessage* msg)
{
    return gameNetwork_send_player(msg, NULL);
}

gameNetworkError
gameNetwork_receive(gameNetworkMessage* msg, gameNetworkAddress* src_addr, unsigned int timeout_ms)
{
    long r;
    struct sockaddr_storage from_addr;
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
        FD_ZERO(&rd_set);
        
        FD_SET(gameNetworkState.hostInfo.socket.s, &rd_set);
        sock_max = gameNetworkState.hostInfo.socket.s;
        
        gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next;
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
            pInfo = pInfo->next;
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
            pInfo = gameNetworkState.player_list_head.next;
            while(pInfo)
            {
                if(FD_ISSET(pInfo->stream_socket.s, &rd_set))
                {
                    pSock = &pInfo->stream_socket.s;
                    break;
                }
                pInfo = pInfo->next;
            }
        }
        
        memset(msg, 0, sizeof(*msg));
        
        from_addr_len = sizeof(from_addr);
        r = (int) recvfrom(*pSock,
                     (void*) msg, sizeof(*msg),
                     0, (struct sockaddr*) &from_addr, &from_addr_len);
        if(r <= 0)
        {
            close(*pSock);
            *pSock = -1;
        }
        
        // ignore packets from ourself
        if(msg->player_id == gameNetworkState.my_player_id) continue;
        
        memset(src_addr, 0, sizeof(*src_addr));
        memcpy(src_addr->storage, &from_addr, from_addr_len);
        src_addr->len = from_addr_len;
        
        if(gameNetworkState.hostInfo.hosting)
        {
            if(msg->cmd >= GAME_NETWORK_MSG_DIRECTORY_ADD &&
               msg->cmd <= GAME_NETWORK_MSG_DIRECTORY_RESPONSE)
            {
            }
            else if(msg->cmd >= GAME_NETWORK_MSG_BEACON &&
                    msg->cmd <= GAME_NETWORK_MSG_BEACON_RESP)
            {
            }
            else if(!msg->rebroadcasted)
            {
                msg->rebroadcasted = 1;
                
                // resend to all players
                gameNetworkPlayerInfo* playerInfo = gameNetworkState.player_list_head.next;
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
                        int* rb_sock = msg->needs_stream? &playerInfo->stream_socket.s:
                                      &gameNetworkState.hostInfo.socket.s;
                        
                        if(gameNetwork_forward_to_player(msg, playerInfo) == GAME_NETWORK_ERR_FAIL)
                        {
                            *rb_sock = -1;
                        }
                           
                    }
                    playerInfo = playerInfo->next;
                }
            }
        }
        else
        {
        }
        break;
    }
    
    return r == sizeof(*msg)? GAME_NETWORK_ERR_NONE: GAME_NETWORK_ERR_FAIL;
}

gameNetworkError
gameNetwork_accept_map_download(gameNetworkAddress* src_addr, char* (*mapRenderCallback)())
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
    gameNetworkPlayerInfo* pInfo = &gameNetworkState.player_list_head;
    while(pInfo->next)
    {
        pInfo = pInfo->next;
    }
    
    pInfo->next = (gameNetworkPlayerInfo*) malloc(sizeof(gameNetworkPlayerInfo));
    if(!pInfo->next) return GAME_NETWORK_ERR_FAIL;
    
    memset(pInfo->next, 0, sizeof(gameNetworkPlayerInfo));
    pInfo->next->time_last_update = get_time_ms();
    pInfo->next->elem_id = WORLD_ELEM_ID_INVALID;
    
    pInfo->next->player_id = player_id;
    
    return GAME_NETWORK_ERR_NONE;
}

gameNetworkError
gameNetwork_getPlayerInfo(int player_id, gameNetworkPlayerInfo** info_out, int add_if_not_found)
{
    gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next;
    
    while(pInfo)
    {
        if(pInfo->player_id == player_id)
        {
            *info_out = pInfo;
            return GAME_NETWORK_ERR_NONE;
        }
        pInfo = pInfo->next;
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
    gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next;
    
    while(pInfo)
    {
        if(pInfo->elem_id == player_elem_id)
        {
            *info_out = pInfo;
            return GAME_NETWORK_ERR_NONE;
        }
        pInfo = pInfo->next;
    }
    
    return GAME_NETWORK_ERR_FAIL;
}

gameNetworkError
gameNetwork_removePlayer(int player_id)
{
    gameNetworkPlayerInfo* pInfo = &gameNetworkState.player_list_head;
    while(pInfo->next)
    {
        if(pInfo->next->player_id == player_id)
        {
            gameNetworkPlayerInfo* pFree = pInfo->next;
            pInfo->next = pFree->next;
            if(pFree->stream_socket.s > 0)
            {
                close(pFree->stream_socket.s);
            }
            free(pFree);
            continue;
        }
        pInfo = pInfo->next;
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
        
        do_game_network_handle_msg(&msg, &fakeAddress);
        
        msg.cmd = GAME_NETWORK_MSG_ALERT_CONTINUE;
        p += l;
    } while(*p != '\0');
}

static void
game_network_periodic_check()
{
    const game_timeval_t time_out_ms = 1000*30;
    gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next;
    game_timeval_t network_time_ms = get_time_ms();
    
    while(pInfo)
    {
        if(pInfo->player_id != gameNetworkState.my_player_id)
        {
            if(network_time_ms - pInfo->time_last_update > time_out_ms)
            {
                console_write("Player %s disconnected (%lu)", pInfo->name,
                              network_time_ms - pInfo->time_last_update);
                
                if(pInfo->player_id == GAME_NETWORK_PLAYER_ID_HOST)
                {
                    console_write("Lost connection to server");
                    gameNetworkState.connected = 1;
                }
                
                world_remove_object(pInfo->elem_id);
                
                gameNetwork_removePlayer(pInfo->player_id);
                break;
            }
        }
        
        if(gameNetworkState.hostInfo.hosting)
        {
            if(network_time_ms - pInfo->time_status_last > 1000)
            {
                gameNetworkMessage statusMsg;
                
                /* TODO: finish */
                pInfo->time_status_last = network_time_ms;
                
                statusMsg.cmd = GAME_NETWORK_MSG_PLAYER_STATUS;
                statusMsg.params.playerStatus.is_towing = 0;
                statusMsg.params.playerStatus.ping_ms = pInfo->time_ping;
                gameNetwork_send_player(&statusMsg, pInfo);
            }
        }
        pInfo = pInfo->next;
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
    
    static game_timeval_t time_last_stats_alert = 0;
    if(gameNetworkState.hostInfo.hosting && network_time_ms - time_last_stats_alert > 10000)
    {
        if(gameNetwork_getPlayerInfo(GAME_NETWORK_PLAYER_ID_HOST, &pInfo, 0) == GAME_NETWORK_ERR_NONE)
        {
            pInfo->elem_id = my_ship_id;
        }
        
        time_last_stats_alert = network_time_ms;
        gameNetwork_sendStatsAlert();
        
        gameNetwork_sendPing();
    }
    
    static game_timeval_t time_last_name_send = 0;
    if(network_time_ms - time_last_name_send > 5000)
    {
        time_last_name_send = network_time_ms;
        
        gameNetworkMessage nameMsg;
        
        nameMsg.cmd = GAME_NETWORK_MSG_MYINFO;
        strncpy(nameMsg.params.c, gameNetworkState.my_player_name, sizeof(nameMsg.params.c)-1);
        gameNetwork_send(&nameMsg);
    }
    
    static game_timeval_t time_last_unacked_send = 0;
    if(network_time_ms - time_last_unacked_send > 200 &&
       gameNetworkState.msgUnacked.cmd != GAME_NETWORK_MSG_NONE &&
       gameNetworkState.msgUnackedRetransmitCount < 10)
    {
        time_last_unacked_send = network_time_ms;
        gameNetwork_send(&gameNetworkState.msgUnacked);
        gameNetworkState.msgUnackedRetransmitCount++;
    }
    
    gameNetworkState.time_last_periodic_check = time_ms;
}

void
do_game_network_write()
{
    gameNetworkMessage netMsg;
    WorldElemListNode* pNode;
    static game_timeval_t beacon_time_last = 0;
    static game_timeval_t network_objects_update_time_last = 0;
    static float net_obj_update_vel_ignore_min = 0.1;
    
    if(!gameNetworkState.connected)
    {
        return;
    }
    
    get_time_ms();
    
    game_network_periodic_check();
    
    if(gameNetworkState.hostInfo.hosting && !gameNetworkState.hostInfo.lan_only &&
       time_ms - beacon_time_last > 5000)
    {
        beacon_time_last = time_ms;
        gameNetworkMessage directoryRegisterMsg;
        
        GAMENET_TRACE();
        
        memset(&directoryRegisterMsg, 0, sizeof(directoryRegisterMsg));
        directoryRegisterMsg.cmd = GAME_NETWORK_MSG_DIRECTORY_ADD;
        // TODO: mention player-counts / game information
        strcpy(directoryRegisterMsg.params.directoryInfo.c, gameNetworkState.hostInfo.name);
        
        // send along INADDR_ANY:PORT, indicating to use the detected srcAddr
        GAME_NETWORK_ADDRESS_FAMILY(&directoryRegisterMsg.params.directoryInfo.addr) = AF_INET;
        GAME_NETWORK_ADDRESS_PORT(&directoryRegisterMsg.params.directoryInfo.addr) = htons(gameNetworkState.hostInfo.port);
        GAME_NETWORK_ADDRESS_INADDR(&directoryRegisterMsg.params.directoryInfo.addr) =
            GAME_NETWORK_ADDRESS_INADDR(&gameNetworkState.hostInfo.local_inet_addr);
        directoryRegisterMsg.params.directoryInfo.addr.len = sizeof(struct sockaddr_in6);
        
        send_to_address_udp(&directoryRegisterMsg, &gameNetworkState.gameDirectory.directory_address);
    }
    
    if(time_ms - update_time_last >= update_frequency_ms)
    {
        update_time_last = get_time_ms();

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
    if(gameNetworkState.hostInfo.hosting /*&& time_ms - network_objects_update_time_last > update_frequency_ms*/)
    {
        network_objects_update_time_last = time_ms;
        
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
                   time_ms - pObjectInfo->update_time >= update_frequency_ms)
                    net_obj_update = 1;
                
                // or time interval sufficient
                if(time_ms - pObjectInfo->update_time >= 500)
                    net_obj_update = 1;
                
                if(net_obj_update)
                {
                    memset(&netMsg, 0, sizeof(netMsg));
                    netMsg.cmd = GAME_NETWORK_MSG_REPLACE_SERVER_OBJECT_WITH_ID;
                    get_world_elem_info(pNode->elem->elem_id, &netMsg);
                    netMsg.params.f[16] = time_ms;
                    netMsg.params.f[17] = pNode->elem->durability;
                    netMsg.params.f[18] = pNode->elem->stuff.affiliation;
                    netMsg.params.f[19] = pNode->elem->stuff.subtype;
                    netMsg.elem_id_net = pObjectInfo->elem_id_net;
                    
                    gameNetwork_send(&netMsg);
                    
                    pObjectInfo->update_time = time_ms;
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
    
    if(!gameNetworkState.connected)
    {
        return;
    }
    
    collision_actions_set_grab_powerup();
    
    get_time_ms();
    
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
                        do_game_network_handle_msg(&netMsg, &gameNetworkState.hostInfo.addr);
                    }
                    
                    world_object_set_lifetime(pNode->elem->elem_id, 1);
                }
                break;
                    
                case OBJ_BULLET:
                {
                    netMsg.cmd = GAME_NETWORK_MSG_FIRE_BULLET;
                    netMsg.rebroadcasted = 0;
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
                    
                    if(gameNetworkState.hostInfo.hosting && gameNetworkState.player_list_head.next)
                    {
                        gameNetworkState.player_list_head.next->stats.shots_fired++;
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
                    netMsg.rebroadcasted = 0;
                    get_world_elem_info(pNode->elem->elem_id, &netMsg);
                    gameNetwork_send(&netMsg);
                }
                break;
                    
                case OBJ_BLOCK:
                if(gameNetworkState.hostInfo.hosting)
                {
                    netMsg.cmd = GAME_NETWORK_MSG_ADD_OBJECT;
                    netMsg.rebroadcasted = 0;
                    netMsg.player_id = gameNetworkState.my_player_id;
                    netMsg.needs_stream = 1;
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
    gameNetworkPlayerInfo* pInfo = gameNetworkState.player_list_head.next;
    while(pInfo)
    {
        if(pInfo->player_id == my_ship_id)
        {
        }
        else
        {
            float euler[3];
            int i;
            for(i = 0; i < 3; i++)
            {
                euler[i] = ((pInfo->euler_last[i][0] - pInfo->euler_last[i][1]) /
                            (pInfo->timestamp_last[0] - pInfo->timestamp_last[1])) *
                    (time_ms - pInfo->euler_last_interp);
            }
            
            WorldElemListNode* pElemNode = world_elem_list_find(pInfo->elem_id, &gWorld->elements_moving);
            
            WorldElem* pElem = NULL;
            if(pElemNode) pElem = pElemNode->elem;
            
            if(pElem)
            {
                if(fabs(euler[0]) < euler_interp_range_max &&
                   fabs(euler[1]) < euler_interp_range_max &&
                   fabs(euler[2]) < euler_interp_range_max)
                {
                    gameNetwork_updatePlayerObject(pInfo,
                                                   pElem->physics.ptr->x, pElem->physics.ptr->y, pElem->physics.ptr->z,
                                                   pElem->physics.ptr->alpha + euler[0],
                                                   pElem->physics.ptr->beta + euler[1],
                                                   pElem->physics.ptr->gamma + euler[2],
                                                   pElem->physics.ptr->vx,
                                                   pElem->physics.ptr->vy,
                                                   pElem->physics.ptr->vz);
                }
                pInfo->euler_last_interp = time_ms;
            }
        }
        pInfo = pInfo->next;
    }
}

void
do_game_network_handle_directory_msg(gameNetworkMessage *msg, gameNetworkAddress *srcAddr)
{
    gameNetworkPlayerInfo* pInfo;
    char game_name[128];
    char msgBuf[128];
    int offset, offset_o;
    
    pInfo = &gameNetworkState.gameDirectory.head;
    while(pInfo)
    {
        if(pInfo->next && time_ms - pInfo->next->time_last_update > 30000)
        {
            // remove timed-out listing
            gameNetworkPlayerInfo* pFree = pInfo->next;
            pInfo->next = pInfo->next->next;
            
            free(pFree);
        }
        pInfo = pInfo->next;
    }
    
    switch(msg->cmd)
    {
        case GAME_NETWORK_MSG_DIRECTORY_ADD:
            if(gameNetworkState.hostInfo.hosting)
            {
                gameNetworkPlayerInfo* pInfoNew = NULL;
                
                msg->params.directoryInfo.c[sizeof(msg->params.directoryInfo.c)-1] = '\0';
                pInfo = &gameNetworkState.gameDirectory.head;
                while(pInfo)
                {
                    // found, refresh listing
                    if(pInfo->next)
                    {
                        if(!strncmp(pInfo->next->name, msg->params.directoryInfo.c, sizeof(msg->params.directoryInfo.c)))
                        {
                            pInfo->next->time_last_update = time_ms;
                            break;
                        }
                    }
                    else
                    {
                        // not found, add new listing
                        pInfoNew = malloc(sizeof(gameNetworkPlayerInfo));
                        
                        if(pInfoNew)
                        {
                            pInfo->next = pInfoNew;
                            
                            memset(pInfoNew, 0, sizeof(gameNetworkPlayerInfo));
                            strncpy(pInfoNew->name, msg->params.directoryInfo.c, sizeof(msg->params.directoryInfo.c));
                            
                            pInfoNew->address = *srcAddr;
                            
                            // allow msg to override address
                            if(GAME_NETWORK_ADDRESS_LEN_CORRECT(&msg->params.directoryInfo.addr))
                            {
                                // if IP is zero, use ip from srcAddr
                                if(IN6_ARE_ADDR_EQUAL(&in6addr_any, &(GAME_NETWORK_ADDRESS_INADDR(&msg->params.directoryInfo.addr))))
                                {
                                    GAME_NETWORK_ADDRESS_INADDR(&pInfoNew->address) =
                                        GAME_NETWORK_ADDRESS_INADDR(&msg->params.directoryInfo.addr);
                                }
                                
                                // if msg port is non-zero, use that port
                                if(GAME_NETWORK_ADDRESS_PORT(&msg->params.directoryInfo.addr) != 0)
                                {
                                    GAME_NETWORK_ADDRESS_PORT(&pInfoNew->address) =
                                        GAME_NETWORK_ADDRESS_PORT(&msg->params.directoryInfo.addr);
                                }
                            }
                            
                            pInfo->next->time_last_update = time_ms;
                        }
                        
                        if(pInfoNew)
                        {
                            console_write("Registered game:%s/%s:%d", pInfoNew->name,
                                          inet_ntop(AF_INET6, &(GAME_NETWORK_ADDRESS_INADDR(&pInfoNew->address)), msgBuf, sizeof(msgBuf)),
                                          ntohs(GAME_NETWORK_ADDRESS_PORT(&pInfoNew->address)));
                        }
                        break;
                    }
                    
                    pInfo = pInfo->next;
                }
            }
            break;
            
        case GAME_NETWORK_MSG_DIRECTORY_QUERY:
            pInfo = &gameNetworkState.gameDirectory.head;
            while(pInfo)
            {
                if(pInfo->next)
                {
                    game_name[sizeof(game_name)-1] = '\0';
                    strncpy(game_name, msg->params.c, sizeof(msg->params.c));
                    
                    if(strncmp(pInfo->next->name, msg->params.c, sizeof(msg->params.c)) == 0)
                    {
                        // TODO: send back game port number as well!
                        gameNetworkMessage queryResponseMsg;
                        
                        memset(&queryResponseMsg, 0, sizeof(queryResponseMsg));
                        queryResponseMsg.cmd = GAME_NETWORK_MSG_DIRECTORY_RESPONSE;
                        struct sockaddr_in* sin_ptr = (struct sockaddr_in*) pInfo->next->address.storage;
                        if(inet_ntoa(sin_ptr->sin_addr))
                        {
                            console_write("QUERY: found game %s at\n%s", game_name, inet_ntoa(sin_ptr->sin_addr));
                            strcpy(queryResponseMsg.params.c, inet_ntoa(sin_ptr->sin_addr));
                        }
                        send_to_address_udp(&queryResponseMsg, srcAddr);
                    }
                }
                
                pInfo = pInfo->next;
            }
            break;
            
        case GAME_NETWORK_MSG_DIRECTORY_QUERY_RANDOM:
            break;
            
        case GAME_NETWORK_MSG_DIRECTORY_QUERY_LIST_AT_OFFSET:
            offset = msg->params.f[1];
            offset_o = offset;
            pInfo = gameNetworkState.gameDirectory.head.next;
            while(pInfo)
            {
                if(offset == 0)
                {
                    if(pInfo)
                    {
                        gameNetworkMessage queryResponseMsg;
                        
                        memset(&queryResponseMsg, 0, sizeof(queryResponseMsg));
                        queryResponseMsg.cmd = GAME_NETWORK_MSG_DIRECTORY_QUERY_LIST_AT_OFFSET_RESPONSE;
                        struct sockaddr_in* sin_ptr = (struct sockaddr_in*) pInfo->address.storage;
                        if(inet_ntoa(sin_ptr->sin_addr))
                        {
                            console_write("QUERY: found game %s at\n%s\noffset:%d(srcAddr=%s)",
                                          pInfo->name, inet_ntoa(sin_ptr->sin_addr), offset_o,
                                          inet_ntop(AF_INET6, &(GAME_NETWORK_ADDRESS_INADDR(srcAddr)), msgBuf, sizeof(msgBuf)));
                            /*
                            strcpy(queryResponseMsg.params.c, inet_ntoa(sin_ptr->sin_addr));
                             */
                            strcpy(queryResponseMsg.params.c, pInfo->name);
                        }
                        send_to_address_udp(&queryResponseMsg, srcAddr);
                    }
                    break;
                }
                
                if(!pInfo)
                {
                    // respond that no game exists at this offset
                    gameNetworkMessage queryResponseMsg;
                    
                    memset(&queryResponseMsg, 0, sizeof(queryResponseMsg));
                    queryResponseMsg.cmd = GAME_NETWORK_MSG_DIRECTORY_QUERY_LIST_AT_OFFSET_RESPONSE;
                    //send_to_address_udp(&queryResponseMsg, srcAddr);
                }
                
                offset--;
                pInfo = pInfo->next;
            }
            break;
            
        case GAME_NETWORK_MSG_DIRECTORY_QUERY_LIST_AT_OFFSET_RESPONSE:
            // add portal
            msg->params.c[sizeof(msg->params.c)-1] = '\0';
            game_add_spawnpoint(rand_in_range(1, gWorld->bound_x),
                                rand_in_range(1, gWorld->bound_y),
                                rand_in_range(1, gWorld->bound_z),
                                msg->params.c);
            
            world_object_set_nametag(world_get_last_object()->elem_id, msg->params.c);
            
            // query for next game in list
            console_append("\nadded portal %d to %s",
                           gameNetworkState.gameDirectory.query_offset,
                           msg->params.c);
            gameNetworkState.gameDirectory.query_offset++;
            gameNetwork_directoryListOffset(gameNetworkState.gameDirectory.query_offset);
            break;
            
        case GAME_NETWORK_MSG_DIRECTORY_RESPONSE:
            // this is going to happen inside gameNetwork_connect()            
            break;
            
        case GAME_NETWORK_MSG_DIRECTORY_REMOVE:
            break;
            
        default:
            break;
    }
}

void
do_game_network_handle_msg(gameNetworkMessage* msg, gameNetworkAddress* srcAddr)
{
    gameNetworkPlayerInfo* playerInfo, *playerInfoFound, *playerInfoTarget;
    WorldElemListNode* pNode;
    gameNetworkObjectInfo* objectInfo;
    game_timeval_t t;
    int net_obj_id;
    int interp_velo = 1;
    game_timeval_t network_time_ms = get_time_ms();
    
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
        if(strncmp(msg->params.c, gameNetworkState.hostInfo.name, sizeof(msg->params.c)) == 0)
        {
            msg->cmd = GAME_NETWORK_MSG_BEACON_RESP;
            msg->player_id = gameNetworkState.my_player_id;
            //send_lan_broadcast(&msg);
            send_to_address_udp(msg, srcAddr);
        }
        return;
    }
    
    // find player info
    // TODO: check game-id matches
    if(gameNetwork_getPlayerInfo(msg->player_id, &playerInfo, 0) == GAME_NETWORK_ERR_FAIL)
    {
        int add_player = 0;
        int add_name = 0;
        gameNetworkMessage msgOut;
        
        if(gameNetworkState.hostInfo.hosting && msg->cmd == GAME_NETWORK_MSG_CONNECT)
        {
            add_player = 1;
            add_name = 1;
        }
        else if(msg->cmd == GAME_NETWORK_MSG_PLAYER_INFO)
        {
            add_player = 1;
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
            memset(&msgOut, 0, sizeof(msgOut));
            msgOut.cmd = GAME_NETWORK_MSG_CONNECT;
            msgOut.player_id = gameNetworkState.my_player_id;
            strncpy(msgOut.params.c, gameNetworkState.my_player_name, GAME_NETWORK_MAX_STRING_LEN);
            gameNetwork_send(&msgOut);
        }
        else
        {
            return;
        }
        
        interp_velo = 0;
    }
    
    while(1)
    {
        int object_target = WORLD_ELEM_ID_INVALID;
        
        switch(msg->cmd)
        {
            case GAME_NETWORK_MSG_CONNECT:
                strncpy(playerInfo->name, msg->params.c, sizeof(msg->params.c));
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
                    
                    t = msg->params.f[16];
                    pNode = world_elem_list_find(playerInfo->elem_id, &gWorld->elements_list);
                    
                    if(pNode)
                    {
                        // blarg
                        /*
                        if(pNode->elem->type != msg->params.f[0] ||
                           pNode->elem->object_type != msg->params.f[15])
                        {
                            world_remove_object(pNode->elem->elem_id);
                            
                            pNode->elem->elem_id = WORLD_ELEM_ID_INVALID;
                        }
                         */
                    }
                    
                    if(!pNode ||
                       playerInfo->elem_id < 0)
                    {
                        //respawn
                        playerInfo->elem_id = world_add_object(msg->params.f[0], 0, 0, 0, 0, 0, 0,
                                                               msg->params.f[12], msg->params.f[13]);
                        pNode = world_elem_list_find(playerInfo->elem_id, &gWorld->elements_list);
                        
                        pNode->elem->object_type = OBJ_PLAYER;
                        pNode->elem->durability = DURABILITY_PLAYER;
                        pNode->elem->stuff.affiliation = playerInfo->player_id;
                        pNode->elem->stuff.game_object_id = playerInfo->player_id;
                        pNode->elem->stuff.network_created = 1;
                        world_object_set_nametag(playerInfo->elem_id, playerInfo->name);
                        interp_velo = 0;
                    }
                    
                    pPlayerElem = pNode->elem;
                    
                    float velDetected[3] =
                    {
                        (msg->params.f[1] - pPlayerElem->physics.ptr->x) * (1000.0 / (t - playerInfo->timestamp_last[0])),
                        (msg->params.f[2] - pPlayerElem->physics.ptr->y) * (1000.0 / (t - playerInfo->timestamp_last[0])),
                        (msg->params.f[3] - pPlayerElem->physics.ptr->z) * (1000.0 / (t - playerInfo->timestamp_last[0]))
                    };
                    
                    // log location
                    for(int i = 2; i > 0; i--)
                    {
                        for(int d = 0; d < 3; d++)
                        {
                            playerInfo->loc_last[d][i] = playerInfo->loc_last[d][i-1];
                            playerInfo->euler_last[d][i] = playerInfo->euler_last[d][i-1];
                        }
                        playerInfo->timestamp_last[i] = playerInfo->timestamp_last[i-1];
                    }
                    playerInfo->loc_last[0][0] = msg->params.f[1];
                    playerInfo->loc_last[1][0] = msg->params.f[2];
                    playerInfo->loc_last[2][0] = msg->params.f[3];
                    playerInfo->euler_last[0][0] = msg->params.f[4];
                    playerInfo->euler_last[1][0] = msg->params.f[5];
                    playerInfo->euler_last[2][0] = msg->params.f[6];
                    playerInfo->timestamp_last[0] = t;
                    
                    float v[3] = {
                        pPlayerElem->physics.ptr->x,
                        pPlayerElem->physics.ptr->y,
                        pPlayerElem->physics.ptr->z
                    };
                    float time_win = playerInfo->timestamp_last[0] - playerInfo->timestamp_last[2];
                    if(interp_velo && time_win < update_frequency_ms * velo_interp_update_mult && time_win > 0)
                    {
                        // predict velocity
                        float v1[3];
                        for(int d = 0; d < 3; d++)
                        {
                            v1[d] = predict_a1_at_b1_for_a_over_b(playerInfo->loc_last[d],
                                                                  playerInfo->timestamp_last,
                                                                  t + 1000);
                            v[d] = v1[d] - v[d];
                        }
                        
                        // calculate delta between predicted vel and sent-velocity
                        playerInfo->vel_predict_delta = pPlayerElem->physics.ptr->velocity -
                            sqrt(msg->params.f[7]*msg->params.f[7] +
                                 msg->params.f[8]*msg->params.f[8] +
                                 msg->params.f[9]*msg->params.f[9]);
                    }
                    else
                    {
                        v[0] = pPlayerElem->physics.ptr->vx;
                        v[1] = pPlayerElem->physics.ptr->vy;
                        v[2] = pPlayerElem->physics.ptr->vz;
                    }
                    
                    gameNetwork_updatePlayerObject(playerInfo,
                                                   msg->params.f[1], msg->params.f[2], msg->params.f[3],
                                                   msg->params.f[4], msg->params.f[5], msg->params.f[6],
                                                   v[0], v[1], v[2]);

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
                world_add_object(msg->params.f[0],
                                 msg->params.f[1], msg->params.f[2], msg->params.f[3],
                                 msg->params.f[4], msg->params.f[5], msg->params.f[6],
                                 msg->params.f[12], msg->params.f[13]);
                
                //world_get_last_object()->stuff.player.player_id = playerInfo->player_id;
                world_get_last_object()->stuff.affiliation = playerInfo->player_id;
                world_get_last_object()->object_type = msg->params.f[15];
                
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
                    }
                    
                    objectInfo->update_time = network_time_ms;
                }
                break;
                
            case GAME_NETWORK_MSG_ALERT_BEGIN:
                console_clear();
                msg->params.c[sizeof(msg->params.c)-1] = '\0';
                console_write("%s:\n%s", playerInfo->name, msg->params.c);
                break;
                
            case GAME_NETWORK_MSG_ALERT_CONTINUE:
                msg->params.c[sizeof(msg->params.c)-1] = '\0';
                console_append(msg->params.c);
                break;
                
            case GAME_NETWORK_MSG_HITME:
                break;
                
            case GAME_NETWORK_MSG_KILLEDME:
                if(gameNetworkState.hostInfo.hosting)
                {
                    char deathMsg[255];
                    
                    playerInfo->stats.killed++;
                    
                    if(gameNetwork_getPlayerInfo(msg->params.i[0], &playerInfoFound, 0) == GAME_NETWORK_ERR_NONE)
                    {
                        playerInfoFound->stats.killer++;
                        
                        // choose a random "death-verb" ;-)
                        int v = 0;
                        v = rand() % (sizeof(gameKillVerbs) / sizeof(char*) );
                        
                        sprintf(deathMsg, "\n%s %s %s (%s)\n", playerInfoFound->name, gameKillVerbs[v],
                                playerInfo->name,
                                msg->params.i[1] == OBJ_MISSLE? "with a missle": "with a laser");
                    }
                    else
                    {
                        sprintf(deathMsg, "\n%s was killed\n", playerInfo->name);
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
                
            case GAME_NETWORK_MSG_GET_MAP:
                {
                    gameNetworkMessage msgOut;
                    read_in_render_thread = 1;
                    char* map_data = do_game_map_render();
                    read_in_render_thread = 0;
                    
                    memset(&msgOut, 0, sizeof(msgOut));
                    msgOut.cmd = GAME_NETWORK_MSG_GET_MAP_BEGIN;
                    msgOut.player_id = gameNetworkState.my_player_id;
                    send_to_address_udp(&msgOut, srcAddr);
                    
                    if(map_data)
                    {
                        char* ptr = map_data;
                        char* ptr_end = map_data + strlen(map_data);
                        
                        do
                        {
                            msgOut.cmd = GAME_NETWORK_MSG_GET_MAP_SOME;
                            msgOut.player_id = gameNetworkState.my_player_id;
                            memset(msgOut.params.mapData.s, 0, sizeof(msgOut.params.mapData.s));
                            strncpy(msgOut.params.mapData.s, ptr, GAME_NETWORK_MAX_MAP_STRING_LEN);
                            send_to_address_udp(&msgOut, srcAddr);
                            ptr += GAME_NETWORK_MAX_MAP_STRING_LEN;
                        } while(ptr < ptr_end && strlen(ptr) > 0);
                    }
                    
                    msgOut.cmd = GAME_NETWORK_MSG_GET_MAP_END;
                    send_to_address_udp(&msgOut, srcAddr);
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
    if(!read_in_render_thread)
    {
        world_lock();
    }
    
    char* map = gameMapReadRendered();
    
    if(!read_in_render_thread)
    {
        world_unlock();
    }
    
    return map;
}
    
void
do_game_network_read_core()
{
    gameNetworkMessage msg;
    gameNetworkAddress srcAddr;
    int receive_block_ms = 1;
    int retries = 100;
    
    if(!gameNetworkState.connected /*|| !world_inited*/)
    {
        return;
    }
    
    if(gameNetworkState.hostInfo.bonjour_lan)
    {
        retries = 1;
        goto do_game_network_read_queue;
    }
    
    if(!read_in_render_thread)
    {
        receive_block_ms = 5;
        retries = 10;
    }
    
    while(retries > 0)
    {
        if(gameNetwork_receive(&msg, &srcAddr, receive_block_ms) == GAME_NETWORK_ERR_FAIL)
        {
            if(gameNetworkState.hostInfo.hosting)
            {
                gameNetwork_accept_map_download(&srcAddr, do_game_map_render);
                
                gameNetwork_accept_stream_connection(&srcAddr);
            }
            
            break;
        }
        
        if(msg.cmd == GAME_NETWORK_MSG_BEACON && gameNetworkState.hostInfo.hosting)
        {
            if(strncmp(msg.params.c, gameNetworkState.hostInfo.name, sizeof(msg.params.c)) == 0)
            {
                msg.cmd = GAME_NETWORK_MSG_BEACON_RESP;
                msg.player_id = gameNetworkState.my_player_id;
                //send_lan_broadcast(&msg);
                send_to_address_udp(&msg, &srcAddr);
            }
            retries--;
            continue;
        }
        else if(msg.cmd >= GAME_NETWORK_MSG_DIRECTORY_ADD && msg.cmd <= GAME_NETWORK_MSG_DIRECTORY_RESPONSE)
        {
            if(!read_in_render_thread)
            {
                world_lock();
            }
            
            do_game_network_handle_directory_msg(&msg, &srcAddr);
            
            if(!read_in_render_thread)
            {
                world_unlock();
            }
            
            retries--;
            continue;
        }
        
    do_game_network_read_queue:
        // clean up processed messages
        gameNetworkState.msgQueue.cleanup = 1;
        if(!read_in_render_thread)
        {
            while(!gameNetworkState.msgQueue.cleanupWaiting) { int i; i++; }
        }
        
        gameNetworkMessageQueued* pMsgCur = &gameNetworkState.msgQueue.head;
        gameNetworkMessageQueued* pMsgProcessed = NULL;
        while(pMsgCur->next && pMsgCur->next->processed)
        {
            if(!pMsgProcessed) pMsgProcessed = pMsgCur->next;
            pMsgCur->next = pMsgCur->next->next;
        }
        while(pMsgProcessed && pMsgProcessed->processed)
        {
            gameNetworkMessageQueued* pMsgFree = pMsgProcessed;
            pMsgProcessed = pMsgProcessed->next;
            pMsgFree->next = NULL;
            free(pMsgFree);
        }
        
        // add to queue
        gameNetworkMessageQueued* pMsg =
            (gameNetworkMessageQueued*) malloc(sizeof(gameNetworkMessageQueued));
        if(pMsg)
        {
            pMsg->next = NULL;
            pMsg->msg = msg;
            pMsg->srcAddr = srcAddr;
            pMsg->processed = 0;
            
            gameNetworkMessageQueued* pTail = &gameNetworkState.msgQueue.head;
            while(pTail->next) pTail = pTail->next;
            pTail->next = pMsg;
        }
        gameNetworkState.msgQueue.cleanup = 0;
        gameNetworkState.msgQueue.cleanupWaiting = 0;
        
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
    if(!gameNetworkState.hostInfo.bonjour_lan)
    {
        read_in_render_thread = 0;
        do_game_network_read_core();
    }
}

void
do_game_network_read_bonjour()
{
    if(gameNetworkState.hostInfo.bonjour_lan)
    {
        read_in_render_thread = 1;
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
        do_game_network_handle_msg(&msg, &fakeAddress);
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
        do_game_network_handle_msg(&msg, &fakeAddress);
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
    char str[1024];
    char tmp[128];
    char nameBuf[32];
    static int time_remaining_last = 1;
    int clear_stats = 0;
    const char* decor = "^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L^L\n";
    
    if(game_time_remaining() <= 0 && time_remaining_last > 0)
    {
        clear_stats = 1;
    }
    time_remaining_last = game_time_remaining();
    
    sprintf(str, "Game stats:\n"
            "%s"
            "Time Remaining:%.0f\n"
            "NAME            : KILL^N:   DEATH^M:   SHOTS:   PING:   POINTS:  \n", decor, game_time_remaining());
    
    pInfo = gameNetworkState.player_list_head.next;
    while(pInfo && strlen(str) < sizeof(str)-64)
    {
        char *statsPrefix = clear_stats? "ALERT:": "";
        if(clear_stats) statsPrefix = "***FINAL SCORE***\n";
        
        strncpy(nameBuf, pInfo->name, 16);
        while(strlen(nameBuf) < 16) strcat(nameBuf, " ");
        
        sprintf(tmp, "%s%s: %02d       %02d        %04d     %02.0f      %02d\n",
                statsPrefix,
                nameBuf,
                pInfo->stats.killer,
                pInfo->stats.killed,
                pInfo->stats.shots_fired,
                pInfo->network_latency,
                pInfo->stats.points);
        strcat(str, tmp);
        
        if(clear_stats)
        {
            pInfo->stats.killed = pInfo->stats.killer = pInfo->stats.shots_fired =
            pInfo->stats.points = pInfo->stats.score_calculated = 0;
        }
        
        pInfo = pInfo->next;
    }
    strcat(str, decor);
    
    gameNetwork_alert(str);
    
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
    
    pInfo = gameNetworkState.player_list_head.next;
    while(pInfo)
    {
        pInfo->time_ping = time_ms;
        pInfo = pInfo->next;
    }
    
    gameNetwork_send(&msg);
}

int
gameNetwork_directoryList()
{
    gameNetwork_getDNSAddress(gameNetworkState.gameDirectory.directory_name,
                              &gameNetworkState.gameDirectory.directory_address);
    
    gameNetworkState.connected = 1;
    
    gameNetworkState.gameDirectory.query_offset = 0;
    gameNetwork_directoryListOffset(gameNetworkState.gameDirectory.query_offset);
    
    return GAME_NETWORK_ERR_NONE;
}

static int
gameNetwork_directoryListOffset(int offset)
{
    gameNetworkMessage msg;
    
    memset(&msg, 0, sizeof(msg));
    msg.cmd = GAME_NETWORK_MSG_DIRECTORY_QUERY_LIST_AT_OFFSET;
    msg.params.f[1] = offset;
    
    send_to_address_udp(&msg, &gameNetworkState.gameDirectory.directory_address);
    
    return GAME_NETWORK_ERR_NONE;
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
    
    if(collision_action != COLLISION_ACTION_DAMAGE) return;
    
    if(gameNetworkState.hostInfo.hosting)
    {
        if(elemB->object_type == OBJ_POWERUP_GENERIC)
        {
            gameNetwork_getPlayerInfo(elemA->stuff.affiliation, &pInfo, 0);
            
            if(pInfo)
            {
                pInfo->stats.points += 100;
            }
        }
    }
    
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
    
    if(gameNetworkState.hostInfo.hosting)
    {
        
    }
    
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
    if(msg->cmd == GAME_NETWORK_MSG_GET_MAP_BEGIN)
    {
        if(gameNetworkState.server_map_data)
        {
            free(gameNetworkState.server_map_data);
        }
        
        gameNetworkState.server_map_data = malloc(1024);
        memset(gameNetworkState.server_map_data, 0, 1024);
        
        return 1;
    }
    else if(msg->cmd == GAME_NETWORK_MSG_GET_MAP_END && gameNetworkState.server_map_data)
    {
        gameNetworkState.gameNetworkHookOnMessage = NULL;
        
        gameMapSetMap(gameNetworkState.server_map_data);
        
        return 1;
    }
    else if(msg->cmd == GAME_NETWORK_MSG_GET_MAP_SOME && gameNetworkState.server_map_data)
    {
        char* catbuf = malloc(strlen(gameNetworkState.server_map_data) + GAME_NETWORK_MAX_MAP_STRING_LEN + 1);
        strcpy(catbuf, gameNetworkState.server_map_data);
        strncat(catbuf, msg->params.mapData.s, GAME_NETWORK_MAX_MAP_STRING_LEN);
        free(gameNetworkState.server_map_data);
        gameNetworkState.server_map_data = catbuf;
        return 1;
    }

    return 0;
}
    
int gameNetwork_onBonjourConnecting2(gameNetworkMessage* msg, gameNetworkAddress* srcAddr)
{
    gameNetworkMessage downloadMsg;
    gameNetworkPlayerInfo *playerInfo;
    
    if(msg->cmd == GAME_NETWORK_MSG_CONNECT)
    {
        gameNetworkState.gameNetworkHookOnMessage = gameNetwork_onBonjourConnecting3;
        
        memset(&downloadMsg, 0, sizeof(downloadMsg));
        downloadMsg.cmd = GAME_NETWORK_MSG_GET_MAP;
        downloadMsg.player_id = gameNetworkState.my_player_id;
        send_to_address_udp(&downloadMsg, srcAddr);
        
        if(gameNetwork_getPlayerInfo(msg->player_id, &playerInfo, 1) == GAME_NETWORK_ERR_NONE)
        {
            memcpy(&playerInfo->address, srcAddr, sizeof(gameNetworkAddress));
            return 1;
        }
        
        return 1;
    }
    
    return 0;
}
    
int gameNetwork_onBonjourConnecting1(gameNetworkMessage* msg, gameNetworkAddress* srcAddr)
{
    gameNetworkMessage connectMsg;
    
    if(msg->cmd == GAME_NETWORK_MSG_BEACON_RESP)
    {
        gameNetworkState.gameNetworkHookOnMessage = gameNetwork_onBonjourConnecting2;
        
        memset(&connectMsg, 0, sizeof(connectMsg));
        connectMsg.cmd = GAME_NETWORK_MSG_CONNECT;
        connectMsg.player_id = gameNetworkState.my_player_id;
        strncpy(connectMsg.params.c, gameNetworkState.my_player_name, sizeof(connectMsg.params.c));
        send_to_address_udp(&connectMsg, srcAddr);
        
        return 1;
    }
    return 0;
}
    
#ifdef __cplusplus
}
#endif
