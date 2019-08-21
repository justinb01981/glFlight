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

void gameInputStatsCollectStart(void);
void gameInputStatsAppend(float ypr[3]);
void gameInputStatsCalc(void);


double rm[3][3];
int rm_count = 0;
float sp = 0;
int sched_input_action[] = {3,1};
int sched_input_val = 1;
float dz_min = 0.001;
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
int initialized = 0;
double motionRoll, motionPitch, motionYaw;
double deviceRollOff, devicePitchOff, deviceYawOff;
double devicePitch, deviceYaw, deviceRoll;
double devicePitchFrac, deviceYawFrac, deviceRollFrac;
double motionRollMotion, motionPitchMotion, motionYawMotion;
int trimCount = 0;
int trimCountLast = 0;
int trimStartTime = 0;
unsigned int gyroInputCount = 0;
unsigned long gyroInputCountLast = 0;
int gyroStableCount = 0;
int gyroStableCountThresh = (GYRO_SAMPLE_RATE*4);
//float gyroInputDeltaLast[3] = {0, 0, 0};
float fcr = 0.0, fcp = 0.0, fcy = 0.0;
//float gyroInputStableThresh = 0.01;
//float gyroSensitivityCAndroid = 0.5;
double gyroInputRange[3][2];
double gyroLastRange[3];
double gyroAxisMinRange = 0.05;

const float tex_pass_initial_sample = 120;

static double trim_dz[3];

int firstCalibrate = 1;

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
    needTrim = 1;
    needTrimLast = 0;

    gyroInputRange[0][0] = gyroInputRange[1][0] = gyroInputRange[2][0] = INT_MAX;
    gyroInputRange[0][1] = gyroInputRange[1][1] = gyroInputRange[2][1] = -INT_MAX;
    
    gyroLastRange[0] = gyroLastRange[1] = gyroLastRange[2] = 1.0;
    
    isLandscape = GAME_PLATFORM_IS_LANDSCAPE;

    controlsCalibrated = 0;
    firstCalibrate = 1;
    trimStartTime = 0;
    
    gameInputStatsCollectStart();

    gyroStableCount = 0;
    
    initialized = 1;
    
    // set offset based on current position
    deviceRollOff = motionRoll;
    deviceYawOff = motionYaw;
    devicePitchOff = motionPitch;
}

void
gameInputUninit()
{
    gameInputInit();
    initialized = 0;
    speed = 0;
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
gameInputGyro2(float roll, float pitch, float yaw)
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
        motionRoll += roll;
        motionPitch += pitch;
        motionYaw += yaw;
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
    
#if GAME_PLATFORM_IOS
    devicePitch = motionRoll - deviceRollOff;
    deviceYaw = -motionPitch - devicePitchOff;
    deviceRoll = -motionYaw - deviceYawOff;
    
#else
    devicePitch = -motionPitch - devicePitchOff;
    deviceYaw = motionYaw - deviceYawOff;
    deviceRoll = motionRoll - deviceRollOff;
    
#endif

    float deviceO[3] = {deviceRoll, devicePitch, deviceYaw};
    
    if(!initialized)
    {
        return;
    }
    
    gameInputStatsAppend(deviceO);
    
    if(!controlsCalibrated) {
        if (!needTrim && trimCountLast == trimCount) {
            if (gyroStableCount == (GYRO_SAMPLE_RATE * 2)) {
                console_clear();
                console_write("calibrating...");
                gameInterfaceSetInterfaceState(INTERFACE_STATE_TRIM_BLINKING);
                gameInputTrimBegin();
            }
        } else {

            gyro_calibrate_log((float) gyroStableCount / (float) (GYRO_SAMPLE_RATE * 4) * 100);

            if (gyroStableCount >= gyroStableCountThresh) {
                DBPRINTF(("%s:%d", __FILE__, __LINE__));
                console_append("\ndone\n");
                needTrim = 0;
                trimCountLast = 1;
                gameInterfaceControls.trim.blinking = 0;
            }
        }

        gyroStableCount += 1;
    }

    // input variance too wide for trimming, ignore
    if(gameInputTrimPending()) return;
    
    if(gyroInputCount == gyroInputCountLast) return;
    gyroInputCountLast = gyroInputCount;

    // don't collect trim inputs but realign to current position
    if(needTrimLock)
    {
        needTrimLock = 0;
        
        needTrim = 0;
        needTrimLast = 1;
        
        trimStartTime = tex_pass - 1;
    }
    
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

    // MARK: -- trim collecting done
    if(!needTrim && needTrimLast)
    {
        DBPRINTF(("%s:%d", __FILE__, __LINE__));

        gameInterfaceControls.calibrateRect.visible = 0;
        // short trim-length means few input values to calculate range, ignore
        if(tex_pass - trimStartTime >= tex_pass_initial_sample ||
           !controlsCalibrated)
        {
            // TODO: could maybe store these in settings so they didnt have to calibrate every time
            // ... or just tell the user "hold still" and sample on startup...
            // best way would be to observe the values and wait for them to stabilize and use that...
            
            // save last range in case trim is cut short
            for(int i = 0; i < 3; i++) gyroLastRange[i] = gyroInputRange[i][1] - gyroInputRange[i][0];

            DBPRINTF(("%s:%d", __FILE__, __LINE__));
            if(!controlsCalibrated)
            {
                //console_clear();
                //console_write("Welcome to d0gf1ght %s\n    tap to re_center\n", GAME_VERSION_STR);
                if(firstCalibrate)
                {
                    DBPRINTF(("%s:%d", __FILE__, __LINE__));
                    firstCalibrate = 0;
                    gameDialogWelcome();
                }
            }
            
            gameInputStatsCalc();
            
            controlsCalibrated = 1;
            
            gameInterfaceCalibrateDone();
        }
        // short trim-length, adjust range to current orientation
        else
        {
            for(int i = 0; i < 3; i++)
            {
                float R = gyroLastRange[i];

                gyroInputRange[i][0] = deviceO[i] - R/2;
                gyroInputRange[i][1] = deviceO[i] + R/2;
            }
        }
    }
    // MARK: -- trim collecting begin
    else if(needTrim && !needTrimLast)
    {
        trimStartTime = tex_pass;
        
        for(int i = 0; i < 3; i++)
        {
            gyroInputRange[i][0] = deviceO[i]+0.0001;
            gyroInputRange[i][1] = deviceO[i]-0.0001;
        }
        
        deviceRollOff = motionRoll;
        devicePitchOff = motionPitch;
        deviceYawOff = motionYaw;
    }
    // MARK: -- trim collecting continue
    else if(needTrim && needTrimLast)
    {
        for(int r = 0; r < 3; r++)
        {
            if(deviceO[r] < gyroInputRange[r][0])
            {
                gyroInputRange[r][0] = deviceO[r];
                gyroStableCount = 0;
            }
            if(deviceO[r] > gyroInputRange[r][1])
            {
                gyroInputRange[r][1] = deviceO[r];
                gyroStableCount = 0;
            }
            
            if(gyroInputRange[r][1] - gyroInputRange[r][0] < gyroAxisMinRange)
            {
                gyroInputRange[r][1] += (gyroAxisMinRange/2);
                gyroInputRange[r][0] -= (-gyroAxisMinRange/2);
                gyroStableCount = 0;
            }
        }
    }

    // preserve last state for next pass
    needTrimLast = needTrim;
    
    // commented to out to allow game-input to continue while trimming
    //if(needTrim) return;
    
    double C = 3;
    double R[] = {
        gyroInputRange[0][1] - gyroInputRange[0][0],
        gyroInputRange[1][1] - gyroInputRange[1][0],
        gyroInputRange[2][1] - gyroInputRange[2][0]
    };
    
    
    /*
                     Rmax-Rmin
              Dr -  ------------
                       2
       R =   -----------------------
                  C * (Rmax-Rmin)
     
     */
    deviceRollFrac = (deviceRoll - R[0] / 2) / (R[0]*C);
    devicePitchFrac = (devicePitch - R[1] / 2) / (R[1]*C);
    deviceYawFrac = (deviceYaw - R[2] / 2) / (R[2]*C);
    
    double input_roll = deviceRollFrac * (M_PI/GYRO_SAMPLE_RATE);
    double input_pitch = devicePitchFrac * (M_PI/GYRO_SAMPLE_RATE);
    double input_yaw = deviceYawFrac * (M_PI/GYRO_SAMPLE_RATE);

    if(tex_pass % GAME_FRAME_RATE == 0)
    {
        DBPRINTF((
                         "\nroll:%f\npitch:%f\nyaw:%f\n",
                                 deviceRollFrac, devicePitchFrac, deviceYawFrac
                 ));
    }
    
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
    
    if(!needTrim && !needTrimLast && !game_paused)
    {
        // output response ampilfiers
        //float sm[] = /*{0.6, 0.8, 0.8}*/ {0.7, 0.8, 0.9};
        float sm[] = /*{0.6, 0.8, 0.8}*/ PLATFORM_INPUT_COEFFICIENTS;
        // TODO: scale inversely based on angle relative to gravity
        
        // drift toward level with the horizon
        float shipy[3], shipx[3], shipz[3];
        
        if(!gameSettingsComplexControls)
        {
            input_roll = input_yaw*sm[2]*-3.0;
            
            gameShip_unfakeRoll();
        }
            
        gameShip_getYVector(shipy);
        gameShip_getXVector(shipx);
        gameShip_getZVector(shipz);
    
        if(!gameSettingsComplexControls)
        {
            float angleGround = gameShip_calcRoll() - M_PI;
            
            if(fabs(angleGround) > 0.02)
            {
                float r = 0.01;
                
                if(angleGround > 0)
                {
                    r = -r;
                }
                else
                {
                    r = r;
                }
                
                if(shipy[1] < 0) r *= 4;
                
                gameShip_roll(r);
            }
        }
        
        if(fabs(input_roll) > dz_min && gameSettingsComplexControls)
        {
            //printf("-----------ROLLING-----------\n");
            
            double s = input_roll * fabs(input_roll*sm[0]) * GYRO_DC * tc;

            gameShip_roll(s);
        }
        
        if(fabs(input_pitch) > dz_min)
        {
            //printf("-----------PITCHING-----------\n");
            
            double s = input_pitch * fabs(input_pitch*sm[1]) * GYRO_DC * tc;

            gameShip_pitch(s);
        }
        
        if(fabs(input_yaw) > dz_min)
        {
            //printf("-----------YAWING-----------\n");
            
            double s = input_yaw * fabs(input_yaw*sm[2]) * GYRO_DC * tc;

            gameShip_yaw(s);
        }
        
        if(!gameSettingsComplexControls)
        {
            gameShip_fakeRoll(input_roll);
        }
    }
    
    deviceLast[0] = deviceRoll;
    deviceLast[1] = devicePitch;
    deviceLast[2] = deviceYaw;
                
    // convert body axes to world-axes
    // http://en.wikipedia.org/wiki/Euler_angles
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
