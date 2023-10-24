//
//  gameMotionInterp.h
//  gl_flight
//
//  Created by Justin Brady on 5/17/19.
//

#ifndef gameMotionInterp_h
#define gameMotionInterp_h

#include <math.h>
#include "gameNetwork.h"

const static float euler_interp_range_max = (M_PI/8);

void
motion_interpolate_euler(game_timeval_t network_time_ms, motion_interp_st* motion, gameNetworkPlayerInfo* pInfo, WorldElem* pElem);

void
motion_interpolate_velocity(game_timeval_t network_time_ms, motion_interp_st* motion, WorldElem* pElem, gameNetworkMessage* msg, int interp_velo);

#endif /* gameMotionInterp_h */
