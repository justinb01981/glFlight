//
//  gameAudio.h
//  gl_flight
//
//  Created by jbrady on 9/29/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef __GAME_AUDIO_H__
#define __GAME_AUDIO_H__

#define MAX_CONCURRENT_SOUNDS 16
#define GAME_AUDIO_QUEUE_READ_INTERVAL_MS 50

struct cocoaMessage
{
    char str[64];
    float f[8];
    struct cocoaMessage* next;
};
typedef struct cocoaMessage cocoaMessage;

extern int gameAudioMuted;

int
cocoaMessageListAdd(cocoaMessage* msg, cocoaMessage* head);

int
cocoaMessageListPop(cocoaMessage* head);

void
cocoaMessageListClear(cocoaMessage* head);

extern cocoaMessage cocoaMessageAudioList;

void
gameAudioInit();

void
gameAudioPlaySoundAtLocation(const char* filename, float x, float y, float z);

void
gameAudioPlaySoundAtLocationWithRate(const char* filename, float volumescale, float x, float y, float z, float rate);

void
gameAudioPlaySoundAtLocationWithDuration(const char* filename, float volumescale, float x, float y, float z, float duration_ms);

void
gameAudioLock();

void
gameAudioUnlock();

#endif
