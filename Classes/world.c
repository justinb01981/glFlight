//
//  world.m
//  gl_flight
//
//  Created by Justin Brady on 12/30/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>

#include "world.h"
#include "quaternions.h"
#include "gameAudio.h"
#include "mesh.h"
#include "gameUtils.h"
#include "gameGlobals.h"
#include "gameBounding.h"
#include "gameNetwork.h"
#include "gameLock.h"
#include "collision.h"
#include "sounds.h"

#define MAX(x, y) ((x) > (y)? (x): (y))


void world_move_elem(WorldElem* pElem, float x, float y, float z, int relative);
WorldElemListNode* add_element_to_region(WorldElem* pElem);
WorldElemListNode* add_element_to_region_for_coord(WorldElem* pElem, model_coord_t x, model_coord_t y, model_coord_t z);
WorldElemListNode* get_region_list_head(float x, float y, float z);
void remove_element_from_region(WorldElem* pElem);
static void get_element_bounding_box(WorldElem* pElem, float box[6]);
inline static int check_bounding_box_overlap(float boxA[6], float boxB[6]);
void update_regions();
void convert_mesh_to_world_elems(struct mesh_t* mesh, unsigned int texture_id, float tile_div, struct mesh_coordinate_t* vis_normal);
static void world_pack_geometry();
void world_build_visibility_data();
void world_update(float tc);

world_t *gWorld = NULL;
game_lock_t gWorldLock;

float C_THRUST = /*0.05*/ 0.050; // higher values = more speed
float C_FRICTION = /*0.04*/ 0.03; // higher values = more friction
float GYRO_FEEDBACK = GYRO_FEEDBACK_DEFAULT;
float GYRO_DC = GYRO_DC_DEFAULT;
float visible_distance = VISIBLE_DISTANCE_PLATFORM;

static struct mesh_t* world_pending_mesh = NULL;
static float world_pending_mesh_info[3];

static float check_bounding_box_overlap_result[6];

models_shared_t models_shared;

static int
world_add_object_core(Model type,
                      float x, float y, float z,
                      float alpha, float beta, float gamma,
                      float scale,
                      int texture_id,
                      int replace_id)
{
	int i;
    model_coord_t* model_coords;
    int model_sizeof;
    model_index_t* model_indices;
    int model_indices_sizeof;
    model_texcoord_t* model_texcoords;
    int model_texcoords_sizeof;
    int model_primitives_sizeof;
    int* model_primitives;
    int new_durability = 1;
    int model_changed = 0;
    WorldElem* pElem;
    
    MODEL_POLY_COMPONENTS_DECLARE();
    
    MODEL_POLY_COMPONENTS_INIT();
    
    if(type >= MODEL_LAST) return WORLD_ELEM_ID_INVALID;
    
    if(replace_id >= 0)
    {
        // removing an existing element (and linked elems)
        // must be of the same type as original
        // must ensure that linked elems stay linked
        
        WorldElemListNode* pIDNode = NULL;
        // cache frequently-replaced elements
        if(!pIDNode) pIDNode = world_elem_list_find(replace_id, &gWorld->elements_list);
        if(!pIDNode) return WORLD_ELEM_ID_INVALID;
        
        pElem = pIDNode->elem;
        
        if(type != pElem->type)
        {
            /* not allowing this for now */
            //model_changed = 1;
            type = pElem->type;
        }
        
        remove_element_from_region(pElem);
    }
    else
    {
        if(MODEL_EXTENDED_MEMORY(type))
        {
            pElem = world_elem_alloc_extended_model();
        }
        else
        {
            pElem = world_elem_alloc();
        }

        if(!pElem) return WORLD_ELEM_ID_INVALID;
        
        world_elem_init(pElem, gWorld->elem_id_next);
        gWorld->elem_id_next++;
        
        // 11-8-2012: changed both of these to add-fast to speed things up
        world_elem_list_add_fast(pElem, &gWorld->elements_list, LIST_TYPE_UNKNOWN);
        world_elem_list_add_fast(pElem, &gWorld->elements_to_be_added, LIST_TYPE_UNKNOWN);
    }
    
    pElem->physics.ptr = &pElem->physics.data;
    pElem->physics.ptr->x = x;
    pElem->physics.ptr->y = y;
    pElem->physics.ptr->z = z;
    pElem->physics.ptr->alpha = alpha;
    pElem->physics.ptr->beta = beta;
    pElem->physics.ptr->gamma = gamma;
    pElem->type = type;
    pElem->texture_id = texture_id;
    pElem->scale = scale;
    pElem->destructible = 1;
    pElem->renderInfo.visible = 1;
    pElem->spans_regions = 1;
	
	switch (type)
	{
        case MODEL_CUBE_INVERTED:
            model_coords = model_cube;
            model_sizeof = sizeof(model_cube);
            model_indices = model_cube_indices_inverted;
            model_indices_sizeof = sizeof(model_cube_indices);
            model_texcoords = model_cube_texcoords_alt /*model_cube_texcoords*/;
            model_texcoords_sizeof = sizeof(model_cube_texcoords);
            model_primitives_sizeof = sizeof(model_cube_primitives);
            model_primitives = model_cube_primitives;
            break;
            
        case MODEL_CUBE:
            model_coords = model_cube;
            model_sizeof = sizeof(model_cube);
            model_indices = model_cube_indices;
            model_indices_sizeof = sizeof(model_cube_indices);
            model_texcoords = model_cube_texcoords_alt /*model_cube_texcoords*/;
            model_texcoords_sizeof = sizeof(model_cube_texcoords);
            model_primitives_sizeof = sizeof(model_cube_primitives);
            model_primitives = model_cube_primitives;
            break;
            
    	case MODEL_CUBE2:
            model_coords = model_cube;
            model_sizeof = sizeof(model_cube);
            model_indices = model_cube_indices;
            model_indices_sizeof = sizeof(model_cube_indices);
            model_texcoords = model_cube_texcoords;
            model_texcoords_sizeof = sizeof(model_cube_texcoords);
            model_primitives_sizeof = sizeof(model_cube_primitives);
            model_primitives = model_cube_primitives;
            break;
        
        case MODEL_SCENERY:
        case MODEL_SQUARE:
            model_coords = model_square;
            model_sizeof = sizeof(model_square);
            model_indices = model_square_indices;
            model_indices_sizeof = sizeof(model_square_indices);
            model_texcoords = model_square_texcoords;
            model_texcoords_sizeof = sizeof(model_square_texcoords);
            model_primitives_sizeof = sizeof(model_square_primatives);
            model_primitives = model_square_primatives;
            break;
            
        case MODEL_PYRAMID:
            model_coords = model_pyramid;
            model_sizeof = sizeof(model_pyramid);
            model_indices = model_pyramid_indices;
            model_indices_sizeof = sizeof(model_pyramid_indices);
            model_texcoords = model_pyramid_texcoords;
            model_texcoords_sizeof = sizeof(model_pyramid_texcoords);
            model_primitives_sizeof = sizeof(model_pyramid_primatives);
            model_primitives = model_pyramid_primatives;
            break;
            
        case MODEL_SHIP1:
            model_coords = model_ship1;
            model_sizeof = sizeof(model_ship1);
            model_indices = model_ship1_indices;
            model_indices_sizeof = sizeof(model_ship1_indices);
            model_texcoords = model_ship1_texcoords;
            model_texcoords_sizeof = sizeof(model_ship1_texcoords);
            model_primitives_sizeof = sizeof(model_ship1_primitives);
            model_primitives = model_ship1_primitives;
            break;
            
        case MODEL_SHIP2:
            model_coords = model_ship2;
            model_sizeof = sizeof(model_ship2);
            model_indices = model_ship2_indices;
            model_indices_sizeof = sizeof(model_ship2_indices);
            model_texcoords = model_ship2_texcoords;
            model_texcoords_sizeof = sizeof(model_ship2_texcoords);
            model_primitives_sizeof = sizeof(model_ship2_primitives);
            model_primitives = model_ship2_primitives;
            break;
            
        case MODEL_SHIP3:
            model_coords = model_ship3;
            model_sizeof = sizeof(model_ship3);
            model_indices = model_ship3_indices;
            model_indices_sizeof = sizeof(model_ship3_indices);
            model_texcoords = model_ship3_texcoords;
            model_texcoords_sizeof = sizeof(model_ship3_texcoords);
            model_primitives_sizeof = sizeof(model_ship3_primitives);
            model_primitives = model_ship3_primitives;
            break;
            
        case MODEL_TURRET:
            model_coords = model_turret1;
            model_sizeof = sizeof(model_turret1);
            model_indices = model_turret_indices;
            model_indices_sizeof = sizeof(model_turret_indices);
            model_texcoords = model_turret_texcoords;
            model_texcoords_sizeof = sizeof(model_turret_texcoords);
            model_primitives_sizeof = sizeof(model_turret_primitives);
            model_primitives = model_turret_primitives;
            break;
            
        case MODEL_BULLET:
            model_coords = model_bullet;
            model_sizeof = sizeof(model_bullet);
            model_indices = model_bullet_indices;
            model_indices_sizeof = sizeof(model_bullet_indices);
            model_texcoords = model_bullet_texcoords;
            model_texcoords_sizeof = sizeof(model_bullet_texcoords);
            model_primitives_sizeof = sizeof(model_bullet_primitives);
            model_primitives = model_bullet_primitives;
            break;
            
        case MODEL_MISSLE:
            model_coords = model_missle;
            model_sizeof = sizeof(model_missle);
            model_indices = model_missle_indices;
            model_indices_sizeof = sizeof(model_missle_indices);
            model_texcoords = model_missle_texcoords;
            model_texcoords_sizeof = sizeof(model_missle_texcoords);
            model_primitives_sizeof = sizeof(model_missle_primitives);
            model_primitives = model_missle_primitives;
            break;
            
        case MODEL_SURFACE:
            model_coords = model_surface;
            model_sizeof = sizeof(model_surface);
            model_indices = model_surface_indices;
            model_indices_sizeof = sizeof(model_surface_indices);
            model_texcoords = model_surface_texcoords;
            model_texcoords_sizeof = sizeof(model_surface_texcoords);
            model_primitives_sizeof = sizeof(model_surface_primitives);
            model_primitives = model_surface_primitives;
            break;
            
        case MODEL_SPRITE:
            model_coords = model_sprite;
            model_sizeof = sizeof(model_sprite);
            model_indices = model_sprite_indices;
            model_indices_sizeof = sizeof(model_sprite_indices);
            model_texcoords = model_sprite_texcoords;
            model_texcoords_sizeof = sizeof(model_sprite_texcoords);
            model_primitives_sizeof = sizeof(model_sprite_primitives);
            model_primitives = model_sprite_primitives;
            break;
            
        case MODEL_VERTICAL_PILLAR:
            model_coords = model_vertical_pillar;
            model_sizeof = sizeof(model_vertical_pillar);
            model_indices = model_vertical_pillar_indices;
            model_indices_sizeof = sizeof(model_vertical_pillar_indices);
            model_texcoords = model_vertical_pillar_texcoords;
            model_texcoords_sizeof = sizeof(model_vertical_pillar_texcoords);
            model_primitives_sizeof = sizeof(model_vertical_pillar_primitives);
            model_primitives = model_vertical_pillar_primitives;
            break;
            
        case MODEL_MESH:
            model_coords = model_cube;
            model_sizeof = sizeof(model_cube);
            model_indices = model_cube_indices;
            model_indices_sizeof = sizeof(model_cube_indices);
            model_texcoords = model_cube_texcoords;
            model_texcoords_sizeof = sizeof(model_cube_texcoords);
            model_primitives_sizeof = sizeof(model_cube_primitives);
            model_primitives = model_cube_primitives;
            break;
            
        case MODEL_TELEPORTER:
            model_coords = model_cube;
            model_sizeof = sizeof(model_cube);
            model_indices = model_cube_indices;
            model_indices_sizeof = sizeof(model_cube_indices);
            model_texcoords = model_cube_texcoords;
            model_texcoords_sizeof = sizeof(model_cube_texcoords);
            model_primitives_sizeof = sizeof(model_cube_primitives);
            model_primitives = model_cube_primitives;
            break;
            
        case MODEL_ICOSAHEDRON:
            model_coords = model_icosahedron;
            model_sizeof = sizeof(model_icosahedron);
            model_indices = model_icosahedron_indices;
            model_indices_sizeof = sizeof(model_icosahedron_indices);
            model_texcoords = model_icosahedron_texcoords;
            model_texcoords_sizeof = sizeof(model_icosahedron_texcoords);
            model_primitives_sizeof = sizeof(model_icosahedron_primitives);
            model_primitives = model_icosahedron_primitives;
            break;
            
        case MODEL_SPHERE:
            model_coords = model_sphere_coords;
            model_sizeof = sizeof(model_sphere_coords);
            model_indices = model_sphere_indices;
            model_indices_sizeof = sizeof(model_sphere_indices);
            model_texcoords = model_sphere_texcoords;
            model_texcoords_sizeof = sizeof(model_sphere_texcoords);
            model_primitives_sizeof = sizeof(model_sphere_primitives);
            model_primitives = model_sphere_primitives;
            break;
            
        case MODEL_TBUILDING:
        {
            /*
            model_coords = model_tbuilding_coords;
            model_sizeof = sizeof(model_tbuilding_coords);
            model_indices = model_tbuilding_indices;
            model_indices_sizeof = sizeof(model_tbuilding_indices);
            model_texcoords = model_tbuilding_texcoords;
            model_texcoords_sizeof = sizeof(model_tbuilding_texcoords);
            model_primitives_sizeof = sizeof(model_tbuilding_primitives);
            model_primitives = model_tbuilding_primitives;
             */
            float M2[] = {
                2.0, 0, 0, 0,
                0, 4.0, 0, 1.5,
                0, 0, 1.5, 0,
                0, 0, 0, 1
            };
            MODEL_POLY_COMPONENTS_ADD(model_tbuilding_coords, model_tbuilding_texcoords, model_tbuilding_indices, M2);
            
            model_coords = poly_comp.model_coords_buffer;
            model_sizeof = poly_comp.model_coords_buffer_len * 3 * sizeof(model_coord_t);
            model_indices = poly_comp.model_indices_buffer;
            model_indices_sizeof = poly_comp.model_faces_buffer_n * 3 * sizeof(model_index_t);
            model_texcoords = poly_comp.model_texcoords_buffer;
            model_texcoords_sizeof = poly_comp.model_coords_buffer_len * 2 * sizeof(model_texcoord_t);
            
            model_primitives = poly_comp.primitives;
            model_primitives_sizeof = 0;
        }
        break;
            
        case MODEL_CBUILDING:
        {
#if 1
            model_coords = model_2building_coords;
            model_sizeof = sizeof(model_2building_coords);
            model_texcoords = model_2building_texcoords;
            model_texcoords_sizeof = sizeof(model_2building_texcoords);
            model_indices = model_2building_indices;
            model_indices_sizeof = sizeof(model_2building_indices);
            
#else
            /*
             model_coords = model_tbuilding_coords;
             model_sizeof = sizeof(model_tbuilding_coords);
             model_indices = model_tbuilding_indices;
             model_indices_sizeof = sizeof(model_tbuilding_indices);
             model_texcoords = model_tbuilding_texcoords;
             model_texcoords_sizeof = sizeof(model_tbuilding_texcoords);
             model_primitives_sizeof = sizeof(model_tbuilding_primitives);
             model_primitives = model_tbuilding_primitives;
             */
            
            float M3[] = {
                0.75, 0, 0, 0.01,
                0, 4.0, 0, 5.51,
                0, 0, 0.75, 0.01,
                0, 0, 0, 1
            };
            MODEL_POLY_COMPONENTS_ADD(model_cube, model_cube_texcoords4x, model_cube_indices_nobottom, M3);
            
            float M2[] = {
                1, 0, 0, 0,
                0, 4.0, 0, 1.5,
                0, 0, 1, 0,
                0, 0, 0, 1
            };
            MODEL_POLY_COMPONENTS_ADD(model_cube, model_cube_texcoords4x, model_cube_indices_nobottom, M2);
            
            model_coords = poly_comp.model_coords_buffer;
            model_sizeof = poly_comp.model_coords_buffer_len * 3 * sizeof(model_coord_t);
            model_indices = poly_comp.model_indices_buffer;
            model_indices_sizeof = poly_comp.model_faces_buffer_n * 3 * sizeof(model_index_t);
            model_texcoords = poly_comp.model_texcoords_buffer;
            model_texcoords_sizeof = poly_comp.model_coords_buffer_len * 2 * sizeof(model_texcoord_t);
#endif
            
            model_primitives = poly_comp.primitives;
            model_primitives_sizeof = 0;
        }
        break;
            
        case MODEL_ENEMY_BASE:
        {   
            // TODO: build model by applying rotation matrix to convex primitive and appending coordinates/indices/texture-coordinates
            float M2[] = {
                1, 0, 0, 0,
                0, 0.70710678118, -0.70710678118, -1.0,
                0, 0.70710678118, 0.70710678118, 0,
                0, 0, 0, 1
            };
            MODEL_POLY_COMPONENTS_ADD(model_enemy_base_core, model_enemy_base_core_texcoords, model_enemy_base_core_indices, M2);
            float M3[] = {
                1, 0, 0, 0,
                0, 0.70738826916, 0.70710807985, 1.0,
                0, -0.70710807985, 0.70738826916, 0,
                0, 0, 0, 1
            };
            MODEL_POLY_COMPONENTS_ADD(model_enemy_base_core, model_enemy_base_core_texcoords, model_enemy_base_core_indices, M3);
            float M4[] = {
                0.70710678118, 0, -0.70710678118, -1.0,
                0, 1, 0, 0,
                0.70710678118, 0, 0.70710678118, 0,
                0, 0, 0, 1
            };
            MODEL_POLY_COMPONENTS_ADD(model_enemy_base_core, model_enemy_base_core_texcoords, model_enemy_base_core_indices, M4);
            float M5[] = {
                0.70710678118, 0, 0.70710678118, 1.0,
                0, 1, 0, 0,
                -0.70710678118, 0, 0.70710678118, 0,
                0, 0, 0, 1
            };
            MODEL_POLY_COMPONENTS_ADD(model_enemy_base_core, model_enemy_base_core_texcoords, model_enemy_base_core_indices, M5);
            
            model_coords = poly_comp.model_coords_buffer;
            model_sizeof = poly_comp.model_coords_buffer_len * 3 * sizeof(model_coord_t);
            model_indices = poly_comp.model_indices_buffer;
            model_indices_sizeof = poly_comp.model_faces_buffer_n * 3 * sizeof(model_index_t);
            model_texcoords = poly_comp.model_texcoords_buffer;
            model_texcoords_sizeof = poly_comp.model_coords_buffer_len * 2 * sizeof(model_texcoord_t);
            
            model_primitives = poly_comp.primitives;
            model_primitives_sizeof = 0;
        }
            break;
            
        case MODEL_FLATTENED_CUBE:
        {
            float M1[] = {
                1, 0, 0, 0,
                0, 0.1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            };
            MODEL_POLY_COMPONENTS_ADD(model_cube, model_cube_texcoords, model_cube_indices, M1);
        
            model_coords = poly_comp.model_coords_buffer;
            model_sizeof = poly_comp.model_coords_buffer_len * 3 * sizeof(model_coord_t);
            model_indices = poly_comp.model_indices_buffer;
            model_indices_sizeof = poly_comp.model_faces_buffer_n * 3 * sizeof(model_index_t);
            model_texcoords = poly_comp.model_texcoords_buffer;
            model_texcoords_sizeof = poly_comp.model_coords_buffer_len * 2 * sizeof(model_texcoord_t);
            
            model_primitives = poly_comp.primitives;
            model_primitives_sizeof = 0;
        }
            break;
            
        case MODEL_CONTRAIL:
        {
            model_coords = model_contrail;
            model_sizeof = sizeof(model_contrail);
            model_indices = model_contrail_indices;
            model_indices_sizeof = sizeof(model_contrail_indices);
            model_texcoords = model_contrail_texcoords;
            model_texcoords_sizeof = sizeof(model_contrail_texcoords);
            model_primitives_sizeof = sizeof(model_contrail_primitives);
            model_primitives = model_contrail_primitives;
        }
            break;
            
        default:
            assert(0);
            break;
	}
    
    // HACK: make some decisions based off model-type
    if(replace_id < 0)
    switch(type)
    {
        case MODEL_ICOSAHEDRON:
            pElem->renderInfo.priority = 1;
            new_durability = DURABILITY_ASTEROID;
            break;
          
        case MODEL_CUBE:
    	case MODEL_CUBE2:
            new_durability = DURABILITY_BLOCK;
            break;
            
        case MODEL_PYRAMID:
            new_durability = DURABILITY_BLOCK;
            break;
            
        case MODEL_SHIP1:
            pElem->renderInfo.priority = 1;
            new_durability = 5;
            pElem->spans_regions = 0;
            pElem->bounding_remain = 0;
            pElem->bounding_wrap = 0;
            pElem->stuff.radar_visible = 1;
            break;
            
        case MODEL_SHIP2:
        case MODEL_SHIP3:
            pElem->renderInfo.priority = 1;
            new_durability = 5;
            pElem->spans_regions = 0;
            pElem->bounding_remain = 0;
            pElem->stuff.radar_visible = 1;
            break;
            
        case MODEL_TURRET:
            pElem->renderInfo.priority = 1;
            new_durability = 5;
            pElem->spans_regions = 0;
            pElem->bounding_remain = 1;
            break;
            
        case MODEL_BULLET:
            pElem->destructible = 1;
            pElem->spans_regions = 0;
            pElem->object_type = OBJ_BULLET;
            break;
            
        case MODEL_MISSLE:
            pElem->destructible = 1;
            pElem->spans_regions = 0;
            pElem->object_type = OBJ_MISSLE;
            break;
            
        case MODEL_SURFACE:
            pElem->renderInfo.priority = 1;
            break;
            
        case MODEL_TBUILDING:
        case MODEL_CBUILDING:
            pElem->renderInfo.priority = 1;
            new_durability = DURABILITY_BLOCK;
            break;
            
        case MODEL_ENEMY_BASE:
            pElem->renderInfo.priority = 1;
            pElem->renderInfo.concavepoly = 1;
            break;
            
        case MODEL_FLATTENED_CUBE:
            pElem->renderInfo.concavepoly = 1;
            pElem->renderInfo.priority = 1;
            new_durability = DURABILITY_BLOCK;
            break;
            
        case MODEL_MESH:
            new_durability = 100;
            break;
            
        case MODEL_TELEPORTER:
            pElem->destructible = 0;
            new_durability = 100;
            break;
            
        case MODEL_SCENERY:
            pElem->renderInfo.visible = 1;
            pElem->renderInfo.priority = 1;
            break;
            
        case MODEL_CONTRAIL:
            pElem->renderInfo.visible = 1;
            pElem->renderInfo.priority = 1;
            break;

        default:
            break;
    }
    
    for(i = 0; i < (model_sizeof/sizeof(model_coord_t)); i+=3)
    {
        quaternion_t pt = {0, model_coords[i+0] * scale, model_coords[i+1] * scale, model_coords[i+2] * scale};
        quaternion_t xq = {0, 1, 0, 0};
        quaternion_t yq = {0, 0, 1, 0};
        quaternion_t zq = {0, 0, 0, 1};
        
        // yaw
        if(alpha != 0)
        {
            quaternion_rotate_inplace(&pt, &zq, alpha);
            quaternion_rotate_inplace(&xq, &zq, alpha);
            quaternion_rotate_inplace(&yq, &zq, alpha);
        }
        
        // pitch
        if(beta != 0)
        {
            quaternion_rotate_inplace(&pt, &xq, beta);
            quaternion_rotate_inplace(&yq, &xq, beta);
            quaternion_rotate_inplace(&zq, &xq, beta);
        }
        
        // roll
        if(gamma != 0)
        {
            quaternion_rotate_inplace(&pt, &zq, gamma);
            quaternion_rotate_inplace(&xq, &zq, gamma);
            quaternion_rotate_inplace(&yq, &zq, gamma);
        }
        
        pElem->coords[i+0] = x + pt.x;
        pElem->coords[i+1] = y + pt.y;
        pElem->coords[i+2] = z + pt.z;
    }
    pElem->n_coords = model_sizeof / sizeof(model_coord_t);
    
    memcpy(pElem->indices, model_indices, model_indices_sizeof);
    pElem->n_indices = model_indices_sizeof / sizeof(model_index_t);
    
    memcpy(pElem->texcoords, model_texcoords, model_texcoords_sizeof);
    pElem->n_texcoords = model_texcoords_sizeof / sizeof(model_texcoord_t);
    
    /* problem here where num of primitives (linked_elems) needs to change
     * but we can't simply free here until rendering is done, elements
     * need to be added to pending-free list.. */
    if(model_changed && pElem->linked_elem)
    {
        printf("linked elem left behind\n");
    }
    
    int p;
    int offset = 0;
    WorldElem* pElemHead = pElem;
    pElem->head_elem = NULL;
    for(p = 0; p < model_primitives_sizeof/sizeof(int); p++)
    {
        // clone this element
        int primitive_size = model_primitives[p];
        
        if(p > 0)
        {
            WorldElem* pElemNext = NULL;
            
            // IMPORTANT: be very careful when adding fields to elements
            // because it has implications for linked-elements, some fields
            // should not be copied over, and others should...
            // TODO: move non-cloneable data (like pointers) to a union within the elem
            // to clean this up
            
            // specially handled fields:
            // elem_id, linked_elem, head_elem, listRefHead
            if(replace_id >= 0 && pElem->linked_elem)
            {
                pElemNext = (WorldElem*) pElem->linked_elem;
                
                // preserve non-cloneable fields
                WorldElem elemSave;
                memcpy(&elemSave, pElemNext, sizeof(elemSave));
                
                // TODO: probably only necessary to update coords...
                memcpy(pElemNext, pElem, pElem->size);
                
                pElemNext->elem_id = elemSave.elem_id;
                pElemNext->linked_elem = elemSave.linked_elem;
                pElemNext->head_elem = elemSave.head_elem;
                pElemNext->listRefHead = elemSave.listRefHead;
                pElemNext->coords = elemSave.coords;
                pElemNext->texcoords = elemSave.texcoords;
                pElemNext->indices = elemSave.indices;
                memcpy(&pElemNext->stuff, &elemSave.stuff, sizeof(pElemNext->stuff));
                
                world_elem_replace_fix(pElemNext);
                world_elem_adjust_geometry_pointers(pElemNext);
            }
            else
            {
                pElemNext = world_elem_clone(pElemHead);
                
                world_elem_list_add(pElemNext, &gWorld->elements_list);
                world_elem_list_add(pElemNext, &gWorld->elements_to_be_added);
                pElem->linked_elem = (struct WorldElem*) pElemNext;
            }
            
            pElemNext->head_elem = (struct WorldElem*) pElemHead;
            pElemNext->physics.ptr = pElemHead->physics.ptr;
            
            pElem = pElemNext;
        }
        
        memcpy(pElem->indices, &pElem->indices[offset], primitive_size * sizeof(model_index_t));
        pElem->n_indices = primitive_size;
        offset += primitive_size;
        
        break;
    }
    
    pElem = pElemHead;
    
    world_elem_replace_fix(pElem);
    
    if(replace_id < 0)
    {
        pElem->durability = new_durability;
    }
	
	switch (texture_id)
	{
	default:
		break;
	}
    
    /*
    if(is_collideable && replace_id == WORLD_ELEM_ID_INVALID)
    {
        world_elem_list_add(pElem, &gWorld->elements_moving);
    }
     */
    
    /*
    float elem_bbox[6];
    get_element_bounding_box(pElem, elem_bbox);
    // if element is sufficiently large...
    float k = 2;
    if(elem_bbox[3] - elem_bbox[0] >= (gWorld->bound_x / WORLD_MAX_REGIONS)*k) pElem->spans_regions = true;
    if(elem_bbox[4] - elem_bbox[1] >= (gWorld->bound_y / WORLD_MAX_REGIONS)*k) pElem->spans_regions = true;
    if(elem_bbox[5] - elem_bbox[2] >= (gWorld->bound_z / WORLD_MAX_REGIONS)*k) pElem->spans_regions = true;
     */
    
    /*
    // insert element into sorted region table
    if(pElem->spans_regions)
    {    
        for(float rx = elem_bbox[0]; rx <= elem_bbox[3]; rx += gWorld->region_size_x)
        {
            for(float ry = elem_bbox[1]; ry <= elem_bbox[4]; ry += gWorld->region_size_y)
            {
                for(float rz = elem_bbox[2]; rz <= elem_bbox[5]; rz += gWorld->region_size_z)
                {
                    add_element_to_region_for_coord(pElem, rx, ry, rz);
                }
            }
        }
    }
    else
    {
        add_element_to_region(pElem);
    }
     */
    
    add_element_to_region(pElem);
    
    if(replace_id < 0)
    {
        gWorld->last_elem_added = pElem;
    }
    
    return pElem->elem_id;
}

int world_add_object(Model type, float x, float y, float z, float yaw, float pitch, float roll, float scale, int texture_id)
{
    if(gWorld->ignore_add) return WORLD_ELEM_ID_INVALID;
    return world_add_object_core(type, x, y, z, yaw, pitch, roll, scale, texture_id, WORLD_ELEM_ID_INVALID);
}

void
world_remove_object(int elem_id)
{
    WorldElem* pElem = NULL;
    
    if(gWorld->ignore_remove) return;
    
    WorldElemListNode* pRemoveNode;
    pRemoveNode = world_elem_list_find(elem_id, &gWorld->elements_list);
    if(pRemoveNode) pElem = pRemoveNode->elem;
    
    if(pElem && !pElem->remove_pending)
    {
        if(gWorld->world_update_state.world_remove_hook) gWorld->world_update_state.world_remove_hook(pElem);
        
        remove_element_from_region(pElem);
        
        world_elem_list_remove(pElem, &gWorld->elements_expiring);
        world_elem_list_remove(pElem, &gWorld->elements_moving);
        
        world_elem_list_remove(pElem, &gWorld->elements_list);
        world_elem_list_add(pElem, &gWorld->elements_to_be_freed);
        pElem->remove_pending = 1;
    }
}

int world_replace_object(int elem_id, Model type, float x, float y, float z, float yaw, float pitch, float roll, float scale, int texture_id)
{
    assert(elem_id >= 0);
    
    return world_add_object_core(type, x, y, z, yaw, pitch, roll, scale, texture_id, elem_id);
}

void
world_replace_add_object(int* elem_id_storage,
                         Model type,
                         float x, float y, float z,
                         float yaw, float pitch, float roll,
                         float scale, int texture_id,
                         WorldElem** elem_out)
{
    while(*elem_id_storage == WORLD_ELEM_ID_INVALID)
    {
        *elem_id_storage = world_add_object(type, x, y, z, yaw, pitch, roll, scale, texture_id);
    }
    
    *elem_id_storage = world_replace_object(*elem_id_storage, type, x, y, z, yaw, pitch, roll, scale, texture_id);
    
    WorldElemListNode* foundNode = world_elem_list_find(*elem_id_storage, &gWorld->elements_list);
    
    if(elem_out && foundNode) *elem_out = foundNode->elem;
}

WorldElem* world_get_last_object()
{
    return gWorld->last_elem_added;
}

WorldElem* world_get_object_n_at_loc(int n, float x, float y, float z, float range)
{
    //WorldElemListNode *pNode = get_region_list_head(x, y, z);
    WorldElemListNode *pNode = gWorld->elements_list.next;
    WorldElemListNode *pFound = NULL;
    
    while(pNode)
    {
        if(fabs(x - pNode->elem->physics.ptr->x) <= range &&
           fabs(y - pNode->elem->physics.ptr->y) <= range &&
           fabs(z - pNode->elem->physics.ptr->z) <= range)
        {
            pFound = pNode;
            if(n <= 0) break;
            n--;
        }
        
        pNode = pNode->next;
    }
    
    return pFound? pFound->elem: NULL;
}

WorldElem*
world_find_elem_with_attrs(WorldElemListNode* head, int object_type, int affiliation)
{
    WorldElemListNode* pNode = head->next;
    while(pNode)
    {
        if(pNode->elem->object_type == object_type &&
           pNode->elem->stuff.affiliation == affiliation)
        {
            return pNode->elem;
        }
        pNode = pNode->next;
    }
    return NULL;
}

static void update_object_in_motion(WorldElem* pElem)
{
    // add to "elements-moving" list
    if(!world_elem_list_find_elem(pElem, &gWorld->elements_moving))
    {
        world_elem_list_add(pElem, &gWorld->elements_moving);
    }
    
    pElem->physics.ptr->velocity = sqrt(pElem->physics.ptr->vx*pElem->physics.ptr->vx +
                                        pElem->physics.ptr->vy*pElem->physics.ptr->vy +
                                        pElem->physics.ptr->vz*pElem->physics.ptr->vz);
    pElem->moving = 1;
}

int update_object_velocity(int object_id, float x, float y, float z, int relative)
{
    WorldElemListNode* pElemNode = world_elem_list_find(object_id, &gWorld->elements_list);
    
    if(pElemNode)
    {
        WorldElem* pElem = pElemNode->elem;
        
        // not allowing moving elements to span regions (for speed)
        /*
        if(pElem->spans_regions) 
        {
            remove_element_from_region(pElem);
            pElem->spans_regions = 0;
        }
         */
        
        if(!relative)
        {
            pElem->physics.ptr->vx = x;
            pElem->physics.ptr->vy = y;
            pElem->physics.ptr->vz = z;
        }
        else
        {
            pElem->physics.ptr->vx += x;
            pElem->physics.ptr->vy += y;
            pElem->physics.ptr->vz += z;
        }
        
        update_object_in_motion(pElem);
        
        return object_id;
    }
    
    return WORLD_ELEM_ID_INVALID;
}

int update_object_velocity_with_friction(int object_id, float v[3], float cthrust, float cfriction)
{
    /*
     float c_thrust = C_THRUST;
     float c_friction = C_FRICTION;
     float ship_vnew[3] =
     {
     (-ship_z_vec[0]*speed * c_thrust) + (-pWorldElemMyShip->physics.ptr->vx * c_friction),
     (-ship_z_vec[1]*speed * c_thrust) + (-pWorldElemMyShip->physics.ptr->vy * c_friction),
     (-ship_z_vec[2]*speed * c_thrust) + (-pWorldElemMyShip->physics.ptr->vz * c_friction),
     };
     
     update_object_velocity(my_ship_id,
     ship_vnew[0],
     ship_vnew[1],
     ship_vnew[2],
     1);
     */
    
    WorldElemListNode* pElemNode = world_elem_list_find(object_id, &gWorld->elements_list);
    
    if(pElemNode)
    {
        float ship_vnew[3] =
        {
            (v[0] * cthrust) + (pElemNode->elem->physics.ptr->vx * -cfriction),
            (v[1] * cthrust) + (pElemNode->elem->physics.ptr->vy * -cfriction),
            (v[2] * cthrust) + (pElemNode->elem->physics.ptr->vz * -cfriction)
        };
        
        /*
        update_object_velocity(object_id,
                               ship_vnew[0],
                               ship_vnew[1],
                               ship_vnew[2],
                               1);
         */
        pElemNode->elem->physics.ptr->vx += ship_vnew[0];
        pElemNode->elem->physics.ptr->vy += ship_vnew[1];
        pElemNode->elem->physics.ptr->vz += ship_vnew[2];
    
        update_object_in_motion(pElemNode->elem);
        
        return object_id;
    }

    return WORLD_ELEM_ID_INVALID;
}

int world_object_set_lifetime(int object_id, int tex_passes)
{
    WorldElemListNode* pElemNode = world_elem_list_find(object_id, &gWorld->elements_list);
    
    if (pElemNode)
    {
        WorldElem* pElem = pElemNode->elem;
        
        if (!world_elem_list_find_elem(pElem, &gWorld->elements_expiring))
        {
            world_elem_list_add(pElem, &gWorld->elements_expiring);
        }
        else
        {
            // already in list
        }
        pElem->lifetime = tex_passes;
        
        return object_id;
    }
    return WORLD_ELEM_ID_INVALID;
}

void world_object_set_nametag(int object_id, char* nametag)
{
    WorldElemListNode* pElemNode = world_elem_list_find(object_id, &gWorld->elements_list);
    
    if (pElemNode)
    {
        world_elem_set_nametag(pElemNode->elem, nametag);
    }
}

void
world_random_spawn_location(float loc[6], int affiliation)
{
    WorldElemListNode* pCur = gWorld->elements_list.next;
    int n = rand() % 64;
    int i = 0;
    int found_spawns = 0;
    
    while(n > 0)
    {
        while(pCur)
        {
            if(pCur->elem->object_type == OBJ_SPAWNPOINT)
            {
                found_spawns++;
                n--;
                if(n == 0)
                {
                    i = 0;
                    loc[i++] = pCur->elem->physics.ptr->x;
                    loc[i++] = pCur->elem->physics.ptr->y;
                    loc[i++] = pCur->elem->physics.ptr->z;
                    
                    // random heading on x/z axis
                    loc[i++] = M_PI/2; // alpha
                    loc[i++] = rand_in_range(0, M_PI*2 * 100) / 100; // beta
                    loc[i++] = -M_PI/2; // gamma
                    return;
                }
            }
            pCur = pCur->next;
        }
        
        if(!found_spawns) break;
        
        pCur = gWorld->elements_list.next;
    }
    
    // no spawn point found, spawn randomly
    loc[0] = rand_in_range(-gWorld->bound_radius/2, gWorld->bound_radius/2);
    loc[1] = rand_in_range(1, gWorld->bound_radius/2);
    loc[2] = rand_in_range(-gWorld->bound_radius/2, gWorld->bound_radius/2);
    
    loc[3] = M_PI/2;
    loc[4] = rand_in_range(-M_PI+0.1, M_PI-0.1);
    loc[5] = -M_PI/2;
}

void world_free()
{
	if(gWorld)
	{
        gWorld->elements_list.hash_ptr = NULL;
        if(gWorld->element_id_hash) simple_hash_table_destroy(gWorld->element_id_hash);
        
        // TODO: release all objects and lists
        WorldElemListNode* pCur = gWorld->elements_list.next;
        WorldElemListNode* pNext = NULL;
        while(pCur)
        {
            WorldElem* pCurElem = pCur->elem;
            pNext = pCur->next;
            
            WorldElemListNode* pListHead;
            
            while((pListHead = world_elem_get_member_list_head(pCurElem, 0)))
            {
                world_elem_list_remove(pCurElem, pListHead);
            }
            world_elem_free(pCurElem);
            
            pCur = pNext;
        }
        
        if(gWorld->boundingRegion)
        {
            boundingRegionUninit(gWorld->boundingRegion);
        }
        
        if(gWorld->elements_by_region) free(gWorld->elements_by_region);
        
        // release meshes
        WorldElemListNode* pHead = &gWorld->triangle_mesh_head;
        pCur = pHead->next;
        while(pCur)
        {
            WorldElemListNode* pNext = pCur->next;
            WorldElem* pElem = pCur->elem;
            
            world_elem_list_remove(pCur->elem, pHead);
            world_elem_free(pElem);
            pCur = pNext;
        }
        
        // release lines
        pHead = &gWorld->drawline_list_head;
        pCur = pHead->next;
        while(pCur)
        {
            WorldElemListNode* pNext = pCur->next;
            WorldElem* pElem = pCur->elem;
            
            world_elem_list_remove(pCur->elem, pHead);
            world_elem_free(pElem);
            pCur = pNext;
        }
        
		free(gWorld);
		gWorld = NULL;
	}
    
    if(world_pending_mesh)
    {
        free(world_pending_mesh);
        world_pending_mesh = NULL;
    }
}

void
region_for_coord(float x, float y, float z, int rgn[3])
{
    float Rs = gWorld->region_size;
    
    rgn[0] = (x/Rs) + (gWorld->regions_x/2);
    rgn[1] = (y/Rs) + (gWorld->regions_y/2);
    rgn[2] = (z/Rs) + (gWorld->regions_z/2);
}

WorldElemListNode*
world_region_head(float x, float y, float z)
{
    int rgn[3];
    region_for_coord(x, y, z, rgn);
    
    WorldElemListNode* pNode = gWorld->elements_by_region;
    
    pNode += rgn[0] * (int) gWorld->regions_y * (int) gWorld->regions_z;
    pNode += rgn[1] * (int) gWorld->regions_z;
    pNode += rgn[2];
    
    return pNode;
}

void world_init(float radius)
{
    float ws = 25;
    
	gWorld = malloc(sizeof(world_t));
	memset(gWorld, 0, sizeof(world_t));
    
    gWorld->bound_radius = radius;
    gWorld->region_size = ws;
    
    gWorld->regions_x = (gWorld->bound_radius*2) / ws;
    gWorld->regions_y = (gWorld->bound_radius*2) / ws;
    gWorld->regions_z = (gWorld->bound_radius*2) / ws;
    
    float alloc_size = sizeof(WorldElemListNode) * gWorld->regions_x * gWorld->regions_y * gWorld->regions_z;
    
    gWorld->elements_by_region = (WorldElemListNode*) malloc(alloc_size);
    memset(gWorld->elements_by_region, 0, alloc_size);
    
    gWorld->elem_id_next = 1000;
    
    gWorld->element_id_hash = simple_hash_table_init(1024);
    gWorld->elements_list.hash_ptr = gWorld->element_id_hash;
    
    assert(floor(gWorld->regions_x) == gWorld->regions_x);
    assert(floor(gWorld->regions_y) == gWorld->regions_y);
    assert(floor(gWorld->regions_z) == gWorld->regions_z);
    
    // build bounding vectors (rectangle)
    /*
    boundingRegion* br = boundingRegionInit(6);
    boundingRegionAddVec(br, 0, 0, 0, 1, 0, 0);
    boundingRegionAddVec(br, 0, 0, 0, 0, 1, 0);
    boundingRegionAddVec(br, 0, 0, 0, 0, 0, 1);
    boundingRegionAddVec(br, gWorld->bound_x, gWorld->bound_y, gWorld->bound_z, -1, 0, 0);
    boundingRegionAddVec(br, gWorld->bound_x, gWorld->bound_y, gWorld->bound_z, 0, -1, 0);
    boundingRegionAddVec(br, gWorld->bound_x, gWorld->bound_y, gWorld->bound_z, 0, 0, -1);
    gWorld->boundingRegion = br;
    */
    
    // build spherical bounding
    float Ty, Tx;
    float I = WORLD_BOUNDING_SPHERE_STEPS;
    float R = M_PI*2;
    float Rad = gWorld->bound_radius;
    float RL = (R/I);
    quaternion_t Qx, Qy, Qz;
    
    boundingRegion* br = boundingRegionInit(I * I + 1);
    
    Qx.w = Qy.w = Qz.w = 1.0;
    Qx.x = Qy.y = Qz.z = 1.0;
    Qx.y = Qx.z = Qy.x = Qy.z = Qz.x = Qz.y = 0.0;

    for(Ty = 0; Ty < R; Ty += RL)
    {
        quaternion_t U, V;
        
        U = Qx;
        V = Qz;
        
        quaternion_rotate_inplace(&U, &Qy, Ty);
        quaternion_rotate_inplace(&V, &Qy, Ty);
        
        // bounding vectors only for top hemisphere
        for(Tx = 0; Tx < R/2; Tx += R/I)
        {
            int d;
            float P[] = {
                U.x * Rad,
                U.y * Rad,
                U.z * Rad
            };
            
            boundingRegionAddVec(br, P[0], P[1], P[2], -U.x, -U.y, -U.z);
            
            float s;
            
            for(s = -1; s <= 1; s += 2)
            {
                float UVcross[] = {
                    U.x, U.y, U.z,
                    V.x * s, V.y * s, V.z * s,
                    Qy.x, Qy.y, Qy.z,
                    0, 0, 0,
                    0, 0, 0
                };
                float L1[3], L2[3], L3[3], L4[3];
                
                vector_cross_product(&UVcross[0], &UVcross[3], &(UVcross[9]));
                vector_cross_product(&UVcross[0], &UVcross[6], &(UVcross[12]));
                
                float L = tan(RL/2)*Rad * 2;
                
                for(d = 0; d < 3; d++)
                {
                    int i;
                    float *LClamp[] = { L1, L2, L3, L4 };
                    
                    L1[d] = P[d] + (UVcross[d+9]) * (L / 2);
                    L2[d] = P[d] + (UVcross[d+9]) * (L / -2);
                    L3[d] = P[d] + (UVcross[d+12]) * (L / 2);
                    L4[d] = P[d] + (UVcross[d+12]) * (L / -2);
                    
                    for(i = 0; i < sizeof(LClamp)/sizeof(float*); i++)
                    {
                        if(LClamp[i][1] < 0) LClamp[i][1] = 0;
                    }
                }
                
                float LColor[] = {
                    0, 0xff, 0
                };
                
                world_add_drawline(L1, L2, LColor, 999999);
                
                world_add_drawline(L3, L4, LColor, 999999);
            }
            
            quaternion_rotate_inplace(&U, &V, R/I);
        }
    }
    
    // boundary on the floor
    boundingRegionAddVec(br, 0, 0.1, 0, 0, 1, 0);
    
    gWorld->boundingRegion = br;
    
    // gravity vector
    gWorld->vec_gravity[1] = -32.0;
    
    //world_build_visibility_data();
    
    models_init();

	return;
}

void world_clear_pending()
{
    // clear pending additions
    world_elem_list_clear(&gWorld->elements_to_be_added);
    
    // clear collisions
    world_elem_list_clear(&gWorld->elements_collided);
    
    // clear pending removals
    WorldElemListNode* pCur = gWorld->elements_to_be_freed.next;
    while(pCur)
    {
        WorldElem* pFreeElem = pCur->elem;
        pCur = pCur->next;
        
        world_elem_list_remove(pFreeElem, &gWorld->elements_expiring);
        world_elem_list_remove(pFreeElem, &gWorld->elements_moving);
        remove_element_from_region(pFreeElem);
        
        world_elem_list_remove(pFreeElem, &gWorld->elements_to_be_freed);
        
        while(pFreeElem)
        {
            WorldElem* pFreeCur = pFreeElem;
            pFreeElem = pFreeElem->linked_elem;
            
            world_elem_list_remove(pFreeCur, &gWorld->elements_list);
            world_elem_free(pFreeCur);
        }
    }
}

void world_lock_init()
{
    game_lock_init(&gWorldLock);
}

void world_lock_uninit()
{
    game_lock_uninit(&gWorldLock);
}

void world_lock()
{
    game_lock_lock(&gWorldLock);
}

void world_unlock()
{
    game_lock_unlock(&gWorldLock);
}

/*
void
rebuild_world_done()
{
    // empty list of newly-added elements
    world_elem_list_clear(&gWorld->elements_to_be_added);
    
    // free pending-freed elements
    WorldElemListNode* pPendingCur = gWorld->elements_to_be_freed.next;
    while(pPendingCur)
    {
        //world_elem_free(pPendingCur->elem);
        pPendingCur = pPendingCur->next;
    }
    world_elem_list_clear(&gWorld->elements_to_be_freed);
}
 */

void
move_elem_relative(WorldElem* pElem, float x, float y, float z)
{
    int i;
    
    pElem->physics.ptr->x += x;
    pElem->physics.ptr->y += y;
    pElem->physics.ptr->z += z;
    
    while(pElem)
    {
        for(i = 0; i < pElem->n_coords; i += 3)
        {
            pElem->coords[i] += x;
            pElem->coords[i+1] += y;
            pElem->coords[i+2] += z;
        }
        pElem = (WorldElem*) pElem->linked_elem;
    }
}

void
world_move_elem(WorldElem* pElem, float x, float y, float z, int relative)
{
    remove_element_from_region(pElem);
    if(relative)
    {
        move_elem_relative(pElem, x, y, z);
    }
    else
    {
        move_elem_relative(pElem, x - pElem->physics.ptr->x,
                           y - pElem->physics.ptr->y,
                           z - pElem->physics.ptr->z);
    }
    add_element_to_region(pElem);
}

void
world_repulse_elem(WorldElem* pCollisionA, WorldElem* pCollisionB, float tc, float Frepulse)
{
    float mv[3];
    float s = Frepulse;
    float repulse_min = 0.1;
    int dmin;
    int i;

    float v = pCollisionA->physics.ptr->velocity + repulse_min;

    mv[0] = (pCollisionA->physics.ptr->x - pCollisionB->physics.ptr->x) * v * tc * s;
    mv[1] = (pCollisionA->physics.ptr->y - pCollisionB->physics.ptr->y) * v * tc * s;
    mv[2] = (pCollisionA->physics.ptr->z - pCollisionB->physics.ptr->z) * v * tc * s;
    
    // repulse along min overlapping axis
    dmin = 0;
    for(i = 0; i < 3; i++)
    {
        if(check_bounding_box_overlap_result[i] < check_bounding_box_overlap_result[dmin]) dmin = i;
    }
    for(i = 0; i < 3; i++) if(dmin != i) mv[i] = 0.0;
    
    // cancel velocity along one axis
    float* p[] = {
        &pCollisionA->physics.ptr->vx,
        &pCollisionA->physics.ptr->vy,
        &pCollisionA->physics.ptr->vz
    };
    for(i = 0; i < 3; i++) if(dmin == i) *(p[i]) = 0;

    move_elem_relative(pCollisionA, mv[0], mv[1], mv[2]);
    
    if(isnan(pCollisionA->physics.ptr->x) || isnan(pCollisionA->physics.ptr->y) || isnan(pCollisionA->physics.ptr->z))
    {
        
        printf("world_repulse_elem/isNan\n");
    }
}

inline static void
get_element_bounding_box(WorldElem* pElem, float box[6])
{   
    // format of box will be {xmin, ymin, zmin, xmax, ymax, zmax}
    for(int i = 0; i < pElem->n_coords/3; i++)
    {
        model_coord_t coord[3];
        
        world_elem_get_coord(pElem, i, coord);

        model_coord_t x = coord[0];
        model_coord_t y = coord[1];
        model_coord_t z = coord[2];
        
        //TODO: hacky
        if(i == 0)
        {
            box[0] = box[3] = x;
            box[1] = box[4] = y;
            box[2] = box[5] = z;
        }
        else
        {
            if(x > box[3]) box[3] = x;
            if(x < box[0]) box[0] = x;
            if(y > box[4]) box[4] = y;
            if(y < box[1]) box[1] = y;
            if(z > box[5]) box[5] = z;
            if(z < box[2]) box[2] = z;
        }
    }
}

inline static int
check_bounding_box_overlap(float boxA_in[6], float boxB_in[6])
{
    float amin, amax;
    float bmin, bmax;
    float *boxA, *boxB;
    float R;
    int i;
    
    boxA = boxA_in;
    boxB = boxB_in;

    amin = boxA[0]; amax = boxA[3];
    bmin = boxB[0]; bmax = boxB[3];
    R = MAX(bmax-amin, amax-bmin);
    check_bounding_box_overlap_result[0] = (amax-amin) + (bmax-bmin) - R;
    
    amin = boxA[1]; amax = boxA[4];
    bmin = boxB[1]; bmax = boxB[4];
    R = MAX(bmax-amin, amax-bmin);
    check_bounding_box_overlap_result[1] = (amax-amin) + (bmax-bmin) - R;
    
    amin = boxA[2]; amax = boxA[5];
    bmin = boxB[2]; bmax = boxB[5];
    R = MAX(bmax-amin, amax-bmin);
    check_bounding_box_overlap_result[2] = (amax-amin) + (bmax-bmin) - R;
    
    for(i = 0; i < 3; i++) if(check_bounding_box_overlap_result[i] < 0) return 0;
    
    return 1;
}

static int
check_collision(WorldElem* pElemA, WorldElem* pElemB)
{
    model_coord_t boxA[6];
    model_coord_t boxB[6];
    
    get_element_bounding_box(pElemA, boxA);
    get_element_bounding_box(pElemB, boxB);
    
    return check_bounding_box_overlap(boxA, boxB);
}

static inline float
froundlong(float f)
{
    return rintf(f);
}

WorldElemListNode*
get_region_list_head(float x, float y, float z)
{
    if(x < -gWorld->bound_radius || x >= gWorld->bound_radius ||
       y < -gWorld->bound_radius || y >= gWorld->bound_radius ||
       z < -gWorld->bound_radius || z >= gWorld->bound_radius) return NULL;
    
    return world_region_head(x, y, z);
}

WorldElemListNode*
add_element_to_region_for_coord(WorldElem* pElem, float x, float y, float z)
{
    WorldElemListNode* pRegionHead = NULL;

    WorldElemListNode* pCur = get_region_list_head(x, y, z);
    pRegionHead = pCur;
    
    if(!pCur) return pRegionHead;
    
    world_elem_list_add_fast(pElem, pRegionHead, LIST_TYPE_REGION);
    
    return pRegionHead;
}

WorldElemListNode*
add_element_to_region(WorldElem* pElem)
{
    float elem_bbox[6];
  
    WorldElemListNode* pRegionHead = NULL;
    
    float Rs = gWorld->bound_radius*2 / gWorld->regions_x;
    
    float x = pElem->physics.ptr->x;
    float y = pElem->physics.ptr->y;
    float z = pElem->physics.ptr->z;
    
    WorldElemListNode* pCur = get_region_list_head(x, y, z);
    pRegionHead = pCur;
    
    if(!pCur) return pRegionHead;
    
    if(pElem->spans_regions)
    {
        get_element_bounding_box(pElem, elem_bbox);
        
        int regions_spanned = 0;
        
        int rgn_b[3];
        region_for_coord(elem_bbox[0], elem_bbox[1], elem_bbox[2], rgn_b);
        
        int rgn_e[3];
        region_for_coord(elem_bbox[3], elem_bbox[4], elem_bbox[5], rgn_e);
        
        for(int xr = rgn_b[0]; xr <= rgn_e[0]; xr++)
        {
            for(int yr = rgn_b[1]; yr <= rgn_e[1]; yr++)
            {
                for(int zr = rgn_b[2]; zr <= rgn_e[2]; zr++)
                {
                    if(xr < 0 || xr >= gWorld->regions_x ||
                       yr < 0 || yr >= gWorld->regions_y ||
                       zr < 0 || zr >= gWorld->regions_z)
                    {
                        // out of bounds
                        continue;
                    }
                    
                    WorldElemListNode* pListHead = world_region_head(xr*Rs - gWorld->bound_radius,
                                                                     yr*Rs - gWorld->bound_radius,
                                                                     zr*Rs - gWorld->bound_radius);
                
                    if(pListHead) world_elem_list_add_fast(pElem, pListHead, LIST_TYPE_REGION);
                    regions_spanned++;
                }
            }
        }
    }
    else
    {
        world_elem_list_add_fast(pElem, pRegionHead, LIST_TYPE_REGION);
    }
    
    return pRegionHead;
}

void
remove_element_from_region(WorldElem* pElem)
{
    if(gWorld->world_update_state.world_rgn_remove_hook != NULL) gWorld->world_update_state.world_rgn_remove_hook(pElem);
    
    if(pElem->spans_regions)
    {
        // walk list-back references and remove from all LIST_TYPE_REGION lists
        WorldElemListNode* pListHead;
        int i = 0;
        
        do
        {
            pListHead = world_elem_get_member_list_head(pElem, i);
            if(pListHead)
            {
                // head-node type is undefined now
                assert(pListHead->next);
                
                if(pListHead->next->type == LIST_TYPE_REGION)
                {
                    world_elem_list_remove(pElem, pListHead);
                    continue;
                }
            }   
            i++;
        } while(pListHead);
    }
    else
    {
        WorldElemListNode* pPrev =
            get_region_list_head(pElem->physics.ptr->x, pElem->physics.ptr->y, pElem->physics.ptr->z);
        if(!pPrev) return;
           
        world_elem_list_remove(pElem, pPrev);
    }
}

void
world_update_elem_removed_hook(WorldElem* pElem)
{
    world_update_state_t *state = &gWorld->world_update_state;
    if(state->world_region_iterate_cur && state->world_region_iterate_cur->elem == pElem)
    {
        // our position in the list was invalidated
        state->world_region_iterate_cur = state->world_region_iterate_cur->next;
    }
    
    if(state->ptr_objects_moving && state->ptr_objects_moving->elem == pElem)
    {
        state->ptr_objects_moving = NULL;
    }
}

void
world_update(float tc)
{
    // TODO: for meshes test collision by checking if object is < plane-normal vector
    int do_check_collisions = 1;
    int do_sound_checks = 1;
    float FRdiv = 64;
    float FRcollision = collision_repulsion_coeff;
    // HACK: -- apply min velocity "wobble back/forth"
    float Fmin = 0.01 * (tex_pass % 2) - 1;
    int iF;
    
    WorldElemListNode* pCur = gWorld->elements_moving.next;
    
    // TODO: first apply boundary enforcement, THEN iterate moving objects again and apply collision
    
    gWorld->world_update_state.world_remove_hook = world_update_elem_removed_hook;
    
    gWorld->world_update_state.ptr_objects_moving = &gWorld->elements_moving;
    while(gWorld->world_update_state.ptr_objects_moving && gWorld->world_update_state.ptr_objects_moving->next)
    {
        WorldElem* pElem;
        
        if(gWorld->world_update_state.ptr_objects_moving) gWorld->world_update_state.ptr_objects_moving = gWorld->world_update_state.ptr_objects_moving->next;
        
        //remove_retry:
        
        pElem = gWorld->world_update_state.ptr_objects_moving->elem;
        
        if(!pElem->head_elem) // not a child elem
        {
            
            float* pFloatCheckIsNan[] = {
                &pElem->physics.ptr->vx,
                &pElem->physics.ptr->vy,
                &pElem->physics.ptr->vz
            };
            for(iF = 0; iF < sizeof(pFloatCheckIsNan)/sizeof(float*); iF++)
            {
                if(isnan(*pFloatCheckIsNan[iF]))
                {
                    *pFloatCheckIsNan[iF] = 0;
                    printf("isnan: %p\n", pElem);
                }
            }
            
            if(pElem->physics.ptr->vx != 0 || pElem->physics.ptr->vy+Fmin != 0 || pElem->physics.ptr->vz != 0 || pElem->physics.ptr->gravity)
            {
                int out_of_bounds_remove = 0;
                
                if(pElem->physics.ptr->gravity)
                {
                    pElem->physics.ptr->vx += gWorld->vec_gravity[0] * tc;
                    pElem->physics.ptr->vy += gWorld->vec_gravity[1] * tc;
                    pElem->physics.ptr->vz += gWorld->vec_gravity[2] * tc;
                }
                
                if(pElem->physics.ptr->friction)
                {
                    pElem->physics.ptr->vx *= 1 - C_FRICTION;
                    pElem->physics.ptr->vy *= 1 - C_FRICTION;
                    pElem->physics.ptr->vz *= 1 - C_FRICTION;
                }
                
                float vm[] = {
                    pElem->physics.ptr->vx,
                    pElem->physics.ptr->vy,
                    pElem->physics.ptr->vz
                };
                
                // TODO: get rid of this it's expensive
                pElem->physics.ptr->velocity = sqrt(vm[0]*vm[0] + vm[1]*vm[1] + vm[2]*vm[2]);
                
                gWorld->world_update_state.world_rgn_remove_hook = NULL;
                
                remove_element_from_region(pElem);
                
                gWorld->world_update_state.world_rgn_remove_hook = world_update_elem_removed_hook;
                
                if(do_sound_checks)
                {
                    if(pElem->stuff.sound.emit_sound_id)
                    {
                        const char *soundName = gameSoundNames[pElem->stuff.sound.emit_sound_id];
                        
                        game_timeval_t next_play = pElem->stuff.sound.emit_sound_duration + pElem->stuff.sound.time_last_emit;
                        
                        if(time_ms >= next_play)
                        {
                            pElem->stuff.sound.time_last_emit = time_ms;
                            
                            gameAudioPlaySoundAtLocation(soundName,
                                                         pElem->physics.ptr->x,
                                                         pElem->physics.ptr->y,
                                                         pElem->physics.ptr->z);
                        }
                    }
                }
                
                // apply boundary enforcement, which takes priority over object-collision
                if(pElem->object_type != OBJ_DISPLAYONLY)
                {
                    int bounding_enforced = 0;
                    int i;

                    for(i = 0; i < gWorld->boundingRegion->nVectorsInited; i++)
                    {
                        float U[] = {vm[0]*tc, vm[1]*tc, vm[2]*tc};
                        float V[] = {gWorld->boundingRegion->v[i].f[3], gWorld->boundingRegion->v[i].f[4], gWorld->boundingRegion->v[i].f[5]};
                        float P[3];
                        
                        P[0] = (pElem->physics.ptr->x - gWorld->boundingRegion->v[i].f[0]) + U[0];
                        P[1] = (pElem->physics.ptr->y - gWorld->boundingRegion->v[i].f[1]) + U[1];
                        P[2] = (pElem->physics.ptr->z - gWorld->boundingRegion->v[i].f[2]) + U[2];
                        
                        float dot = dot2(V, P);
             
                        if(dot < 0.001)
                        {
                            if(pElem->bounding_remain)
                            {
                                pElem->physics.ptr->vx += V[0]*maxAccelDecel*-dot;
                                pElem->physics.ptr->vy += V[1]*maxAccelDecel*-dot;
                                pElem->physics.ptr->vz += V[2]*maxAccelDecel*-dot;
                                
                                bounding_enforced = 1;
                            }
                            else if(pElem->bounding_reflect)
                            {
                                if(V[0] / pElem->physics.ptr->vx < 0.0) pElem->physics.ptr->vx = -pElem->physics.ptr->vx;
                                if(V[1] / pElem->physics.ptr->vy < 0.0) pElem->physics.ptr->vy = -pElem->physics.ptr->vy;
                                if(V[2] / pElem->physics.ptr->vz < 0.0) pElem->physics.ptr->vz = -pElem->physics.ptr->vz;
                                
                                bounding_enforced = 1;
                            }
                            else
                            {
                                out_of_bounds_remove = 1;
                            }
                        }
                    }
                    
                    if(bounding_enforced)
                    {
                        game_handle_collision(pElem, NULL, COLLISION_ACTION_REPULSE);
                        gameNetwork_handle_collision(pElem, NULL, COLLISION_ACTION_REPULSE);
                        game_ai_collision(pElem, NULL, COLLISION_ACTION_REPULSE);
                    }
                    
                    if(!out_of_bounds_remove)
                    {
                        // MARK: -- attempt to move along vm
                        move_elem_relative(pElem, vm[0] * tc, vm[1] * tc, vm[2] * tc);
                        
                        // MARK: -- resolve collisions with other objects
                        collision_action_table_t* collision_actions_table_list[] =
                        {
                            &collision_actions,
                            NULL
                        };
                        int cl = 0;
                        
                        while(collision_actions_table_list[cl])
                        {
                            collision_action_table_t *collision_actions_cur = collision_actions_table_list[cl];
                            
                            if(do_check_collisions && pElem->collision_start_time <= time_ms)
                            {
                                int sound_played = 0;
                                
                                // check collisions
                                WorldElemListNode* pRegionElemsHead =
                                get_region_list_head(pElem->physics.ptr->x,
                                                     pElem->physics.ptr->y,
                                                     pElem->physics.ptr->z);

                                if(pRegionElemsHead)
                                {
                                    WorldElem* pRegionElem;
                                    
                                    region_collision_retry:
                                    
                                    if(FRcollision <= 0) break;
                                    
                                    gWorld->world_update_state.world_region_iterate_cur = pRegionElemsHead->next;
                                    
                                    while(gWorld->world_update_state.world_region_iterate_cur)
                                    {
                                        pRegionElem = gWorld->world_update_state.world_region_iterate_cur->elem;
                                        
                                        if(pRegionElem != pElem)
                                        {
                                            if(check_collision(pRegionElem, pElem))
                                            {
                                                WorldElem* pElemCollided = pRegionElem;
                                                
                                                // record collision
                                                collision_action_t colact = (*collision_actions_cur)[pElemCollided->object_type][pElem->object_type];
                                                
                                                if(
                                                   (!pElemCollided->destructible || !pElem->destructible) ||
                                                   colact == COLLISION_ACTION_NONE
                                                   )
                                                {
                                                    gWorld->world_update_state.world_region_iterate_cur = gWorld->world_update_state.world_region_iterate_cur->next;
                                                    continue;
                                                }
                                                
                                                //pElem->physics.ptr->x = collision_rollback_coord[0];
                                                //pElem->physics.ptr->y = collision_rollback_coord[1];
                                                //pElem->physics.ptr->z = collision_rollback_coord[2];
//                                                pElem->physics.ptr->velocity = collision_rollback_velocity;
//
//                                                // move all coordinates (for bounding boxes)
//                                                if(!rollback_done)
//                                                {
//                                                    move_elem_relative(pElem, -vm[0] * tc, vm[1] * tc, vm[2] * tc);
//                                                    rollback_done = 1;
//                                                }

                                                // HACK: removed a hack that enforced elemA had higher velocity than B and swapped
                                                
                                                if(colact == COLLISION_ACTION_REPULSE)
                                                {
                                                    world_repulse_elem(pElem, pElemCollided, tc, FRcollision);
                                                    
                                                    // re-start collision test at new (repulsed) coordinates
                                                    cl = 0;
                                                    FRcollision -= collision_repulsion_coeff/FRdiv;
                                                    
                                                    if(pElem->elem_id == my_ship_id && !sound_played)
                                                    {
                                                        sound_played = 1;
                                                        gameAudioPlaySoundAtLocation("bump",
                                                                                     pElem->physics.ptr->x,
                                                                                     pElem->physics.ptr->y,
                                                                                     pElem->physics.ptr->z);
                                                    }
                                                    
                                                    goto region_collision_retry;
                                                }

                                            collision_list_add_retry:
                                                {
                                                    WorldElemListNode *nA, *nB;
                                                    
                                                    nA = world_elem_list_find_elem(pElem, &gWorld->elements_collided);
                                                    nB = world_elem_list_find_elem(pElemCollided, &gWorld->elements_collided);
                                                    
                                                    if(!nA && !nB)
                                                    {
                                                        WorldElemListNode* pNodeA, *pNodeB;
                                                        
                                                        // HACK: order by durability to make collision-list parsing simpler (list is LIFO)
                                                        if(pElem->durability <= pElemCollided->durability)
                                                        {
                                                            pNodeB = world_elem_list_add(pElemCollided, &gWorld->elements_collided);
                                                        }
                                                        else
                                                        {
                                                            pNodeA = world_elem_list_add(pElem, &gWorld->elements_collided);
                                                        }
                                                        
                                                        if(pElem->durability <= pElemCollided->durability)
                                                        {
                                                            pNodeA = world_elem_list_add(pElem, &gWorld->elements_collided);
                                                        }
                                                        else
                                                        {
                                                            pNodeB = world_elem_list_add(pElemCollided, &gWorld->elements_collided);
                                                        }
                                                        
                                                        pNodeA->elem_collided = pNodeB->elem;
                                                        pNodeB->elem_collided = pNodeA->elem;
                                                        pNodeA->userarg = colact;
                                                        pNodeB->userarg = colact;
                                                        
                                                        //game_handle_collision(pElem, pElemCollided, colact);
                                                        //gameNetwork_handle_collision(pElem, pElemCollided, colact);
                                                        //game_ai_collision(pElem, pElemCollided, colact);
                                                    }
                                                    else
                                                    {
                                                        if((nA && nA->userarg < colact) ||
                                                           (nB && nB->userarg < colact))
                                                        {
                                                            if(nA)
                                                            {
                                                                if(nA->elem_collided) world_elem_list_remove(nA->elem_collided, &gWorld->elements_collided);
                                                                world_elem_list_remove(nA->elem, &gWorld->elements_collided);
                                                            }
                                                            
                                                            if(nB)
                                                            {
                                                                if(nB->elem_collided) world_elem_list_remove(nB->elem_collided, &gWorld->elements_collided);
                                                                world_elem_list_remove(nB->elem, &gWorld->elements_collided);
                                                            }
                                                            
                                                            goto collision_list_add_retry;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        
                                        // check if this was invalidated by the remove-callback
                                        if(!gWorld->world_update_state.world_region_iterate_cur)
                                        {
                                            gWorld->world_update_state.world_region_iterate_cur = pRegionElemsHead;
                                            FRcollision -= collision_repulsion_coeff/FRdiv;
                                        }
                                        
                                        gWorld->world_update_state.world_region_iterate_cur = gWorld->world_update_state.world_region_iterate_cur->next;
                                    }
                                }
                            }
                            cl++;
                        }
                        
                        // MARK: -- resolve collisions with other objects - DONE (restore collision priority)
                        
                        add_element_to_region(pElem);
                        
                        FRcollision = collision_repulsion_coeff;
                    }
                    else
                    {
                        WorldElem* pCurRemoveElem = pElem;

                        
                        // out of bounds
                        world_elem_list_remove(pCurRemoveElem, &gWorld->elements_expiring);
                        world_elem_list_remove(pCurRemoveElem, &gWorld->elements_list);
                        world_elem_list_add(pCurRemoveElem, &gWorld->elements_to_be_freed);
                        
                        gWorld->world_update_state.ptr_objects_moving = gWorld->world_update_state.ptr_objects_moving->next;
                        
                        world_elem_list_remove(pCurRemoveElem, &gWorld->elements_moving);
                    }
                }
            }
        }
    }
        
    // check for temporary objects
    pCur = gWorld->elements_expiring.next;
    while(pCur)
    {
        WorldElemListNode* pNext = pCur->next;
        
        pCur->elem->lifetime--;
        if(pCur->elem->lifetime <= 0)
        {
            WorldElem* pCurRemoveElem = pCur->elem;
            
            world_elem_list_remove(pCurRemoveElem, &gWorld->elements_expiring);
            world_elem_list_remove(pCurRemoveElem, &gWorld->elements_moving);
            world_elem_list_remove(pCurRemoveElem, &gWorld->elements_list);
            remove_element_from_region(pCurRemoveElem);
            
            world_elem_list_add(pCurRemoveElem, &gWorld->elements_to_be_freed);
        }
        
        pCur = pNext;
    }
    
    gWorld->world_update_state.world_remove_hook = NULL;
    gWorld->world_update_state.world_rgn_remove_hook = NULL;
}

void
convert_mesh_to_world_elems_helper(struct mesh_t* mesh, unsigned int texture_id, float tile_div,
                                   struct mesh_coordinate_t* vis_normal,
                                   unsigned int row_start, unsigned int col_start,
                                   unsigned int max_rows, unsigned int max_cols)
{
    WorldElem* pElem = NULL;
    int *mesh_to_elem_map = NULL;
    
    mesh_to_elem_map = malloc(sizeof(*mesh_to_elem_map) * (mesh->n_coordinates));
    if(!mesh_to_elem_map) return;
    
    // find point in the middle of this piece
    struct mesh_coordinate_t c1 = mesh->coordinates[MESH_COORD_IDX(mesh, col_start, row_start)];
    struct mesh_coordinate_t c2 = mesh->coordinates[MESH_COORD_IDX(mesh, col_start+max_cols, row_start+max_rows)];
    float x = c1.x + (c2.x-c1.x);
    float y = c1.y + (c2.y-c1.y);
    float z = c1.z + (c2.z-c1.z);
    // add new element
    int obj = world_add_object(MODEL_MESH, x, y, z, 0, 0, 0, 1, texture_id);
    if(obj < 0) return;
    
    pElem = world_get_last_object();
    
    //remove_element_from_region(pElem); // avoid inserting to same region twice
    
    pElem->n_indices = pElem->n_coords = pElem->n_texcoords = 0;
    pElem->texture_id = texture_id;
    pElem->destructible = 0;
    
    // first copy over mesh coordinates
    for(int row = row_start; row <= row_start+max_rows; row++)
    {
        for(int col = col_start; col <= col_start+max_cols; col++)
        {
            // save mapping of row/col to index
            int k = MESH_COORD_IDX(mesh, col, row);
            
            if(k < mesh->n_coordinates)
            {
                mesh_to_elem_map[MESH_COORD_IDX(mesh, col, row)] = pElem->n_coords/3;
                
                pElem->coords[pElem->n_coords] = mesh->coordinates[k].x;
                pElem->coords[pElem->n_coords+1] = mesh->coordinates[k].y;
                pElem->coords[pElem->n_coords+2] = mesh->coordinates[k].z;
                pElem->n_coords += 3;
                
                pElem->texcoords[pElem->n_texcoords] = (model_texcoord_t) col / (mesh->dim_x/tile_div);
                pElem->texcoords[pElem->n_texcoords+1] = (model_texcoord_t) row / (mesh->dim_y/tile_div);
                pElem->n_texcoords += 2;
            }
            else
            {
                //assert(false);
            }
        }
    }
    
    // now triangles
    for(int row = row_start; row < (row_start+max_rows); row++)
    {
        for(int col = col_start; col < (col_start+max_cols); col++)
        {
            if(MESH_COORD_IDX(mesh, col+1, row+1) < mesh->n_coordinates)
            {
                int s0 = mesh_to_elem_map[MESH_COORD_IDX(mesh, col, row)];
                int s1 = mesh_to_elem_map[MESH_COORD_IDX(mesh, col+1, row)];
                int s2 = mesh_to_elem_map[MESH_COORD_IDX(mesh, col, row+1)];
                int s3 = mesh_to_elem_map[MESH_COORD_IDX(mesh, col+1, row+1)];
                
                // copy over triangles
                unsigned int i = pElem->n_indices;
                // get dot product of view normal and plane-normal
                float viewdot = vis_normal->x*mesh->normal.x
                                + vis_normal->y*mesh->normal.y
                                + vis_normal->z*mesh->normal.z;
                
                if(viewdot >= 0)
                {
                    pElem->indices[i++] = s0;
                    pElem->indices[i++] = s1;
                    pElem->indices[i++] = s2;
                    
                    pElem->indices[i++] = s2;
                    pElem->indices[i++] = s1;
                    pElem->indices[i++] = s3;
                }
                else
                {
                    pElem->indices[i++] = s2;
                    pElem->indices[i++] = s1;
                    pElem->indices[i++] = s0;
                    
                    pElem->indices[i++] = s2;
                    pElem->indices[i++] = s3;
                    pElem->indices[i++] = s1;
                }
                pElem->n_indices += 6;
            }
        }
    }
    
    free(mesh_to_elem_map);
    
    // since element coordinates modified here update the bounding regions
    remove_element_from_region(pElem);
    add_element_to_region(pElem);
}

void
convert_mesh_to_world_elems(struct mesh_t* mesh, unsigned int texture_id, float tile_div, struct mesh_coordinate_t* vis_normal)
{
    int m = 2;
    
    for(int row_st = 0; row_st < mesh->dim_y; row_st += m)
    {
        for(int col_st = 0; col_st < mesh->dim_x; col_st += m)
        {
            convert_mesh_to_world_elems_helper(mesh, texture_id, tile_div, vis_normal, row_st, col_st, m, m);
        }
    }
}

void
world_add_mesh(float x, float y, float z,
               float dx_r, float dy_r, float dz_r,
               float dx_c, float dy_c, float dz_c,
               unsigned int mesh_r, unsigned int mesh_c,
               float vis_norm_x, float vis_norm_y, float vis_norm_z,
               int tex_id, float tiling_m)
{
    struct mesh_t* mesh_tmp =
    build_mesh(x, y, z,
               dx_r, dy_r, dz_r,
               dx_c, dy_c, dz_c,
               mesh_c, mesh_r);
    if(mesh_tmp)
    {
        struct mesh_coordinate_t vis_normal;
        
        vis_normal.x = -vis_norm_x;
        vis_normal.y = -vis_norm_y;
        vis_normal.z = -vis_norm_z;
        
        convert_mesh_to_world_elems(mesh_tmp, tex_id, tiling_m, &vis_normal);
        
        free_mesh(mesh_tmp);
    }
}

int
world_prepare_mesh(float x, float y, float z,
                   float dx_r, float dy_r, float dz_r,
                   float dx_c, float dy_c, float dz_c,
                   unsigned int mesh_r, unsigned int mesh_c)
{
    struct mesh_t* mesh_tmp =
        build_mesh(x, y, z,
                   dx_r, dy_r, dz_r,
                   dx_c, dy_c, dz_c,
                   mesh_c, mesh_r);
    
    if(world_pending_mesh) free_mesh(world_pending_mesh);
    world_pending_mesh = mesh_tmp;
    
    memset(world_pending_mesh_info, 0, sizeof(world_pending_mesh_info));
    
    if(world_pending_mesh) return 1;
    
    return 0;
}

float
world_mesh_pending_x()
{
    if(!world_pending_mesh) return 0;
    return world_pending_mesh->dim_x;
}

float
world_mesh_pending_y()
{
    if(!world_pending_mesh) return 0;
    return world_pending_mesh->dim_y;
}

void
world_set_mesh_object_info(float a, float b, float g)
{
    world_pending_mesh_info[0] = a;
    world_pending_mesh_info[1] = b;
    world_pending_mesh_info[2] = g;
}

void
world_complete_mesh(int type, int tex_id, float tiling_m, float scale)
{
    int r, c;
    if(world_pending_mesh)
    {        
        for(r = 0; r < world_pending_mesh->dim_x; r++)
        {
            for(c = 0; c < world_pending_mesh->dim_y; c++)
            {
                struct mesh_coordinate_t coord = world_pending_mesh->coordinates[MESH_COORD_IDX(world_pending_mesh, r, c)];
                
                int obj_id = world_add_object(type, coord.x, coord.y, coord.z,
                                              world_pending_mesh_info[0], world_pending_mesh_info[1],
                                              world_pending_mesh_info[2],
                                              scale, tex_id);
                (void)obj_id;
            }
        }
        
        free_mesh(world_pending_mesh);
        world_pending_mesh = NULL;
    }
}

void
world_convert_mesh_to_gltriangles(int tex_id)
{
    struct mesh_opengl_t* glmesh = mesh_to_opengl_triangles(world_pending_mesh, 1, 1);
    if(glmesh)
    {
        WorldElem* elemMesh = world_elem_alloc();
        if(elemMesh)
        {
            elemMesh->pVoid = (void*) glmesh;
            elemMesh->texture_id = tex_id;
            world_elem_list_add(elemMesh, &gWorld->triangle_mesh_head);
        }
    }
}

void
world_manip_mesh(float xy_coord[2], float d_xyz[3], float m_c)
{
    if(world_pending_mesh)
    {
        pull_mesh(world_pending_mesh, xy_coord[0], xy_coord[1], d_xyz[0], d_xyz[1], d_xyz[2], m_c);
    }
}

void
world_manip_mesh_round(float x, float y, float z)
{
    if(world_pending_mesh)
    {
        apply_mesh_rounding(world_pending_mesh, x, y, z);
    }
}

void
world_set_plane(int idx, float ox, float oy, float oz, float x1, float y1, float z1, float x2, float y2, float z2)
{
    gWorld->world_planes[idx].o[0] = ox;
    gWorld->world_planes[idx].o[1] = oy;
    gWorld->world_planes[idx].o[2] = oz;
    
    gWorld->world_planes[idx].v1[0] = x1;
    gWorld->world_planes[idx].v1[1] = y1;
    gWorld->world_planes[idx].v1[2] = z1;
    gWorld->world_planes[idx].v2[0] = x2;
    gWorld->world_planes[idx].v2[1] = y2;
    gWorld->world_planes[idx].v2[2] = z2;
}

void
world_add_drawline(float a[3], float b[3], float color[3], unsigned int lifetime)
{
    WorldElem* pElem = world_elem_alloc();
    if(pElem)
    {
        int i = 0;
        pElem->coords[i++] = a[0];
        pElem->coords[i++] = a[1];
        pElem->coords[i++] = a[2];
        pElem->coords[i++] = b[0];
        pElem->coords[i++] = b[1];
        pElem->coords[i++] = b[2];
        
        i = 0;
        pElem->texcoords[i] = color[i]; i++;
        pElem->texcoords[i] = color[i]; i++;
        pElem->texcoords[i] = color[i]; i++;
        
        pElem->lifetime = lifetime;
        
        world_elem_list_add(pElem, &gWorld->drawline_list_head);
    }
}

void
world_build_run_program(float x, float y, float z)
{
    float a, b;
    for(a = 0; a < M_PI*2; a += 0.4)
    {
        for(b = 0; b < 50; b += 10)
        {
            float c[] = {sin(a)*b, 0, cos(a)*b};
            int obj_id =
                world_add_object(MODEL_ICOSAHEDRON, x + c[0], y + c[1], z + c[2], 0, 0, 0, 2, TEXTURE_ID_BALL);
        }
    }
}

int
world_bounding_violations(float location[3], float vnormal[3])
{
    int i, d;
    for(i = 0; i < gWorld->boundingRegion->nVectorsInited; i++)
    {
        // normal unit vector
        float V[3];
        float P[3];
        
        for(d = 0; d < 3; d++) V[d] = gWorld->boundingRegion->v[i].f[3+d];
        
        P[0] = location[0] - gWorld->boundingRegion->v[i].f[0];
        P[1] = location[1] - gWorld->boundingRegion->v[i].f[1];
        P[2] = location[2] - gWorld->boundingRegion->v[i].f[2];
        
        float di = distance(P[0], P[1], P[2], 0, 0, 0);
        
        for(d = 0; d < 3; d++) P[d] /= di;
        
        float dot = dot2(V, P);
        
        if(dot < 0.001)
        {
            for(d = 0; d < 3; d++)
            {
                // return 1 and the normal vector that was in conflict
                vnormal[d] = V[d];
            }
            return 1;
        }
    }
    
    return 0;
}
