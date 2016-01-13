//
//  gameDialogs.h
//  gl_flight
//
//  Created by Justin Brady on 4/2/14.
//
//

#ifndef gl_flight_gameDialogs_h
#define gl_flight_gameDialogs_h

#include "gameGlobals.h"
#include "gameInterface.h"
#include "gameSettings.h"
#include "gamePlay.h"
#include "maps.h"
#include "world_file.h"

struct gameDialogStateStruct
{
    int ratingDesired;
    int controlsResuming;
};

struct gameDialogStateStruct gameDialogState;

static void
gameDialogCancel(void)
{
}

static void
gameDialogResetScoresYes()
{
    gameSettingsStatsReset();
}

static void
gameDialogResetScores()
{
    gameInterfaceModalDialog("Reset scores?\nYou sure?", "Yes", "No", gameDialogResetScoresYes, gameDialogCancel);
}

static void
gameDialogWelcomeSingleNew()
{
    gameInterfaceControls.menuAction = ACTION_START_GAME;
}

static void
gameDialogWelcomeSingleResume()
{
    gameInterfaceControls.menuAction = ACTION_RESUME_GAME;
}

static void
gameDialogWelcomeSingle()
{
    if(/*gameStateSinglePlayer.last_game.difficulty > 1*/ 0)
    {
        gameInterfaceModalDialog("Resume last game?", "Resume", "New",
                                 gameDialogWelcomeSingleResume, gameDialogWelcomeSingleNew);
    }
    else
    {
        gameInterfaceControls.menuAction = ACTION_START_GAME;
    }
}

static void
gameDialogWelcomeMultiDirectory()
{
    actions_menu_set(ACTION_BROWSE_GAMES);
    gameInterfaceControls.menuAction = ACTION_BROWSE_GAMES;
}

static void
gameDialogWelcomeMultiHost()
{
    actions_menu_set(ACTION_HOST_GAME);
    gameInterfaceControls.menuAction = ACTION_HOST_GAME;
}

static void
gameDialogWelcomeMultiHostLAN()
{
    actions_menu_set(ACTION_HOST_LAN_GAME);
    gameInterfaceControls.menuAction = ACTION_HOST_LAN_GAME;
}

static void
gameDialogWelcomeMultiJoinLAN()
{
    actions_menu_set(ACTION_CONNECT_TO_LAN_GAME);
    gameInterfaceControls.menuAction = ACTION_CONNECT_TO_LAN_GAME;
}

static void
gameDialogWelcomeMultiInternet()
{
    gameInterfaceModalDialog("Find a game\nor Host\n(see help)", "Find", "Host",
                             gameDialogWelcomeMultiDirectory, gameDialogWelcomeMultiHost);
}

static void
gameDialogWelcomeMultiLan()
{
    gameInterfaceModalDialog("Join or host:", "Join", "Host",
                             gameDialogWelcomeMultiJoinLAN, gameDialogWelcomeMultiHostLAN);
}

static void
gameDialogWelcomeMulti()
{
    /*
    gameInterfaceModalDialog("Local/Internet:", "Local\nWI-FI", "Internet",
                             gameDialogWelcomeMultiLan, gameDialogWelcomeMultiInternet);
     */
    gameDialogWelcomeMultiLan();
}

static void
gameDialogWelcomeQuick()
{
    gameInterfaceModalDialog("Game type:", "Firewall", "Multiplayer",
                             gameDialogWelcomeSingle, gameDialogWelcomeMulti);
}

static void
gameDialogWelcomeMenu()
{
    gameInterfaceControls.mainMenu.visible = 1;
    gameInterfaceControls.textMenuControl.visible = 1;
}

static void gameDialogWelcome();

static void
gameDialogRating()
{
    gameDialogState.ratingDesired = 1;
    gameSettingsRatingGiven = 1;
    gameInterfaceModalDialog("Launching ratings app...",
                             "Done", "OK",
                             gameDialogCancel, gameDialogCancel);
}

static void
gameDialogWelcomeRating()
{
    gameDialogState.ratingDesired = 1;
    gameSettingsRatingGiven = 1;
    gameDialogWelcome();
}

static void
gameDialogWelcomeNoRating()
{
    gameSettingsRatingGiven = 1;
    gameDialogWelcome();
}

#define WELCOMESTR                                  \
"^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D\n"    \
"^DWelcome to d0gf1ght!^D\n"                        \
"^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D\n"

static void
gameDialogWelcome()
{
    gameInterfaceControls.mainMenu.visible = 1;
    
    if(gameSettingsLaunchCount % 2 == 1 && !gameSettingsRatingGiven)
    {
        gameInterfaceModalDialog(WELCOMESTR
                                 "Please rate d0gf1ght!\n(and unlock new ships!)\n"
                                 "It only takes a second!\n",
                                 "Sure", "No way",
                                 gameDialogWelcomeRating, gameDialogWelcomeNoRating);
    }
    else
    {
        gameInterfaceModalDialog(WELCOMESTR, "New Game", "Menu",
                                 /*gameDialogWelcomeQuick*/ gameDialogWelcomeSingle, gameDialogWelcomeMenu);
    }
}

static void
gameDialogCalibrate()
{
    gameInterfaceModalDialog(
                             "^A^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^A\n"
                             "^C Calibrating controls:       ^C\n"
                             "^C Hold device still...        ^C\n"
                             "^C (to re-calibrate later use  ^C\n"
                             "^C  flashing TRIM button)      ^C\n"
                             "^A^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^A",
                             "", "cancel", gameDialogCancel, gameDialogCancel);
}

static void
gameDialogResumePaused()
{
    gameDialogState.controlsResuming = 1;
}

static void
gameDialogResumePausedNo()
{
    gameInterfaceControls.mainMenu.visible = 1;
    controlsCalibrated = 0;
}

static void
gameDialogResume()
{
    gameInterfaceModalDialog("Resume paused game?", "Yes", "No",
                             gameDialogResumePaused, gameDialogResumePausedNo);
}

static void
gameDialogConnectToGameYes()
{
    actions_menu_set(ACTION_CONNECT_TO_GAME);
    gameInterfaceControls.menuAction = ACTION_CONNECT_TO_GAME;
}

static void
gameDialogPortalToGame()
{
    gameInterfaceModalDialog("Connect to this game?", "Yes", "No",
                             gameDialogConnectToGameYes, gameDialogCancel);
}

static void
gameDialogBrowseGames()
{
    gameInterfaceModalDialog("Enter node to connect...", "OK", "", gameDialogCancel, gameDialogCancel);
}

static void
gameDialogStartNetworkGame2()
{
    game_start(1, GAME_TYPE_DEATHMATCH);
    gameNetwork_startGame(300);
    gameInterfaceControls.mainMenu.visible = gameInterfaceControls.textMenuControl.visible = 0;
}

static void gameDialogStartNetworkGame();

static void
gameDialogStartNetworkGameNewMap()
{
    extern int maps_list_idx;
    
    maps_list_idx++;
    if(maps_list[maps_list_idx] == NULL) maps_list_idx = 0;
    
    gameMapSetMap(maps_list[maps_list_idx]);
    
    gameStateSinglePlayer.map_use_current = 1;
    
    console_append("MAP: %s", maps_list_names[maps_list_idx]);

    gameDialogStartNetworkGame();
}

static void
gameDialogStartNetworkGame()
{
    gameStateSinglePlayer.map_use_current = 1;
    gameInterfaceModalDialog("Gathering players...\nWait...\nStart when ready\n", "Start", "Change\nmap", gameDialogStartNetworkGame2, gameDialogStartNetworkGameNewMap);
}

static char gameDialogDisplayStringStr[16][1024];
static int gameDialogDisplayString_n = 0;

static void gameDialogCancelString();

static void
gameDialogDisplayString(char *str)
{
    if(gameDialogDisplayString_n > 15) return;
    
    strcpy(gameDialogDisplayStringStr[gameDialogDisplayString_n], str);
    gameDialogDisplayString_n++;
    
    if(gameDialogDisplayString_n == 1)
    {
        gameInterfaceModalDialog(gameDialogDisplayStringStr[gameDialogDisplayString_n-1], "OK", "",
                                 gameDialogCancelString, gameDialogCancelString);
    }
}

static void
gameDialogCancelString()
{
    gameDialogDisplayString_n--;
    if(gameDialogDisplayString_n >= 1)
    {
        gameInterfaceModalDialog(gameDialogDisplayStringStr[gameDialogDisplayString_n],
                                 "OK", "OK",
                                 gameDialogCancelString, gameDialogCancelString);
    }
}

static char gameDialogScoresString[1024];

static void
gameDialogScores()
{
    sprintf(gameDialogScoresString, "firewall:%d\nSurvival:%d\nDefend:%d\nDeathmatch:%d\n",
            gameStateSinglePlayer.high_score[GAME_TYPE_COLLECT],
            gameStateSinglePlayer.high_score[GAME_TYPE_SURVIVAL],
            gameStateSinglePlayer.high_score[GAME_TYPE_DEFEND],
            gameStateSinglePlayer.high_score[GAME_TYPE_DEATHMATCH]);
    
    gameInterfaceModalDialog(gameDialogScoresString,
                             "OK", "OK",
                             gameDialogCancelString, gameDialogCancelString);
}

static void
gameDialogScorePopup(char *scoreStr)
{
    controlRect r;
    r = gameInterfaceControls.dialogRectDefault;
    
    r.x -= gameInterfaceControls.interfaceWidth / 3;
    
    float scale = 3;
    r.x += r.xw/scale;
    r.y += r.yw/scale;
    r.xw /= scale;
    r.yw /= scale;
    r.xm = -1;
    r.ym = 0;
    r.tex_id = 4;
    
    gameInterfaceModalDialogWithRect(scoreStr, "", "", gameDialogCancelString, gameDialogCancelString, &r, GAME_FRAME_RATE * 2.0);
    gameInterfaceControls.dialogRect.modal = 0;
}

#endif
