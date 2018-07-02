//
//  gamePlay.c
//  gl_flight
//
//  Created by Justin Brady on 5/27/13.
//
//

// IDEAS:
// - when all nodes are down, spawn super-node you have to kill.. hitting it cause bullets to fire back
// - defend-your-base-mode with progressively more aggressive enemies spawning
// - hold onto capture-points (accruing points/spawning allies)
// - capture the flag
// - 10-9-2017 - remove enemy base, enemies should come in from map borders, and teleport out with data
//   better indication of system distress state, based on number of enemies in system
//   instead of saving floppies you have to expunge intruders (with new/bigger waves starting?)
//

#include <stdio.h>
#include <math.h>
#include <limits.h>


#include "gamePlay.h"
#include "gameGlobals.h"
#include "gameAI.h"
#include "gameTimeval.h"
#include "world_motion.h"
#include "gameUtils.h"
#include "textures.h"
#include "gameAudio.h"
#include "gameNetwork.h"
#include "maps.h"
#include "world_file.h"
#include "action.h"
#include "gameShip.h"
#include "gameInterface.h"
#include "gameCamera.h"
#include "gameSettings.h"
#include "gameGraphics.h"
#include "gameDialogs.h"
#include "sounds.h"
#include "collision.h"

static int game_setup_needed = 0;
static int game_reset_needed = 0;
int game_collision_last_bullet_affiliation = 0;
int save_map = 0;
int game_target_missle_id = -1;
int game_target_objective_id = -1;
float game_ammo_bullets = game_ammo_bullets_max;
float game_ammo_missles = game_ammo_missles_max;
float game_ammo_missle_recharge = 0.0;
float game_ammo_bullets_recharge = 2.0;
float game_boost_recharge_rate = 0.1;
int model_my_ship = MODEL_SHIP1;
int new_level = 0;
int game_delay_frames = 0;
int invuln_count = 0;
unsigned int score_last_checked = 0;
//float collects_count_gameover = 10;
char game_status_string_[255] = {0};
char *game_status_string = game_status_string_;
const float randH = 100000;

game_timeval_t firedLast = 0, firedLastMissle = 0;
float blr_last = 1;

float game_variables_val[GAME_VARIABLES_MAX];

extern void random_heading_vector(float v[3]);

int
game_add_ally(float x, float y, float z);

// scoring: "capture" data powerup = 10
//          kill enemy bot = 2
//          kill enemy turret = 2
//          kill enemy bounty hunter = 5
//          points modifier = 0.5 * level
//
static void
score_total()
{
    int *high_score;
    int *high_score_session;
    
    float m = gameStateSinglePlayer.difficulty;
    
    high_score = &(gameStateSinglePlayer.high_score[gameStateSinglePlayer.game_type]);
    high_score_session = &(gameStateSinglePlayer.high_score_session[gameStateSinglePlayer.game_type]);
    
    float fscore = 0;
    
    fscore += gameStateSinglePlayer.stats.data_collected * gameStateSinglePlayer.points_data_grabbed;
    fscore += gameStateSinglePlayer.stats.enemies_killed * gameStateSinglePlayer.points_enemy_killed;
    fscore += gameStateSinglePlayer.stats.enemies_bh_killed * gameStateSinglePlayer.points_enemy_bh_killed;
    fscore += gameStateSinglePlayer.stats.turrets_killed * gameStateSinglePlayer.points_turret_killed;
    fscore += gameStateSinglePlayer.points_per_second_elapsed * game_time_elapsed();
    
    float point_bonus_collected = 0;
    while(console_log_search(game_log_messages[GAME_LOG_POWERUP_POINTS], point_bonus_collected) != NULL) point_bonus_collected++;
    
    fscore += point_bonus_collected * 10;
    
    gameStateSinglePlayer.stats.score = /*gameStateSinglePlayer.stats.score_last + */ (fscore * m);
    
    if(gameStateSinglePlayer.stats.score > *high_score)
    {
        *high_score = gameStateSinglePlayer.stats.score;
        if(console_log_search(game_log_messages[GAME_LOG_HIGHSCORE], 0) == NULL)
        {
            console_write(game_log_messages[GAME_LOG_HIGHSCORE]);
            gameAudioPlaySoundAtLocation("highscore", my_ship_x, my_ship_y, my_ship_z);
        }
    }
}

static void
score_clear()
{
    score_total();
    
    gameStateSinglePlayer.stats.level_last = gameStateSinglePlayer.difficulty;
    
    gameStateSinglePlayer.stats.data_collected = 0;
    gameStateSinglePlayer.stats.enemies_killed = 0;
    gameStateSinglePlayer.stats.enemies_bh_killed = 0;
    gameStateSinglePlayer.stats.turrets_killed = 0;
    gameStateSinglePlayer.stats.score_last = gameStateSinglePlayer.stats.score;
    gameStateSinglePlayer.stats.score = 0;
}

static void
score_display()
{
    int *high_score;
    int *high_score_session;
    char* score_prefix = "";
    
    high_score = &(gameStateSinglePlayer.high_score[gameStateSinglePlayer.game_type]);
    high_score_session = &(gameStateSinglePlayer.high_score_session[gameStateSinglePlayer.game_type]);
    
    score_total();
    
    if(console_log_search(game_log_messages[GAME_LOG_GAMEOVER_KILLED], 0) != NULL) score_prefix = "KILLED\n";
    
    float capture_log_count = 0;
    while(console_log_search(game_log_messages[GAME_LOG_FILESAVED], capture_log_count) != NULL) capture_log_count++;
    
    float lost_log_count = 0;
    while(console_log_search(game_log_messages[GAME_LOG_FILELOST], lost_log_count) != NULL) lost_log_count++;
    
    float shots_fired = 0;
    while(console_log_search(game_log_messages[GAME_LOG_SHOTFIRED], shots_fired) != NULL) shots_fired++;
    
    float enemies_killed1 = 0;
    while(console_log_search(game_log_messages[GAME_LOG_KILLED_ENEMY1], enemies_killed1) != NULL) enemies_killed1++;
    
    float enemies_killed2 = 0;
    while(console_log_search(game_log_messages[GAME_LOG_KILLED_ENEMY2], enemies_killed2) != NULL) enemies_killed2++;
    
    float point_bonus_collected = 0;
    while(console_log_search(game_log_messages[GAME_LOG_POWERUP_POINTS], point_bonus_collected) != NULL) point_bonus_collected++;
    
    float enemies_killed = enemies_killed1 + enemies_killed2;
    
    char scoreStr[255];
    sprintf(scoreStr, "%sScore:** %d **\n"
            "^D saved/lost: %.0f/%.0f (%.0fpct)\n"
            "bot kills:%d\n"
            "bounty-hunter kills:%d\n"
            "Turret kills:%d\n"
            "Accuracy:%f\n"
            "Time:%02dsec\n"
            "High score:%d\n"
            /* "Session High score:%d\n" */
            "",
            score_prefix,
            gameStateSinglePlayer.stats.score,
            capture_log_count,
            lost_log_count,
            (capture_log_count / (capture_log_count+lost_log_count)) * 100.0,
            gameStateSinglePlayer.stats.enemies_killed,
            gameStateSinglePlayer.stats.enemies_bh_killed,
            gameStateSinglePlayer.stats.turrets_killed,
            shots_fired / enemies_killed,
            game_time_elapsed(),
            *high_score
            /* ,*high_score_session */
            );
    
    gameDialogGraphic(TEXTURE_ID_DIALOG_GAMEOVER);
    gameInterfaceControls.dialogRect.tex_id = TEXTURE_ID_CONTROLS_DIALOG_BOX;
    gameDialogDisplayString(scoreStr);
    
    gameAudioPlaySoundAtLocation("victory", my_ship_x, my_ship_y, my_ship_z);
    
    game_paused = 1;
}

static void
score_pop()
{
    char str[255];
    sprintf(str, "+ %d", gameStateSinglePlayer.stats.score - score_last_checked);
    gameDialogScorePopup(str);
}

static void
score_kill(WorldElem* pElem)
{
    switch(pElem->object_type)
    {
        case OBJ_TURRET:
            gameStateSinglePlayer.stats.turrets_killed++;
            console_write(game_log_messages[GAME_LOG_KILLED_TURRET]);
            break;
            
        case OBJ_SHIP:
            if(pElem->stuff.u.enemy.fires_missles)
            {
                gameStateSinglePlayer.stats.enemies_bh_killed++;
                console_write(game_log_messages[GAME_LOG_KILLED_ENEMY2]);
            }
            else
            {
                gameStateSinglePlayer.stats.enemies_killed++;
                console_write(game_log_messages[GAME_LOG_KILLED_ENEMY1]);
            }
            break;
            
        case OBJ_POWERUP_GENERIC:
            if(pElem->stuff.subtype == GAME_SUBTYPE_COLLECT) gameStateSinglePlayer.points_data_grabbed++;
            break;
            
        default:
            break;
    }
}

static void
game_over()
{
    score_display();
    
    memset(&gameStateSinglePlayer.last_game, 0, sizeof(gameStateSinglePlayer.last_game));
    console_write("game over!");
    game_reset_needed = /*1*/ 0;
    
    camera_locked_frames = 120;
    gameInterfaceControls.mainMenu.visible = 1;
    gameInterfaceControls.textMenuControl.visible = 1;
    
    gameAudioPlaySoundAtLocation("vibrate",
                                 my_ship_x,
                                 my_ship_y,
                                 my_ship_z);
    
    gameStateSinglePlayer.started = 0;
    
    gameStateSinglePlayer.stats.score = 0;
}

int
game_findMissleTarget(float p[3], float u[3], int *obj_types, float infoOut[3], int src_id)
{
    // info out = dot,dist,id
    WorldElemListNode* cur = gWorld->elements_moving.next;
    infoOut[0] = -1;
    infoOut[1] = 0;
    infoOut[2] = -1;
    int id = -1;
    
    float f[3];
    
    while(cur)
    {
        int type_matched = 0;
        
        for(int i = 0; obj_types[i] != OBJ_UNKNOWN; i++)
        {
            if(cur->elem->elem_id != src_id &&
               cur->elem->object_type == obj_types[i]) type_matched = 1;
        }
        
        if(type_matched)
        {
            float v[3] =
            {
                cur->elem->physics.ptr->x - p[0],
                cur->elem->physics.ptr->y - p[1],
                cur->elem->physics.ptr->z - p[2]
            };
            
            float d = distance(p[0], p[1], p[2],
                               cur->elem->physics.ptr->x, cur->elem->physics.ptr->y,
                               cur->elem->physics.ptr->z);
            v[0] /= d;
            v[1] /= d;
            v[2] /= d;
            
            f[0] = -dot2(u, v);
            f[1] = d;
            f[2] = cur->elem->elem_id;
            
            if(f[0] > infoOut[0])
            {
                id = f[2];
                
                infoOut[0] = f[0];
                infoOut[1] = f[1];
                infoOut[2] = f[2];
            }
        }
        
        cur = cur->next;
    }
    
    return id;
}

int
game_add_powerup(float x, float y, float z, int type, int lifetime)
{
    float spawn[6];
    int tex_id;
    int radar_visible = 0;
    
    spawn[0] = x;
    spawn[1] = y;
    spawn[2] = z;
    
    spawn[3] = spawn[4] = spawn[5] = 0;
    
    switch(type)
    {
        case GAME_SUBTYPE_COLLECT:
            tex_id = TEXTURE_ID_DATA;
            radar_visible = 1;
            break;
            
        case GAME_SUBTYPE_LIFE:
            tex_id = TEXTURE_ID_POWERUP_LIFE;
            break;
            
        case GAME_SUBTYPE_TURRET:
            tex_id = TEXTURE_ID_POWERUP_TURRET;
            break;
            
        case GAME_SUBTYPE_MISSLE:
            tex_id = TEXTURE_ID_POWERUP_MISSLE;
            break;
            
        case GAME_SUBTYPE_POINTS:
            tex_id = TEXTURE_ID_POWERUP_POINTS;
            break;
            
        default:
            tex_id = TEXTURE_ID_CAPTUREPOINT;
            radar_visible = 1;
            break;
    }
    
    int obj_id =
    world_add_object(MODEL_SPRITE,
                     spawn[0], spawn[1], spawn[2],
                     spawn[3], spawn[4], spawn[5],
                     2.0, tex_id);
    
    world_get_last_object()->durability = 0;
    world_get_last_object()->destructible = 1;
    world_get_last_object()->object_type = OBJ_POWERUP_GENERIC;
    world_get_last_object()->stuff.subtype = type;
    world_get_last_object()->stuff.radar_visible = radar_visible;
    world_get_last_object()->stuff.affiliation = AFFILIATION_POWERUP;
    world_get_last_object()->bounding_remain = 1;
    world_get_last_object()->physics.ptr->friction = 1;
    world_get_last_object()->physics.ptr->gravity = 1;
    update_object_velocity(obj_id, 0, 1, 0, 0);
    game_elem_setup_powerup(world_get_last_object());
    
    if(lifetime > 0)
    {
        world_object_set_lifetime(obj_id, /* tex-passes */ lifetime);
    }
    
    return obj_id;
}

int
game_powerup_add_random(float x, float y, float z)
{
    int ret = WORLD_ELEM_ID_INVALID;
    
    int pow_t = GAME_SUBTYPE_NONE;
    float pow_r = rand_in_range(0, 1000);
    
    pow_r = floor(pow_r / (1000 / GAME_POWERUP_DROP_TABLE_LEN));
    pow_t = gameStateSinglePlayer.powerup_drop_chance[(int) pow_r];
    
    if(pow_t != GAME_SUBTYPE_NONE)
    {
        switch(pow_t)
        {
            case GAME_SUBTYPE_ALLY:
                ret = game_add_ally(x, y, z);
                break;
                
            default:
                ret = game_add_powerup(x, y, z,
                                       pow_t, gameStateSinglePlayer.powerup_lifetime_frames);
                break;
        }
    }
    
    return ret;
}

void
game_add_asteroid(float x, float y, float z, float dx, float dy, float dz)
{
    float spawn[6];
    
    spawn[0] = x;
    spawn[1] = y;
    spawn[2] = z;
    
    spawn[3] = spawn[4] = spawn[5] = (rand() % 628)/100.0;
    
    int obj_id =
    world_add_object(MODEL_ICOSAHEDRON,
                     spawn[0], spawn[1], spawn[2],
                     spawn[3], spawn[4], spawn[5],
                     5, TEXTURE_ID_ASTEROID);
    
    float dest[3] = {dx, dy, dz};
    float speed = (rand() % 10);
    
    world_get_last_object()->durability = 5;
    world_get_last_object()->destructible = 1;
    world_get_last_object()->object_type = OBJ_BLOCK_MOVING;
    world_get_last_object()->stuff.subtype = GAME_SUBTYPE_ASTEROID;
    update_object_velocity(obj_id,
                           (dest[0] - spawn[0]) / speed,
                           (dest[1] - spawn[1]) / speed,
                           (dest[2] - spawn[2]) / speed,
                           0);
}

int
game_add_enemy_core(float x, float y, float z, int model)
{
    float spawn[6];
    
    spawn[0] = x;
    spawn[1] = y;
    spawn[2] = z;
    
    spawn[3] = spawn[4] = spawn[5] = 0;
    
    int obj_id =
    world_add_object(model,
                     spawn[0], spawn[1], spawn[2],
                     spawn[3], spawn[4], spawn[5],
                     1, TEXTURE_ID_ENEMYSHIP);
    world_get_last_object()->object_type = OBJ_SHIP;
    
    world_get_last_object()->stuff.u.enemy.target_id = -1;
    world_get_last_object()->stuff.u.enemy.intelligence = gameStateSinglePlayer.enemy_intelligence;
    world_get_last_object()->stuff.u.enemy.enemy_state = ENEMY_STATE_PATROL;
    world_get_last_object()->durability = gameStateSinglePlayer.enemy_durability;
    world_get_last_object()->stuff.intelligent = 1;
    world_get_last_object()->physics.ptr->friction = 1;
    game_elem_setup_ship(world_get_last_object(), gameStateSinglePlayer.enemy_intelligence);
    update_object_velocity(obj_id, 1, 1, 1, 0);
    
    console_write(game_log_messages[GAME_LOG_NEWENEMY1]);
    if(gameStateSinglePlayer.log_event_camwatch[GAME_LOG_NEWENEMY1] > 0)
    {
        gameStateSinglePlayer.log_event_camwatch[GAME_LOG_NEWENEMY1]--;
        glFlightCamFix(obj_id);
    }
    
    return obj_id;
}

int
game_add_enemy(float x, float y, float z)
{
    int obj_id = game_add_enemy_core(x, y, z, MODEL_SHIP2);
    world_get_last_object()->stuff.u.enemy.ignore_player = (rand_in_range(1, 100) <= gameStateSinglePlayer.enemy1_ignore_player_pct);
    return obj_id;
}

int
game_add_enemy_bountyhunter(float x, float y, float z)
{
    int obj_id = game_add_enemy_core(x, y, z, MODEL_SHIP3);
    game_elem_setup_ship(world_get_last_object(), gameStateSinglePlayer.enemy_intelligence);
    world_get_last_object()->texture_id = TEXTURE_ID_ENEMYSHIP_ACE;
    world_get_last_object()->stuff.u.enemy.fires_missles = 0;
    world_get_last_object()->durability *= 4;
    world_get_last_object()->stuff.u.enemy.ignore_collect = 1;
    console_write(game_log_messages[GAME_LOG_NEWENEMY2]);
    return obj_id;
}

int
game_add_ally(float x, float y, float z)
{
    int obj_id = game_add_enemy_core(x, y, z, MODEL_SHIP1);
    
    world_get_last_object()->texture_id = TEXTURE_ID_ALLYSHIP;
    world_get_last_object()->stuff.affiliation = gameNetworkState.my_player_id;
    
    console_write(game_log_messages[GAME_LOG_NEWALLY]);
    return obj_id;
}

void
game_add_turret(float x, float y, float z)
{
    float spawn[6];
    float scale = 4;
    
    spawn[0] = x;
    spawn[1] = y;
    spawn[2] = z;
    
    /*
    while(world_get_object_n_at_loc(0, spawn[0], spawn[1], spawn[2], scale) != NULL)
    {
        if(spawn[1] < gWorld->bound_y) spawn[1] += scale;
        else break;
    }
     */
    
    spawn[3] = spawn[4] = spawn[5] = 0;
    
    int obj_id =
    world_add_object(MODEL_TURRET,
                     spawn[0], spawn[1], spawn[2],
                     spawn[3], spawn[4], spawn[5],
                     scale, TEXTURE_ID_TURRET);
    
    world_get_last_object()->stuff.u.enemy.target_id = -1;
    world_get_last_object()->stuff.u.enemy.intelligence = gameStateSinglePlayer.enemy_intelligence;
    world_get_last_object()->stuff.u.enemy.enemy_state = ENEMY_STATE_PURSUE;
    world_get_last_object()->durability = DURABILITY_TURRET;
    world_get_last_object()->object_type = OBJ_TURRET;
    world_get_last_object()->stuff.intelligent = 1;
    game_elem_setup_turret(world_get_last_object(), gameStateSinglePlayer.enemy_intelligence);
    update_object_velocity(obj_id, 0, -0.1, -0.1, -0.1);
}

int
game_add_spawnpoint(float x, float y, float z, char* game_name)
{
    float spawn[6];
    float scale = 8;
    
    spawn[0] = x;
    spawn[1] = y;
    spawn[2] = z;
    
    spawn[3] = spawn[4] = spawn[5] = 0;
    
    int obj_id =
    world_add_object(MODEL_SPRITE,
                     spawn[0], spawn[1], spawn[2],
                     spawn[3], spawn[4], spawn[5],
                     scale, TEXTURE_ID_ENEMY_SPAWNPOINT);
    
    world_get_last_object()->durability = 0;
    world_get_last_object()->destructible = 1;
    world_get_last_object()->object_type = OBJ_SPAWNPOINT_ENEMY;
    world_get_last_object()->stuff.radar_visible = 1;
    update_object_velocity(obj_id, 0, 0, 0, 0);
    
    if(game_name)
    {
        world_object_set_nametag(world_get_last_object()->elem_id, game_name);
        world_get_last_object()->object_type = OBJ_PORTAL;
    }
    
    return obj_id;
}

static int object_id_portal = WORLD_ELEM_ID_INVALID;

void
game_move_spawnpoint(WorldElem* pElem)
{
    static float orbit_T = 0.0;
    float radius = 75;
    float orbit_origin[] = {0, atoi(MAP_BASE_ALT), 0};
    
    if(1)
    {
        // orbit around center of world
        // orbit period
        orbit_T += (M_PI * game_variable_get("ENEMY_SPAWN_MOVE_RATE")) / 15000;
        if(orbit_T > 2.0*M_PI) orbit_T = 0;
        
        orbit_around(pElem, orbit_origin, orbit_T, radius);
        
        // add decoration
        float vD[] = {
            orbit_origin[0] + sin(orbit_T)*(radius+1),
            orbit_origin[1],
            orbit_origin[2] + cos(orbit_T)*(radius+1)
        };
        if(object_id_portal != WORLD_ELEM_ID_INVALID)
        {
            object_id_portal =
            world_replace_object(object_id_portal, MODEL_SPRITE, vD[0], vD[1], vD[2],
                                 0, 0, 0, pElem->scale, TEXTURE_ID_PORTAL);
        }
        else
        {
            object_id_portal =
            world_add_object(MODEL_SPRITE, vD[0], vD[1], vD[2], 0, 0, 0, pElem->scale, TEXTURE_ID_PORTAL);
            world_get_last_object()->object_type = OBJ_DISPLAYONLY;
            world_get_last_object()->renderInfo.priority = 1;
        }
    }
}

void
game_reset()
{
    WorldElemListNode* pCur = gWorld->elements_moving.next;
    
    while(pCur)
    {
        if(/*pCur->elem->object_type != OBJ_PLAYER*/ 1)
        {
            if(!world_elem_list_find_elem(pCur->elem, &gWorld->elements_to_be_freed))
            {
                world_elem_list_add(pCur->elem, &gWorld->elements_to_be_freed);
            }
        }
        pCur = pCur->next;
    }
}

void
game_init_objects()
{
    WorldElemListNode* pCur = gWorld->elements_moving.next;
    while(pCur)
    {
        switch(pCur->elem->object_type)
        {
            case OBJ_SPAWNPOINT_ENEMY:
                pCur->elem->stuff.u.spawnpoint.spawn_coeff = 0;
                pCur->elem->stuff.u.spawnpoint.spawn_intelligence = 3 + gameStateSinglePlayer.difficulty;
                break;
                
            case OBJ_BASE:
                pCur->elem->destructible = 0;
                break;
                
            case OBJ_BLOCK_MOVING:
                pCur->elem->stuff.u.orbiter.radius = rand_in_range(12, 75);
                pCur->elem->stuff.u.orbiter.period = rand_in_range(4, 16);
                pCur->elem->stuff.u.orbiter.theta = rand_in_range(0, 628) / 100.0;
                break;
                
            default:
                break;
        }
        pCur = pCur->next;
    }
}

void
game_init()
{
    int i;
    
    memset(&gameStateSinglePlayer.high_score_session, 0, sizeof(gameStateSinglePlayer.high_score_session));
    gameStateSinglePlayer.map_use_current = 0;
    
    for(i = 0; i < GAME_VARIABLES_MAX; i++) game_variables_val[i] = game_variables_default[i];
}

void
game_start(float difficulty, int type)
{
    char dialogStr[255];
    
    score_clear();
    
    console_log_clear(0, 1);
    
    if(difficulty <= 1)
    {
        memset(&gameStateSinglePlayer.last_game, 0, sizeof(gameStateSinglePlayer.last_game));
        gameStateSinglePlayer.stats.score_last = 0;
        gameStateSinglePlayer.stats.score = 0;
        gameStateSinglePlayer.stats.level_last = 0;
        texture_id_background = BACKGROUND_TEXTURE;
    }
    else
    {
        gameStateSinglePlayer.last_game.difficulty = difficulty;
        gameStateSinglePlayer.last_game.score = gameStateSinglePlayer.stats.score;
    }
    
    game_setup_needed = 1;
    
    gameStateSinglePlayer.difficulty = difficulty;
    
    gameStateSinglePlayer.game_type = type;
    
    gameStateSinglePlayer.delay_new_level = 0;
    
    gameStateSinglePlayer.defend_id = -1;
    gameStateSinglePlayer.rate_enemy_skill_increase = 0;
    gameStateSinglePlayer.rate_enemy_count_increase = 0;
    gameStateSinglePlayer.rate_enemy_p_increase = 0;
    gameStateSinglePlayer.rate_point_increase = 0;
    gameStateSinglePlayer.one_collect_at_a_time = 0;
    gameStateSinglePlayer.spawn_powerup_lifetime = 0;
    gameStateSinglePlayer.spawn_powerup_m = 0;
    gameStateSinglePlayer.enemy_durability = DURABILITY_ENEMY_DEATHMATCH;
    gameStateSinglePlayer.max_enemies = 0;
    gameStateSinglePlayer.counter_enemies_spawned = 99999;
    gameStateSinglePlayer.n_turrets = 0;
    gameStateSinglePlayer.n_asteroids = 30;
    
    gameStateSinglePlayer.enemy_intelligence = gameStateSinglePlayer.ally_intelligence = difficulty;
    
    gameStateSinglePlayer.collect_data.collect_vec_delt = 5;
    gameStateSinglePlayer.collect_data.collect_vec_mag = 20;
    
    gameStateSinglePlayer.points_data_grabbed = 200;
    gameStateSinglePlayer.points_enemy_killed = 50;
    gameStateSinglePlayer.points_enemy_bh_killed = gameStateSinglePlayer.points_enemy_killed*2;
    gameStateSinglePlayer.points_turret_killed = 25;
    gameStateSinglePlayer.points_per_second_elapsed = 0;
    gameStateSinglePlayer.invuln_time_after_hit = 0;
    gameStateSinglePlayer.powerup_lifetime_frames = GAME_FRAME_RATE * 10;
    gameStateSinglePlayer.player_drops_powerup = 0;
    gameStateSinglePlayer.enemy1_ignore_player_pct = 0;
    gameStateSinglePlayer.score_pop_threshold = 10;
    gameStateSinglePlayer.enemy_spawnpoint_interval = 1;
    
    gameStateSinglePlayer.collect_system_integrity = 100;
    
    for(int t = 0; t < GAME_LOG_LAST; t++) gameStateSinglePlayer.log_event_camwatch[t] = 0;
    
    gameStateSinglePlayer.log_event_camwatch[GAME_LOG_NEWENEMY1] = 1;
    gameStateSinglePlayer.log_event_camwatch[GAME_LOG_WARN_CAPTURE] = 1;
    
    collision_actions_set_default();
    
    for(int t = 0; t < OBJ_LAST; t++) gameStateSinglePlayer.object_types_towed[t] = 0;
    
    for(int t = 0; t < GAME_POWERUP_DROP_TABLE_LEN; t++) gameStateSinglePlayer.powerup_drop_chance[t] = 0.0;
    
    if(gameStateSinglePlayer.game_type == GAME_TYPE_DEATHMATCH)
    {
        strcpy(gameStateSinglePlayer.game_help_message, "begin deathmatch");
        //game_reset();
        
        gameStateSinglePlayer.ship_destruction_change_alliance = 0;
        gameStateSinglePlayer.n_asteroids = 0;
        
        gameStateSinglePlayer.spawn_powerup_lifetime = 60*10;
        gameStateSinglePlayer.spawn_powerup_m = 20;
        
        gameStateSinglePlayer.object_types_towed[OBJ_TURRET] = 1;

        gameStateSinglePlayer.powerup_drop_chance[5] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[6] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[7] = GAME_SUBTYPE_LIFE;
        
        collision_actions_set_grab_powerup();
        
        gameStateSinglePlayer.player_drops_powerup = 1;
    }
    else if(gameStateSinglePlayer.game_type == GAME_TYPE_COLLECT)
    {
        sprintf(dialogStr, "^D^D^D     INTRUDER ALERT    ^D^D^D\n"
                           "^D^D^D   ACTIVATE FIREWALL   ^D^D^D\n"
                           "^D^D^D   PROTECT CORE ^2^D^1     ^D^D^D\n"
                            "\n"
                           "****   High Score: %d  ****",
                gameStateSinglePlayer.high_score[gameStateSinglePlayer.game_type]);
        console_write(dialogStr);
        
        game_reset();
        
        gameStateSinglePlayer.max_enemies = 1 + difficulty;
        gameStateSinglePlayer.enemy_durability = game_variable_get("ENEMY1_COLLECT_DURABILITY");
        gameStateSinglePlayer.n_turrets = 2 + gameStateSinglePlayer.difficulty * 1;
        gameStateSinglePlayer.n_collect_points = 0;
        gameStateSinglePlayer.powerup_drop_chance[5] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[6] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[7] = GAME_SUBTYPE_LIFE;
        gameStateSinglePlayer.powerup_drop_chance[8] = GAME_SUBTYPE_LIFE;
        //gameStateSinglePlayer.powerup_drop_chance[8] = GAME_SUBTYPE_SHIP;
        //gameStateSinglePlayer.powerup_drop_chance[9] = GAME_SUBTYPE_POINTS;
        gameStateSinglePlayer.ship_destruction_change_alliance = 0;
        gameStateSinglePlayer.counter_enemies_spawned = /* 3 + difficulty * 2 */ 9999;
        gameStateSinglePlayer.enemy_spawnpoint_interval = 5;
        gameStateSinglePlayer.base_spawn_collect_m = 0.9;
        gameStateSinglePlayer.log_event_camwatch[GAME_LOG_NEWDATA] = 3;
        
        gameStateSinglePlayer.rate_enemy_skill_increase = 1.0/20;
        gameStateSinglePlayer.rate_enemy_count_increase = 1.0/20;
        
        gameStateSinglePlayer.enemy1_ignore_player_pct = 75;
        
        // different map for "collection" game type
        gameMapSetMap(initial_map_collection);
        
        gameStateSinglePlayer.game_action_spawnpoint_new_game = 0;
        
        gameStateSinglePlayer.invuln_time_after_hit = 2;
        
        collision_actions[OBJ_PLAYER][OBJ_POWERUP_GENERIC] = COLLISION_ACTION_FLAG;
        collision_actions[OBJ_POWERUP_GENERIC][OBJ_PLAYER] = COLLISION_ACTION_FLAG;
    }
    else if(gameStateSinglePlayer.game_type == GAME_TYPE_SURVIVAL)
    {
        sprintf(dialogStr, "survive as long as possible\n"
                "**** High Score: %d ****",
                gameStateSinglePlayer.high_score[gameStateSinglePlayer.game_type]);
        gameDialogDisplayString(dialogStr);
        
        game_reset();
        
        gameStateSinglePlayer.max_enemies = 3 + difficulty;
        gameStateSinglePlayer.n_turrets = gameStateSinglePlayer.difficulty * 4;
        gameStateSinglePlayer.n_collect_points = 0;
        gameStateSinglePlayer.powerup_drop_chance[5] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[6] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[7] = GAME_SUBTYPE_LIFE;
        gameStateSinglePlayer.powerup_drop_chance[8] = GAME_SUBTYPE_ALLY;
        gameStateSinglePlayer.ship_destruction_change_alliance = 0;
        gameStateSinglePlayer.enemy_durability = 3;
        
        gameStateSinglePlayer.rate_enemy_skill_increase = 1.0/10;
        gameStateSinglePlayer.rate_enemy_count_increase = 1.0/10;
        gameStateSinglePlayer.rate_point_increase = 1;
        gameStateSinglePlayer.enemy_durability = 3;
        
        gameStateSinglePlayer.points_per_second_elapsed = 1;
        
        // different map for "collection" game typ
        gameMapSetMap(initial_map_survival);
        
        gameStateSinglePlayer.invuln_time_after_hit = 3;
    }
    else if(gameStateSinglePlayer.game_type == GAME_TYPE_SPEEDRUN)
    {
        sprintf(dialogStr, "run!\n"
                "**** High Score: %d ****",
                gameStateSinglePlayer.high_score[gameStateSinglePlayer.game_type]);
        gameDialogDisplayString(dialogStr);
        
        game_reset();
        
        gameStateSinglePlayer.max_enemies = 3 + difficulty;
        gameStateSinglePlayer.n_turrets = gameStateSinglePlayer.difficulty * 10;
        gameStateSinglePlayer.n_collect_points = 0;
        gameStateSinglePlayer.powerup_drop_chance[5] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[6] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[7] = GAME_SUBTYPE_LIFE;
        gameStateSinglePlayer.powerup_drop_chance[8] = GAME_SUBTYPE_ALLY;
        gameStateSinglePlayer.ship_destruction_change_alliance = 0;
        gameStateSinglePlayer.enemy_durability = 3;
        
        gameStateSinglePlayer.rate_enemy_skill_increase = 1.0/10;
        gameStateSinglePlayer.rate_enemy_count_increase = 1.0/10;
        gameStateSinglePlayer.rate_point_increase = 1;
        gameStateSinglePlayer.enemy_durability = 3;
        
        gameStateSinglePlayer.points_per_second_elapsed = 1;
        
        gameMapSetMap(map_speedrun);
        
        gameStateSinglePlayer.invuln_time_after_hit = 0;
    }
    else if(gameStateSinglePlayer.game_type == GAME_TYPE_DEFEND)
    {
        strcpy(gameStateSinglePlayer.game_help_message, "defend ally");
        
        game_reset();
        
        gameStateSinglePlayer.max_enemies = 3 + difficulty;
        gameStateSinglePlayer.n_turrets = gameStateSinglePlayer.difficulty * 4;
        gameStateSinglePlayer.n_collect_points = 0;
        gameStateSinglePlayer.powerup_drop_chance[5] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[6] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[7] = GAME_SUBTYPE_LIFE;
        gameStateSinglePlayer.powerup_drop_chance[8] = GAME_SUBTYPE_ALLY;
        gameStateSinglePlayer.ship_destruction_change_alliance = 0;
        gameStateSinglePlayer.enemy_durability = 3;
        
        gameStateSinglePlayer.rate_enemy_skill_increase = 1/10;
        
        gameStateSinglePlayer.invuln_time_after_hit = 3;
        
        // different map for "collection" game typ
        //gameMapSetMapData(initial_map_survival);
    }
    else if(gameStateSinglePlayer.game_type == GAME_TYPE_TURRET)
    {
        strcpy(gameStateSinglePlayer.game_help_message, "defend your base!");
        
        game_reset();
        
        gameStateSinglePlayer.max_enemies = 3 + difficulty;
        gameStateSinglePlayer.n_turrets = 0;
        gameStateSinglePlayer.n_collect_points = 0;
        gameStateSinglePlayer.powerup_drop_chance[5] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[6] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[7] = GAME_SUBTYPE_LIFE;
        gameStateSinglePlayer.powerup_drop_chance[8] = GAME_SUBTYPE_ALLY;
        gameStateSinglePlayer.ship_destruction_change_alliance = 0;
        gameStateSinglePlayer.enemy_durability = 3;
        
        gameStateSinglePlayer.rate_enemy_skill_increase = 1/10;
        
        gameStateSinglePlayer.invuln_time_after_hit = 3;
        
        gameStateSinglePlayer.enemy1_ignore_player_pct = 100;
        
        gameMapSetMap(initial_map_turret);
    }
    else if(gameStateSinglePlayer.game_type == GAME_TYPE_LOBBALL)
    {
        sprintf(dialogStr, "^D^D^D1. Throw energy balls!^D^D^D\n"
                "^D^D^D   2. SURVIVE   ^D^D^D\n"
                "^D^D^D       3. PROFIT!!! ^D       ^D^D^D\n"
                "****   High Score: %d  ****",
                gameStateSinglePlayer.high_score[gameStateSinglePlayer.game_type]);
        gameDialogDisplayString(dialogStr);
        
        game_reset();
        
        gameStateSinglePlayer.max_enemies = 3 + difficulty;
        gameStateSinglePlayer.n_turrets = 0;
        gameStateSinglePlayer.n_collect_points = 0;
        gameStateSinglePlayer.powerup_drop_chance[5] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[6] = GAME_SUBTYPE_MISSLE;
        gameStateSinglePlayer.powerup_drop_chance[7] = GAME_SUBTYPE_LIFE;
        gameStateSinglePlayer.powerup_drop_chance[8] = GAME_SUBTYPE_ALLY;
        gameStateSinglePlayer.ship_destruction_change_alliance = 0;
        gameStateSinglePlayer.enemy_durability = 3;
        
        gameStateSinglePlayer.counter_enemies_spawned = /* 3 + difficulty * 2 */ 9999;
        gameStateSinglePlayer.enemy_spawnpoint_interval = 5;
        gameStateSinglePlayer.log_event_camwatch[GAME_LOG_NEWDATA] = 3;
        
        gameStateSinglePlayer.rate_enemy_skill_increase = 1.0/10;
        gameStateSinglePlayer.rate_enemy_count_increase = 1.0/20;
        
        gameStateSinglePlayer.invuln_time_after_hit = 3;
        
        gameStateSinglePlayer.enemy1_ignore_player_pct = 0;
        
        gameMapSetMap(pokeball_map);
    }
    
    if(GAME_AI_DEBUG)
    {
        gameStateSinglePlayer.n_turrets = 0;
        gameStateSinglePlayer.max_enemies = 1;
    }
    
    gameAIState.started = 1;
    
    gameStateSinglePlayer.time_elapsed = 0;
    gameStateSinglePlayer.end_time = time_ms + (300 * 1000); // 5 min
    
    console_write(gameStateSinglePlayer.game_help_message);
    //console_append("\n***high score this session: %d***",
    //               gameStateSinglePlayer.high_score_session[gameStateSinglePlayer.game_type]);
    console_append("\n***record high score: %d***",
                   gameStateSinglePlayer.high_score[gameStateSinglePlayer.game_type]);
    
    for(int i = 0; i < 4; i++) gameStateSinglePlayer.elem_id_arrow3[i] = WORLD_ELEM_ID_INVALID;
}

void
path_generate_random_with_delta(float cur[3], float vec[3], float mag, float delt)
{
    float out[3];
    
    for(int d = 0; d < 3; d++)
    {
        out[d] = cur[d] + (vec[d] * mag) + rand_in_range(-1, 1)*delt;
    }
    
    float dist = sqrt((out[0]-cur[0])*(out[0]-cur[0])+(out[1]-cur[1])*(out[1]-cur[1])+(out[2]-cur[2])*(out[2]-cur[2]));
    
    for(int d = 0; d < 3; d++)
    {
        vec[d] = (out[d] - cur[d]) / dist;
        cur[d] = out[d];
    }
}

static void
game_setup()
{
    int i;
    float vp[3];
    float pos[3];
    float boundmin[3];
    float boundmax[3];
    float origin[3] = {0, 0, 0};
    
    vp[0] = vp[1] = vp[2] = 0;
    vp[0] = 1;
    
    WorldElem *elemSpawnPoint = world_find_elem_with_attrs(&gWorld->elements_list, OBJ_SPAWNPOINT, AFFILIATION_NONE);
    if(elemSpawnPoint)
    {
        origin[0] = elemSpawnPoint->physics.ptr->x;
        origin[1] = elemSpawnPoint->physics.ptr->y;
        origin[2] = elemSpawnPoint->physics.ptr->z;
    }
    
    for(i = 0; i < 3; i++) { boundmin[i] = -gWorld->bound_radius+1; boundmax[i] = gWorld->bound_radius-1; }
    
    int grouplen = 1;
    
    for(int i = 0; i < gameStateSinglePlayer.n_collect_points; i++)
    {
        /*
        int td = 10; // turret dist
        int c[3] =
        {
            rand_in_range(td, gWorld->bound_x-td),
            rand_in_range(10, gWorld->bound_y*0.4),
            rand_in_range(td, gWorld->bound_z-td)
        };
         */
        
        // for each collection point, spawn some number of caps
        for(int j = 0; j < grouplen; j++)
        {
            if(j == 0)
            {
                pos[0] = rand_in_range(boundmin[0], boundmax[0]);
                pos[1] = rand_in_range(boundmin[1], boundmax[1]);
                pos[2] = rand_in_range(boundmin[2], boundmax[2]);
            }
            
            float mag = 10;
            float delt = 2;
            path_generate_random_with_delta(pos, vp, mag, delt);
            
            for(int k = 0; k < 3; k++)
            {
                if(pos[k] <= boundmin[k]) {pos[k] += mag; vp[k] = -vp[k];}
                if(pos[k] >= boundmax[k]) {pos[k] -= mag; vp[k] = -vp[k];}
            }
            int c[3] = {pos[0], pos[1], pos[2]};

            game_add_powerup(c[0], c[1], c[2], GAME_SUBTYPE_COLLECT, 0);
            
            /*
            game_add_turret(c[0] + ((rand() % (td*2))-td),
                            c[1] + ((rand() % (td*2))-td),
                            c[2] + ((rand() % (td*2))-td));
             */
        }
    }
    
    if(gameStateSinglePlayer.game_type == GAME_TYPE_DEFEND)
    {
        gameStateSinglePlayer.defend_id = game_add_ally(rand_in_range(-gWorld->bound_radius, gWorld->bound_radius),
                                                        rand_in_range(-gWorld->bound_radius, gWorld->bound_radius),
                                                        rand_in_range(-gWorld->bound_radius, gWorld->bound_radius));
        world_get_last_object()->durability = 50;
        world_get_last_object()->stuff.u.enemy.max_speed = MAX_SPEED;
    }
    else if(gameStateSinglePlayer.game_type == GAME_TYPE_COLLECT)
    {
        /*
        game_add_ally(rand_in_range(1, gWorld->bound_x),
                      rand_in_range(1, gWorld->bound_y),
                      rand_in_range(1, gWorld->bound_z));
        world_get_last_object()->stuff.u.enemy.changes_target = 0;
        world_get_last_object()->stuff.u.enemy.deploys_collect = 1;
        world_get_last_object()->durability = 999999;
         */
    }
}

void
game_handle_collision_powerup(WorldElem* elemA, WorldElem* elemB)
{
    int do_data_grab_score_immediate = 0;
    WorldElemListNode* myShipListNode = world_elem_list_find(my_ship_id, &gWorld->elements_moving);
    if(!myShipListNode) return;

    int collect_sound = 0;
    switch(elemB->stuff.subtype)
    {
        case GAME_SUBTYPE_LIFE:
            {
                elemA->durability = DURABILITY_PLAYER;
                world_remove_object(elemB->elem_id);
                collect_sound = 1;
            }
            break;
            
        case GAME_SUBTYPE_MISSLE:
            {
                game_ammo_missles = game_ammo_missles_max;
                world_remove_object(elemB->elem_id);
                collect_sound = 1;
            }
            break;
            
        case GAME_SUBTYPE_COLLECT:
            if(elemA->object_type == OBJ_PLAYER)
            {
                world_remove_object(elemB->elem_id);
                gameStateSinglePlayer.stats.data_collected++;
                collect_sound = 1;
                camera_locked_frames = 120;
                
                // points-indicator
                world_add_object(MODEL_CUBE2,
                                 elemB->physics.ptr->x,
                                 elemB->physics.ptr->y,
                                 elemB->physics.ptr->z,
                                 0, 0, 0, elemB->scale, TEXTURE_ID_DATA_GRABBED);
                world_get_last_object()->object_type = OBJ_WRECKAGE;
                world_object_set_lifetime(world_get_last_object()->elem_id, 60);
                update_object_velocity(world_get_last_object()->elem_id,
                                       elemA->physics.ptr->vx * 1.5,
                                       elemA->physics.ptr->vy * 1.5,
                                       elemA->physics.ptr->vz * 1.5,
                                       0);
                
                world_add_object(MODEL_CUBE2,
                                 elemB->physics.ptr->x,
                                 elemB->physics.ptr->y,
                                 elemB->physics.ptr->z,
                                 0, 0, 0, elemB->scale, TEXTURE_ID_DATA_GRABBED);
                world_get_last_object()->object_type = OBJ_DISPLAYONLY;
                
                console_write(game_log_messages[GAME_LOG_FILESAVED]);
            }
            else 
            {
                if(elemA->stuff.towed_elem_id == WORLD_ELEM_ID_INVALID)
                {
                    if(elemA->elem_id != my_ship_id)
                    {
                        console_write(game_log_messages[GAME_LOG_WARN_CAPTURE]);
                        if(gameStateSinglePlayer.log_event_camwatch[GAME_LOG_WARN_CAPTURE] > 0)
                        {
                            gameStateSinglePlayer.log_event_camwatch[GAME_LOG_WARN_CAPTURE]--;
                            glFlightCamFix(elemA->elem_id);
                        }
                    }
                    else
                    {
                        camera_locked_frames = 120;
                    }
                    
                    elemA->stuff.towed_elem_id = elemB->elem_id;
                    collect_sound = 1;
                }
            }
            break;
            
        case GAME_SUBTYPE_POINTS:
            world_remove_object(elemB->elem_id);
            console_write(game_log_messages[GAME_LOG_POWERUP_POINTS]);
            collect_sound = 1;
            break;
            
        default:
            {
                world_remove_object(elemB->elem_id);
                collect_sound = 1;
            }
            break;
    }
    
    if(collect_sound)
    {
        gameAudioPlaySoundAtLocation("collect",
                                     myShipListNode->elem->physics.ptr->x,
                                     myShipListNode->elem->physics.ptr->y,
                                     myShipListNode->elem->physics.ptr->z);
    }
}

void
game_handle_collision(WorldElem* elemA, WorldElem* elemB, int collision_action)
{
    int vibrate_on_collide = 0;
    
    if(collision_action != COLLISION_ACTION_DAMAGE && collision_action != COLLISION_ACTION_FLAG) return;
    
    if(!gameStateSinglePlayer.started) return;
    
    if(elemA->remove_pending || elemB->remove_pending) return;
    
    WorldElemListNode* myShipListNode = world_elem_list_find(my_ship_id, &gWorld->elements_moving);
    if(!myShipListNode) return;
    
    if(elemA->object_type == OBJ_BULLET || elemA->object_type == OBJ_MISSLE)
    {
        game_collision_last_bullet_affiliation = elemA->stuff.affiliation;
        
        if(elemB == myShipListNode->elem)
        {
            if(vibrate_on_collide)
            {
                gameAudioPlaySoundAtLocation("vibrate",
                                             myShipListNode->elem->physics.ptr->x,
                                             myShipListNode->elem->physics.ptr->y,
                                             myShipListNode->elem->physics.ptr->z);
            }
            
            if(gameStateSinglePlayer.invuln_time_after_hit)
            {
                collision_actions_set_player_invuln();
                invuln_count = gameStateSinglePlayer.invuln_time_after_hit;
            }
        }
        
        if(elemB->object_type == OBJ_SHIP)
        {
            elemB->stuff.towed_elem_id = WORLD_ELEM_ID_INVALID;
        }
    }
    
    if(elemA->object_type == OBJ_PLAYER)
    {
        if(elemB->stuff.affiliation == elemA->stuff.affiliation &&
           gameStateSinglePlayer.object_types_towed[elemB->object_type] &&
           (gameNetworkState.hostInfo.hosting || !gameNetworkState.connected))
        {
            elemA->stuff.towed_elem_id = elemB->elem_id;
        }
    }
    
    switch(elemB->object_type)
    {
        case OBJ_POWERUP_GENERIC:
            {
                game_handle_collision_powerup(elemA, elemB);
            }
            break;
            
        case OBJ_SPAWNPOINT:
            if(elemA->object_type == OBJ_PLAYER &&
               gameStateSinglePlayer.collects_found == 0 &&
               new_level)
            {
                new_level = 0;
                
                gameMapSetMap(initial_map_collection);
                game_delay_frames = GAME_FRAME_RATE;
                
                game_start(gameStateSinglePlayer.difficulty+1, gameStateSinglePlayer.game_type);
                
                console_write("Level:%d", gameStateSinglePlayer.difficulty);
            }
            
            if(elemA->object_type == OBJ_POWERUP_GENERIC)
            {
                if(elemA->stuff.subtype == GAME_SUBTYPE_COLLECT)
                {
                    world_remove_object(elemA->elem_id);
                    
                    gameStateSinglePlayer.stats.data_collected++;
                    
                    camera_locked_frames = 120;
                    
                    gameAudioPlaySoundAtLocation("dropoff",
                                                 myShipListNode->elem->physics.ptr->x,
                                                 myShipListNode->elem->physics.ptr->y,
                                                 myShipListNode->elem->physics.ptr->z);
                    
                    int obj_id =
                        world_add_object(MODEL_CUBE2,
                                         elemB->physics.ptr->x,
                                         elemB->physics.ptr->y,
                                         elemB->physics.ptr->z,
                                         elemB->physics.ptr->alpha,
                                         elemB->physics.ptr->beta,
                                         elemB->physics.ptr->gamma,
                                         elemB->scale/2, TEXTURE_ID_DATA_GRABBED);
                    world_get_last_object()->object_type = OBJ_BLOCK;
                    
                    world_get_last_object()->destructible = 0;
                    world_object_set_lifetime(obj_id, 60);
                    
                    console_write(game_log_messages[GAME_LOG_FILESAVED]);
                    
                    gameStateSinglePlayer.collect_system_integrity += 10;
                    if(gameStateSinglePlayer.collect_system_integrity >= 100)
                    {
                        gameStateSinglePlayer.collect_system_integrity = 100;
                    }
                    
                    int powerup_obj_id =
                        game_add_powerup(rand_in_range(-gWorld->bound_radius, gWorld->bound_radius),
                                     rand_in_range(-gWorld->bound_radius, gWorld->bound_radius),
                                     rand_in_range(-gWorld->bound_radius, gWorld->bound_radius),
                                     GAME_SUBTYPE_COLLECT, 0);
                    
                }
            }
            break;
            
        case OBJ_SPAWNPOINT_ENEMY:
            if(elemA->object_type == OBJ_POWERUP_GENERIC)
            {
                elemA->stuff.u.spawnpoint.spawn_intelligence *= 1.5;
                elemB->stuff.u.spawnpoint.spawn_coeff += rand_in_range(-INT_MAX/2, INT_MAX) / (float) INT_MAX;
                
                gameStateSinglePlayer.collect_system_integrity -= 20;
                
                console_write(game_log_messages[GAME_LOG_FILELOST]);
                gameDialogMessagePopup(game_log_messages[GAME_LOG_FILELOST]);
                
                glFlightDrawframeHook = gameDialogGraphicDangerCountdown;
                
                // add a marker
                {
                    int obj_id =
                    world_add_object(MODEL_SPRITE,
                                     elemA->physics.ptr->x,
                                     elemA->physics.ptr->y,
                                     elemA->physics.ptr->z,
                                     elemA->physics.ptr->alpha,
                                     elemA->physics.ptr->beta,
                                     elemA->physics.ptr->gamma,
                                     elemA->scale, TEXTURE_ID_FILELOST);
                    world_get_last_object()->object_type = OBJ_BLOCK;
                    
                    world_get_last_object()->destructible = 0;
                    //world_object_set_lifetime(obj_id, 15);
                    update_object_velocity(obj_id, 0, 0, 0, 0);
                }
                
                world_remove_object(elemA->elem_id);
                gameAudioPlaySoundAtLocationWithRate("filelost", 1.0, gameCamera_getX(), gameCamera_getY(), gameCamera_getZ(), 1.0);
            }
            break;
            
        default:
            break;
    }
}

void
game_handle_destruction(WorldElem* elem)
{
    if(!gameStateSinglePlayer.started) return;
    int add_wreck = 0;
    int add_pow = 0;
    int add_score = 1;
    int score_snap = gameStateSinglePlayer.stats.score;
    char score_buf[255];
    
    WorldElemListNode* myShipListNode = world_elem_list_find(my_ship_id, &gWorld->elements_moving);
    if(!myShipListNode) return;
    
    switch(elem->object_type)
    {
        case OBJ_SHIP:
            {
                add_wreck = 1;
                add_pow = 1;
                
                score_kill(elem);
                
                // add powerup
                if(gameStateSinglePlayer.ship_destruction_change_alliance)
                {
                    WorldElem* elemPlayer = world_find_elem_with_attrs(&gWorld->elements_list,
                                                                       OBJ_PLAYER,
                                                                       game_collision_last_bullet_affiliation);
                    if(elemPlayer)
                    {
                        game_add_enemy(elem->physics.ptr->x, elem->physics.ptr->y, elem->physics.ptr->z);
                        world_get_last_object()->stuff.affiliation = game_collision_last_bullet_affiliation;
                        world_get_last_object()->texture_id = elemPlayer->texture_id;
                    }
                    add_wreck = 0;
                }
            }
            break;
            
        case OBJ_TURRET:
            {
                add_wreck = 0;
                add_pow = 1;
                
                score_kill(elem);
            }
            break;
            
        case OBJ_POWERUP_GENERIC:
            switch(elem->stuff.subtype)
            {
                case GAME_SUBTYPE_COLLECT:
                    break;
                    
                case GAME_SUBTYPE_LIFE:
                    myShipListNode->elem->durability = DURABILITY_PLAYER;
                    break;
                    
                case GAME_SUBTYPE_TURRET:
                    myShipListNode->elem->stuff.flags.mask |= STUFF_FLAGS_TURRET;
                    fireAction = ACTION_PLACE_TURRET;
                    gameInterfaceControls.menuControl.visible = 1;
                    break;
            }
            break;
            
        case OBJ_PLAYER:
            if(gameStateSinglePlayer.game_type != GAME_TYPE_DEATHMATCH)
            {
                console_write(game_log_messages[GAME_LOG_GAMEOVER_KILLED]);
                game_over();
            }
            
            if(elem->stuff.flags.mask &= STUFF_FLAGS_TURRET)
            {
                world_add_object(MODEL_CUBE2, elem->physics.ptr->x, elem->physics.ptr->y, elem->physics.ptr->z,
                                 0, 0, 0, 1, TEXTURE_ID_POWERUP);
                world_get_last_object()->object_type = OBJ_POWERUP_GENERIC;
                world_get_last_object()->stuff.subtype = GAME_SUBTYPE_TURRET;
                world_get_last_object()->durability = 0;
            }

            if(elem->stuff.flags.mask &= STUFF_FLAGS_COLLECT)
            {
                game_add_powerup(elem->physics.ptr->x,
                                 elem->physics.ptr->y,
                                 elem->physics.ptr->z,
                                 GAME_SUBTYPE_COLLECT, 0);
                
            }
            
            if(gameStateSinglePlayer.player_drops_powerup)
            {
                add_pow = 1;
            }
            break;
            
        default:
            break;
    }
    
    if(add_pow)
    {
        if(game_powerup_add_random(elem->physics.ptr->x,
                                   elem->physics.ptr->y,
                                   elem->physics.ptr->z)
           != WORLD_ELEM_ID_INVALID)
        {
            add_wreck = 0;
        }
        
    }
    
    if(add_wreck)
    {
        int wreck_id;
        // add wreckage
        wreck_id =
        world_add_object(MODEL_SPRITE, elem->physics.ptr->x, elem->physics.ptr->y, elem->physics.ptr->z,
                         0, 0, 0, 1, TEXTURE_ID_WRECKAGE);
        world_get_last_object()->destructible = 0;
        world_get_last_object()->object_type = OBJ_WRECKAGE;
        world_object_set_lifetime(wreck_id, GAME_FRAME_RATE * 5);
        update_object_velocity(wreck_id, 0, 0, 0, 0);
        
        if(add_score)
        {
            score_total();
            sprintf(score_buf, "%d", gameStateSinglePlayer.stats.score - score_snap);
            world_object_set_nametag(wreck_id, score_buf);
        }
    }
    
    if(elem->elem_id == gameStateSinglePlayer.defend_id)
    {
        gameInterfaceControls.mainMenu.visible = 1;
        console_write("game over!");
        score_display();
        game_reset_needed = 1;
        
        gameAudioPlaySoundAtLocation("vibrate",
                                     myShipListNode->elem->physics.ptr->x,
                                     myShipListNode->elem->physics.ptr->y,
                                     myShipListNode->elem->physics.ptr->z);
    }
}

void
game_run()
{
    WorldElemListNode* pMyShipNode;
    float ship_z_vec[3];
    static game_timeval_t game_run_last;
    float p[3] = {my_ship_x, my_ship_y, my_ship_z};
    float v[3], r[3];
    int target_types[] = {OBJ_SHIP, OBJ_PLAYER, OBJ_TURRET, OBJ_UNKNOWN};
    
    static game_timeval_t logSoundTimeLast = 0;
    static int game_target_enemy_near = WORLD_ELEM_ID_INVALID;
    WorldElem* pElemBallHeld = NULL;
    
    if(game_delay_frames > 0)
    {
        game_delay_frames--;
        return;
    }
    
    pMyShipNode = world_elem_list_find(my_ship_id, &gWorld->elements_moving);
    if(!pMyShipNode) return;
    
    if(time_ms - game_run_last >= 1000/GAME_FRAME_RATE)
    {
        gameStateSinglePlayer.time_elapsed += 1000/GAME_FRAME_RATE;
        
        if(game_setup_needed)
        {
            game_setup();
            game_init_objects();
            game_setup_needed = 0;
            gameStateSinglePlayer.started = 1;
            return;
        }
        
        game_run_last = time_ms;
        
        WorldElemListNode* pCur;
        float enemies_found = 0;
        int turrets_found = 0;
        int missles_found = 0;
        int spawn_points_found = 0;
        int caps_found = 0;
        int caps_owned = 0;
        int collects_found = 0;
        int asteroids_found = 0;
        float objective_target_dist = 99999999;
        static int caps_found_last = 0;
        int obj_id_enemy_spawn = WORLD_ELEM_ID_INVALID;
        int obj_id_friendly_spawn = WORLD_ELEM_ID_INVALID;
        int obj_id_base = WORLD_ELEM_ID_INVALID;
        float enemy_dist_near = 0;
        
        
        pCur = gWorld->elements_moving.next;
        while(pCur)
        {
            switch(pCur->elem->object_type)
            {                    
                case OBJ_SHIP:
                    enemies_found++;
                    {
                        float d = distance(my_ship_x, my_ship_y, my_ship_z,
                                           pCur->elem->physics.ptr->x,
                                           pCur->elem->physics.ptr->y,
                                           pCur->elem->physics.ptr->z);
                        if(enemy_dist_near == 0 || d < enemy_dist_near)
                        {
                            enemy_dist_near = d;
                            game_target_enemy_near = pCur->elem->elem_id;
                        }
                    }
                    break;
                    
                case OBJ_MISSLE:
                    missles_found++;
                    break;
                    
                case OBJ_TURRET: {
                        turrets_found++;
                    }
                    break;
                    
                case OBJ_SPAWNPOINT:
                    obj_id_friendly_spawn = pCur->elem->elem_id;
                    break;
                    
                case OBJ_SPAWNPOINT_ENEMY:
                {
                    float rand_range = 65536;
                    spawn_points_found++;
                    obj_id_enemy_spawn = pCur->elem->elem_id;
                    
                    // mark: -- spawn enemies randomly
                    pCur->elem->stuff.u.spawnpoint.spawn_coeff += rand_in_range(-rand_range*0.5, rand_range) / (rand_range*100);
                    
                    if (gameStateSinglePlayer.counter_enemies_spawned > 0 &&
                        enemies_found < floor(gameStateSinglePlayer.max_enemies) &&
                        pCur->elem->stuff.u.spawnpoint.spawn_coeff > 1.0)
                    {
                        float spawn_rand = roundf(rand_in_range(0, rand_range));
                        
                        pCur->elem->stuff.u.spawnpoint.spawn_coeff = 0;
                        gameStateSinglePlayer.counter_enemies_spawned--;
                        
                        // TODO: spawn faster
                        pCur->elem->stuff.u.spawnpoint.spawn_intelligence *= 1.1;
                        
                        if(spawn_rand > (rand_range/3) * 2)
                        {
                            game_add_enemy_bountyhunter(pCur->elem->physics.ptr->x,
                                                        pCur->elem->physics.ptr->y,
                                                        pCur->elem->physics.ptr->z);
                            world_get_last_object()->stuff.u.enemy.intelligence =
                            pCur->elem->stuff.u.spawnpoint.spawn_intelligence * 2;
                        }
                        else if(spawn_rand > (rand_range/3))
                        {
                            game_add_turret(pCur->elem->physics.ptr->x, pCur->elem->physics.ptr->y, pCur->elem->physics.ptr->z);
                            world_get_last_object()->stuff.u.enemy.intelligence =
                            pCur->elem->stuff.u.spawnpoint.spawn_intelligence;
                        }
                        else
                        {
                            game_add_enemy(pCur->elem->physics.ptr->x,
                                           pCur->elem->physics.ptr->y,
                                           pCur->elem->physics.ptr->z);
                            world_get_last_object()->stuff.u.enemy.intelligence =
                            pCur->elem->stuff.u.spawnpoint.spawn_intelligence;
                        }
                        
                        world_get_last_object()->stuff.affiliation = pCur->elem->stuff.affiliation;
                    }
                    
                    game_move_spawnpoint(pCur->elem);
                }
                break;
                
                case OBJ_BASE:
                    obj_id_base = pCur->elem->elem_id;
                    pCur->elem->renderInfo.priority = 1;
                    break;
                    
                case OBJ_POWERUP_GENERIC:
                    if(pCur->elem->stuff.subtype == GAME_SUBTYPE_COLLECT)
                    {
                        collects_found++;
                    }
                    break;
                    
                case OBJ_BLOCK:
                case OBJ_BLOCK_MOVING:
                    if(pCur->elem->stuff.subtype == GAME_SUBTYPE_ASTEROID)
                    {
                        asteroids_found++;
                    }
                    
                    if(pCur->elem->object_type == OBJ_BLOCK_MOVING)
                    {
                        float orbit_origin[] = {0, atof(MAP_BASE_ALT), 0};
                        
                        orbit_around(pCur->elem, orbit_origin, pCur->elem->stuff.u.orbiter.theta, pCur->elem->stuff.u.orbiter.radius);
                        
                        pCur->elem->stuff.u.orbiter.theta += M_PI / (pCur->elem->stuff.u.orbiter.period * GAME_FRAME_RATE);
                    }
                    break;
                    
                case OBJ_BULLET:
                    break;
                    
                case OBJ_PLAYER:
                    break;
                    
                case OBJ_TOUCHCONTROLBALL:
                    if(!pCur->elem->physics.ptr->gravity)
                    {
                        pElemBallHeld = pCur->elem;
                    }
                    break;
                    
                case OBJ_WRECKAGE:
                    {
                        world_replace_object(pCur->elem->elem_id, pCur->elem->type,
                                             pCur->elem->physics.ptr->x, pCur->elem->physics.ptr->y, pCur->elem->physics.ptr->z,
                                             0, 0, 0, pCur->elem->scale * 1.01, pCur->elem->texture_id);
                    }
                    break;
                    
                default:
                    break;
            }
            
            if(pCur->elem->stuff.towed_elem_id != WORLD_ELEM_ID_INVALID)
            {
                WorldElemListNode *listNodeTowed =
                    world_elem_list_find(pCur->elem->stuff.towed_elem_id, &gWorld->elements_list);
                
                if(listNodeTowed)
                {
                    quaternion_t bx, by, bz;
                    
                    get_body_vectors_for_euler(pCur->elem->physics.ptr->alpha,
                                               pCur->elem->physics.ptr->beta,
                                               pCur->elem->physics.ptr->gamma,
                                               &bx, &by, &bz);
                    float tow_v[] =
                    {
                        pCur->elem->physics.ptr->x + (bz.x*4.0 - by.x*1) - listNodeTowed->elem->physics.ptr->x,
                        pCur->elem->physics.ptr->y + (bz.y*4.0 - by.y*1) - listNodeTowed->elem->physics.ptr->y,
                        pCur->elem->physics.ptr->z + (bz.z*4.0 - by.z*1) - listNodeTowed->elem->physics.ptr->z
                    };
                    float tow_d = sqrt(tow_v[0]*tow_v[0] + tow_v[1]*tow_v[1] + tow_v[2]*tow_v[2]);
                    float tow_m = tow_d * /*4.5*/ 1.0;
                    
                    tow_v[0] = (tow_v[0]/tow_d);
                    tow_v[1] = (tow_v[1]/tow_d);
                    tow_v[2] = (tow_v[2]/tow_d);
                    
                    update_object_velocity(pCur->elem->stuff.towed_elem_id,
                                           tow_v[0]*tow_m, tow_v[1]*tow_m, tow_v[2]*tow_m, 1);
                    
                    if(pCur->elem->elem_id == my_ship_id)
                    {
                        /*
                        gameAudioPlaySoundAtLocation("towing",
                                                     pCur->elem->physics.ptr->x,
                                                     pCur->elem->physics.ptr->y,
                                                     pCur->elem->physics.ptr->z);
                         */
                    }
                    
                    if(tow_d > TOW_DISTANCE_MAX) pCur->elem->stuff.towed_elem_id = WORLD_ELEM_ID_INVALID;
                }
                else
                {
                    pCur->elem->stuff.towed_elem_id = WORLD_ELEM_ID_INVALID;
                }
            }
            
            if(pCur->elem->elem_id == my_ship_id)
            {
                if(gameStateSinglePlayer.game_type == GAME_TYPE_TURRET)
                {
                    world_move_elem(pCur->elem, 0, 0, 0, 0);
                }
            }
            
            pCur = pCur->next;
        }
        
        // once per second
        if(time_ms - gameStateSinglePlayer.last_run >= 1000)
        {
            gameStateSinglePlayer.last_run = time_ms;
            
            game_target_objective_id = gameStateSinglePlayer.elem_id_spawnpoint;
            
            if(strncmp(gameSettingsPlayerName, "god", 3) == 0) pMyShipNode->elem->durability = 9999;
         
            if(gameStateSinglePlayer.started)
            {
                // pass 2 - actions
                pCur = gWorld->elements_moving.next;
                while(pCur)
                {
                    float dist = distance(pCur->elem->physics.ptr->x,
                                          pCur->elem->physics.ptr->y,
                                          pCur->elem->physics.ptr->z,
                                          my_ship_x,
                                          my_ship_y,
                                          my_ship_z);
                    
                    switch(pCur->elem->object_type)
                    {
                        case OBJ_SPAWNPOINT:
                            pCur->elem->stuff.affiliation = gameNetworkState.my_player_id;
                            gameStateSinglePlayer.elem_id_spawnpoint = pCur->elem->elem_id;
                            game_move_spawnpoint(pCur->elem);
                            break;
                            
                        case OBJ_POWERUP_GENERIC:
                            if(pCur->elem->stuff.subtype == GAME_SUBTYPE_COLLECT)
                            {
                                if(pMyShipNode->elem->stuff.towed_elem_id == WORLD_ELEM_ID_INVALID)
                                {
                                    if(time_ms - gameInterfaceControls.action.touch_end_last > 1000)
                                    {
                                        if(dist < objective_target_dist)
                                        {
                                            objective_target_dist = dist;
                                            game_target_objective_id = pCur->elem->elem_id;
                                        }
                                    }
                                }
                            }
                            break;
                            
                        default:
                            break;
                    }
                    pCur = pCur->next;
                }
            
                gameStateSinglePlayer.caps_owned = caps_owned;
                gameStateSinglePlayer.collects_found = collects_found;
                
                if(gameStateSinglePlayer.spawn_powerup_m &&
                   rand_in_range(1, gameStateSinglePlayer.spawn_powerup_m) == 1)
                {
                    game_add_powerup(rand_in_range(-gWorld->bound_radius, gWorld->bound_radius),
                                     rand_in_range(-gWorld->bound_radius, gWorld->bound_radius),
                                     rand_in_range(-gWorld->bound_radius, gWorld->bound_radius),
                                     GAME_SUBTYPE_LIFE, 0);
                    
                    world_object_set_lifetime(world_get_last_object()->elem_id,
                                              gameStateSinglePlayer.spawn_powerup_lifetime);
                }
                
                /*
                if(caps_found < 4 && rand() % 10 == 0)
                {
                    game_add_cap(rand() % (int) gWorld->bound_x, rand() % (int) gWorld->bound_y, rand() % (int) gWorld->bound_z);
                }
                 */
                
                /*
                if(missles_found < 3)
                {
                    game_add_missle(rand_in_range(1, gWorld->bound_x),
                                    rand_in_range(1, gWorld->bound_y),
                                    rand_in_range(1, gWorld->bound_z));
                }
                 */
                
                if(asteroids_found < gameStateSinglePlayer.n_asteroids)
                {
                    float asteroid[] =
                    {
                        -gWorld->bound_radius + (rand() % (int) gWorld->bound_radius*2),
                        -gWorld->bound_radius + (rand() % (int) gWorld->bound_radius*2),
                        -gWorld->bound_radius + (rand() % (int) gWorld->bound_radius*2)
                    };
                    game_add_asteroid(asteroid[0], asteroid[1], asteroid[2], asteroid[0], 0, asteroid[2]);
                    world_get_last_object()->physics.ptr->gravity = 1;
                }
                
                if(gameStateSinglePlayer.game_type == GAME_TYPE_COLLECT)
                {
                    /*
                    if(collects_found <= 0)
                    {
                        if(console_log_search(game_log_messages[GAME_LOG_GAMEOVER], 0) == NULL)
                        {
                            console_write(game_log_messages[GAME_LOG_GAMEOVER]);
                            game_over();
                        }
                        
                        if(gameStateSinglePlayer.game_action_spawnpoint_new_game &&
                           !new_level)
                        {
                            console_write("LEVEL clear\nReturn to spawnpoint\n");
                            score_display();
                            
                            gameAudioPlaySoundAtLocation("victory",
                                                         pMyShipNode->elem->physics.ptr->x,
                                                         pMyShipNode->elem->physics.ptr->y,
                                                         pMyShipNode->elem->physics.ptr->z);
                            
                            if(gameStateSinglePlayer.delay_new_level)
                            {
                                new_level = 1;
                            }
                            else
                            {
                                gameMapSetMap(initial_map_collection);
                                game_delay_frames = 60;
                                
                                game_start(gameStateSinglePlayer.difficulty+1, gameStateSinglePlayer.game_type);
                                
                                console_write("Level:%d", gameStateSinglePlayer.difficulty);
                            }
                        }
                        
                        float loc_powerup[3] =
                        {
                            rand_in_range(1, gWorld->bound_x),
                            rand_in_range(1, gWorld->bound_y),
                            rand_in_range(1, gWorld->bound_z),
                        };
                        game_add_powerup(loc_powerup[0], loc_powerup[1], loc_powerup[2],
                                         GAME_TYPE_COLLECT, 0);
                        
                        game_add_turret(loc_powerup[0], loc_powerup[1], loc_powerup[2]);
                    }
                     */
                }
                
                // mark: -- spawn collect points randomly
                if(obj_id_base != WORLD_ELEM_ID_INVALID)
                {
                    gameStateSinglePlayer.base_spawn_collect_m += rand_in_range(-65535*0.8, 65535) / 65535;
                    
                    if(gameStateSinglePlayer.base_spawn_collect_m >= 1.0)
                    {
                        gameStateSinglePlayer.base_spawn_collect_m = 0;
                        
                        // spawn a random collect powerup
                        WorldElemListNode* pElemNodeSpawn = world_elem_list_find(obj_id_base, &gWorld->elements_list);
                        
                        if(pElemNodeSpawn)
                        {
                            float v[3];
                            float vel = rand_in_range(gWorld->bound_radius / 100 * 20, gWorld->bound_radius / 2);
                            
                            int collect_elem_new_id =
                                game_add_powerup(pElemNodeSpawn->elem->physics.ptr->x,
                                                 pElemNodeSpawn->elem->physics.ptr->y,
                                                 pElemNodeSpawn->elem->physics.ptr->z,
                                                 GAME_SUBTYPE_COLLECT, 0);
                            
                            world_get_last_object()->collision_start_time = time_ms + 1000;
                            random_heading_vector(v);
                            v[0] *= vel; v[1] *= vel; v[2] *= vel;
                            update_object_velocity(world_get_last_object()->elem_id, v[0], v[1], v[2], 0);
                            
                            console_write(game_log_messages[GAME_LOG_NEWDATA]);
                            
                            if(gameStateSinglePlayer.log_event_camwatch[GAME_LOG_NEWDATA] > 0)
                            {
                                gameStateSinglePlayer.log_event_camwatch[GAME_LOG_NEWDATA]--;
                                glFlightCamFix(collect_elem_new_id);
                            }
                        }
                        
                        console_write(game_log_messages[GAME_LOG_SYSTEMINTEGRITY],
                                      gameStateSinglePlayer.collects_found,
                                      gameStateSinglePlayer.collect_system_integrity);
                    }
                    
                    if(gameStateSinglePlayer.collect_system_integrity <= 0)
                    {
                        game_over();
                    }
                }
                
                if(gameStateSinglePlayer.game_type == GAME_TYPE_TURRET)
                {
                    if(collects_found <= 0)
                    {
                        game_over();
                    }
                }
                
                // add a turret which comes online in X seconds
                /*
                if(turrets_found < gameStateSinglePlayer.n_turrets)
                {
                    console_write(game_log_messages[GAME_LOG_NEWTURRET]);
                    
                    game_add_turret(rand_in_range(-gWorld->bound_radius, gWorld->bound_radius),
                                    rand_in_range(0, gWorld->bound_radius),
                                    rand_in_range(-gWorld->bound_radius, gWorld->bound_radius));
                    
                    world_get_last_object()->stuff.u.enemy.time_last_run = time_ms + 2000;
                }
                */
            }
            
            if(invuln_count > 0)
            {
                invuln_count--;
            }
            else
            {
                collision_actions_set_player_vuln();
            }
            
            if(!gameSettingsSimpleControls)
            {                
                if(gameStateSinglePlayer.boost_charge < 1.0)
                {
                    if(!gameInterfaceControls.fireRectBoost.visible)
                    {
                        gameStateSinglePlayer.boost_charge += game_boost_recharge_rate;
                    }
                }
                else
                {
                    gameStateSinglePlayer.boost_charge = 0;
                    gameInterfaceSetInterfaceState(INTERFACE_STATE_BOOST_AVAIL);
                }
            }
            
            gameStateSinglePlayer.enemy_intelligence += gameStateSinglePlayer.rate_enemy_skill_increase;
            gameStateSinglePlayer.max_enemies += gameStateSinglePlayer.rate_enemy_count_increase;
            
            gameStateSinglePlayer.stats.score += gameStateSinglePlayer.rate_point_increase;
            
            caps_found_last = caps_found;
            gameStateSinglePlayer.caps_found = caps_found;
            
            gameShip_getZVector(v);
            
            // set missle-target
            game_target_missle_id = game_findMissleTarget(p, v,
                                                          target_types,
                                                          r, my_ship_id);
            
            // recharged ammo
            if(game_ammo_bullets < game_ammo_bullets_max) game_ammo_bullets += game_ammo_bullets_recharge;
            if(game_ammo_missles < game_ammo_missles_max) game_ammo_missles += game_ammo_missle_recharge;
            
            score_total();
        }
        
        if(gameStateSinglePlayer.stats.score - score_last_checked > gameStateSinglePlayer.score_pop_threshold)
        {
            if(gameStateSinglePlayer.stats.score > score_last_checked) score_pop();
            score_last_checked = gameStateSinglePlayer.stats.score;
        }
        
        // play sounds based on log events
        char* soundName = NULL;
        if(strstr(consoleMessage, game_log_messages[GAME_LOG_FILESAVED])) soundName = "victory";
        else if(strstr(consoleMessage, game_log_messages[GAME_LOG_FILELOST])) soundName = "dropoff";
        else if(strstr(consoleMessage, game_log_messages[GAME_LOG_WARN_CAPTURE])) soundName = "warning";
        else if(strstr(consoleMessage, game_log_messages[GAME_LOG_KILLED_ENEMY1])) soundName = "none";
        else if(strstr(consoleMessage, game_log_messages[GAME_LOG_KILLED_ENEMY2])) soundName = "none";
        else if(strstr(consoleMessage, game_log_messages[GAME_LOG_KILLED_TURRET])) soundName = "none";
        if(soundName && logSoundTimeLast != console_write_time)
        {
            gameAudioPlaySoundAtLocation(soundName, my_ship_x, my_ship_y, my_ship_z);
            logSoundTimeLast = console_write_time;
        }
        
        // update game status string
        if(game_status_string)
        {
            if(gameStateSinglePlayer.game_type == GAME_TYPE_COLLECT)
            {
                int collect_system_integrity = gameStateSinglePlayer.collect_system_integrity;
                sprintf(game_status_string, "^2^2^d^1^1 %00d%%",
                        collect_system_integrity);
            }
        }
    }
    
    static game_timeval_t game_target_objective_update_last = 0;
    
    if(time_ms - game_target_objective_update_last >= (1000/GAME_FRAME_RATE))
    {
        game_target_objective_update_last = time_ms;
        
        int* arrow_objects[] = {
            &game_target_objective_id,
            //&game_target_enemy_near
        };
        
        for(int a = 0; a < sizeof(arrow_objects)/sizeof(int*); a++)
        {
            WorldElemListNode* pNodeObjective = world_elem_list_find(*arrow_objects[a], &gWorld->elements_moving);
            if(pNodeObjective)
            {
                float arrowD = 2.0;
                WorldElem* elemArrow = NULL;
                
                float h[3] = {
                    pNodeObjective->elem->physics.ptr->x - my_ship_x,
                    pNodeObjective->elem->physics.ptr->y - my_ship_y,
                    pNodeObjective->elem->physics.ptr->z - my_ship_z
                };
                float d = sqrt(h[0]*h[0] + h[1]*h[1] + h[2]*h[2]);
                float eu[3];
                
                eulerHeading1(h, eu);
                
                world_replace_add_object(&gameStateSinglePlayer.elem_id_arrow3[a],
                                         MODEL_BULLET,
                                         my_ship_x + (h[0]/d)*arrowD,
                                         my_ship_y + (h[1]/d)*arrowD,
                                         my_ship_z + (h[2]/d)*arrowD,
                                         eu[0], eu[1], eu[2],
                                         1,
                                         a == 0? TEXTURE_ID_ARROW_OBJ: TEXTURE_ID_ARROW_ENEMY,
                                         &elemArrow);
                
                if(elemArrow)
                {
                    elemArrow->object_type = OBJ_DISPLAYONLY;
                    elemArrow->destructible = 0;
                }
            }
            else
            {
                *arrow_objects[a] = WORLD_ELEM_ID_INVALID;
            }
        }
    }
    
    // ball handling
    while(gameStateSinglePlayer.game_type == GAME_TYPE_LOBBALL)
    {
        static float ballXLast, ballYLast;
        static int touchUnmappedLast = 0;
        
        float m = 10;
        float cvz[3];
        float cvx[3];
        float cvy[3];
        
        gameCamera_getZVector(cvz);
        gameCamera_getXVector(cvx);
        gameCamera_getYVector(cvy);
        
        float pos[3] = {
            gameCamera_getX() + cvz[0] * m,
            gameCamera_getY() + cvz[1] * m,
            gameCamera_getZ() + cvz[2] * m
        };
        
        pMyShipNode = world_elem_list_find(my_ship_id, &gWorld->elements_moving);
        if(!pMyShipNode) break;
        
        // allow moving along x/z plane only
        update_object_velocity(pMyShipNode->elem->elem_id, 0, -pMyShipNode->elem->physics.ptr->vy, 0, 1);
        
        pElemBallHeld = world_find_elem_with_attrs(&gWorld->elements_moving, OBJ_TOUCHCONTROLBALL,
                                                   pMyShipNode->elem->stuff.affiliation);
        
        if(gameInterfaceControls.touchUnmapped && !touchUnmappedLast) // touch started
        {
            // add ball
            float ball_scale = 2.0;
            
            world_add_object(MODEL_SPHERE, pos[0], pos[1], pos[2], 0, 0, 0, ball_scale, TEXTURE_ID_BALL);
            if((pElemBallHeld = world_get_last_object()))
            {
                update_object_velocity(pElemBallHeld->elem_id, 0, 0, 0, 0);
                
                pElemBallHeld->object_type = OBJ_TOUCHCONTROLBALL;
                pElemBallHeld->physics.ptr->gravity = 0;
                pElemBallHeld->stuff.affiliation = pMyShipNode->elem->stuff.affiliation;
            }
            
            ballXLast = gameInterfaceControls.touchUnmappedX;
            ballYLast = gameInterfaceControls.touchUnmappedY;
        }
        else if(touchUnmappedLast && !gameInterfaceControls.touchUnmapped) //touch ended
        {
            if(pElemBallHeld)
            {
                // indicate "released" ball if gravity set
                pElemBallHeld->physics.ptr->gravity = 1;
                pElemBallHeld->bounding_remain = 1;
                world_object_set_lifetime(pElemBallHeld->elem_id, GAME_BULLET_LIFETIME);
            }
        }
        else if(touchUnmappedLast && gameInterfaceControls.touchUnmapped)
        {
            if(pElemBallHeld && pMyShipNode)
            {
                // update velocity
                float touchX = (gameInterfaceControls.touchUnmappedX - ballXLast) * 1.0;
                float touchY = -(gameInterfaceControls.touchUnmappedY - ballYLast) * 1.0;
                
                pElemBallHeld->physics.ptr->friction = 0;
                pElemBallHeld->renderInfo.priority = 1;
                
                for (int i = 0; i < 3; i++) cvz[i] *= 0.8;
                for (int i = 0; i < 3; i++) cvy[i] *= 1.8;
                
                update_object_velocity(pElemBallHeld->elem_id, pMyShipNode->elem->physics.ptr->vx,
                                       pMyShipNode->elem->physics.ptr->vy, pMyShipNode->elem->physics.ptr->vz, 0);
                update_object_velocity(pElemBallHeld->elem_id, cvz[0] * touchX, cvz[1] * touchX, cvz[2] * touchX, 1);
                update_object_velocity(pElemBallHeld->elem_id, cvx[0] * touchY, cvx[1] * touchY, cvx[2] * touchY, 1);
                update_object_velocity(pElemBallHeld->elem_id, cvy[0] * touchY, cvy[1] * touchY, cvy[2] * touchY, 1);
            }
        }
        
        touchUnmappedLast = gameInterfaceControls.touchUnmapped;
        break;
    }
    
    if(gameInterfaceControls.fire.touch_began && time_ms - firedLast > 200)
    {
        firedLast = time_ms;
        fireBullet(ACTION_FIRE_BULLET);
        
        console_write(game_log_messages[GAME_LOG_SHOTFIRED]);
    }
    
    if(gameInterfaceControls.fireRectMissle.touch_began && time_ms - firedLastMissle > 1000)
    {
        firedLastMissle = time_ms;
        fireBullet(ACTION_FIRE_MISSLE);
    }
    
    if(pMyShipNode->elem->stuff.towed_elem_id >= 0)
    {
        gameInterfaceSetInterfaceState(INTERFACE_STATE_TOWING);
        
        if(gameInterfaceControls.fireRectMisc.touch_began)
        {
            gameInterfaceControls.fireRectMisc.touch_began = 0;
            pMyShipNode->elem->stuff.towed_elem_id = WORLD_ELEM_ID_INVALID;
            gameInterfaceSetInterfaceState(INTERFACE_STATE_NONE);
        }
    }
    else
    {
        gameInterfaceSetInterfaceState(INTERFACE_STATE_TOWING_NONE);
    }
    
    if(fireAction >= ACTION_FIRST && fireAction < ACTION_LAST)
    {
        int obj_id;
        int newBlock[3];
        
        block_scale = 1;
        
        ship_z_vec[0] = ship_z_vec[1] = ship_z_vec[2] = 0;
        
        newBlock[0] = round((my_ship_x + ship_z_vec[0]*2)/block_scale) * block_scale;
        newBlock[1] = round((my_ship_y + ship_z_vec[1]*2)/block_scale) * block_scale;
        newBlock[2] = round((my_ship_z + ship_z_vec[2]*2)/block_scale) * block_scale;
        
        if(actions_enabled[fireAction])
        {
            
            switch(fireAction)
            {
                case ACTION_DROP_BLOCK:
                    camera_locked_frames = 120;
                    
                    obj_id =
                    world_add_object(MODEL_CUBE2, newBlock[0], newBlock[1], newBlock[2],
                                     0, 0, 0, block_scale, texture_id_block);
                    
                    world_get_last_object()->object_type = OBJ_BLOCK;
                    break;
                    
                case ACTION_PLACE_TURRET:
                    if(pMyShipNode->elem->stuff.flags.mask &= STUFF_FLAGS_TURRET)
                    {
                        pMyShipNode->elem->stuff.flags.mask ^= STUFF_FLAGS_TURRET;
                        /*
                        obj_id =
                        world_add_object(MODEL_TURRET, newBlock[0], newBlock[1], newBlock[2],
                                         0, 0, 0, 1, texture_id_playership);

                        world_get_last_object()->object_type = OBJ_TURRET;
                        world_get_last_object()->stuff.affiliation = pMyShipNode->elem->stuff.affiliation;
                        update_object_velocity(obj_id, 0, 0, 0, 0);
                         */
                        
                        game_add_turret(newBlock[0], newBlock[1], newBlock[2]);
                        world_get_last_object()->stuff.affiliation = pMyShipNode->elem->stuff.affiliation;
                    }
                    break;
                    
                case ACTION_FIRE_MISSLE:
                    fireBullet(ACTION_FIRE_MISSLE);
                    break;
                    
                case ACTION_SHOOT_BLOCK:
                    fireBullet(ACTION_SHOOT_BLOCK);
                    break;
                    
                case ACTION_REPLACE_OBJECT:
                    fireBullet(ACTION_REPLACE_OBJECT);
                    break;
                    
                case ACTION_SCALE_OBJECT:
                    fireBullet(ACTION_SCALE_OBJECT);
                    break;
                    
                case ACTION_BLOCK_TEXTURE:
                    fireBullet(ACTION_BLOCK_TEXTURE);
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    /*
    static game_timeval_t time_missle_last = 0;
    if(time_ms - time_missle_last > 10000)
    {
        time_missle_last = time_ms;
        console_write("Incoming missle!");
        
        game_add_missle(rand_in_range(1, gWorld->bound_x),
                        rand_in_range(1, gWorld->bound_y),
                        rand_in_range(1, gWorld->bound_z),
                        my_ship_id);
    }
     */
    
    // poop cubes
    static game_timeval_t time_cube_poop_last = 0;
    if(time_ms - time_cube_poop_last > pooped_cube_interval_ms)
    {
        time_cube_poop_last = time_ms;
        firePoopedCube(pMyShipNode->elem);
    }
    
    if(game_reset_needed)
    {
        game_reset_needed = 0;
        game_reset();
    }
}

game_timeval_t
game_time_remaining()
{
    game_timeval_t r = (gameStateSinglePlayer.end_time - time_ms) / 1000;
    if (r < 0) r = 0;
    return r;
}

int
game_time_elapsed()
{
    return gameStateSinglePlayer.time_elapsed / 1000;
}

int
game_add_bullet(float pos[3], float euler[3], float vel, float leadV, int bulletAction, int missle_target, int affiliation)
{
    float bvec[3][3];
    
    get_body_vectors_for_euler2(euler, bvec[0], bvec[1], bvec[2]);
    
    int obj = world_add_object(missle_target >= 0? MODEL_MISSLE: MODEL_BULLET,
                               pos[0] - bvec[2][0]*leadV,
                               pos[1] - bvec[2][1]*leadV,
                               pos[2] - bvec[2][2]*leadV,
                               euler[0], euler[1], euler[2], 1, TEXTURE_ID_BULLET);

    if(obj >= 0)
    {
        world_get_last_object()->object_type = missle_target >= 0? OBJ_MISSLE: OBJ_BULLET;
        update_object_velocity(obj, -bvec[2][0]*vel, -bvec[2][1]*vel, -bvec[2][2]*vel, 0);
        
        world_get_last_object()->stuff.affiliation = affiliation;
        world_get_last_object()->stuff.bullet.action = bulletAction;
        world_get_last_object()->durability = DURABILITY_BULLET;
        world_object_set_lifetime(obj, GAME_BULLET_LIFETIME);
        
        world_get_last_object()->physics.ptr->gravity = gameStateSinglePlayer.game_type == GAME_TYPE_TURRET;
        
        if(missle_target >= 0)
        {
            game_elem_setup_missle(world_get_last_object());
            
            // TODO: improve find_target to take obj_type to target
            // and choose target by vector dot product OR proximity
            
            world_get_last_object()->stuff.u.enemy.target_id = missle_target;
            world_get_last_object()->texture_id = TEXTURE_ID_MISSLE;
            world_object_set_lifetime(obj, GAME_MISSLE_LIFETIME);
            
            gameAudioPlaySoundAtLocation("missle", pos[0], pos[1], pos[2]);
            
            if(affiliation != gameNetworkState.my_player_id)
            {
                world_get_last_object()->stuff.sound.emit_sound_id = GAME_SOUND_ID_MISSLE_FLYBY;
                world_get_last_object()->stuff.sound.emit_sound_duration = GAME_SOUND_DURATION_MISSLE_FLYBY;
            }
        }
        else
        {
            gameAudioPlaySoundAtLocation("shoot", pos[0], pos[1], pos[2]);
            
            if(affiliation != gameNetworkState.my_player_id)
            {
                world_get_last_object()->stuff.sound.emit_sound_id = GAME_SOUND_ID_BULLET_FLYBY;
                world_get_last_object()->stuff.sound.emit_sound_duration = GAME_SOUND_DURATION_BULLET_FLYBY;
            }
        }
    }
    return obj;
}

void
fireBullet(int bulletAction)
{
    WorldElemListNode* listNodeShip =  world_elem_list_find(my_ship_id, &gWorld->elements_list);
    float bullet_z_vec[3], bullet_x_vec[3];
    float bv, blr;
    
    int missle = (bulletAction == ACTION_FIRE_MISSLE);
    
    if(missle)
    {
        if(floor(game_ammo_missles) <= 0) return;
        game_ammo_missles--;
    }
    else
    {
        if(floor(game_ammo_bullets) <= 0) return;
        game_ammo_bullets--;
    }
    
    if(!listNodeShip) return;
    
    gameShip_getZVector(bullet_z_vec);
    gameShip_getXVector(bullet_x_vec);
    
    bullet_z_vec[0] = -bullet_z_vec[0];
    bullet_z_vec[1] = -bullet_z_vec[1];
    bullet_z_vec[2] = -bullet_z_vec[2];
    
    bv = 2 * listNodeShip->elem->scale;
    blr = missle? 0: 0.3 * listNodeShip->elem->scale;
    
    int shots = listNodeShip->elem->type == MODEL_SHIP3 && !missle? 2: 1;
    
    while(shots > 0)
    {
        shots--;
        
        float bullet_x = listNodeShip->elem->physics.ptr->x + bullet_z_vec[0]*bv + bullet_x_vec[0]*blr*blr_last;
        float bullet_y = listNodeShip->elem->physics.ptr->y + bullet_z_vec[1]*bv + bullet_x_vec[1]*blr*blr_last;
        float bullet_z = listNodeShip->elem->physics.ptr->z + bullet_z_vec[2]*bv + bullet_x_vec[2]*blr*blr_last;
        float bullet_euler[3];
        float bullet_pos[] = {bullet_x, bullet_y, bullet_z};
        
        blr_last = -blr_last;
        
        gameShip_getEuler(&bullet_euler[0], &bullet_euler[1], &bullet_euler[2]);
        
        /*
        int obj = world_add_object(missle? MODEL_MISSLE: MODEL_BULLET, bullet_x, bullet_y, bullet_z,
                                   bullet_alpha, bullet_beta, bullet_gamma, 1, 2);
        if(obj >= 0)
        {
            world_get_last_object()->object_type = missle? OBJ_MISSLE: OBJ_BULLET;
            update_object_velocity(obj,
                                   bullet_z_vec[0]*bulletVel,
                                   bullet_z_vec[1]*bulletVel,
                                   bullet_z_vec[2]*bulletVel,
                                   0);
            
            world_get_last_object()->stuff.affiliation = listNodeShip->elem->stuff.affiliation;
            world_get_last_object()->stuff.bullet.action = bulletAction;
            world_get_last_object()->durability = DURABILITY_BULLET;
            
            if(missle)
            {
                game_elem_setup_missle(world_get_last_object());
                // TODO: improve find_target to take obj_type to target
                // and choose target by vector dot product OR proximity
            
                world_get_last_object()->stuff.u.enemy.target_id = game_target_missle_id;
                world_get_last_object()->texture_id = TEXTURE_ID_MISSLE;
                world_object_set_lifetime(obj, GAME_MISSLE_LIFETIME);
                
                if(!sound_played)
                {
                    gameAudioPlaySoundAtLocation("missle", gameCamera_getX(), gameCamera_getY(), gameCamera_getZ());
                    sound_played = 1;
                }
            }
            else
            {
                if(!sound_played)
                {
                    gameAudioPlaySoundAtLocation("shoot", gameCamera_getX(), gameCamera_getY(), gameCamera_getZ());
                    sound_played = 1;
                }
            }
        }
         */
        
        game_add_bullet(bullet_pos, bullet_euler, missle? MAX_SPEED_MISSLE: bulletVel, bv, bulletAction,
                        missle? game_target_missle_id: -1, listNodeShip->elem->stuff.affiliation);
    }
}

int
firePoopedCube(WorldElem *elem)
{
    int use_drawline = 1;
    quaternion_t qx, qy, qz;
    
    if(elem->physics.ptr->velocity <= 1) return WORLD_ELEM_ID_INVALID;
    
    get_body_vectors_for_euler(elem->physics.ptr->alpha, elem->physics.ptr->beta, elem->physics.ptr->gamma,
                               &qx, &qy, &qz);
    
    int texture_id = /*elem->texture_id*/ TEXTURE_ID_POOPED_CUBE_ENEMY;
    int model = MODEL_CUBE;
    
    if(elem->elem_id == my_ship_id)
    {
        texture_id = TEXTURE_ID_POOPED_CUBE;
        
        if(speedBoost > 0)
        {
            texture_id = TEXTURE_ID_POOPED_CUBE_BOOST;
        }
        else
        {
            if(elem->durability <= DURABILITY_LOW) texture_id = TEXTURE_ID_POOPED_CUBE_SHIELDLOW;
        }
    }
    else
    {
        texture_id = TEXTURE_ID_POOPED_CUBE_ENEMY;
        if(elem->durability <= DURABILITY_LOW) texture_id = TEXTURE_ID_POOPED_CUBE_SHIELDLOW;
    }
    
    if(!use_drawline)
    {
        int obj =
            world_add_object(model,
                             elem->physics.ptr->x + qz.x*1,
                             elem->physics.ptr->y + qz.y*1,
                             elem->physics.ptr->z + qz.z*1,
                             elem->physics.ptr->alpha, elem->physics.ptr->beta, elem->physics.ptr->gamma,
                             0.5,
                             texture_id);
        world_get_last_object()->object_type = OBJ_POOPEDCUBE;
        world_get_last_object()->renderInfo.priority = 1;
        world_get_last_object()->destructible = 0;
        world_get_last_object()->physics.ptr->friction = 1;
        world_get_last_object()->is_line = 1;
        world_get_last_object()->elem_id_line_next = elem->stuff.line_id_last;
        elem->stuff.line_id_last = obj;
        world_object_set_lifetime(obj, pooped_cube_lifetime);
        
        /*
         update_object_velocity(obj, elem->physics.ptr->vx, elem->physics.ptr->vy, elem->physics.ptr->vz, 0);
         */
        
        return obj;
    }
    else
    {
        float m = 0.1;
        float n = -1.0;
        float lineA[] = {
            elem->physics.ptr->x - qz.x*n,
            elem->physics.ptr->y - qz.y*n,
            elem->physics.ptr->z - qz.z*n
        };
        float lineB[] = {
            lineA[0]-elem->physics.ptr->vx*m,
            lineA[1]-elem->physics.ptr->vy*m,
            lineA[2]-elem->physics.ptr->vz*m
        };

        static float lineColorMine[3] = {0, 1, 0};
        static float lineColorEnemy[3] = {1, 0, 0};
        static float lineColorEnemyAce[3] = {0xde, 0, 0xff};
        static float lineColorLowShield[3] = {1, 1, 0};
        float *lineColor = lineColorEnemy;

        if(elem->elem_id == my_ship_id) lineColor = lineColorMine;
        if(elem->texture_id == TEXTURE_ID_ENEMYSHIP_ACE) lineColor = lineColorEnemyAce;
        //if(elem->durability <= DURABILITY_LOW) lineColor = lineColorLowShield;
        
        world_add_drawline(lineA, lineB, lineColor, 240);
        
        return WORLD_ELEM_ID_INVALID;
    }
}

int
game_ai_target_priority(WorldElem* pSearchElem, WorldElem* pTargetElem, float* dist_ignore)
{
    const static float velocity_pickup = 3.0;
    
    if(pSearchElem == pTargetElem) return 0;
    
    if(pSearchElem->stuff.towed_elem_id != WORLD_ELEM_ID_INVALID)
    {
        *dist_ignore = INT_MAX;
     
        if((pTargetElem->object_type == OBJ_SPAWNPOINT_ENEMY || pTargetElem->object_type == OBJ_SPAWNPOINT) &&
           pTargetElem->stuff.affiliation == pSearchElem->stuff.affiliation) return 1;
    }
    else
    {
        *dist_ignore = pSearchElem->stuff.u.enemy.scan_distance;
     
        if(pTargetElem->object_type == OBJ_PLAYER && !pSearchElem->stuff.u.enemy.ignore_player) return 2;
        else if(pTargetElem->object_type == OBJ_SHIP) return 2;
        else if(pTargetElem->object_type == OBJ_TURRET) return 1;
        else if(pTargetElem->object_type == OBJ_POWERUP_GENERIC &&
                !pSearchElem->stuff.u.enemy.ignore_collect &&
                pTargetElem->stuff.subtype == GAME_SUBTYPE_COLLECT &&
                pTargetElem->physics.ptr->velocity <= velocity_pickup)
        {
            return 3;
        }
    }
    return 0;
}
