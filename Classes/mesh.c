//
//  mesh.c
//  gl_flight
//
//  Created by jbrady on 9/7/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mesh.h"

struct mesh_t* 
build_mesh(float x, float y, float z,
           float dx_r, float dy_r, float dz_r,
           float dx_c, float dy_c, float dz_c,
           unsigned int mesh_c, unsigned int mesh_r)
{
    struct mesh_t* mesh = malloc(sizeof(struct mesh_t));
    int row, col;
    if(!mesh) return NULL;
    int ccw = 0;
    
    mesh->dim_x = mesh_c;
    mesh->dim_y = mesh_r;
    
    // calculate plane normal (cross product of row/column vectors)
    float a1 = dx_r, a2 = dy_r, a3 = dz_r;
    float b1 = dx_c, b2 = dy_c, b3 = dz_c;
    mesh->normal.x = a2*b3 - a3*b2;
    mesh->normal.y = a3*b1 - a1*b3;
    mesh->normal.z = a1*b2 - a2*b1;
    
    unsigned int n_triangles = mesh->dim_x * mesh->dim_y * 2;
    if(n_triangles == 0) 
    {
        free(mesh);
        return NULL;
    }
    
    mesh->triangles = malloc(sizeof(struct mesh_triangle_t) * n_triangles);
    if(!mesh->triangles)
    {
        free(mesh);
        return NULL;
    }
    
    mesh->n_row_coords = 1 + (mesh->dim_x);
    unsigned int n_coordinates = (1+mesh->dim_y) * mesh->n_row_coords;
    
    mesh->coordinates = malloc(sizeof(struct mesh_coordinate_t) * (n_coordinates));
    if(!mesh->coordinates)
    {
        free(mesh->triangles);
        free(mesh);
        return NULL;
    }
    mesh->n_coordinates = n_coordinates;
    
    mesh->round_inc_x = mesh->round_inc_y = mesh->round_inc_z = 0;
    
    unsigned int triangle_n = 0;
    
    for(row = 0; row <= mesh->dim_y-1; row++)
    {
        for(col = 0; col <= mesh->dim_x-1; col++)
        {
            struct mesh_triangle_t t;
            
            // we want counter-clockwise winding
            if(ccw)
            {
                t.points[0].x = col; t.points[0].y = row; // 0
                t.points[1].x = col; t.points[1].y = row+1; // 1
                t.points[2].x = col+1; t.points[2].y = row; // 2
            }
            else
            {
                t.points[0].x = col; t.points[0].y = row; // 0
                t.points[1].x = col+1; t.points[1].y = row; // 1
                t.points[2].x = col; t.points[2].y = row+1; // 2
            }
            
            mesh->triangles[triangle_n] = t;
            triangle_n++;
            
            if(ccw)
            {
                t.points[0].x = col+1; t.points[0].y = row; // 2
                t.points[1].x = col; t.points[1].y = row+1; // 1
                t.points[2].x = col+1; t.points[2].y = row+1; // 3
            }
            else
            {
                t.points[0].x = col; t.points[0].y = row+1; // 2
                t.points[1].x = col+1; t.points[1].y = row; // 1
                t.points[2].x = col+1; t.points[2].y = row+1; // 3
            }
            
            mesh->triangles[triangle_n] = t;
            triangle_n++;
        }
    }
    mesh->n_triangles = triangle_n;
    
    struct mesh_coordinate_t coord = {x, y, z};
    for(row = 0; row <= mesh->dim_y; row++)
    {
        struct mesh_coordinate_t coord_cur_r = coord;
        
        coord_cur_r.x += dx_r * row;
        coord_cur_r.y += dy_r * row;
        coord_cur_r.z += dz_r * row;
        
        for(int col = 0; col <= mesh->dim_x; col++)
        {
            struct mesh_coordinate_t coord_cur = coord_cur_r;
            
            float fcol = col;
            
            coord_cur.x += (dx_c*fcol);
            coord_cur.y += (dy_c*fcol);
            coord_cur.z += (dz_c*fcol);
            
            mesh->coordinates[MESH_COORD_IDX(mesh, col, row)] = coord_cur;
        }
    }
    
    return mesh;
}

static float meshround(float v, float m)
{
    v = v / m;
    float r = v - floor(v);
    
    if(fabs(r) >= 0.5) v += (v/fabs(v));
    
    return floor(v) * m;
}

void
apply_mesh_rounding(struct mesh_t* mesh, float dx, float dy, float dz)
{
    mesh->round_inc_x = dx;
    mesh->round_inc_y = dy;
    mesh->round_inc_z = dz;
    
    int r;
    for(r = 0; r < mesh->dim_y; r++)
    {
        int c;
        for(c = 0; c < mesh->dim_x; c++)
        {
            if(mesh->round_inc_x)
                mesh->coordinates[MESH_COORD_IDX(mesh, r, c)].x =
                meshround(mesh->coordinates[MESH_COORD_IDX(mesh, r, c)].x, mesh->round_inc_x);
            if(mesh->round_inc_y)
                mesh->coordinates[MESH_COORD_IDX(mesh, r, c)].y =
                meshround(mesh->coordinates[MESH_COORD_IDX(mesh, r, c)].y, mesh->round_inc_y);
            if(mesh->round_inc_z)
                mesh->coordinates[MESH_COORD_IDX(mesh, r, c)].z =
                meshround(mesh->coordinates[MESH_COORD_IDX(mesh, r, c)].z, mesh->round_inc_z);
        }
    }
}

void
pull_mesh(struct mesh_t* mesh,
          unsigned int coord_x, unsigned int coord_y,
          float dx,
          float dy,
          float dz,
          float transfer_c)
{
    int r;
    int c;
    for(r = 0; r < mesh->dim_y; r++)
    {
        for(c = 0; c < mesh->dim_x; c++)
        {
            float a = fabs((float) r - coord_x);
            float b = fabs((float) c - coord_y);
            float dc = sqrt(a*a + b*b);
            float tc = transfer_c;
            
            if(a == 0 && b == 0)
            {
                tc = 1;
                dc = 1;
            }
            
            mesh->coordinates[MESH_COORD_IDX(mesh, r, c)].x += ((1.0/dc) * tc) * dx;
            mesh->coordinates[MESH_COORD_IDX(mesh, r, c)].y += ((1.0/dc) * tc) * dy;
            mesh->coordinates[MESH_COORD_IDX(mesh, r, c)].z += ((1.0/dc) * tc) * dz;
        }
    }
}

void free_mesh(struct mesh_t* mesh)
{
    if(mesh->triangles) free(mesh->triangles);
    if(mesh->coordinates) free(mesh->coordinates);
    free(mesh);
}

void
mesh_opengl_index_sort(float pos[3], struct mesh_opengl_t* ogl_mesh)
{
    unsigned int i;
    int c = 3;
    for(i = 0; i < ogl_mesh->n_indices-c; i += 3)
    {
        
        float da = ((ogl_mesh->coords[ogl_mesh->indices[i]] - pos[0]) * (ogl_mesh->coords[ogl_mesh->indices[i]] - pos[0])) +
                   ((ogl_mesh->coords[ogl_mesh->indices[i]+1] - pos[1]) * (ogl_mesh->coords[ogl_mesh->indices[i]+1] - pos[1])) +
                   ((ogl_mesh->coords[ogl_mesh->indices[i]+2] - pos[2]) * (ogl_mesh->coords[ogl_mesh->indices[i]+2] - pos[2]));
        float db = ((ogl_mesh->coords[ogl_mesh->indices[i+c]] - pos[0]) * (ogl_mesh->coords[ogl_mesh->indices[i+c]] - pos[0])) +
                    ((ogl_mesh->coords[ogl_mesh->indices[i+c]+1] - pos[1]) * (ogl_mesh->coords[ogl_mesh->indices[i+c]+1] - pos[1])) +
                    ((ogl_mesh->coords[ogl_mesh->indices[i+c]+2] - pos[2]) * (ogl_mesh->coords[ogl_mesh->indices[i+c]+2] - pos[2]));
        
        if(da < db) {
            // swap
            unsigned int tmp;	
            tmp = ogl_mesh->indices[i];
            ogl_mesh->indices[i] = ogl_mesh->indices[i+c]; ogl_mesh->indices[i+c] = tmp;
            tmp = ogl_mesh->indices[i+1];
            ogl_mesh->indices[i+1] = ogl_mesh->indices[i+c+1]; ogl_mesh->indices[i+c+1] = tmp;
            tmp = ogl_mesh->indices[i+2];
            ogl_mesh->indices[i+2] = ogl_mesh->indices[i+c+2]; ogl_mesh->indices[i+c+2] = tmp;
            continue;
        }
    }
}

struct mesh_opengl_t*
mesh_to_opengl_triangles(struct mesh_t* mesh, float tx_m, float ty_m)
{
    struct mesh_opengl_t* ogl_mesh;
    
    ogl_mesh = malloc(sizeof(*ogl_mesh));
    if(!ogl_mesh) return NULL;
    
    ogl_mesh->coords = malloc(sizeof(*(ogl_mesh->coords)) * mesh->n_coordinates * 3);
    ogl_mesh->indices = malloc(sizeof(*(ogl_mesh->indices)) * mesh->n_triangles * 3);
    ogl_mesh->tex_coords = malloc(sizeof(*(ogl_mesh->tex_coords)) * mesh->n_coordinates * 2);
    
    ogl_mesh->n_indices = 0;
    
    // build indices from triangles
    for(unsigned int t = 0; t < mesh->n_triangles; t++)
    {
        unsigned int coord_idx;
        
        coord_idx = MESH_COORD_IDX(mesh, mesh->triangles[t].points[0].x,
                                   mesh->triangles[t].points[0].y);
        ogl_mesh->indices[t*3] = coord_idx;
        
        coord_idx = MESH_COORD_IDX(mesh, mesh->triangles[t].points[1].x,
                                   mesh->triangles[t].points[1].y);
        ogl_mesh->indices[t*3+1] = coord_idx;
        
        coord_idx = MESH_COORD_IDX(mesh, mesh->triangles[t].points[2].x,
                                   mesh->triangles[t].points[2].y);
        ogl_mesh->indices[t*3+2] = coord_idx;
        
        ogl_mesh->n_indices += 3;
    }
    
    // build coordinates from coordinates
    for(unsigned int c = 0; c < mesh->n_coordinates; c++)
    {
        ogl_mesh->coords[c*3] = mesh->coordinates[c].x;
        ogl_mesh->coords[c*3+1] = mesh->coordinates[c].y;
        ogl_mesh->coords[c*3+2] = mesh->coordinates[c].z;
        
        mesh_glfloat_t tx;
        if(MESH_X_FOR_COORD(mesh, c) == 0) tx = 0;
        else tx = (float) MESH_X_FOR_COORD(mesh, c)/(mesh->dim_x*tx_m);
        
        mesh_glfloat_t ty;
        if(MESH_Y_FOR_COORD(mesh, c) == 0) ty = 0;
        else ty = (float) MESH_Y_FOR_COORD(mesh, c)/(mesh->dim_y*ty_m);
        
        ogl_mesh->tex_coords[c*2] = tx;
        ogl_mesh->tex_coords[c*2+1] = ty;
    }
    
    return ogl_mesh;
}

void
free_mesh_opengl(struct mesh_opengl_t* ogl_mesh)
{
    if(!ogl_mesh) return;
    
    if(ogl_mesh->coords) free(ogl_mesh->coords);
    if(ogl_mesh->indices) free(ogl_mesh->indices);
    if(ogl_mesh->tex_coords) free(ogl_mesh->tex_coords);
    
    free(ogl_mesh);
}


/**
 // possiblw "triangle-fan" like solution
 8-----7-----6
 | \   |   / |
 |  \  |  /  |
 |   \ | /   |
 |    \|/    |
 0-----1-----5
 |    /|\    |
 |   / | \   |
 |  /  |  \  |
 | /   |   \ |
 2-----3-----4
 0,2,1
 2,3,1
 3,4,1
 4,5,1
 5,6,1
 6,7,1
 7,8,1
 8,0,1 <-- end-step
 
 // possible triangle-strip solution
 0-----1-----4
 |    /|    /|
 |   / |   / |
 |  /  |  /  |
 | /   | /   |
 2-----3-----5
       |\    |
       | \   |
       |  \  |
       |   \ |
       7-----6
 
1,0,2
3,1,2  m=1 // (next, read prev left-right, skip m)
4,1,3  m=2 // (next, read prev right-left, skip m)
5,4,3  m=1
            // (end row, move backwards)
6,5,3  m=2
7,6,3  m=1
8,7,3  m=2

 **/
TESS_BEGIN_FUNCTION
{
    S->Icur = S->Is;
    S->Mcur = S->Ms;
    S->Tcur = S->Ts;
    S->Imir = S->Icur;
    
    S->Ist = NULL;
    
    // current direction = U+
    S->Id = 1;
    S->Idc = 0;
    
    // store next set of vertices
    *(S->Icur) = 0; S->Icur += 3;
    S->Inext = 1;
    
    // store model (x,y,z) origin
    *S->Mcur = Mo[0]; S->Mcur++; *S->Mcur = Mo[1]; S->Mcur++; *S->Mcur = Mo[2]; S->Mcur++;
    
    // store texture (u,v) origin
    *S->Tcur = To[0]; S->Tcur++; *S->Tcur = To[1]; S->Tcur++;
}

TESS_STEPU_FUNCTION
{
    *S->Mcur = Mu[0]; S->Mcur++; *S->Mcur = Mu[1]; S->Mcur++; *S->Mcur = Mu[2]; S->Mcur++;
    
    *S->Tcur = Tu[0]; S->Tcur++; *S->Tcur = Tu[1]; S->Tcur++;
    
    if(S->Idc)
    {
        // store triangles
        *(S->Icur) = S->Inext;
        *(S->Icur+1) = S->Imir;
        *(S->Icur+2) = S->Inext-1;
        
        if(S->Id < 0)
        {
            unsigned int tmp = *(S->Icur+1);
            *(S->Icur+1) = *(S->Icur+2);
            *(S->Icur+2) = tmp;
        }
        S->Icur += 3;
        //printf("TESS_STEPU[0]: %d %d %d\n", *(S->Icur-3), *(S->Icur-2), *(S->Icur-1));
        
         S->Inext += 1;
        
        *(S->Icur) = S->Inext;
        *(S->Icur+1) = S->Imir;
        *(S->Icur+2) = S->Inext-1;
        
        if(S->Id < 0)
        {
            unsigned int tmp = *(S->Icur+1);
            *(S->Icur+1) = *(S->Icur+2);
            *(S->Icur+2) = tmp;
        }
        S->Icur += 3;
        
        S->Inext += 1;
        
        //printf("TESS_STEPU[1]: %d %d %d\n", *(S->Icur-3), *(S->Icur-2), *(S->Icur-1));
    }
    else
    {
        // store triangles
        *(S->Icur) = S->Inext;
        *(S->Icur+1) = S->Inext-1;
        *(S->Icur+2) = S->Imir+1;
        
        if(S->Id > 0)
        {
            unsigned int tmp = *(S->Icur+1);
            *(S->Icur+1) = *(S->Icur+2);
            *(S->Icur+2) = tmp;
        }
        S->Icur += 3;
        //printf("TESS_STEPU[0]: %d %d %d\n", *(S->Icur-3), *(S->Icur-2), *(S->Icur-1));
        
        *(S->Icur) = S->Inext;
        *(S->Icur+1) = S->Imir;
        *(S->Icur+2) = S->Imir+1;
        
        if(S->Id < 0)
        {
            unsigned int tmp = *(S->Icur+1);
            *(S->Icur+1) = *(S->Icur+2);
            *(S->Icur+2) = tmp;
        }
        S->Icur += 3;
        
        //printf("TESS_STEPU[1]: %d %d %d\n", *(S->Icur-3), *(S->Icur-2), *(S->Icur-1));
        
        S->Inext += 1;
    }
    
    S->Imir -= 1;
    S->Idc = 0;
}

TESS_STEPV_FUNCTION
{
    //printf("TESS_STEPV[0]: %d %d %d\n", *(S->Icur-3), *(S->Icur-2), *(S->Icur-1));
    *S->Mcur = Mu[0]; S->Mcur++; *S->Mcur = Mu[1]; S->Mcur++; *S->Mcur = Mu[2]; S->Mcur++;
    
    *S->Tcur = Tu[0]; S->Tcur++; *S->Tcur = Tu[1]; S->Tcur++;
    
    S->Imir = S->Inext-2;
    //S->Inext++;
    
    //S->Icur += 3;
    
    // move to next row
    if(!S->Ist)
    {
        S->Ist = S->Icur;
    }
    S->Id = -S->Id;
    S->Idc = 1;
    //printf("TESS_STEPV[1]: %d %d %d\n", *(S->Icur-3), *(S->Icur-2), *(S->Icur-1));
}

TESS_END_FUNCTION
{
}

TESS_WALK_FUNCTION
{
    cb(S->Ms, S->Ts, S->Ist, S->Icur - S->Ist);
}
