//
//  gameAI.m
//  gl_flight
//
//  Created by jbrady on 8/9/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <math.h>
#include <limits.h>

#include "gameAI.h"
#include "quaternions.h"
#include "gameGlobals.h"
#include "gameAudio.h"
#include "gameUtils.h"
#include "textures.h"
#include "gamePlay.h"
#include "collision.h"

float skill_m = 0.1; // 10% per skill-level

float diff_m = 0.1;
float scan_dist_increase = 60;
float game_ai_fire_rate_ms = 800.0;
float patrol_area_reached = 2.0;
float tow_v_scale = /*0.5*/ 1.5;
float min_patrol_distance = 20;
float collect_deploy_distance = -2.5;
float change_target_frequency = 10000;
float enemy_turn_rate_dampc = 0.9;

float juke_distance = 20.0, juke_distance_patrol = 50.0;

gameAIState_t gameAIState;


static float
pursuit_speed_for_object(WorldElem* elem, float* accel);

void
ai_debug(char* msg, WorldElem* elem, float f)
{
    if(GAME_AI_DEBUG)
    {
        printf("ai_debug (%p):%s (%f)\n", elem, msg, f);
    }
}

void
game_ai_init()
{
}

static int
target_priority(WorldElem* pSearchElem, WorldElem* pTargetElem, float* dist_ignore)
{
    return game_ai_target_priority(pSearchElem, pTargetElem, dist_ignore);
}

static int
target_avoid_collision(int object_type)
{
    if(object_type == OBJ_PLAYER || object_type == OBJ_SHIP) return 1;
    return 0;
}

void random_heading_vector(float result[3])
{
    float rand_e[3] = {
        rand_in_range(0, M_PI*2*1000)/1000.0,
        rand_in_range(0, M_PI*2*1000)/1000.0,
        rand_in_range(0, M_PI*2*1000)/1000.0
    };
    
    quaternion_t q;
    euler_to_quaternion(rand_e[0], rand_e[1], rand_e[2],
                        &q.w, &q.x, &q.y, &q.z);
    
    result[0] = q.x;
    result[1] = q.y;
    result[2] = q.z;
}

void
game_ai_juke(WorldElem* pElem, float distance)
{
    int i;
    
    pElem->stuff.u.enemy.enemy_state = ENEMY_STATE_JUKE;
    
    float b[] = {gWorld->bound_x, gWorld->bound_y, gWorld->bound_z};
    float p[] = {pElem->physics.ptr->x, pElem->physics.ptr->y, pElem->physics.ptr->z};
    float rvec[3], rdist = distance;
    
    random_heading_vector(rvec);
    
    for(i = 0; i < 3; i++) rvec[i] = p[i] + (rvec[i] * rdist);
    for(i = 0; i < 3; i++) if(rvec[i] < 0) rvec[i] += rdist;
    for(i = 0; i < 3; i++) if(rvec[i] > b[i]) rvec[i] -= rdist;
    
    pElem->stuff.u.enemy.tgt_x = rvec[0];
    pElem->stuff.u.enemy.tgt_y = rvec[1];
    pElem->stuff.u.enemy.tgt_z = rvec[2];
}

int
game_ai_find_target(WorldElem *pElem)
{
    float minDist = INT_MAX;
    int minId = WORLD_ELEM_ID_INVALID;
    float wMin = 0;
    float dist_thresh = pElem->stuff.u.enemy.scan_distance;
    
    if(pElem->stuff.towed_elem_id != WORLD_ELEM_ID_INVALID) return WORLD_ELEM_ID_INVALID;
    
    WorldElemListNode* pTarget = gWorld->elements_moving.next;
    while(pTarget)
    {
        int priority = target_priority(pElem, pTarget->elem, &dist_thresh);
        
        if(priority > 0)
        {
            if(pTarget->elem != pElem &&
               !IS_LINKED_ELEM(pTarget->elem) &&
               pTarget->elem->stuff.affiliation != pElem->stuff.affiliation &&
               pTarget->elem->elem_id != pElem->stuff.towed_elem_id)
            {
                float vx = pTarget->elem->physics.ptr->x - pElem->physics.ptr->x;
                float vy = pTarget->elem->physics.ptr->y - pElem->physics.ptr->y;
                float vz = pTarget->elem->physics.ptr->z - pElem->physics.ptr->z;
                float dist = sqrt(vx*vx + vy*vy + vz*vz);
                
                // calculate "weight"
                float w = (1.0/dist) * priority;
                
                // don't always take highest-weighted target, some element of randomness
                if(dist <= dist_thresh)
                if(w > wMin || rand_in_range(1, 100) <= 25)
                {
                    minId = pTarget->elem->elem_id;
                    minDist = dist;
                    wMin = w;
                }
            }
        }
        
        pTarget = pTarget->next;
    }
    
    if(minDist <= dist_thresh)
    {
        /*
        if(minId == my_ship_id)
        {
            gameAudioPlaySoundAtLocation("lockedon", my_ship_x, my_ship_y, my_ship_z);
        }
         */
        
        pElem->stuff.u.enemy.time_target_acquired = time_ms;
        
        return minId;
    }
    
    if(pElem->stuff.u.enemy.scan_distance > 0 &&
       pElem->stuff.u.enemy.scan_distance < GAME_VARIABLE("ENEMY1_SCAN_DISTANCE_MAX"))
    {
        pElem->stuff.u.enemy.scan_distance += scan_dist_increase;
        ai_debug("scan distance:", pElem, pElem->stuff.u.enemy.scan_distance);
        ai_debug("scan distance time:", pElem, time_ms - pElem->stuff.u.enemy.time_next_decision);
    }
    
    return WORLD_ELEM_ID_INVALID;
}

void
game_ai_run()
{
    // TODO: problem here where placed objects may not be moving, but should get AI time
    WorldElemListNode* pCur = gWorld->elements_moving.next, *pNext;
    while(pCur)
    {
        pNext = pCur->next;
        WorldElem* pCurElem = pCur->elem;
        
        // TODO: periodically enemy should look for nearest target
        // and if gets beyond some threshold forgets it and just targets a new object
        if(pCurElem->stuff.intelligent
           /*(pCurElem->object_type == OBJ_SHIP || pCurElem->object_type == OBJ_TURRET || pCurElem->object_type == OBJ_MISSLE)*/
           /*&& pCurElem->elem_id >= 0*/)
        {
            float pv[6];
            int pi = 0;
            
            if(pCurElem->stuff.u.enemy.time_last_run == 0)
            {
                // brand new object
                pCurElem->stuff.u.enemy.time_last_run = pCurElem->stuff.u.enemy.time_next_decision = time_ms;
            }
            
            if(time_ms - pCurElem->stuff.u.enemy.time_last_run < pCurElem->stuff.u.enemy.time_run_interval)
            {
                goto game_ai_run_skip_ai;
            }
            
            /*
            if(pCurElem->stuff.u.enemy.collided != 0)
            {
                pCurElem->stuff.u.enemy.collided = 0;
                
                if(pCurElem->stuff.u.enemy.enemy_state != ENEMY_STATE_PULLUP)
                {
                    pCurElem->stuff.u.enemy.last_state = pCurElem->stuff.u.enemy.enemy_state;
                    pCurElem->stuff.u.enemy.enemy_state = ENEMY_STATE_PULLUP;
                    pCurElem->stuff.u.enemy.time_next_decision = time_ms + 1000;
                    
                    pCurElem->stuff.u.enemy.tgt_x = pCurElem->stuff.u.enemy.tgt_y = pCurElem->stuff.u.enemy.tgt_z = 0;
                    pCurElem->stuff.u.enemy.target_id = WORLD_ELEM_ID_INVALID;
                }
                
                if(GAME_AI_DEBUG)
                {
                    gameAudioPlaySoundAtLocation("bump", my_ship_x, my_ship_y, my_ship_z);
                }
                
                pCur = pNext;
                continue;
                
                pi = 1;
            }
             */
            
            //printf("%p: %d [%f, %f, %f]\n", pCurElem, pCurElem->stuff.u.enemy.enemy_state,
            //       pCurElem->stuff.u.enemy.tgt_x, pCurElem->stuff.u.enemy.tgt_y, pCurElem->stuff.u.enemy.tgt_z);
            
            // if towing, aim for base to drop off
            if(pCurElem->stuff.towed_elem_id != WORLD_ELEM_ID_INVALID)
            {
                WorldElem* pBaseElem = world_find_elem_with_attrs(&gWorld->elements_list,
                                                                  OBJ_SPAWNPOINT_ENEMY,
                                                                  pCurElem->stuff.affiliation);
                if(pBaseElem)
                {
                    pCurElem->stuff.u.enemy.tgt_x = pBaseElem->physics.ptr->x;
                    pCurElem->stuff.u.enemy.tgt_y = pBaseElem->physics.ptr->y;
                    pCurElem->stuff.u.enemy.tgt_z = pBaseElem->physics.ptr->z;
                    pCurElem->stuff.u.enemy.target_id = WORLD_ELEM_ID_INVALID;
                    pCurElem->stuff.u.enemy.enemy_state = ENEMY_STATE_PATROL;
                }
            }
            
            if(pCurElem->stuff.u.enemy.enemy_state == ENEMY_STATE_JUKE &&
               pCurElem->stuff.u.enemy.changes_target)
            {
                pv[pi++] = pCurElem->stuff.u.enemy.tgt_x;
                pv[pi++] = pCurElem->stuff.u.enemy.tgt_y;
                pv[pi++] = pCurElem->stuff.u.enemy.tgt_z;
                pv[pi++] = 0;
                pv[pi++] = 0;
                pv[pi++] = 0;
            }
            else if(pCurElem->stuff.u.enemy.target_id >= 0) // target, pursue
            {
                WorldElemListNode* pTargetListNode = world_elem_list_find(pCurElem->stuff.u.enemy.target_id,
                                                                          &gWorld->elements_list);
                if(pTargetListNode)
                {
                    pv[pi++] = pTargetListNode->elem->physics.ptr->x;
                    pv[pi++] = pTargetListNode->elem->physics.ptr->y;
                    pv[pi++] = pTargetListNode->elem->physics.ptr->z;
                    pv[pi++] = pTargetListNode->elem->physics.ptr->vx;
                    pv[pi++] = pTargetListNode->elem->physics.ptr->vy;
                    pv[pi++] = pTargetListNode->elem->physics.ptr->vz;
                }
                else
                {
                    pCurElem->stuff.u.enemy.target_id = WORLD_ELEM_ID_INVALID;
                }
            }
            else // no target
            {
                if(pCurElem->stuff.u.enemy.changes_target &&
                   time_ms > pCurElem->stuff.u.enemy.time_next_decision)
                {
                    pCurElem->stuff.u.enemy.time_next_decision = time_ms + 2000;
                    
                    if((pCurElem->stuff.u.enemy.target_id = game_ai_find_target(pCurElem)) != WORLD_ELEM_ID_INVALID)
                    {
                        // picked up new target
                    }
                }
                
                if(pCurElem->stuff.u.enemy.target_id == WORLD_ELEM_ID_INVALID)
                {
                    if(pCurElem->stuff.u.enemy.patrols_no_target_jukes)
                    {
                        // patrol randomly
                        if(pCurElem->stuff.u.enemy.tgt_x == 0 &&
                           pCurElem->stuff.u.enemy.tgt_y == 0 &&
                           pCurElem->stuff.u.enemy.tgt_z == 0)
                        {
                            int prev_state;
                            
                            // make this a random heading instead
                            /*
                            int patrol_retries = 10;
                            do {
                                pCurElem->stuff.u.enemy.tgt_x = rand_in_range(1, gWorld->bound_x);
                                pCurElem->stuff.u.enemy.tgt_y = rand_in_range(1, gWorld->bound_y);
                                pCurElem->stuff.u.enemy.tgt_z = rand_in_range(1, gWorld->bound_z);
                                patrol_retries--;
                            }
                            while(distance(pCurElem->physics.ptr->x, pCurElem->physics.ptr->y, pCurElem->physics.ptr->z,
                                           pCurElem->stuff.u.enemy.tgt_x, pCurElem->stuff.u.enemy.tgt_y, pCurElem->stuff.u.enemy.tgt_z) < min_patrol_distance &&
                                  patrol_retries > 0);
                             */
                            
                            /*
                            float rvec[3], rdist = 50;
                            random_heading_vector(rvec);
                            
                            pCurElem->stuff.u.enemy.tgt_x = pCurElem->physics.ptr->x + rvec[0] * rdist;
                            pCurElem->stuff.u.enemy.tgt_y = pCurElem->physics.ptr->y + rvec[1] * rdist;
                            pCurElem->stuff.u.enemy.tgt_z = pCurElem->physics.ptr->z + rvec[2] * rdist;
                            */
                            prev_state = pCurElem->stuff.u.enemy.enemy_state;
                            game_ai_juke(pCurElem, juke_distance_patrol);
                            pCurElem->stuff.u.enemy.enemy_state = prev_state;
                            
                            ai_debug("patrolling location changed:", pCurElem, 0);
                            
                            ai_debug("rand_e[0]:", pCurElem, pCurElem->stuff.u.enemy.tgt_x);
                            ai_debug("rand_e[1]:", pCurElem, pCurElem->stuff.u.enemy.tgt_y);
                            ai_debug("rand_e[2]:", pCurElem, pCurElem->stuff.u.enemy.tgt_z);
                            
                        }
                        else if(distance(pCurElem->physics.ptr->x,
                                         pCurElem->physics.ptr->y,
                                         pCurElem->physics.ptr->z,
                                         pCurElem->stuff.u.enemy.tgt_x,
                                         pCurElem->stuff.u.enemy.tgt_y,
                                         pCurElem->stuff.u.enemy.tgt_z) <= patrol_area_reached)
                        {
                            pCurElem->stuff.u.enemy.tgt_x = 0;
                            pCurElem->stuff.u.enemy.tgt_y = 0;
                            pCurElem->stuff.u.enemy.tgt_z = 0;
                        }
                        
                        pv[pi++] = pCurElem->stuff.u.enemy.tgt_x;
                        pv[pi++] = pCurElem->stuff.u.enemy.tgt_y;
                        pv[pi++] = pCurElem->stuff.u.enemy.tgt_z;
                        pv[pi++] = 0;
                        pv[pi++] = 0;
                        pv[pi++] = 0;
                    }
                }
            }
            
            if(pi > 0)
            {
                object_pursue(pv[0], pv[1], pv[2],
                              pv[3], pv[4], pv[5],
                              pCurElem);
            }
            
            if(GAME_AI_DEBUG)
            {
                char tag_debug[255];
                sprintf(tag_debug, "%d,%d", pCurElem->stuff.u.enemy.enemy_state, pCurElem->stuff.u.enemy.target_id);
                world_object_set_nametag(pCurElem->elem_id, tag_debug);
            }
            
            pCurElem->stuff.u.enemy.collided = 0;
            pCurElem->stuff.u.enemy.time_last_run = time_ms;
        }
        
    game_ai_run_skip_ai:
        
        pCur = pNext;
    }
}

void
object_pursue(float x, float y, float z, float vx, float vy, float vz, WorldElem *elem)
{
    int fireBullet = 0;
    int do_pitch = 1;
    int do_yaw = 1;
    int skill = elem->stuff.u.enemy.intelligence;
    quaternion_t xq, yq, zq;
    float tc;
    WorldElem* pTargetElem = NULL;
    Object targetElemType = OBJ_UNKNOWN;
    float zdot_ikillyou = 0.5;
    float zdot_juke = 0.3;
    float direction = 1.0;
    
    //if(time_ms - elem->stuff.u.enemy.time_last_run < elem->stuff.u.enemy.time_run_interval) return;
    
    tc = time_ms - elem->stuff.u.enemy.time_last_run;
    tc /= 1000.0;
    
    WorldElemListNode* pTargetNode = world_elem_list_find(elem->stuff.u.enemy.target_id, &gWorld->elements_list);
    if(pTargetNode)
    {
        pTargetElem = pTargetNode->elem;
        targetElemType = pTargetElem->object_type;
    }
    
    // AI speed
    float vcur = world_elem_get_velocity(elem);
    
    float ax = elem->physics.ptr->x - x;
    float ay = elem->physics.ptr->y - y;
    float az = elem->physics.ptr->z - z;

    float dist = sqrt(ax*ax + ay*ay + az*az);
    
    if(elem->stuff.towed_elem_id != WORLD_ELEM_ID_INVALID)
    {
    }
    else
    {
        // search for better target every 5 sec
        if(elem->stuff.u.enemy.changes_target &&
           time_ms - elem->stuff.u.enemy.time_target_acquired > change_target_frequency)
        {
            elem->stuff.u.enemy.target_id = WORLD_ELEM_ID_INVALID;
        }
    }
    
    // find current object heading vector
    get_body_vectors_for_euler(elem->physics.ptr->alpha, elem->physics.ptr->beta, elem->physics.ptr->gamma,
                               &xq, &yq, &zq);
    
    float zdot = zq.x*(ax/dist) + zq.y*(ay/dist) + zq.z*(az/dist);
    
    unsigned long prev_state = elem->stuff.u.enemy.enemy_state;
    
    ai_debug("object_pursue state:", elem, elem->stuff.u.enemy.enemy_state);
    
    if(elem->stuff.u.enemy.enemy_state == ENEMY_STATE_PURSUE)
    {
        // adjust target vector to intercept (with some fudge-factor)
        float fm = (1.0 + (1.0 / (float) rand_in_range(1, 100)));
        float leadVel = bulletVel;
        
        // if not firing, intercept course
        if(!elem->stuff.u.enemy.fires)
        {
            leadVel = vcur;
            fm = 1.0;
        }
        
        float leadv = (dist / leadVel) * fm;
        
        ax -= vx*leadv;
        ay -= vy*leadv;
        az -= vz*leadv;
        
        // if distance is too great, forget target
        // TODO: why qualify with fixed?
        //if(!elem->stuff.u.enemy.fixed)
        if(dist > elem->stuff.u.enemy.scan_distance && elem->stuff.u.enemy.changes_target)
        {
            elem->stuff.u.enemy.target_id = WORLD_ELEM_ID_INVALID;
            
            elem->stuff.u.enemy.tgt_x = elem->stuff.u.enemy.tgt_y = elem->stuff.u.enemy.tgt_z = 0;
            
            elem->stuff.u.enemy.enemy_state = ENEMY_STATE_PATROL;
            elem->stuff.u.enemy.scan_distance = 1;
        }
        
        if(elem->stuff.u.enemy.run_distance > 0 &&
           dist <= elem->stuff.u.enemy.run_distance /* - (run_distance*skill*diff_m) && zdot < 0 */ &&
           zdot <= 0.2 &&
           pTargetElem &&
           target_avoid_collision(pTargetElem->object_type))
        {
            elem->stuff.u.enemy.enemy_state = ENEMY_STATE_RUN;
            ax = -ax; ay = -ay; az = -vz;
        }
        
        if(time_ms >= elem->stuff.u.enemy.time_next_decision &&
           zdot < zdot_juke &&
           elem->stuff.u.enemy.patrols_no_target_jukes &&
           elem->stuff.u.enemy.enemy_state != ENEMY_STATE_JUKE)
        {
            if (rand_in_range(1, 100) <= game_variable_get("ENEMY1_JUKE_PCT"))
            {
                ai_debug("juking:", elem, 0);
                elem->stuff.u.enemy.last_state = elem->stuff.u.enemy.enemy_state;
                elem->stuff.u.enemy.time_next_decision = 3000;
                
                game_ai_juke(elem, juke_distance);
            }
        }
    }
    else if(elem->stuff.u.enemy.enemy_state == ENEMY_STATE_PATROL)
    {
        if(time_ms >= elem->stuff.u.enemy.time_next_decision)
        {
            if(elem->stuff.u.enemy.target_id >= 0)
            {
                ai_debug("PATROL->PURSUE:", elem, 0);
                elem->stuff.u.enemy.enemy_state = ENEMY_STATE_PURSUE;
            }
            
            // add unpredictable behavior by heading off on a tangent periodically for a short period
            if (rand_in_range(1, 100) <= 25)
            {
                elem->stuff.u.enemy.last_state = elem->stuff.u.enemy.enemy_state;
                
                elem->stuff.u.enemy.tgt_x = elem->stuff.u.enemy.tgt_x = elem->stuff.u.enemy.tgt_x = 0;
                
                ai_debug("PATROL->JUKE:", elem, 0);
                game_ai_juke(elem, juke_distance);
            }
        }
    }
    else if(elem->stuff.u.enemy.enemy_state == ENEMY_STATE_RUN)
    {
        // reverse vector, run away! Book it/flee/amscray
        ax = -ax; ay = -ay; az = -az;
        
        // add unpredictable behavior by heading off on a tangent periodically for a short period
        if(time_ms >= elem->stuff.u.enemy.time_next_decision &&
           /* zdot < zdot_juke && */
           elem->stuff.u.enemy.enemy_state != ENEMY_STATE_JUKE)
        {
            if (rand_in_range(1, 100) <= game_variable_get("ENEMY1_JUKE_PCT"))
            {
                elem->stuff.u.enemy.last_state = elem->stuff.u.enemy.enemy_state;
                
                game_ai_juke(elem, juke_distance);
            }
        }
        
        if(dist >= (elem->stuff.u.enemy.pursue_distance - (skill * elem->stuff.u.enemy.pursue_distance * diff_m))
           /* || time_ms - elem->stuff.u.enemy.time_last_decision > 2000 */
           || zdot <= 0)
        {
            if(rand_in_range(1, 100) <= 25)
            {
                elem->stuff.u.enemy.enemy_state = ENEMY_STATE_PURSUE;
            }
        }
    }
    else if(elem->stuff.u.enemy.enemy_state == ENEMY_STATE_JUKE)
    {
        if(time_ms > elem->stuff.u.enemy.time_next_decision)
        {
            ai_debug("JUKE->", elem, elem->stuff.u.enemy.last_state);
            
            elem->stuff.u.enemy.enemy_state = elem->stuff.u.enemy.last_state;
        }
    }
    else
    {
        assert(0);
    }
    
    // on state change, default to 1s between decision
    if(elem->stuff.u.enemy.enemy_state != prev_state && time_ms > elem->stuff.u.enemy.time_next_decision)
    {
        elem->stuff.u.enemy.time_next_decision = time_ms + 1000;
    }
    
    // rate at which enemy rotates
    float turn_r = (elem->stuff.u.enemy.max_turn / 4) * tc;
    
    // rate at which object accelerates
    float accel = 0;
    
    // HACK
    float vdesired = pursuit_speed_for_object(elem, &accel);
    
    if(vdesired < MAX_SPEED) ai_debug("vdesired:", elem, vdesired);
    
    // accel/decel
    float v = vcur > vdesired? 0.1: accel;
    
    dist = sqrt(ax*ax + ay*ay + az*az);
    
    float xdot = xq.x*(ax/dist) + xq.y*(ay/dist) + xq.z*(az/dist);
    float ydot = yq.x*(ax/dist) + yq.y*(ay/dist) + yq.z*(az/dist);
    
    if(elem->stuff.u.enemy.collided)
    {
        // force firing a bullet (in case of object-collision, run through)
        fireBullet = 1;
        dist = 0;
        zdot = zdot_ikillyou;
        vdesired = 1.0;
    }
    
    if(do_pitch)
    {
        float p = ydot < 0? turn_r: -turn_r;
        
        elem->stuff.u.enemy.rate_pitch += p;
        elem->stuff.u.enemy.rate_pitch *= enemy_turn_rate_dampc;
        
        quaternion_rotate_inplace(&yq, &xq, elem->stuff.u.enemy.rate_pitch);
        quaternion_rotate_inplace(&zq, &xq, elem->stuff.u.enemy.rate_pitch);
    }

    if(do_yaw)
    {
        float y = xdot > 0? turn_r: -turn_r;
        
        elem->stuff.u.enemy.rate_yaw = elem->stuff.u.enemy.rate_yaw + y;
        elem->stuff.u.enemy.rate_yaw *= enemy_turn_rate_dampc;
        
        quaternion_rotate_inplace(&xq, &yq, elem->stuff.u.enemy.rate_yaw);
        quaternion_rotate_inplace(&zq, &yq, elem->stuff.u.enemy.rate_yaw);
    }
    
    // convert new heading vector back
    /*
    float alpha = atan2(zq.x, -zq.y);
	float beta = acos(zq.z);
    float gamma = atan2(zq.z, yq.z);
     */
    
    float alpha, beta, gamma;
    get_euler_from_body_vectors(&xq, &yq, &zq, &alpha, &beta, &gamma);
    
    world_replace_object(elem->elem_id, elem->type,
                         elem->physics.ptr->x, elem->physics.ptr->y, elem->physics.ptr->z,
                         alpha, beta, gamma,
                         elem->scale,
                         elem->texture_id);
    
    if(!elem->stuff.u.enemy.fixed)
    {
        float tv[3] = {
            -zq.x * v,
            -zq.y * v,
            -zq.z * v
        };
        
        update_object_velocity(elem->elem_id, tv[0], tv[1], tv[2], direction * 1.0);
    }
    else
    {
        // TODO: allow for TANKS by having a turret that only moves on 2 axes
        //update_object_velocity(elem->elem_id, 0, 0, 0, 0);
    }
    
    if(elem->stuff.u.enemy.enemy_state == ENEMY_STATE_PURSUE &&
       elem->stuff.u.enemy.fires &&
       (collision_actions[OBJ_BULLET][targetElemType] == COLLISION_ACTION_DAMAGE ||
        collision_actions[OBJ_MISSLE][targetElemType] == COLLISION_ACTION_DAMAGE) &&
       zdot >= zdot_ikillyou)
    {
        fireBullet = 1;
    }
    
    // fire a bullet
    assert(skill > 0.0);
    
    if(fireBullet &&
       time_ms - elem->stuff.u.enemy.time_last_bullet > (game_ai_fire_rate_ms-(game_ai_fire_rate_ms*(diff_m/skill))))
    {
        elem->stuff.u.enemy.collided = 0;
        
        elem->stuff.u.enemy.time_last_bullet = time_ms;
        
        float bv = 2 * elem->scale;
        float bulletPos[3] = {
            elem->physics.ptr->x + -zq.x*bv,
            elem->physics.ptr->y + -zq.y*bv,
            elem->physics.ptr->z + -zq.z*bv
        };
        float bulletEuler[3] = {
            elem->physics.ptr->alpha,
            elem->physics.ptr->beta,
            elem->physics.ptr->gamma,
        };
        
        if(elem->stuff.u.enemy.fires_missles)
        {
            elem->stuff.u.enemy.time_last_bullet += 3000; // fire missles less frequently
            
            game_add_bullet(bulletPos, bulletEuler, bulletVel, bv, 0, elem->stuff.u.enemy.target_id, elem->stuff.affiliation);
            
        }
        else
        {
            game_add_bullet(bulletPos, bulletEuler, bulletVel, bv, 0, -1, elem->stuff.affiliation);
        }
    }
    
    // leave trail
    if(elem->stuff.u.enemy.leaves_trail &&
       time_ms - elem->stuff.u.enemy.time_last_trail >= pooped_cube_interval_ms)
    {
        firePoopedCube(elem);
        elem->stuff.u.enemy.time_last_trail = time_ms;
    }
}

void
game_ai_collision(WorldElem* elemA, WorldElem* elemB, int collision_action)
{
    WorldElem* elemAI = NULL;
    WorldElem* elemC = NULL;
    
    if(elemA && elemA->object_type == OBJ_SHIP)
    {
        elemAI = elemA;
        elemC = elemB;
    }
    
    if(elemB && elemB->object_type == OBJ_SHIP)
    {
        elemAI = elemB;
        elemC = elemA;
    }
    
    if(elemAI && !elemC)
    {
        if(elemAI->stuff.u.enemy.enemy_state != ENEMY_STATE_JUKE)
        {
            elemAI->stuff.u.enemy.last_state = elemAI->stuff.u.enemy.enemy_state;
            game_ai_juke(elemAI, juke_distance);
            
            elemAI->stuff.u.enemy.target_id = WORLD_ELEM_ID_INVALID;
            elemAI->stuff.u.enemy.time_next_decision = time_ms + 2000;
            
            update_object_velocity(elemAI->elem_id, 0, 0, 0, 0);
            
            ai_debug("juke target set: ", elemAI, elemAI->physics.ptr->x - elemAI->stuff.u.enemy.tgt_x);
            ai_debug("juke target set: ", elemAI, elemAI->physics.ptr->y - elemAI->stuff.u.enemy.tgt_y);
            ai_debug("juke target set: ", elemAI, elemAI->physics.ptr->z - elemAI->stuff.u.enemy.tgt_z);
        }
        
        return;
    }
    
    if(!elemAI || !elemC) return;
    
    // ignore towing-collisions
    if(elemAI->stuff.towed_elem_id == elemC->elem_id) return;
    
    elemAI->stuff.u.enemy.collided = ENEMY_COLLIDE_OBJECT;
    
    elemAI->stuff.u.enemy.time_last_trail = time_ms + pooped_cube_interval_ms;
    
    elemAI->stuff.u.enemy.target_id = WORLD_ELEM_ID_INVALID;
    
    // drop tow
    elemAI->stuff.towed_elem_id = WORLD_ELEM_ID_INVALID;
    
    if(elemAI->stuff.u.enemy.target_id == WORLD_ELEM_ID_INVALID &&
       elemC->stuff.affiliation != 0)
    {
        elemAI->stuff.u.enemy.target_id = elemC->stuff.affiliation;
        ai_debug("enemy targeting affiliation:%d", elemAI, elemC->stuff.affiliation);
    }
    
    if(elemAI && elemC->physics.ptr->velocity == 0)
    {
        // head away from collision
        if(elemAI->stuff.u.enemy.enemy_state != ENEMY_STATE_JUKE)
        {
            elemAI->stuff.u.enemy.last_state = elemAI->stuff.u.enemy.enemy_state;
            
            float cPos[3] = {elemC->physics.ptr->x, elemC->physics.ptr->y, elemC->physics.ptr->z};
            float aiPos[3] = {elemAI->physics.ptr->x, elemAI->physics.ptr->y, elemAI->physics.ptr->z};
            float uv[3];
            float l = 20;
            
            unit_vector_ab(cPos, aiPos, uv);
            
            elemAI->stuff.u.enemy.tgt_x = uv[0] * l;
            elemAI->stuff.u.enemy.tgt_y = uv[1] * l;
            elemAI->stuff.u.enemy.tgt_z = uv[2] * l;
            
            elemAI->stuff.u.enemy.target_id = WORLD_ELEM_ID_INVALID;
            elemAI->stuff.u.enemy.enemy_state = ENEMY_STATE_PATROL;
            elemAI->stuff.u.enemy.time_next_decision = time_ms + 2000;
        }
    }
}

static float
pursuit_speed_for_object(WorldElem* elem, float* accel)
{
    float v = 0;
    float s = 1.0;
    
    if(elem->stuff.towed_elem_id != WORLD_ELEM_ID_INVALID) s = tow_v_scale;
    
    switch(elem->object_type)
    {
        case OBJ_MISSLE:
            v = elem->stuff.u.enemy.max_speed;
            s = 2.0;
            *accel = 3;
            break;
            
        default:
            v = (elem->stuff.u.enemy.enemy_state == ENEMY_STATE_RUN ||
                 elem->stuff.u.enemy.enemy_state == ENEMY_STATE_PATROL ||
                 elem->stuff.u.enemy.enemy_state == ENEMY_STATE_JUKE) ?
                    // not pursuing
                 elem->stuff.u.enemy.max_speed:
                    // pursue
                 elem->stuff.u.enemy.max_speed;
            *accel = 2;
            break;
    }
    
    return v*s;
}
