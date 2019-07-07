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
#include "gameLock.h"

#define GAME_NETWORK_MAX_PLAYERS 64
#define GAME_NETWORK_MAX_STRING_LEN 128
#define GAME_NETWORK_MAX_MAP_STRING_LEN 256
#define GAME_NETWORK_PORT 52000
#define GAME_NETWORK_PORT_BONJOUR 52010
#define GAME_NETWORK_PORT_STR "52000"
#define GAME_NETWORK_PLAYER_ID_HOST 16000
#define GAME_NETWORK_PLAYER_ID_MAX (GAME_NETWORK_PLAYER_ID_HOST-1)
#define GAME_NETWORK_OBJECT_ID_MIN (GAME_NETWORK_PLAYER_ID_HOST+1)
#define GAME_NETWORK_STRING_FMT_KILLED "killed: %s -> %s"
#define GAME_NETWORK_ADDRESS_FAMILY(x) (((struct sockaddr_in6*) (x)->storage)->sin6_family)
#define GAME_NETWORK_ADDRESS_INADDR(x) (((struct sockaddr_in6*) (x)->storage)->sin6_addr)
#define GAME_NETWORK_ADDRESS_PORT(x) (((struct sockaddr_in6*) (x)->storage)->sin6_port)
#define GAME_NETWORK_ADDRESS_LEN_CORRECT(x) ((x)->len == sizeof(struct sockaddr_in6))
#define GAME_NETWORK_ADDRESS_ISEQUAL(x, y) (memcmp((x)->storage, (y)->storage, (x)->len) == 0)
#define GAME_NETWORK_BONJOUR_ADDRFAMILY_HACK 0x09
#define GAME_NETWORK_MAP_REQUEST_WINDOW_LEN (GAME_NETWORK_MAX_MAP_STRING_LEN * 1)
#define GAME_NETWORK_HOST_PORTAL_NAME "*host*"
#define GAME_NETWORK_READ_THREAD_IDLE_USLEEP_INTERVAL (100000)
#define MESSAGES_SIGNAL_MAX (256)

//const static char *GAME_NETWORK_LAN_GAME_NAME = "d0gf1ght_lan";

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
    GAME_NETWORK_MSG_GET_MAP_REQUEST,
    GAME_NETWORK_MSG_GET_MAP_BEGIN,
    GAME_NETWORK_MSG_GET_MAP_SOME,
    GAME_NETWORK_MSG_GET_MAP_END,
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
    GAME_NETWORK_MSG_ALERT_END,
    GAME_NETWORK_MSG_KILLEDME,
    GAME_NETWORK_MSG_MYINFO,
    GAME_NETWORK_MSG_TOW_DROP,
    GAME_NETWORK_MSG_PLAYER_STATUS,
    GAME_NETWORK_MSG_HITME,
    
    //GAME_NETWORK_MSG_DIRECTORY_ADD, /* 29 */
    //GAME_NETWORK_MSG_DIRECTORY_REMOVE,
    //GAME_NETWORK_MSG_DIRECTORY_QUERY,
    //GAME_NETWORK_MSG_DIRECTORY_QUERY_RANDOM,
    //GAME_NETWORK_MSG_DIRECTORY_QUERY_LIST_AT_OFFSET,
    //GAME_NETWORK_MSG_DIRECTORY_QUERY_LIST_AT_OFFSET_RESPONSE,
    //GAME_NETWORK_MSG_DIRECTORY_RESPONSE,
    GAME_NETWORK_MSG_STARTGAME,
    GAME_NETWORK_MSG_ENDGAME,
    GAME_NETWORK_MSG_LAST
} gameNetworkMsgType;

typedef struct
{
    unsigned char storage[128];
    int len;
} gameNetworkAddress;

struct bonjour_addr_stuffed_in_sockaddr_in {
    uint8_t len;
    uint8_t sa_family;
    uint32_t peer_id;
};

typedef struct {
    float loc_last[3][3];
    float euler_last[3][3];
    float vel_predict_delta;
    
    game_timeval_t timestamp_adjust;
    game_timeval_t timestamp_last[3];
    game_timeval_t euler_last_interp;
} motion_interp_st;

typedef struct __attribute__((packed))
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
            unsigned long offset;
            char s[GAME_NETWORK_MAX_MAP_STRING_LEN];
        } mapData;
        
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
    game_timeval_t receive_time;
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
    game_timeval_t time_ping;
    game_timeval_t network_latency;
    game_timeval_t time_status_last;
    game_timeval_t time_cube_pooped_last;
    char* map_data;
    
    struct
    {
        int killed;
        int killer;
        int shots_fired;
        int points;
        int score_calculated;
    } stats;
    
    motion_interp_st motion;
    
    int shot_fired;
    
    struct gameNetworkPlayerInfo* next_connected;
    struct gameNetworkPlayerInfo* next;
};

struct gameNetworkObjectInfo
{
    struct gameNetworkObjectInfo* next;
    
    int elem_id_net;
    int elem_id_local;
    float loc_last[3][3];
    float timestamp_last[3];
    motion_interp_st motion;
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
        int bonjour_lan;
        char name[GAME_NETWORK_MAX_STRING_LEN];
        gameNetworkSocket socket;
        gameNetworkSocket map_socket;
        gameNetworkSocket stream_socket;
        gameNetworkSocket map_socket_sending;
        gameNetworkAddress addr;
        //gameNetworkMessage retransmitMsg;
        //int retransmitAcks;
        unsigned short port;
        gameNetworkAddress local_inet_addr;
        game_timeval_t time_last_stats_alert;
    } hostInfo;
    
    gameNetworkObjectInfo game_object_list_head;
    int game_object_id_next;
    
    gameNetworkSocket server_socket;
    
    char *server_map_data, *server_map_data_end;
    
    int inited;
    int connected;
    int connected_signal;
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
        game_lock_t lock;
    } msgQueue;
    
    game_timeval_t time_last_periodic_check;
    game_timeval_t time_last_name_send;
    game_timeval_t time_last_unacked_send;
    game_timeval_t time_game_remaining;
    
    gameNetworkMessage msgUnacked;
    int msgUnackedRetransmitCount;
    
    unsigned int msg_seq_next;
    
    unsigned int msg_seq_acked_last;
    
    int (*gameNetworkHookOnMessage)(gameNetworkMessage*, gameNetworkAddress*);
    
    void (*gameNetworkHookGameDiscovered)(char*);
    
    char gameStatsMessage[1024];
    
} gameNetworkState_t;

extern gameNetworkState_t gameNetworkState;

static void
gameNetwork_init_mem()
{
    gameNetworkState.inited = 0;
}

gameNetworkError
gameNetwork_init(int broadcast_mode,
                 const char* server_name,
                 const char* player_name,
                 int update_ms,
                 unsigned short host_port,
                 const char* local_inet_addr);

gameNetworkError
gameNetwork_resume();

int
gameNetwork_connect(char* server_name, void (*callback_becomehost)());

int
gameNetwork_host(char* server_name, void (*callback_becamehost)());

int
gameNetwork_eagerConnectInit();

void
gameNetwork_worldInit();

int
gameNetwork_directoryList();

int
gameNetwork_directoryRegister(const char* roomName);

void
gameNetwork_disconnectSignal();

void
gameNetwork_disconnect();

void
gameNetwork_sendPlayersDisconnect();

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

gameNetworkError
gameNetwork_removePlayerBonjour(int player_id);

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
do_game_network_handle_msg(gameNetworkMessage *msg, gameNetworkAddress *srcAddr, game_timeval_t received_at_time);

#endif
