//
//  gameAudio.c
//  gl_flight
//
//  Created by jbrady on 12/2/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <math.h>

#include "gameAudio.h"
#include "gameUtils.h"
#include "gameLock.h"

cocoaMessage cocoaMessageAudioList;

game_lock_t cocoaMessageAudioListLock;

int gameAudioMuted = 0;

// keep in mind: all queued sounds scheduled at GAME_AUDIO_QUEUE_READ_INTERVAL_MS
const static int gameAudioMaxQueuedSounds = 8;

int
cocoaMessageListAdd(cocoaMessage* msg, cocoaMessage* head)
{
    int length = 0;

    cocoaMessage* pCur = head;
    while(pCur->next)
    {
        length++;
        pCur = pCur->next;
    }
    
    if(length < gameAudioMaxQueuedSounds)
    {
        pCur->next = malloc(sizeof(*pCur));
        if(pCur->next)
        {
            memcpy(pCur->next, msg, sizeof(*msg));
            pCur->next->next = NULL;
            
            return 1;
        }
    }
    
    return 0;
}

int
cocoaMessageListPop(cocoaMessage* head)
{
    cocoaMessage* pCur = head->next;
    if(!pCur) return 0;
    
    head->next = head->next->next;
    
    free(pCur);
    
    return 1;
}

void
cocoaMessageListClear(cocoaMessage* head)
{
    while(cocoaMessageListPop(head)) {}
}

void
gameAudioInit()
{
    memset(&cocoaMessageAudioList, 0, sizeof(cocoaMessageAudioList));
    
    game_lock_init(&cocoaMessageAudioListLock);
}

void
gameAudioLock()
{
    game_lock_lock(&cocoaMessageAudioListLock);
}

void
gameAudioUnlock()
{
    game_lock_unlock(&cocoaMessageAudioListLock);
}

void
gameAudioPlaySoundAtLocationWithRate(const char* filename, float x, float y, float z, float rate)
{
    if(gameAudioMuted) return;
    
    // TODO: audio panning
    cocoaMessage msg;
    float vol_distance = cam_distance(x, y, z);
    float vol_distance_max = 25;
    float v = (vol_distance / vol_distance_max);
    float vol_c[] = {0.5, 0.3, 0.2, 0.1, 0.1, 0.01, 0}; //{1, 0.8, 0.5, 0.2, 0.1, 0, 0};
    
    if(v > 1.0) v = 1.0;
    
    if(vol_distance < vol_distance_max)
    {
        float r = (sizeof(vol_c)/sizeof(float));
        float idx = round(v * r);
        
        float vol = vol_c[(int) idx];
        
        if(vol > 0 && vol < 1.0)
        {
            msg.f[0] = vol;
            msg.f[1] = rate;
            strncpy(msg.str, filename, sizeof(msg.str)-1);
            
            gameAudioLock();
            cocoaMessageListAdd(&msg, &cocoaMessageAudioList);
            gameAudioUnlock();
            
            printf("Playing sound %s with vol:%f\n", msg.str, msg.f[0]);
        }
    }
}

void
gameAudioPlaySoundAtLocation(const char* filename, float x, float y, float z)
{
    gameAudioPlaySoundAtLocationWithRate(filename, x, y, z, 1.0);
}