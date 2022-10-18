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

#define MAX_ELEM 256
#define MODEL_EXTENSION_MAX 1024

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
    size_t size;
    
    model_index_t *indices;
	model_index_t indices_[MAX_ELEM];
	GLint n_indices;
	
    model_coord_t *coords;
	model_coord_t coords_[MAX_ELEM];
	GLint n_coords;
	
    model_texcoord_t *texcoords;
	model_texcoord_t texcoords_[MAX_ELEM];
	GLint n_texcoords;
    
    void* btree_node;
	
	struct {
		float distance;
		GLint tex_pass;
        char visible;
        GLint priority;
        GLint concavepoly;
        char wireframe;
        char tex_adjust;
        char tex_phase;
	} renderInfo;
    
    struct 
    {
        phys_data data;
        phys_data* ptr;
    } physics;
    
    struct
    {
        model_coord_t* pnorm;
        int normals;
    } bounding;
    
    struct
    {
        model_coord_t last_coord[3];
    } trail;
    
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
    int bounding_reflect:1;
    
    int in_visible_list:1;
    int visible_list_by_region_sorted:1;
    int collision_handle_remove:1;
    int remove_pending:1;
    int moving:1;
    game_timeval_t collision_start_time;
    
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
            unsigned int emit_sound_duration;
            game_timeval_t time_last_emit;
        } sound;
        
        union
        {
            struct
            {
                //float spawn_coeff;
                float spawn_intelligence;
                float spawn_last;
            } spawnpoint;
            
            struct
            {
                float theta;
                float radius;
                float period;
            } orbiter;
            
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
                game_timeval_t time_last_run, time_last_bullet, time_last_trail, time_next_retarget, time_last_deploy, time_run_interval;
                int target_id;
                float vthrust[3];
                float tgt_x, tgt_y, tgt_z;
                float speed, max_speed;
                float max_slerp;
                float run_distance, pursue_distance, scan_distance;
                float intelligence;
                //float pitch_last, yaw_last;
                int appearance;
                int changes_target:1, fires:1, fires_missles:1, leaves_trail:1, patrols_no_target_jukes:1, deploys_collect:1, ignore_player:1, ignore_collect:1;
                int collided;
            } enemy;
            
            struct
            {
                float x1[3];
                float x2[3];
                float color[3];
                float width;
            } drawline;
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
    
    struct {
        model_index_t indices_[1];
        model_coord_t coords_[1];
        model_texcoord_t texcoords_[1];
    } model_data_extension_stub;
    
};

typedef struct {
    model_index_t indices_[MODEL_EXTENSION_MAX];
    model_coord_t coords_[MODEL_EXTENSION_MAX];
    model_texcoord_t texcoords_[MODEL_EXTENSION_MAX];
} model_data_extension_t;

typedef struct WorldElem WorldElem;

typedef enum
{
    LIST_TYPE_UNKNOWN = 0,
    LIST_TYPE_REGION = 1,
    LIST_TYPE_REGION_VIS = 2
} ListType;

struct WorldElemListNode {
    WorldElem* elem;
    WorldElem* elem_collided;
    struct WorldElemListNode* next;
    unsigned int type:8, userarg:8;
    simple_hash_table_t* hash_ptr;
};

typedef struct WorldElemListNode WorldElemListNode;

extern volatile WorldElemListNode* world_elem_list_remove_watch_elem;

static void
world_elem_list_init(WorldElemListNode* pHeadNode)
{
    memset(pHeadNode, 0, sizeof(*pHeadNode));
}

WorldElem*
world_elem_alloc(void);

WorldElem*
world_elem_alloc_extended_model(void);

void
world_elem_adjust_geometry_pointers(WorldElem* pElem);

WorldElem*
world_elem_clone(WorldElem*);

static void
world_elem_init(WorldElem* pElem, int elem_id)
{
    pElem->elem_id = elem_id;
    pElem->stuff.towed_elem_id = WORLD_ELEM_ID_INVALID;
}

void
world_elem_free(WorldElem* pElem);

void
world_elem_replace_fix(WorldElem*);

static void
world_elem_get_coord(WorldElem* pElem, int n, model_coord_t dest[3]) {
    dest[0] = pElem->coords[n*3];
    dest[1] = pElem->coords[n*3+1];
    dest[2] = pElem->coords[n*3+2];
}

static float
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


typedef struct world_elem_btree_node_ {
    struct world_elem_btree_node_* left, *right;
    WorldElem* elem;
    float order;
} world_elem_btree_node;

#define WORLD_ELEM_BTREE_NODE_ZERO {NULL, NULL, NULL, 0};

void world_elem_btree_insert(world_elem_btree_node* root, WorldElem* elem, float order);

void world_elem_btree_walk(world_elem_btree_node* root, void ((*walk_func)(WorldElem* elem, float order)));

void world_elem_btree_destroy(world_elem_btree_node* root);

void world_elem_btree_destroy_root(world_elem_btree_node* root);

void world_elem_btree_remove(world_elem_btree_node* root, WorldElem* elem);

void world_elem_btree_remove_all(world_elem_btree_node* root, WorldElem* elem);

#endif
