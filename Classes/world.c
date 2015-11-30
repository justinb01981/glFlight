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
inline static void get_region_bounding_box(float x, float y, float z, float region_bbox[6]);
static void get_element_bounding_box(WorldElem* pElem, float box[6]);
inline static int check_bounding_box_overlap(float boxA[6], float boxB[6]);
void update_regions();
void convert_mesh_to_world_elems(struct mesh_t* mesh, unsigned int texture_id, float tile_div, struct mesh_coordinate_t* vis_normal);
static void world_pack_geometry();
void world_build_visibility_data();
void world_update(float tc);

world_t *gWorld = NULL;
game_lock_t gWorldLock;

float C_THRUST = /*0.05*/ 0.035; // higher values = more speed
float C_FRICTION = /*0.04*/ 0.03; // higher values = more friction
float GYRO_FEEDBACK = GYRO_FEEDBACK_DEFAULT;
float GYRO_DC = GYRO_DC_DEFAULT;
float visible_distance = 100;

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
        pElem = world_elem_alloc();
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
            
        default:
            break;
	}
    
    // HACK: make some decisions based off model-type
    if(replace_id < 0)
    switch(type)
    {
        case MODEL_ICOSAHEDRON:
            new_durability = DURABILITY_ASTEROID;
            break;
          
        case MODEL_CUBE:
    	case MODEL_CUBE2:
            new_durability = DURABILITY_BLOCK;
            break;
            
        case MODEL_SHIP1:
            pElem->renderInfo.priority = 1;
            new_durability = 5;
            pElem->spans_regions = 0;
            pElem->bounding_remain = 1;
            pElem->bounding_wrap = 0;
            pElem->stuff.radar_visible = 1;
            break;
            
        case MODEL_SHIP2:
        case MODEL_SHIP3:
            pElem->renderInfo.priority = 1;
            new_durability = 5;
            pElem->spans_regions = 0;
            pElem->bounding_remain = 1;
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
            
        case MODEL_MESH:
            new_durability = 100;
            break;
            
        case MODEL_TELEPORTER:
            pElem->destructible = 0;
            new_durability = 100;
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
                memcpy(pElemNext, pElem, sizeof(*pElem));
                
                pElemNext->elem_id = elemSave.elem_id;
                pElemNext->linked_elem = elemSave.linked_elem;
                pElemNext->head_elem = elemSave.head_elem;
                pElemNext->listRefHead = elemSave.listRefHead;
                memcpy(&pElemNext->stuff, &elemSave.stuff, sizeof(pElemNext->stuff));
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
    }
    
    pElem = pElemHead;
    
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
        world_vis_rgn_add(visible_distance, pElem);
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
    
    {
        WorldElemListNode* pRemoveNode;
        pRemoveNode = world_elem_list_find(elem_id, &gWorld->elements_list);
        if(pRemoveNode) pElem = pRemoveNode->elem;
    }
    
    if(pElem && !pElem->remove_pending)
    {
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
                    loc[i++] = rand_in_range(0, M_PI*2 * 100) / 100; // alpha
                    loc[i++] = rand_in_range(0, M_PI*2 * 100) / 100; // beta
                    loc[i++] = rand_in_range(0, M_PI*2 * 100) / 100; // gamma
                    return;
                }
            }
            pCur = pCur->next;
        }
        
        if(!found_spawns) break;
        
        pCur = gWorld->elements_list.next;
    }
    
    // no spawn point found, spawn randomly
    loc[i++] = rand_in_range(1, gWorld->bound_x-1);
    loc[i++] = rand_in_range(1, gWorld->bound_y-1);
    loc[i++] = rand_in_range(1, gWorld->bound_z-1);
    loc[i++] = rand_in_range(0, M_PI*2 * 100) / 100; // alpha;
    loc[i++] = rand_in_range(0, M_PI*2 * 100) / 100; // beta;
    loc[i++] = rand_in_range(0, M_PI*2 * 100) / 100; // gamma;
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
        
        if(gWorld->shared_texcoords) free(gWorld->shared_texcoords);
        if(gWorld->shared_coordinates) free(gWorld->shared_coordinates);
        
        if(gWorld->boundingRegion)
        {
            boundingRegionUninit(gWorld->boundingRegion);
        }
        
		free(gWorld);
		gWorld = NULL;
	}
}

void world_init(float size_x, float size_y, float size_z)
{			
    int use_test_level = 0;
    
	gWorld = malloc(sizeof(world_t));
	memset(gWorld, 0, sizeof(world_t));
        
    if(size_x == -1 && size_y == -1 && size_z == -1)
    {
        use_test_level = 1;
        size_x = 200;
        size_y = 100;
        size_z = 200;
    }
    
    gWorld->elem_id_next = 1000;
    
    gWorld->element_id_hash = simple_hash_table_init(1024);
    gWorld->elements_list.hash_ptr = gWorld->element_id_hash;
	
	gWorld->bound_x = size_x;
	gWorld->bound_y = size_y;
	gWorld->bound_z = size_z;
    
    gWorld->region_size_x = gWorld->bound_x / WORLD_MAX_REGIONS;
    gWorld->region_size_y = gWorld->bound_y / WORLD_MAX_REGIONS;
    gWorld->region_size_z = gWorld->bound_z / WORLD_MAX_REGIONS;
    
    assert(floor(gWorld->region_size_x) == gWorld->region_size_x);
    assert(floor(gWorld->region_size_y) == gWorld->region_size_y);
    assert(floor(gWorld->region_size_z) == gWorld->region_size_z);
    
    // build bounding vectors
    boundingRegion* br = boundingRegionInit(6);
    boundingRegionAddVec(br, 0, 0, 0, 1, 0, 0);
    boundingRegionAddVec(br, 0, 0, 0, 0, 1, 0);
    boundingRegionAddVec(br, 0, 0, 0, 0, 0, 1);
    boundingRegionAddVec(br, gWorld->bound_x, gWorld->bound_y, gWorld->bound_z, -1, 0, 0);
    boundingRegionAddVec(br, gWorld->bound_x, gWorld->bound_y, gWorld->bound_z, 0, -1, 0);
    boundingRegionAddVec(br, gWorld->bound_x, gWorld->bound_y, gWorld->bound_z, 0, 0, -1);
    gWorld->boundingRegion = br;
    
    // gravity vector
    gWorld->vec_gravity[1] = -2.0;
    
    // map building begins here
    if(use_test_level)
    {
        int n_barriers = 100;
        while(n_barriers > 0)
        {
            int x = rand() % (int) gWorld->bound_x;
            int y = rand() % (int) gWorld->bound_y;
            int z = rand() % (int) gWorld->bound_z;
            
            world_add_object(MODEL_CUBE, x, y, z, 0, 0, 0, rand() % 5 + 1, 0);
            n_barriers--;
        }
        
        /*
        int n_pillars = 50;
        while(n_pillars > 0)
        {
            int x = rand() % (int) gWorld->bound_x;
            //int y = rand() % (int) gWorld->bound_y;
            int y = 2;
            int z = rand() % (int) gWorld->bound_z;
            
            for(int f = 0; f < 24; f += 4)
            {
                world_add_object(MODEL_CUBE, x, y + f, z, 0, 0, 0, 4, 3);  
            }
            n_pillars--;
        }
        
        int n_enemies = 0;
        while(n_enemies > 0)
        {
            int x = rand() % (int) gWorld->bound_x;
            int y = rand() % (int) gWorld->bound_y;
            int z = rand() % (int) gWorld->bound_z;
            
            world_add_object(MODEL_SHIP1, x, y, z, 0, 0, 0, 1, 1);
            n_enemies--;
        }
         */
        
        /****/

        // if mesh results in a concave object then face-culling will not work correctly
        // TODO: build mesh, then cut it into many elements and add to world
        // make an attempt to share coordinates
        unsigned int m = 20;
        unsigned int mesh_row_size = gWorld->bound_x / m;
        unsigned int mesh_col_size = gWorld->bound_x / m;
        unsigned int mesh_rows_n = m;
        unsigned int mesh_cols_n = m;
        unsigned int mesh_random_pulls = 10;
        float mesh_pull_height = 10.0;
        float mesh_propagation_c = 0.6;
        struct mesh_t *mesh_tmp;
        struct mesh_coordinate_t vis_normal;
        
        // floor
        mesh_tmp = build_mesh(0, 0, 0,
                              mesh_row_size, 0, 0,
                              0, 0, mesh_col_size,
                              mesh_cols_n, mesh_rows_n);
        if(mesh_tmp)
        {
            for(int i = 0; i < mesh_random_pulls; i++)
            {
                int x = rand() % mesh_cols_n;
                int y = rand() % mesh_rows_n;
                pull_mesh(mesh_tmp, x, y, 0, mesh_pull_height, 0, mesh_propagation_c);
            }
            
            vis_normal.x = vis_normal.z = 1;
            vis_normal.y = -1;
            convert_mesh_to_world_elems(mesh_tmp, 10, 1, &vis_normal);
            
            free_mesh(mesh_tmp);
        }

        // ceiling
        mesh_tmp = build_mesh(0, gWorld->bound_y, 0,
                              mesh_row_size, 0, 0,
                              0, 0, mesh_col_size,
                              mesh_cols_n, mesh_rows_n);
        if(mesh_tmp)
        {        
            vis_normal.x = vis_normal.z = 1;
            vis_normal.y = 1;
            convert_mesh_to_world_elems(mesh_tmp, 12, 1, &vis_normal);
            
            free_mesh(mesh_tmp);
        }
        
        // wall z0
        mesh_tmp = build_mesh(0, 0, 0,
                              mesh_row_size, 0, 0,
                              0, mesh_col_size, 0,
                              mesh_cols_n, mesh_rows_n);
        if(mesh_tmp)
        {
            vis_normal.x = vis_normal.y = 1;
            vis_normal.z = -1;
            convert_mesh_to_world_elems(mesh_tmp, 4, 10, &vis_normal);
            
            free_mesh(mesh_tmp);
        }
        
        // wall z1
        mesh_tmp = build_mesh(0, 0, gWorld->bound_z,
                              mesh_row_size, 0, 0,
                              0, mesh_col_size, 0,
                              mesh_cols_n, mesh_rows_n);
        if(mesh_tmp)
        {
            vis_normal.x = vis_normal.y = 1;
            vis_normal.z = 1;
            convert_mesh_to_world_elems(mesh_tmp, 4, 10, &vis_normal);
            
            free_mesh(mesh_tmp);
        }
        
        // wall x0
        mesh_tmp = build_mesh(0, 0, 0,
                              0, 0, mesh_row_size,
                              0, mesh_col_size, 0,
                              mesh_cols_n, mesh_rows_n);
        if(mesh_tmp)
        {
            vis_normal.x = vis_normal.y = 1;
            vis_normal.x = -1;
            convert_mesh_to_world_elems(mesh_tmp, 4, 10, &vis_normal);
            
            free_mesh(mesh_tmp);
        }
        
        // wall x1
        mesh_tmp = build_mesh(gWorld->bound_x, 0, 0,
                              0, 0, mesh_row_size,
                              0, mesh_col_size, 0,
                              mesh_cols_n, mesh_rows_n);
        if(mesh_tmp)
        {
            vis_normal.x = vis_normal.y = 1;
            vis_normal.x = 1;
            convert_mesh_to_world_elems(mesh_tmp, 4, 10, &vis_normal);
            
            free_mesh(mesh_tmp);
        }
    
    }
    
    //world_build_visibility_data();

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
        
        world_vis_rgn_remove(pFreeElem);
        
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

static void
world_pack_geometry(WorldElemListNode* listHead)
{
    unsigned long coordinates_n = 0;
    
    WorldElemListNode* pCurNode = listHead->next;
    while(pCurNode)
    {
        coordinates_n += pCurNode->elem->n_coords;
        pCurNode = pCurNode->next;
    }
    
    unsigned int coord_size = sizeof(model_coord_t) * coordinates_n;
    gWorld->shared_coordinates = malloc(coord_size);
    if(!gWorld->shared_coordinates) return;
    
    gWorld->shared_texcoords = malloc((coordinates_n/3) * sizeof(model_texcoord_t) * 2);
    if(!gWorld->shared_texcoords)
    {
        free(gWorld->shared_coordinates);
        return;
    }
    
    model_coord_t *pCoord = gWorld->shared_coordinates;
    unsigned long coord_offset = 0;
    
    model_texcoord_t* pTexCoord = gWorld->shared_texcoords;
    
    pCurNode = gWorld->elements_list.next;
    while(pCurNode)
    {
        // only pack static objects
        if(/*pCurNode->elem->type != MODEL_CUBE*/ !object_is_static(pCurNode->elem->object_type))
        {
            // skip
            pCurNode = pCurNode->next;
            continue;
        }
        
        pCurNode->elem->uses_global_coords = 1;
    
        for(unsigned int i = 0; i < pCurNode->elem->n_coords; i++)
        {
            *pCoord = pCurNode->elem->coords[i];
            pCoord++;
        }
        
        for(unsigned int i = 0; i < pCurNode->elem->n_texcoords; i++)
        {
            *pTexCoord = pCurNode->elem->texcoords[i];
            pTexCoord++;
        }
        
        for(unsigned int i = 0; i < pCurNode->elem->n_indices; i++)
        {
            pCurNode->elem->global_coord_data.indices[i] =
                pCurNode->elem->indices[i]+coord_offset;
        }
        
        pCurNode->elem->global_coord_data.coord_offset = coord_offset;
        
        coord_offset += pCurNode->elem->n_coords/3;
        
        pCurNode = pCurNode->next;
    }
}

void world_optimize()
{
    world_pack_geometry(&gWorld->elements_list);
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

void move_elem_wrapped(WorldElem* elem)
{
    float m[3];
    float b[3];
    
    b[0] = gWorld->bound_x;
    b[1] = gWorld->bound_y;
    b[2] = gWorld->bound_z;
    m[0] = elem->physics.ptr->x;
    m[1] = elem->physics.ptr->y;
    m[2] = elem->physics.ptr->z;
    for(int d = 0; d < 3; d++)
    {
        if(m[d] < 0) m[d] += b[d];
        if(m[d] > b[d]) m[d] -= b[d];
        
    }
    
    world_move_elem(elem, m[0], m[1], m[2], 0);
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
world_repulse_elem(WorldElem* pCollisionA, WorldElem* pCollisionB, float tc)
{
    float mv[3];
    float s = collision_repulsion_coeff;
    float repulse_min = 0.001;

    float v = sqrt((pCollisionA->physics.ptr->vx*pCollisionA->physics.ptr->vx) +
                   (pCollisionA->physics.ptr->vy*pCollisionA->physics.ptr->vy) +
                   (pCollisionA->physics.ptr->vz*pCollisionA->physics.ptr->vz));
    
    /*
    mv[0] = (pCollisionA->physics.ptr->x - pCollisionB->physics.ptr->x) * fabs(pCollisionA->physics.ptr->vx) * tc * s;
    mv[1] = (pCollisionA->physics.ptr->y - pCollisionB->physics.ptr->y) * fabs(pCollisionA->physics.ptr->vy) * tc * s;
    mv[2] = (pCollisionA->physics.ptr->z - pCollisionB->physics.ptr->z) * fabs(pCollisionA->physics.ptr->vz) * tc * s;
     */
    mv[0] = (pCollisionA->physics.ptr->x - pCollisionB->physics.ptr->x) * v * tc * s;
    mv[1] = (pCollisionA->physics.ptr->y - pCollisionB->physics.ptr->y) * v * tc * s;
    mv[2] = (pCollisionA->physics.ptr->z - pCollisionB->physics.ptr->z) * v * tc * s;

    for(int i = 0; i < 3; i++) if(fabs(mv[i]) < repulse_min) mv[i] = 0.1;

    world_move_elem(pCollisionA, mv[0], mv[1], mv[2], 1);
}

inline static void
get_region_bounding_box(float x, float y, float z, float region_bbox[6])
{
    float xm = gWorld->bound_x / WORLD_MAX_REGIONS;
    float ym = gWorld->bound_y / WORLD_MAX_REGIONS;
    float zm = gWorld->bound_z / WORLD_MAX_REGIONS;
    
    region_bbox[0] = x * xm; // xmin
    region_bbox[3] = (x+1) * xm; // ymax
    
    region_bbox[1] = y * ym; // ymin
    region_bbox[4] = (y+1) * ym; // ymax
    
    region_bbox[2] = z * zm; // zmin
    region_bbox[5] = (z+1) * zm; // zmax
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
    
    boxA = boxA_in;
    boxB = boxB_in;

    amin = boxA[0]; amax = boxA[3];
    bmin = boxB[0]; bmax = boxB[3];
    if(amax < bmin || amin > bmax) return 0;
    
    amin = boxA[1]; amax = boxA[4];
    bmin = boxB[1]; bmax = boxB[4];
    if(amax < bmin || amin > bmax) return 0;
    
    amin = boxA[2]; amax = boxA[5];
    bmin = boxB[2]; bmax = boxB[5];
    if(amax < bmin || amin > bmax) return 0;
    
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

static inline void
region_for_coord(float x, float y, float z, int rgn[3])
{
    rgn[0] = floor(x / gWorld->region_size_x);
    rgn[1] = floor(y / gWorld->region_size_y);
    rgn[2] = floor(z / gWorld->region_size_z);
}

WorldElemListNode*
get_region_list_head(float x, float y, float z)
{
    int rgn[3];
    region_for_coord(x, y, z, rgn);
    
    if(rgn[0] < 0 || rgn[0] >= WORLD_MAX_REGIONS ||
       rgn[1] < 0 || rgn[1] >= WORLD_MAX_REGIONS ||
       rgn[2] < 0 || rgn[2] >= WORLD_MAX_REGIONS) return NULL;
    
    return &(gWorld->elements_by_region[rgn[0]][rgn[1]][rgn[2]]);
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
    
    float x = pElem->physics.ptr->x;
    float y = pElem->physics.ptr->y;
    float z = pElem->physics.ptr->z;
    
    WorldElemListNode* pCur = get_region_list_head(x, y, z);
    pRegionHead = pCur;
    
    if(!pCur) return pRegionHead;
    
    if(pElem->spans_regions)
    {
        get_element_bounding_box(pElem, elem_bbox);
        
        /*
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
         */
        
        int regions_spanned = 0;
        
        int rgn_b[3];
        region_for_coord(elem_bbox[0], elem_bbox[1], elem_bbox[2], rgn_b);
        
        int rgn_e[3];
        region_for_coord(elem_bbox[3], elem_bbox[4], elem_bbox[5], rgn_e);
        
        //memset(regions_filled, 0, sizeof(regions_filled));
        
        for(int xr = rgn_b[0]; xr <= rgn_e[0]; xr++)
        {
            for(int yr = rgn_b[1]; yr <= rgn_e[1]; yr++)
            {
                for(int zr = rgn_b[2]; zr <= rgn_e[2]; zr++)
                {
                    if(xr >= 0 && xr < WORLD_MAX_REGIONS && yr >= 0 && yr < WORLD_MAX_REGIONS && zr >= 0 && zr < WORLD_MAX_REGIONS)
                    {
                        WorldElemListNode* pListHead = get_region_list_head(xr*gWorld->region_size_x, yr*gWorld->region_size_y, zr*gWorld->region_size_z);
                    
                        if(pListHead) world_elem_list_add_fast(pElem, pListHead, LIST_TYPE_REGION);
                        regions_spanned++;
                    }
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
world_update(float tc)
{
    // TODO: for meshes test collision by checking if object is < plane-normal vector
    int do_check_collisions = 1;
    int do_check_boundaries = 1;
    int do_oob_gravity_sound = 1;
    int do_sound_checks = 1;
    
    
    WorldElemListNode* pRegionElemsHead;
    WorldElemListNode* pCur = gWorld->elements_moving.next;
    
    while(pCur)
    {
        int retry = 0;
        WorldElem* pElem = pCur->elem;
        
        // elements with packed coords cannot move currently
        if(pElem->uses_global_coords)
        {
            pCur = pCur->next;
            continue;
        }
        
        if(!pElem->head_elem) // not a child elem
        {
            pRegionElemsHead =
            get_region_list_head(pCur->elem->physics.ptr->x,
                                 pCur->elem->physics.ptr->y,
                                 pCur->elem->physics.ptr->z);
            
            if(pCur->elem->physics.ptr->vx != 0 || pCur->elem->physics.ptr->vy != 0 || pCur->elem->physics.ptr->vz != 0 || pCur->elem->physics.ptr->gravity)
            {
                int out_of_bounds_remove = 0;
                int my_ship_oob = 0;
                
                if(pCur->elem->physics.ptr->gravity)
                {
                    pCur->elem->physics.ptr->vx += gWorld->vec_gravity[0] * tc;
                    pCur->elem->physics.ptr->vy += gWorld->vec_gravity[1] * tc;
                    pCur->elem->physics.ptr->vz += gWorld->vec_gravity[2] * tc;
                }
                
                if(pCur->elem->physics.ptr->friction)
                {
                    pCur->elem->physics.ptr->vx *= 1 - C_FRICTION;
                    pCur->elem->physics.ptr->vy *= 1 - C_FRICTION;
                    pCur->elem->physics.ptr->vz *= 1 - C_FRICTION;
                }
                
                float vm[] = {
                    pCur->elem->physics.ptr->vx,
                    pCur->elem->physics.ptr->vy,
                    pCur->elem->physics.ptr->vz
                };
                
                remove_element_from_region(pCur->elem);
                
                move_elem_relative(pCur->elem, vm[0] * tc, vm[1] * tc, vm[2] * tc);
                
                // TODO: get rid of this it's expensive
                pCur->elem->physics.ptr->velocity = sqrt(vm[0]*vm[0] + vm[1]*vm[1] + vm[2]*vm[2]);
                
                if(do_check_boundaries)
                {
                    // only doing complex bounding test for some objects (players)
                    if(pCur->elem->bounding_remain)
                    {
                        float v_oob[3];
                        float g = /*0.05*/ 0.2;
                        int sound_played = 0;
                        
                        // HACK: janky while loop but avoids having to return magnitude of oob-vector
                        while(!boundingRegionCheckPointInside(gWorld->boundingRegion,
                                                              pCur->elem->physics.ptr->x,
                                                              pCur->elem->physics.ptr->y,
                                                              pCur->elem->physics.ptr->z,
                                                              v_oob))
                        {
                            out_of_bounds_remove = 0;
                            
                            if(!pCur->elem->bounding_wrap)
                            {
                                float v_gravity[3];
                                
                                v_gravity[0] = v_oob[0] * g;
                                v_gravity[1] = v_oob[1] * g;
                                v_gravity[2] = v_oob[2] * g;
                                
                                move_elem_relative(pCur->elem, v_gravity[0], v_gravity[1], v_gravity[2]);
                                
                                if(do_oob_gravity_sound && pCur->elem->elem_id == my_ship_id)
                                {
                                    my_ship_oob = 1;
                                }
                            }
                            else
                            {
                                move_elem_wrapped(pCur->elem);
                            }
                            
                            g *= 1.2;
                        }
                        
                        add_element_to_region(pCur->elem);
                        
                        if(my_ship_oob && !sound_played)
                        {
                            sound_played = 1;
                            gameAudioPlaySoundAtLocation("bump",
                                                         pCur->elem->physics.ptr->x,
                                                         pCur->elem->physics.ptr->y,
                                                         pCur->elem->physics.ptr->z);
                        }
                    }
                    else
                    {
                        if(!add_element_to_region(pCur->elem))
                        {
                            out_of_bounds_remove = 1;
                        }
                    }
                }
                
                if(out_of_bounds_remove)
                {
                    WorldElem* pCurRemoveElem = pElem;
                    while(pCurRemoveElem)
                    {
                        // out of bounds
                        world_elem_list_remove(pCurRemoveElem, &gWorld->elements_expiring);
                        world_elem_list_remove(pCurRemoveElem, &gWorld->elements_moving);
                        world_elem_list_remove(pCurRemoveElem, &gWorld->elements_list);

                        world_elem_list_add(pCurRemoveElem, &gWorld->elements_to_be_freed);
                        
                        pCurRemoveElem = NULL;
                    }
                    
                    retry = 1;
                }
                else // added to region successfully
                {
                    collision_action_table_t* collision_actions_table_list[] =
                        {
                            &collision_actions_stage1,
                            &collision_actions,
                            NULL
                        };
                    int cl = 0;
                    
                    while(collision_actions_table_list[cl])
                    {
                        collision_action_table_t *collision_actions_cur = collision_actions_table_list[cl];
                        
                        if(do_check_collisions)
                        {
                            int sound_played = 0;
                            int repulsed;
                            int repulse_retry = 100;
                            
                            do
                            {
                                repulsed = 0;
                                
                                // check collisions
                                WorldElemListNode* pRegionElemsHead =
                                    get_region_list_head(pCur->elem->physics.ptr->x,
                                                         pCur->elem->physics.ptr->y,
                                                         pCur->elem->physics.ptr->z);
                                
                                if(pRegionElemsHead)
                                {
                                    WorldElemListNode* pRegionElemsCur = pRegionElemsHead->next;
                                    
                                    while(pRegionElemsCur)
                                    {
                                        if(pRegionElemsCur->elem != pCur->elem)
                                        {
                                            if(pRegionElemsCur->elem->type == MODEL_TELEPORTER &&
                                               check_collision(pRegionElemsCur->elem, pCur->elem))
                                            {
                                                remove_element_from_region(pCur->elem);
                                                
                                                float dx = pCur->elem->physics.ptr->x - pRegionElemsCur->elem->stuff.u.teleporter.x;
                                                float dy = pCur->elem->physics.ptr->y - pRegionElemsCur->elem->stuff.u.teleporter.y;
                                                float dz = pCur->elem->physics.ptr->z - pRegionElemsCur->elem->stuff.u.teleporter.z;
                                                
                                                move_elem_relative(pCur->elem, -dx, -dy, -dz);
                                                add_element_to_region(pCur->elem);
                                            }
                                            else if(pRegionElemsCur->elem->destructible &&
                                                    pCur->elem->destructible &&
                                                    check_collision(pRegionElemsCur->elem, pCur->elem))
                                            {
                                                WorldElem *elemA = pRegionElemsCur->elem;
                                                WorldElem *elemB = pCur->elem;
                                                
                                                // elemA = slower
                                                // elemB = faster
                                                if(elemA->physics.ptr->velocity > elemB->physics.ptr->velocity)
                                                {
                                                    WorldElem* elemTmp = elemB;
                                                    elemB = elemA;
                                                    elemA = elemTmp;
                                                }
                                                
                                                collision_action_t colact = (*collision_actions_cur)[elemB->object_type][elemA->object_type];
                                                
                                                if(colact == COLLISION_ACTION_REPULSE)
                                                {
                                                    repulsed = 1;
                                                    
                                                    // repulse
                                                    world_repulse_elem(elemB, elemA, tc);
                                                    
                                                    if(elemB->elem_id == my_ship_id && !sound_played)
                                                    {
                                                        sound_played = 1;
                                                        gameAudioPlaySoundAtLocation("bump",
                                                                                     elemA->physics.ptr->x,
                                                                                     elemA->physics.ptr->y,
                                                                                     elemA->physics.ptr->z);
                                                    }
                                                }
                                                else if(colact == COLLISION_ACTION_NONE)
                                                {
                                                    // ignored
                                                }
                                                else
                                                {
                                                    if(!world_elem_list_find_elem(elemA, &gWorld->elements_collided) &&
                                                       !world_elem_list_find_elem(elemB, &gWorld->elements_collided))
                                                    {
                                                        WorldElemListNode* pNodeA =
                                                            world_elem_list_add(elemA, &gWorld->elements_collided);
                                                        
                                                        WorldElemListNode* pNodeB =
                                                            world_elem_list_add(elemB, &gWorld->elements_collided);
                                                        
                                                        pNodeA->userarg = colact;
                                                        pNodeB->userarg = colact;
                                                    }
                                                }
                                            }
                                        }
                                        
                                        if(repulsed) break;
                                        
                                        pRegionElemsCur = pRegionElemsCur->next;
                                    }
                                }
                                repulse_retry--;
                            } while(repulsed && repulse_retry > 0);
                        }
                        cl++;
                    }
                    
                    if(do_sound_checks)
                    {
                        if(pCur->elem->stuff.sound.emit_sound_id)
                        {
                            const char *soundName = gameSoundNames[pCur->elem->stuff.sound.emit_sound_id];
                            
                            game_timeval_t next_play = gameAudioSoundDuration(soundName) + pCur->elem->stuff.sound.time_last_emit;
                            
                            if(time_ms >= next_play)
                            {
                                pCur->elem->stuff.sound.time_last_emit = time_ms;
                                
                                gameAudioPlaySoundAtLocation(soundName,
                                                             pCur->elem->physics.ptr->x,
                                                             pCur->elem->physics.ptr->y,
                                                             pCur->elem->physics.ptr->z);
                                
                            }
                        }
                    }
                }
            }
        }
        
        if(retry)
        {
            pCur = gWorld->elements_moving.next;
        }
        else
        {
            pCur = pCur->next;
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

static struct mesh_t* world_pending_mesh = NULL;
static float world_pending_mesh_info[3];

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
world_vis_rgn_remove(WorldElem* elem)
{
    if(elem->visible_list_by_region_sorted)
    {
        // walk list-back references and remove from all LIST_TYPE_REGION lists
        WorldElemListNode* pListHead;
        int i = 0;
        
        do
        {
            pListHead = world_elem_get_member_list_head(elem, i);
            if(pListHead)
            {
                // head-node type is undefined now
                assert(pListHead->next);
                
                if(pListHead->next->type == LIST_TYPE_REGION_VIS)
                {
                    world_elem_list_remove(elem, pListHead);
                    continue;
                }
            }
            i++;
        } while(pListHead);
        
        elem->visible_list_by_region_sorted = 0;
    }
}

void
world_vis_rgn_add(float vis_dist, WorldElem* elem)
{
    float m[3] = {
        gWorld->bound_x / WORLD_VIS_REGIONS_X,
        gWorld->bound_y / WORLD_VIS_REGIONS_Y,
        gWorld->bound_z / WORLD_VIS_REGIONS_Z
    };
    
    if(elem->visible_list_by_region_sorted)
    {
        return;
    }
    
    for(int x = 0; x < WORLD_VIS_REGIONS_X; x++)
    {
        for(int y = 0; y < WORLD_VIS_REGIONS_Y; y++)
        {
            for(int z = 0; z < WORLD_VIS_REGIONS_Z; z++)
            {
                float p[3] = {
                    m[0] * x + (m[0] / 2),
                    m[1] * y + (m[1] / 2),
                    m[2] * z + (m[2] / 2)
                };
                
                float d = distance(elem->physics.ptr->x, elem->physics.ptr->y, elem->physics.ptr->z,
                                   p[0], p[1], p[2]);
                
                if(fabs(d) <= vis_dist)
                {
                    world_elem_list_add_fast(elem, &gWorld->elements_visible_by_region[x][y][z],
                                             LIST_TYPE_REGION_VIS);
                }
            }
        }
    }
    
    elem->visible_list_by_region_sorted = 1;
}
