//
//  models.h
//  gl_flight
//
//  Created by Justin Brady on 12/26/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "string.h"

/*
#import <Foundation/Foundation.h>
#import <OpenGLES/EAGL.h>
#import "EAGLView.h"
 */

#ifndef __MODELS_H__
#define __MODELS_H__

#include "gameIncludes.h"

const static int MODEL_PRIMITIVES_NONE = 0;

typedef GLfloat model_coord_t;
// keep these in-sync
#define index_type_enum GL_UNSIGNED_SHORT
typedef GLushort model_index_t;
typedef GLfloat model_texcoord_t;

typedef struct
{
    model_coord_t model_coords_buffer[512];
    model_texcoord_t model_texcoords_buffer[512];
    model_index_t model_indices_buffer[512];
    unsigned int model_coords_buffer_len;
    unsigned int model_faces_buffer_n;
    int primitives[512];
} model_poly_components_t;

#define MODEL_POLY_COMPONENTS_DECLARE() \
    model_poly_components_t poly_comp

#define MODEL_POLY_COMPONENTS_INIT() \
    poly_comp.model_faces_buffer_n = poly_comp.model_coords_buffer_len = poly_comp.primitives[0] = 0;

#define MODEL_EXTENDED_MEMORY(modeltype) (\
    modeltype == MODEL_SPHERE ||          \
    modeltype == MODEL_ENEMY_BASE ||      \
    modeltype == MODEL_SHIP1 ||           \
    modeltype == MODEL_BUILDING3 ||       \
    modeltype == MODEL_CBUILDING)

/* static array of coordinates, texture-coordinates, triangle-faces, matrix */
#define MODEL_POLY_COMPONENTS_ADD(Ac, At, Af, M)    \
    {   \
        int i;  \
        int triangle_st = poly_comp.model_coords_buffer_len; \
        \
        for(i = 0; i < sizeof(Ac)/sizeof(model_coord_t); i += 3)    \
        {   \
            int R = 4;  \
            poly_comp.model_coords_buffer[poly_comp.model_coords_buffer_len*3] = M[R*0]*Ac[i] + M[R*0+1]*Ac[i+1] + M[R*0+2]*Ac[i+2] + M[R*0+3];   \
            poly_comp.model_coords_buffer[poly_comp.model_coords_buffer_len*3+1] = M[R*1]*Ac[i] + M[R*1+1]*Ac[i+1] + M[R*1+2]*Ac[i+2] + M[R*1+3]; \
            poly_comp.model_coords_buffer[poly_comp.model_coords_buffer_len*3+2] = M[R*2]*Ac[i] + M[R*2+1]*Ac[i+1] + M[R*2+2]*Ac[i+2] + M[R*2+3]; \
            \
            int Ti = i/3;    \
            poly_comp.model_texcoords_buffer[poly_comp.model_coords_buffer_len*2] = At[Ti*2];    \
            poly_comp.model_texcoords_buffer[poly_comp.model_coords_buffer_len*2+1] = At[Ti*2+1];    \
            poly_comp.model_coords_buffer_len++;    \
        }   \
            \
         \
        for(i = 0; i < sizeof(Af)/sizeof(model_index_t)/3; i++) \
        {   \
            poly_comp.model_indices_buffer[poly_comp.model_faces_buffer_n*3] = Af[i*3] + triangle_st;    \
            poly_comp.model_indices_buffer[poly_comp.model_faces_buffer_n*3+1] = Af[i*3+1] + triangle_st;    \
            poly_comp.model_indices_buffer[poly_comp.model_faces_buffer_n*3+2] = Af[i*3+2] + triangle_st;    \
            poly_comp.model_faces_buffer_n ++;  \
        }   \
            \
        for(i = 0; poly_comp.primitives[i] != 0; i++) model_primitives_sizeof += sizeof(int);   \
    }

#include "sphere_model.h"
#include "building2_model.h"

typedef enum {
    MODEL_FIRST,
	MODEL_SHIP1 = 1,
	MODEL_CUBE,
    MODEL_CUBE2,
	MODEL_PYRAMID,
	MODEL_SQUARE, //
    MODEL_BULLET,
    MODEL_SURFACE,
    MODEL_VERTICAL_PILLAR, //
    MODEL_MESH,
    MODEL_TELEPORTER,
    MODEL_SPRITE,          // 
    MODEL_TURRET,
    MODEL_SHIP2,
    MODEL_MISSLE,
    MODEL_SHIP3,
    MODEL_ICOSAHEDRON,
    MODEL_CUBE_INVERTED,
    MODEL_SPHERE,
    MODEL_ENEMY_BASE,
    MODEL_TBUILDING,
    MODEL_CBUILDING,
    MODEL_FLATTENED_CUBE,
    MODEL_SCENERY,
    MODEL_CONTRAIL,
    MODEL_LINE,
    MODEL_BUILDING3,
    MODEL_LAST,
} Model;

/********************************************************/
static model_coord_t model_pyramid[] = 
{
	0, 0.5, 0,
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5
};

static model_index_t model_pyramid_indices[] = 
{
    1,4,0,
    4,3,0,
    3,2,0,
    2,1,0,
    3,4,1,
    2,3,1
};

static model_texcoord_t model_pyramid_texcoords[] = 
{
    0.5,0.5,
    0,0,
    1,0,
    1,1,
    0,1
};

/********************************************************/

/********************************************************/
static model_coord_t model_ship1[] = 
{
    // diamond polygon cockpit
    0, 0, -1,
    -0.15, 0, -0.5,
    0.15, 0, -0.5,
    0, 0, 0,
    0, 0.15, -0.5,
    0, -0.15, -0.5,
    
    // 6 - wing 1
    -1, 0, 0, // [6]
    -1, 0, 0.2,

    // wing 2
    1, 0, 0, // [8]
    1, 0, 0.2,
    
    // engine 1 pyramid
    -0.25, 0, 0.5, // [10]
    -0.35, 0, 0,
    -0.25, 0.1, 0,
    -0.15, 0, 0,
    -0.25, -0.1, 0,

    // engine 2 pyramid
    0.25, 0, 0.5, // [15]
    0.35, 0, 0,
    0.25, 0.1, 0,
    0.15, 0, 0,
    0.25, -0.1, 0,
    
    -1, 0, 0,
    -0.3, 0, 0.2
    -0.3, 0, 0,
    
    1, 0, 0,
    0.3, 0, 0.2,
    0.3, 0, 0,
    
    // some bullshit points to increase bounding extent
    0.0, 0.5, 0,
    0.0, -0.5, 0

};

static model_index_t model_ship1_indices[] = {
    
    //MACRO_DIAMOND_INDICES(0)
    0,1,4,
    0,4,2,
    0,2,5,
    0,5,1,
    4,1,3,
    2,4,3,
    5,2,3,
    1,5,3,
    
    // wing 1
    1,6,3,
    3,6,1,
    7,6,11,
    11,6,7,
    
    // wing 2
    2,3,8,
    8,3,2,
    8,16,9,
    9,16,8,
    
    // engine 1
    12,11,10,
    13,12,10,
    14,13,10,
    11,14,10,
    14,11,12,
    14,12,13,
    
    // engine 2
    17,18,15,
    16,17,15,
    19,16,15,
    18,19,15,
    18,17,19,
    17,16,19,
};

static model_texcoord_t model_ship1_texcoords[] = {
    // diamond polygon cockpit
    0.5, 1,
    0.40, 0.75, //1
    0.40, 0.75,
    0.5, /*0.5*/0.4, //3
    0.5, 0.75,
    0.05, 0.98,
    
    // wing 1
    0, 1-0.6, //6
    0, 1-0.9,
    
    //wing 2
    0, 1-0.6,
    0, 1-0.9,
    
    // engine 1 pyramid
    0.3, 0,
    0.25, 0.25,
    0.30, 0.25,
    0.35, 0.3,
    0.30, 0.25,
    
    // engine 2 pyramid
    0.3, 0,
    0.25, 0.25,
    0.30, 0.25,
    0.35, 0.3,
    0.30, 0.25,
    
};

/********************************************************/
static model_coord_t model_turret1[] =
{
    // center diamond
    0,0.3,0, // top       0
    0,-0.3,0, //bottom    1
    0,0,-0.6, //front     2
    0,0,0.5, //back       3
    -0.5,0,0, //left      4
    0.5,0,0, //right      5
    
    // left gun
    -0.5,-0.1,-0.5, //    6
    -0.5,0.1,-1.0, //     7
    -0.5,0.1,-0.5, //     8
    
    // right gun
    0.5,-0.1,-0.5, //     9
    0.5,0.1,-1.0,  //     10
    0.5,0.1,-0.5,  //     11
};

static model_index_t model_turret_indices[] = {
    // diamond indices
    5,2,0,
    2,4,0,
    4,3,0,
    3,5,0,
    4,2,1,
    2,5,1,
    5,3,1,
    3,4,1,
    
    8,7,6,
    6,7,8,
    
    11,10,9,
    9,10,11
};

static model_texcoord_t model_turret_texcoords[] = {
    0.5,0.5,
    0.5,0.5,
    0.5,0.0,
    0.5,1.0,
    0.0,0.5,
    1.0,0.5,
    
    0.0,0.0,
    0.0,0.4,
    0.4,0.1,
    
    0.0,0.0,
    0.0,0.4,
    0.4,0.1
};

/********************************************************/
static model_coord_t model_bullet[] =
{
    0, 0, 1,
    -0.1, 0, 0,
    0.1, 0, 0,
    0, 0, -1,
    0, -0.1, 0,
    0, 0.1, 0,
};

static model_index_t model_bullet_indices[] = {
    0, 1, 4,
    4, 2, 0,
    2, 5, 0,
    5, 1, 0,
    5, 3, 1,
    5, 2, 3,
    2, 4, 3,
    4, 1, 3,
};

static model_texcoord_t model_bullet_texcoords[] = { 
    0.5, 0,
    0, 0.5,
    1, 0.5,
    0.5, 1,
    0.5, 0.5,
    0.5, 0.5
};

/********************************************************/

static model_texcoord_t model_contrail_texcoords[] =
{
    0,0,
    1,0,
    0,0,
    1,0,
    
    0,1,
    1,1,
    0,1,
    1,1,
    
    0,0,
    1,0,
    1,1,
    0,1,
    
    0,0,
    1,0,
    1,1,
    0,1,
};

static model_coord_t model_contrail[] =
{
    // WARNING: these are being manipulated at model creation see firePoopedCube
    
    //bottom
    -0.06, -0.06, 0,
    0.06, -0.06, 0,
    0.06, -0.06, 0.2,
    -0.06, -0.06, 0.2,
    
    // top
    -0.06, 0.06, 0,
    0.06, 0.06, 0,
    0.06, 0.06, 0.2,
    -0.06, 0.06, 0.2,
    
    //bottom2
    -0.06, -0.06, 0,
    0.06, -0.06, 0,
    0.06, -0.06, 0.2,
    -0.06, -0.06, 0.2,
    
    //top2
    -0.06, 0.06, 0,
    0.06, 0.06, 0,
    0.06, 0.06, 0.2,
    -0.06, 0.06, 0.2,
};

static model_index_t model_contrail_indices[] =
{
    // ordering: counter-clockwise for each face (2 triangles) looking inwards to cube
    13,12,15, 14,13,15, // top
    
    0,3,7, 7,4,0,
    
    3,2,6, 6,7,3,
    
    2,1,5, 5,6,2,
    
    1,0,4, 4,5,1,
    
    11,8,9, 10,11,9, // bottom
};

/********************************************************/
static model_texcoord_t model_cube_texcoords_alt[] =
{
    0.0, 0.5,
    0.25, 0.5,
    0.5, 0.5,
    0.25, 0.5,
    
    0.0, 0.25,
    0.25, 0.25,
    0.5, 0.25,
    0.25, 0.25,
    
    0.25, 0.5,
    0.5, 0.5,
    0.5, 0.75,
    0.25, 0.75,
    
    0.25, 0.25,
    0.5, 0.25,
    0.5, 0,
    0.25, 0,
};

static model_texcoord_t model_cube_texcoords[] =
{
    0,0,
    1,0,
    0,0,
    1,0,
    
    0,1,
    1,1,
    0,1,
    1,1,
    
    0,0,
    1,0,
    1,1,
    0,1,
    
    0,0,
    1,0,
    1,1,
    0,1,
};

static model_texcoord_t model_cube_texcoords4x[] =
{
    0,0,
    1,0,
    0,0,
    1,0,
    
    0,4,
    1,4,
    0,4,
    1,4,
    
    0,0,
    1,0,
    1,4,
    0,4,
    
    0,0,
    1,0,
    1,4,
    0,4,
};

static model_coord_t model_cube[] = 
{
	//bottom
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
	
	// top
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,
    
    //bottom2
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,
	0.5, -0.5, 0.5,
	-0.5, -0.5, 0.5,
    
    //top2
	-0.5, 0.5, -0.5,
	0.5, 0.5, -0.5,
	0.5, 0.5, 0.5,
	-0.5, 0.5, 0.5,
};

static model_index_t model_cube_indices[] = 
{
	// ordering: counter-clockwise for each face (2 triangles) looking inwards to cube
    13,12,15, 14,13,15, // top

	0,3,7, 7,4,0,
	
	3,2,6, 6,7,3,
    
	2,1,5, 5,6,2,

	1,0,4, 4,5,1,

    11,8,9, 10,11,9, // bottom
};

static model_index_t model_cube_indices_inverted[] =
{
    // ordering: counter-clockwise for each face (2 triangles) looking inwards to cube
    15,12,13, 15,13,14, // top
    
    7,3,0, 0,4,7,
    
    6,2,3, 3,7,6,
    
    5,1,2, 2,6,5,
    
    4,0,1, 1,5,3,
    
    9,8,11, 9,11,10, // bottom
};

static model_index_t model_cube_indices_nobottom[] =
{
    // ordering: counter-clockwise for each face (2 triangles) looking inwards to cube
    13,12,15, 14,13,15, // top
    
    0,3,7, 7,4,0,
    
    3,2,6, 6,7,3,
    
    2,1,5, 5,6,2,
    
    1,0,4, 4,5,1
};

static model_coord_t model_cube_normals[] =
{
    0.5, 0.0, 0.0, // coord
    1.0, 0.0, 0.0, // vec
    
    0.0, 0.5, 0.0, // coord
    0.0, 1.0, 0.0, // vec
    
    0.0, 0.0, 0.5, // coord
    0.0, 0.0, 1.0, // vec
    
    -0.5, 0.0, 0.0, // coord
    -1.0, 0.0, 0.0, // vec
    
    0.0, -0.5, 0.0, // coord
    0.0, -1.0, 0.0, // vec
    
    0.0, 0.0, -0.5, // coord
    0.0, 0.0, -1.0, // vec
};

static model_coord_t model_flattenedcube_normals[] =
{
    0.5, 0.0, 0.0, // coord
    1.0, 0.0, 0.0, // vec
    
    0.0, 0.1, 0.0, // coord
    0.0, 1.0, 0.0, // vec
    
    0.0, 0.0, 0.5, // coord
    0.0, 0.0, 1.0, // vec
    
    -0.5, 0.0, 0.0, // coord
    -1.0, 0.0, 0.0, // vec
    
    0.0, -0.1, 0.0, // coord
    0.0, -1.0, 0.0, // vec
    
    0.0, 0.0, -0.5, // coord
    0.0, 0.0, -1.0, // vec
};

static model_coord_t model_enemybase_normals[] =
{
    2.0, 0.0, 0.0, // coord
    1.0, 0.0, 0.0, // vec
    
    0.0, 2.0, 0.0, // coord
    0.0, 1.0, 0.0, // vec
    
    0.0, 0.0, 2.0, // coord
    0.0, 0.0, 1.0, // vec
    
    -2.0, 0.0, 0.0, // coord
    -1.0, 0.0, 0.0, // vec
    
    0.0, -2.0, 0.0, // coord
    0.0, -1.0, 0.0, // vec
    
    0.0, 0.0, -2.0, // coord
    0.0, 0.0, -1.0, // vec
};

/********************************************************/

/********************************************************/
static model_coord_t model_square[] = 
{
	-0.5, -0.5, 0.0,
	0.5, -0.5, 0.0,
	0.5, 0.5, 0.0,
	-0.5, 0.5, 0.0
};

static model_index_t model_square_indices[] =
{
	0,1,2, 2,3,0,
    0,3,2, 2,1,0
};

static model_texcoord_t model_square_texcoords[] =
{
	0,0,
	1,0,
	1,1,
	0,1
};

/********************************************************/
static model_coord_t model_surface[] = 
{
    -.5, -.5, 0,
    .5, -.5, 0,
    .5, .5, 0,
    -.5, .5, 0
};
static model_index_t model_surface_indices[] =
{
	0,1,2, 2,3,0,
    0,3,2, 2,1,0
};
static model_texcoord_t model_surface_texcoords[] =
{
	0,0,
	1,0,
	1,1,
	0,1
};

/********************************************************/
static model_coord_t model_sprite[] = 
{
    -1, -1, 0,
    1, -1, 0,
    1, 1, 0,
    -1, 1, 0,
    
    0, -1, -1,
    0, 1, -1,
    0, 1, 1,
    0, -1, 1,
    
    -1, 0, -1,
    1, 0, -1,
    1, 0, 1,
    -1, 0, 1,
};
static model_index_t model_sprite_indices[] =
{
	0,1,2, 2,3,0,
    0,3,2, 2,1,0,
    
    4,5,6, 6,7,4,
    4,7,6, 6,5,4,
    
    8,9,10, 10,11,8,
    8,11,10, 10,9,8
};
static model_texcoord_t model_sprite_texcoords[] =
{
	0,0,
	1,0,
	1,1,
	0,1,
    
    0,0,
	1,0,
	1,1,
	0,1,
    
    0,0,
	1,0,
	1,1,
	0,1,
};

static model_coord_t model_sprite_normals[] =
{
    1.0, 0.0, 0.0, // coord
    1.0, 0.0, 0.0, // vec
    
    0.0, 1.0, 0.0, // coord
    0.0, 1.0, 0.0, // vec
    
    0.0, 0.0, 1.0, // coord
    0.0, 0.0, 1.0, // vec
    
    -1.0, 0.0, 0.0, // coord
    -1.0, 0.0, 0.0, // vec
    
    0.0, -1.0, 0.0, // coord
    0.0, -1.0, 0.0, // vec
    
    0.0, 0.0, -1.0, // coord
    0.0, 0.0, -1.0, // vec
};

/********************************************************/

static model_texcoord_t model_vertical_pillar_texcoords[] =
{
    0,0,
    0.25, 0,
    0.5, 0,
    0.75, 0,
    
    0,   13,
    0.25, 13,
    0.5, 13,
    0.75, 13
};

static model_coord_t model_vertical_pillar[] = 
{
	//bottom
	-1, -10, -1,
	1, -10, -1,
	1, -10, 1,
	-1, -10, 1,
	
	// top
	-1, 10, -1,
	1, 10, -1,
	1, 10, 1,
	-1, 10, 1,
};

static model_index_t model_vertical_pillar_indices[] = 
{
	// ordering: counter-clockwise for each face (2 triangles) looking out from inside the cube
	2,3,0, 0,1,2,
    
	0,3,7, 7,4,0,
	
	3,2,6, 6,7,3,
	
	2,1,5, 5,6,2,
	
	1,0,4, 4,5,1,
    
	5,4,7, 7,6,5,
};

/********************************************************/

static model_coord_t model_ship2[] =
{
    // diamond polygon cockpit
    0, 0, -0.7,
    -0.15, 0, -0.5,
    0.15, 0, -0.5,
    0, 0, 0,
    0, 0.2, -0.5,
    0, -0.1, -0.5,
    
    // 6 - wing 1 [6]
    -0.2, 0.1, 0,
    -0.25, 0.0, -1,
    -0.5, -0.05, -0.8,
    -0.8, -0.1, 0.0,
    -0.3, -0.02, 0.3,
    
    // wing 2  [11]
    0.2, 0.1, 0,
    0.25, 0.0, -1,
    0.5, -0.05, -0.8,
    0.8, -0.1, 0.0,
    0.3, -0.02, 0.3,
    
    // some bullshit points to increase bounding extent
    0.0, 0.5, 0,
    0.0, -0.5, 0
};

static model_index_t model_ship2_indices[] = {
    
    //MACRO_DIAMOND_INDICES(0)
    0,1,4,
    0,4,2,
    0,2,5,
    0,5,1,
    4,1,3,
    2,4,3,
    5,2,3,
    1,5,3,
    
    // wing 1
    6,7,9, 9,7,6,
    7,8,9, 9,8,7,
    6,9,10, 10,9,6,
    
    // wing 2
    11,12,14, 14,12,11,
    12,13,14, 14,13,12,
    11,14,15, 15,14,11
};

static model_texcoord_t model_ship2_texcoords[] = {
    // diamond polygon cockpit
    0.5, 1,
    0.4, 0.75, //1
    0.6, 0.75,
    0.5, 0.0, //3
    0.5, 0.75,
    0.5, 0.25,
    
    // wing 1
    0.4,0.6,
    0.3,0.0,
    0.2,0.1,
    0.1,0.6,
    0.1,0.7,
    
    // wing 2
    0.4,0.6,
    0.3,0.0,
    0.2,0.1,
    0.1,0.6,
    0.1,0.7,
};

static model_coord_t model_ship2_normals[] =
{
    1.0, 0.0, 0.0, // coord
    1.0, 0.0, 0.0, // vec
    
    0.0, 1.0, 0.0, // coord
    0.0, 1.0, 0.0, // vec
    
    0.0, 0.0, 1.0, // coord
    0.0, 0.0, 1.0, // vec
    
    -1.0, 0.0, 0.0, // coord
    -1.0, 0.0, 0.0, // vec
    
    0.0, -1.0, 0.0, // coord
    0.0, -1.0, 0.0, // vec
    
    0.0, 0.0, -1.0, // coord
    0.0, 0.0, -1.0, // vec
};

/********************************************************/

static model_coord_t model_missle[] =
{
    0, 0, -1.0,
    -0.25, 0, -0.7,
    0.25, 0, -0.7,
    0, 0, 0,
    0, 0.15, -0.7,
    0, -0.15, -0.7
};

static model_index_t model_missle_indices[] = {
    0,1,4,
    0,4,2,
    0,2,5,
    0,5,1,
    4,1,3,
    2,4,3,
    5,2,3,
    1,5,3,
};

static model_texcoord_t model_missle_texcoords[] = {
    0.5, 1,
    0.4, 0.75, //1
    0.6, 0.75,
    0.5, 0.5, //3
    0.5, 0.75,
    0.5, 0.25
};

/********************************************************/

static model_coord_t model_ship3[] =
{
    // diamond polygon cockpit
    0, 0, -0.5,
    -0.15, 0, 0,
    0.15, 0, 0,
    0, 0, 0.6,
    0, 0.15, 0,
    0, -0.1, 0,
    
    // diamond engine right
    0.4, 0, -0.1,
    0.3, 0, 0.1,
    0.5, 0, 0.1,
    0.4, 0, 0.35,
    0.4, 0.1, 0.1,
    0.4, -0.1, 0.1,
    
    // wing tip right
    1.0, -0.1, -0.05,
    1.0, -0.1, 0.1,
    
    //... flip
    // diamond engine left
    -0.4, 0, -0.1,
    -0.3, 0, 0.1,
    -0.5, 0, 0.1,
    -0.4, 0, 0.35,
    -0.4, 0.1, 0.1,
    -0.4, -0.1, 0.1,
    
    // wing tip left
    -1.0, -0.1, -0.05,
    -1.0, -0.1, 0.1,
    
    // some bullshit points to increase bounding extent
    0.0, 0.5, 0.5,
    0.0, -0.5, 0.5
    
};

static model_index_t model_ship3_indices[] = {
    
    //MACRO_DIAMOND_INDICES(0)
    0,1,4,
    0,4,2,
    0,2,5,
    0,5,1,
    4,1,3,
    2,4,3,
    5,2,3,
    1,5,3,
    
    // +6
    6,7,10,
    6,10,8,
    6,8,11,
    6,11,7,
    10,7,9,
    8,10,9,
    11,8,9,
    7,11,9,
    
    2,7,9,
    9,7,2,
    2,9,3,
    3,9,2,
    12,8,13,
    13,8,12,
    13,8,9,
    9,8,13,
    
    // +8
    15,14,18,
    18,14,16,
    16,14,19,
    14,15,19,
    15,18,17,
    17,18,16,
    16,19,17,
    19,15,17,
    
    16,1,17,
    17,1,16,
    17,1,3,
    3,1,17,
    20,15,21,
    21,15,20,
    21,15,17,
    17,15,21
};

static model_texcoord_t model_ship3_texcoords[] = {
    // diamond polygon cockpit
    0.5, 1,
    0.4, .85, //1
    0.4, .85,
    0.5, 0.1, //3
    0.5, 0.85,
    0.5, 0.15,
    
    // engine right
    0.70, 0.70,
    0.65, 0.60, //1
    0.75, 0.60,
    0.70, 0.20, //3
    0.70, 0.60,
    0.70, 0.60,
    
    // wingtip right
    1,0.5,
    1,0.8,
    
    // flip
    // engine left
    0.70, 0.70,
    0.65, 0.60, //1
    0.75, 0.60,
    0.70, 0.20, //3
    0.70, 0.60,
    0.70, 0.60,
    
    // wingtip left
    1,0.5,
    1,0.8
};

static model_coord_t model_ship3_normals[] =
{
    1.0, 0.0, 0.0, // coord
    1.0, 0.0, 0.0, // vec
    
    0.0, 1.0, 0.0, // coord
    0.0, 1.0, 0.0, // vec
    
    0.0, 0.0, 1.0, // coord
    0.0, 0.0, 1.0, // vec
    
    -1.0, 0.0, 0.0, // coord
    -1.0, 0.0, 0.0, // vec
    
    0.0, -1.0, 0.0, // coord
    0.0, -1.0, 0.0, // vec
    
    0.0, 0.0, -1.0, // coord
    0.0, 0.0, -1.0, // vec
};

/********************************************************/

/********************************************************/
static model_coord_t model_icosahedron[] =
{
    0.0,  1,  0.0, // 1 (top)

    0.0, 0.5, 1.0,  // sin(0) = 0, cos(0) = 1
    0.95, 0.5, 0.3, // theta=1.256
    0.59, 0.5, -0.81, // theta=2.513
    -0.59, 0.5, -0.81, // theta=3.769
    -0.95, 0.5, 0.31, // theta=5.026
    
    0.59, -0.5, 0.81, // theta = 0.628
    0.95, -0.5, -0.31, // theta = 1.884
    0.00, -0.5, -0.99, // theta = 3.14
    -0.95, -0.5, -0.31, // theta = 4.396
    -0.59, -0.5, 0.81, // theta = 5.652
    
    0.0,  -1,  0.0
};

static model_index_t model_icosahedron_indices[] =
{
    2,0,1,
    3,0,2,
    4,0,3,
    5,0,4,
    1,0,5,
    
    2,1,6,
    7,2,6,
    3,2,7,
    3,7,8,
    4,3,8,
    9,4,8,
    5,4,9,
    10,5,9,
    1,5,10,
    6,1,10,
    
    7,6,11,
    8,7,11,
    9,8,11,
    10,9,11,
    6,10,11
};

static model_texcoord_t model_icosahedron_texcoords[] =
{
    0.1,0,
    0,0.25,
    0.2,0.25,
    0.4,0.25,
    0.6,0.25,
    0.8,0.25,
    0.2,0.75,
    0.4,0.75,
    0.6,0.75,
    0.8,0.75,
    0,0.75,
    0.3,1.0
};

static model_coord_t model_icosahedron_normals[] =
{
    1.0, 0.0, 0.0, // coord
    1.0, 0.0, 0.0, // vec
    
    0.0, 1.0, 0.0, // coord
    0.0, 1.0, 0.0, // vec
    
    0.0, 0.0, 1.0, // coord
    0.0, 0.0, 1.0, // vec
    
    -1.0, 0.0, 0.0, // coord
    -1.0, 0.0, 0.0, // vec
    
    0.0, -1.0, 0.0, // coord
    0.0, -1.0, 0.0, // vec
    
    0.0, 0.0, -1.0, // coord
    0.0, 0.0, -1.0, // vec
};

/********************************************************/

/*
  ___\
 X   /
 

 
 
       0      1              / \
           F                  |
       3      2               z+

8     9      12    14    16   / \
    F    F     F     F         |
11    10     13    15    17    Y
 
       5      6                z-
           F                   |
       4      7               \ /
 */
static model_texcoord_t model_background_texcoords[] =
{
    //0
    0.25, 0.0,
    //1
    0.50, 0.0,
    //2
    0.50, 0.25,
    //3
    0.25, 0.25,
    //4
    0.25, 1.0,
    //5
    0.25, 0.75,
    //6
    0.5, 0.75,
    //7
    0.5, 1.0,
    //8
    0.0, 0.25,
    //9
    0.25, 0.25,
    //10
    0.25, 0.75,
    //11
    0.0, 0.75,
    //12
    0.5, 0.25,
    //13
    0.5, 0.75,
    //14
    0.75, 0.25,
    //15
    0.75, 0.75,
    //16
    1.0, 0.25,
    //17
    1.0, 0.75
};

static model_coord_t model_background[] =
{

    //0
    -1, -1, -1,
    //1
    1.0, -1, -1,
    //2
    1.0, -1, 1.0,
    //3
    -1, -1, 1.0,
    //4
    -1, 1.0, -1,
    //5
    -1, 1.0, 1.0,
    //6
    1.0, 1.0, 1.0,
    //7
    1.0, 1.0, -1,
    //8
    -1, -1, -1,
    //9
    -1, -1, 1.0,
    //10
    -1, 1.0, 1.0,
    //11
    -1, 1.0, -1,
    //12
    1.0, -1, 1.0,
    //13
    1.0, 1.0, 1.0,
    //14
    1.0, -1, -1,
    //15
    1.0, 1.0, -1,
    //16
    -1, -1, -1,
    //17
    -1, 1.0, -1
};

static model_index_t model_background_indices1[] =
{
    // ordering: counter-clockwise for each face (2 triangles) looking inwards to cube
    0, 1, 3, 3, 1, 2,
    5, 6, 7, 7, 4, 5,
    
    8, 9, 11, 11, 9, 10,
    9, 12, 10, 10, 12, 13,
    12, 14, 13, 13, 14, 15,
    15, 14, 16, 15, 16, 17
    /*
    0,3,7, 7,4,0,
    
    3,2,6, 6,7,3,
    
    2,1,5, 5,6,2,
    
    1,0,4, 4,5,1,
    
    11,8,9, 10,11,9, // bottom
     */
};

/********************************************************/

static model_coord_t model_enemy_base_coords[] =
{
    0, 0.5, 0,
    -0.5, -0.5, -0.5,
    0.5, -0.5, -0.5,
    0.5, -0.5, 0.5,
    -0.5, -0.5, 0.5,
    
    0, 1.0 - 0.5, 0,
    -0.5, 1.0+0.5, -0.5,
    0.5, 1.0+0.5, -0.5,
    0.5, 1.0+0.5, 0.5,
    -0.5, 1.0+0.5, 0.5,
    
    0.500000, 0.500000, 0.000000,
    -0.500000, 0.000000, -0.500000,
    -0.500000, 1.000000, -0.500000,
    -0.500000, 1.000000, 0.500000,
    -0.500000, 0.000000, 0.500000,
    
    -0.500000, 0.500000, 0.000000,
    0.500000, 0.000000, -0.500000,
    0.500000, 1.000000, -0.500000,
    0.500000, 1.000000, 0.500000,
    0.500000, 0.000000, 0.500000
};

static model_index_t model_enemy_base_indices[] =
{
    1,4,0,
    4,3,0,
    3,2,0,
    2,1,0,
    3,4,1,
    2,3,1,
    
    5,9,6,
    5,8,9,
    5,7,8,
    5,6,7,
    6,9,8,
    6,8,7,
    
    10,14,11,
    10,13,14,
    10,12,13,
    10,11,12,
    11,14,13,
    11,13,12,
    
    16,19,15,
    19,18,15,
    18,17,15,
    17,16,15,
    18,19,16,
    17,18,16
};

static model_texcoord_t model_enemy_base_texcoords[] =
{
    0.5,0.5,
    0,0,
    1,0,
    1,1,
    0,1,
    
    0.5,0.5,
    0,0,
    1,0,
    1,1,
    0,1,
    
    0.5,0.5,
    0,0,
    1,0,
    1,1,
    0,1,
    
    0.5,0.5,
    0,0,
    1,0,
    1,1,
    0,1
};

static model_coord_t model_enemy_base_core[] =
{
    0, 0, 0.5,
    -0.2, 0, 0,
    0.2, 0, 0,
    0, 0, -0.5,
    0, -0.2, 0,
    0, 0.2, 0,
};

static model_index_t model_enemy_base_core_indices[] = {
    0, 1, 4,
    4, 2, 0,
    2, 5, 0,
    5, 1, 0,
    5, 3, 1,
    5, 2, 3,
    2, 4, 3,
    4, 1, 3,
};

static model_texcoord_t model_enemy_base_core_texcoords[] = {
    0.5, 0,
    0, 0.5,
    1, 0.5,
    0.5, 1,
    0.5, 0.5,
    0.5, 0.5
};

/********************************************************/

static model_coord_t model_blendship_coords[] = {
    -0.000853, 1.882923, -0.0157,
    -0.31343, 0.81416, -0.218069,
    0.118537, 0.814164, -0.343143,
    0.385509, 0.814161, -0.0157,
    0.118537, 0.814164, 0.311743,
    -0.31343, 0.81416, 0.186669,
    -0.120246, -0.915118, -0.343143,
    0.311721, -0.915114, -0.218069,
    0.311721, -0.915114, 0.186669,
    -0.120246, -0.915118, 0.311743,
    -0.387218, -0.915114, -0.0157,
    -0.000856, -1.983876, -0.0157,
    0.069322, 1.594172, -0.208165,
    -0.18458, 1.594171, -0.134649,
    -0.114405, 0.965969, -0.327116,
    -0.368308, 0.965969, -0.0157,
    -0.18458, 1.594171, 0.103249,
    0.226243, 1.594171, -0.0157,
    0.29642, 0.965968, -0.208166,
    0.069322, 1.594172, 0.176765,
    0.29642, 0.965968, 0.176766,
    -0.114405, 0.965969, 0.295717,
    -0.411681, -0.050476, -0.13465,
    -0.41168, -0.050476, 0.10325,
    -0.000855, -0.050477, -0.400631,
    -0.254758, -0.050477, -0.327117,
    0.409971, -0.050478, -0.13465,
    0.253049, -0.050476, -0.327117,
    0.253049, -0.050476, 0.295717,
    0.409971, -0.050478, 0.10325,
    -0.254758, -0.050477, 0.295717,
    -0.000855, -0.050477, 0.369231,
    -0.298129, -1.066921, -0.208166,
    0.112696, -1.066923, -0.327116,
    0.366599, -1.066922, -0.0157,
    0.112696, -1.066923, 0.295717,
    -0.298129, -1.066922, 0.176766,
    -0.071031, -1.695125, -0.208165,
    -0.227952, -1.695124, -0.0157,
    0.182871, -1.695124, -0.134649,
    0.182871, -1.695124, 0.103249,
    -0.071031, -1.695125, 0.176765,
    0.364087, 1.152156, -0.029476,
    1.807028, 0.015903, -0.0242,
    0.364087, -1.197678, 0.020042,
    2.364087, -0.847232, 0.020042,
    -0.357415, 1.161218, -0.027402,
    -1.800356, 0.024965, -0.022126,
    -0.357415, -1.188615, 0.022116,
    -2.357415, -0.838169, 0.022116
};
static model_texcoord_t model_blendship_texcoords[] = {
    0.457002, 0.469053,
    0.483742, 0.53178,
    0.433831, 0.523535,
    0.880061, 0.501535,
    0.84549, 0.653957,
    0.836499, 0.526889,
    0.715486, 0.681991,
    0.753832, 0.62446,
    0.715486, 0.62446,
    0.715486, 0.681991,
    0.715486, 0.62446,
    0.677139, 0.624461,
    0.572538, 0.469053,
    0.549367, 0.523535,
    0.599277, 0.53178,
    0.880061, 0.501535,
    0.836499, 0.526889,
    0.883577, 0.328502,
    0.210189, 0.969086,
    0.164026, 0.999708,
    0.185001, 0.797016,
    0.041894, 0.503551,
    0.000292, 0.477967,
    0.038379, 0.677406,
    0.33252, 0.968675,
    0.368206, 0.998631,
    0.357918, 0.796193,
    0.246459, 0.969374,
    0.286357, 0.999296,
    0.256747, 0.797016,
    0.880061, 0.501535,
    0.883577, 0.328502,
    0.921664, 0.332959,
    0.210189, 0.969086,
    0.185001, 0.797016,
    0.235587, 0.796605,
    0.041894, 0.503551,
    0.038379, 0.677406,
    0.085457, 0.671897,
    0.33252, 0.968675,
    0.357918, 0.796193,
    0.307333, 0.796605,
    0.799969, 0.492162,
    0.798413, 0.318535,
    0.8365, 0.322993,
    0.159813, 0.624946,
    0.124127, 0.59499,
    0.168355, 0.469464,
    0.245875, 0.624246,
    0.205977, 0.594324,
    0.21894, 0.469053,
    0.121987, 0.841066,
    0.085457, 0.875794,
    0.123543, 0.997352,
    0.282145, 0.624534,
    0.328308, 0.593913,
    0.290686, 0.469053,
    0.880062, 0.154646,
    0.845491, 0.120148,
    0.898125, 0.000292,
    0.638208, 0.526583,
    0.599862, 0.526583,
    0.638209, 0.469053,
    0.638208, 0.526583,
    0.599862, 0.651745,
    0.599862, 0.526583,
    0.754416, 0.596815,
    0.791692, 0.624769,
    0.791692, 0.469053,
    0.846966, 0.825107,
    0.798413, 0.810907,
    0.817397, 0.8764,
    0.846966, 0.825107,
    0.846966, 0.694703,
    0.798413, 0.810907,
    0.846966, 0.694703,
    0.817397, 0.654542,
    0.798413, 0.810907,
    0.894121, 0.819147,
    0.84755, 0.810328,
    0.860133, 0.871263,
    0.894121, 0.819147,
    0.894121, 0.691761,
    0.84755, 0.810328,
    0.894121, 0.691761,
    0.860132, 0.654542,
    0.84755, 0.810328,
    0.650136, 0.735686,
    0.599862, 0.741302,
    0.60737, 0.682575,
    0.650136, 0.735686,
    0.650136, 0.861626,
    0.599862, 0.741302,
    0.650136, 0.861626,
    0.60737, 0.896837,
    0.599862, 0.741302,
    0.676555, 0.526583,
    0.638208, 0.526583,
    0.638209, 0.469053,
    0.676555, 0.526583,
    0.676555, 0.651745,
    0.638208, 0.526583,
    0.676555, 0.651745,
    0.638208, 0.681991,
    0.638208, 0.526583,
    0.8365, 0.322993,
    0.845491, 0.120148,
    0.880062, 0.154646,
    0.8365, 0.322993,
    0.798413, 0.318535,
    0.845491, 0.120148,
    0.256747, 0.797016,
    0.282145, 0.624534,
    0.246459, 0.594579,
    0.307333, 0.796605,
    0.328308, 0.593913,
    0.282145, 0.624534,
    0.307333, 0.796605,
    0.357918, 0.796193,
    0.328308, 0.593913,
    0.357918, 0.796193,
    0.368206, 0.623835,
    0.328308, 0.593913,
    0.085457, 0.671897,
    0.085457, 0.875794,
    0.121987, 0.841066,
    0.085457, 0.671897,
    0.038379, 0.677406,
    0.085457, 0.875794,
    0.038379, 0.677406,
    0.041895, 0.850439,
    0.085457, 0.875794,
    0.235587, 0.796605,
    0.205977, 0.594324,
    0.245875, 0.624246,
    0.235587, 0.796605,
    0.185001, 0.797016,
    0.205977, 0.594324,
    0.185001, 0.797016,
    0.159813, 0.624946,
    0.205977, 0.594324,
    0.134416, 0.797427,
    0.124127, 0.59499,
    0.159813, 0.624946,
    0.921664, 0.332959,
    0.883577, 0.328502,
    0.921664, 0.129063,
    0.883577, 0.328502,
    0.880062, 0.154646,
    0.921664, 0.129063,
    0.256747, 0.797016,
    0.307333, 0.796605,
    0.282145, 0.624534,
    0.256747, 0.797016,
    0.286357, 0.999296,
    0.307333, 0.796605,
    0.286357, 0.999296,
    0.33252, 0.968675,
    0.307333, 0.796605,
    0.123543, 0.66744,
    0.085457, 0.671897,
    0.121987, 0.841066,
    0.123543, 0.66744,
    0.076465, 0.469053,
    0.085457, 0.671897,
    0.076465, 0.469053,
    0.041894, 0.503551,
    0.085457, 0.671897,
    0.038379, 0.677406,
    0.000292, 0.681863,
    0.041895, 0.850439,
    0.038379, 0.677406,
    0.000292, 0.477967,
    0.000292, 0.681863,
    0.245875, 0.999042,
    0.210189, 0.969086,
    0.235587, 0.796605,
    0.185001, 0.797016,
    0.134416, 0.797427,
    0.159813, 0.624946,
    0.185001, 0.797016,
    0.164026, 0.999708,
    0.134416, 0.797427,
    0.164026, 0.999708,
    0.124127, 0.969785,
    0.134416, 0.797427,
    0.883577, 0.328502,
    0.8365, 0.322993,
    0.880062, 0.154646,
    0.883577, 0.328502,
    0.836499, 0.526889,
    0.8365, 0.322993,
    0.836499, 0.526889,
    0.799969, 0.492162,
    0.8365, 0.322993,
    0.599277, 0.53178,
    0.565084, 0.652989,
    0.599277, 0.689294,
    0.599277, 0.53178,
    0.549367, 0.523535,
    0.565084, 0.652989,
    0.549367, 0.523535,
    0.514364, 0.675266,
    0.565084, 0.652989,
    0.549367, 0.523535,
    0.484326, 0.639649,
    0.514364, 0.675266,
    0.677139, 0.624461,
    0.715486, 0.62446,
    0.677139, 0.499298,
    0.715486, 0.62446,
    0.715486, 0.469053,
    0.677139, 0.499298,
    0.715486, 0.62446,
    0.753832, 0.499298,
    0.715486, 0.469053,
    0.715486, 0.62446,
    0.753832, 0.62446,
    0.753832, 0.499298,
    0.433831, 0.523535,
    0.398829, 0.675266,
    0.368791, 0.639649,
    0.836499, 0.526889,
    0.798413, 0.648448,
    0.799969, 0.492162,
    0.836499, 0.526889,
    0.84549, 0.653957,
    0.798413, 0.648448,
    0.708251, 0.729973,
    0.65072, 0.706274,
    0.708251, 0.682575,
    0.433831, 0.523535,
    0.449548, 0.652989,
    0.398829, 0.675266,
    0.433831, 0.523535,
    0.483742, 0.53178,
    0.449548, 0.652989,
    0.483742, 0.53178,
    0.483742, 0.689294,
    0.449548, 0.652989,
    0.797828, 0.468468,
    0.51034, 0.226677,
    0.399352, 0.398646,
    0.797828, 0.468468,
    0.51034, 0.226677,
    0.797828, 0.000292,
    0.398768, 0.000292,
    0.111279, 0.242084,
    0.000292, 0.070114,
    0.398768, 0.000292,
    0.111279, 0.242084,
    0.398768, 0.468468
};
static model_index_t model_blendship_indices[] = {
    0, 12, 13,
    1, 15, 13,
    0, 17, 12,
    0, 19, 17,
    0, 16, 19,
    1, 22, 15,
    2, 24, 14,
    3, 26, 18,
    4, 28, 20,
    5, 30, 21,
    1, 25, 22,
    2, 27, 24,
    3, 29, 26,
    4, 31, 28,
    5, 23, 30,
    6, 37, 32,
    7, 39, 33,
    8, 40, 34,
    9, 41, 35,
    10, 38, 36,
    38, 11, 41,
    38, 41, 36,
    36, 41, 9,
    41, 11, 40,
    41, 40, 35,
    35, 40, 8,
    40, 11, 39,
    40, 39, 34,
    34, 39, 7,
    39, 11, 37,
    39, 37, 33,
    33, 37, 6,
    37, 11, 38,
    37, 38, 32,
    32, 38, 10,
    23, 10, 36,
    23, 36, 30,
    30, 36, 9,
    31, 9, 35,
    31, 35, 28,
    28, 35, 8,
    29, 8, 34,
    29, 34, 26,
    26, 34, 7,
    27, 7, 33,
    27, 33, 24,
    24, 33, 6,
    25, 6, 32,
    25, 32, 22,
    22, 32, 10,
    30, 9, 31,
    30, 31, 21,
    21, 31, 4,
    28, 8, 29,
    28, 29, 20,
    20, 29, 3,
    26, 7, 27,
    26, 27, 18,
    18, 27, 2,
    24, 6, 25,
    24, 25, 14,
    14, 25, 1,
    22, 10, 23,
    22, 23, 15,
    15, 23, 5,
    16, 5, 21,
    16, 21, 19,
    19, 21, 4,
    19, 4, 20,
    19, 20, 17,
    17, 20, 3,
    17, 3, 18,
    17, 18, 12,
    12, 18, 2,
    15, 5, 16,
    15, 16, 13,
    13, 16, 0,
    12, 2, 14,
    12, 14, 13,
    13, 14, 1,
    44, 42, 43,
    48, 46, 47
};

/********************************************************/
static model_coord_t model_tbuilding_coords[] = {
    0.5, -0.5, -0.5,
    0.5, 0.5, -0.5,
    0.5, 0.5, 0.5,
    0.5, 0.5, 0.5,
    0.5, -0.5, 0.5,
    0.5, -0.5, -0.5,
    0.5, -0.5, 0.5,
    0.5, 0.5, 0.5,
    -0.5, 0.5, 0.5,
    -0.5, 0.5, 0.5,
    -0.5, -0.5, 0.5,
    0.5, -0.5, 0.5,
    -0.5, 0.5, 0.5,
    0.5, 0.5, 0.5,
    0.5, 0.5, -0.5,
    -0.5, -0.5, 0.5,
    -0.5, 0.5, 0.5,
    0.5, 0.5, -0.5,
    0.5, 0.5, -0.5,
    0.5, -0.5, -0.5,
    -0.5, -0.5, 0.5,
    0.5, -0.5, -0.5,
    0.5, -0.5, 0.5,
    -0.5, -0.5, 0.5
};
static model_texcoord_t model_tbuilding_texcoords[] = {
    0.261415, 0.738585,
    0.261415, 0.369514,
    0.630486, 0.369514,
    0.630486, 0.369514,
    0.630486, 0.738585,
    0.261415, 0.738585,
    0.261415, 0.369218,
    0.261415, 0.000148,
    0.630486, 0.000148,
    0.630486, 0.000148,
    0.630486, 0.369218,
    0.261415, 0.369218,
    0.26112, 0.522093,
    0.000148, 0.26112,
    0.26112, 0.000148,
    0.999852, 0.369218,
    0.999852, 0.000148,
    0.630781, 0.000148,
    0.630781, 0.000148,
    0.630782, 0.369218,
    0.999852, 0.369218,
    0.261415, 0.738585,
    0.630486, 0.738585,
    0.630486, 0.369218
};
static model_index_t model_tbuilding_indices[] = {
    0, 1, 2,
    3, 4, 5,
    6, 7, 8,
    9, 10, 11,
    12, 13, 14,
    15, 16, 17,
    18, 19, 20,
    21, 22, 23
};

static model_coord_t model_tbuilding_normals[] =
{
    1.0, 0.0, 0.0, // coord
    1.0, 0.0, 0.0, // vec
    
    0.0, 4.0, 0.0, // coord
    0.0, 1.0, 0.0, // vec
    
    0.0, 0.0, 1.0, // coord
    0.0, 0.0, 1.0, // vec
    
    -1.0, 0.0, 0.0, // coord
    -1.0, 0.0, 0.0, // vec
    
    0.0, -1.0, 0.0, // coord
    0.0, -1.0, 0.0, // vec
    
    0.0, 0.0, -1.0, // coord
    0.0, 0.0, -1.0, // vec
};

/********************************************************/

static model_coord_t model_building3_normals_trimmed[] = {
    1.0, 2.5, 0,
    1.0, 0, 0,
    
    -1.0, 2.5, 0,
    -1.0, 0, 0,
    
    0.0, 2.5, 1.0,
    0.0, 0, 1.0,
    
    0.0, 2.5, -1.0,
    0.0, 0, -1.0,
    
    0.0, 2.5, 1.0,
    0.0, 0, 1.0,
    
    0.0, 2.5, -1.0,
    0.0, 0, -1.0,
    
    0.0, 5.0, 0,
    0.0, 1.0, 0,
    
    0.0, -1.0, 0,
    0.0, -1.0, 0
};

/********************************************************/

static inline void
models_iterate()
{
    int l;
    
    l = model_cube[0];
    l = model_cube_indices[0];
    l = model_cube_texcoords[0];
    l = model_cube_texcoords_alt[0];
    
    l = model_vertical_pillar[0];
    l = model_vertical_pillar_indices[0];
    
}
/********************************************************/

struct models_shared_t_
{
    struct
    {
        model_index_t indices[1024];
        model_coord_t coords[3072];
        model_texcoord_t texcoords[2048];
        size_t indices_count;
    } sphere;
};
typedef struct models_shared_t_ models_shared_t;
extern models_shared_t models_shared;

static inline void
models_init()
{
    return;
}

/********************************************************/
									   

#endif /* MODELS_H */
