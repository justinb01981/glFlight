//
//  gameInput.c
//  gl_flight
//
//  Created by Justin Brady on 7/2/14.
//
//

#include <stdio.h>


//
//  moved from gameInput.m
//  gl_flight
//
//  Created by Justin Brady on 2/1/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

// TODO: google yaw, pitch, roll rotation matrix
// and an input of roll results in changing the camera y,p,r relative to it's current orientation
// see euler angles
// http://mathworld.wolfram.com/EulerAngles.html
//
// 3-21-2012:
// after many blind alleys.. I have something close to working.. but the drift is just horrible
// ... you can't move on one axis for more than a few hundred frames before you are way off
// so I'm thinking that I can pre-compute positions and interpolate between them
// ...googling doesn't fill me with a lot of confidence..
//
// 5-11-2012:
// Now im using 3 vectors to represent our axes, and rotating 2 (using quaternion_rotate)
// about the axis we want to rotate.. but this is prone to drift, so you have to reset the
// vectors to be perpendicular to each other every so often. Got that working.
// One thing that really fucked me hard forever was the difference between euler and tait-bryan
// angles.. since I'm getting my information from multiple sources, I wasn't realizing there
// was such a difference in the formulas.. euler rotations re-use the already rotated axes
// NOTE: 'yaw' 'pitch' and 'roll' are misleading terms usually associated with tait-bryan
// right now I still have the body-quaternion 'b', even though I'm not using it, I'm
// deriving the camera rotations via geometry from the 3 vectors.. bettr way would be to just
// conver the quaternion 'b' to euler, but haven't bothered debugging that yet.
//

#include <math.h>
#include "quaternions.h"
#include "gameInterface.h"
#include "gameCamera.h"
#include "gameShip.h"
#include "gameUtils.h"
#include "gameDialogs.h"
#include "gameGlobals.h"

#define AVG_INPUTS_LENGTH 5


double rm[3][3];
int rm_count = 0;
float sp = 0;
int sched_input_action[] = {3,1};
int sched_input_val = 1;
double dz_roll, dz_pitch, dz_yaw;
double speed;
double maxAccelDecel;
double targetSpeed;
double maxSpeed;
double minSpeed;
double speedBoost = 0;
double bulletVel;
int needTrimLast;
int needTrim;
int isLandscape;
int controlsCalibrated = 0;
quaternion_t b;
float rspin = 1;
double yaw = 0;
double pitch = 0;
double roll = 0;
float inputAvg[3][AVG_INPUTS_LENGTH];
int inputAvg_i = 0;
int initialized = 0;
double motionRoll, motionPitch, motionYaw;
double roll_m, pitch_m, yaw_m;
int trimCount = 0;
int trimCountLast = 0;
unsigned int gyroInputCount = 0;

void
gameInputInit()
{
    initialized = 0;
    
    // TODO: move these to some gameGlobals file
    // how large is our control "dead zone"
    //dz_roll = /*0.005*/ 0.1;
    //dz_pitch = /*0.005*/ 0.1;
    //dz_yaw = /*0.005*/ 0.1;
    // movement speed
    speed = minSpeed;
    targetSpeed = speed;
    maxSpeed = MAX_SPEED;
    maxAccelDecel = /*5*/ 20; // change per second
    minSpeed = 0;
    bulletVel = 40;
    
    isLandscape = GAME_PLATFORM_IS_LANDSCAPE;
}

void
gameInputTrimBegin()
{
    needTrim = 1;
    trimCount++;
}

int
gameInputTrimPending()
{
    return trimCount != trimCountLast;
}

void
gameInputGyro(float roll, float pitch, float yaw)
{
    gyroInputCount++;
    motionRoll = roll;
    motionPitch = pitch;
    motionYaw = yaw;
    
    trimCountLast = trimCount;
}

void
gameInputGyro2(float roll, float pitch, float yaw, float c)
{
    gyroInputCount++;
    if(needTrim)
    {
        motionRoll = roll;
        motionPitch = pitch;
        motionYaw = yaw;
    }
    else
    {
        motionRoll += roll * c;
        motionPitch += pitch * c;
        motionYaw += yaw * c;
    }

    trimCountLast = trimCount;
}

void
gameInput()
{
	double pi = M_PI;
    float tex_pass_initial_sample = 30;
    static float deviceLast[3];
    static unsigned long gyroInputCountLast = 0;
    
    /* wait for sensor to calibrate */
    if(gameInputTrimPending()) return;
    
    if(gyroInputCount == gyroInputCountLast) return;
    gyroInputCountLast = gyroInputCount;
	
	if(!initialized)
	{
        euler_to_quaternion(pi, 0.1, 0.1, &b.w, &b.x, &b.y, &b.z);
		
		initialized = 1;
	}
    
    double devicePitch;
    double deviceYaw;
    double deviceRoll;
    
#if GAME_PLATFORM_IOS
    devicePitch = motionRoll;
    deviceYaw = -motionPitch;
    deviceRoll = -motionYaw;
    
    float dz_m[3] = {1.01, 1.01, 1.01}; // deadzone-multiplier
#else
    devicePitch = -motionPitch;
    deviceYaw = motionYaw;
    deviceRoll = motionRoll;
    
    float dz_m[3] = {1.00, 1.00, 1.00}; // deadzone-multiplier
#endif
    
    static double trimStart[3];
    static double trim_dz[3];
    static int trimStartTime;
    
    /*
     if(tex_pass == 0)
     {
     needTrim = false;
     needTrimLast = true;
     }
     if(tex_pass == tex_pass_initial_sample)
     {
     needTrimLast = true;
     }
     */
    
    if(gameDialogState.controlsResuming)
    {
        gameDialogState.controlsResuming = 0;
        
        needTrim = 0;
        needTrimLast = 1;
        
        trimStartTime = tex_pass - 1;
    }
    
    if(!controlsCalibrated)
    {
        if(!needTrim && !needTrimLast)
        {
            console_clear();
            console_write("Hold trim button\nto calibrate controls\n"
                          "---------------------------------------------------->\n");
            gameInterfaceSetInterfaceState(INTERFACE_STATE_TRIM_BLINKING);
            return;
        }
    }
    
    // indicate to user when sampling
    if(needTrimLast)
    {
        int samp_min = 10;
        
        if(tex_pass - trimStartTime == samp_min)
        {
            console_append("\ncalibrating:");
        }
        else if(tex_pass - trimStartTime < tex_pass_initial_sample &&
                tex_pass - trimStartTime > samp_min)
        {
            console_append(".");
        }
        else if(tex_pass - trimStartTime == tex_pass_initial_sample)
        {
            console_append(" done");
        }
    }
    
    if(!needTrim && needTrimLast)
    {
        // short trim-length means few input values to calculate range, ignore
        if(tex_pass - trimStartTime >= tex_pass_initial_sample)
        {
            // TODO: could maybe store these in settings so they didnt have to calibrate every time
            // ... or just tell the user "hold still" and sample on startup...
            // best way would be to observe the values and wait for them to stabilize and use that...
            dz_roll = trim_dz[0];
            dz_pitch = trim_dz[1];
            dz_yaw = trim_dz[2];
            
            if(!controlsCalibrated)
            {
                console_clear();
                console_write("Welcome to d0gf1ght %s\n    tap to re_center\n", GAME_VERSION_STR);
                gameDialogWelcome();
            }
            else
            {
                console_write("\n");
            }
            
            controlsCalibrated = 1;
        }
        else
        {
            console_write("\n");
        }
        
        // zero out input-averaging buffer
        for(int a = 0; a < AVG_INPUTS_LENGTH; a++)
        {
            inputAvg[0][a] = deviceRoll;
            inputAvg[1][a] = devicePitch;
            inputAvg[2][a] = deviceYaw;
        }
    }
    
    if(needTrim)
    {
        if(!needTrimLast)
        {
            // begin learning trim
            trimStart[0] = deviceRoll;
            trimStart[1] = devicePitch;
            trimStart[2] = deviceYaw;
            trim_dz[0] = trim_dz[1] = trim_dz[2] = 0;
            trimStartTime = tex_pass;
        }
        else
        {
            double range;
            
            range = fabs(deviceRoll - trimStart[0]);
            if(range > trim_dz[0]) trim_dz[0] = range;
            range = fabs(devicePitch - trimStart[1]);
            if(range > trim_dz[1]) trim_dz[1] = range;
            range = fabs(deviceYaw - trimStart[2]);
            if(range > trim_dz[2]) trim_dz[2] = range;
        }
        
        yaw_m = deviceYaw;
        pitch_m = devicePitch;
        roll_m = deviceRoll;
        
        // uncomment this to cause holding trim button to not continue reading input
        //needTrim = false;
    }
    needTrimLast = needTrim;
    
    if(needTrim) return;
    
    // average values between inputs
    inputAvg[0][inputAvg_i] = deviceRoll;
    inputAvg[1][inputAvg_i] = devicePitch;
    inputAvg[2][inputAvg_i] = deviceYaw;
    
    deviceRoll = devicePitch = deviceYaw = 0;
    for(int i = 0; i < AVG_INPUTS_LENGTH; i++)
    {
        deviceRoll += inputAvg[0][i];
        devicePitch += inputAvg[1][i];
        deviceYaw += inputAvg[2][i];
    }
    deviceRoll /= AVG_INPUTS_LENGTH;
    devicePitch /= AVG_INPUTS_LENGTH;
    deviceYaw /= AVG_INPUTS_LENGTH;
    
    // make relative to trimmed/center position
    deviceRoll -= roll_m;
    devicePitch -= pitch_m;
    deviceYaw -= yaw_m;
    
    inputAvg_i++;
    if(inputAvg_i >= AVG_INPUTS_LENGTH) inputAvg_i = 0;
        
        // read device orientation/inputs
        //double input_roll = deviceRoll - roll_m;
        //double input_pitch = devicePitch - pitch_m; // pitch is inverted
        //double input_yaw = deviceYaw - yaw_m;
        double input_roll = deviceRoll;
        double input_pitch = devicePitch;
        double input_yaw = deviceYaw;
        
        // periodically do some stuff
        if(rm_count <= 0)
        {
            rm_count = 60;
            
            // renormalize body vectors every so often
            gameCamera_normalize();
            gameShip_normalize();
        }
	rm_count--;
    
    static float time_input_last = 0;
    float tc = time_ms - time_input_last;
    time_input_last = time_ms;
    
    if(!needTrim)
    {
        float driftComp[3] = {
            /*GYRO_FEEDBACK_COEFF*/(trim_dz[0]/(GAME_FRAME_RATE)) * 2.0,
            /*GYRO_FEEDBACK_COEFF*/(trim_dz[1]/(GAME_FRAME_RATE)) * 2.0,
            /*GYRO_FEEDBACK_COEFF*/(trim_dz[2]/(GAME_FRAME_RATE)) * 2.0
        };
        float fcr = driftComp[0] * (deviceRoll-deviceLast[0]) /*dz_roll*GYRO_FEEDBACK*/;
        float fcp = driftComp[1] * (devicePitch-deviceLast[1]) /*dz_pitch*GYRO_FEEDBACK*/;
        float fcy = driftComp[2] * (deviceYaw-deviceLast[2]) /*dz_yaw*GYRO_FEEDBACK*/;
        
        float cap = /*0.05*/ 0.1; // radians
        float sm[] = /*{0.6, 0.8, 0.8}*/ {0.7, 0.9, 0.6};
        
        if(fabs(input_roll) > dz_roll*dz_m[0] /*&& touchThrottle*/)
        {
            //printf("-----------ROLLING-----------\n");
            
            double s = input_roll * fabs(input_roll*sm[0]) * GYRO_DC * tc;
            
            if(fabs(s) > cap) s = cap * (s/fabs(s));
            
            quaternion_rotate_inplace(&b, &cam_pos.bz, s);
            //gameCamera_rollRadians(s);
            gameShip_roll(s);
        }
        //else
        {
            // feedback to avoid drift
            roll_m += /*input_roll*/ deviceRoll * fcr;
        }
        
        if(fabs(input_pitch) > dz_pitch*dz_m[1])
        {
            //printf("-----------PITCHING-----------\n");
            
            double s = input_pitch * fabs(input_pitch*sm[1]) * GYRO_DC * tc;
            
            if(fabs(s) > cap) s = cap * (s/fabs(s));
            
            quaternion_rotate_inplace(&b, &cam_pos.bx, s);
            //gameCamera_pitchRadians(s);
            gameShip_pitch(s);
        }
        //else
        {
            // feedback to avoid drift
            pitch_m += /*input_pitch*/ devicePitch * fcp;
        }
        
        if(fabs(input_yaw) > dz_yaw*dz_m[2])
        {
            //printf("-----------YAWING-----------\n");
            
            double s = input_yaw * fabs(input_yaw*sm[2]) * GYRO_DC * tc;
            
            if(fabs(s) > cap) s = cap * (s/fabs(s));
            
            quaternion_rotate_inplace(&b, &cam_pos.by, s);
            //gameCamera_yawRadians(s);
            gameShip_yaw(s);
        }
        //else
        {
            // feedback to avoid drift
            yaw_m += /*input_yaw*/ deviceYaw * fcy;
        }
    }
    
    deviceLast[0] = deviceRoll;
    deviceLast[1] = devicePitch;
    deviceLast[2] = deviceYaw;
    
    float cap_radians = M_PI/2;
    if(fabs(roll_m) > cap_radians) /*roll_m -= roll_m/fabs(roll_m)*0.1*/ roll_m = 0;
    if(fabs(pitch_m) > cap_radians) /*pitch_m -= pitch_m/fabs(pitch_m)*0.1*/ pitch_m = 0;
    if(fabs(yaw_m) > cap_radians) /*yaw_m -= yaw_m/fabs(yaw_m)*0.1*/ yaw_m = 0;
                
    // convert body axes to world-axes
    // http://en.wikipedia.org/wiki/Euler_angles
    /*
     yaw = atan2(bz.x, -bz.y);
     pitch = acos(bz.z); // asin(bz.z) // this appeared to work when using tait-bryan angles
     roll = atan2(bx.z, by.z);
     */
}

