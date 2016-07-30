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
    STUFF_FLAGS_SHIP = 0x01,
    STUFF_FLAGS_TURRET = 0x02,
    STUFF_FLAGS_COLLECT = 0x04,
};

enum
{
    GAME_TYPE_DEATHMATCH = 0,
    GAME_TYPE_COLLECT,
    GAME_TYPE_SURVIVAL,
    GAME_TYPE_DEFEND,
    GAME_TYPE_TURRET,
    GAME_TYPE_RESERVED2,
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
    GAME_SUBTYPE_SHIP,
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

const static char *game_variables_name[] = {
    "ENEMY_SPAWN_MAX_COUNT",
    "ENEMY_SPAWN_MAX_ALIVE",
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
    NULL
};

static char *game_log_messages[] = {
    /*"^D: firewalled :-)\n"*/ "^D saved - system integrity restored\n",
    "detected: virus\n",
    "detected: bounty hunter\n",
    "virus deleted ^D :-(\n",
    "detected: ally AI\n",
    "warning: virus capturing ^D\n",
    "killed: virus\n",
    "killed: bounty-hunter\n",
    "killed: turret\n",
    "HIDDEN: shot-fired\n",
    "****NEW HIGH SCORE!****\n",
    "^K^K^KYOU WERE DESTROYED (GAME_OVER)^K^K^K\n",
    "^K^K^KSYSTEM_COMPROMISED (GAME_OVER)^K^K^K\n",
    "HIDDEN: points collected\n",
    "HIDDEN: detected: ^D vulnerable\n",
    "enemies spawning faster!\n",
    "vulnerable ^D:%d system integrity:%f \045\n",
    "HIDDEN: connection successful",
    "detected: new turret",
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


extern float game_variables_val[GAME_VARIABLES_MAX];

const static float game_variables_default[] = {
    99999999,    // MAX_SPAWN_COUNT
    10,          // MAX_ALIVE_COUNT
    1,           // PHYSICS_FRICTION_C
    50,          // ENEMY1_FORGET_DISTANCE
    30,          // ENEMY1_PURSUE_DISTANCE
    MAX_SPEED,   // MAX_SPEED
    /*1.2*/0.8,  // MAX_TURN_RADIANS
    0,           // FIRES MISSLES
    1,           // FIRES LASERS
    1,           // CHANGES_TARGET
    1,           // PATROLS
    50,          // RUN_INTERVAL_MS
    1,           // LEAVES TRAIL
    800.0,       // SCAN_DISTANCE_MAX
    0.05,        // ENEMY1_MAX_TURN_SKILL_SCALE
    25,          // ENEMY1_JUKE_PCT
    2,           // ENEMY1_COLLECT_DURABILITY
    20,          // ENEMY_RUN_DISTANCE
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0            // END
};

struct
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
    int turret_destruction_change_alliance;
    int one_collect_at_a_time;
    
    float base_spawn_collect_pct;
    float base_spawn_collect_pct_rate_increase;
    float base_spawn_collect_initial;
    
    float log_event_camwatch[GAME_LOG_LAST];
    
    float rate_enemy_skill_increase;
    float rate_point_increase;
    float rate_enemy_count_increase;
    float rate_enemy_p_increase;
    
    float enemy_durability;
    
    int spawn_powerup_lifetime;
    int spawn_powerup_m;
    
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
    
    struct {
        int difficulty;
        int score;
    } last_game;
    
    int map_use_current;

} gameStateSinglePlayer;

extern float game_ammo_missles;
extern float game_ammo_bullets;
extern float game_ammo_missle_recharge;
extern float game_ammo_bullets_recharge;
const static float GAME_BULLET_LIFETIME = (60*6);
const static float GAME_MISSLE_LIFETIME = (60*6);
extern char *game_status_string;
extern int model_my_ship;

/*
void game_add_cap();

void game_add_enemy();

void game_add_turret();
 */

void game_init();

void game_start(float difficulty, int type);

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

static float
game_variable_get(const char* name)
{
    int i = 0;
    while(game_variables_name[i] != NULL && strcmp(game_variables_name[i], name) != 0)
    {
        i++;
    }
    assert(game_variables_name[i] != NULL);
    return game_variables_val[i];
}

static float
game_variable_set(const char* name, float val)
{
    int i = 0;
    while(game_variables_name[i] != NULL && strcmp(game_variables_name[i], name) != 0)
    {
        i++;
    }
    if(i < sizeof(game_variables_val)/sizeof(float)) game_variables_val[i] = val;
    return val;
}

inline static void
game_elem_setup_ship(WorldElem* elem, int skill)
{
    if(skill > 5) skill = 5;
    
    elem->stuff.u.enemy.leaves_trail = GAME_VARIABLE("ENEMY1_LEAVES_TRAIL");
    elem->stuff.u.enemy.run_distance = rand_in_range(4, GAME_VARIABLE("ENEMY_RUN_DISTANCE"));
    elem->stuff.u.enemy.fixed = 0;
    elem->stuff.u.enemy.changes_target = GAME_VARIABLE("ENEMY1_CHANGES_TARGET");
    elem->stuff.u.enemy.fires = GAME_VARIABLE("ENEMY1_FIRES_LASERS");
    elem->stuff.u.enemy.patrols_no_target = GAME_VARIABLE("ENEMY1_PATROLS_NO_TARGET");
    elem->stuff.u.enemy.max_speed = GAME_VARIABLE("ENEMY1_SPEED_MAX");
    elem->stuff.u.enemy.max_turn = /*0.5*/ GAME_VARIABLE("ENEMY1_TURN_MAX_RADIANS")+ GAME_VARIABLE("ENEMY1_MAX_TURN_SKILL_SCALE")*skill;
    elem->stuff.u.enemy.time_run_interval = GAME_VARIABLE("ENEMY1_RUN_INTERVAL_MS")
    //MAX(20, GAME_AI_UPDATE_INTERVAL_MS - (10 * skill));
    ;
    elem->stuff.u.enemy.scan_distance = /*GAME_VARIABLE("ENEMY1_FORGET_DISTANCE")*/ 1.0
    //50 + (10 * skill);
    ;
    elem->stuff.u.enemy.pursue_distance = GAME_VARIABLE("ENEMY1_PURSUE_DISTANCE")
    //30 - (5 * skill);
    ;
    elem->stuff.u.enemy.ignore_collect = 0;
}

inline static void
game_elem_setup_turret(WorldElem* elem, int skill)
{
    if(skill > 5) skill = 5;
    
    elem->stuff.intelligent = 1;
    elem->stuff.u.enemy.leaves_trail = 0;
    elem->stuff.u.enemy.run_distance = 0;
    elem->stuff.u.enemy.fixed = 1;
    elem->stuff.u.enemy.changes_target = 1;
    elem->stuff.u.enemy.fires = 1;
    elem->stuff.u.enemy.fires_missles = 0;
    elem->stuff.u.enemy.patrols_no_target = 0;
    elem->stuff.u.enemy.max_speed = 0;
    elem->stuff.u.enemy.max_turn = 0.8;
    elem->stuff.u.enemy.time_run_interval = GAME_AI_UPDATE_INTERVAL_MS;
    elem->stuff.u.enemy.scan_distance = 1;
    elem->stuff.u.enemy.pursue_distance = GAME_VARIABLE("ENEMY1_PURSUE_DISTANCE");
}

inline static void
game_elem_setup_missle(WorldElem* x)
{
    x->stuff.intelligent = 1;
    x->physics.ptr->friction = 1;
    x->stuff.u.enemy.changes_target = 0;
    x->stuff.u.enemy.patrols_no_target = 1;
    x->stuff.u.enemy.leaves_trail = 0;
    x->stuff.u.enemy.run_distance = 0;
    x->stuff.u.enemy.fixed = 0;
    x->stuff.u.enemy.fires = 0;
    x->stuff.u.enemy.enemy_state = ENEMY_STATE_PURSUE;
    x->stuff.u.enemy.time_last_run = time_ms;
    x->durability = DURABILITY_MISSLE;
    x->stuff.u.enemy.max_speed = MAX_SPEED_MISSLE;
    x->stuff.u.enemy.max_turn = 4.0; // radians per second
    x->stuff.u.enemy.time_run_interval = GAME_AI_UPDATE_INTERVAL_MS;
    x->stuff.u.enemy.scan_distance = 50;
    x->stuff.u.enemy.pursue_distance = 30;
}

inline static void
game_elem_setup_powerup(WorldElem* elem)
{
    elem->renderInfo.priority = 1;
}

inline static void
game_elem_setup_collect(WorldElem* elem)
{
    world_get_last_object()->durability = 0;
    world_get_last_object()->destructible = 1;
    world_get_last_object()->object_type = OBJ_POWERUP_GENERIC;
    world_get_last_object()->stuff.subtype = GAME_SUBTYPE_COLLECT;
    world_get_last_object()->stuff.radar_visible = 0;
    world_get_last_object()->stuff.affiliation = AFFILIATION_POWERUP;
    world_get_last_object()->bounding_remain = 1;
    world_get_last_object()->physics.ptr->friction = 1;
    update_object_velocity(elem->elem_id, 0, 1, 0, 0);
}

inline static void
game_elem_setup_spawnpoint_enemy(WorldElem* elem)
{
    elem->renderInfo.priority = 1;
}

inline static void
game_elem_setup_spawnpoint(WorldElem* elem)
{
    elem->renderInfo.priority = 1;
}

inline static void
game_elem_identify(WorldElem* elem, char* name)
{
    if(strcmp(name, "collect") == 0)
    {
        game_elem_setup_collect(elem);
    }
    else if(strcmp(name, "turret") == 0)
    {
        game_elem_setup_turret(elem, gameStateSinglePlayer.difficulty);
    }
}

void
game_configure_missle(WorldElem *elem);

int
game_add_bullet(float pos[3], float euler[3], float vel, float leadV, int bulletAction, int missle_target, int affiliation);

int
game_ai_target_priority(WorldElem* pSearchElem, WorldElem* pTargetElem, float* dist_ignore);

#endif
