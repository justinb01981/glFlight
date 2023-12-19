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
    COLLISION_ACTION_POWERUP_GRAB_OR_TOW = 5,
    COLLISION_ACTION_POWERUP_CAPTURE = 6,
    COLLISION_ACTION_NEXTLEVEL = 7,
    COLLISION_ACTION_REFLECT = 8
};

typedef unsigned int collision_action_t;

typedef collision_action_t collision_action_table_t[OBJ_LAST][OBJ_LAST];

const static int TURRETXPOWERUP = COLLISION_ACTION_REPULSE;
const static int SHIPXPOWERUP = COLLISION_ACTION_POWERUP_GRAB_OR_TOW;
const static int PLYRXPOWERUP = COLLISION_ACTION_POWERUP_GRAB_OR_TOW;
const static int SPAWNXCAPT = COLLISION_ACTION_POWERUP_CAPTURE;

// THIS MUST MATCH OBJ ENUM ORDINALITY
const static collision_action_t
collision_actions_default[OBJ_LAST][OBJ_LAST] =
{
    // TODO: (generic blocks treated equivalent to OBJ_BLOCK?)
    {0, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0,       2, 0, 0, 0, 0, 0, 2, 0, 0}, // unknown
    {2, 2, 2, 0, 2, 2, 1, 0, 0, 0, SHIPXPOWERUP,       0, 1, 1, 1, 0, 0, 1, 0, 0}, // ship
    {2, 2, 2, 3, 2, 2, 1, COLLISION_ACTION_PORTAL_TELEPORT, 7, 0, PLYRXPOWERUP, /*3*/ 0, 1, 1, 1, 0, 0, 0, 0, 0}, // player
    {0, 0, 2, 2, 0, 2, 1, 0, 0, 0, TURRETXPOWERUP,       0, 0, 1, 0, 0, 0, 0, 0, 0}, // turret
    {0, 2, 2, 0, 0, 0, 1, 0, 0, 0, 0,       0, 0, 1, 0, 0, 0, 0, 0, 0}, // block_moving (4)
    {0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0,       0, 0, 1, 0, 0, 0, 0, 0, 0}, // block
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,       0, 1, 0, 0, 0, 0, 0, 0, 0}, // bullet
    {0, 0, COLLISION_ACTION_PORTAL_TELEPORT, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // portal

    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // poopedcube
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // wreckage

    {2, SHIPXPOWERUP, PLYRXPOWERUP, TURRETXPOWERUP, 0, 2, 0, 0, 3, 0, 0,       0, 0, 0, 0, COLLISION_ACTION_POWERUP_CAPTURE, 0, 0, 0, 0}, // powerup generic (10)
    {0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // capture
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // base
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,       0, 1, 0, 0, 0, 0, 0, 0, 0}, // missle
    {0, 0, 7, 0, 0, 0, 0, 0, 0, 0, SPAWNXCAPT,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // spawnpoint
    {0, 0, 3, 0, 0, 0, 0, 0, 3, 0, SPAWNXCAPT,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // spawnpoint enemy
    {0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // DISPLAYONLY
    {2, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // BALL
    {0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // reserved
    {0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // reserved
};

extern collision_action_t collision_actions[OBJ_LAST][OBJ_LAST];
extern collision_action_t collision_actions_stage1[OBJ_LAST][OBJ_LAST];

extern void load_map_and_host_game(void);

// TODO: make this an array of callback-lists
extern collision_action_t collision_actions[OBJ_LAST][OBJ_LAST];

extern char collidedPortalNametagLast[256];

static void
collision_actions_set_default()
{
    // TODO: remove this and make collision table static
    memcpy(collision_actions, collision_actions_default, sizeof(collision_actions));

//    int r, c;
//    for(r = 0; r < OBJ_LAST; r++)
//        for(c = 0; c < OBJ_LAST; c++)
//        {
//            collision_actions[r][c] = collision_actions_default[r][c];
//            collision_actions[c][r] = collision_actions_default[c][r];
//        }
//
//    // check that both actions agree
//    for(r = 0; r < OBJ_LAST; r++)
//        for(c = 0; c < OBJ_LAST; c++)
//        {
//            DBPRINTF(("Warning: collision_actions_default row/column mismatch: r=%d c=%d\n", r, c));
//        }
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

    world_update_state_t *state = &gWorld->world_update_state;
    char collisionStr[255] = {0}, *collisionStrPfx = "collision omeboi:";

    // BEgin walking recorded collisions
    state->collision_recs_iterate_cur = gWorld->elements_collided.next;

    // IDEA: set a hook for objects which get removed during the course of collision-handling as a half-ass step to collision-callbacks
    gWorld->world_update_state.world_remove_hook = collision_handling_remove_hook;

    strcat(collisionStr, collisionStrPfx);

    while(state->collision_recs_iterate_cur && state->collision_recs_iterate_cur->next)
    {
        // pairs of objects
        // elements_collided list does not add objects to multiple pairs so consider that
        // element-moved at time of collision added first

        WorldElemListNode* pCollisionA = state->collision_recs_iterate_cur;
        WorldElemListNode* pCollisionB = world_elem_list_find_elem(pCollisionA->elem_collided, &gWorld->elements_collided);

        char* pStrDst = collisionStr + strlen(collisionStr);
        sprintf(pStrDst, " (%ld) (%ld)", (long)pCollisionA->elem->object_type,
               (long) pCollisionB->elem->object_type);


        
        if(pCollisionB)
        {

            // a,b priority determined by object-id ascending in world.c - e.g. pCollisionB will point at the OBJ_BULLET, pCollisionA at the OBJ_SHIP
            if(pCollisionA->elem->object_type > pCollisionB->elem->object_type)
            {
                WorldElemListNode *tmp = pCollisionA;
                pCollisionA = pCollisionB;
                pCollisionB = tmp;
            }


            // action recorded by world-boundary-checks
            //collision_action_t world_coll_act = pCollisionA->userarg;

            //assert(world_coll_act == collision_actions[pCollisionA->elem->object_type][pCollisionB->elem->object_type]);
            collision_action_t world_coll_act = collision_actions[pCollisionA->elem->object_type][pCollisionB->elem->object_type];
            
            // HACK: always trigger collision logic in certain cases (map building)
            if(pCollisionB->elem->object_type == OBJ_BULLET)
            {
                if(pCollisionB->elem->stuff.bullet.action == ACTION_REPLACE_OBJECT)
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
                    
            case COLLISION_ACTION_POWERUP_GRAB_OR_TOW:
                    
            case COLLISION_ACTION_DAMAGE:
                // bullet placing object?
                if(pCollisionB->elem->type == MODEL_BULLET
                   /*
                   && pCollisionA->elem->stuff.bullet.action >= ACTION_PLACE_BLOCK
                   && pCollisionA->elem->stuff.bullet.action <= ACTION_PLACE_TURRET
                    */
                   && pCollisionB->elem->stuff.bullet.action == ACTION_SHOOT_BLOCK)
                {
                    DBPRINTF(("THIS HAS NOT BEEN TESTED SINCE CHANGING COLLISION ORDINALITY! REMOVE THIS LINE BEFORE PROCEEDING"));
                    assert(0);

                    world_elem_list_add(pCollisionB->elem, &gWorld->elements_to_be_freed);
                    
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
                    
                    if(pCollisionB->elem->stuff.bullet.action == ACTION_SHOOT_BLOCK)
                    {
                        int model = pCollisionB->elem->type;
                        int tex_id = pCollisionB->elem->texture_id;
                        
                        obj_id =
                        world_add_object(model, newBlock[0], newBlock[1], newBlock[2],
                                         0, 0, 0, pCollisionB->elem->scale, tex_id);
                        
                        world_get_last_object()->object_type = OBJ_BLOCK;
                        world_get_last_object()->stuff.affiliation =
                            pCollisionB->elem->stuff.affiliation;
                    }
                    else if(pCollisionB->elem->stuff.bullet.action == ACTION_PLACE_TURRET)
                    {
                        obj_id =
                        world_add_object(MODEL_PYRAMID, newBlock[0], newBlock[1], newBlock[2],
                                         0, 0, 0, 1, TEXTURE_ID_TURRET);
                        
                        update_object_velocity(obj_id, 0, 0, 0, 0);
                        world_get_last_object()->object_type = OBJ_TURRET;
                        world_get_last_object()->stuff.affiliation =
                            pCollisionB->elem->stuff.affiliation;
                    }
                    else if(pCollisionB->elem->stuff.bullet.action == ACTION_PLACE_SHIP)
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
                else if(pCollisionB->elem->stuff.bullet.action == ACTION_BLOCK_TEXTURE)
                {
                    pCollisionA->elem->texture_id++;
                    
                    if(!world_elem_list_find(pCollisionB->elem->elem_id, &gWorld->elements_to_be_freed))
                    {
                        world_elem_list_add(pCollisionB->elem, &gWorld->elements_to_be_freed);
                    }
                }
                else if(pCollisionB->elem->stuff.bullet.action == ACTION_SCALE_OBJECT)
                {
                    float scale_new = pCollisionB->elem->scale * 2;
                    if(scale_new >= 16) scale_new = 0.5;
                    
                    int obj_id = world_add_object(pCollisionA->elem->type,
                                                  pCollisionA->elem->physics.ptr->x,
                                                  pCollisionA->elem->physics.ptr->y,
                                                  pCollisionA->elem->physics.ptr->z,
                                                  pCollisionA->elem->physics.ptr->alpha,
                                                  pCollisionA->elem->physics.ptr->beta,
                                                  pCollisionA->elem->physics.ptr->gamma,
                                                  scale_new,
                                                  pCollisionA->elem->texture_id);
                    
                    world_get_last_object()->object_type = pCollisionA->elem->object_type;
                    
                    if(!world_elem_list_find(pCollisionA->elem->elem_id, &gWorld->elements_to_be_freed))
                    {
                        world_elem_list_add(pCollisionA->elem, &gWorld->elements_to_be_freed);
                    }
                    
                    if(!world_elem_list_find(pCollisionB->elem->elem_id, &gWorld->elements_to_be_freed))
                    {
                        world_elem_list_add(pCollisionB->elem, &gWorld->elements_to_be_freed);
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

                // MARK: -- okay this is the common case / vanilla collision handling for "durable" (not scenery) objects
                else
                {
                    int object_destroyed = 0;
                    int durability_a = pCollisionA->elem->durability;
                    int durability_b = pCollisionB->elem->durability;


                    // MARK: -- apply force (in some collision cases)
                    if(world_coll_act == COLLISION_ACTION_DAMAGE)
                    {
                        // pCollisionA points at OBJ_SHIP instead of OBJ_BULLET
                        if(pCollisionA->elem->moving &&
                            pCollisionB->elem->moving)
                        {
                            float bullet_vtransfer = 0.2;

                            update_object_velocity(pCollisionA->elem->elem_id,
                                                   pCollisionB->elem->physics.ptr->vx*bullet_vtransfer,
                                                   pCollisionB->elem->physics.ptr->vy*bullet_vtransfer,
                                                   pCollisionB->elem->physics.ptr->vz*bullet_vtransfer,
                                                   1);
                        }
                    }
                    
                    game_handle_collision(pCollisionA->elem, pCollisionB->elem, world_coll_act);
                    gameNetwork_handle_collision(pCollisionA->elem, pCollisionB->elem, world_coll_act);
                    game_ai_collision(pCollisionA->elem, pCollisionB->elem, world_coll_act);

                    // MARK: -- BY NOW all logical collision handling applicable to  game state is done
                    if(world_coll_act == COLLISION_ACTION_DAMAGE)
                    {
                        float durability_tmp = pCollisionA->elem->durability;

                        if(pCollisionB->elem->object_type == OBJ_POWERUP_GENERIC ||
                           pCollisionA->elem->object_type == OBJ_POWERUP_GENERIC)
                        {
                            DBPRINTF(("powerup collided with %d\n", pCollisionA->elem->object_type));
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
                                // add explosion graphic
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
                            if(pCollisionB->elem->object_type == OBJ_SHIP || pCollisionB->elem->object_type == OBJ_PLAYER)
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
                    char* nametag = pCollisionB->elem->stuff.nametag;

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
                    world_remove_object(pCollisionB->elem->elem_id);
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

    if(strlen(collisionStr) > strlen(collisionStrPfx))// debug printing
    {
        strcat(collisionStr, "\t END collision omeboi\n");
        DBPRINTF((collisionStr));
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


// TODO: use callbacks instead
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

#endif
