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
    char gameDialogPortalToGameLast[255];
    int networkGameNameEntered;
    int hideNetworkStatus;
    
    char displayStringBuf[1024][16];
    int displayStringCount;
};

extern struct gameDialogStateStruct gameDialogState;

extern int gameDialogCounter;

extern void AppDelegateOpenURL(const char* url);

static void
gameDialogCancel(void)
{
    fireActionQueuedAfterEdit = ACTION_INVALID;
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
    gameDialogGraphic(TEXTURE_ID_DAMAGE_FLASH);
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
    gameSettingsRatingGiven = 1;
    AppDelegateOpenURL(APP_ITUNES_URL);
    gameInterfaceModalDialog("Launching ratings app...",
                             "Done", "OK",
                             gameDialogCancel, gameDialogCancel);
}

static void
gameDialogWelcomeRating()
{
    gameSettingsRatingGiven = 1;
    gameDialogWelcome();
}

static void
gameDialogWelcomeNoRating()
{
    gameSettingsRatingGiven = 1;
    gameDialogWelcome();
}

static void
gameDialogVisitHomepageYes()
{
    AppDelegateOpenURL("https://www.domain17.net/d0gf1ght");
}

static void
gameDialogVisitHomepage()
{
    gameInterfaceModalDialog("open browser\n(www.domain17.net/d0gf1ght)?",
                             "Done", "OK",
                             gameDialogVisitHomepageYes, gameDialogCancel);
}

#define WELCOMESTR                                  \
"^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D\n"    \
"^DWelcome to d0gf1ght!^D\n"                        \
"^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D\n"

static void
gameDialogMenuCountdown()
{
    static int passes = 240;
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

static void
gameDialogBrowseGamesCountdown()
{
    static int passes = 120;
    passes--;
    if(passes <= 0)
    {
        extern void gameInterfaceGameSearchTimedOut();
        extern void load_map_and_host_game();
        
        if(gameNetworkState.hostInfo.bonjour_lan)
        {
            if(gameNetwork_connect(gameSettingGameTitle, load_map_and_host_game) == GAME_NETWORK_ERR_NONE)
            {
            }
            else
            {
                console_write("Connection failed to %s", gameSettingGameTitle);
            }
        }
        else
        if(!gameNetworkState.map_downloaded)
        {
            gameInterfaceGameSearchTimedOut();
            /*
            strcpy(gameSettingGameTitle, "host");
            if(gameNetwork_connect(gameSettingGameTitle, load_map_and_host_game) == GAME_NETWORK_ERR_NONE)
            {
            }
            else
            {
                console_write("Connection failed to %s", gameSettingGameTitle);
            }
             */
        }
        
        glFlightDrawframeHook = NULL;
        passes = 120;
    }
}

static void
gameDialogMuteCountdown()
{
    static int passes = 8;
    passes--;
    if(passes <= 0)
    {
        gameInterfaceModalDialogDequeue();
        glFlightDrawframeHook = NULL;
        passes = 8;
    }
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
    gameInterfaceModalDialog("Waiting for guests\nTap when everyone has\njoined, and 5 min round\nwill start...", "", "",
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
gameDialogEnterGameNameDoneInternet()
{
    gameDialogClose();
    gameInterfaceProcessAction();
}

static void
gameDialogEnterGameNameDoneLan()
{
    gameDialogClose();
    fireAction = ACTION_HOST_GAME_LAN;
    gameInterfaceProcessAction();
}

static void
gameDialogEnterGameName(int action)
{
    const char* msg = ""
    "^D^D^Dmultiplayer howto^D^D^D\n"
    "^D^D^Don the next screen^D^D^D\n"
    "^Blocal games:\n"
    "^A to host - enter any room-name\n"
    "^A guest - enter host room-name\n"
    "^Binternet games:\n"
    "^A to host - enter any room name\n"
    "^A or blank to host a public game\n"
    "^A guest - enter d0gf1ght.domain17.net\n"
    "  for public game\n"
    "  or IP address of host\n"
    ;
    fireActionQueuedAfterEdit = action;
    if(action == ACTION_HOST_GAME_LAN)
    {
        gameInterfaceModalDialog(msg, "OK", "OK", gameDialogEnterGameNameDoneLan, gameDialogCancel);
    }
    else
    {
        gameInterfaceModalDialog(msg, "OK", "OK", gameDialogEnterGameNameDoneInternet, gameDialogCancel);
    }
}

static void
gameDialogNetworkGameStatus()
{
    void (*okFunc)(void) = gameDialogCancel;

    if(gameNetworkState.hostInfo.hosting)
    {
        okFunc = gameDialogStartNetworkGame2;
    }
    else
    {
        okFunc = gameDialogClose;
    }
    
    if(strstr(gameNetworkState.gameStatsMessage, "ALERT:"))
    {
        // modal dialog if > 1 line
        if(gameDialogState.hideNetworkStatus) return;
        gameInterfaceModalDialog(gameNetworkState.gameStatsMessage, "OK", "OK", gameDialogStopGameStatusMessages, gameDialogStopGameStatusMessages);
    }
    else if(strstr(gameNetworkState.gameStatsMessage, "HIDDEN:"))
    {
        
    }
    else
    {
        console_write(gameNetworkState.gameStatsMessage);
    }
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
