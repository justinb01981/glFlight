//
//  gameCamera.c
//  gl_flight
//
//  Created by jbrady on 8/23/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include "gameCamera.h"


camera_position_t cam_pos;
camera_position_t cam_pos_chase[GAME_CAMERA_CHASE_LAG_FRAMES_MAX];

int camera_locked_frames = 0;
int camera_chase_frames = 8;
float camera_z_trail = GAME_CAMERA_Z_TRAIL_DEFAULT;

const static quaternion_t init_qx = {0, 1, 0, 0};
const static quaternion_t init_qy = {0, 0, 1, 0};
const static quaternion_t init_qz = {0, 0, 0, 1};

void
gameCamera_init(float x, float y, float z,
                float eulerAlpha, float eulerBeta, float eulerGamma)
{
    cam_pos.bx = init_qx;
    cam_pos.by = init_qy;
    cam_pos.bz = init_qz;
    
    cam_pos.x = x;
    cam_pos.y = y;
    cam_pos.z = z;
    
    get_body_vectors_for_euler(eulerAlpha, eulerBeta, eulerGamma,
                               &cam_pos.bx, &cam_pos.by, &cam_pos.bz);
    
    if(camera_chase_frames)
    {
        for(int i = camera_chase_frames-1; i > 0; i--)
        {
            cam_pos_chase[i] = cam_pos_chase[i-1];
        }
        
        cam_pos_chase[0] = cam_pos;
        cam_pos = cam_pos_chase[camera_chase_frames-1];
    }
}

void
gameCamera_initWithHeading(float x, float y, float z,
                           float vx, float vy, float vz)
{
    float cam[] = {
        x,
        y,
        z
    };
    
    float d = sqrt(vx*vx + vy*vy + vz*vz);
    float o = atan2(vx / d, vz / d);
    
    float k = sqrt(vx*vx + 0 + vz*vz);
    float p = atan2(vy / d, k / d);
    
    gameCamera_init(cam[0], cam[1], cam[2],
                    0.0, 0.0, 0.0);
    gameCamera_yawRadians(M_PI+o);
    gameCamera_pitchRadians(-p);
}

void
gameCamera_normalize()
{
    camera_position_t tmp;
    
    float a = gameCamera_getEulerAlpha();
    float b = gameCamera_getEulerBeta();
    float c = gameCamera_getEulerGamma();
    
    tmp = cam_pos;
    
    tmp.bx = init_qx;
    tmp.by = init_qy;
    tmp.bz = init_qz;
    
    // Z
    quaternion_rotate_inplace(&tmp.bx, &tmp.bz, a);
    quaternion_rotate_inplace(&tmp.by, &tmp.bz, a);
    // X'
    quaternion_rotate_inplace(&tmp.bz, &tmp.bx, b);
    quaternion_rotate_inplace(&tmp.by, &tmp.bx, b);
    // Z''
    quaternion_rotate_inplace(&tmp.by, &tmp.bz, c);
    quaternion_rotate_inplace(&tmp.bx, &tmp.bz, c);
    
    cam_pos = tmp;
}

float
gameCamera_getX() { return cam_pos.x; }

float
gameCamera_getY() { return cam_pos.y; }

float
gameCamera_getZ() { return cam_pos.z; }

// convert body axes to world-axes
// http://en.wikipedia.org/wiki/Euler_angles

float
gameCamera_getEulerAlpha() {
    //yaw = atan2(bz.x, -bz.y);
    return atan2(cam_pos.bz.x, -cam_pos.bz.y);
}

float
gameCamera_getEulerBeta() {
    //pitch = acos(bz.z); // asin(bz.z) // this appeared to work when using tait-bryan angles
    return acos(cam_pos.bz.z);
}

float
gameCamera_getEulerGamma() {
    //roll = atan2(bx.z, by.z);
    return atan2(cam_pos.bx.z, cam_pos.by.z);
}

void
gameCamera_yawRadians(float r) {
    quaternion_rotate_inplace(&cam_pos.bz, &cam_pos.by, r);
    quaternion_rotate_inplace(&cam_pos.bx, &cam_pos.by, r);
}

void
gameCamera_pitchRadians(float r) {
    quaternion_rotate_inplace(&cam_pos.bz, &cam_pos.bx, r);
    quaternion_rotate_inplace(&cam_pos.by, &cam_pos.bx, r);    
}

void
gameCamera_rollRadians(float r) {
    quaternion_rotate_inplace(&cam_pos.by, &cam_pos.bz, r);
    quaternion_rotate_inplace(&cam_pos.bx, &cam_pos.bz, r);
}
