//
//  gameMotionInterp.c
//  gl_flight
//
//  Created by Justin Brady on 5/17/19.
//

#include <stdio.h>
#include "gameMotionInterp.h"
#include "world.h"

const static float velo_interp_update_mult = 4;
extern unsigned long update_frequency_ms;

gameNetworkError
gameNetwork_updatePlayerObject(gameNetworkPlayerInfo* playerInfo,
                               float x, float y, float z,
                               float alpha, float beta, float gamma,
                               float velx, float vely, float velz);

void
motion_interpolate_euler(game_timeval_t network_time_ms, motion_interp_st* motion, gameNetworkPlayerInfo* pInfo, WorldElem* pElem)
{
    float euler[3];
    int i;
    for(i = 0; i < 3; i++)
    {
        euler[i] = ((motion->euler_last[i][0] - motion->euler_last[i][1]) /
                    (motion->timestamp_last[0] - motion->timestamp_last[1])) *
        (network_time_ms - motion->euler_last_interp);
    }
    
    if(pElem)
    {
        if(fabs(euler[0]) < euler_interp_range_max &&
           fabs(euler[1]) < euler_interp_range_max &&
           fabs(euler[2]) < euler_interp_range_max)
        {
            gameNetwork_updatePlayerObject(pInfo,
                                           pElem->physics.ptr->x, pElem->physics.ptr->y, pElem->physics.ptr->z,
                                           pElem->physics.ptr->alpha + euler[0],
                                           pElem->physics.ptr->beta + euler[1],
                                           pElem->physics.ptr->gamma + euler[2],
                                           pElem->physics.ptr->vx,
                                           pElem->physics.ptr->vy,
                                           pElem->physics.ptr->vz);
        }
        motion->euler_last_interp = network_time_ms;
    }
}

void
motion_interpolate_velocity(game_timeval_t network_time_ms, motion_interp_st* motion, WorldElem* pElem, gameNetworkMessage* msg, int interp_velo)
{
    game_timeval_t t = msg->params.f[16];
    
    // adjust timing depending up or down depending on which is closer to new position
    float plottedMore[3], plottedLess[3], plottedCompare[3];
    for(int d = 0; d < 3; d++)
    {
        plottedMore[d] = predict_a1_at_b1_for_a_over_b(motion->loc_last[d], motion->timestamp_last, t+motion->timestamp_adjust+1);
        plottedLess[d] = predict_a1_at_b1_for_a_over_b(motion->loc_last[d], motion->timestamp_last, t+motion->timestamp_adjust-1);
        plottedCompare[d] = msg->params.f[1+d];
    }
    
    // log location
    for(int i = 2; i > 0; i--)
    {
        for(int d = 0; d < 3; d++)
        {
            motion->loc_last[d][i] = motion->loc_last[d][i-1];
            motion->euler_last[d][i] = motion->euler_last[d][i-1];
            
            if(!interp_velo)
            {
                motion->loc_last[d][i] = msg->params.f[1+d];
                motion->euler_last[d][i] = msg->params.f[4+d];
            }
        }
        motion->timestamp_last[i] = motion->timestamp_last[i-1];
        
        if(!interp_velo)
        {
            motion->timestamp_last[i] = 0;
        }
    }
    motion->loc_last[0][0] = msg->params.f[1];
    motion->loc_last[1][0] = msg->params.f[2];
    motion->loc_last[2][0] = msg->params.f[3];
    motion->euler_last[0][0] = msg->params.f[4];
    motion->euler_last[1][0] = msg->params.f[5];
    motion->euler_last[2][0] = msg->params.f[6];
    
    motion->timestamp_last[0] = t;
    
    float v[3] = {
        pElem->physics.ptr->x,
        pElem->physics.ptr->y,
        pElem->physics.ptr->z
    };
    
    float time_win = motion->timestamp_last[0] - motion->timestamp_last[2];
    if(interp_velo && time_win < update_frequency_ms * velo_interp_update_mult && time_win > 0)
    {
        // predict velocity
        float v1[3];
        for(int d = 0; d < 3; d++)
        {
            v1[d] = predict_a1_at_b1_for_a_over_b(motion->loc_last[d],
                                                  motion->timestamp_last,
                                                  t + 1000
                                                  /*+ playerInfo->timestamp_adjust*/);
            v[d] = v1[d] - v[d];
        }
        
        // TODO: calculate timestamp compensation (and use it)
        if(distance(plottedLess[0], plottedLess[1], plottedLess[2], plottedCompare[0], plottedCompare[1], plottedCompare[2]) <
           distance(plottedMore[0], plottedMore[1], plottedMore[2], plottedCompare[0], plottedCompare[1], plottedCompare[2]))
        {
            if(motion->timestamp_adjust > update_frequency_ms*2) motion->timestamp_adjust--;
        }
        else
        {
            if(motion->timestamp_adjust < update_frequency_ms*2) motion->timestamp_adjust++;
        }
        
        /*
        gameNetwork_updatePlayerObject(pInfo,
                                       pElem->physics.ptr->x, pElem->physics.ptr->y, pElem->physics.ptr->z,
                                       msg->params.f[4], msg->params.f[5], msg->params.f[6],
                                       v[0], v[1], v[2]);
         */
        
        world_replace_object(pElem->elem_id, pElem->type,
                             pElem->physics.ptr->x, pElem->physics.ptr->y, pElem->physics.ptr->z,
                             msg->params.f[4], msg->params.f[5], msg->params.f[6],
                             pElem->scale, pElem->texture_id);
        update_object_velocity(pElem->elem_id, v[0], v[1], v[2], 0);
    }
    else
    {
        v[0] = pElem->physics.ptr->vx;
        v[1] = pElem->physics.ptr->vy;
        v[2] = pElem->physics.ptr->vz;
        
        /*
        gameNetwork_updatePlayerObject(pInfo,
                                       msg->params.f[1], msg->params.f[2], msg->params.f[3],
                                       msg->params.f[4], msg->params.f[5], msg->params.f[6],
                                       v[0], v[1], v[2]);
         */
        world_replace_object(pElem->elem_id, pElem->type,
                             msg->params.f[1], msg->params.f[2], msg->params.f[3],
                             msg->params.f[4], msg->params.f[5], msg->params.f[6],
                             pElem->scale, pElem->texture_id);
        update_object_velocity(pElem->elem_id, v[0], v[1], v[2], 0);
    }
}
