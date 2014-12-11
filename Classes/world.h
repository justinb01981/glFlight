//
//  world.h
//  gl_flight
//
//  Created by Justin Brady on 12/30/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef __world_h__
#define __world_h__

#include "models.h"
#include "object.h"
#include "worldElem.h"
#include "gameLock.h"

#include "mesh.h"
#include "simple_list.h"
#include "simple_hash.h"
#include "gameBounding.h"


// MUST DIVIDE bound_x, bound_y, bound_z evenly!
#define WORLD_MAX_REGIONS /*50*/ 25
#define WORLD_MAX_PLANES 16
#define WORLD_VIS_REGIONS_X 4
#define WORLD_VIS_REGIONS_Y 2
#define WORLD_VIS_REGIONS_Z 4
#define VIS_REGION_FOR_COORD(v, x, y, z)                  \
    v[0] = x / (gWorld->bound_x / WORLD_VIS_REGIONS_X);   \
    v[1] = y / (gWorld->bound_y / WORLD_VIS_REGIONS_Y);   \
    v[2] = z / (gWorld->bound_z / WORLD_VIS_REGIONS_Z);
#define VIS_COORD_VALID(x, y, z)                              \
    (x >= 0 && x < gWorld->bound_x && \
     y >= 0 && y < gWorld->bound_y && \
     z >= 0 && z < gWorld->bound_z && \
     gWorld->bound_x > 0 && gWorld->bound_y > 0 && gWorld->bound_z > 0)



extern float visible_distance;

typedef struct
{
    WorldElemListNode elements_list; // master list of all elements in existence
    
    WorldElemListNode elements_by_region[WORLD_MAX_REGIONS][WORLD_MAX_REGIONS][WORLD_MAX_REGIONS];
    
    WorldElemListNode elements_moving;
    
    WorldElemListNode elements_expiring;
    
    WorldElemListNode elements_collided;
    
    WorldElemListNode elements_to_be_freed;
    WorldElemListNode elements_to_be_added;
    
    WorldElemListNode elements_visible;
    
    WorldElemListNode elements_visible_by_region[WORLD_VIS_REGIONS_X][WORLD_VIS_REGIONS_Y][WORLD_VIS_REGIONS_Z];
    
    //WorldElemListNode elements_by_type[MODEL_LAST];
    
    WorldElem* last_elem_added;
    
    model_coord_t *shared_coordinates;
    model_texcoord_t* shared_texcoords;
    
    int elem_id_next;
	
	float bound_x, bound_y, bound_z;
    
    float region_size_x, region_size_y, region_size_z;
    
    boundingRegion* boundingRegion;
    
    simple_hash_table_t *element_id_hash;
    
    struct
    {
        float o[3];
        float v1[3], v2[3];
    } world_planes[WORLD_MAX_PLANES];
    
    float vec_gravity[3];
    
    float vec[6];
    
    int ignore_add:1;
    int ignore_remove:1;
    int visible_list_change:1;
	
} world_t;

extern world_t *gWorld;

void world_init(float size_x, float size_y, float size_z);

void world_update(float tc);

void world_clear_pending();

void world_free();

void world_optimize();

void
world_vis_rgn_add(float vis_dist, WorldElem* elem);

void
world_vis_rgn_remove(WorldElem* elem);

int world_add_object(Model type, float x, float y, float z, float euler_alpha, float euler_beta, float euler_gamma, float scale, int texture_id);

void world_remove_object(int elem_id);

int world_replace_object(int elem_id, Model type, float x, float y, float z, float yaw, float pitch, float roll, float scale, int texture_id);

WorldElem* world_get_object_n_at_loc(int n, float x, float y, float z, float range);

WorldElem* world_find_elem_with_attrs(WorldElemListNode* head, int object_type, int affiliation);

WorldElem* world_get_last_object();

void world_move_elem(WorldElem* pElem, float x, float y, float z, int relative);

void world_repulse_elem(WorldElem* pCollisionFast, WorldElem* pCollisionSlow, float tc);

void
world_add_mesh(float x, float y, float z,
               float dx_r, float dy_r, float dz_r,
               float dx_c, float dy_c, float dz_c,
               unsigned int mesh_r, unsigned int mesh_c,
               float vis_norm_x, float vis_norm_y, float vis_norm_z,
               int tex_id, float tiling_m);

int
world_prepare_mesh(float x, float y, float z,
                   float dx_r, float dy_r, float dz_r,
                   float dx_c, float dy_c, float dz_c,
                   unsigned int mesh_r, unsigned int mesh_c);

void
world_set_mesh_object_info(float a, float b, float g);

void
world_complete_mesh(int type, int tex_id, float tiling_m, float scale);

void
world_manip_mesh(float xy_coord[2], float d_xyz[3], float m_c);

float
world_mesh_pending_x();

float
world_mesh_pending_y();

void
world_set_plane(int idx, float ox, float oy, float oz, float x1, float y1, float z1, float x2, float y2, float z2);

void
world_random_spawn_location(float loc[6], int affiliation);

int update_object_velocity(int object_id, float x, float y, float z, int relative);

int update_object_velocity_with_friction(int object_id, float v[3], float cthrust, float cfriction);

int world_object_set_lifetime(int object_id, int tex_passes);

void world_object_set_nametag(int object_id, char* nametag);

void world_lock_init();

void world_lock_destroy();

void world_lock();
void world_unlock();

#endif /* __world_h__ */
