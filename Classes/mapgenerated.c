//
//  mapgenerated.c
//  gl_flight
//
//  Created by Justin Brady on 6/29/19.
//

#include <stdio.h>
#include <limits.h>
#include <math.h>

#include "mapgenerated.h"
#include "models.h"
#include "world.h"
#include "textures.h"
#include "gameUtils.h"
#include "gamePlay.h"

typedef struct {
    Model model;
    int texture_id;
    float scale;
} terrain_t;

static float SCALE_MAX = 4;
static int num_asteroids = 0;
static int cube_groups = 16;

WorldElemListNode*
world_elem_list_find_nearest(WorldElemListNode* head, float A[3], float* result_distance)
{
    WorldElemListNode* cur = head->next;
    WorldElemListNode* result = NULL;
    float *dist = result_distance;
    
    *dist = -1;
    
    while(cur)
    {
        float eP[] = {cur->elem->physics.ptr->x, cur->elem->physics.ptr->y, cur->elem->physics.ptr->z};
        float d = distance(A[0], A[1], A[2], eP[0], eP[1], eP[2]);
        if(*dist < 0 || *dist < d)
        {
            *dist = d;
            result = cur;
        }
        cur = cur->next;
    }
    return result;
}

void
add_models_and_rotate(float Ao[3], float Auv[3], float Am, float R[16], Model m, int texture_id, int depth)
{
    float P[3];
    float V[3];
    int C = 4;
    int i;
    
    memcpy(P, Ao, sizeof(P));
    
    V[0] = Auv[0]*R[0] + Auv[1]*R[1] + Auv[2]*R[2] + R[3];
    V[1] = Auv[0]*R[C] + Auv[1]*R[C+1] + Auv[2]*R[C+2] + R[C+3];
    V[2] = Auv[0]*R[C*2] + Auv[1]*R[C*2+1] + Auv[2]*R[C*2+2] + R[C*2+3];
    
    for(i = 0; i < 3; i++) P[i] = Ao[i]+V[i];
    
    world_add_object(m, P[0], P[1], P[2], 0, 0, 0, 1.0, texture_id);
    
    if(depth > 0) add_models_and_rotate(P, V, Am, R, m, texture_id, depth-1);
}

void
distribute_models_spaced(
                         WorldElemListNode* listhead,
                         WorldElemListNode* listtail,
                         float region[3],
                         float diststep,
                         terrain_t t
                         )
{
    float nregion[3];
    WorldElemListNode *cur;
    float neardist;
    
    if(fabs(region[0]) >= gWorld->bound_radius-diststep ||
       region[1] < 0 ||
       fabs(region[1]) >= gWorld->bound_radius-diststep ||
       fabs(region[2]) >= gWorld->bound_radius-diststep)
    {
        return;
    }
    
    cur = listhead->next;
    
    do {
        nregion[0] = cur->elem->physics.ptr->x + rand_in_range(-1, 1)*t.scale;
        nregion[1] = cur->elem->physics.ptr->y + rand_in_range(-1, 1)*t.scale;
        nregion[2] = cur->elem->physics.ptr->z + rand_in_range(-1, 1)*t.scale;
        
        if(world_elem_list_find_nearest(listhead, nregion, &neardist) && neardist >= t.scale)
        {
            world_add_object(t.model, nregion[0], nregion[1], nregion[2], 0, 0, 0, t.scale, t.texture_id);
        
            world_elem_list_add(world_get_last_object(), listhead);
        }
        cur = cur->next;
    } while (cur != listtail);
}

void
world_build_run_program(float x, float y, float z)
{
    
    /*
    float origin[] = {0, 0, 0};
    float V[] = {1, 0, 0};
    float R[] = {
        0.70710725027, 0, -0.70710631209, 0,
        0, 1, 0, 5,
        0.70710725027, 0, 0.70710631209, 0,
        0, 0, 0, 1
    };
    
    add_models_and_rotate(origin, V, 1, R, MODEL_BUILDING3, TEXTURE_ID_BUILDING3, 5);
     */
    
    WorldElemListNode head;
    WorldElemListNode *headnext, *headlast;
    float neardist;
    
    world_elem_list_init(&head);
    
    float O[] = {0, gWorld->bound_radius/2, 0};
    
    terrain_t tCube;
    tCube.texture_id = TEXTURE_ID_FLOATING_BLOCKS;
    tCube.model = MODEL_CUBE2;
    tCube.scale = 2.0;
    
    // MARK: seed some objects
    for(int d = 0; d < cube_groups; d++)
    {
        float R = gWorld->bound_radius;
        
        world_add_object(tCube.model,
                         rand_in_range(-R, R), gWorld->bound_radius/4, rand_in_range(-R, R),
                         0, 0, 0, tCube.scale, tCube.texture_id);
        
        world_elem_list_add(world_get_last_object(), &head);
    }
    
    // MARK: iteratively grow objects "neighborhoods"
    headnext = &head;
    headlast = &head;
    while(headlast->next) headlast = headlast->next;
    for(int d = 0; d < 16; d++)
    {
        headnext = head.next;
        distribute_models_spaced(&head, headlast, O, gWorld->bound_radius / 64, tCube);
        headlast = headnext;
    }
    
    // MARK: add some buildings
    float side_bound = sin(M_PI/4)*gWorld->bound_radius;
    float B = 64;
    float Y = 2;
    int model_t[] = {MODEL_BUILDING3, MODEL_CBUILDING};
    int model_tex[] = {TEXTURE_ID_BUILDING3, TEXTURE_ID_BUILDING2};
    int model_scales[] = {SCALE_MAX, SCALE_MAX*2};
    
    for(int iSet = 0; iSet < sizeof(model_t)/sizeof(int); iSet++)
    {
        for(int x = -side_bound; x < side_bound; x += B)
        {
            for(int z = -side_bound; z < side_bound; z += B)
            {
                float C[] = {x + rand_in_range(0, B), Y, z + rand_in_range(0, B)};
                
                if(world_elem_list_find_nearest(&head, C, &neardist) != NULL && neardist >= SCALE_MAX)
                {
                    world_add_object(model_t[iSet], C[0], C[1], C[2], M_PI/2, rand_in_range(-M_PI, M_PI), -M_PI/2, model_scales[iSet], model_tex[iSet]);
                    world_elem_list_add(world_get_last_object(), &head);
                }
            }
        }
    }
    
    // MARK: add some asteroids
    float asteroid_scale = SCALE_MAX;
    float asteroid_speed = (MAX_SPEED/10)*GAME_TICK_RATE;
    for(int i = 0; i < num_asteroids; i++)
    {
        float C[] = {
            rand_in_range(-gWorld->bound_radius, gWorld->bound_radius),
            rand_in_range(1, gWorld->bound_radius),
            rand_in_range(-gWorld->bound_radius, gWorld->bound_radius)
        };
        
        if(world_elem_list_find_nearest(&head, C, &neardist) != NULL && neardist >= asteroid_scale)
        {
            world_add_object(MODEL_ICOSAHEDRON, C[0], C[1], C[2], 0, 0, 0, asteroid_scale, TEXTURE_ID_ASTEROID);
            world_get_last_object()->object_type = OBJ_BLOCK_MOVING;
            update_object_velocity(world_get_last_object()->elem_id,
                                   rand_in_range(-asteroid_speed, asteroid_speed), rand_in_range(-asteroid_speed, asteroid_speed), rand_in_range(-asteroid_speed, asteroid_speed), 0);
        }
    }
    
    world_elem_list_clear(&head);
    
    // MARK: add enemy base
    
    if(gameStateSinglePlayer.game_type == GAME_TYPE_COLLECT)
    {
        /*
         #define BASE_ENEMY_1                           \
         "add_object " MAPMODEL_ENEMYBASE " rndx 50 rndz 0 0 0 4 " BASE_TEXTURE_ID_ENEMY "\n"       \
         "object_set_info 15\n"
         */
        world_add_object(MODEL_ENEMY_BASE,
                         rand_in_range(-gWorld->bound_radius, gWorld->bound_radius),
                         gWorld->bound_radius/4,
                         rand_in_range(-gWorld->bound_radius, gWorld->bound_radius),
                         0.01, 0.01, 0.01,
                         8,
                         TEXTURE_ID_ENEMY_SPAWNPOINT);
        world_get_last_object()->object_type = OBJ_SPAWNPOINT_ENEMY;
        update_object_velocity(world_get_last_object()->elem_id, 0, 0, 0, 0);
        game_elem_setup_spawnpoint_enemy(world_get_last_object());
        
        // MARK: -- add friendly core (hemmoraging collect nodes)
        
        world_add_object(MODEL_SPHERE,
                         0, gWorld->bound_radius/4, 0,
                         0.01, 0.01, 0.01,
                         8, TEXTURE_ID_ANIMATED_STATIC);
        world_get_last_object()->object_type = OBJ_BASE;
        update_object_velocity(world_get_last_object()->elem_id, 0, 0, 0, 0);
        game_elem_setup_spawnpoint(world_get_last_object());
        
        // MARK: -- add friendly spawn location
        world_add_object(MODEL_CUBE,
                         gWorld->bound_radius/2, gWorld->bound_radius/4, gWorld->bound_radius/2,
                         0, M_PI*1.5, M_PI,
                         6, TEXTURE_ID_ANIMATED_STATIC);
        world_get_last_object()->object_type = OBJ_SPAWNPOINT;
        update_object_velocity(world_get_last_object()->elem_id, 0, 0, 0, 0);
        world_object_set_lifetime(world_get_last_object()->elem_id, 300);
    }
    
}
