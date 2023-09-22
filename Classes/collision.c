//
//  collision.c
//  gl_flight
//
//  Created by Justin Brady on 11/10/14.
//
//

#include <stdio.h>

#include "collision.h"

// this is dead code but im too lazy to update makefiles
// TODO: collision handling should be done via static callbacks! much more maintainable #facepalm

typedef collision_action_t;

collision_action_t collision_mem[OBJ_LAST][OBJ_LAST];

void collision_actions_set_default() {
    int r, c;
    for (r = 0; r < OBJ_LAST; r++)
        for (c = 0; c < OBJ_LAST; c++) {
            if (collision_actions[r][c] != collision_actions[c][r]) assert(0);
        }

        collision_action_t collision_mem[OBJ_LAST][OBJ_LAST] = {

            // TODO: keep this order matching the OBJTYPE enum

// unknown
            {0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // same as begin - sometimes the default type is 0
// begin
            {0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 1, 0, 0}, // begin
// ship
            {0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0,       1, COLLISION_ACTION_CAPTURE, 1, 5, 6, 0, 1, 0, 0}, // ship
// player
            {0, 2, 2, 2, 2, 2, 2, 4, 7, 0, 0,       1, COLLISION_ACTION_CAPTURE, 1, 1, 0, 0, 0, 0, 0}, // player
// turret
            {0, 0, 2, 2, 0, 1, 1, 0, 0, 0, 0,       1, 0, 1, 0, 0, 0, 0, 0, 0}, // turret
// moving block
            {0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0,       1, 0, 1, 0, 0, 0, 0, 0, 0}, // block moving
// static block
            {0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0,       1, 0, 1, 0, 0, 0, 0, 0, 0}, // block
// portal
            {0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // portal
// pupedcube
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // poopedcube
// wreckage?
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // wreckage
//powerup
            {0, 5, 6, 0, 2, 2, 0, 0, 3, 0, 0,       0, 0, 0, 0, 6, 0, 0, 0, 0}, // powerup generic
// bullet
            {0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // bullet
//capoture
            {0, 2, COLLISION_ACTION_CAPTURE, COLLISION_ACTION_CAPTURE, 1, 1, 1, 2, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, COLLISION_ACTION_CAPTURE}, // capture
// base - player
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // base
// mnissle
            {0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,       0, 1, 0, 0, 0, 0, 0, 0, 0}, // missle
// spawnpoint
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // spawnpoint
// spawnpoint enemy
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       COLLISION_ACTION_CAPTURE, 0, 0, 0, 0, 0, 0, 0, 0}, // spawnpoint enemy
// display only
            {0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // DISPLAYONLY
// ball
            {0, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // BALL
            // reserved
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // reserved
// reserrved
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0} // reserved
    };
}