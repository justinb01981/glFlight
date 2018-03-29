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
#include "gameInput.h"

struct gameDialogStateStruct
{
    int ratingDesired;
    char gameDialogPortalToGameLast[255];
    int networkGameNameEntered;
    int hideNetworkStatus;
    
    char displayStringBuf[1024][16];
    int displayStringCount;
};

extern struct gameDialogStateStruct gameDialogState;

extern int gameDialogCounter;

static void
gameDialogCancel(void)
{
}

static void
gameDialogGraphicCancel()
{
    gameInterfaceControls.graphicDialog.visible = 0;
    gameInterfaceControls.graphicDialogHelp.visible = 0;
}

static void
gameDialogGraphic(int tex_id)
{
    gameInterfaceControls.graphicDialog.visible = 1;
    gameInterfaceControls.graphicDialog.tex_id = tex_id;
}

static void
gameDialogGraphicDangerCountdown()
{
    static int passes = 60;
    gameDialogGraphic(TEXTURE_ID_DIALOG_GAMEOVER);
    passes--;
    if(passes <= 0)
    {
        gameDialogGraphicCancel();
        glFlightDrawframeHook = NULL;
        passes = 60;
    }
}

static void
gameDialogStopGameStatusMessages(void)
{
    gameDialogState.hideNetworkStatus = 1;
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
    static int passes = 120;
    passes--;
    
    if(passes > 0)
    {
        gameInterfaceControls.graphicDialogHelp.visible = 1;
        return;
    }
    
    gameDialogWelcomeMenu();
    gameDialogGraphicCancel();
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
    /*
    extern void gameInputTrimAbort(void);
    
    gameInputTrimAbort();
    gameDialogCancel();
     */
}

static void
gameDialogCalibrate()
{
    gameInputInit();
    gameInterfaceModalDialog(
                             "^A^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^A\n"
                             "^C                             ^C\n"
                             "^C                             ^C\n"
                             "^C   Calibrating controls..    ^C\n"
                             "^C                             ^C\n"
                             "^C                             ^C\n"
                             "^C                             ^C\n"
                             "^A^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^A",
                             "", "cancel", gameDialogCancelTrim, gameDialogCancelTrim);
}

static void
gameDialogCalibrateBegin()
{
    console_clear();
    
    gameInterfaceModalDialog(
                             "^A^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^A\n"
                             "^C                             ^C\n"
                             "^C    Hold your device still   ^C\n"
                             "^C  while calibrating controls ^C\n"
                             "^C                             ^C\n"
                             "^C       (Tap to begin)        ^C\n"
                             "^C                             ^C\n"
                             "^A^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^C^A",
                             "", "cancel", gameDialogCalibrate, gameDialogCalibrate);
}

static void
gameDialogResumePaused()
{
    needTrimLock = 1;
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

//static void gameDialogLanConnect()
//{
//    gameDialogCancel();
//    fireAction = ACTION_CONNECT_TO_GAME_LAN;
//    gameInterfaceProcessAction();
//}

//static int
//gameDialogSearchingForGame()
//{
//    extern int GameNetworkBonjourManagerBrowseBegin();
//    
//    if(gameDialogState.lanGameScanning)
//    {
//        gameDialogState.lanGameScanning = 0;
//        return 1;
//    }
//    else
//    {
//        gameNetwork_disconnect();
//        GameNetworkBonjourManagerBrowseBegin();
//        gameInterfaceModalDialog("Searching for game...\nConnect?", "", "",
//                                 gameDialogLanConnect, gameDialogCancel);
//        gameDialogState.lanGameScanning = 1;
//        return 0;
//    }
//}

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
    if(gameStateSinglePlayer.game_type == GAME_TYPE_DEATHMATCH && gameStateSinglePlayer.started)
    {
        return;
    }
    
    game_start(1, GAME_TYPE_DEATHMATCH);
    gameNetwork_startGame(300);
    gameStateSinglePlayer.max_enemies = gameDialogCounter;
    
    gameInterfaceControls.mainMenu.visible = gameInterfaceControls.textMenuControl.visible = 0;
    
    gameNetwork_alert("ALERT:game started!");
}

static void
gameDialogStartNetworkGameWait()
{
    gameInterfaceModalDialog("Waiting for guests\nTap when ready to start", "", "",
                             gameDialogStartNetworkGame2, gameDialogStartNetworkGame2);
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

//static void
//gameDialogStartNetworkGame()
//{
//    gameDialogCounter = 0;
//    gameStateSinglePlayer.map_use_current = 1;
//    gameInterfaceModalDialog("Ready to start?\nWait for more?\n", "Start", "Wait", gameDialogStartNetworkGameAddBots, gameDialogCancel);
//}

static void
gameDialogEnterGameNameDone()
{
    gameDialogClose();
    fireActionQueuedAfterEdit = ACTION_HOST_GAME;
    fireAction = ACTION_HOST_GAME;
    gameInterfaceProcessAction();
}

static void
gameDialogEnterGameName()
{
    const char* msg = ""
    "Local Game:\n"
    "* enter same \"game name\" as\n"
    "  your guests\n"
    "* first person hosts\n"
    "Internet Game:\n"
    "* host - leave this blank\n"
    "  (or type \"host\")\n"
    "* guest - enter IP address of\n"
    "  your host (see the help)\n"
    "* read help for firewall port config\n"
    ;
    gameInterfaceModalDialog(msg, "OK", "OK", gameDialogEnterGameNameDone, gameDialogCancel);
}

static void
gameDialogNetworkGameStatus()
{
    void (*okFunc)(void) = gameDialogCancel;
    
    if(gameDialogState.hideNetworkStatus) return;
    
    if(gameNetworkState.hostInfo.hosting)
    {
        okFunc = gameDialogStartNetworkGame2;
    }
    else
    {
        okFunc = gameDialogClose;
    }
    
    gameInterfaceModalDialog(gameNetworkState.gameStatsMessage, "OK", "OK", gameDialogStopGameStatusMessages, gameDialogStopGameStatusMessages);
}

static void gameDialogCancelString();

static void
gameDialogDisplayString(char *str)
{
    if(gameDialogState.displayStringCount > 15) return;
    
    strcpy(gameDialogState.displayStringBuf[gameDialogState.displayStringCount], str);
    gameDialogState.displayStringCount++;
    
    if(gameDialogState.displayStringCount == 1)
    {
        gameInterfaceModalDialog(gameDialogState.displayStringBuf[0], "OK", "",
                                 gameDialogCancelString, gameDialogCancelString);
    }
}

static void
gameDialogCancelString()
{
    if(gameDialogState.displayStringCount > 0) gameDialogState.displayStringCount--;
    
    if(gameDialogState.displayStringCount > 0)
    {
        gameInterfaceModalDialog(gameDialogState.displayStringBuf[gameDialogState.displayStringCount-1], "OK", "",
                                 gameDialogCancelString, gameDialogCancelString);
    }
}

static void
gameDialogScores()
{
    static char gameDialogScoresString[1024];
    
    sprintf(gameDialogScoresString, "firewall:%d\nSurvival:%d\nDefend:%d\nDeathmatch:%d\nTurret:%d\n",
            gameStateSinglePlayer.high_score[GAME_TYPE_COLLECT],
            gameStateSinglePlayer.high_score[GAME_TYPE_SURVIVAL],
            gameStateSinglePlayer.high_score[GAME_TYPE_DEFEND],
            gameStateSinglePlayer.high_score[GAME_TYPE_DEATHMATCH],
            gameStateSinglePlayer.high_score[GAME_TYPE_TURRET]);
    
    gameInterfaceModalDialog(gameDialogScoresString,
                             "OK", "OK",
                             gameDialogCancel, gameDialogCancel);
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
    
    gameInterfaceModalDialogEnqueue(scoreStr, "", "", gameDialogCancelString, gameDialogCancelString, &r, GAME_FRAME_RATE * 2.0);
    r.tex_id = TEXTURE_ID_CONTROLS_POPUPOVER_DIALOG_BOX;
    gameInterfaceControls.dialogRect.modal = 0;
}

static void
gameDialogMessagePopup(char *str)
{
    const float xscale = 2.0, yscale = 3.0;
    controlRect r;
    r = gameInterfaceControls.dialogRectDefault;
    
    r.x -= gameInterfaceControls.interfaceWidth / xscale;
    
    r.x += r.xw/xscale;
    r.y += r.yw/yscale;
    r.xw /= xscale;
    r.yw /= yscale;
    r.xm = -1;
    r.ym = 0;
    r.tex_id = 4;
    
    gameInterfaceModalDialogEnqueue(str, "", "", gameDialogCancelString, gameDialogCancelString, &r, GAME_FRAME_RATE * 2.0);
    r.tex_id = TEXTURE_ID_CONTROLS_POPUPOVER_DIALOG_BOX;
    gameInterfaceControls.dialogRect.modal = 0;
}

#endif
