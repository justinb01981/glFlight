//
//  gameGlobals.h
//  gl_flight
//
//  Created by jbrady on 8/29/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef gl_flight_gameGlobals_h
#define gl_flight_gameGlobals_h

#include "gamePlatform.h"
#include "gameIncludes.h"
#include "gameTimeval.h"

#include "gameDebug.h"

enum {
    DURABILITY_PLAYER = 10,
    DURABILITY_BLOCK = 5,
    DURABILITY_ASTEROID = 250,
    DURABILITY_LOW = 5,
    DURABILITY_ENEMY = 10,
    DURABILITY_ENEMY_DEATHMATCH = 10,
    DURABILITY_TURRET = 3,
    DURABILITY_BULLET = 1,
    DURABILITY_MISSLE = 3,
};

extern int tex_pass;
extern volatile float time_ms;
extern volatile float time_ms_wall;
extern double speed;
extern double maxAccelDecel;
extern double maxSpeed;
extern double minSpeed;
extern double speedBoost;
extern double targetSpeed;
extern double bulletVel;
extern int needTrim;
extern int trimReady;
extern int my_ship_id;
extern int shared_model_sphere_id;
extern int my_ship_changed;
extern float my_ship_x;
extern float my_ship_y;
extern float my_ship_z;
extern float my_ship_alpha;
extern float my_ship_beta;
extern float my_ship_gamma;
static const float brick_size = 1;
extern int needFire;
extern game_timeval_t firedLast;
extern float viewRotationDegrees;
extern float drawDistanceFar;
extern char consoleMessage[2048];
extern int numEnemies;
extern float block_scale;
extern int radar_mode;
extern int save_map;
extern int game_map_custom_loaded;
extern int controls_simple;
extern int fireAction;
extern int fireActionQueuedAfterEdit;

#define APP_ITUNES_URL_BASE "itms://itunes.apple.com/us/app/apple-store/"
#ifdef BUILD_SC95
#define APP_ITUNES_URL APP_ITUNES_URL_BASE"id1348498612?mt=8"
#else
#define APP_ITUNES_URL APP_ITUNES_URL_BASE"id698938088?mt=8"
#endif

#if GAME_PLATFORM_ANDROID
#define GAME_PLATFORM_IS_LANDSCAPE 0
#define GYRO_FEEDBACK_COEFF (0.001)
#define GYRO_DC_DEFAULT /*0.004*/ (0.002)
#define GYRO_FEEDBACK_DEFAULT (0.01)
#define PLATFORM_TICK_RATE 60
#define PLATFORM_DRAW_ELEMS_MAX 1000
#define GYRO_SAMPLE_RATE (60)
#define VISIBLE_DISTANCE_PLATFORM 200
#define PLATFORM_INPUT_COEFFICIENTS {0.5, 0.6, 0.7}
#else
#define GAME_PLATFORM_IS_LANDSCAPE 0
#define GYRO_FEEDBACK_COEFF (0.002)
#define GYRO_DC_DEFAULT /*0.004*/ (0.05)
#define GYRO_FEEDBACK_DEFAULT (0.1)
#define PLATFORM_TICK_RATE 60
#define PLATFORM_DRAW_ELEMS_MAX 1000
#define GYRO_SAMPLE_RATE (30)
#define VISIBLE_DISTANCE_PLATFORM 200
#define PLATFORM_INPUT_COEFFICIENTS {0.5, 0.6, 0.7}
#endif

/* reducing friction influences this... should be MAX_THRUST really */
#define MAX_SPEED /*(15)*/ (15)

#define GAME_AI_DEBUG 0

const static char* GAME_VERSION_STR = "1.5.4_core";

const static char* GAME_NETWORK_DIRECTORY_HOSTNAME_DEFAULT = "d0gf1ght.domain17.net";

const static float MAX_SPEED_MISSLE = (MAX_SPEED * 3);
const static float SPEED_BOOST_FRAMES = 60 * 5;
extern float C_THRUST;
extern float C_FRICTION;
extern float GYRO_FEEDBACK;
extern float GYRO_DC;

const static int pooped_cube_interval_ms = 50;
const static int pooped_cube_lifetime = 100;

const static float collision_repulsion_coeff = 0.02;

const static float RADAR_MIN_VELOCITY = 1;

const static float TOW_DISTANCE_MAX = 20;

const static float GAME_AI_UPDATE_INTERVAL_MS = 50;

#define GAME_FRAME_RATE 60
#define GAME_FRAME_RATE_TIMES_4 240
#define GAME_FRAME_RATE_TIMES_10 600
#define GAME_TICK_RATE PLATFORM_TICK_RATE

extern int controlsCalibrated;

extern game_timeval_t time_engine_sound_next;

#endif
