//
//  worldElem.h
//  gl_flight
//
//  Created by jbrady on 8/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef __WORLD_ELEM_H__
#define __WORLD_ELEM_H__

#include "models.h"
#include "object.h"
#include "simple_list.h"
#include "simple_hash.h"
#include "gameTimeval.h"

#define MAX_ELEM 128

#define IS_LINKED_ELEM(e) ((e)->head_elem != NULL)

#define WORLD_ELEM_ID_INVALID -1

//const static GLenum index_type_enum = /* GL_UNSIGNED_BYTE */ GL_UNSIGNED_INT;

struct WorldElemListNode;

typedef struct
{
    GLfloat vx, vy, vz;
    GLfloat x, y, z;
    GLfloat alpha, beta, gamma; //euler
    GLfloat velocity;
    
    int     gravity:1, friction:1;
} phys_data;

typedef struct
{
    GLfloat x_min, x_max;
    GLfloat y_min, y_max;
    GLfloat z_min, z_max;
} bounding_box;

struct ListNode_t
{
    struct WorldElemListNode* listElem;
    struct ListNode_t* listNext;
};

typedef struct ListNode_t ListNode_t;

struct WorldElem {
	model_index_t indices[MAX_ELEM];
	GLint n_indices;
	
	model_coord_t coords[MAX_ELEM];
	GLint n_coords;
	
	model_texcoord_t texcoords[MAX_ELEM];
	GLint n_texcoords;
    
    // TODO: unionize global data with non-global data fields to save mem
    unsigned int uses_global_coords;
    struct {
        model_index_t indices[MAX_ELEM];
        unsigned long coord_offset;
    } global_coord_data;
	
	struct {
		float distance;
		GLint tex_pass;
        char visible;
        GLint priority;
        char wireframe;
        char tex_adjust;
	} renderInfo;
    
    struct 
    {
        phys_data data;
        phys_data* ptr;
    } physics;
    
    struct WorldElem* linked_elem;
    struct WorldElem* head_elem;
    
    unsigned char type;
    
    unsigned char texture_id;
    
    int elem_id;
    int map_id;
    
    unsigned char destructible;
    int destroyed_by_id;
    float durability;
    int lifetime;
    float scale;
    
    int spans_regions:1;

    int invisible:1;
    
    int bounding_wrap:1;
    int bounding_remain:1;
    
    int in_visible_list:1;
    int visible_list_by_region_sorted:1;
    int ignore_collisions:1;
    int collision_handle_remove:1;
    int remove_pending:1;
    int moving:1;
    
    struct
    {
        int affiliation;
        int subtype;
        int radar_visible:1;
        int intelligent:1;
        int game_object_id;
        int network_created:1;
        char *nametag;
        
        struct
        {
            int emit_sound_id;
            game_timeval_t time_last_emit;
        } sound;
        
        union
        {
            struct
            {
                game_timeval_t time_last_spawn, time_spawn_interval, time_last_move, time_move_interval;
                float spawn_propability;
                float spawn_intelligence;
            } spawnpoint;
            
            struct
            {
                int inventory[OBJ_LAST];
            } player;
            
            struct
            {
                float x, y, z;
            } teleporter;
            
            struct
            {
                int enemy_state;
                int last_state;
                game_timeval_t time_last_run, time_last_bullet, time_last_trail, time_next_decision, time_last_deploy, time_target_acquired, time_run_interval;
                int target_id;
                float spawn_x, spawn_y, spawn_z;
                float tgt_x, tgt_y, tgt_z;
                float max_speed;
                float max_turn;
                float run_distance, pursue_distance, forget_distance;
                float intelligence;
                float pitch_last, yaw_last;
                int appearance;
                int fixed:1, changes_target:1, fires:1, fires_missles:1, leaves_trail:1, patrols_no_target:1, deploys_collect:1;
                int collided:1;
            } enemy;
        } u;
        
        struct
        {
            int action;
            float origin[3];
        } bullet;
        
        struct
        {
            unsigned int mask;
        } flags;
        
        int towed_elem_id;
        
    } stuff;
    
    Object object_type;
    
    void* pVoid;
    
    struct ListNode_t listRefHead;
};

typedef struct WorldElem WorldElem;

typedef enum
{
    LIST_TYPE_UNKNOWN = 0,
    LIST_TYPE_REGION = 1,
    LIST_TYPE_REGION_VIS = 2
} ListType;

struct WorldElemListNode {
    WorldElem* elem;
    struct WorldElemListNode* next;
    ListType type;
    simple_hash_table_t* hash_ptr;
};

typedef struct WorldElemListNode WorldElemListNode;

extern volatile WorldElemListNode* world_elem_list_remove_watch_elem;

inline static void
world_elem_list_init(WorldElemListNode* pHeadNode)
{
    memset(pHeadNode, 0, sizeof(*pHeadNode));
}

WorldElem*
world_elem_alloc();

WorldElem*
world_elem_clone(WorldElem*);

inline static void
world_elem_init(WorldElem* pElem, int elem_id)
{
    pElem->elem_id = elem_id;
    pElem->stuff.towed_elem_id = -1;
}

void
world_elem_free(WorldElem* pElem);

static inline void
world_elem_get_coord(WorldElem* pElem, int n, model_coord_t dest[3]) {
    dest[0] = pElem->coords[n*3];
    dest[1] = pElem->coords[n*3+1];
    dest[2] = pElem->coords[n*3+2];
}

static inline float
world_elem_get_velocity(WorldElem* pElem)
{
    /*
    return sqrt(pElem->physics.ptr->vx * pElem->physics.ptr->vx +
                pElem->physics.ptr->vy * pElem->physics.ptr->vy +
                pElem->physics.ptr->vz * pElem->physics.ptr->vz);
     */
    return pElem->physics.ptr->velocity;
}

void
world_elem_list_clear(WorldElemListNode* pHeadNode);

WorldElemListNode*
world_elem_list_add_fast(WorldElem* pElem, WorldElemListNode* pHeadNode, ListType type);

WorldElemListNode*
world_elem_list_add(WorldElem* pElem, WorldElemListNode* pHeadNode);

// a > b
typedef int (*world_elem_list_sort_compare_func)(WorldElem *a, WorldElem *b);

void
world_elem_list_add_sorted(WorldElemListNode* pHeadNode,
                           WorldElemListNode* pSkipListNode,
                           WorldElem* pElem,
                           world_elem_list_sort_compare_func compare_func);

WorldElemListNode*
world_elem_list_find(int elem_id, WorldElemListNode* pHeadNode);

WorldElemListNode*
world_elem_list_find_elem(WorldElem* pElem, WorldElemListNode* pHeadNode);

void
world_elem_list_remove(WorldElem* pElem, WorldElemListNode* pHeadNode);

WorldElemListNode*
world_elem_get_member_list_head(WorldElem* pElem, unsigned int n);

void
world_elem_list_sort_1(WorldElemListNode* pHeadNode,
                       world_elem_list_sort_compare_func compare_func,
                       int resume_sort, int max_steps);

void
world_elem_list_sort_1_with_max(WorldElemListNode* pHeadNode, world_elem_list_sort_compare_func compare_func,
                                int max_steps);

void
world_elem_list_sort_first_elem(WorldElemListNode* pHeadNode, world_elem_list_sort_compare_func compare_func);

void
world_elem_list_build_skip_list(WorldElemListNode* pHeadNode, WorldElemListNode* pSkipListHead, int nSkip);

void
world_elem_list_release_skip_list(WorldElemListNode* pSkipListHead);

void
world_elem_list_remove_skip_list(WorldElemListNode* pSkipListHead, WorldElem* pElem);

void
world_elem_set_nametag(WorldElem* elem, char* tag);

#endif