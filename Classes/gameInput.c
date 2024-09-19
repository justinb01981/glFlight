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
#include <limits.h>
#include "quaternions.h"
#include "gameInterface.h"
#include "gameCamera.h"
#include "gameShip.h"
#include "gameUtils.h"
#include "gameDialogs.h"
#include "gameGlobals.h"

#define AVG_INPUTS_LENGTH 5

void trimDoneIgnore(void);

int rm_count = 1;

float dz_min = 0.001;
double speed = 0;
double maxAccelDecel = 0;
double targetSpeed = 0;
double maxSpeed = MAX_SPEED;

double speedBoost = 0;
double bulletVel;
int needTrim = 1;
int initialized = 0;
double motionRoll, motionPitch, motionYaw;
double devicePitch, deviceYaw, deviceRoll;

double motionRollMotion, motionPitchMotion, motionYawMotion;
#define maxInputShipRotate 0.2
float yprResponse[3] = { 0, 0, 0 };
float yprD = (maxInputShipRotate / 360.0);
float rollOffset = 0, pitchOffset = 0, yawOffset = 0;
double devicePitchFrac = 0, deviceYawFrac = 0;  // used for controls rendering in gameGraphics
float gyroSenseScale = PLATFORM_GYRO_SENSE_SCALE;
double gyroLastRange[3];
void (*trimDoneCallback)(void);

const float tex_pass_initial_sample = 120;

static double trim_dz[3];

struct
{
    float* buf, *dest;
    float storage[4096];
    unsigned long samples, x;
    float min[3];
    float max[3];
    float avg[3];
} gameInputStats;

int gameInputStatsInited = 0;

void
gameInputInit()
{
    // TODO: move these to some gameGlobals file
    // how large is our control "dead zone"
    //dz_roll = /*0.005*/ 0.1;
    //dz_pitch = /*0.005*/ 0.1;
    //dz_yaw = /*0.005*/ 0.1;
    // movement speed
    assert(minSpeed > 0);

    speed = minSpeed; // 
    targetSpeed = speed;
    maxAccelDecel = /*5*/ MAX_SPEED/3; // change per second
    bulletVel = MAX_SPEED*3;

    needTrim = 1;

//    controlsCalibrated = 0;
    
    initialized = 1;

    //    needTrim = 1;
    gameInputTrimBegin(trimDoneIgnore);
}

void
gameInputUninit()
{
    gameInputInit();
    initialized = 0;
    speed = 0;
}

void
gameInputTrimBegin(void (*callback)(void))
{
    trimDoneCallback = callback;
    needTrim = 1;
}

void
gameInputTrimEnd(void) {
    needTrim = 0;
}

int
gameInputInitialTrimPending()
{
    return 0;
}

// values in range -M_PI - M_PI
void
gameInputGyro(float roll, float pitch, float yaw)
{
    motionRoll = roll;
    motionPitch = pitch;
    motionYaw = yaw;
}

void
gameInputGyro2(float roll, float pitch, float yaw)
{
    if(needTrim)
    {
        motionRoll = roll;
        motionPitch = pitch;
        motionYaw = yaw;
    }
    else
    {
        motionRoll += roll;
        motionPitch += pitch;
        motionYaw += yaw;
    }
}

void
gameInputMotion(float roll, float pitch, float yaw)
{
    motionRollMotion = roll;
    motionPitchMotion = pitch;
    motionYawMotion = yaw;
}

void
gyro_calibrate_log(float pct)
{
    char buf[1024];
    
    console_clear();

    sprintf(buf, "calibrating: ");
    char *p = buf + strlen(buf);
    
    int s = 0;
    float width = (24.0 * pct) / 100;
    for(s = 0; s < width && s <= 24; s++)
    {
        *p++ = '^';
        *p++ = 'I';
    }
    while(s < 24)
    {
        *p++ = '^';
        *p++ = 'J';
        s++;
    }
    
    *p++ = '\0';
    console_write(buf);
    console_flush();
}

void
gameInput()
{
    static float deviceLast[3];
    
#if GAME_PLATFORM_IOS
    devicePitch = motionRoll;
    deviceYaw = -motionPitch;
    deviceRoll = -motionYaw;
#else
    devicePitch = -motionPitch;
    deviceYaw = motionYaw;
    deviceRoll = motionRoll;
#endif

    if(!initialized)
    {
        gameInputInit(); // lazy init self
    }

//
//    float deviceO[3] = {deviceRoll, devicePitch, deviceYaw};
//    gameInputStatsAppend(deviceO);
//
    if(needTrim)
    {
        rollOffset = deviceRoll;
        pitchOffset = devicePitch;
        yawOffset = deviceYaw;

        //gameInterfaceSetInterfaceState(INTERFACE_STATE_CLOSE_MENU);   // NOT HERE - in gameDialogs

//        if(!controlsCalibrated)
//        {
//            controlsCalibrated = 1;
//            if(trimDoneCallback) trimDoneCallback();
//        }

        // HACK: moving trim callback to gameInterface button handling

    }

    if(needTrim && trimDoneCallback)
    {  // falling-edge of trim btn pushf(trimDoneCallback)
        trimDoneCallback();
        trimDoneCallback = NULL;
        needTrim = 0;
    }

    /*
                         (Rmax-Rmin)
            Dr - Rmin + ------------
                            2
       R =   -----------------------
          gyroSenseScale * (Rmax-Rmin)
     
     */
    const float R = M_PI*2;
    float input_roll = (deviceRoll - rollOffset) / R;
    float input_pitch = (devicePitch - pitchOffset) / R;
    float input_yaw = (deviceYaw - yawOffset) / R;
    
    devicePitchFrac = devicePitch / M_PI;
    deviceYawFrac =  deviceYaw / M_PI;
    
    // periodically do some stuff
    if(rm_count <= 0)
    {
        rm_count = 33;
        
        // renormalize body vectors every so often
        gameCamera_normalize();
        gameShip_normalize();
    }
	rm_count--;
    
    static float time_input_last = 0;
    float tc = time_ms_wall - time_input_last;
    time_input_last = time_ms_wall;
    
    if(!game_paused)
    {
//        if(!gameSettingsComplexControls)
//        {
//            input_roll = input_yaw * -3.0;
//
//            gameShip_unfakeRoll();
//        }
//
//        if(!gameSettingsComplexControls)
//        {
//            float angleGround = gameShip_calcRoll() - M_PI;
//
//            if(fabs(angleGround) > 0.02)
//            {
//                float shipy[3];
//                gameShip_getYVector(shipy);
//
//                float r = 0.01;
//
//                if(angleGround > 0)
//                {
//                    r = -r;
//                }
//                else
//                {
//                    r = r;
//                }
//
//                if(shipy[1] < 0) r *= 4;
//
//                gameShip_roll(r);
//            }
//        }
        float INPUT_SPEED = (  1.0 + log(targetSpeed)  ) / maxSpeed; // targetSpeed - not effective-speed/momentum
        // TODO: prevent  < 0.
        
        if(fabs(input_roll) > dz_min
//           && gameSettingsComplexControls
           )
        {
//            printf("-----------ROLLING-----------\n");
            
            //double s = input_roll * fabs(input_roll) * GYRO_DC * tc * (0.01 + speed/MAX_SPEED); // "roll dominant" multiplier
            float s = input_roll * INPUT_SPEED;

            if(fabs(yprResponse[0]) < maxInputShipRotate) yprResponse[0] += yprD /* * (s/fabs(s)) */;
            //s = s / (1.0 - yprResponse[0]);
            
            gameShip_roll(s);
        }
        
        if(fabs(input_pitch) > dz_min)
        {
//            printf("-----------PITCHING-----------\n");
            
            //double s = input_pitch * fabs(input_pitch) * GYRO_DC * tc * (0.01 + speed/MAX_SPEED);
            float s = input_pitch * INPUT_SPEED;

            if(fabs(yprResponse[1]) < maxInputShipRotate) yprResponse[1] += yprD /* * (s/fabs(s)) */;
            //s = s / (1.0 - yprResponse[1]);
            
            gameShip_pitch(s);
        }
        
        if(fabs(input_yaw) > dz_min)
        {
//            printf("-----------YAWING-----------\n");
            
            //double s = input_yaw * fabs(input_yaw) * GYRO_DC * tc * (0.01 + speed/MAX_SPEED);
            float s = input_yaw * INPUT_SPEED;

            if(fabs(yprResponse[2]) < maxInputShipRotate) yprResponse[2] += yprD /* * (s/fabs(s)) */;
            //s = s / (1.0 - yprResponse[2]);
            
            gameShip_yaw(s);
        }
        
//        if(!gameSettingsComplexControls)
//        {
//            gameShip_fakeRoll(input_roll);
//        }
    }
    
    // HACK: in extreme cases the quaternion becomes NaN and we will crash - catch that here
    if(!isnormal(gameShip_getEulerAlpha()) || !isnormal(gameShip_getEulerBeta()) || !isnormal(gameShip_getEulerGamma()))
    {
        // copy camera values
        extern quaternion_t my_ship_bx, my_ship_by, my_ship_bz;
        
        my_ship_bx = cam_pos.bx;
        my_ship_by = cam_pos.by;
        my_ship_bz = cam_pos.bz;
    }
    
    deviceLast[0] = deviceRoll;
    deviceLast[1] = devicePitch;
    deviceLast[2] = deviceYaw;
                
    // convert body axes to world-axes
    // http://en.wikipedia.org/wiki/Euler_angles
}

void trimDoneIgnore(void)
{

}
