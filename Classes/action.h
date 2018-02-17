//
//  action.h
//  gl_flight
//
//  Created by jbrady on 2/19/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef gl_flight_action_h
#define gl_flight_action_h

#include <string.h>
#include "gameGlobals.h"
#include "action.h"

#define ACTIONS_CURSOR_STR "->"
typedef enum
{
    ACTION_INVALID = -1,
    ACTION_FIRST = 0,
    ACTION_START_GAME = 0,
    ACTION_RESUME_GAME,
    ACTION_START_SURVIVAL_GAME,
    ACTION_START_SPEEDRUN_GAME,
    ACTION_START_DEFEND_GAME,
    ACTION_START_TURRET_GAME,
    ACTION_NETWORK_MULTIPLAYER_MENU,
    ACTION_START_LOBBALL_GAME,
    ACTION_DISPLAY_SCORES,
    ACTION_HELP, /**/
    
    ACTION_HOST_GAME,
    ACTION_HOST_GAME_LAN,
    ACTION_CONNECT_TO_GAME,
    ACTION_CONNECT_TO_GAME_LAN,
    ACTION_START_DEATHMATCH_GAME,  /**/
    ACTION_NETWORK_GAME_SCORE,  /**/
    
    ACTION_FIRE_BULLET,
    ACTION_FIRE_MISSLE,
    ACTION_GAME_PLACEHOLDER,
    ACTION_DROP_TOW,  /**/
    ACTION_PLACE_SHIP,
    ACTION_PLACE_TURRET,  /**/
    ACTION_MAP_EDIT,
    ACTION_MAP_SAVE,
    ACTION_SHOOT_BLOCK,
    ACTION_DROP_BLOCK,
    ACTION_BLOCK_TEXTURE,
    ACTION_SCALE_OBJECT,
    ACTION_REPLACE_OBJECT,
    ACTION_CLEAR_MAP,  /**/
    
    ACTION_SETTING_PLAYER_NAME,
    ACTION_SETTING_GAME_NAME,
    ACTION_SETTING_SHIP_MODEL,
    ACTION_SETTING_LOCK_CAMERA,
    ACTION_SETTING_INPUT_SENSITIVITY,
    ACTION_SETTING_CONTROL_MODE,
    ACTION_SETTING_BOTINTELLIGENCE,
    ACTION_SETTING_AUDIO,
    ACTION_SETTING_SHIP_TEXTURE,
    ACTION_SETTING_UPDATE_FREQUENCY,
    ACTION_SETTING_SHOW_DEBUG,
    ACTION_SETTING_TWEAK_PHYSICS,
    ACTION_SETTING_GYRO_FEEDBACK,
    ACTION_SETTING_PORT_NUMBER,
    ACTION_SETTING_LOCAL_IP_OVERRIDE,
    ACTION_SETTING_RESET_SCORE,
    ACTION_SETTING_RESET_DEFAULT,
    ACTION_LAST /* 46 */
} action_t;

static int actions_sub[][ACTION_LAST] =
{
    // main
    {1,0,1,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    // net
    {0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    // game
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    // build
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    // settings
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

extern int *actions_enabled;

static const int actions_sub_last = 5;
extern int actions_sub_cur;

static const char* action_strings[ACTION_LAST] =
{
    "game: firewall",
    "game: resume",
    "game: survival",
    "game: speedrun",
    "game: defend",
    "game: turret",
    "game: multiplayer",
    "game: lob",
    "display scores",
    "help/about",
    "host internet game",
    "host local game",
    "join game (internet)",
    "join game (local WIFI)",
    "start game",
    "scores",
    "shoot",
    "shoot missle",
    "<empty>",
    "drop tow",
    "deploy ally",
    "deploy turret",
    "map: load custom",
    "map: save custom",
    "edit: clone block",
    "edit: drop block",
    "edit: block texture >>",
    "edit: scale object",
    "edit: change object",
    "map: regenerate",
    "player name",
    "game server name",
    "ship type",
    "camera mode",
    "input sensitivity",
    "control mode",
    "deathmatch bot skill",
    "mute audio",
    "ship appearance(texture)",
    "net update interval(ms)",
    "show debug",
    "friction level",
    "gyroscope drift correct",
    "port number",
    "Directory IP addr override",
    "reset scores",
    "restore default"
};

static void action_next()
{
    do {
        fireAction++;
        if(fireAction >= ACTION_LAST) fireAction = 0;
    } while(!actions_enabled[fireAction]);
    
    while(!actions_enabled[fireAction]) {
        fireAction++;
        if(fireAction >= ACTION_LAST) fireAction = 0;
    };
}

static void action_prev()
{
    do {
        fireAction--;
        if(fireAction < ACTION_FIRST) fireAction = ACTION_LAST-1;
    } while (!actions_enabled[fireAction]);
    
    while(!actions_enabled[fireAction]) {
        fireAction--;
        if(fireAction < ACTION_FIRST) fireAction = ACTION_LAST-1;
    };
}

static void action_sub_next()
{
    actions_sub_cur++;
    if(actions_sub_cur >= actions_sub_last) actions_sub_cur = 0;
    actions_enabled = actions_sub[actions_sub_cur];
    
    fireAction = 0;
    while(!actions_enabled[fireAction]) {
        fireAction++;
        if(fireAction >= ACTION_LAST) fireAction = 0;
    };
}

static void action_sub_prev()
{
    actions_sub_cur--;
    if(actions_sub_cur < ACTION_FIRST) actions_sub_cur = actions_sub_last-1;
    actions_enabled = actions_sub[actions_sub_cur];
    
    fireAction = ACTION_FIRST;
    while(!actions_enabled[fireAction]) {
        fireAction++;
        if(fireAction >= ACTION_LAST) fireAction = 0;
    };
}

static void actions_menu_set(int action)
{
    fireAction = ACTION_FIRST;
    actions_sub_cur = 0;
    
    fireAction = action;
    while(!actions_enabled[action]) action_sub_next();
}

static void actions_menu_reset()
{
    actions_menu_set(ACTION_FIRST);
}

static void actions_display_menu(char *dest)
{
    dest[0] = '\0';
    int lines = 0;
    int maxlines = 10;
    
    sprintf(dest,
            "%s %s %s %s %s\n",
            (actions_sub_cur == 0? "*MAIN*": "MAIN"),
            (actions_sub_cur == 1? "*NET*": "NET"),
            (actions_sub_cur == 2? "*GAME*": "GAME"),
            (actions_sub_cur == 3? "*BUILD*": "BUILD"),
            (actions_sub_cur == 4? "*SETTINGS*": "SETTINGS"));
    int dlen = strlen(dest);
    while(dlen > 0 ) { strcat(dest, "-"); dlen--; }
    strcat(dest, "\n");
    
    for(int i = ACTION_FIRST; i < ACTION_LAST; i++)
    {
        if(actions_enabled[i]) lines++;
    }
    
    for(int i = (lines <= maxlines? ACTION_FIRST: fireAction); i < ACTION_LAST && maxlines > 0; i++)
    {    
        if(actions_enabled[i])
        {
            sprintf(dest+strlen(dest), "%s%s\n", (fireAction == i? ACTIONS_CURSOR_STR: ""), action_strings[i]);
            maxlines--;
        }
    }
    
    if(maxlines == 0) sprintf(dest+strlen(dest), "...");
}

#endif
