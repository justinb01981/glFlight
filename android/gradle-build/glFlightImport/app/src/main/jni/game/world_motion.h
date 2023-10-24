//
//  world_motion.h
//  gl_flight
//
//  Created by Justin Brady on 5/14/18.
//

#ifndef world_motion_h
#define world_motion_h

#include "world.h"

inline static void orbit_around(WorldElem* orbiter, float o[3], float orbit_T, float R)
{
    WorldElem* pElem = orbiter;
    float radius = R;
    float orbit_origin[3] = {o[0], o[1], o[2]};
    
    float v[] = {
        orbit_origin[0] + sin(orbit_T)*radius,
        orbit_origin[1],
        orbit_origin[2] + cos(orbit_T)*radius
    };
    
    world_replace_object(pElem->elem_id, pElem->type, v[0], v[1], v[2],
                         M_PI/2, orbit_T, -M_PI/2, pElem->scale,
                         pElem->texture_id);
}

#endif /* world_motion_h */
