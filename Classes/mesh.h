//
//  mesh.h
//  gl_flight
//
//  Created by jbrady on 9/7/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef gl_flight_mesh_h
#define gl_flight_mesh_h

#define MESH_COORD_IDX(mesh, x, y) ((((mesh)->n_row_coords)*(y))+(x))
#define MESH_X_FOR_COORD(mesh, idx) ((idx)%((mesh)->n_row_coords))
#define MESH_Y_FOR_COORD(mesh, idx) ((idx)/((mesh)->n_row_coords))

//#include <OpenGLES/EAGL.h>
#include "models.h"

// TODO: include opengl types
typedef /*GLfloat*/ float mesh_glfloat_t;
typedef /*GLubyte*/ unsigned char mesh_glubyte_t;

struct mesh_point_t {
    model_index_t x;
    model_index_t y;
};

struct mesh_triangle_t {
    struct mesh_point_t points[3];
};

struct mesh_coordinate_t {
    float x, y, z;
};

struct mesh_t {
    struct mesh_triangle_t* triangles;
    struct mesh_coordinate_t* coordinates;
    struct mesh_coordinate_t normal;
    unsigned int dim_x, dim_y; // in rows/cols
    unsigned int n_row_coords;
    unsigned int n_triangles;
    unsigned int n_coordinates;
    float round_inc_x;
    float round_inc_y;
    float round_inc_z;
};

struct mesh_opengl_t
{
    mesh_glfloat_t *coords;
    /*mesh_glubyte_t*/model_index_t *indices;
    unsigned int n_indices;
    mesh_glfloat_t *tex_coords;
    unsigned int u;
};

struct tess_storage_t
{
    model_index_t *Is, *Icur, *Ist;
    float *Ms, *Mcur;
    float *Ts, *Tcur;
    model_index_t Inext, Imir;
    int Id;
    int Idc;
};

struct mesh_t* 
build_mesh(float x, float y, float z,
           float dx_r, float dy_r, float dz_r,
           float dx_c, float dy_c, float dz_c,
           unsigned int mesh_r, unsigned int mesh_c);

void
apply_mesh_rounding(struct mesh_t* mesh, float dx, float dy, float dz);

void
pull_mesh(struct mesh_t* mesh,
          unsigned int coord_x, unsigned int coord_y,
          float dx,
          float dy,
          float dz,
          float transfer_c);

struct mesh_opengl_t*
mesh_to_opengl_triangles(struct mesh_t*, float tx_m, float ty_m);

void
mesh_opengl_index_sort(float pos[3], struct mesh_opengl_t* ogl_mesh);

void
free_mesh_opengl(struct mesh_opengl_t* ogl_mesh);

void
free_mesh(struct mesh_t* mesh);

#define TESS_BEGIN_FUNCTION \
void \
tess_begin(float Mo[3], float To[2], \
           struct tess_storage_t* S)

extern TESS_BEGIN_FUNCTION;

#define TESS_STEPU_FUNCTION \
void \
tess_step(float Mu[3], float Tu[2], \
          struct tess_storage_t* S)
TESS_STEPU_FUNCTION;

#define TESS_STEPV_FUNCTION \
void \
tess_step_row(float Mu[3], float Tu[2], \
struct tess_storage_t* S)
TESS_STEPV_FUNCTION;

#define TESS_END_FUNCTION \
void \
tess_end(struct tess_storage_t* S)
TESS_END_FUNCTION;

#define TESS_WALK_FUNCTION \
void \
tess_walk(struct tess_storage_t* S, \
          void (*cb)(float*, float*, unsigned int*, unsigned long))
TESS_WALK_FUNCTION;

#endif
