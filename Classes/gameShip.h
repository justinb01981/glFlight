//
//  gameShip.h
//  gl_flight
//
//  Created by jbrady on 9/5/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef gl_flight_gameShip_h
#define gl_flight_gameShip_h

#include "quaternions.h"

void
gameShip_init(float x, float y, float z, float alpha, float beta, float gamma);

void
gameShip_normalize();

void
gameShip_roll(float r);

void
gameShip_pitch(float r);

void
gameShip_yaw(float r);

void
gameShip_moveZ(float s);

void
gameShip_getXVector(float vec[3]);

void
gameShip_getYVector(float vec[3]);

void
gameShip_getZVector(float vec[3]);

float
gameShip_getEulerAlpha();

float
gameShip_getEulerBeta();

float
gameShip_getEulerGamma();

static inline void
gameShip_getEuler(float* alpha, float* beta, float* gamma)
{
    extern quaternion_t my_ship_bx, my_ship_by, my_ship_bz;
    
    get_euler_from_body_vectors(&my_ship_bx, &my_ship_by, &my_ship_bz,
                                alpha, beta, gamma);
}

float
gameShip_calcRoll();

#endif
