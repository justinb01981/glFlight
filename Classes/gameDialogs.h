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
#include "gameCamera.h"

#define GAME_DIALOG_LIFE_MAX 999999

struct gameDialogStateStruct
{
    char gameDialogPortalToGameLast[255];
    int hideNetworkStatus;
    
    char displayStringBuf[1024][16];
    int displayStringCount;
};

extern struct gameDialogStateStruct gameDialogState;

extern int gameDialogCounter;

extern void AppDelegateOpenURL(const char* url);

extern void gameDialogStartNetworkGame();

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
gameDialogWelcomeMenu()
{
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
    extern int AppDelegateIsMultiplayerEager();
    static int passes = 200;
    passes--;
    
    if(gameInterfaceControls.touchCount > 0)
    {
        passes = 0;
    }
    
    if(passes > 0)
    {
        gameInterfaceControls.graphicDialogHelp.visible = 1;
        return;
    }
    
    glFlightDrawframeHook = NULL;
    
    if(AppDelegateIsMultiplayerEager())
    {
        //actions_menu_set(ACTION_CONNECT_TO_GAME);
        //fireAction = ACTION_CONNECT_TO_GAME;
    }

    gameDialogWelcomeMenu();
    
    gameDialogGraphicCancel();
}

static void
gameDialogGameOverCountdown()
{
    static int passes = 200;
    passes--;
    
    if(passes > 0)
    {
        return;
    }
    
    game_over();
    gameDialogWelcomeMenu();
    glFlightDrawframeHook = NULL;
}

static void
gameDialogGameOver()
{
    camera_locked_frames = 200;
    glFlightDrawframeHook = gameDialogGameOverCountdown;
}

static void
gameDialogWelcome()
{
    gameInterfaceControls.calibrateRect.visible = 0;
    
    if(/*gameSettingsLaunchCount % 2 == 1 &&  */ !gameSettingsRatingGiven && gameSettingsLaunchCount >= 2)
    {
        gameInterfaceModalDialog(WELCOMESTR
                                 "Please rate d0gf1ght!\n"
                                 "Unlock new vehicles in multiplayer!\n",
                                 "Sure", "No way",
                                 gameDialogRating, /*gameDialogWelcomeNoRating*/ gameDialogWelcomeMenu);
    }
    else
    {
        glFlightDrawframeHook = gameDialogMenuCountdown;
        gameDialogClose();
    }
}

static void
gameDialogCancelTrim()
{
}

static void
gameDialogCalibrate()
{
    gameDialogClose();
    gameInterfaceModalDialogDequeue();
    
    gameInputInit();
    /*
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
     */
    gameInterfaceControls.calibrateRect.visible = 1;
}

static void
gameDialogInitial()
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
gameDialogInitialCountdown()
{
    static int count = 240;
    
    if(count == 0)
    {
        glFlightDrawframeHook = NULL;
        count = 240;
        gameDialogInitial();
        
        return;
    }
    
    if(count == 240-32)
    {
        // TODO: freeze camera here and display ship
        gameInterfaceControls.mainMenu.visible = 0;
        
        // init camera
        camera_locked_frames = 240-32;
        
        gameCamera_yawRadians(3.14);
        gameCamera_MoveZ(-11);
        gameCamera_MoveY(3);
        gameCamera_pitchRadians(0.52);
    }
    gameCamera_MoveZ(0.03);
    
    count--;
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
gameDialogNetworkHostRegister()
{
    const int timeout = GAME_FRAME_RATE * 4;
    static int passes = timeout;
    
    passes--;
    if(passes <= 0)
    {
        glFlightDrawframeHook = NULL;
        gameNetworkState.gameNetworkHookOnMessage = NULL;
        
        passes = timeout;
    }
}

static void
gameDialogNetworkCountdownEnded()
{
    const int timeout = GAME_FRAME_RATE * 4;
    static int passes = timeout;
    
    passes--;
    if(passes <= 0)
    {
        console_write("NO games found - hosting\n");
        glFlightDrawframeHook = NULL;
        gameNetworkState.gameNetworkHookOnMessage = NULL;
        
        fireAction = ACTION_HOST_GAME_OR_COMMIT;
        gameInterfaceProcessAction();
        passes = timeout;
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
    
    gameDialogConnectToGameSuccessful2();
    
    game_start(1, GAME_TYPE_DEATHMATCH);
    gameNetwork_startGame(300);
    gameStateSinglePlayer.max_enemies = gameDialogCounter;
    
    gameNetwork_alert("ALERT:game started!");
}

static void
gameDialogStartNetworkGameWait()
{
    gameInterfaceModalDialog("Waiting for guests\nTap when everyone has\njoined, and 5 min game\nwill start...", "", "",
                             gameDialogStartNetworkGame2, gameDialogCancel);
}

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
    "^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D\n"
    "^D^D^DOn next screen  ^D^D^D\n"
    "^D^D^D                ^D^D^D\n"
    "^D^D^DHost: type name ^D^D^D\n"
    "^D^D^D                ^D^D^D\n"
    "^D^D^DGuest: type name^D^D^D\n"
    "^D^D^Dof host         ^D^D^D\n"
    "^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D^D\n"
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
    controlRect* pNextDialog = gameInterfaceModalDialogPeek();
    
    // modal dialog if ALERT
    if(strstr(gameNetworkState.gameStatsMessage, "ALERT:"))
    {
        if(gameDialogState.hideNetworkStatus || (pNextDialog && strstr(pNextDialog->text, "ALERT:"))) return;
        
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

static void
gameDialogDisconnectAndDequeue()
{
    gameNetwork_disconnect();
    gameDialogClose();
    gameInterfaceModalDialogDequeue();
}

static void
gameDialogNetworkGameEnded()
{
    const char* msg = "(game over)\ndisconnecting from host...";
    gameDialogState.hideNetworkStatus = 0;
    gameDialogNetworkGameStatus();
    
    controlRect* peek = gameInterfaceModalDialogPeek();
    if(strncmp(peek->text, msg, sizeof(peek->text)) == 0) return;
    
    gameInterfaceModalDialog(msg, "OK", "OK", gameDialogDisconnectAndDequeue, gameDialogDisconnectAndDequeue);
    
}

static void gameDialogCancelString();

static void
gameDialogDisplayString(char *str)
{
    //if(gameDialogState.displayStringCount > 15) return;
    
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
