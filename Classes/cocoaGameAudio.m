//
//  cocoaGameAudio.m
//  gl_flight
//
//  Created by jbrady on 12/2/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "cocoaGameAudio.h"

#import "gameAudio.h"
#import "AVFoundation/AVAudioPlayer.h"
#import "AudioToolbox/AudioServices.h"
#import "Foundation/NSUrl.h"
#include "sounds.h"

#define MAX_CACHED_AUDIOPLAYERS 16

@implementation cocoaGameAudio

AVAudioPlayer* soundsList[MAX_CONCURRENT_SOUNDS];
unsigned int nextSound = 0;
struct cache
{
    NSString* str;
    AVAudioPlayer* audioPlayer;
} cachedAudioPlayers[MAX_CACHED_AUDIOPLAYERS];
unsigned int cachedAudioPlayers_next = 0;

int playSound = 1;


- (id) init
{
    memset(&audioPlayersPreloaded, 0, sizeof(audioPlayersPreloaded));
    
    audioPlayersPreloaded.clone = 0;
    audioPlayersPreloaded.available = MAX_CONCURRENT_SOUNDS;
    
    NSError* err = [NSError errorWithDomain:@"cocoaGameAudio" code:0 userInfo:NULL];
    
    int i = 0;
    // 0 is "invalid"
    while(i < GAME_SOUND_NAMES_MAX && gameSoundNames[i])
    {
        int j;
        for(j = 0; j < GAME_SOUND_INSTANCES_MAX; j++)
        {
            audioPlayersPreloaded.ap[i][j] = [[AVAudioPlayer alloc]
                                           initWithContentsOfURL:[[NSBundle mainBundle] URLForResource:
                                                                  [NSString stringWithUTF8String:gameSoundNames[i]]
                                                                                         withExtension:@"wav"]
                                           error:&err];
            if(!audioPlayersPreloaded.ap[i][j]) break;
            
            audioPlayersPreloaded.name[i] = [[NSString alloc] initWithUTF8String:gameSoundNames[i]];
            
            [audioPlayersPreloaded.ap[i][j] enableRate];
            
            audioPlayersPreloaded.duration_ms[i] = 1000 * [audioPlayersPreloaded.ap[i][j] duration];
            
            printf("cocoaGameAudio.init: loaded sound %s with duration %f\n",
                   [audioPlayersPreloaded.name[i] UTF8String], audioPlayersPreloaded.duration_ms[i]);
        }
        
        i++;
    }
    
    return self;
}
- (void) dealloc
{
    int i = 0;
    while(i < GAME_SOUND_NAMES_MAX && gameSoundNames[i])
    {
        int j;
        for(j = 0; j < GAME_SOUND_INSTANCES_MAX; j++)
        {
            if(audioPlayersPreloaded.ap[i][j])
            {
                [audioPlayersPreloaded.ap[i][j] release];
                audioPlayersPreloaded.ap[i][j] = NULL;
            }
        }
        
        i++;
    }
    [super dealloc];
}

- (void) playSound: (NSString*)soundFileName withVolume:(float)volume andRate:(float)rate
{
#if 0
    AVAudioPlayer* audioPlayer = NULL;
    int i;
    int duplicate_cached_audioplayers = 0;
    
    if(!playSound) return;
    
    if([soundFileName compare:@"vibrate"] == NSOrderedSame)
    {
        AudioServicesPlayAlertSound(kSystemSoundID_Vibrate);
        return;
    }
    
    // TODO: use an NSList or NSNamedList or something
    for(i = 0; i < cachedAudioPlayers_next && i < MAX_CACHED_AUDIOPLAYERS; i++)
    {
        if([cachedAudioPlayers[i].str compare:soundFileName] == NSOrderedSame)
        {
            audioPlayer = [cachedAudioPlayers[i].audioPlayer copy];
            
            if(duplicate_cached_audioplayers)
            {
                [audioPlayer enableRate];
                [audioPlayer setRate:rate];
                [audioPlayer play];
                [audioPlayer release];
            }
            else
            {
                [audioPlayer stop];
                [audioPlayer setRate:rate];
                [audioPlayer play];
            }
            return;
        }
    }
    
    NSError* err = [[NSError alloc] init];
    NSURL* soundURL = [[NSBundle mainBundle] URLForResource:soundFileName withExtension:@"wav"];
    
    audioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:soundURL error:&err];
    if(!audioPlayer)return;
    
    [audioPlayer setVolume:volume];
    
    if(soundsList[nextSound] != 0)
    {
        [soundsList[nextSound] stop];
        [soundsList[nextSound] release];
    }
    soundsList[nextSound] = audioPlayer;
    [audioPlayer enableRate];
    [audioPlayer setRate:rate];
    [audioPlayer play];
    
    /*
     cachedAudioPlayers[cachedAudioPlayers_next++].audioPlayer = audioPlayer;
     cachedAudioPlayers[cachedAudioPlayers_next++].str = [[NSString alloc] initWithString:soundFileName];
     if(cachedAudioPlayers_next >= MAX_CACHED_AUDIOPLAYERS)
     {
     cachedAudioPlayers_next = 0;
     }
     
     if(cachedAudioPlayers[cachedAudioPlayers_next].audioPlayer)
     {
     [cachedAudioPlayers[cachedAudioPlayers_next].audioPlayer release];
     [cachedAudioPlayers[cachedAudioPlayers_next].str release];
     }
     */
    
    nextSound++;
    if(nextSound >= MAX_CONCURRENT_SOUNDS) nextSound = 0;
    
#else
    
    int i = 0;
    int found = 0;
    
    while(i < GAME_SOUND_NAMES_MAX && audioPlayersPreloaded.ap[i][0] != NULL)
    {
        if([audioPlayersPreloaded.name[i] compare:soundFileName] == NSOrderedSame)
        {
            AVAudioPlayer *a;
            NSError *e;
            AVAudioPlayer *ac;
            
            found = 1;
            
            if(audioPlayersPreloaded.available > 0) audioPlayersPreloaded.available--;
            
            int ainst = audioPlayersPreloaded.ap_inst[i];
            a = audioPlayersPreloaded.ap[i][ainst];
            if(!a) break;
            
            ainst++;
            if(ainst >= GAME_SOUND_INSTANCES_MAX) ainst = 0;
            audioPlayersPreloaded.ap_inst[i] = ainst;

            if(audioPlayersPreloaded.clone)
            {
                ac = [[AVAudioPlayer alloc] initWithContentsOfURL:[a url] error:&e];
            }
            else
            {
                ac = a;
            }
            
            if([ac isPlaying]) return;
            
            [ac setDelegate:self];
            
            [ac stop];
            
            [ac setVolume:volume];
            [ac enableRate];
            [ac setRate:rate];
            [ac play];
        }
        
        i++;
    }
    
    if(!found)
    {
        printf("Warning: sound %s not found!\n", [soundFileName UTF8String]);
    }
    
#endif
}

- (void) playSound: (NSString*)soundFileName withVolume:(float)volume andDuration:(float) duration_ms
{
    float rate = 1.0;
    
    int i = 0;
    
    while(i < GAME_SOUND_NAMES_MAX && audioPlayersPreloaded.ap[i][0] != NULL)
    {
        if([audioPlayersPreloaded.name[i] compare:soundFileName] == NSOrderedSame)
        {
            rate = audioPlayersPreloaded.duration_ms[i] / duration_ms;
        }
        i += 1;
    }
    [self playSound:soundFileName withVolume:volume andRate:rate];
}

- (void) processMessages: (int) maxMessagesProcessed
{
    
    gameAudioLock();
    
    cocoaMessage* pCur = cocoaMessageAudioList.next;
    cocoaMessage msgList[16];
    int msgListLen = 0;
    
    while(pCur && maxMessagesProcessed > 0 && msgListLen < 16)
    {
        msgList[msgListLen] = *pCur;
        msgListLen++;
        
        pCur = pCur->next;
        
        cocoaMessageListPop(&cocoaMessageAudioList);
        
        maxMessagesProcessed--;
    }
    
    gameAudioUnlock();
    
    for(int i = 0; i < msgListLen; i++)
    {
        NSString* str = [[NSString alloc] initWithUTF8String:msgList[i].str];
        
        if(msgList[i].f[1] != 0)
        {
            [self playSound:str withVolume:msgList[i].f[0] andRate:msgList[i].f[1]];
        }
        else
        {
            [self playSound:str withVolume:msgList[i].f[0] andDuration:msgList[i].f[2]];
        }
        
        [str release];
    }
}

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag;
{
    if(audioPlayersPreloaded.clone)
    {
        [player release];
    }
    audioPlayersPreloaded.available++;
}

- (void)audioPlayerEndInterruption:(AVAudioPlayer *)player withFlags:(NSUInteger)flags
{
    [player play];
}

@end


