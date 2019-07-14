//
//  gameNetworkByteOrder.h
//  gl_flight
//
//  Created by Justin Brady on 7/7/19.
//

#ifndef gameNetworkByteOrder_h
#define gameNetworkByteOrder_h

#include <assert.h>

static gameNetworkArgsType
gameMessage_args_for_type(int type)
{
    gameNetworkArgsType t;
    
    switch(type) {
        case GAME_NETWORK_MSG_GET_MAP_SOME:
        case GAME_NETWORK_MSG_GET_MAP_END:
        case GAME_NETWORK_MSG_GET_MAP_BEGIN:
        case GAME_NETWORK_MSG_GET_MAP_REQUEST:
            t = GAME_NETWORK_MSGARG_MAPINFO;
            break;
            
        case GAME_NETWORK_MSG_PLAYER_INFO:
        case GAME_NETWORK_MSG_ADD_OBJECT:
        case GAME_NETWORK_MSG_FIRE_BULLET:
        case GAME_NETWORK_MSG_FIRE_MISSLE:
        case GAME_NETWORK_MSG_REMOVE_OBJECT:
        case GAME_NETWORK_MSG_REPLACE_SERVER_OBJECT_WITH_ID:
        case GAME_NETWORK_MSG_REMOVE_SERVER_OBJECT_WITH_ID:
        case GAME_NETWORK_MSG_TOW_DROP:
        case GAME_NETWORK_MSG_STARTGAME:
            t = GAME_NETWORK_MSGARG_FLOATARRAY;
            break;
            
        case GAME_NETWORK_MSG_PLAYER_STATUS:
            t = GAME_NETWORK_MSGARG_PLAYERINFO;
            break;
            
        case GAME_NETWORK_MSG_HITME:
            t = GAME_NETWORK_MSGARG_INTARRAY;
            break;
        
        case GAME_NETWORK_MSG_CONNECT:
        default:
            t = GAME_NETWORK_MSGARG_CHAR;
            break;
    };
    
    return t;
}

static void
gameMessage_to_nbo(gameNetworkMessage* msg)
{
    int i;
    
    gameNetworkArgsType t = gameMessage_args_for_type(msg->cmd);
    
    unsigned int* nbo_long[] = {
        &msg->vers,
        &msg->game_id,
        &msg->cmd,
        &msg->player_id,
        &msg->elem_id_net,
        &msg->msg_seq
    };
    
    assert(!msg->is_nbo);
    msg->is_nbo = 1;
    
    for(i = 0; i < sizeof(nbo_long)/sizeof(unsigned int*); i++)
    {
        *nbo_long[i] = htonl(*nbo_long[i]);
    }
    
    switch(t) {
        case GAME_NETWORK_MSGARG_CHAR:
            break;
            
        case GAME_NETWORK_MSGARG_DIRECTORY:
            break;
            
        case GAME_NETWORK_MSGARG_PLAYERINFO:
            msg->params.playerStatus.is_towing = htonl(msg->params.playerStatus.is_towing);
            break;
            
        case GAME_NETWORK_MSGARG_MAPINFO:
            msg->params.mapData.offset = htonll(msg->params.mapData.offset);
            break;
            
        case GAME_NETWORK_MSGARG_FLOATARRAY:
            break;
            
        case GAME_NETWORK_MSGARG_INTARRAY:
            for(i = 0; i < sizeof(msg->params.i)/sizeof(int); i++) msg->params.i[i] = htonl(msg->params.i[i]);
            break;
    }
}

static void
gameMessage_from_nbo(gameNetworkMessage* msg)
{
    int i;
    
    unsigned int* nbo_long[] = {
        &msg->vers,
        &msg->game_id,
        &msg->cmd,
        &msg->player_id,
        &msg->elem_id_net,
        &msg->msg_seq
    };
    
    assert(msg->is_nbo);
    msg->is_nbo = 0;
    
    for(i = 0; i < sizeof(nbo_long)/sizeof(unsigned int*); i++)
    {
        *nbo_long[i] = ntohl(*nbo_long[i]);
    }
    
    gameNetworkArgsType t = gameMessage_args_for_type(msg->cmd);
    
    switch(t) {
        case GAME_NETWORK_MSGARG_CHAR:
            break;
            
        case GAME_NETWORK_MSGARG_DIRECTORY:
            break;
            
        case GAME_NETWORK_MSGARG_PLAYERINFO:
            msg->params.playerStatus.is_towing = ntohl(msg->params.playerStatus.is_towing);
            break;
            
        case GAME_NETWORK_MSGARG_MAPINFO:
            msg->params.mapData.offset = ntohll(msg->params.mapData.offset);
            break;
            
        case GAME_NETWORK_MSGARG_FLOATARRAY:
            break;
            
        case GAME_NETWORK_MSGARG_INTARRAY:
            for(i = 0; i < sizeof(msg->params.i)/sizeof(int); i++) msg->params.i[i] = ntohl(msg->params.i[i]);
            break;
    }
}

#endif /* gameNetworkByteOrder_h */
