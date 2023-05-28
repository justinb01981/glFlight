//
//  gamePlay.h
//  gl_flight
//
//  Created by Justin Brady on 5/14/13.
//
//

#ifndef gl_flight_gamePlay_h
#define gl_flight_gamePlay_h

#include "gameTimeval.h"
#include "world.h"
#include "gameUtils.h"
#include "gameGlobals.h"
#include "gameAI.h"
#include "assert.h"

#define game_ammo_missles_max 4
#define game_ammo_bullets_max 32
#define GAME_POWERUP_DROP_TABLE_LEN 10

enum
{
    STUFF_FLAGS_TURRET = 0x02,
    STUFF_FLAGS_COLLECT = 0x04,
};

enum
{
    GAME_TYPE_NONE = 0,
    GAME_TYPE_DEATHMATCH,
    GAME_TYPE_COLLECT,
    GAME_TYPE_SURVIVAL,
    GAME_TYPE_SPEEDRUN,
    GAME_TYPE_DEFEND,
    GAME_TYPE_TURRET,
    GAME_TYPE_LOBBALL,
    GAME_TYPE_RESERVED3,
    GAME_TYPE_RESERVED4,
    GAME_TYPE_RESERVED5,
    GAME_TYPE_RESERVED6,
    GAME_TYPE_RESERVED7,
    GAME_TYPE_RESERVED8,
    GAME_TYPE_LAST
};

enum
{
    AFFILIATION_NONE = 0,
    AFFILIATION_POWERUP = 1001
};

enum
{
    GAME_SUBTYPE_NONE = 0,
    GAME_SUBTYPE_MISSLE = 1,
    GAME_SUBTYPE_POINTS1,
    GAME_SUBTYPE_COLLECT,
    GAME_SUBTYPE_ASTEROID,
    GAME_SUBTYPE_LIFE,
    GAME_SUBTYPE_TURRET,
    GAME_SUBTYPE_ALLY,
    GAME_SUBTYPE_POINTS,
    GAME_SUBTYPE_LAST,
    
};

typedef enum
{
    GAME_OBJECTIVE_TYPE_DESTROY,
    GAME_OBJECTIVE_TYPE_CAPTURE,
    GAME_OBJECTIVE_DEFEND,
    GAME_OBJECTIVE_TYPE_SURVIVE_UNTIL
} game_objective_type_t;

typedef struct
{
    game_objective_type_t type;
    float params[16];
    char str[256];
} game_objective_t;

#define GAME_VARIABLES_MAX 32
#define GAME_VARIABLE(x) (game_variable_get(x))

static const char *game_log_messages[] = {
    /*"^D: firewalled :-)\n"*/ "^D secured\n",
    "HIDDEN: detected: virus\n",
    "HIDDEN: detected: bounty hunter\n",
    "virus\ndeleted \n^D\n",
    "detected: ally AI\n",
    "HIDDEN: warning: virus capturing ^D\n",
    "killed: virus\n",
    "killed: bounty-hunter\n",
    "killed: turret\n",
    "HIDDEN: shot-fired\n",
    "****NEW HIGH SCORE!****\n",
    "^K^K^KYOU WERE DESTROYED (GAME_OVER)^K^K^K\n",
    "^K^K^KSYSTEM_COMPROMISED (GAME_OVER)^K^K^K\n",
    "HIDDEN: points collected\n",
    "HIDDEN: detected: ^D vulnerable\n",
    "HIDDEN: enemies spawning faster!\n",
    "HIDDEN: vulnerable ^D:%d system integrity:%.0f%% \045\n",
    "HIDDEN: connection successful",
    "HIDDEN: detected: new turret",
    NULL
};

enum {
    GAME_LOG_FILESAVED,
    GAME_LOG_NEWENEMY1,
    GAME_LOG_NEWENEMY2,
    GAME_LOG_FILELOST,
    GAME_LOG_NEWALLY,
    GAME_LOG_WARN_CAPTURE,
    GAME_LOG_KILLED_ENEMY1,
    GAME_LOG_KILLED_ENEMY2,
    GAME_LOG_KILLED_TURRET,
    GAME_LOG_SHOTFIRED,
    GAME_LOG_HIGHSCORE,
    GAME_LOG_GAMEOVER_KILLED,
    GAME_LOG_GAMEOVER,
    GAME_LOG_POWERUP_POINTS,
    GAME_LOG_NEWDATA,
    GAME_LOG_SPAWNFASTER,
    GAME_LOG_SYSTEMINTEGRITY,
    GAME_LOG_TELEPORT,
    GAME_LOG_NEWTURRET,
    GAME_LOG_LAST
};

const static char *game_variables_name[] = {
    "ENEMY_SPAWN_MAX_COUNT",
    "ENEMY_SPAWN_MAX_ALIVE",
    "ENEMY_SPAWN_MOVE_RATE",
    "PHYSICS_FRICTION_C",
    "ENEMY1_FORGET_DISTANCE",
    "ENEMY1_PURSUE_DISTANCE",
    "ENEMY1_SPEED_MAX",
    "ENEMY1_TURN_MAX_RADIANS",
    "ENEMY1_FIRES_MISSLES",
    "ENEMY1_FIRES_LASERS",
    "ENEMY1_CHANGES_TARGET",
    "ENEMY1_PATROLS_NO_TARGET",
    "ENEMY1_RUN_INTERVAL_MS",
    "ENEMY1_LEAVES_TRAIL",
    "ENEMY1_SCAN_DISTANCE_MAX",
    "ENEMY1_MAX_TURN_SKILL_SCALE",
    "ENEMY1_JUKE_PCT",
    "ENEMY1_COLLECT_DURABILITY",
    "ENEMY_RUN_DISTANCE",
    "COLLECT_POINT_LIFETIME",
    NULL
};

extern float game_variables_val[GAME_VARIABLES_MAX];

const static float game_variables_default[] = {
    99999999,    // MAX_SPAWN_COUNT
    10,          // MAX_ALIVE_COUNT
    8,           // ENEMY_SPAWN_MOVE_RATE
    1,           // PHYSICS_FRICTION_C
    50,          // ENEMY1_FORGET_DISTANCE
    30,          // ENEMY1_PURSUE_DISTANCE
    MAX_SPEED,   // MAX_SPEED
    1.2,         // MAX_TURN_RADIANS
    0,           // FIRES MISSLES
    1,           // FIRES LASERS
    1,           // CHANGES_TARGET
    1,           // PATROLS
    50,          // RUN_INTERVAL_MS
    1,           // LEAVES TRAIL
    800.0,       // SCAN_DISTANCE_MAX
    0.02,        // ENEMY1_MAX_TURN_SKILL_SCALE
    /*25*/25,    // ENEMY1_JUKE_PCT
    2,           // ENEMY1_COLLECT_DURABILITY
    20,          // ENEMY_RUN_DISTANCE
    PLATFORM_TICK_RATE*300,       // COLLECT_POINT_LIFETIME
    0,0,0,0,0,0,0,0,0,0,0,0,
    0            // END
};

typedef struct
{
    int started;
    int delay_new_level;
    
    float n_collect_points;
    float max_enemies;
    float counter_enemies_spawned;
    float n_turrets;
    int n_asteroids;
    int n_spawnpoints;
    int difficulty;
    float enemy_intelligence;
    float ally_intelligence;
    int defend_id;
    
    int setting_bot_intelligence;
    
    int ship_destruction_change_alliance;
    //int ship_destruction_drop_powerup;
    //int turret_destruction_drop_powerup;
    int player_drops_powerup;
    float powerup_drop_chance[GAME_POWERUP_DROP_TABLE_LEN];
    int one_collect_at_a_time;
    
    float base_spawn_collect_m;
    float base_spawn_collect_w;
    
    float log_event_camwatch[GAME_LOG_LAST];
    
    float rate_enemy_skill_increase;
    float rate_point_increase;
    float rate_enemy_count_increase;
    float rate_enemy_p_increase;
    
    float enemy_durability;
    
    int invuln_time_after_hit;
    
    float boost_charge;
    
    int powerup_lifetime_frames;
    
    int enemy1_ignore_player_pct;
    
    int points;
    struct
    {
        int enemies_killed;
        int enemies_bh_killed;
        int turrets_killed;
        int score;
        int score_last;
        int level_last;
        float data_collected;
    } stats;
    
    float points_enemy_killed;
    float points_turret_killed;
    float points_enemy_bh_killed;
    float points_data_grabbed;
    float points_per_second_elapsed;
    float enemy_spawnpoint_interval;
    float collect_system_integrity;
    
    game_timeval_t last_run;
    game_timeval_t time_elapsed;
    game_timeval_t end_time;
    
    int caps_owned;
    int caps_found;
    int collects_found;
    int game_type;
    int elem_id_spawnpoint;
    int high_score[GAME_TYPE_LAST];
    unsigned long lifetime_credits;
    int high_score_session[GAME_TYPE_LAST];
    
    int game_action_spawnpoint_new_game;
    
    int score_pop_threshold;
    
    struct {
        float vec[2][3];
        float collect_vec_mag;
        float collect_vec_delt;
        float score;
    } collect_data;
    
    char game_help_message[256];
    
    int object_types_towed[OBJ_LAST];
    
    int elem_id_arrow3[4];
    
    struct {
        int difficulty;
        int score;
    } last_game;
    
    int map_use_current;

} GameStateSinglePlayer;

extern GameStateSinglePlayer gameStateSinglePlayer;

extern float game_ammo_missles;
extern float game_ammo_bullets;
extern float game_ammo_missle_recharge;
extern float game_ammo_bullets_recharge;
const static float GAME_BULLET_LIFETIME = (60*6);
const static float GAME_MISSLE_LIFETIME = (60*10);
extern char *game_status_string;
extern int model_my_ship;

/*
void game_add_cap();

void game_add_enemy();

void game_add_turret();
 */

void game_init();

void game_start(float difficulty, int type);

void game_start_network_guest();

void game_run();

void game_run_paused();

void game_handle_collision(WorldElem* elemA, WorldElem* elemB, int collision_action);

void game_handle_collision_powerup(WorldElem* elemA, WorldElem* elemB);

void game_handle_destruction(WorldElem* elem);

game_timeval_t game_time_remaining();

int game_time_elapsed();

int game_add_spawnpoint(float x, float y, float z, char* game_name);

int game_add_powerup(float x, float y, float z, int type, int lifetime);

void fireBullet(int bulletAction);

int firePoopedCube(WorldElem *elem);

float
game_variable_get(const char* name);

float
game_variable_set(const char* name, float val);

void
game_elem_setup_ship(WorldElem* elem, int skill);

void
game_elem_setup_turret(WorldElem* elem, int skill);

void
game_elem_setup_missle(WorldElem* x);

void
game_elem_setup_powerup(WorldElem* elem);

void
game_elem_setup_collect(WorldElem* elem);

void
game_elem_setup_spawnpoint_enemy(WorldElem* elem);

void
game_elem_setup_spawnpoint(WorldElem* elem);

void
game_elem_setup_base(WorldElem* elem);

void
game_elem_identify(WorldElem* elem, char* name);

void
game_over();

void
game_configure_missle(WorldElem *elem);

int
game_add_bullet(float pos[3], float euler[3], float vel, float leadV, int bulletAction, int missle_target, int affiliation);

int
game_ai_target_priority(WorldElem* pSearchElem, WorldElem* pTargetElem, float* dist_ignore);

#endif
