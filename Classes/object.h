//
//  object.h
//  gl_flight
//
//  Created by Justin Brady on 12/26/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef __OBJECT_H__
#define __OBJECT_H__


#include <string.h>
#include <stdlib.h>

//#include "gameIncludes.h"

// TODO: these enums are used in maps, so don't change their order/remove...
typedef enum {
    OBJ_ZERO,
    OBJ_BEGIN = 1,
    OBJ_SHIP = 2,
    OBJ_PLAYER = 3,
    OBJ_TURRET = 4,
    OBJ_BLOCK_MOVING = 5,
    OBJ_BLOCK = 6,
    OBJ_PORTAL = 7,
    OBJ_POOPEDCUBE = 8,                                                                             // 8
    OBJ_WRECKAGE = 9,                                                           // 9
    OBJ_POWERUP_GENERIC = 10,                                                               // 10
    OBJ_BULLET = 11, // these are used as ordinals in collision handling see collision.h  11
    OBJ_CAPTURE = 12, // replaced powerup with subtype capture with this object-type  12
    OBJ_BASE = 13,
    OBJ_MISSLE = 14,
    OBJ_SPAWNPOINT = 15,  //15
    OBJ_SPAWNPOINT_ENEMY = 16,
    OBJ_DISPLAYONLY = 17,
    OBJ_TOUCHCONTROLBALL = 18,
    OBJ_RESERVED5 = 19,
//    OBJ_RESERVED6 = 20,
    // TODO: has gravity, pulls objects towards it when in range
    /* OBJ_BLACKHOLE */
    OBJ_UNKNOWN = 0,
    OBJ_LAST = 20
} Object;

#define OBJ_PLAYER_STR "3"
#define OBJ_BLOCK_STR "6"
#define OBJ_BEGIN_STR "1"

inline static int
object_is_static(Object o)
{
    switch (o)
    {
        case OBJ_UNKNOWN:
        case OBJ_BLOCK:
        case OBJ_DISPLAYONLY:
            return 1;
            
        default:
            return 0;
    }
}

inline static int
object_type_impacts(Object o)
{
    if(o == OBJ_SHIP || o == OBJ_PLAYER) return 1;
    return 0;
}

inline static int
object_enforce_bounding(Object o)
{
    return o != OBJ_DISPLAYONLY;
}


#endif /* __OBJECT_H__ */
