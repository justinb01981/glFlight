//
//  glFlight.c
//  gl_flight
//
//  Created by Justin Brady on 2/28/13.
//
//

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "gameIncludes.h"

#include "object.h"
#include "models.h"
#include "world.h"
#include "textures.h"
#include "gameInterface.h"
#include "gameAI.h"
#include "gameCamera.h"
#include "gameUtils.h"
#include "quaternions.h"
#include "gameAudio.h"
#include "gameGraphics.h"
#include "gameShip.h"
#include "world_file.h"
#include "gameNetwork.h"
#include "maps.h"
#include "action.h"
#include "glFlight.h"
#include "collision.h"
#include "gamePlay.h"
#include "gameGlobals.h"

static void clear_world_pending_removals();

extern int isLandscape;

int world_inited = 0;
int game_terminated_gracefully = 0;
char *world_data = NULL;
int game_paused = 0;

unsigned int visibleElementsLen = 0;

int numEnemies = 0;

int tex_pass = 0;
int goal_fps = GAME_FRAME_RATE;

float block_scale = 1;

WorldElem targetingReticleElem;
float reticleZDist = -10;

unsigned int count_elems_last = 0, count_elems_grid = 100;

struct
{
    int tex_pass_last;
    time_t tex_time_last;
    int fps;
} perf_data;

struct
{
    int frames;
    int elem_id;
} camera_fix = {0, WORLD_ELEM_ID_INVALID};

void
glFlightCamFix(int elem_id)
{
    camera_fix.frames = 120;
    camera_fix.elem_id = elem_id;
}

int drawElemsGoal = 200;

int partial_sort_count = 1;

float viewRotationDegrees = 0;

static WorldElemListNode* pVisCheckPtr = NULL;
static WorldElemListNode* pVisCheckPtrHead = NULL;
static game_timeval_t world_update_time_last;
static int do_track_fps = 0;
WorldElemListNode visibleSkipList;

unsigned int draw_elem_max = PLATFORM_DRAW_ELEMS_MAX;

void
glFlightFrameStage1()
{
    int dynamic_draw_distance = 0;
    int backface_culling = 1;
    WorldElemListNode* pListNode = NULL;
    WorldElem* pWorldElemMyShip = NULL;
    int n_visibleChecks = 400;
    char statsMessage[256];
    static int respawned = 0;
    int ship_durability = DURABILITY_PLAYER;
    float tc = 0;
    float spawn[6];
    
    glEnableClientState(GL_VERTEX_ARRAY);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// turn off bilinear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    float ship_z_vec_prev[3];
    gameShip_getZVector(ship_z_vec_prev);
    
    gameNetwork_lock();
    world_lock();
    
    if(!world_inited && world_data)
    {
        gameGraphicsUninit();
        gameMapReRender();
        gameNetwork_worldInit();
        my_ship_id = WORLD_ELEM_ID_INVALID;
        gameStateSinglePlayer.started = 0;
        game_terminated_gracefully = 1;
        gameGraphicsInit();
        gameInterfaceReset();
        
        visibleElementsLen = 0;
        
        //do_draw_bail = 1;
        // 1.x clear pending removals from last pass
        // 1.1 walk list of pending removals, removing from visible list
        
        goto draw_bail;
    } 

    extern void update_time_ms_frame_tick();
    update_time_ms_frame_tick();
    
    get_time_ms();
#if GAME_PLATFORM_ANDROID
    time_ms = time_ms_wall;
#endif
    
    if(game_paused)
    {
        world_update_time_last = get_time_ms();
        goto paused_bail;
    }
    
    // calculate time since last pass and use that to update world physics
    tc = ((float) time_ms - (float) world_update_time_last) / 1000.0;
    
    if(tc > 1.0)
    {
        world_update_time_last = time_ms;
        goto draw_bail;
    }

    world_update(tc);
    world_update_time_last = time_ms;
    
    // physical collisions between world objects
    do_world_collision_handling(tc);
    
    // play engine sound
    static game_timeval_t time_engine_sound_next = 0;
    const float engine_sound_duration = 350;
    if(time_ms > time_engine_sound_next || time_engine_sound_next == 0)
    {
        float minSpeed = 0.1;
        
        if(speed/maxSpeed >= minSpeed)
        {
            float rateRange[] = {0.75, 1.2};
            float rate = rateRange[0] + (rateRange[1]-rateRange[0])*(speed/maxSpeed);
            
            char* sndName = "engine2";
            if(speed/maxSpeed < 0.33)
            {
                sndName = "engineslow";
            }
            else if(speed/ maxSpeed >= 0.66)
            {
                sndName = "enginefast";
            }
            gameAudioPlaySoundAtLocationWithRate(sndName, my_ship_x, my_ship_y, my_ship_z,
                                                 rate);
            
            time_engine_sound_next = time_ms + (engine_sound_duration / rate);
        }
    }
    
    game_run();
    
    get_time_ms();
    
    if(!gameNetworkState.connected || gameNetworkState.hostInfo.hosting) game_ai_run();
    
    // JB: moved to background thread
    //do_game_network_read();
    
    // TODO: this is being called at most once every 1/60th of a second (16ms)
    do_game_network_world_update();
    
    gameNetworkMessageQueued* pNetworkMsg = gameNetworkState.msgQueue.head.next;
    while(pNetworkMsg)
    {
        gameNetworkMessageQueued* pNetworkMsgNext = pNetworkMsg->next;
        if(!pNetworkMsg->processed)
        {
            do_game_network_handle_msg(&pNetworkMsg->msg, &pNetworkMsg->srcAddr);
        }
        pNetworkMsg->processed = 1;
        pNetworkMsg = pNetworkMsgNext;
    }
    
    do_game_network_write();
    
    paused_bail:
    
    pListNode = world_elem_list_find(my_ship_id, &gWorld->elements_list);
    if(pListNode) pWorldElemMyShip = pListNode->elem;
    
    // accelerate/decelerate
    float sc = maxAccelDecel*tc;
    float boostTargetSpeed = targetSpeed;
    if(speedBoost > 0)
    {
        boostTargetSpeed *= 2;
        speedBoost--;
    }
    if(boostTargetSpeed == 0 && speed < 1) speed = 0;
    else if(speed >= boostTargetSpeed) speed -= sc;
    else if(speed < boostTargetSpeed) speed += sc;
    
    float ship_alpha, ship_beta, ship_gamma;
    gameShip_getEuler(&ship_alpha, &ship_beta, &ship_gamma);
    
    float ship_z_vec[3];
    gameShip_getZVector(ship_z_vec);
    float ship_y_vec[3];
    gameShip_getYVector(ship_y_vec);
    
    int update_ship_stats = 0;
    static int model_my_ship_last = 0;

    if(pWorldElemMyShip && model_my_ship != model_my_ship_last)
    {
        update_ship_stats = 1;
        
        spawn[0] = pWorldElemMyShip->physics.ptr->x;
        spawn[1] = pWorldElemMyShip->physics.ptr->y;
        spawn[2] = pWorldElemMyShip->physics.ptr->z;
        spawn[3] = pWorldElemMyShip->physics.ptr->alpha;
        spawn[4] = pWorldElemMyShip->physics.ptr->beta;
        spawn[5] = pWorldElemMyShip->physics.ptr->gamma;
        
        world_remove_object(my_ship_id);
        my_ship_id = WORLD_ELEM_ID_INVALID;
        
        pWorldElemMyShip = NULL;
    }
    model_my_ship_last = model_my_ship;
    
    if(my_ship_id == WORLD_ELEM_ID_INVALID) // respawn
    {
        respawned = 1;
        
        pWorldElemMyShip = NULL;
        
        game_ammo_missles = game_ammo_missles_max;
        
        if(update_ship_stats)
        {
        }
        else
        {
            world_random_spawn_location(spawn, gameNetworkState.my_player_id);
        }
        
        gameShip_init(spawn[0], spawn[1], spawn[2],
                      spawn[3], spawn[4], spawn[5]);
        
        update_ship_stats = 1;
        
        my_ship_id =
            world_add_object(model_my_ship,
                             my_ship_x, my_ship_y, my_ship_z,
                             my_ship_alpha, my_ship_beta, my_ship_gamma,
                             1, texture_id_playership);
        world_get_last_object()->object_type = OBJ_PLAYER;
        targetSpeed = minSpeed;
        speed = 6;
        update_object_velocity(my_ship_id, ship_z_vec[0]*speed, ship_z_vec[1]*speed, ship_z_vec[2]*speed, 0);
        world_get_last_object()->bounding_remain = 1;
        world_get_last_object()->durability = ship_durability;
        
        gameCamera_init(my_ship_x, my_ship_y, my_ship_z,
                        -spawn[3], -spawn[4], -spawn[5]);
        gameCamera_yawRadians((viewRotationDegrees/180.0) * M_PI);
        //gameCamera_MoveY(5);
        gameCamera_MoveZ(-camera_z_trail);
        //gameCamera_pitchRadians(-M_PI/2);
        //camera_locked_frames = 120;
        
        /* TODO: figure out why a crash in visible-checks if this isn't here */
        // bail this draw
        
        console_write(game_log_messages[GAME_LOG_TELEPORT]);
        gameAudioPlaySoundAtLocationWithRate("teleport", gameCamera_getX(), gameCamera_getY(), gameCamera_getZ(), 1.0);
        return;
    }
    else
    {
        if(pWorldElemMyShip)
        {
            my_ship_x = pWorldElemMyShip->physics.ptr->x;
            my_ship_y = pWorldElemMyShip->physics.ptr->y;
            my_ship_z = pWorldElemMyShip->physics.ptr->z;
            
            pWorldElemMyShip->stuff.affiliation = gameNetworkState.my_player_id;
            
            world_replace_object(pWorldElemMyShip->elem_id, model_my_ship,
                                 my_ship_x, my_ship_y, my_ship_z,
                                 ship_alpha, ship_beta, ship_gamma,
                                 1, texture_id_playership);
            
            // sliding physics -- I just found the mathematical formula for "whee!"
            float tv[3] = {
                -ship_z_vec[0]*speed,
                -ship_z_vec[1]*speed,
                -ship_z_vec[2]*speed
            };
            update_object_velocity_with_friction(my_ship_id, tv, C_THRUST, C_FRICTION);
            
            if(camera_locked_frames > 0)
            {
                camera_locked_frames--;
            }
            else if(camera_fix.frames > 0)
            {
                camera_fix.frames--;
                
                WorldElemListNode* pWatchNode = world_elem_list_find(camera_fix.elem_id, &gWorld->elements_moving);
                if(pWatchNode)
                {
                    gameCamera_initWithHeading(my_ship_x, my_ship_y, my_ship_z,
                                               pWatchNode->elem->physics.ptr->x - my_ship_x,
                                               pWatchNode->elem->physics.ptr->y - my_ship_y,
                                               pWatchNode->elem->physics.ptr->z - my_ship_z);
                    gameCamera_MoveZ(-4);
                    gameCamera_MoveY(1);
                }
            }
            else
            {
                gameCamera_init(my_ship_x,
                                my_ship_y,
                                my_ship_z,
                                -ship_alpha, -ship_beta, -ship_gamma);
                
                gameCamera_yawRadians((viewRotationDegrees/180.0) * M_PI);
                gameCamera_MoveZ(-camera_z_trail);
                gameCamera_MoveY(1);
            }

            respawned = 0;
        }
        else
        {
            // failed, we probably died
            my_ship_id = WORLD_ELEM_ID_INVALID;
        }
    }
    
    if(update_ship_stats)
    {
        switch(model_my_ship)
        {
            case 0:
                maxSpeed = MAX_SPEED * 1.0;
                ship_durability = ship_durability * 1.0;
                game_ammo_missle_recharge = 0.0;
                game_ammo_bullets_recharge = 2.0;
                break;
                
            case 12:
                maxSpeed = MAX_SPEED * 1.0;
                ship_durability = ship_durability * 0.5;
                game_ammo_missle_recharge = 0.0;
                game_ammo_bullets_recharge = 4.0;
                break;
                
            case 14:
                maxSpeed = MAX_SPEED * 1.0;
                ship_durability = ship_durability * 1.5;
                game_ammo_missle_recharge = 0.0;
                game_ammo_bullets_recharge = 2.0;
                break;
                
            default:
                maxSpeed = MAX_SPEED * 1;
                break;
        }
    }
    
    /*****************************************************
     * actual drawing                                    *
     * starts here, no further affecting                 *
     * world-state (drawing happens asynchronously)      *
     *****************************************************/
     
     // 1.x clear pending removals from last pass
     // 1.1 walk list of pending removals, removing from visible list
    clear_world_pending_removals();
    
     // TODO: may be possible to unlock world-state here
     
     drawElem_newFrame();
     
     // set up drawing modes
     if(backface_culling)
     {
         glEnable(GL_CULL_FACE);
         glCullFace(GL_BACK);
     }
     else
     {
         glDisable(GL_CULL_FACE);
         glCullFace(GL_BACK);
     }
     //glDepthFunc(GL_GREATER);
     glDisable(GL_DEPTH_TEST);
     glFrontFace(GL_CCW);
    
    drawBackground();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// set up perspective projection...
	// http://www.songho.ca/opengl/gl_transform.html
	float xfrust = 1;
	float yfrust = xfrust * (viewHeight/viewWidth);
    if(!isLandscape)
    {
        yfrust = 1;
        xfrust = yfrust * (viewWidth/viewHeight);
    }
	glFrustumf(-xfrust, xfrust, -yfrust, yfrust, 2, drawDistanceFar);
    
    n_elements_out_of_order = 0;
    
    // sort elements by distance
    static float visible_dot = 0.7;
    
    // catch our pVisCheckPtr list-node becoming invalid
    if(world_elem_list_remove_watch_elem == NULL)
    {
        pVisCheckPtr = NULL;
    }
    
    // start visible-subset building/policing (spans multiple frames) over if necessary
    // alternating between current-visible-list, global moving/nonstatic objects list and
    // potentially-visible list for region XYZ
    if(pVisCheckPtr == NULL)
    {
        if(pVisCheckPtrHead == &gWorld->elements_visible)
        {
            pVisCheckPtrHead = &gWorld->elements_moving;
        }
        else if(pVisCheckPtrHead == &gWorld->elements_moving)
        {
            WorldElemListNode* pVisCheckWorldHead = &gWorld->elements_list;
            

            WorldElemListNode* pVisRegionHead = world_vis_region_head(gameCamera_getX(), gameCamera_getY(), gameCamera_getZ());
            
            if(VIS_COORD_VALID(gameCamera_getX(), gameCamera_getY(), gameCamera_getZ()) && pVisRegionHead->next) {
                pVisCheckWorldHead = pVisRegionHead;
            }
            
            pVisCheckPtrHead = pVisCheckWorldHead;
        }
        else
        {
            pVisCheckPtrHead = &gWorld->elements_visible;
        }
        
        pVisCheckPtr = pVisCheckPtrHead->next;
    }
    
    // prepare a new skip-list to make inserts quicker
    
    world_elem_list_init(&visibleSkipList);
    world_elem_list_build_skip_list(&gWorld->elements_visible, &visibleSkipList, visibleElementsLen / 20);
    
    // add non-static (moving) objects to visible-list
    
    // walk part of the list and add some potentially-visible elements
    while(pVisCheckPtr && n_visibleChecks > 0)
    {
        const static int min_tex_passes_between_visibility_test = 5;
        
        WorldElem* visListTestCur = pVisCheckPtr->elem;
        WorldElemListNode* pVisCheckPtrNext = pVisCheckPtr->next;
        
        while(visListTestCur)
        {
            if(!visListTestCur->in_visible_list &&
               tex_pass - visListTestCur->renderInfo.tex_pass < min_tex_passes_between_visibility_test)
            {
                // ignored for now
            }
            else
            {
                visListTestCur->renderInfo.tex_pass = tex_pass;
                
                visListTestCur->renderInfo.distance = distance(gameCamera_getX(),
                                                           gameCamera_getY(),
                                                           gameCamera_getZ(),
                                                           visListTestCur->physics.ptr->x,
                                                           visListTestCur->physics.ptr->y,
                                                           visListTestCur->physics.ptr->z);
                
                int visible = element_visible(visListTestCur, visible_distance, visible_dot);
                
                if(visListTestCur->in_visible_list)
                {
                    if(!visible)
                    {
                        // catch list-removals that may invalidate our visCheckPtr
                        if(pVisCheckPtrNext && pVisCheckPtrNext->elem == visListTestCur) pVisCheckPtrNext = pVisCheckPtrNext->next;
                        
                        world_elem_list_remove_skip_list(&visibleSkipList, visListTestCur);
                        world_elem_list_remove(visListTestCur, &gWorld->elements_visible);
                        visListTestCur->in_visible_list = 0;
                        visibleElementsLen--;
                    }
                }
                else
                {
                    if(visible)
                    {
                        world_elem_list_add_sorted(&gWorld->elements_visible, &visibleSkipList,
                                                   visListTestCur, element_dist_compare);
                        visListTestCur->in_visible_list = 1;
                        visibleElementsLen++;
                    }
                }
            }
            
            visListTestCur = visListTestCur->linked_elem;
        }
        
        pVisCheckPtr = pVisCheckPtrNext;
        n_visibleChecks--;
    }
    
    world_elem_list_remove_watch_elem = pVisCheckPtr;
    
    // partially sort current visible subset
    world_elem_list_sort_1(&gWorld->elements_visible, element_dist_compare, 0, INT_MAX);
    
    // if over our ideal draw count, reduce draw distance
    if(dynamic_draw_distance)
    {
        if(visibleElementsLen >= drawElemsGoal && visible_distance > 20) visible_distance -= 0.1;
        else if(visible_distance < 100) visible_distance += 0.1;
    }
    
    glMatrixMode(GL_MODELVIEW);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    
    glEnable(GL_TEXTURE_2D);
    
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glLoadIdentity();
    
    // using rotate/translate functions instead of
    // building the transform matrix myself and using glMultMatrix
    // but keep in mind that each operation is relative to the current
    // reference-frame...
    
    if(isLandscape)
    {
        glRotatef(-90, 0, 0, 1);
    }
    
    // building a stack here, so reverse order..
    // translate->yaw->pitch->roll
    // (worth noting that the terms are misleading because we're using euler
    // angles, and yaw/pitch/roll are usually tait-bryan)
    
    // Z''
    glRotatef(RADIANS_TO_DEGREES(gameCamera_getEulerGamma()), 0, 0, 1);
    // X'
    glRotatef(RADIANS_TO_DEGREES(gameCamera_getEulerBeta()), 1, 0, 0);
    // Z
    glRotatef(RADIANS_TO_DEGREES(gameCamera_getEulerAlpha()), 0, 0, 1);
    glTranslatef(-gameCamera_getX(), -gameCamera_getY(), -gameCamera_getZ());
    
    // walk list of new elements, adding to visible list
    WorldElemListNode* pAddedPtr = gWorld->elements_to_be_added.next;
    while(pAddedPtr)
    {
        pAddedPtr->elem->in_visible_list = 1;
        world_elem_list_add_sorted(&gWorld->elements_visible, &visibleSkipList,
                                   pAddedPtr->elem, element_dist_compare);
        visibleElementsLen++;
        
        pAddedPtr = pAddedPtr->next;
    }
    
    WorldElemListNode* pDrawCur = gWorld->elements_visible.next;
    
    // draw bounding lines
    drawLineBegin();
    float lineBounds = 5;
    if(my_ship_x <= lineBounds || my_ship_x >= gWorld->bound_x-lineBounds ||
       my_ship_y <= lineBounds || my_ship_y >= gWorld->bound_y-lineBounds ||
       my_ship_z <= lineBounds || my_ship_z >= gWorld->bound_z-lineBounds ||
       count_elems_last <= count_elems_grid)
    {
        drawBoundingLineGrid();
    }
    drawLineEnd();
    
    // draw mesh
    WorldElemListNode* cur = gWorld->triangle_mesh_head.next;
    while(cur)
    {
        float c[] = {gameCamera_getX(), gameCamera_getY(), gameCamera_getZ()};
        struct mesh_opengl_t* mesh = (struct mesh_opengl_t*) cur->elem->pVoid;
        mesh_opengl_index_sort(c, mesh);
        drawTriangleMesh(mesh, cur->elem->texture_id);
        cur = cur->next;
    }
    
    // actual drawing (happens asynchronouly)
    unsigned int count_elems = 0, drawn_elems = 0;
    drawElemStart(pDrawCur);
    while(pDrawCur)
    {
        if((visibleElementsLen-count_elems) <= draw_elem_max)
        {
            WorldElem* pElem = pDrawCur->elem;
            
            if(pElem->renderInfo.wireframe || (pElem->head_elem && pElem->head_elem->renderInfo.wireframe))
            {
                drawLineBegin();
                drawLinesElemTriangles(pElem);
                drawLineEnd();
            }
            else
            {
                drawElem(pElem);
            }
            drawn_elems++;
        }
        pDrawCur = pDrawCur->next;
        count_elems++;
    }
    
    drawElemEnd();
    count_elems_last = count_elems;
    
    if(pWorldElemMyShip)
    {
        targetingReticleElem.physics.ptr = &targetingReticleElem.physics.data;
        targetingReticleElem.physics.ptr->x = pWorldElemMyShip->physics.ptr->x + ship_z_vec[0] * reticleZDist;
        targetingReticleElem.physics.ptr->y = pWorldElemMyShip->physics.ptr->y + ship_z_vec[1] * reticleZDist;
        targetingReticleElem.physics.ptr->z = pWorldElemMyShip->physics.ptr->z + ship_z_vec[2] * reticleZDist;
        targetingReticleElem.texture_id = TEXTURE_ID_CONTROLS_FIRE;
        targetingReticleElem.scale = 2;
        drawBillboard(&targetingReticleElem);
        
        sprintf(statsMessage, "^C:%.0f ^B:%.0f ^A:%.0f "
                //"CP:%d/%d Trt:%d Ally:%d "
                "Score:%d Time:%d %s",
                (pWorldElemMyShip->durability / (float) DURABILITY_PLAYER) * 100.0,
                (game_ammo_bullets / game_ammo_bullets_max) * 100,
                game_ammo_missles,
                //gameStateSinglePlayer.caps_owned, gameStateSinglePlayer.caps_found,
                //pWorldElemMyShip->stuff.flags.mask & STUFF_FLAGS_TURRET? 1: 0,
                //pWorldElemMyShip->stuff.flags.mask & STUFF_FLAGS_SHIP? 1: 0,
                gameStateSinglePlayer.stats.score,
                game_time_elapsed(),
                game_status_string);
        strcpy(gameInterfaceControls.statsTextRect.text, statsMessage);
    }
    
    drawLineBegin();
    if(pWorldElemMyShip && pWorldElemMyShip->stuff.towed_elem_id != WORLD_ELEM_ID_INVALID)
    {
        WorldElemListNode *pNodeTowed = world_elem_list_find(pWorldElemMyShip->stuff.towed_elem_id,
                                                             &gWorld->elements_moving);
        if(pNodeTowed)
        {
            float lineA[] = {my_ship_x, my_ship_y, my_ship_z};
            float lineB[] =
            {
                pNodeTowed->elem->physics.ptr->x,
                pNodeTowed->elem->physics.ptr->y,
                pNodeTowed->elem->physics.ptr->z
            };
            float lineColor[] = {1.0, 0.5, 0.0};
            drawLineWithColorAndWidth(lineA, lineB, lineColor, 2.0);
        }
    }
    
    // draw world-defined lines
    WorldElemListNode* lineCur = gWorld->drawline_list_head.next;
    while(lineCur)
    {
        WorldElem* pElem = lineCur->elem;
        lineCur = lineCur->next;
        
        float lineA[3] = {pElem->coords[0], pElem->coords[1], pElem->coords[2]};
        float lineB[3] = {pElem->coords[3], pElem->coords[4], pElem->coords[5]};
        float lineColor[] = {pElem->texcoords[0], pElem->texcoords[1], pElem->texcoords[2]};
        if(distance(gameCamera_getX(), gameCamera_getY(), gameCamera_getZ(),
                    lineA[0], lineA[1], lineA[2]) <= visible_distance ||
           distance(gameCamera_getX(), gameCamera_getY(), gameCamera_getZ(),
                    lineB[0], lineB[1], lineB[2]) <= visible_distance)
        {
            drawLineWithColorAndWidth(lineA, lineB, lineColor, 2.0);
        }
        pElem->lifetime--;
        if(pElem->lifetime <= 0)
        {
            world_elem_list_remove(pElem, &gWorld->drawline_list_head);
            world_elem_free(pElem);
        }
    }

    /*
    float a[] = {0, 0, 0};
    for(int i = 0; i < 3; i++)
    {
        a[i] = 1;
        float j[3] = {
            my_ship_x,
            my_ship_y,
            my_ship_z
        };
        float d = 1.5;
        float k1[3] = {
            j[0] + d*a[0],
            j[1] + d*a[1],
            j[2] + d*a[2]
        };
        float k2[3] = {
            j[0] + -d*a[0],
            j[1] + -d*a[1],
            j[2] + -d*a[2]
        };
        drawLine(j, k1);
        float color[] = {0xff, 0x00, 0x00};
        drawLineWithColorAndWidth(j, k2, color, 1);
        a[i] = 0;
    }
     */
    
    drawLineEnd();
    
    drawRadar();
    drawControls();
    
    if(!gameInterfaceControls.consoleHidden)
    {
        if(time_ms - console_write_time > 5000 && !game_paused)
        {
            consoleMessage[0] = '\0';
        }
        strncpy(gameInterfaceControls.consoleTextRect.text, consoleMessage, sizeof(gameInterfaceControls.consoleTextRect.text)-1);
    }
    
    /****************************************
     * OpenGL frame is ready to be rendered *
     ****************************************/
    
    return;
    
draw_bail:
    world_elem_list_remove_watch_elem = NULL;
    pVisCheckPtr = NULL;
    pVisCheckPtrHead = NULL;
    clear_world_pending_removals();
    return;
}

void
glFlightFrameStage2()
{
    tex_pass++;
    
    // FPS performance tracking
    if(do_track_fps)
    {
        time_t cur_time = time(NULL);
        
        if(cur_time != perf_data.tex_time_last)
        {
            perf_data.fps = tex_pass - perf_data.tex_pass_last;
            
            printf("frames (tex_passes) per sec: %d\n", perf_data.fps);
            perf_data.tex_pass_last = tex_pass;
            perf_data.tex_time_last = cur_time;
            
            if(drawElemsGoal > 50)
            {
                if(perf_data.fps >= goal_fps) drawElemsGoal += 5;
                else drawElemsGoal -= 20;
            }
        }
    }
    
    world_elem_list_release_skip_list(&visibleSkipList);
    
    // done
    world_unlock();
    gameNetwork_unlock();
    return;
}

static void
clear_world_pending_removals()
{
	WorldElemListNode* pRemovedPtr = gWorld->elements_to_be_freed.next;
	while(pRemovedPtr)
	{
		if(pRemovedPtr->elem->in_visible_list)
		{
			visible_list_remove(pRemovedPtr->elem, &visibleElementsLen, &pVisCheckPtr);
		}
        
        // HACK: supposed to be handled by elem_remove(), but android crashes if this isn't here...
        if(world_elem_list_remove_watch_elem == pRemovedPtr) world_elem_list_remove_watch_elem = NULL;

		pRemovedPtr = pRemovedPtr->next;
	}

	// 1.2 clear those bitches, fer realz
	world_clear_pending();
}
