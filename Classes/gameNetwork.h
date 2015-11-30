//
//  gameNetwork.h
//  gl_flight
//
//  Created by jbrady on 11/21/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef gl_flight_gameNetwork_h
#define gl_flight_gameNetwork_h

#include "gameUtils.h"

#define GAME_NETWORK_MAX_PLAYERS 64
#define GAME_NETWORK_MAX_STRING_LEN 32
#define GAME_NETWORK_PORT 52000
#define GAME_NETWORK_PORT_STR "52000"
#define GAME_NETWORK_PLAYER_ID_HOST 16000
#define GAME_NETWORK_PLAYER_ID_MAX (GAME_NETWORK_PLAYER_ID_HOST-1)
#define GAME_NETWORK_OBJECT_ID_MIN (GAME_NETWORK_PLAYER_ID_HOST+1)
#define GAME_NETWORK_STRING_FMT_KILLED "killed: %s -> %s"
#define GAME_NETWORK_ADDRESS_FAMILY(x) (((struct sockaddr_in*) (x)->storage)->sin_family)
#define GAME_NETWORK_ADDRESS_INADDR(x) (((struct sockaddr_in*) (x)->storage)->sin_addr)
#define GAME_NETWORK_ADDRESS_PORT(x) (((struct sockaddr_in*) (x)->storage)->sin_port)
#define GAME_NETWORK_ADDRESS_LEN_CORRECT(x) ((x)->len == sizeof(struct sockaddr_in))

const static char *GAME_NETWORK_LAN_GAME_NAME = "d0gf1ght_lan";

typedef enum
{
    GAME_NETWORK_ERR_NONE = 0,
    GAME_NETWORK_ERR_FAIL = -1,
} gameNetworkError;

typedef enum
{
    GAME_NETWORK_MSG_NONE = 0,
    GAME_NETWORK_MSG_BEACON = 3,
    GAME_NETWORK_MSG_BEACON_RESP,
    GAME_NETWORK_MSG_PING,
    GAME_NETWORK_MSG_PONG,
    GAME_NETWORK_MSG_CONNECT,
    GAME_NETWORK_MSG_DISCONNECT,
    GAME_NETWORK_MSG_GET_MAP,
    GAME_NETWORK_MSG_PLAYER_INFO,
    GAME_NETWORK_MSG_ADD_OBJECT,
    GAME_NETWORK_MSG_FIRE_BULLET,
    GAME_NETWORK_MSG_FIRE_MISSLE,
    GAME_NETWORK_MSG_REMOVE_OBJECT,
    GAME_NETWORK_MSG_KILLED,
    GAME_NETWORK_MSG_REPLACE_SERVER_OBJECT_WITH_ID,
    GAME_NETWORK_MSG_REMOVE_SERVER_OBJECT_WITH_ID,
    GAME_NETWORK_MSG_ALERT_BEGIN,
    GAME_NETWORK_MSG_ALERT_CONTINUE,
    GAME_NETWORK_MSG_KILLEDME,
    GAME_NETWORK_MSG_MYINFO,
    GAME_NETWORK_MSG_TOW_DROP,
    GAME_NETWORK_MSG_PLAYER_STATUS,
    GAME_NETWORK_MSG_HITME,
    
    GAME_NETWORK_MSG_DIRECTORY_ADD,
    GAME_NETWORK_MSG_DIRECTORY_REMOVE,
    GAME_NETWORK_MSG_DIRECTORY_QUERY,
    GAME_NETWORK_MSG_DIRECTORY_QUERY_RANDOM,
    GAME_NETWORK_MSG_DIRECTORY_QUERY_LIST_AT_OFFSET,
    GAME_NETWORK_MSG_DIRECTORY_QUERY_LIST_AT_OFFSET_RESPONSE,
    GAME_NETWORK_MSG_DIRECTORY_RESPONSE,
    GAME_NETWORK_MSG_LAST
} gameNetworkMsgType;

typedef struct
{
    unsigned char storage[32];
    int len;
} gameNetworkAddress;

typedef struct
{
    unsigned int vers;
    unsigned int game_id;
    unsigned int cmd;
    unsigned int player_id;
    unsigned int rebroadcasted:1;
    unsigned int needs_stream:1;
    unsigned int needs_ack:1;
    unsigned int is_ack:1;
    unsigned int elem_id_net;
    unsigned int msg_seq;
    game_timeval_t timestamp;
    union 
    {
        float f[24];
        char c[GAME_NETWORK_MAX_STRING_LEN];
        unsigned int i[16];
        
        struct {
            char c[GAME_NETWORK_MAX_STRING_LEN];
            gameNetworkAddress addr;
        } directoryInfo;
        
        struct {
            int is_towing;
            float ping_ms;
        } playerStatus;
        
    } params;
} gameNetworkMessage;

struct gameNetworkMessageQueued {
    struct gameNetworkMessageQueued* next;
    gameNetworkMessage msg;
    gameNetworkAddress srcAddr;
    int processed;
};
typedef struct gameNetworkMessageQueued gameNetworkMessageQueued;

typedef struct
{
    int s;
} gameNetworkSocket;

typedef int gameNetworkPlayerID;

struct gameNetworkPlayerInfo
{
    gameNetworkPlayerID player_id;
    int elem_id;
    char name[GAME_NETWORK_MAX_STRING_LEN];
    gameNetworkAddress address;
    gameNetworkSocket stream_socket;
    game_timeval_t time_last_update;
    game_timeval_t timestamp_last[3];
    game_timeval_t time_ping;
    game_timeval_t network_latency;
    game_timeval_t time_status_last;
    game_timeval_t time_cube_pooped_last;
    
    struct
    {
        int killed;
        int killer;
        int shots_fired;
        int points;
        int score_calculated;
    } stats;
    
    float loc_last[3][3];
    float vel_predict_delta;
    
    int shot_fired;
    
    struct gameNetworkPlayerInfo* next;
};

struct gameNetworkObjectInfo
{
    struct gameNetworkObjectInfo* next;
    
    int elem_id_net;
    int elem_id_local;
    float loc_last[3][3];
    float timestamp_last[3];
    game_timeval_t update_time, send_update_time;
};
typedef struct gameNetworkObjectInfo gameNetworkObjectInfo;

typedef struct gameNetworkPlayerInfo gameNetworkPlayerInfo;

typedef struct
{
    gameNetworkPlayerInfo player_list_head;
    
    gameNetworkPlayerID my_player_id;
    char my_player_name[128];
    
    int broadcast_mode;
    struct
    {
        int hosting;
        int lan_only;
        char name[GAME_NETWORK_MAX_STRING_LEN];
        gameNetworkSocket socket;
        gameNetworkSocket map_socket;
        gameNetworkSocket stream_socket;
        gameNetworkAddress addr;
        //gameNetworkMessage retransmitMsg;
        //int retransmitAcks;
        unsigned short port;
        gameNetworkAddress local_inet_addr;
    } hostInfo;
    
    gameNetworkObjectInfo game_object_list_head;
    int game_object_id_next;
    
    gameNetworkSocket server_socket;
    
    char *server_map_data;
    
    int inited;
    int connected;
    int map_downloaded;
    unsigned int server_msg_id_next;
    
    struct
    {
        gameNetworkPlayerInfo head;
        gameNetworkAddress directory_address;
        char directory_name[128];
        int query_offset;
    } gameDirectory;
    
    struct
    {
        int affiliation_hit_last;
        int obj_type_hit_last;
    } collision;

    struct
    {
        gameNetworkMessageQueued head;
    } msgQueue;
    
    game_timeval_t time_last_periodic_check;
    
    gameNetworkMessage msgUnacked;
    int msgUnackedRetransmitCount;
    
    unsigned int msg_seq_next;
    
    unsigned int msg_seq_acked_last;
    
} gameNetworkState_t;

extern gameNetworkState_t gameNetworkState;

gameNetworkError
gameNetwork_init(int broadcast_mode,
                 const char* server_name,
                 const char* player_name,
                 int update_ms,
                 const char* directory_name,
                 unsigned short host_port,
                 const char* local_inet_addr);

gameNetworkError
gameNetwork_resume();

int
gameNetwork_connect(char* server_name, int host, int lan_only);

void
gameNetwork_worldInit();

int
gameNetwork_directoryList();

void
gameNetwork_disconnect();

int
gameNetwork_countPlayers();

void
gameNetwork_startGame(unsigned int sec);

gameNetworkError
gameNetwork_send(gameNetworkMessage* msg);

gameNetworkError
gameNetwork_receive(gameNetworkMessage* msg_out, gameNetworkAddress* src_addr, unsigned int timeout_ms);

gameNetworkError
gameNetwork_getPlayerInfo(int player_id, gameNetworkPlayerInfo**, int add_if_not_found);

gameNetworkError
gameNetwork_getPlayerInfoForElemID(int player_elem_id, gameNetworkPlayerInfo**);

gameNetworkError
gameNetwork_addPlayerInfo(int player_id);

gameNetworkError
gameNetwork_removePlayer(int player_id);

void
gameNetwork_alert(char* msg);

void
do_game_network_world_update();

void
do_game_network_write();

void
do_game_network_read();

void
gameNetwork_sendKilledBy(gameNetworkPlayerID killer,
                         int object_type);

void
gameNetwork_sendStatsAlert();

void
gameNetwork_sendPing();

void
gameNetwork_handle_collision(WorldElem* elemA, WorldElem* elemB, int collision_action);

void
gameNetwork_handle_destruction(WorldElem* elem);

void
gameNetwork_action_handle(int action);

void
gameNetwork_lock();

void
gameNetwork_unlock();

void
do_game_network_handle_msg(gameNetworkMessage *msg, gameNetworkAddress *srcAddr);

#endif
