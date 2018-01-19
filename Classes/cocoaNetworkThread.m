//
//  cocoaNetworkThread.m
//  gl_flight
//
//  Created by jbrady on 12/23/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "cocoaNetworkThread.h"
#import "gameGlobals.h"
#import "world.h"
#import "gameNetwork.h"
#import "GameNetworkBonjourManager.h"

@implementation cocoaNetworkThread

-(cocoaNetworkThread*) init
{
    return self;
}

-(void) run
{
    while(!gameNetworkState.inited)
    {
        sleep(1);
    }
    
    while(1)
    {
        world_lock();
        
        //do_game_network_read();
        do_game_network_write();
        
        world_unlock();
        usleep(2000);
    }
}

@end
