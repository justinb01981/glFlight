//
//  collision.h
//  gl_flight
//
//  Created by Justin Brady on 4/3/13.
//
//

#ifndef gl_flight_collision_h
#define gl_flight_collision_h

#include <math.h>

#include "object.h"
#include "gamePlay.h"
#include "action.h"
#include "textures.h"
#include "gameAI.h"
#include "gameDialogs.h"

enum
{
    COLLISION_ACTION_NONE = 0,
    COLLISION_ACTION_DAMAGE = 1,
    COLLISION_ACTION_REPULSE = 2,
    COLLISION_ACTION_FLAG = 3,
    COLLISION_ACTION_PORTAL_TELEPORT = 4,
    COLLISION_ACTION_POWERUP_GRAB = 5,
    COLLISION_ACTION_POWERUP_CAPTURE = 6,
    COLLISION_ACTION_NEXTLEVEL = 7,
    COLLISION_ACTION_REFLECT = 8
};

typedef unsigned int collision_action_t;

typedef collision_action_t collision_action_table_t[OBJ_LAST][OBJ_LAST];

const static collision_action_t
collision_actions_default[OBJ_LAST][OBJ_LAST] =
{
    {0, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0,       2, 0, 0, 0, 0, 0, 2, 0, 0}, // unknown
    {2, 2, 2, 0, 2, 2, 1, 0, 0, 0, 0,       5, 1, 1, 1, 0, 0, 1, 0, 0}, // ship
    {2, 2, 2, 3, 2, 2, 1, 4, 7, 0, 0, /*3*/ 5, 1, 1, 1, 0, 0, 0, 0, 0}, // player
    {0, 0, 2, 2, 0, 2, 1, 0, 0, 0, 0,       0, 0, 1, 0, 0, 0, 0, 0, 0}, // turret
    {0, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0,       0, 0, 1, 0, 0, 0, 0, 0, 0}, // block_moving
    {0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0,       0, 0, 1, 0, 0, 0, 0, 0, 0}, // block
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,       0, 1, 0, 0, 0, 0, 0, 0, 0}, // bullet
    {0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // portal
    {0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // spawnpoint
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // poopedcube
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // wreckage
    {2, 5, 5, 0, 2, 2, 0, 0, 3, 0, 0,       0, 0, 0, 0, 6, 0, 0, 0, 0}, // powerup generic
    {0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // capture
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // base
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,       0, 1, 0, 0, 0, 0, 0, 0, 0}, // missle
    {0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0,       6, 0, 0, 0, 0, 0, 0, 0, 0}, // spawnpoint enemy
    {0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // DISPLAYONLY
    {2, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // BALL
    {0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // reserved
    {0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // reserved
};

extern collision_action_t collision_actions[OBJ_LAST][OBJ_LAST];
extern collision_action_t collision_actions_stage1[OBJ_LAST][OBJ_LAST];

extern void load_map_and_host_game();

// TODO: make this an array of callback-lists
extern collision_action_t collision_actions[OBJ_LAST][OBJ_LAST];

extern char collidedPortalNametagLast[256];

static void
collision_actions_set_default()
{
    int r, c;
    for(r = 0; r < OBJ_LAST; r++)
        for(c = 0; c < OBJ_LAST; c++)
        {
            collision_actions[r][c] = collision_actions_default[r][c];
            collision_actions[c][r] = collision_actions_default[c][r];
        }
    
    // check that both actions agree
    for(r = 0; r < OBJ_LAST; r++)
        for(c = 0; c < OBJ_LAST; c++)
        {
            DBPRINTF(("Warning: collision_actions_default row/column mismatch: r=%d c=%d\n", r, c));
        }
}

static void
collision_actions_set_grab_powerup()
{
    collision_actions[OBJ_PLAYER][OBJ_POWERUP_GENERIC] = COLLISION_ACTION_POWERUP_GRAB;
    collision_actions[OBJ_POWERUP_GENERIC][OBJ_PLAYER] = COLLISION_ACTION_POWERUP_GRAB;
}

static void
collision_actions_set_ship_grab_powerup()
{
    collision_actions[OBJ_SHIP][OBJ_POWERUP_GENERIC] = COLLISION_ACTION_POWERUP_GRAB;
    collision_actions[OBJ_POWERUP_GENERIC][OBJ_SHIP] = COLLISION_ACTION_POWERUP_GRAB;
}

static void
collision_actions_set_player_invuln()
{
    collision_actions[OBJ_PLAYER][OBJ_BULLET] = COLLISION_ACTION_NONE;
    collision_actions[OBJ_BULLET][OBJ_PLAYER] = COLLISION_ACTION_NONE;
}

static void
collision_actions_set_player_vuln()
{
    collision_actions[OBJ_PLAYER][OBJ_BULLET] = COLLISION_ACTION_DAMAGE;
    collision_actions[OBJ_BULLET][OBJ_PLAYER] = COLLISION_ACTION_DAMAGE;
}

static void collision_handling_remove_hook(WorldElem* pElem)
{
    if(pElem == gWorld->world_update_state.collision_recs_iterate_cur->elem)
    {
        gWorld->world_update_state.collision_recs_iterate_cur = gWorld->world_update_state.collision_recs_iterate_cur->next;
    }
}

static void
do_world_collision_handling(float tc)
{
    // collisions
    //WorldElemListNode* pCollisionCur = gWorld->elements_collided.next;
    WorldElemListNode* pCollisionTmp;
    world_update_state_t *state = &gWorld->world_update_state;
    
    state->collision_recs_iterate_cur = gWorld->elements_collided.next;
    
    gWorld->world_update_state.world_remove_hook = collision_handling_remove_hook;
    
    while(state->collision_recs_iterate_cur && state->collision_recs_iterate_cur->next)
    {
        // pairs of objects
        // elements_collided list does not add duplicates
        // element-moved at time of collision added first
        // TODO: if both elements are moving... who knows?
        WorldElemListNode* pCollisionA = state->collision_recs_iterate_cur;
        WorldElemListNode* pCollisionB =  state->collision_recs_iterate_cur->next;

        // a,b ordered by object-id ascending
        if(pCollisionA->elem->object_type > pCollisionB->elem->object_type) {
            WorldElem *tmp = pCollisionB->elem;
            pCollisionB->elem = pCollisionA->elem;
            pCollisionA->elem = tmp;
        }
        
        if(pCollisionB)
        {
            // action recorded by world-boundary-checks
            collision_action_t world_coll_act = pCollisionA->userarg;
            
            // always trigger collision logic in certain cases (map building)
            if(pCollisionA->elem->object_type == OBJ_BULLET)
            {
                if(pCollisionA->elem->stuff.bullet.action == ACTION_REPLACE_OBJECT)
                {
                    world_coll_act = COLLISION_ACTION_DAMAGE;
                }
            }
            
            switch(world_coll_act)
            {
            // JB: this should now be handled in world_update()
            case COLLISION_ACTION_REPULSE:
                /*
                game_handle_collision(pCollisionA->elem, pCollisionB->elem, world_coll_act);
                gameNetwork_handle_collision(pCollisionA->elem, pCollisionB->elem, world_coll_act);
                game_ai_collision(pCollisionA->elem, pCollisionB->elem, world_coll_act);
                */
                break;
    
            case COLLISION_ACTION_NONE:
                break;
                    
            case COLLISION_ACTION_FLAG:
                pCollisionA->elem->stuff.flags.mask |= pCollisionB->elem->stuff.flags.mask;
                // continue to normal damage handling
                    
            case COLLISION_ACTION_POWERUP_CAPTURE:
                    
            case COLLISION_ACTION_POWERUP_GRAB:
                    
            case COLLISION_ACTION_DAMAGE:
                // bullet placing object?
                if(pCollisionA->elem->type == MODEL_BULLET
                   /*
                   && pCollisionA->elem->stuff.bullet.action >= ACTION_PLACE_BLOCK
                   && pCollisionA->elem->stuff.bullet.action <= ACTION_PLACE_TURRET
                    */
                   && pCollisionA->elem->stuff.bullet.action == ACTION_SHOOT_BLOCK)
                {
                    world_elem_list_add(pCollisionA->elem, &gWorld->elements_to_be_freed);
                    
                    /*
                    if(pCollisionA->elem->in_visible_list)
                    {
                        visible_list_remove(pCollisionA->elem, &visibleElementsLen);
                    }
                     */
                    
                    float newBlock[3];
                    float block_size = pCollisionB->elem->scale;
                    float k[3][6];
                    float v[3];
                    float dist;
                    
                    k[0][0] = 1; k[1][0] = 0; k[2][0] = 0;
                    k[0][1] = -1; k[1][1] = 0; k[2][1] = 0;
                    k[0][2] = 0; k[1][2] = 1; k[2][2] = 0;
                    k[0][3] = 0; k[1][3] = -1; k[2][3] = 0;
                    k[0][4] = 0; k[1][4] = 0; k[2][4] = 1;
                    k[0][5] = 0; k[1][5] = 0; k[2][5] = -1;
                    
                    v[0] = pCollisionB->elem->physics.ptr->x - pCollisionA->elem->physics.ptr->x;
                    v[1] = pCollisionB->elem->physics.ptr->y - pCollisionA->elem->physics.ptr->y;
                    v[2] = pCollisionB->elem->physics.ptr->z - pCollisionA->elem->physics.ptr->z;
                    
                    dist = distance(pCollisionB->elem->physics.ptr->x,
                                    pCollisionB->elem->physics.ptr->y,
                                    pCollisionB->elem->physics.ptr->z,
                                    pCollisionA->elem->physics.ptr->x,
                                    pCollisionA->elem->physics.ptr->y,
                                    pCollisionA->elem->physics.ptr->z);
                    int m = 0;
                    float md = 0;
                    for(int i = 0; i < 6; i++)
                    {
                        float d = (k[0][i]*v[0] + k[1][i]*v[1] + k[2][i]*v[2]) / dist;
                        if(md == 0 || d < md)
                        {
                            m = i;
                            md = d;
                        }
                    }
                    
                    newBlock[0] = pCollisionB->elem->physics.ptr->x + k[0][m]*block_size;
                    newBlock[1] = pCollisionB->elem->physics.ptr->y + k[1][m]*block_size;
                    newBlock[2] = pCollisionB->elem->physics.ptr->z + k[2][m]*block_size;
                    
                    int obj_id;
                    
                    if(pCollisionA->elem->stuff.bullet.action == ACTION_SHOOT_BLOCK)
                    {
                        int model = pCollisionB->elem->type;
                        int tex_id = pCollisionB->elem->texture_id;
                        
                        obj_id =
                        world_add_object(model, newBlock[0], newBlock[1], newBlock[2],
                                         0, 0, 0, pCollisionB->elem->scale, tex_id);
                        
                        world_get_last_object()->object_type = OBJ_BLOCK;
                        world_get_last_object()->stuff.affiliation =
                            pCollisionA->elem->stuff.affiliation;
                    }
                    else if(pCollisionA->elem->stuff.bullet.action == ACTION_PLACE_TURRET)
                    {
                        obj_id =
                        world_add_object(MODEL_PYRAMID, newBlock[0], newBlock[1], newBlock[2],
                                         0, 0, 0, 1, TEXTURE_ID_TURRET);
                        
                        update_object_velocity(obj_id, 0, 0, 0, 0);
                        world_get_last_object()->object_type = OBJ_TURRET;
                        world_get_last_object()->stuff.affiliation =
                            pCollisionA->elem->stuff.affiliation;
                    }
                    else if(pCollisionA->elem->stuff.bullet.action == ACTION_PLACE_SHIP)
                    {
                        obj_id =
                        world_add_object(MODEL_SHIP1, newBlock[0], newBlock[1], newBlock[2],
                                         0, 0, 0, 1, TEXTURE_ID_ENEMYSHIP);
                        
                        update_object_velocity(obj_id, 0, 0, 0, 0);
                        world_get_last_object()->stuff.u.enemy.target_id = WORLD_ELEM_ID_INVALID;
                        world_get_last_object()->stuff.u.enemy.intelligence = gameStateSinglePlayer.enemy_intelligence;
                        world_get_last_object()->stuff.u.enemy.enemy_state = ENEMY_STATE_PATROL;
                        world_get_last_object()->durability = gameStateSinglePlayer.enemy_durability;
                        world_get_last_object()->object_type = OBJ_SHIP;
                    }
                }
                else if(pCollisionA->elem->stuff.bullet.action == ACTION_BLOCK_TEXTURE)
                {
                    pCollisionB->elem->texture_id++;
                    if(pCollisionB->elem->texture_id >= n_textures) pCollisionB->elem->texture_id = 0;
                    
                    if(!world_elem_list_find(pCollisionA->elem->elem_id, &gWorld->elements_to_be_freed))
                    {
                        world_elem_list_add(pCollisionA->elem, &gWorld->elements_to_be_freed);
                    }
                }
                else if(pCollisionA->elem->stuff.bullet.action == ACTION_SCALE_OBJECT)
                {
                    float scale_new = pCollisionB->elem->scale * 2;
                    if(scale_new >= 16) scale_new = 0.5;
                    
                    int obj_id = world_add_object(pCollisionB->elem->type,
                                                  pCollisionB->elem->physics.ptr->x,
                                                  pCollisionB->elem->physics.ptr->y,
                                                  pCollisionB->elem->physics.ptr->z,
                                                  pCollisionB->elem->physics.ptr->alpha,
                                                  pCollisionB->elem->physics.ptr->beta,
                                                  pCollisionB->elem->physics.ptr->gamma,
                                                  scale_new,
                                                  pCollisionB->elem->texture_id);
                    
                    world_get_last_object()->object_type = pCollisionB->elem->object_type;
                    
                    if(!world_elem_list_find(pCollisionB->elem->elem_id, &gWorld->elements_to_be_freed))
                    {
                        world_elem_list_add(pCollisionB->elem, &gWorld->elements_to_be_freed);
                    }
                    
                    if(!world_elem_list_find(pCollisionA->elem->elem_id, &gWorld->elements_to_be_freed))
                    {
                        world_elem_list_add(pCollisionA->elem, &gWorld->elements_to_be_freed);
                    }
                }
                else if(pCollisionA->elem->stuff.bullet.action == ACTION_REPLACE_OBJECT)
                {
                    static int replace_object_last = 0;
                    
                    int replace_object_table[][3] = {
                        {MODEL_CUBE2, OBJ_BLOCK, TEXTURE_ID_BLOCK},
                        {MODEL_CUBE, OBJ_BLOCK, TEXTURE_ID_BLOCK},
                        {MODEL_TURRET, OBJ_TURRET, TEXTURE_ID_TURRET},
                        {MODEL_SPRITE, OBJ_POWERUP_GENERIC, TEXTURE_ID_POWERUP},
                        {MODEL_CUBE2, OBJ_SPAWNPOINT, TEXTURE_ID_SPAWNPOINT},
                        {MODEL_CUBE2, OBJ_SPAWNPOINT_ENEMY, TEXTURE_ID_ENEMY_SPAWNPOINT},
                        {MODEL_SHIP2, OBJ_SHIP, TEXTURE_ID_ENEMYSHIP},
                        {MODEL_SHIP3, OBJ_SHIP, TEXTURE_ID_ENEMYSHIP_ACE}
                    };
                    
                    int r = replace_object_last+1;
                    if(r >= (sizeof(replace_object_table) / sizeof(replace_object_table[0]))) r = 0;
                       
                    int replace_obj_model = replace_object_table[r][0];
                    int replace_obj_type = replace_object_table[r][1];
                    int replace_tex_id = replace_object_table[r][2];
                    float replace_obj_scale = pCollisionB->elem->scale;
                    replace_object_last = r;
                    
                    int obj_id =
                    world_add_object(replace_obj_model,
                                     pCollisionB->elem->physics.ptr->x,
                                     pCollisionB->elem->physics.ptr->y,
                                     pCollisionB->elem->physics.ptr->z,
                                     0, 0, 0,
                                     replace_obj_scale,
                                     replace_tex_id);
                    world_get_last_object()->object_type = replace_obj_type;
                    update_object_velocity(obj_id, 0, 0, 0, 0);
                    
                    if(!world_elem_list_find(pCollisionB->elem->elem_id, &gWorld->elements_to_be_freed))
                    {
                        world_elem_list_add(pCollisionB->elem, &gWorld->elements_to_be_freed);
                    }
                    
                    if(!world_elem_list_find(pCollisionA->elem->elem_id, &gWorld->elements_to_be_freed))
                    {
                        world_elem_list_add(pCollisionA->elem, &gWorld->elements_to_be_freed);
                    }
                }
                else
                {
                    int object_destroyed = 0;
                    int object_damage = world_coll_act == COLLISION_ACTION_DAMAGE;
                    int durability_a = pCollisionA->elem->durability;
                    int durability_b = pCollisionB->elem->durability;
                    
                    // HACK: expect A to point at faster object (missle/bullet)
//                    if(pCollisionA->elem->physics.ptr->velocity < pCollisionB->elem->physics.ptr->velocity)
//                    {
//                        WorldElemListNode* tmp = pCollisionA;
//                        pCollisionA = pCollisionB;
//                        pCollisionB = tmp;
//                    }
                    
                    if(object_damage)
                    {
                        if(/*pCollisionA->elem->object_type == OBJ_BULLET &&*/
                            pCollisionA->elem->physics.ptr->velocity > 0 && 
                            pCollisionB->elem->physics.ptr->velocity > 0)
                        {
                            float bullet_vtransfer = 0.2 * pCollisionA->elem->durability;

                            update_object_velocity(pCollisionB->elem->elem_id,
                                                   pCollisionA->elem->physics.ptr->vx*bullet_vtransfer,
                                                   pCollisionA->elem->physics.ptr->vy*bullet_vtransfer,
                                                   pCollisionA->elem->physics.ptr->vz*bullet_vtransfer,
                                                   1);
                        }
                    }
                    
                    game_handle_collision(pCollisionA->elem, pCollisionB->elem, world_coll_act);
                    gameNetwork_handle_collision(pCollisionA->elem, pCollisionB->elem, world_coll_act);
                    game_ai_collision(pCollisionA->elem, pCollisionB->elem, world_coll_act);
                    
                    if(object_damage)
                    {
                        float durability_tmp = pCollisionA->elem->durability;

                        if(pCollisionB->elem->object_type == OBJ_SHIP
                           && pCollisionA->elem->object_type == OBJ_POWERUP_GENERIC)
                        {
                            assert(0);
                        }
                        
                        pCollisionA->elem->durability -= pCollisionB->elem->durability;
                        pCollisionB->elem->durability -= durability_tmp;
                        
                        // moving object destroyed
                        if(pCollisionA->elem->durability <= 0)
                        {
                            game_handle_destruction(pCollisionA->elem);
                            gameNetwork_handle_destruction(pCollisionA->elem);
                            
                            if(!world_elem_list_find(pCollisionA->elem->elem_id, &gWorld->elements_to_be_freed))
                            {
                                world_elem_list_add(pCollisionA->elem, &gWorld->elements_to_be_freed);
                            }
                            
                            if(durability_a > 0)
                            {
                                int obj_id =
                                world_add_object(MODEL_ICOSAHEDRON,
                                                 pCollisionA->elem->physics.ptr->x + (pCollisionA->elem->physics.ptr->vx * -tc),
                                                 pCollisionA->elem->physics.ptr->y + (pCollisionA->elem->physics.ptr->vy * -tc),
                                                 pCollisionA->elem->physics.ptr->z + (pCollisionA->elem->physics.ptr->vz * -tc),
                                                 pCollisionA->elem->physics.ptr->alpha,
                                                 pCollisionA->elem->physics.ptr->beta,
                                                 pCollisionA->elem->physics.ptr->gamma,
                                                 pCollisionA->elem->scale, TEXTURE_ID_EXPLOSION);
                                world_get_last_object()->object_type = OBJ_BLOCK;
                                
                                world_get_last_object()->destructible = 0;
                                world_object_set_lifetime(obj_id, 15);
                                update_object_velocity(obj_id, 0, 0, 0, 0);
                                object_destroyed = 1;
                            }
                        }
                        
                        // "static" object destroyed
                        if(pCollisionB->elem->durability <= 0)
                        {
                            game_handle_destruction(pCollisionB->elem);
                            gameNetwork_handle_destruction(pCollisionB->elem);
                            
                            if(!world_elem_list_find(pCollisionB->elem->elem_id, &gWorld->elements_to_be_freed))
                            {
                                world_elem_list_add(pCollisionB->elem, &gWorld->elements_to_be_freed);
                            }
                            
                            if(durability_b > 0)
                            {
                                int obj_id = 
                                world_add_object(MODEL_ICOSAHEDRON,
                                                 pCollisionB->elem->physics.ptr->x + (pCollisionB->elem->physics.ptr->vx * -tc),
                                                 pCollisionB->elem->physics.ptr->y + (pCollisionB->elem->physics.ptr->vy * -tc),
                                                 pCollisionB->elem->physics.ptr->z + (pCollisionB->elem->physics.ptr->vz * -tc),
                                                 pCollisionB->elem->physics.ptr->alpha,
                                                 pCollisionB->elem->physics.ptr->beta,
                                                 pCollisionB->elem->physics.ptr->gamma,
                                                 pCollisionB->elem->scale, TEXTURE_ID_EXPLOSION);
                                world_get_last_object()->object_type = OBJ_BLOCK;
                                
                                world_get_last_object()->destructible = 0;
                                world_object_set_lifetime(obj_id, 15);
                                update_object_velocity(obj_id, 0, 0, 0, 0);
                                object_destroyed = 1;
                            }
                            
                            // handle spawned-objects (bullets)
                            if(pCollisionB->elem->type == MODEL_SHIP1)
                            {
                                gameAudioPlaySoundAtLocation("dead",
                                                             pCollisionB->elem->physics.ptr->x,
                                                             pCollisionB->elem->physics.ptr->y,
                                                             pCollisionB->elem->physics.ptr->z);
                                
                                if(pCollisionB->elem->elem_id == my_ship_id)
                                {
                                    extern int camera_locked_frames;
                                    camera_locked_frames = 240;
                                    
                                    /*
                                    gameAudioPlaySoundAtLocation("dead",
                                                                 my_ship_x,
                                                                 my_ship_y,
                                                                 my_ship_z);
                                    */
                                    
                                    // add wreckage
                                    world_add_object(MODEL_SPRITE, my_ship_x, my_ship_y, my_ship_z,
                                                     0, 0, 0, 1, TEXTURE_ID_WRECKAGE);
                                    world_get_last_object()->destructible = 0;
                                    world_get_last_object()->object_type = OBJ_WRECKAGE;
                                }
                            }
                        }
                        
                        if(object_destroyed)
                        {
                            gameAudioPlaySoundAtLocation("boom",
                                                         pCollisionA->elem->physics.ptr->x,
                                                         pCollisionA->elem->physics.ptr->y,
                                                         pCollisionA->elem->physics.ptr->z);
                        }
                    }
                }
                break;
                    
            case COLLISION_ACTION_PORTAL_TELEPORT:
                {
                    char* nametag = pCollisionA->elem->stuff.nametag;

                    console_clear();
                    gameNetwork_disconnectSignal();
                    if(nametag != NULL) {
                        if(strcmp(nametag, GAME_NETWORK_HOST_PORTAL_NAME) == 0)
                        {
                            gameNetwork_host(gameSettingGameTitle, load_map_and_host_game);
                        }
                        else
                        {
                            gameNetwork_connect(nametag, NULL);
                        }
                    }
                    world_remove_object(pCollisionA->elem->elem_id);
                }
                break;
            }
        }
        else
        {
            break;
        }
        
    ignore_collision:
        if(state->collision_recs_iterate_cur)
        {
            state->collision_recs_iterate_cur = state->collision_recs_iterate_cur->next;
        }
    }
    
    gWorld->world_update_state.world_remove_hook = NULL;
    
    /* HACK: remove elements marked for removal during collison handling
     * TODO: check if added to pending-free list at post-handling of above loop
     */
    state->collision_recs_iterate_cur = gWorld->elements_collided.next;
    while(state->collision_recs_iterate_cur)
    {
        WorldElemListNode* remove = state->collision_recs_iterate_cur;
        state->collision_recs_iterate_cur = state->collision_recs_iterate_cur->next;
        if(remove->elem->collision_handle_remove)
        {
            assert(0);
            
            //world_elem_list_add(remove->elem, &gWorld->elements_to_be_freed);
        }
    }
}

typedef int (*collision_callback_t)(WorldElem* elemA, WorldElem* elemB);

struct collision_callback_list
{
    struct collision_callback_list* next;
    collision_callback_t cb;
};

static struct collision_callback_list collision_callbacks[OBJ_LAST][OBJ_LAST] =
{
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static void
add_collision_callback(int obj_type_a, int obj_type_b, collision_callback_t cb)
{
    struct collision_callback_list* cur = &collision_callbacks[obj_type_a][obj_type_b];
    
    while(cur->next)
    {
        cur = cur->next;
    }
    
    cur->next = malloc(sizeof(struct collision_callback_list));
    if(cur->next)
    {
        cur->next->cb = cb;
        cur->next->next = NULL;
    }
}

static void
collision_init()
{
}

#endif
