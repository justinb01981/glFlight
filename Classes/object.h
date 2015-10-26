//
//  object.h
//  gl_flight
//
//  Created by Justin Brady on 12/26/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef __OBJECT_H__
#define __OBJECT_H__

typedef enum {
    // TODO: these enums are used in maps, so don't change their order/remove...
    OBJ_UNKNOWN = 0,
    OBJ_SHIP = 1,
    OBJ_PLAYER,
    OBJ_TURRET,
    OBJ_BLOCK_MOVING,
    OBJ_BLOCK,
    OBJ_BULLET,
    OBJ_PORTAL,
    OBJ_SPAWNPOINT,
    OBJ_POOPEDCUBE,
    OBJ_WRECKAGE,
    OBJ_POWERUP_GENERIC,
    OBJ_CAPTURE,
    OBJ_BASE,
    OBJ_MISSLE,
    OBJ_SPAWNPOINT_ENEMY,
    OBJ_RESERVED3,
    OBJ_RESERVED4,
    OBJ_RESERVED5,
    OBJ_RESERVED6,
    // TODO: has gravity, pulls objects towards it when in range
    /* OBJ_BLACKHOLE */
    OBJ_LAST
} Object;

inline static int
object_is_static(Object o)
{
    switch (o)
    {
        case OBJ_UNKNOWN:
        case OBJ_BLOCK:
        case OBJ_WRECKAGE:
            return 1;
            
        default:
            return 0;
    }
}

#endif /* __OBJECT_H__ */