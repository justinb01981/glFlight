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
    char gameDialogPortalToGameLast[255];
    int lanGameScanning;
    int networkGameNameEntered;
};

extern struct gameDialogStateStruct gameDialogState;

extern int gameDialogCounter;

static void
gameDialogCancel(void)
{
}

static void
gameDialogClose(void)
{
    gameInterfaceControls.dialogRect.visible = 0;
}

static int
gameDialogVisible()
{
    return gameInterfaceControls.dialogRect.visible;
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
gameDialogWelcomeSingle()
{
    fireAction = ACTION_START_GAME;
    gameInterfaceControls.textMenuControl.visible = 1;
}

static void
gameDialogWelcomeMultiHost()
{
    actions_menu_set(ACTION_HOST_GAME);
    fireAction = ACTION_HOST_GAME;
}

static void
gameDialogWelcomeMultiJoin()
{
    actions_menu_set(ACTION_CONNECT_TO_GAME);
    fireAction = ACTION_CONNECT_TO_GAME;
}

static void
gameDialogWelcomeMulti()
{
    gameInterfaceModalDialog("Join or host:", "Join", "Host",
                             gameDialogWelcomeMultiJoin, gameDialogWelcomeMultiHost);
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
gameDialogMenuCountdown()
{
    static int passes = 60;
    passes--;
    
    if(passes > 0) return;
    
    gameDialogWelcomeMenu();
    glFlightDrawframeHook = NULL;
}

static void
gameDialogWelcome()
{
    gameInterfaceControls.mainMenu.visible = 1;
    
    if(/*gameSettingsLaunchCount % 2 == 1 && !gameSettingsRatingGiven*/ 0)
    {
        gameInterfaceModalDialog(WELCOMESTR
                                 "Please rate d0gf1ght\n"
                                 "to get new jets/levels!\n",
                                 "Sure", "No way",
                                 gameDialogWelcomeRating, gameDialogWelcomeNoRating);
    }
    else
    {
        //gameInterfaceModalDialog(WELCOMESTR, "Quick Game", "Menu",
        //                         /*gameDialogWelcomeQuick*/ gameDialogWelcomeSingle, gameDialogWelcomeMenu);
        
        glFlightDrawframeHook = gameDialogMenuCountdown;
    }
    
    gameDialogClose();
}

static void
gameDialogCancelTrim()
{
    extern void gameInputTrimAbort(void);
    
    gameInputTrimAbort();
    gameDialogCancel();
}

static void
gameDialogLoading()
{
    gameInterfaceModalDialog(
                             "^A^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^A\n"
                             "^C                             ^C\n"
                             "^C                             ^C\n"
                             "^C        Loading....          ^C\n"
                             "^C                             ^C\n"
                             "^C                             ^C\n"
                             "^C                             ^C\n"
                             "^A^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^A",
                             "", "cancel", gameDialogCancelTrim, gameDialogCancelTrim);
}

static void
gameDialogCalibrate()
{
    gameInterfaceModalDialog(
                             "^A^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^A\n"
                             "^C Calibrating controls:       ^C\n"
                             "^C 1. Turn left/right          ^C\n"
                             "^C 2. Center controls          ^C\n"
                             "^C 3. Hold still               ^C\n"
                             "^C Use flashing trim button    ^C\n"
                             "^C to calibrate later          ^C\n"
                             "^A^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^A",
                             "", "cancel", gameDialogCancelTrim, gameDialogCancelTrim);
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
    fireAction = ACTION_CONNECT_TO_GAME;
}

static char*
gameDialogPortalToGameLast()
{
    gameDialogClose();
    return gameDialogState.gameDialogPortalToGameLast;
}

static void
gameDialogPortalToGame(const char* address)
{
    char str[255];
    sprintf(str, "Click to join\n \"%s\"", address);
    gameInterfaceModalDialog(str, "Yes", "No",
                             gameDialogConnectToGameYes, gameDialogCancel);
    strcpy(gameDialogState.gameDialogPortalToGameLast, address);
}

static void
gameDialogError(const char* errorMessage)
{
    char str[255];
    sprintf(str, "Error:\n%s\n", errorMessage);
    gameInterfaceModalDialog(str, "Yes", "No",
                             gameDialogCancel, gameDialogCancel);
}

static void
gameDialogBrowseGames()
{
    gameInterfaceModalDialog("Enter node to connect...", "OK", "", gameDialogCancel, gameDialogCancel);
}

static void gameDialogLanConnect()
{
    gameDialogCancel();
    fireAction = ACTION_CONNECT_TO_GAME_LAN;
    gameInterfaceProcessAction();
}

static int
gameDialogSearchingForGame()
{
    extern int GameNetworkBonjourManagerBrowseBegin();
    
    if(gameDialogState.lanGameScanning)
    {
        gameDialogState.lanGameScanning = 0;
        return 1;
    }
    else
    {
        GameNetworkBonjourManagerBrowseBegin();
        gameInterfaceModalDialog("Searching for game...", "", "",
                                 gameDialogLanConnect, gameDialogCancel);
        gameDialogState.lanGameScanning = 1;
        return 0;
    }
}

static void
gameDialogConnectToGameSuccessful()
{
    gameInterfaceModalDialog("Connecting...\nDownloading...", "Ok", "",
                             gameDialogCancel, gameDialogCancel);
}

static void
gameDialogConnectToGameSuccessful2()
{
    gameDialogClose();
    
    gameInterfaceControls.mainMenu.visible = 0;
    actions_menu_reset();
    gameStateSinglePlayer.started = 0;
    gameInterfaceSetInterfaceState(INTERFACE_STATE_CLOSE_MENU);
}

static void
gameDialogStartNetworkGame2()
{
    gameNetwork_sendPlayersDisconnect();
    
    game_start(1, GAME_TYPE_DEATHMATCH);
    gameNetwork_startGame(300);
    gameStateSinglePlayer.max_enemies = gameDialogCounter;
    
    gameInterfaceControls.mainMenu.visible = gameInterfaceControls.textMenuControl.visible = 0;
    
    gameNetwork_alert("ALERT:game started!");
}

static void
gameDialogStartNetworkGameBotAdded()
{
    gameDialogCounter++;
    if(world_find_elem_with_attrs(&gWorld->elements_list, OBJ_SPAWNPOINT_ENEMY, AFFILIATION_NONE) == NULL)
    {
        game_add_spawnpoint(rand_in_range(1, gWorld->bound_x),
                            rand_in_range(1, gWorld->bound_y),
                            rand_in_range(1, gWorld->bound_z),
                            NULL);
    }

    // TODO: dialog to indicate local address
    gameInterfaceModalDialog("(Added)\nReady to start game?", "Start", "+Bot",
                             gameDialogStartNetworkGame2, gameDialogStartNetworkGameBotAdded);
}

static void
gameDialogStartNetworkGameAddBots()
{
    gameInterfaceModalDialog("Round started!", "Start", "+1",
                             gameDialogStartNetworkGame2, gameDialogStartNetworkGameBotAdded);
}

static void gameDialogStartNetworkGame();

static void
gameDialogStartNetworkGameNewMap()
{
    extern unsigned int maps_list_idx;
    
    maps_list_idx++;
    if(maps_list[maps_list_idx] == NULL) maps_list_idx = 0;
    
    gameDialogStartNetworkGame();
    
    gameMapSetMap(maps_list[maps_list_idx]);
    
    gameStateSinglePlayer.map_use_current = 1;
    
    console_append("MAP: %s\n", maps_list_names[maps_list_idx]);
}

static void
gameDialogStartNetworkGame()
{
    gameDialogCounter = 0;
    gameStateSinglePlayer.map_use_current = 1;
    gameInterfaceModalDialog("Ready to start?\nWait for more?\n", "Start", "Wait", gameDialogStartNetworkGameAddBots, gameDialogCancel);
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
gameDialogScoresDone()
{
    gameStateSinglePlayer.stats.score = 0;
    
    gameDialogWelcome();
}

static void
gameDialogScores()
{
    sprintf(gameDialogScoresString, "firewall:%d\nSurvival:%d\nDefend:%d\nDeathmatch:%d\nTurret:%d\n",
            gameStateSinglePlayer.high_score[GAME_TYPE_COLLECT],
            gameStateSinglePlayer.high_score[GAME_TYPE_SURVIVAL],
            gameStateSinglePlayer.high_score[GAME_TYPE_DEFEND],
            gameStateSinglePlayer.high_score[GAME_TYPE_DEATHMATCH],
            gameStateSinglePlayer.high_score[GAME_TYPE_TURRET]);
    
    gameInterfaceModalDialog(gameDialogScoresString,
                             "OK", "OK",
                             gameDialogScoresDone, gameDialogScoresDone);
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
    r.tex_id = TEXTURE_ID_CONTROLS_POPUPOVER_DIALOG_BOX;
    gameInterfaceControls.dialogRect.modal = 0;
}

static void
gameDialogMessagePopup(char *str)
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
    
    gameInterfaceModalDialogWithRect(str, "", "", gameDialogCancelString, gameDialogCancelString, &r, GAME_FRAME_RATE * 2.0);
    r.tex_id = TEXTURE_ID_CONTROLS_POPUPOVER_DIALOG_BOX;
    gameInterfaceControls.dialogRect.modal = 0;
}

static void
gameDialogGraphicCancel()
{
    gameInterfaceControls.graphicDialog.visible = 0;
}

static void
gameDialogGraphic(int tex_id)
{
    gameInterfaceControls.graphicDialog.visible = 1;
    gameInterfaceControls.graphicDialog.tex_id = tex_id;
}

#endif
