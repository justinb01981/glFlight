//
//  gameAI.h
//  gl_flight
//
//  Created by jbrady on 8/9/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "world.h"

#ifndef __GAME_AI_H__
#define __GAME_AI_H__

enum
{
    ENEMY_STATE_PURSUE = 0,
    ENEMY_STATE_RUN = 1,
    ENEMY_STATE_PATROL = 2,
    ENEMY_STATE_JUKE = 4,
    //ENEMY_STATE_PULLUP = 5
};

enum
{
    ENEMY_COLLIDE_NONE = 0,
    ENEMY_COLLIDE_OBJECT = 1,
    ENEMY_COLLIDE_BOUNDARY = 2
};

typedef struct
{
    int started;
} gameAIState_t;

extern gameAIState_t gameAIState;

void
game_ai_init();

void
game_ai_run();

void
object_pursue(float x, float y, float z, float vx, float vy, float vz, WorldElem *elem, int target_objtype);

void
game_ai_collision(WorldElem* elemA, WorldElem* elemB, int collision_action);

int
game_ai_find_target(WorldElem *pElem);

#endif
