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
    COLLISION_ACTION_CAPTURE = 6,
    COLLISION_ACTION_NEXTLEVEL = 7,
    COLLISION_ACTION_REFLECT = 8
};

typedef unsigned int collision_action_t;

typedef collision_action_t collision_action_table_t[OBJ_LAST][OBJ_LAST];

const static collision_action_t
collision_actions[OBJ_LAST][OBJ_LAST];

extern void load_map_and_host_game();

//extern char collidedPortalNametagLast[256];

void
collision_actions_set_default();

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
    
    while(state->collision_recs_iterate_cur && state->collision_recs_iterate_cur->next) {

        // distinct pairs of objects
        WorldElemListNode *pCollisionA = state->collision_recs_iterate_cur;
        WorldElemListNode *pCollisionB = state->collision_recs_iterate_cur->next;
        WorldElem *pElem = pCollisionA->elem, *pRegionElem = pCollisionB->elem;

        // MARK: -- "initiator" of the colliision (A) determined by object_type ordinal     - see world.c
        if (pElem->object_type > pRegionElem->object_type)
        {
            DBPRINTF(("collision.h WARN: collision-pair objects ordinality violation prolly buggy / THISISATODO"));
        }

        if(pCollisionB)
        {
            WorldElemListNode* pCollision = pCollisionB, *pCollision2 = pCollisionA;

            // action recorded by world-boundary-checks
            collision_action_t world_coll_act = pCollision->userarg;
            
            // always trigger collision logic in certain cases (map building)
            if(pCollision->elem->object_type == OBJ_BULLET)
            {
                if(pCollision->elem->stuff.bullet.action == ACTION_REPLACE_OBJECT)
                {
                    world_coll_act = COLLISION_ACTION_DAMAGE;
                }
            }
            
            switch(world_coll_act) {
                // JB: this should now be handled in world_update()
                case COLLISION_ACTION_REPULSE:
                    break;

                case COLLISION_ACTION_NONE:
                    break;

                case COLLISION_ACTION_FLAG:
                    pCollision->elem->stuff.flags.mask |= pCollision2->elem->stuff.flags.mask;
                    // continue to normal damage handling

                case COLLISION_ACTION_CAPTURE:

                case COLLISION_ACTION_POWERUP_GRAB:

                case COLLISION_ACTION_DAMAGE:
                    // bullet placing object?
                    if (pCollision->elem->type == MODEL_BULLET
                        /*
                        && pCollision->elem->stuff.bullet.action >= ACTION_PLACE_BLOCK
                        && pCollision->elem->stuff.bullet.action <= ACTION_PLACE_TURRET
                         */
                        && pCollision->elem->stuff.bullet.action == ACTION_SHOOT_BLOCK)
                    {
                        pCollision->elem->remove_pending = 1;
                        world_elem_list_add(pCollision->elem, &gWorld->elements_to_be_freed);

                        /*
                        if(pCollisionA->elem->in_visible_list)
                        {
                            visible_list_remove(pCollisionA->elem, &visibleElementsLen);
                        }
                         */

                        float newBlock[3];
                        float block_size = pCollision2->elem->scale;
                        float k[3][6];
                        float v[3];
                        float dist;

                        k[0][0] = 1;
                        k[1][0] = 0;
                        k[2][0] = 0;
                        k[0][1] = -1;
                        k[1][1] = 0;
                        k[2][1] = 0;
                        k[0][2] = 0;
                        k[1][2] = 1;
                        k[2][2] = 0;
                        k[0][3] = 0;
                        k[1][3] = -1;
                        k[2][3] = 0;
                        k[0][4] = 0;
                        k[1][4] = 0;
                        k[2][4] = 1;
                        k[0][5] = 0;
                        k[1][5] = 0;
                        k[2][5] = -1;

                        v[0] = pCollision->elem->physics.ptr->x - pCollision2->elem->physics.ptr->x;
                        v[1] = pCollision->elem->physics.ptr->y - pCollision2->elem->physics.ptr->y;
                        v[2] = pCollision->elem->physics.ptr->z - pCollision2->elem->physics.ptr->z;

                        dist = distance(pCollision->elem->physics.ptr->x,
                                        pCollision->elem->physics.ptr->y,
                                        pCollision->elem->physics.ptr->z,
                                        pCollision2->elem->physics.ptr->x,
                                        pCollision2->elem->physics.ptr->y,
                                        pCollision2->elem->physics.ptr->z);
                        int m = 0;
                        float md = 0;
                        for (int i = 0; i < 6; i++) {
                            float d = (k[0][i] * v[0] + k[1][i] * v[1] + k[2][i] * v[2]) / dist;
                            if (md == 0 || d < md) {
                                m = i;
                                md = d;
                            }
                        }

                        newBlock[0] = pCollision->elem->physics.ptr->x + k[0][m] * block_size;
                        newBlock[1] = pCollision->elem->physics.ptr->y + k[1][m] * block_size;
                        newBlock[2] = pCollision->elem->physics.ptr->z + k[2][m] * block_size;

                        int obj_id;

                        if (pCollision->elem->stuff.bullet.action == ACTION_SHOOT_BLOCK) {
                            int model = pCollision2->elem->type;
                            int tex_id = pCollision2->elem->texture_id;

                            obj_id =
                                    world_add_object(model, newBlock[0], newBlock[1], newBlock[2],
                                                     0, 0, 0, pCollision2->elem->scale, tex_id);

                            world_get_last_object()->object_type = OBJ_BLOCK;
                            world_get_last_object()->stuff.affiliation =
                                    pCollision->elem->stuff.affiliation;
                        } else if (pCollision->elem->stuff.bullet.action == ACTION_PLACE_TURRET) {
                            obj_id =
                                    world_add_object(MODEL_PYRAMID, newBlock[0], newBlock[1],
                                                     newBlock[2],
                                                     0, 0, 0, 1, TEXTURE_ID_TURRET);

                            update_object_velocity(obj_id, 0, 0, 0, 0);
                            world_get_last_object()->object_type = OBJ_TURRET;
                            world_get_last_object()->stuff.affiliation =
                                    pCollision->elem->stuff.affiliation;
                        } else if (pCollision->elem->stuff.bullet.action == ACTION_PLACE_SHIP) {
                            obj_id =
                                    world_add_object(MODEL_SHIP1, newBlock[0], newBlock[1],
                                                     newBlock[2],
                                                     0, 0, 0, 1, TEXTURE_ID_ENEMYSHIP);

                            update_object_velocity(obj_id, 0, 0, 0, 0);
                            world_get_last_object()->stuff.u.enemy.target_id = WORLD_ELEM_ID_INVALID;
                            world_get_last_object()->stuff.u.enemy.intelligence = gameStateSinglePlayer.enemy_intelligence;
                            world_get_last_object()->stuff.u.enemy.enemy_state = ENEMY_STATE_PATROL;
                            world_get_last_object()->durability = gameStateSinglePlayer.enemy_durability;
                            world_get_last_object()->object_type = OBJ_SHIP;
                        }
                    } else if (pCollision->elem->stuff.bullet.action == ACTION_BLOCK_TEXTURE) {
                        pCollision2->elem->texture_id++;
                        if (pCollision2->elem->texture_id >= n_textures)
                            pCollision2->elem->texture_id = 0;

                        if (!pCollision->elem->remove_pending) {
                            pCollision->elem->remove_pending = 1;
                            world_elem_list_add(pCollision->elem, &gWorld->elements_to_be_freed);
                        }
                    } else if (pCollision->elem->stuff.bullet.action == ACTION_SCALE_OBJECT) {
                        float scale_new = pCollision2->elem->scale * 2;
                        if (scale_new >= 16) scale_new = 0.5;

                        int obj_id = world_add_object(pCollision2->elem->type,
                                                      pCollision2->elem->physics.ptr->x,
                                                      pCollision2->elem->physics.ptr->y,
                                                      pCollision2->elem->physics.ptr->z,
                                                      pCollision2->elem->physics.ptr->alpha,
                                                      pCollision2->elem->physics.ptr->beta,
                                                      pCollision2->elem->physics.ptr->gamma,
                                                      scale_new,
                                                      pCollision2->elem->texture_id);

                        world_get_last_object()->object_type = pCollision2->elem->object_type;

                        if (!pCollision2->elem->remove_pending) {
                            pCollision2->elem->remove_pending = 1;
                            world_elem_list_add(pCollision2->elem, &gWorld->elements_to_be_freed);
                        }

                        if (!world_elem_list_find(pCollision->elem->elem_id,
                                                  &gWorld->elements_to_be_freed)) {
                            world_elem_list_add(pCollision->elem, &gWorld->elements_to_be_freed);
                        }
                    } else if (pCollision->elem->stuff.bullet.action == ACTION_REPLACE_OBJECT) {
                        static int replace_object_last = 0;

                        int replace_object_table[][3] = {
                                {MODEL_CUBE2,  OBJ_BLOCK,            TEXTURE_ID_BLOCK},
                                {MODEL_CUBE,   OBJ_BLOCK,            TEXTURE_ID_BLOCK},
                                {MODEL_TURRET, OBJ_TURRET,           TEXTURE_ID_TURRET},
                                {MODEL_SPRITE, OBJ_POWERUP_GENERIC,  TEXTURE_ID_POWERUP},
                                {MODEL_CUBE2,  OBJ_SPAWNPOINT,       TEXTURE_ID_SPAWNPOINT},
                                {MODEL_CUBE2,  OBJ_SPAWNPOINT_ENEMY, TEXTURE_ID_ENEMY_SPAWNPOINT},
                                {MODEL_SHIP2,  OBJ_SHIP,             TEXTURE_ID_ENEMYSHIP},
                                {MODEL_SHIP3,  OBJ_SHIP,             TEXTURE_ID_ENEMYSHIP_ACE}
                        };

                        int r = replace_object_last + 1;
                        if (r >= (sizeof(replace_object_table) / sizeof(replace_object_table[0])))
                            r = 0;

                        int replace_obj_model = replace_object_table[r][0];
                        int replace_obj_type = replace_object_table[r][1];
                        int replace_tex_id = replace_object_table[r][2];
                        float replace_obj_scale = pCollision2->elem->scale;
                        replace_object_last = r;

                        int obj_id =
                                world_add_object(replace_obj_model,
                                                 pCollision2->elem->physics.ptr->x,
                                                 pCollision2->elem->physics.ptr->y,
                                                 pCollision2->elem->physics.ptr->z,
                                                 0, 0, 0,
                                                 replace_obj_scale,
                                                 replace_tex_id);
                        world_get_last_object()->object_type = replace_obj_type;
                        update_object_velocity(obj_id, 0, 0, 0, 0);

                        if (!pCollision2->elem->remove_pending) {
                            pCollision2->elem->remove_pending = 1;
                            world_elem_list_add(pCollision2->elem, &gWorld->elements_to_be_freed);
                        }

                        if (!world_elem_list_find(pCollision->elem->elem_id,
                                                  &gWorld->elements_to_be_freed)) {
                            pCollision->elem->remove_pending = 1;
                            world_elem_list_add(pCollision->elem, &gWorld->elements_to_be_freed);
                        }
                    } else {
                        int object_destroyed = 0;
                        int object_damage = world_coll_act == COLLISION_ACTION_DAMAGE;
                        int durability_a = pCollision->elem->durability;
                        int durability_b = pCollision2->elem->durability;

                        // INFO: moved bullet-impact handling to prior to handle-collision

                        game_handle_collision(pCollision->elem, pCollision2->elem, world_coll_act);
                        gameNetwork_handle_collision(pCollision->elem, pCollision2->elem,
                                                     world_coll_act);
                        game_ai_collision(pCollision->elem, pCollision2->elem, world_coll_act);

                        if (object_damage) {
                            // handle bullet impact
                            WorldElem* pBul = (pCollision2->elem->object_type == OBJ_BULLET) ? pCollision2: pCollision;
                            WorldElem* pImp = pBul == pCollision ? pCollision2 : pCollision;

                            // impact
                            if (pBul == OBJ_BULLET &&
                                object_type_impacts(pImp->object_type)) {

                                float bullet_vtransfer = 0.2 * pImp->durability + 0.001;

                                update_object_velocity(pImp->elem_id,
                                                       pImp->physics.ptr->vx *
                                                       bullet_vtransfer,
                                                       pImp->physics.ptr->vy *
                                                       bullet_vtransfer,
                                                       pImp->physics.ptr->vz *
                                                       bullet_vtransfer,
                                                       1);
                            }

                            //////////////////////////

                            float durability_tmp = pCollision->elem->durability;

                            pCollision->elem->durability -= pCollision2->elem->durability;
                            pCollision2->elem->durability -= durability_tmp;

                            if (pCollision->elem->durability <= 0) {
                                game_handle_destruction(pCollision->elem);
                                gameNetwork_handle_destruction(pCollision->elem);

                                if (!pCollision->elem->remove_pending) {
                                    pCollision->elem->remove_pending = 1;
                                    world_elem_list_add(pCollision->elem,
                                                        &gWorld->elements_to_be_freed);
                                }

                                if (durability_a > 0) {
                                    int obj_id =
                                            world_add_object(MODEL_ICOSAHEDRON,
                                                             pCollision->elem->physics.ptr->x +
                                                             (pCollision->elem->physics.ptr->vx *
                                                              -tc),
                                                             pCollision->elem->physics.ptr->y +
                                                             (pCollision->elem->physics.ptr->vy *
                                                              -tc),
                                                             pCollision->elem->physics.ptr->z +
                                                             (pCollision->elem->physics.ptr->vz *
                                                              -tc),
                                                             pCollision->elem->physics.ptr->alpha,
                                                             pCollision->elem->physics.ptr->beta,
                                                             pCollision->elem->physics.ptr->gamma,
                                                             pCollision->elem->scale,
                                                             TEXTURE_ID_EXPLOSION);
                                    world_get_last_object()->object_type = OBJ_BLOCK;
                                    
                                    world_object_set_lifetime(obj_id, 15);
                                    update_object_velocity(obj_id, 0, 0, 0, 0);
                                    object_destroyed = 1;
                                }
                            }

                            // object destroyed
                            if (pCollision2->elem->durability <= 0) {
                                game_handle_destruction(pCollision2->elem);
                                gameNetwork_handle_destruction(pCollision2->elem);

                                if (!pCollision2->elem->remove_pending) {
                                    pCollision2->elem->remove_pending = 1;
                                    world_elem_list_add(pCollision2->elem,
                                                        &gWorld->elements_to_be_freed);
                                }

                                if (durability_b > 0) {
                                    int obj_id =
                                            world_add_object(MODEL_ICOSAHEDRON,
                                                             pCollision2->elem->physics.ptr->x +
                                                             (pCollision2->elem->physics.ptr->vx *
                                                              -tc),
                                                             pCollision2->elem->physics.ptr->y +
                                                             (pCollision2->elem->physics.ptr->vy *
                                                              -tc),
                                                             pCollision2->elem->physics.ptr->z +
                                                             (pCollision2->elem->physics.ptr->vz *
                                                              -tc),
                                                             pCollision2->elem->physics.ptr->alpha,
                                                             pCollision2->elem->physics.ptr->beta,
                                                             pCollision2->elem->physics.ptr->gamma,
                                                             pCollision2->elem->scale,
                                                             TEXTURE_ID_EXPLOSION);
                                    world_get_last_object()->object_type = OBJ_BLOCK;

                                    world_object_set_lifetime(obj_id, 15);
                                    update_object_velocity(obj_id, 0, 0, 0, 0);
                                    object_destroyed = 1;
                                }

                                // handle spawned-objects (bullets)
                                // mind the OBJ_SHIP/OBJ_PLAYER ordinal - they do not collide
                                if (object_type_impacts(pCollision->elem->object_type)) {
                                    gameAudioPlaySoundAtLocation("dead",
                                                                 pCollision->elem->physics.ptr->x,
                                                                 pCollision->elem->physics.ptr->y,
                                                                 pCollision->elem->physics.ptr->z);

                                    if (pCollision->elem->elem_id == my_ship_id) {
                                        extern int camera_locked_frames;
                                        camera_locked_frames = 240;

                                        /*
                                        gameAudioPlaySoundAtLocation("dead",
                                                                     my_ship_x,
                                                                     my_ship_y,
                                                                     my_ship_z);
                                        */

                                        // add wreckage
                                        world_add_object(MODEL_SPRITE, my_ship_x, my_ship_y,
                                                         my_ship_z,
                                                         0, 0, 0, 1, TEXTURE_ID_WRECKAGE);
                                        world_get_last_object()->object_type = OBJ_DISPLAYONLY;
                                    }
                                }
                            }

                            if (object_destroyed) {
                                gameAudioPlaySoundAtLocation("boom",
                                                             pCollision2->elem->physics.ptr->x,
                                                             pCollision2->elem->physics.ptr->y,
                                                             pCollision2->elem->physics.ptr->z);
                            }
                        }
                    }
                    break;

                case COLLISION_ACTION_PORTAL_TELEPORT:
                {
                    char *nametag = pCollision2->elem->stuff.nametag;

                    console_clear();
                    gameNetwork_disconnectSignal();
                    if (nametag != NULL) {
                        if (strcmp(nametag, GAME_NETWORK_HOST_PORTAL_NAME) == 0) {
                            gameNetwork_host(gameSettingGameTitle, load_map_and_host_game);
                        } else {
                            gameNetwork_connect(nametag, NULL);
                        }
                    }
                    world_remove_object(pCollision2->elem->elem_id);
                }
                    break;
            }
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
