//
//  gameInput.h
//  gl_flight
//
//  Created by Justin Brady on 7/2/14.
//
//

#ifndef gl_flight_gameInput_h
#define gl_flight_gameInput_h

#include "quaternions.h"

extern double yaw;
extern double pitch;
extern double roll;
extern float qw, qx, qy, qz;
extern double speed, maxSpeed, targetSpeed;
extern int needTrim;
extern int isLandscape;
extern double bulletVel;
extern quaternion_t bx, by, bz, b;
extern double dc;
extern float gyroSenseScale;
extern double deviceRoll, devicePitch, deviceYaw;
extern double deviceRollFrac, devicePitchFrac, deviceYawFrac;
extern double gyroLastRange[3];
const static float GYRO_GRAPHIC_S = 3.3;


static unsigned int TOUCHES_MAX = 4;

void
gameInputInit(void);

void
gameInputUninit(void);

void
gameInputTrimBegin(void (*callback)(void));

void
gameInput_trimLock(void);

void
gameInputGyro(float roll, float pitch, float yaw);

void
gameInputGyro2(float roll, float pitch, float yaw);

void
gameInputMotion(float roll, float pitch, float yaw);

void
gameInput(void);

void
gyro_calibrate_log(float pct);

#endif
