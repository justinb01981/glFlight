//
//  models.h
//  gl_flight
//
//  Created by Justin Brady on 12/26/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

/*
#import <Foundation/Foundation.h>
#import <OpenGLES/EAGL.h>
#import "EAGLView.h"
 */

#ifndef __MODELS_H__
#define __MODELS_H__

#include "gameIncludes.h"

typedef GLfloat model_coord_t;
// keep these in-sync
#define index_type_enum GL_UNSIGNED_INT
typedef GLuint model_index_t;
typedef GLfloat model_color_t;
typedef GLfloat model_texcoord_t;

typedef enum {
    MODEL_FIRST = 0,
	MODEL_SHIP1 = 0,
	MODEL_CUBE = 1,
    MODEL_CUBE2,
	MODEL_PYRAMID,
	MODEL_SQUARE, // 4
    MODEL_BULLET,
    MODEL_SURFACE,
    MODEL_VERTICAL_PILLAR, // 7
    MODEL_MESH,
    MODEL_TELEPORTER,
    MODEL_SPRITE,          // 10
    MODEL_TURRET,
    MODEL_SHIP2,
    MODEL_MISSLE,
    MODEL_SHIP3,
    MODEL_ICOSAHEDRON,
    MODEL_CUBE_INVERTED,
    MODEL_LAST
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
    0,4,1,
    0,3,4,
    0,2,3,
    0,1,2,
    1,4,3,
    1,3,2
};

static model_texcoord_t model_pyramid_texcoords[] = 
{
    0.5,0.5,
    0,0,
    1,0,
    1,1,
    0,1
};

static int model_pyramid_primatives[] =
{
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

static int model_ship1_primitives[] = {
    24,
    12,
    12,
    18,
    18,
};

#if 0
static model_texcoord_t model_ship1_texcoords[] = {
    // diamond polygon cockpit
    0.5, 1,
    0.4, 0.75, //1
    0.6, 0.75,
    0.5, 0.5, //3
    0.5, 0.75,
    0.5, 0.25,
    
    // wing 1
    0, 1-0.6, //6
    0, 1-0.9,
    
    //wing 2
    1, 1-0.6,
    1, 1-0.9,
    
    // engine 1 pyramid
    0.3, 1-1,
    0.25, 1-0.7,
    0.30, 1-0.75,
    0.35, 1-0.7,
    0.30, 1-0.75,
    
    // engine 2 pyramid
    1-0.3, 1-1,
    1-0.25, 1-0.7,
    1-0.30, 1-0.75,
    1-0.35, 1-0.7,
    1-0.30, 1-0.75,
    
};
#else
static model_texcoord_t model_ship1_texcoords[] = {
    // diamond polygon cockpit
    0.5, 1,
    0.4, 0.75, //1
    0.4, 0.75,
    0.5, /*0.5*/0.4, //3
    0.5, 0.75,
    0.5, 0.25,
    
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
#endif

/********************************************************/
static model_coord_t model_turret1[] =
{
    // center diamond
    0,0.5,0, // top       0
    0,-0.5,0, //bottom    1
    0,0,-0.8, //front     2
    0,0,0.5, //back       3
    -0.5,0,0, //left      4
    0.5,0,0, //right      5
    
    // left gun
    -0.5,-0.1,-1.0, //    6
    -0.5,0.1,-0.5, //     7
    -0.5,0.1,-1.0, //     8
    
    // right gun
    0.5,-0.1,-1.0, //     9
    0.5,0.1,-0.5,  //     10
    0.5,0.1,-1.0,  //     11
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

static int model_turret_primitives[] = {
    24,
    6,
    6,
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
    0, 0.1, 0,
    0, -0.1, 0,
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

static int model_bullet_primitives[] =
{
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

static int model_cube_primitives[] = 
{
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

static int model_square_primatives[] =
{
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
static int model_surface_primitives[] =
{
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
static int model_sprite_primitives[] =
{
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

static int model_vertical_pillar_primitives[] = 
{
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

static int model_ship2_primitives[] = {
    24,
    18,
    18,
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

static int model_missle_primitives[] =
{
};


/********************************************************/
static model_coord_t model_ship3[] =
{
    // diamond polygon cockpit
    0, 0, -1,
    -0.15, 0, -0.5,
    0.15, 0, -0.5,
    0, 0, 0.1,
    0, 0.15, -0.5,
    0, -0.1, -0.5,
    
    // diamond engine right
    0.4, 0, -0.6,
    0.3, 0, -0.4,
    0.5, 0, -0.4,
    0.4, 0, -0.15,
    0.4, 0.1, -0.4,
    0.4, -0.1, -0.4,
    
    // wing tip right
    1.0, -0.1, -0.55,
    1.0, -0.1, -0.40,

    //... flip
    // diamond engine left
    -0.4, 0, -0.6,
    -0.3, 0, -0.4,
    -0.5, 0, -0.4,
    -0.4, 0, -0.15,
    -0.4, 0.1, -0.4,
    -0.4, -0.1, -0.4,
    
    // wing tip left
    -1.0, -0.1, -0.55,
    -1.0, -0.1, -0.40,
    
    // some bullshit points to increase bounding extent
    0.0, 0.5, 0,
    0.0, -0.5, 0
    
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

static int model_ship3_primitives[] = {
    24,
    24,
    24,
    24,
    24
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

/********************************************************/

/********************************************************/
static model_coord_t model_icosahedron[] =
{
    0.0,  1,  0.0, // 1 (top)

    0.0, 0.5, 0.5,
    0.95, 0.5, 0.31,
    0.59, 0.5, -0.81,
    -0.59, 0.5, -0.81,
    -0.95, 0.5, 0.31,
    
    0.63, -0.5, 0.81,
    0.95, -0.5, -0.31,
    0.00, -0.5, -0.99,
    -0.95, -0.5, -0.30,
    -0.57, -0.5, 0.81,
    
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

static int model_icosahedron_primitives[] =
{
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

/********************************************************/


static inline void
models_iterate()
{
    int l;
    
    l = model_cube[0];
    l = model_cube_indices[0];
    l = model_cube_primitives[0];
    l = model_cube_texcoords[0];
    l = model_cube_texcoords_alt[0];
    
    l = model_vertical_pillar[0];
    l = model_vertical_pillar_indices[0];
    l = model_vertical_pillar_primitives[0];
    
}
/********************************************************/

/********************************************************/
									   

#endif /* MODELS_H */