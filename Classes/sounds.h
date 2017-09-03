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
    GAME_SOUND_ID_BULLET_FLYBY = 16,
    GAME_SOUND_ID_MISSLE_FLYBY = 16,
};

enum {
    GAME_SOUND_DURATION_BULLET_FLYBY = 1000,
    GAME_SOUND_DURATION_MISSLE_FLYBY = 1000,
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
    "engineloop",
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
    "teleport",
    "filelost",
    NULL
};



#endif
