//
//  cocoaGameAudio.h
//  gl_flight
//
//  Created by jbrady on 12/2/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AVFoundation/AVAudioPlayer.h"
#include "sounds.h"

#define GAME_SOUND_INSTANCES_MAX 4

@interface cocoaGameAudio : NSObject <AVAudioPlayerDelegate>
{
    struct
    {
        AVAudioPlayer *ap[GAME_SOUND_NAMES_MAX][GAME_SOUND_INSTANCES_MAX];
        NSString* name[GAME_SOUND_NAMES_MAX];
        int ap_inst[GAME_SOUND_NAMES_MAX];
        float duration_ms[GAME_SOUND_NAMES_MAX];
        int clone;
        int available;
    } audioPlayersPreloaded;
}

-(id)init;
-(void)dealloc;
-(void) playSound: (NSString*)soundFileName withVolume:(float)volume andRate:(float)rate;
-(void) playSound: (NSString*)soundFileName withVolume:(float)volume andDuration:(float) duration_ms;
-(void) processMessages: (int)maxMessagesProcessed;

@end

