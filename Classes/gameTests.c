//
//  gameTests.c
//  gl_flight
//
//  Created by Justin Brady on 7/7/19.
//

#include <stdio.h>
#include <assert.h>
#include "gameNetwork.h"
#include "gameNetworkByteOrder.h"

int
run_network_sanity()
{
    int original_val = 65535;
    
    gameNetworkMessage msg;
    
    msg.cmd = GAME_NETWORK_MSG_HITME;
    
    msg.params.i[0] = original_val;
    
    msg.rebroadcasted = 1;
    gameMessage_to_nbo(&msg);
    assert(msg.rebroadcasted != 0);
    if(msg.params.i[0] == original_val) return -1;
    gameMessage_from_nbo(&msg);
    assert(msg.rebroadcasted != 0);
    
    if(msg.cmd != GAME_NETWORK_MSG_HITME || msg.params.i[0] != original_val) return -1;
    
    return 0;
}

int
gameTests_run_sanity()
{
    int r = 0;
    
    r = run_network_sanity();
    if(r != 0) return r;
    
    return 0;
}
