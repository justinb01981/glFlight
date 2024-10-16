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
//#define WORLD_MAX_REGIONS /*50*/ 25
#define WORLD_MAX_PLANES 16
#define WORLD_MAX_TRIANGLE_MESH 16
#define WORLD_BOUNDING_SPHERE_STEPS ((float) 8)
#define WORLD_TERRAIN_COMPLEXITY 64

extern float visible_distance;

typedef struct {
    WorldElemListNode* ptr_objects_moving;
    WorldElemListNode* world_region_iterate_cur, *world_region_iterate_retry_head;
    WorldElemListNode* collision_recs_iterate_cur;
    
    void (*world_remove_hook)(WorldElem* pElem);
    void (*world_rgn_remove_hook)(WorldElem* pElem);
} world_update_state_t;

typedef struct
{
    WorldElemListNode elements_list; // master list of all elements in existence
    
    //WorldElemListNode elements_by_region[WORLD_MAX_REGIONS][WORLD_MAX_REGIONS][WORLD_MAX_REGIONS];
    
    WorldElemListNode elements_moving;

    WorldElemListNode elements_intelligent;
    
    WorldElemListNode elements_expiring;
    
    WorldElemListNode elements_collided;
    
    WorldElemListNode elements_to_be_freed;
    WorldElemListNode elements_to_be_added;
    
    WorldElemListNode elements_visible;
    
    //WorldElemListNode elements_visible_by_region[WORLD_VIS_REGIONS_X][WORLD_VIS_REGIONS_Y][WORLD_VIS_REGIONS_Z];
    //WorldElemListNode *elements_by_region_vis;
    
    //WorldElemListNode elements_by_type[MODEL_LAST];
    
    WorldElem* last_elem_added;
    
    int elem_id_next;
	
	//float bound_x, bound_y, bound_z;
    
    float bound_radius;
    
    float regions_x, regions_y, regions_z;
    float region_size;
    
    boundingRegion* boundingRegion;
    
    float *terrain_height_map;
    
    simple_hash_table_t *element_id_hash;
    
    struct
    {
        float o[3];
        float v1[3], v2[3];
    } world_planes[WORLD_MAX_PLANES];
    
    /*
    struct world_triangle_mesh_head
    {
        struct mesh_opengl_t* glmesh;
        int tex_id;
    } world_triangle_meshes[WORLD_MAX_TRIANGLE_MESH];
    int world_triangle_meshes_n;
     */
    
    WorldElemListNode triangle_mesh_head;
    WorldElemListNode drawline_list_head;
    
    float vec_gravity[3];
    
    float vec[6];

    int ignore_remove:1;
    int visible_list_change:1;
    
    float vis_regions_x, vis_regions_y, vis_regions_z;
    
    WorldElemListNode *elements_by_region;
    
    world_update_state_t world_update_state;
	
} world_t;

extern world_t *gWorld;

void region_for_coord(float x, float y, float z, int rgn[3]);

WorldElemListNode*
world_region_head(float x, float y, float z);

WorldElemListNode*
world_vis_region_head(float x, float y, float z);

void world_init(float radius);

void world_update(float tc);

void world_clear_pending(void);

void world_free(void);

void world_build_run_program(float x, float y, float z);

int world_add_object(Model type, float x, float y, float z, float euler_alpha, float euler_beta, float euler_gamma, float scale, int texture_id);

void world_remove_object(int elem_id);

int world_replace_object(int elem_id, Model type, float x, float y, float z, float yaw, float pitch, float roll, float scale, int texture_id);

void
world_replace_add_object(int* elem_id_storage, Model type,
                         float x, float y, float z,
                         float yaw, float pitch, float roll,
                         float scale, int texture_id,
                         WorldElem** elem_out);

WorldElem* world_get_object_n_at_loc(int n, float x, float y, float z, float range);

WorldElem* world_find_elem_with_attrs(WorldElemListNode* head, int object_type, int affiliation);

WorldElem* world_get_last_object(void);

void world_move_elem(WorldElem* pElem, float x, float y, float z, int relative);

void world_repulse_elem(WorldElem* pCollisionFast, WorldElem* pCollisionSlow, float tc, float Frepulse);

int world_bounding_violations(float location[3], float vnormal[3]);

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

void
world_manip_mesh_round(float x, float y, float z);

float
world_mesh_pending_x(void);

float
world_mesh_pending_y(void);

void
world_convert_mesh_to_gltriangles(int tex_id);

void
world_set_plane(int idx, float ox, float oy, float oz, float x1, float y1, float z1, float x2, float y2, float z2);

void
world_add_drawline(float a[3], float b[3], float color[3], unsigned int lifetime);

void
world_random_spawn_location(float loc[6], int affiliation);

int update_object_velocity(int object_id, float x, float y, float z, int relative);

int update_object_velocity_with_friction(int object_id, float v[3], float cthrust, float cfriction);

int world_object_set_lifetime(int object_id, int tex_passes);

void world_object_set_nametag(int object_id, char* nametag);

void world_lock_init(void);

void world_lock_destroy(void);

void world_lock(void);
void world_unlock(void);

#endif /* __world_h__ */
