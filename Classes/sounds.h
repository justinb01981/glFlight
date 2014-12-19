//
//  sounds.h
//  gl_flight
//
//  Created by Justin Brady on 12/10/13.
//
//

#ifndef gl_flight_sounds_h
#define gl_flight_sounds_h

#include <stdlib.h>

#define GAME_SOUND_NAMES_MAX 32

enum {
    GAME_SOUND_ID_BULLET_FLYBY = 15,
    GAME_SOUND_ID_MISSLE_FLYBY = 15,
};

static const char* gameSoundNames[] = {
    "engine",
    "shoot",
    "missle",
    "boom",
    "bump",
    "dropoff",
    "collect",
    "dead",
    "engine2",
    "victory",
    "warning",
    "highscore",
    "towing",
    "speedboost",
    "lockedon",
    "flyby",
    "engineslow",
    "enginefast",
    NULL
};



#endif
