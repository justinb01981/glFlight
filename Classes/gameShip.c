//
//  gameShip.c
//  gl_flight
//
//  Created by jbrady on 9/5/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include "quaternions.h"
#include "worldElem.h"

int my_ship_id = WORLD_ELEM_ID_INVALID;
int my_ship_changed = 0;
float my_ship_x;
float my_ship_y;
float my_ship_z;
float my_ship_alpha;
float my_ship_beta;
float my_ship_gamma;
quaternion_t my_ship_bx;
quaternion_t my_ship_by;
quaternion_t my_ship_bz;

void
gameShip_init(float x, float y, float z, float alpha, float beta, float gamma)
{
    my_ship_x = x;
    my_ship_y = y;
    my_ship_z = z;
    my_ship_alpha = alpha;
    my_ship_beta = beta;
    my_ship_gamma = gamma;
    
    get_body_vectors_for_euler(alpha, beta, gamma, &my_ship_bx, &my_ship_by, &my_ship_bz);
    
    /*
    my_ship_alpha = alpha;
    my_ship_beta = beta;
    my_ship_gamma = gamma;
     */
}

void
gameShip_normalize()
{
    float alpha, beta, gamma;
    
    get_euler_from_body_vectors(&my_ship_bx, &my_ship_by, &my_ship_bz, &alpha, &beta, &gamma);
    
    get_body_vectors_for_euler(alpha, beta, gamma, &my_ship_bx, &my_ship_by, &my_ship_bz);
}

void
gameShip_roll(float r)
{
    /*
    quaternion_t bodyx, bodyy, bodyz;
    
    r = -r;
    
    get_body_vectors_for_euler(my_ship_alpha, my_ship_beta, my_ship_gamma, &bodyx, &bodyy, &bodyz);
    
    quaternion_rotate_inplace(&bodyx, &bodyz, r);
    quaternion_rotate_inplace(&bodyy, &bodyz, r);
    
    get_euler_from_body_vectors(&bodyx, &bodyy, &bodyz, &my_ship_alpha, &my_ship_beta, &my_ship_gamma);
     */
    
    r = -r;
    
    quaternion_rotate_inplace(&my_ship_bx, &my_ship_bz, r);
    quaternion_rotate_inplace(&my_ship_by, &my_ship_bz, r);
}

void
gameShip_pitch(float r)
{
    /*
    quaternion_t bodyx, bodyy, bodyz;
    
    r = -r;
    
    get_body_vectors_for_euler(my_ship_alpha, my_ship_beta, my_ship_gamma, &bodyx, &bodyy, &bodyz);
    
    quaternion_rotate_inplace(&bodyz, &bodyx, r);
    quaternion_rotate_inplace(&bodyy, &bodyx, r);
    
    get_euler_from_body_vectors(&bodyx, &bodyy, &bodyz, &my_ship_alpha, &my_ship_beta, &my_ship_gamma);
     */
    
    r = -r;
    
    quaternion_rotate_inplace(&my_ship_by, &my_ship_bx, r);
    quaternion_rotate_inplace(&my_ship_bz, &my_ship_bx, r);
}

void
gameShip_yaw(float r)
{
    /*
    quaternion_t bodyx, bodyy, bodyz;
    
    get_body_vectors_for_euler(my_ship_alpha, my_ship_beta, my_ship_gamma, &bodyx, &bodyy, &bodyz);
    
    quaternion_rotate_inplace(&bodyx, &bodyy, r);
    quaternion_rotate_inplace(&bodyz, &bodyy, r);
    
    get_euler_from_body_vectors(&bodyx, &bodyy, &bodyz, &my_ship_alpha, &my_ship_beta, &my_ship_gamma);
     */
    
    quaternion_rotate_inplace(&my_ship_bx, &my_ship_by, r);
    quaternion_rotate_inplace(&my_ship_bz, &my_ship_by, r);
}

void
gameShip_moveZ(float s)
{
    my_ship_x -= my_ship_bz.x*s;
    my_ship_y -= my_ship_bz.y*s;
    my_ship_z -= my_ship_bz.z*s;
}

void
gameShip_getXVector(float vec[3])
{    
    vec[0] = my_ship_bx.x;
    vec[1] = my_ship_bx.y;
    vec[2] = my_ship_bx.z;
}

void
gameShip_getYVector(float vec[3])
{
    vec[0] = my_ship_by.x;
    vec[1] = my_ship_by.y;
    vec[2] = my_ship_by.z;
}

void
gameShip_getZVector(float vec[3])
{
    vec[0] = my_ship_bz.x;
    vec[1] = my_ship_bz.y;
    vec[2] = my_ship_bz.z;
}

float
gameShip_getEulerAlpha() {
    float alpha, beta, gamma;
    get_euler_from_body_vectors(&my_ship_bx, &my_ship_by, &my_ship_bz,
                                &alpha, &beta, &gamma);
    return alpha;
}

float
gameShip_getEulerBeta() {
    float alpha, beta, gamma;
    get_euler_from_body_vectors(&my_ship_bx, &my_ship_by, &my_ship_bz,
                                &alpha, &beta, &gamma);
    return beta;
}

float
gameShip_getEulerGamma() {
    float alpha, beta, gamma;
    get_euler_from_body_vectors(&my_ship_bx, &my_ship_by, &my_ship_bz,
                                &alpha, &beta, &gamma);
    return gamma;
}
