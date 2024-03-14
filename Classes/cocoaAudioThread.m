//
//  cocoaAudioThread.m
//  gl_flight
//
//  Created by jbrady on 1/1/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#import "cocoaAudioThread.h"
#import "cocoaGameAudio.h"
#include "gameAudio.h"

@implementation cocoaAudioThread

cocoaGameAudio* c;

-(cocoaAudioThread*) init
{
    c = [[cocoaGameAudio alloc] init];
    
    return self;
}

-(void) dealloc
{
    [c release];
    [super dealloc];
}

-(void) run
{
    while(1)
    {
        [c processMessages: 4];
        usleep(GAME_AUDIO_QUEUE_READ_INTERVAL_MS * 1000);
    }
}

@end
