
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
static void draw_btree_elements(WorldElem* pElem, float priority);

extern int isLandscape;

int world_inited = 0;
int game_terminated_gracefully = 0;
char *world_data = NULL;
int game_paused = 1;
void (*glFlightDrawframeHook)(void) = gameDialogInitialCountdown;

unsigned int visibleElementsLen = 0;

int numEnemies = 0;

int tex_pass = 0;
int goal_fps = GAME_FRAME_RATE;

float block_scale = 1;

WorldElem targetingReticleElem;
float reticleZDist = -10;

game_timeval_t time_engine_sound_next = 0;

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


world_elem_btree_node visibleBtreeRootStorage1 = WORLD_ELEM_BTREE_NODE_ZERO;
world_elem_btree_node visibleBtreeRootStorage2 = WORLD_ELEM_BTREE_NODE_ZERO;
world_elem_btree_node *visibleBtreeRoot = &visibleBtreeRootStorage1;
world_elem_btree_node *visibleBtreeRootBuilding = &visibleBtreeRootStorage2;
unsigned int visibleBtreeDrawn = 0;
WorldElemListNode* btreeVisibleTest = NULL;

unsigned int draw_elem_max = PLATFORM_DRAW_ELEMS_MAX;

static void
world_elem_btree_restart()
{
    unsigned int prev = world_elem_btree_ptr_idx_set(0);
    world_elem_btree_destroy_root(&visibleBtreeRootStorage1);
    world_elem_btree_ptr_idx_set(1);
    world_elem_btree_destroy_root(&visibleBtreeRootStorage2);
    world_elem_btree_ptr_idx_set(prev);
    btreeVisibleTest = NULL;
}

static void
world_elem_btree_add_all(WorldElem* pElem)
{
    int prev = world_elem_btree_ptr_idx_set(0);
    world_elem_btree_insert(&visibleBtreeRootStorage1, pElem, pElem->renderInfo.distance);
    world_elem_btree_ptr_idx_set(1);
    world_elem_btree_insert(&visibleBtreeRootStorage2, pElem, pElem->renderInfo.distance);
    world_elem_btree_ptr_idx_set(prev);
}

void
glFlightFrameStage1()
{
    int dynamic_draw_distance = 0;
    int backface_culling = 1;
    WorldElemListNode* pListNode = NULL;
    WorldElemListNode* pCameraWatchNode = NULL;
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
    
    world_lock();
    
    if(!world_inited && world_data)
    {
        bindTexture(TEXTURE_ID_FONTMAP2);
        bindTexture(TEXTURE_ID_FONTMAP);
        if(controlsCalibrated) needTrimLock = 1;
        
        gameGraphicsUninit();
        gameMapReRender();
        gameNetwork_worldInit();
        my_ship_id = WORLD_ELEM_ID_INVALID;
        gameStateSinglePlayer.started = 0;
        game_terminated_gracefully = 1;
        gameGraphicsInit();
        gameInterfaceReset();
        
        world_elem_btree_restart();
        
        visibleElementsLen = 0;
        
        //do_draw_bail = 1;
        // 1.x clear pending removals from last pass
        // 1.1 walk list of pending removals, removing from visible list
        
        goto draw_bail;
    }
    
//    if(!gameDialogCalibrateDone && n_textures < TEXTURE_ID_PRELOAD+1)
//    {
//        gameDialogCalibrateDone = 1;
//        gameDialogInitialCountdown();
//    }

    extern void update_time_ms_frame_tick();
    update_time_ms_frame_tick();
    
calibrate_bail:
    get_time_ms();
#if GAME_PLATFORM_ANDROID
    time_ms = time_ms_wall;
#endif
    
    if(gameInterfaceControls.textMenuControl.visible ||
       gameInterfaceControls.keyboardEntry.visible || !controlsCalibrated)
    {
        game_paused = 1;
    }
    else game_paused = 0;
    
    if(game_paused && !gameNetworkState.connected)
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
    const char* engine_sounds[] = {"engine", "engineslow"};
    const float engine_sounds_duration[] = {1250, 347};
    if(time_ms >= time_engine_sound_next || time_engine_sound_next == 0)
    {
        float minSpeed = 0.1;
        
        if(targetSpeed/maxSpeed >= minSpeed)
        {
            int sound_idx = 0;
            float rate = 0.25 + 2.0*(speed/maxSpeed);
            
            char* sndName = engine_sounds[sound_idx];
            
            /*
            if(speed/maxSpeed < 0.33)
            {
                sndName = "engineslow";
            }
            else if(speed/ maxSpeed >= 0.66)
            {
                sndName = "enginefast";
            }
             */
            
            gameAudioPlaySoundAtLocationWithRate(sndName, 0.75, my_ship_x, my_ship_y, my_ship_z, rate);
            
            time_engine_sound_next = time_ms + (engine_sounds_duration[sound_idx] * (1.0/rate));
        }
    }
    
    game_run();
    
    get_time_ms();
    
    if(!gameNetworkState.connected || gameNetworkState.hostInfo.hosting) game_ai_run();
    
    // JB: moved to background thread
    //do_game_network_read();
    
    // TODO: this is being called at most once every 1/60th of a second (16ms)
    do_game_network_world_update();
    
    gameNetworkState.msgQueue.cleanupWaiting = 1;
    while(gameNetworkState.msgQueue.cleanup) { int i; i++; }
    gameNetworkState.msgQueue.cleanupWaiting = 0;
    
    gameNetworkMessageQueued* pNetworkMsg = gameNetworkState.msgQueue.head.next;
    while(pNetworkMsg && gameNetworkState.msgQueue.cleanup == 0)
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
    
    if(glFlightDrawframeHook) glFlightDrawframeHook();
    
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
        world_get_last_object()->renderInfo.fade_in_count = 300;
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
        gameAudioPlaySoundAtLocationWithRate("teleport", 1.0, gameCamera_getX(), gameCamera_getY(), gameCamera_getZ(), 1.0);
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
            else if(camera_fix.frames > 0 && (pCameraWatchNode = world_elem_list_find(camera_fix.elem_id, &gWorld->elements_moving)))
            {
                camera_fix.frames--;
                
                if(pCameraWatchNode)
                {
                    gameCamera_initWithHeading(my_ship_x, my_ship_y, my_ship_z,
                                               pCameraWatchNode->elem->physics.ptr->x - my_ship_x,
                                               pCameraWatchNode->elem->physics.ptr->y - my_ship_y,
                                               pCameraWatchNode->elem->physics.ptr->z - my_ship_z);
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
            
            // updating targeting reticle
            targetingReticleElem.physics.ptr = &targetingReticleElem.physics.data;
            targetingReticleElem.physics.ptr->x = pWorldElemMyShip->physics.ptr->x + ship_z_vec[0] * reticleZDist;
            targetingReticleElem.physics.ptr->y = pWorldElemMyShip->physics.ptr->y + ship_z_vec[1] * reticleZDist;
            targetingReticleElem.physics.ptr->z = pWorldElemMyShip->physics.ptr->z + ship_z_vec[2] * reticleZDist;
            targetingReticleElem.texture_id = TEXTURE_ID_CONTROLS_FIRE;
            targetingReticleElem.scale = 2;
            
            sprintf(statsMessage, "%.0f %.0f  %.0f   %03d %03d\n"
                    "^2^2^C   ^B   ^A    ^P   ^O   ^1^1  %s",
                    (pWorldElemMyShip->durability / (float) DURABILITY_PLAYER) * 100.0,
                    (game_ammo_bullets / game_ammo_bullets_max) * 100,
                    game_ammo_missles,
                    //gameStateSinglePlayer.caps_owned, gameStateSinglePlayer.caps_found,
                    //pWorldElemMyShip->stuff.flags.mask & STUFF_FLAGS_TURRET? 1: 0,
                    //pWorldElemMyShip->stuff.flags.mask & STUFF_FLAGS_SHIP? 1: 0,
                    gameStateSinglePlayer.stats.score,
                    gameNetworkState.connected ?
                    (int) 0 : 
                    game_time_elapsed(),
                    game_status_string);
            strcpy(gameInterfaceControls.statsTextRect.text, statsMessage);

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
    
    // short-cut adding new elements to visibility trees
    pVisCheckPtr = gWorld->elements_to_be_added.next;
    while(pVisCheckPtr)
    {
        pVisCheckPtr->elem->renderInfo.distance =
            distance(gameCamera_getX(),
                     gameCamera_getY(),
                     gameCamera_getZ(),
                     pVisCheckPtr->elem->physics.ptr->x,
                     pVisCheckPtr->elem->physics.ptr->y,
                     pVisCheckPtr->elem->physics.ptr->z);
        
        world_elem_btree_add_all(pVisCheckPtr->elem);
        
        pVisCheckPtr = pVisCheckPtr->next;
    }
    
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
    
    // start visible-subset building/policing (spans multiple frames) over if necessary
    // alternating between current-visible-list, global moving/nonstatic objects list and
    // potentially-visible list for region XYZ
    
    // visibility-sorting binary tree
    if(!btreeVisibleTest)
    {
        if(visibleBtreeRoot == &visibleBtreeRootStorage1)
        {
            visibleBtreeRoot = &visibleBtreeRootStorage2;
            visibleBtreeRootBuilding = &visibleBtreeRootStorage1;
            world_elem_btree_ptr_idx_set(0);
        }
        else
        {
            visibleBtreeRoot = &visibleBtreeRootStorage1;
            visibleBtreeRootBuilding = &visibleBtreeRootStorage2;
            world_elem_btree_ptr_idx_set(1);
        }
        
        world_elem_btree_destroy_root(visibleBtreeRootBuilding);
        
        btreeVisibleTest = gWorld->elements_list.next;
    }
    
    // walk part of the list and add some potentially-visible elements
    unsigned int btreeVisibleTestCount = 400;
    unsigned int btreeVisIdx = (visibleBtreeRootBuilding == &visibleBtreeRootStorage1 ? 0 : 1);
    while(btreeVisibleTest && btreeVisibleTestCount > 0)
    {
        if(btreeVisibleTest->elem->stuff.btree_node[btreeVisIdx])
        {
        }
        else
        {
            btreeVisibleTest->elem->renderInfo.distance =
                distance(gameCamera_getX(),
                         gameCamera_getY(),
                         gameCamera_getZ(),
                         btreeVisibleTest->elem->physics.ptr->x,
                         btreeVisibleTest->elem->physics.ptr->y,
                         btreeVisibleTest->elem->physics.ptr->z);
            
            // TODO: account for scale approx dist to boundaries of the model poly
            
            int visible = element_visible(btreeVisibleTest->elem, visible_distance, visible_dot);
            
            if(visible)
            {
                world_elem_btree_insert(visibleBtreeRootBuilding, btreeVisibleTest->elem, btreeVisibleTest->elem->renderInfo.distance);
            }
        }
        btreeVisibleTest = btreeVisibleTest->next;
        btreeVisibleTestCount--;
    }
    
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
    // TODO: THIS IS NOT WORKING because added-list is being cleaned in clear_pending
    WorldElemListNode* pAddedPtr = gWorld->elements_to_be_added.next;
    while(pAddedPtr)
    {
        pAddedPtr->elem->in_visible_list = 1;
        world_elem_list_add_sorted(&gWorld->elements_visible, &visibleSkipList,
                                   pAddedPtr->elem, element_dist_compare);
        visibleElementsLen++;
        
        pAddedPtr->elem->renderInfo.distance = distance(gameCamera_getX(),
                                                        gameCamera_getY(),
                                                        gameCamera_getZ(),
                                                        pAddedPtr->elem->physics.ptr->x,
                                                        pAddedPtr->elem->physics.ptr->y,
                                                        pAddedPtr->elem->physics.ptr->z);
        
        // insert into all visible-trees immediately
        unsigned int prev_world_elem_btree_ptr_idx = world_elem_btree_ptr_idx_set(0);
        world_elem_btree_insert(&visibleBtreeRootStorage1, pAddedPtr->elem, pAddedPtr->elem->renderInfo.distance);
        world_elem_btree_ptr_idx_set(1);
        world_elem_btree_insert(&visibleBtreeRootStorage2, pAddedPtr->elem, pAddedPtr->elem->renderInfo.distance);
        world_elem_btree_ptr_idx = prev_world_elem_btree_ptr_idx;
        
        world_elem_list_remove(pAddedPtr->elem, &gWorld->elements_to_be_added);
        
        pAddedPtr = pAddedPtr->next;
    }
    
    WorldElemListNode* pDrawCur = gWorld->elements_visible.next;
    
    // draw bounding lines
    drawLineBegin();
    float lineBounds = 10;
    /*
    if(my_ship_x <= lineBounds || my_ship_x >= gWorld->bound_x-lineBounds ||
       my_ship_y <= lineBounds || my_ship_y >= gWorld->bound_y-lineBounds ||
       my_ship_z <= lineBounds || my_ship_z >= gWorld->bound_z-lineBounds)
     */
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
    
    // draw world-defined lines
    drawLineBegin();
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
            drawLineWithColorAndWidth(lineA, lineB, lineColor, 1.0);
        }
        pElem->lifetime--;
        if(pElem->lifetime <= 0)
        {
            world_elem_list_remove(pElem, &gWorld->drawline_list_head);
            world_elem_free(pElem);
        }
    }
    drawLineEnd();
    
    // actual drawing (happens asynchronouly)
    drawElemStart(pDrawCur);
    visibleBtreeDrawn = 0;
    world_elem_btree_walk_should_abort = 0;
    world_elem_btree_walk(visibleBtreeRoot, draw_btree_elements);
    drawElemEnd();
    count_elems_last = visibleBtreeDrawn;
    
    drawBillboard(&targetingReticleElem);
    
    drawRadar();
    drawControls();
    
    if(!gameInterfaceControls.consoleHidden)
    {
        /*
        if(time_ms - console_write_time > 5000 && !game_paused)
        {
            consoleMessage[0] = '\0';
        }
        */
        
        unsigned l = console_typewriter_lag+1;
        char* p = consoleMessage, *pdest = gameInterfaceControls.consoleTextRect.text;
        while(*p && l > 0)
        {
            if(*p == '\n')
            {
                
            }
            
            *pdest = *p;
            pdest++;
            p++;
            l--;
        }
        *pdest = '\0';
        
        if(l == 0) console_typewriter_lag += 1;
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
    return;
}

static void
clear_vis_btree_removed(WorldElem* headElem)
{
    while(headElem)
    {
        if (btreeVisibleTest && btreeVisibleTest->elem == headElem)
        {
            btreeVisibleTest = NULL;
        }
        world_elem_btree_remove_all(NULL, headElem, headElem->renderInfo.distance);
        
        break;
    }
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
        
        clear_vis_btree_removed(pRemovedPtr->elem);
        
        // HACK: supposed to be handled by elem_remove(), but android crashes if this isn't here...
        if(world_elem_list_remove_watch_elem == pRemovedPtr) world_elem_list_remove_watch_elem = NULL;

		pRemovedPtr = pRemovedPtr->next;
	}

	// 1.2 clear those bitches, fer realz
	world_clear_pending();
}

static void
draw_btree_elements(WorldElem* pElem, float priority)
{
    if(priority < /*drawDistanceFar*/ 100 || pElem->renderInfo.priority)
    {
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
    }
    else
    {
        //world_elem_btree_walk_should_abort = 1;
    }
}
