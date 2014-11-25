//
//  gameCamera.h
//  gl_flight
//
//  Created by jbrady on 8/23/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

/*******************************************************************************
 * // OpenGL rotation/translation block to be used                             *
 * glRotatef(RADIANS_TO_DEGREES(gameCamera_getEulerGamma()), 0, 0, 1);         *
 * glRotatef(RADIANS_TO_DEGREES(gameCamera_getEulerBeta()), 1, 0, 0);          *
 * glRotatef(RADIANS_TO_DEGREES(gameCamera_getEulerAlpha()), 0, 0, 1);         *
 * glTranslatef(-gameCamera_getX(),                                            *
 *              -gameCamera_getY(),                                            *
 *              -gameCamera_getZ());                                           *
 *******************************************************************************/

// TODO: inline as many as possible

#ifndef gl_flight_gameCamera_h
#define gl_flight_gameCamera_h

#include "quaternions.h"
#include <math.h>

#define RADIANS_TO_DEGREES(x) (((float)x*360.0)/(M_PI*2))

#define GAME_CAMERA_CHASE_LAG_FRAMES_MAX 64

#define GAME_CAMERA_Z_TRAIL_DEFAULT 4

typedef struct
{
    // cam position in the world
    float x, y, z;
    // all perpendicular to each other
    quaternion_t bz, bx, by;
} camera_position_t;

extern camera_position_t cam_pos;

extern int camera_locked_frames;
extern int camera_chase_frames;
extern float camera_z_trail;

void
gameCamera_init(float x, float y, float z,
                float eulerAlpha, float eulerBeta, float eulerGamma);

// called periodically to fix-up body axes which suffer from drift
void
gameCamera_normalize();

float
gameCamera_getX();

float
gameCamera_getY();

float
gameCamera_getZ();

float
gameCamera_getEulerAlpha();

float
gameCamera_getEulerBeta();

float
gameCamera_getEulerGamma();

void
gameCamera_pitchRadians(float r);

void
gameCamera_yawRadians(float r);

void
gameCamera_rollRadians(float r);

inline static void
gameCamera_getZVector(float d[3]) { 
    d[0] = -cam_pos.bz.x; d[1] = cam_pos.bz.y; d[2] = -cam_pos.bz.z;
}

inline static void
gameCamera_getXVector(float d[3]) { 
    d[0] = -cam_pos.bx.x; d[1] = cam_pos.bx.y; d[2] = -cam_pos.bx.z;
}

inline static void
gameCamera_getYVector(float d[3]) { 
    d[0] = -cam_pos.by.x; d[1] = cam_pos.by.y; d[2] = -cam_pos.by.z;
}

inline static void
gameCamera_MoveX(float s) {
    cam_pos.x -= cam_pos.bx.x*s;
    cam_pos.y += cam_pos.bx.y*s;
    cam_pos.z -= cam_pos.bx.z*s;
}

inline static void
gameCamera_MoveY(float s) {
    cam_pos.x -= cam_pos.by.x*s;
    cam_pos.y += cam_pos.by.y*s;
    cam_pos.z -= cam_pos.by.z*s;
}

inline static void
gameCamera_MoveZ(float s) {
    cam_pos.x -= cam_pos.bz.x*s;
    cam_pos.y += cam_pos.bz.y*s;
    cam_pos.z -= cam_pos.bz.z*s;
}

#endif
