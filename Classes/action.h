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

#define ACTIONS_CURSOR_STR "->"
typedef enum
{
    ACTION_INVALID = -1,
    ACTION_FIRST = 0,
    ACTION_START_GAME = 0,
    ACTION_RESUME_GAME,
    ACTION_START_SURVIVAL_GAME,
    ACTION_START_DEFEND_GAME,
    ACTION_NETWORK_MULTIPLAYER_MENU,
    ACTION_DISPLAY_SCORES,
    ACTION_HELP,
    
    ACTION_BROWSE_GAMES,
    ACTION_CONNECT_TO_LAN_GAME,
    ACTION_HOST_LAN_GAME,
    ACTION_CONNECT_TO_GAME,
    ACTION_HOST_GAME,
    ACTION_START_DEATHMATCH_GAME,
    
    ACTION_FIRE_BULLET,
    ACTION_FIRE_MISSLE,
    ACTION_DROP_TOW,
    
    ACTION_PLACE_SHIP,
    ACTION_PLACE_TURRET,
    
    ACTION_MAP_EDIT,
    ACTION_MAP_SAVE,
    ACTION_SHOOT_BLOCK,
    ACTION_DROP_BLOCK,
    ACTION_BLOCK_TEXTURE,
    ACTION_SCALE_OBJECT,
    ACTION_REPLACE_OBJECT,
    ACTION_CLEAR_MAP,
    
    ACTION_SETTING_SHIP_MODEL,
    ACTION_SETTING_INPUT_SENSITIVITY,
    ACTION_SETTING_NUMAIS,
    ACTION_SETTING_BOTINTELLIGENCE,
    ACTION_SETTING_DIFFICULTY,
    ACTION_SETTING_AUDIO,
    ACTION_SETTING_SHIP_TEXTURE,
    ACTION_SETTING_PLAYER_NAME,
    ACTION_SETTING_GAME_NAME,
    ACTION_SETTING_UPDATE_FREQUENCY,
    ACTION_SETTING_DIRECTORY_SERVER,
    ACTION_SETTING_LOCK_CAMERA,
    ACTION_SETTING_SHOW_DEBUG,
    ACTION_SETTING_TWEAK_PHYSICS,
    ACTION_SETTING_GYRO_FEEDBACK,
    ACTION_SETTING_PORT_NUMBER,
    ACTION_SETTING_LOCAL_IP_OVERRIDE,
    ACTION_SETTING_RESET_SCORE,
    ACTION_LAST
} action_t;

static int actions_sub[][ACTION_LAST] =
{
    // main
    {1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // net
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // game
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // build
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // settings
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

extern int *actions_enabled;

static const int actions_sub_last = 5;
extern int actions_sub_cur;

static const char* action_strings[] =
{
    "game: firewall",
    "game: resume",
    "game: survival",
    "game: defend",
    "game: network multiplayer DM",
    "display scores",
    "help/about",
    "game directory",
    "join (LAN/WiFi local)",
    "host (LAN/WiFi local)",
    "join (Internet directory)",
    "host (Internet directory)",
    "start game",
    "shoot",
    "shoot missle",
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
    "ship type",
    "input sensitivity",
    "deathmatch num bots",
    "deathmatch bot skill",
    "difficulty",
    "mute audio",
    "ship appearance",
    "player name",
    "game name",
    "net frequency ms",
    "directory server",
    "camera mode",
    "show debug",
    "friction level",
    "gyroscope drift correct",
    "server port number",
    "local IP detect/override",
    "reset scores",
    "UNKNOWN"
};

extern int fireAction;

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
