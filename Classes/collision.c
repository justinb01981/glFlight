//
//  collision.c
//  gl_flight
//
//  Created by Justin Brady on 11/10/14.
//
//

#include <stdio.h>

#include "collision.h"

collision_action_t collision_actions[OBJ_LAST][OBJ_LAST];

collision_action_t collision_actions_stage1[OBJ_LAST][OBJ_LAST];


char collidedPortalNametagLast[256] = {0};
