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

#define TURRETXPOWERUP COLLISION_ACTION_REPULSE
#define SHIPXPOWERUP COLLISION_ACTION_POWERUP_GRAB_OR_TOW
#define PLYRXPOWERUP COLLISION_ACTION_POWERUP_GRAB_OR_TOW
#define SPAWNXCAPT COLLISION_ACTION_POWERUP_CAPTURE

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
    {0, 0, /*3*/0, 0, 0, 0, 0, 0, 3, 0, SPAWNXCAPT,       0, 0, 0, 0, 0, 0, 0, 0, 0}, // spawnpoint enemy
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

static void collision_handle_impact(WorldElem* a, WorldElem* b, float tc)
{
    // a,b priority determined by object-id ascending in world.c - e.g. pCollisionB will point at the OBJ_BULLET, pCollisionA at the OBJ_SHIP

    // action recorded by world-boundary-checks
    collision_action_t world_coll_act = a->object_type > b->object_type ? collision_actions[a->object_type][b->object_type] : collision_actions[b->object_type][a->object_type];

    switch (world_coll_act) {
    case COLLISION_ACTION_REPULSE:
        // ignored and handled in world_update
        break;

    case COLLISION_ACTION_NONE:
        break;

    case COLLISION_ACTION_FLAG:
        a->stuff.flags.mask |= b->stuff.flags.mask;
        // continue to normal damage handling

    case COLLISION_ACTION_POWERUP_CAPTURE:

    case COLLISION_ACTION_POWERUP_GRAB_OR_TOW:

    case COLLISION_ACTION_DAMAGE:
        
        {
            int object_destroyed = 0;

            // MARK: -- apply force (in some collision cases)
            if (world_coll_act == COLLISION_ACTION_DAMAGE)
            {
                // pCollisionA points at OBJ_SHIP instead of OBJ_BULLET
                if (a->moving &&
                    b->moving)
                {
                    float bullet_vtransfer = 0.2;

                    update_object_velocity(a->elem_id,
                        b->physics.ptr->vx * bullet_vtransfer,
                        b->physics.ptr->vy * bullet_vtransfer,
                        b->physics.ptr->vz * bullet_vtransfer,
                        1);
                }
            }

            game_handle_collision(a, b, world_coll_act);
            gameNetwork_handle_collision(a, b, world_coll_act);
            game_ai_collision(a, b, world_coll_act);

            // MARK: -- BY NOW all logical collision handling applicable to  game state is done
            if (world_coll_act == COLLISION_ACTION_DAMAGE)
            {
                DBPRINTF(("dmg (type%s->%s)%d->%d", typestr(a->object_type), typestr(b->object_type), a->durability, b->durability));

                int adtmp = a->durability;
                a->durability -= MIN(a->durability, b->durability);
                b->durability -= MIN(b->durability, adtmp);

                if (b->durability >= 0)
                {
                    // add explosion graphic
                    int obj_id =
                        world_add_object(MODEL_ICOSAHEDRON,
                            b->physics.ptr->x + (b->physics.ptr->vx * -tc),
                            b->physics.ptr->y + (b->physics.ptr->vy * -tc),
                            b->physics.ptr->z + (b->physics.ptr->vz * -tc),
                            b->physics.ptr->alpha,
                            b->physics.ptr->beta,
                            b->physics.ptr->gamma,
                            b->scale, TEXTURE_ID_EXPLOSION);
                    world_get_last_object()->object_type = OBJ_BLOCK;

                    world_get_last_object()->destructible = 0;
                    world_object_set_lifetime(obj_id, 30);
                    update_object_velocity(obj_id, 0, 0, 0, 0);

                    object_destroyed = 1;
                    
                }

                // static object destroyed
                if (a->durability <= 0)
                {
                    game_handle_destruction(a);
                    gameNetwork_handle_destruction(a);

                    if (!world_elem_list_find(a->elem_id, &gWorld->elements_to_be_freed))
                    {
                        world_elem_list_add(a, &gWorld->elements_to_be_freed);
                    }

                    if (adtmp > 0)
                    {
                        int obj_id =
                            world_add_object(MODEL_ICOSAHEDRON,
                                a->physics.ptr->x + (a->physics.ptr->vx * -tc),
                                a->physics.ptr->y + (a->physics.ptr->vy * -tc),
                                a->physics.ptr->z + (a->physics.ptr->vz * -tc),
                                a->physics.ptr->alpha,
                                a->physics.ptr->beta,
                                a->physics.ptr->gamma,
                                a->scale, TEXTURE_ID_EXPLOSION);
                        world_get_last_object()->object_type = OBJ_WRECKAGE;

                        world_get_last_object()->destructible = 0;
                        world_object_set_lifetime(obj_id, 30);
                        update_object_velocity(obj_id, 0, 0, 0, 0);
                        object_destroyed = 1;
                    }

                }

                // "moving" object destroyed
                if (b->durability <= 0)
                {
                    game_handle_destruction(b);
                    gameNetwork_handle_destruction(b);

                    if (!world_elem_list_find(b->elem_id, &gWorld->elements_to_be_freed))
                    {
                        world_elem_list_add(b, &gWorld->elements_to_be_freed);
                    }


                    // handle spawned-objects (bullets)
                    if (b->object_type == OBJ_SHIP || b->object_type == OBJ_PLAYER)
                    {
                        gameAudioPlaySoundAtLocation("dead",
                            b->physics.ptr->x,
                            b->physics.ptr->y,
                            b->physics.ptr->z);

                        if (b->elem_id == my_ship_id)
                        {
                            extern int camera_locked_frames;
                            camera_locked_frames = 240;

                            // add wreckage
                            int obj_id = world_add_object(MODEL_SPRITE, my_ship_x, my_ship_y, my_ship_z,
                                0, 0, 0, 1, TEXTURE_ID_WRECKAGE);
                            world_get_last_object()->object_type = OBJ_WRECKAGE;
                            world_get_last_object()->destructible = 0;
                            update_object_velocity(obj_id, 0, 0, 0, 0);
                            world_object_set_lifetime(obj_id, OBJ_LIFETIME_WRECKAGE_FRAMES);
                        }
                    }
                }

                if (object_destroyed)
                {
                    gameAudioPlaySoundAtLocation("boom",
                        a->physics.ptr->x,
                        a->physics.ptr->y,
                        a->physics.ptr->z);
                }
            }
        }
        break;

        case COLLISION_ACTION_PORTAL_TELEPORT:
        {
            char* nametag = b->stuff.nametag;

            console_clear();
            gameNetwork_disconnectSignal();
            if (nametag != NULL) {
                if (strcmp(nametag, GAME_NETWORK_HOST_PORTAL_NAME) == 0)
                {
                    gameNetwork_host(gameSettingGameTitle, load_map_and_host_game);
                }
                else
                {
                    gameNetwork_connect(nametag, NULL);
                }
            }
            world_remove_object(b->elem_id);
        }
        break;
    default:
        break;
    }
}

static void collision_handling_remove_hook(WorldElem* pElem)
{
    if(pElem == gWorld->world_update_state.collision_recs_iterate_cur->elem)
    {
        gWorld->world_update_state.collision_recs_iterate_cur = gWorld->world_update_state.collision_recs_iterate_cur->next;
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
