//
//  actions.c
//  gl_flight
//
//  Created by Justin Brady on 7/1/13.
//
//

#include <stdio.h>
#include "action.h"

int *actions_enabled = actions_sub[0];
int actions_sub_cur = 0;
action_t fireAction = ACTION_FIRST;