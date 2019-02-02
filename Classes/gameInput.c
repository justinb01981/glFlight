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

void gameInputStatsCollectStart();
void gameInputStatsAppend(float ypr[3]);
void gameInputStatsCalc();


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
int needTrim = 1;
int needTrimLock = 0;
int isLandscape;
int controlsCalibrated = 0;
float rspin = 1;
double yaw = 0;
double pitch = 0;
double roll = 0;
float inputAvg[3][AVG_INPUTS_LENGTH];
int inputAvg_i = 0;
int initialized = 0;
double motionRoll, motionPitch, motionYaw;
double motionRollMotion, motionPitchMotion, motionYawMotion;
double roll_m, pitch_m, yaw_m;
int trimCount = 0;
int trimCountLast = 0;
unsigned int gyroInputCount = 0;
int gyroStableCount = 0;
int gyroStableCountThresh = (GYRO_SAMPLE_RATE*4);
float gyroInputDeltaLast[3] = {0, 0, 0};
float fcr = 0.0, fcp = 0.0, fcy = 0.0;
float gyroInputStableThresh = 0.01;

const float tex_pass_initial_sample = 60;

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
    speed = minSpeed;
    targetSpeed = speed;
    maxSpeed = MAX_SPEED;
    maxAccelDecel = /*5*/ MAX_SPEED/2; // change per second
    minSpeed = MAX_SPEED / 20;
    bulletVel = MAX_SPEED*5;
    
    isLandscape = GAME_PLATFORM_IS_LANDSCAPE;
    
    gameInputStatsCollectStart();
    
    initialized = 1;
}

void
gameInputTrimBegin()
{
    needTrim = 1;
    needTrimLast = 0;
    trimCount++;
}

void
gameInputTrimAbort()
{
    needTrim = 0;
    needTrimLast = 0;
    trimCount = trimCountLast = 9999;
    gyroStableCount = 9999;
    controlsCalibrated = 1;
    
    trim_dz[0] = trim_dz[1] = trim_dz[2] = 0;
    
    gameDialogWelcome();
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
    static unsigned long gyroInputCountLast = 0;
    
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
    
    float dz_m[3] = {0.0, 0.0, 0.0}; // deadzone-multiplier
#endif
    
    float s[3] = {deviceRoll, devicePitch, deviceYaw};
    
    if(!initialized)
    {
        return;
    }
    
    gameInputStatsAppend(s);
    
    if(gyroStableCount > 0 &&
       (fabs(devicePitch-gyroInputDeltaLast[0]) >= gyroInputStableThresh ||
        fabs(deviceYaw-gyroInputDeltaLast[1]) >= gyroInputStableThresh ||
        fabs(deviceRoll-gyroInputDeltaLast[2]) >= gyroInputStableThresh))
    {
        gyroStableCount = 0;

        gyroInputStableThresh += fabs(((devicePitch-gyroInputDeltaLast[0]) +
                                  (deviceYaw-gyroInputDeltaLast[1]) +
                                  (deviceRoll-gyroInputDeltaLast[2])) / 3) * 0.1;
    }
    else
    {
        gyroStableCount ++;
    }
    
    gyroInputDeltaLast[0] = devicePitch;
    gyroInputDeltaLast[1] = deviceYaw;
    gyroInputDeltaLast[2] = deviceRoll;
    
    if(!controlsCalibrated)
    {
        if(!needTrim && trimCountLast == trimCount)
        {
            if(gyroStableCount == (GYRO_SAMPLE_RATE*2))
            {
                console_clear();
                console_write("calibrating...");
                gameInterfaceSetInterfaceState(INTERFACE_STATE_TRIM_BLINKING);
                gameInputTrimBegin();
                gyroStableCount = 1;
            }
        }
        else
        {
            if(gyroStableCount == 0)
            {
                /* restart learning */
                gameInputTrimBegin();
            }
            
            gyro_calibrate_log((float) gyroStableCount / (float) (GYRO_SAMPLE_RATE*4) * 100);
            
            if(gyroStableCount >= gyroStableCountThresh)
            {
                console_append("\ndone\n");
                needTrim = 0;
                trimCountLast = 1;
                gameInterfaceControls.trim.blinking = 0;
                
                roll_m = deviceRoll;
                pitch_m = devicePitch;
                yaw_m = deviceYaw;
            }
        }
    }
    
    /* wait for sensor to come to rest */
    if(gameInputTrimPending()) return;
    
    if(gyroInputCount == gyroInputCountLast) return;
    gyroInputCountLast = gyroInputCount;
    
    static double trimStart[3];
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
    
    if(needTrimLock)
    {
        needTrimLock = 0;
        
        needTrim = 0;
        needTrimLast = 1;
        
        trimStartTime = tex_pass - 1;
    }
    
    /*
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
     */
    
    // indicate to user when sampling
    if(needTrimLast && controlsCalibrated)
    {
        int samp_min = 10;
        
        if(tex_pass - trimStartTime == samp_min)
        {
            gameInterfaceControls.calibrateRect.visible = 1;
            gyro_calibrate_log(0);
        }
        else if(tex_pass - trimStartTime < tex_pass_initial_sample &&
                tex_pass - trimStartTime > samp_min)
        {
            gyro_calibrate_log((100 / tex_pass_initial_sample) * (tex_pass - trimStartTime));
        }
        else if(tex_pass - trimStartTime == tex_pass_initial_sample)
        {
            gyro_calibrate_log(100);
        }
    }
    
    if(!needTrim && needTrimLast)
    {
        gameInterfaceControls.calibrateRect.visible = 0;
        // short trim-length means few input values to calculate range, ignore
        if(tex_pass - trimStartTime >= tex_pass_initial_sample ||
           !controlsCalibrated)
        {
            // TODO: could maybe store these in settings so they didnt have to calibrate every time
            // ... or just tell the user "hold still" and sample on startup...
            // best way would be to observe the values and wait for them to stabilize and use that...
            dz_roll = trim_dz[0];
            dz_pitch = trim_dz[1];
            dz_yaw = trim_dz[2];
            
            if(!controlsCalibrated)
            {
                //console_clear();
                //console_write("Welcome to d0gf1ght %s\n    tap to re_center\n", GAME_VERSION_STR);
                static int firstCalibrate = 1;
                if(firstCalibrate)
                {
                    firstCalibrate = 0;
                    gameDialogWelcome();
                }
            }
            
            float div = 6;
            gameInputStatsCalc();
            trim_dz[0] = ((gameInputStats.max[0] - gameInputStats.min[0]) / div);
            trim_dz[1] = ((gameInputStats.max[1] - gameInputStats.min[1]) / div);
            trim_dz[2] = ((gameInputStats.max[2] - gameInputStats.min[2]) / div);
            
            controlsCalibrated = 1;
            
            gameInterfaceCalibrateDone();
        }
        
        roll_m = deviceRoll;
        pitch_m = devicePitch;
        yaw_m = deviceYaw;
        
        // zero out input-averaging buffer
        for(int a = 0; a < AVG_INPUTS_LENGTH; a++)
        {
            inputAvg[0][a] = deviceRoll;
            inputAvg[1][a] = devicePitch;
            inputAvg[2][a] = deviceYaw;
        }
    }

    if(needTrim && !needTrimLast)
    {
        trimStartTime = tex_pass;
    }
    needTrimLast = needTrim;
    
    // commented to out to allow game-input to continue while trimming
    //if(needTrim) return;
    
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
    
    /*
    float inputScale = 2.0;
    deviceRoll = deviceRoll * fabs(deviceRoll) * inputScale;
    devicePitch = devicePitch * fabs(devicePitch) * inputScale;
    deviceYaw = deviceYaw * fabs(deviceYaw) * inputScale;
     */
    
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
    float tc = time_ms_wall - time_input_last;
    time_input_last = time_ms_wall;
    
    if(!needTrim && !game_paused)
    {
        float driftComp[3] = {
            /*GYRO_FEEDBACK_COEFF*/(trim_dz[0]) * 4.0,
            /*GYRO_FEEDBACK_COEFF*/(trim_dz[1]) * 4.0,
            /*GYRO_FEEDBACK_COEFF*/(trim_dz[2]) * 4.0
        };
      
        // disabling this for now
        float fcamp = 0.000001;
        fcr = deviceRoll/fabs(deviceRoll) > 0? -fcamp: fcamp;
        fcp = devicePitch/fabs(devicePitch) > 0? -fcamp: fcamp;
        fcy = deviceYaw/fabs(deviceYaw) > 0? -fcamp: fcamp;
        
        float cap = /*0.05*/ 0.1; // radians
        // output response amplfiers
        float sm[] = /*{0.6, 0.8, 0.8}*/ {0.7, 0.8, 0.9};
        
        float cs = (speed - minSpeed) / (maxSpeed*2);
        float speedC = 1.0 /* - (cs*cs*0.9) */ ;
        
        if(gameSettingsSimpleControls)
        {
            // simplified controls (roll buttons)? on-screen controls?
            gameInterfaceSetInterfaceState(INTERFACE_STATE_ONSCREEN_INPUT_CONTROL);
            if(!gameInterfaceControls.altControl.touch_began)
            {
                roll_m = deviceRoll;
                input_roll = 0;
                fcr += 0;
            }
        }
        
        if(fabs(input_roll) > dz_roll*dz_m[0] /*&& touchThrottle*/)
        {
            //printf("-----------ROLLING-----------\n");
            
            double s = input_roll * fabs(input_roll*sm[0]) * GYRO_DC * tc * speedC;
            
            // apply capping
            if(fabs(s) > cap) s = cap * (s/fabs(s));
            
            //gameCamera_rollRadians(s);
            gameShip_roll(s);
        }
        //else
        {
            // feedback to avoid drift
            roll_m += /*input_roll*/ fcr;
        }
        
        if(fabs(input_pitch) > dz_pitch*dz_m[1])
        {
            //printf("-----------PITCHING-----------\n");
            
            double s = input_pitch * fabs(input_pitch*sm[1]) * GYRO_DC * tc * speedC;
            
            if(fabs(s) > cap) s = cap * (s/fabs(s));
            
            //gameCamera_pitchRadians(s);
            gameShip_pitch(s);
        }
        //else
        {
            // feedback to avoid drift
            pitch_m += /*input_pitch*/ fcp;
        }
        
        if(fabs(input_yaw) > dz_yaw*dz_m[2])
        {
            //printf("-----------YAWING-----------\n");
            
            double s = input_yaw * fabs(input_yaw*sm[2]) * GYRO_DC * tc * speedC;
            
            if(fabs(s) > cap) s = cap * (s/fabs(s));
            
            //gameCamera_yawRadians(s);
            gameShip_yaw(s);
        }
        //else
        {
            // feedback to avoid drift
            yaw_m += /*input_yaw*/ fcy;
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

void
gameInputStatsCollectStart()
{
    gameInputStats.buf = gameInputStats.storage;
    gameInputStats.x = 0;
    gameInputStats.samples = /*gameInputStats.max_samples*/ tex_pass_initial_sample;
}

void
gameInputStatsAppend(float ypr[3])
{
    unsigned int l = (unsigned int) gameInputStats.samples * 3;
    
    gameInputStats.buf[(gameInputStats.x) % l] = ypr[0];
    gameInputStats.buf[(gameInputStats.x+1) % l] = ypr[1];
    gameInputStats.buf[(gameInputStats.x+2) % l] = ypr[2];
    gameInputStats.x += 3;
}

void
gameInputStatsCalc()
{
    int j;
    unsigned int l = (unsigned int) gameInputStats.samples * 3;
    
    if(gameInputStats.buf)
    {
        float *src = gameInputStats.buf;
        
        for(j = 0; j < 3; j++)
        {
            gameInputStats.avg[j] = 0;
            gameInputStats.min[j] = 9999;
            gameInputStats.max[j] = -9999;
        }
        
        int i = 0;
        while(i < l)
        {
            for(j = 0; j < 3; j++)
            {
                float s = *(src+j);
                gameInputStats.avg[j] += s/gameInputStats.samples;
                if(s < gameInputStats.min[j]) gameInputStats.min[j] = s;
                if(s > gameInputStats.max[j]) gameInputStats.max[j] = s;
            }
            
            src++;
            i++;
        }
    }
    
    printf("gameInputStats avg:"); for(j = 0; j < 3; j++) printf("%f ", gameInputStats.avg[j]); printf("\n");
    printf("gameInputStats min:"); for(j = 0; j < 3; j++) printf("%f ", gameInputStats.min[j]); printf("\n");
    printf("gameInputStats max:"); for(j = 0; j < 3; j++) printf("%f ", gameInputStats.max[j]); printf("\n");
}
