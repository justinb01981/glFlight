
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

static void clear_world_pending_removals(void);
static void draw_btree_elements(WorldElem* pElem, float priority);

int isLandscape;

int world_inited = 0;
int game_terminated_gracefully = 0;
char *world_data = NULL;
int game_paused = 1;
void (*glFlightDrawframeHook)(void) = gameDialogInitialCountdownDrawCallback;

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
world_elem_btree_node *visibleBtreeRoot = &visibleBtreeRootStorage1;
world_elem_btree_node *visibleBtreeRootBuilding = &visibleBtreeRootStorage1;
unsigned int visibleBtreeDrawn = 0;
WorldElemListNode* btreeVisibleTest = NULL;

unsigned int draw_elem_max = PLATFORM_DRAW_ELEMS_MAX;

static void
world_elem_btree_restart()
{
    world_elem_btree_destroy_root(&visibleBtreeRootStorage1);
    btreeVisibleTest = NULL;
}

static void
world_elem_btree_add_all(WorldElem* pElem)
{
    world_elem_btree_insert(&visibleBtreeRootStorage1, pElem, pElem->renderInfo.distance);
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
    glEnable(GL_DEPTH_TEST);
    
    glClearColor(0, 0, 0, 1.0f);
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
        
        world_elem_btree_restart();
        
        gameGraphicsUninit();
        gameMapReRender();
        gameNetwork_worldInit();
        
        my_ship_id = WORLD_ELEM_ID_INVALID;
        
        gameStateSinglePlayer.started = 0;
        //gameStateSinglePlayer.game_type = GAME_TYPE_NONE;
        game_terminated_gracefully = 1;
        gameGraphicsInit();
        gameInterfaceReset();
        
        visibleElementsLen = 0;
        
        //do_draw_bail = 1;
        // 1.x clear pending removals from last pass
        // 1.1 walk list of pending removals, removing from visible list
        
        goto draw_bail;
    }

    //extern void update_time_ms_frame_tick(void);
    //update_time_ms_frame_tick();
    
calibrate_bail:
    get_time_ms();

    // TODO: move timer handleing to platform code - i think this is only for android?
    time_ms = time_ms_wall;

//    if(gameInputInitialTrimPending())
//    {
//        speed = 0.0;
//    }
    
    if(gameInterfaceControls.textMenuControl.visible ||
       gameInterfaceControls.keyboardEntry.visible
//       || gameInputInitialTrimPending()
       )
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

    // DBPRINTF(("tc:%f", tc));

    world_update(tc);
    world_update_time_last = time_ms;
    
    // physical collisions between world objects
    do_world_collision_handling(tc);
    
    // play engine sound
    const char* engine_sounds[] = {"engine", "engineslow"};
    const float engine_sounds_duration[] = {1250, 347};

    if(time_ms >= time_engine_sound_next
//       && controlsCalibrated
       )
    {
        if(targetSpeed/maxSpeed >= minSpeed)
        {
            int sound_idx = 0;
            float rate = 0.25 + 2.0*(speed/maxSpeed);
            
            const char* sndName = engine_sounds[sound_idx];
            
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
    
    // moved game_ai_run to do_networkd_world_update
    if(gameNetworkState.hostInfo.hosting || !gameNetworkState.connected) game_ai_run();
    
    // JB: moved to background thread
    //do_game_network_read();
    
    // TODO: this is being called at most once every 1/60th of a second (16ms)
    do_game_network_world_update();

    game_lock_lock(&gameNetworkState.msgQueue.lock);
    
    gameNetworkMessageQueued* pNetworkMsg = gameNetworkState.msgQueue.head.next;
    while(pNetworkMsg && !pNetworkMsg->processed)
    {
        gameNetworkMessageQueued* pNetworkMsgNext = pNetworkMsg->next;

        do_game_network_handle_msg(&pNetworkMsg->msg, &pNetworkMsg->srcAddr, pNetworkMsg->receive_time);
        
        pNetworkMsg->processed = 1;
        pNetworkMsg = pNetworkMsgNext;
    }
    
    game_lock_unlock(&gameNetworkState.msgQueue.lock);
    
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

        if (update_ship_stats)
        {
        }
        else
        {
            world_random_spawn_location(spawn, gameNetworkState.my_player_id);  // i am respawning - update global ship orientation/camera?
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
        targetSpeed = MAX_SPEED/10;

        update_object_velocity(my_ship_id, 0,0,0, 0);   // fuck these dont matter because next frame does them

        world_get_last_object()->bounding_remain = 1;
        world_get_last_object()->durability = ship_durability;
        
        console_write(game_log_messages[GAME_LOG_TELEPORT]);
        gameAudioPlaySoundAtLocationWithRate("teleport", 1.0, gameCamera_getX(), gameCamera_getY(), gameCamera_getZ(), 1.0);

        return; // HACK: is this necessary anymore - testing needed
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
                    gameCamera_MoveZ(-5);
                    gameCamera_MoveY(1);
                }
                else
                {
                    camera_fix.frames = 0;
                    camera_fix.elem_id = WORLD_ELEM_ID_INVALID;
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
                    (int) gameNetworkState.time_game_remaining :
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
    
    gameCamera_sanity();
    
    if(update_ship_stats)
    {
        switch(model_my_ship)
        {
            case MODEL_SHIP1:
                maxSpeed = MAX_SPEED * 1.0;
                ship_durability = ship_durability * 1.0;
                game_ammo_missle_recharge = 0.0;
                game_ammo_bullets_recharge = 2.0;
                break;
                
            case MODEL_SHIP2:
                maxSpeed = MAX_SPEED * 1.0;
                ship_durability = ship_durability * 0.5;
                game_ammo_missle_recharge = 0.0;
                game_ammo_bullets_recharge = 2.0;
                break;
                
            case MODEL_SHIP3:
                maxSpeed = MAX_SPEED * 0.8;
                ship_durability = ship_durability * 1.5;
                game_ammo_missle_recharge = 0.0;
                game_ammo_bullets_recharge = 2.0;
                break;
                
            default:
                assert(0);
                //maxSpeed = MAX_SPEED * 1;
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
	glFrustumf(-xfrust, xfrust, -yfrust, yfrust, 2.0, drawDistanceFar);
    
    /*
    glClearColor(0.5f,0.5f,0.5f,1.0f);          // We'll Clear To The Color Of The Fog ( Modified )
    
    glFogf(GL_FOG_MODE, GL_LINEAR);        // Fog Mode
    GLfloat fogColor[4]= {0.5f, 0.5f, 0.5f, 1.0f};
    glFogfv(GL_FOG_COLOR, fogColor);            // Set Fog Color
    glFogf(GL_FOG_DENSITY, 1.0f);              // How Dense Will The Fog Be
    glHint(GL_FOG_HINT, GL_DONT_CARE);          // Fog Hint Value
    glFogf(GL_FOG_START, 50);             // Fog Start Depth
    glFogf(GL_FOG_END, 150);               // Fog End Depth
    glEnable(GL_FOG);
     */
    
    n_elements_out_of_order = 0;

    // 11-29-2023 - visibility tree test was happening here

    // partially sort current visible subset
    //world_elem_list_sort_1(&gWorld->elements_visible, element_dist_compare, 0, /*INT_MAX*/ 128);
    
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
        world_elem_btree_insert(visibleBtreeRootBuilding, pAddedPtr->elem, pAddedPtr->elem->renderInfo.distance);

        world_elem_list_remove(pAddedPtr->elem, &gWorld->elements_to_be_added);

        pAddedPtr = pAddedPtr->next;
    }


    // 1.x clear pending removals from last pass
    // 1.1 walk list of pending removals, removing from visible list
    clear_world_pending_removals();

    // now sort visibile elements

    // sort elements by distance
    static float visible_dot = 0.7;

    // start visible-subset building/policing (spans multiple frames) over if necessary
    // alternating between current-visible-list, global moving/nonstatic objects list and
    // potentially-visible list for region XYZ

    // visibility-sorting binary tree
    if(!btreeVisibleTest)
    {
        visibleBtreeRootBuilding = &visibleBtreeRootStorage1;

        //world_elem_btree_destroy_root(visibleBtreeRootBuilding);

        btreeVisibleTest = gWorld->elements_list.next;
    }

    // walk part of the list and add some potentially-visible elements
    unsigned int btreeVisibleTestCount = 100;
    //unsigned int btreeVisIdx = (visibleBtreeRootBuilding == &visibleBtreeRootStorage1 ? 0 : 1);
    while(btreeVisibleTest && btreeVisibleTestCount > 0)
    {
        btreeVisibleTest->elem->renderInfo.distance =
            distance(gameCamera_getX(),
                     gameCamera_getY(),
                     gameCamera_getZ(),
                     btreeVisibleTest->elem->physics.ptr->x,
                     btreeVisibleTest->elem->physics.ptr->y,
                     btreeVisibleTest->elem->physics.ptr->z);

        world_elem_btree_remove(visibleBtreeRootBuilding, btreeVisibleTest->elem);

        // TODO: account for scale approx dist to boundaries of the model poly
        int visible = element_visible(btreeVisibleTest->elem, visible_distance, visible_dot);

        if(visible)
        {
            world_elem_btree_insert(visibleBtreeRootBuilding, btreeVisibleTest->elem, btreeVisibleTest->elem->renderInfo.distance);
        }

        btreeVisibleTest = btreeVisibleTest->next;
        btreeVisibleTestCount--;
    }



    WorldElemListNode* pDrawCur = gWorld->elements_visible.next;
    
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
    drawElemStart(pDrawCur);
    visibleBtreeDrawn = 0;
    world_elem_btree_walk(visibleBtreeRoot, draw_btree_elements);
    drawElemEnd();
    count_elems_last = visibleBtreeDrawn;
    
    drawBillboard(&targetingReticleElem);
    
    glDisable(GL_DEPTH_TEST);
    
    drawRadar();
    drawControls();
    
    if(!gameInterfaceControls.consoleHidden)
    {        
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
    
    glEnable(GL_DEPTH_TEST);
    
    /****************************************
     * OpenGL frame is ready to be rendered *
     ****************************************/
    
    return;
    
draw_bail:
    world_elem_list_remove_watch_elem = NULL;
    pVisCheckPtr = NULL;
    pVisCheckPtrHead = NULL;
    btreeVisibleTest = NULL;
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
    assert(visibleBtreeRootBuilding);

    while(headElem)
    {
        
//        if (btreeVisibleTest && btreeVisibleTest->elem == headElem)
//        {
//            btreeVisibleTest = btreeVisibleTest->next;
//        }
        world_elem_btree_remove_all(visibleBtreeRootBuilding, headElem);
        
        headElem = headElem->linked_elem;
    }
}

static void
clear_world_pending_removals(void)
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
    if(priority < drawDistanceFar || pElem->renderInfo.priority)
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
        drawElem(pElem);
    }
}
