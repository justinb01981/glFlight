//
//  gameInterface.c
//  gl_flight
//
//  Created by jbrady on 5/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "math.h"
#include "gameInterface.h"
#include "quaternions.h"
#include "gameInput.h"
#include "textures.h"
#include "gameGlobals.h"
#include "gameUtils.h"
#include "gamePlay.h"
#include "gameAudio.h"
#include "gameCamera.h"
#include "gameNetwork.h"
#include "glFlight.h"
#include "gameDebug.h"
#include "gameSettings.h"
#include "gameDialogs.h"

int texture_id_block = TEXTURE_ID_BLOCK;

const char *charMap = "abcdefghijklmnopqrstuvwxyz0123456789. _-@!$%^&*";

static float x_last = -1;
static float y_last = -1;

void
gameInterfaceInit(double screenWidth, double screenHeight)
{
    int i;
    
    gameInterfaceControls.interfaceHeight = screenHeight;
    gameInterfaceControls.interfaceWidth = screenWidth;
    
    // origin (in non-landscape mode) is upper left
    controlRect accelRect = {0, 0, screenWidth-(screenWidth*0.15), (screenHeight * 0.4507)/4};
    gameInterfaceControls.accelerator = accelRect;
    gameInterfaceControls.accelerator.tex_id = /*37*/ 5;
    gameInterfaceControls.accelerator.visible = 1;
    
    controlRect trimRect = {screenWidth-(screenWidth * 0.15),
        screenHeight-(screenHeight * 0.15),
        screenWidth * 0.15,
        screenHeight * 0.15};
    gameInterfaceControls.trim = trimRect;
    gameInterfaceControls.trim.tex_id = 7;
    gameInterfaceControls.trim.visible = 1;
    
    
    controlRect fireRect = {0,
        screenHeight-(screenHeight*0.15),
        screenWidth * 0.15,
        screenHeight * 0.15};
    gameInterfaceControls.fire = fireRect;
    gameInterfaceControls.fire.tex_id = TEXTURE_ID_CONTROLS_FIRE;
    gameInterfaceControls.fire.visible = 1;
    
    /*
     controlRect lookRect = {screenWidth*0.15,
     screenHeight-(screenHeight*0.15),
     screenWidth * 0.15,
     screenHeight * 0.15};
     */
    controlRect lookRect = {screenWidth*0.0,
        screenHeight-(screenHeight*0.0),
        screenWidth * 0.0,
        screenHeight * 0.0};
    gameInterfaceControls.look = lookRect;
    gameInterfaceControls.look.tex_id = 5;
    gameInterfaceControls.look.visible = 1;
    
    controlRect actionRect = {screenWidth - (screenWidth*0.30),
        screenHeight-(screenHeight*0.15),
        screenWidth * 0.15,
        screenHeight * 0.15};
    /*
     controlRect actionRect = {screenWidth*0.0,
     screenHeight-(screenHeight*0.0),
     screenWidth * 0.0,
     screenHeight * 0.0};
     */
    gameInterfaceControls.action = actionRect;
    gameInterfaceControls.action.tex_id = 36;
    gameInterfaceControls.action.visible = 1;
    
    gameInterfaceControls.accelIndicator = accelRect;
    gameInterfaceControls.accelIndicator.xw = 10;
    gameInterfaceControls.accelIndicator.tex_id = 37;
    gameInterfaceControls.accelIndicator.visible = 1;
    
    gameInterfaceControls.radar = trimRect;
    gameInterfaceControls.radar.y -= (screenHeight*0.15);
    gameInterfaceControls.radar.tex_id = TEXTURE_ID_CONTROLS_RADAR;
    gameInterfaceControls.radar.visible = 1;
    
    controlRect menuRect = {
        screenWidth - (screenWidth*0.70),
        screenHeight - (screenHeight*0.30),
        screenWidth * 0.40,
        screenHeight * 0.30,
        39
    };
    gameInterfaceControls.menuControl = menuRect;
    
    gameInterfaceControls.mainMenu.x = 0;
    gameInterfaceControls.mainMenu.y = 0;
    gameInterfaceControls.mainMenu.xw = screenWidth;
    gameInterfaceControls.mainMenu.yw = screenHeight;
    gameInterfaceControls.mainMenu.tex_id = /*29*/ TEXTURE_ID_MAINMENU;
    gameInterfaceControls.mainMenu.visible = 1;
    
    float menuHeight = gameInterfaceControls.interfaceHeight * 0.7;
    float menuWidth = gameInterfaceControls.interfaceWidth * 0.7;
    gameInterfaceControls.textMenuControl.x = screenWidth/2 - menuWidth/2;
    gameInterfaceControls.textMenuControl.y = screenHeight/2 - menuHeight/2;
    gameInterfaceControls.textMenuControl.xw = menuWidth;
    gameInterfaceControls.textMenuControl.yw = menuHeight;
    gameInterfaceControls.textMenuControl.tex_id = TEXTURE_ID_CONTROLS_TEXTMENU;
    gameInterfaceControls.textMenuControl.visible = 0;
    

    gameInterfaceControls.textEditControl.xw = screenWidth * 0.3;
    gameInterfaceControls.textEditControl.yw = screenHeight * 0.4;
    gameInterfaceControls.textEditControl.x = (screenWidth - gameInterfaceControls.textEditControl.xw)/2;
    gameInterfaceControls.textEditControl.y = (screenHeight - gameInterfaceControls.textEditControl.yw)/2;
    gameInterfaceControls.textEditControl = gameInterfaceControls.textMenuControl;
    
    float helpWidth = 0.8;
    float helpHeight = 0.8;
    gameInterfaceControls.help.x = screenWidth/2 - (helpWidth*gameInterfaceControls.interfaceWidth)/2;
    gameInterfaceControls.help.y = screenHeight/2 - (helpHeight*gameInterfaceControls.interfaceHeight)/2;
    gameInterfaceControls.help.xw = helpWidth*gameInterfaceControls.interfaceWidth;
    gameInterfaceControls.help.yw = helpHeight*gameInterfaceControls.interfaceHeight;
    gameInterfaceControls.help.tex_id = TEXTURE_ID_CONTROLS_HELP;
    gameInterfaceControls.help.visible = 0;
    
    float dialogWidth = 0.48;
    float dialogHeight = 0.512;
    gameInterfaceControls.dialogRectDefault.x = gameInterfaceControls.interfaceWidth/2 - (dialogWidth*gameInterfaceControls.interfaceWidth)/2;
    gameInterfaceControls.dialogRectDefault.y = gameInterfaceControls.interfaceHeight/2 - (dialogHeight*gameInterfaceControls.interfaceHeight)/2;
    gameInterfaceControls.dialogRectDefault.xw = (dialogWidth*gameInterfaceControls.interfaceWidth);
    gameInterfaceControls.dialogRectDefault.yw = (dialogHeight*gameInterfaceControls.interfaceHeight);
    gameInterfaceControls.dialogRectDefault.tex_id = TEXTURE_ID_CONTROLS_DIALOG_BOX;
    gameInterfaceControls.dialogRectDefault.visible = 0;
    gameInterfaceControls.dialogRect = gameInterfaceControls.dialogRectDefault;
    
    float reticlem = 0.2;
    gameInterfaceControls.reticleRect.x = ((screenWidth)/2 - (reticlem*screenWidth)/2) - screenWidth*0.025;
    gameInterfaceControls.reticleRect.y = (screenHeight/2) - (reticlem*screenHeight)/2;
    gameInterfaceControls.reticleRect.xw = screenWidth*reticlem;
    gameInterfaceControls.reticleRect.yw = screenHeight*reticlem;
    gameInterfaceControls.reticleRect.tex_id = 49;
    /* replaced by 3d reticle */
    gameInterfaceControls.reticleRect.visible = 0;
    
    gameInterfaceControls.fireRect2 = gameInterfaceControls.fire;
    gameInterfaceControls.fireRect2.x += gameInterfaceControls.fireRect2.xw;
    gameInterfaceControls.fireRect2.visible = 0;
    
    gameInterfaceControls.fireRectMissle = gameInterfaceControls.fireRect2;
    gameInterfaceControls.fireRectMissle.tex_id = TEXTURE_ID_CONTROLS_MISSLE;
    gameInterfaceControls.fireRectMissle.visible = 1;
    
    gameInterfaceControls.fireRectMisc = gameInterfaceControls.fireRectMissle;
    gameInterfaceControls.fireRectMisc.y -= gameInterfaceControls.fireRectMisc.yw;
    gameInterfaceControls.fireRectMisc.tex_id = TEXTURE_ID_CONTROLS_TOW;
    
    gameInterfaceControls.fireRectBoost.x = screenWidth - (screenWidth*0.15);
    gameInterfaceControls.fireRectBoost.y = screenHeight * 0.0;
    gameInterfaceControls.fireRectBoost.xw = 0.15*screenWidth;
    gameInterfaceControls.fireRectBoost.yw = 0.15*screenHeight;
    
    gameInterfaceControls.fireRectBoost.tex_id = TEXTURE_ID_CONTROLS_BOOST;
    gameInterfaceControls.fireRectBoost.visible = 0;
    
    gameInterfaceControls.consoleTextRect.x = screenWidth * 0.95;
    gameInterfaceControls.consoleTextRect.y = screenHeight * 0.1;
    gameInterfaceControls.consoleTextRect.visible = 1;
    gameInterfaceControls.consoleTextRect.tex_id = -1;
    
    gameInterfaceControls.statsTextRect.x = screenWidth * 0.0;
    gameInterfaceControls.statsTextRect.y = screenHeight * 0.1;
    gameInterfaceControls.statsTextRect.visible = 1;
    gameInterfaceControls.statsTextRect.tex_id = -1;
    
    gameInterfaceControls.altControl = gameInterfaceControls.fireRectBoost;
    gameInterfaceControls.altControl.tex_id = TEXTURE_ID_CONTROLS_ROLL;
    gameInterfaceControls.altControl.visible = 0;
    
    i = 0;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.mainMenu;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.accelerator;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.trim;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.fire;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.look;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.action;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.accelIndicator;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.radar;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.menuControl;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.fireRect2;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.fireRectBoost;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.fireRectMisc;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.reticleRect;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.textMenuControl;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.fireRectMissle;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.help;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.dialogRect;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.consoleTextRect;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.statsTextRect;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.textEditControl;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.altControl;
    gameInterfaceControls.controlArray[i++] = NULL;
    
    i = 0;
    while(gameInterfaceControls.controlArray[i])
    {
        gameInterfaceControls.controlArray[i]->text[0] = '\0';
        gameInterfaceControls.controlArray[i]->textLeft[0] = '\0';
        gameInterfaceControls.controlArray[i]->textRight[0] = '\0';
        i++;
    }
    
    gameInterfaceControls.textHeight = (screenWidth/320) * 12;
    gameInterfaceControls.textWidth = (screenHeight/568) * 8;
    
    gameInterfaceControls.menuAction = ACTION_INVALID;
    
    gameInterfaceControls.menuControl.visible = 0;
    
    gameInterfaceControls.consoleHidden = 0;
    
    gameInterfaceControls.touchCount = 0;
}

void
gameInterfaceReset()
{
    gameInterfaceControls.menuAction = ACTION_INVALID;
    
    gameInterfaceControls.menuControl.visible = 0;
    
    gameInterfaceControls.consoleHidden = 0;
    
    gameInterfaceControls.touchCount = 0;
    
    gameInterfaceHandleAllTouchEnd();
}

static controlRect*
gameInterfaceFindControl(float x, float y)
{
    controlRect** controlSet = gameInterfaceControls.controlArray;
    controlRect* f = NULL;
    
    for(int i = 0; controlSet[i] != NULL; i++)
    {
        if(controlSet[i]->visible && controlSet[i] != &gameInterfaceControls.mainMenu &&
           x >= controlSet[i]->x && x < controlSet[i]->x+controlSet[i]->xw)
        {
            if(y >= controlSet[i]->y && y < controlSet[i]->y+controlSet[i]->yw)
            {
                f = controlSet[i];
            }
        }
    }
    
    // modal, this must be clicked
    if(gameInterfaceControls.dialogRect.visible &&
       gameInterfaceControls.dialogRect.modal)
    {
        if(f != &gameInterfaceControls.dialogRect &&
           f != &gameInterfaceControls.trim) return NULL;
    }
    
    return f;
}

void
gameInterfaceHandleTouchMove(float x, float y)
{
    const float move_thresh = 10;
    
    if(fabs(x-x_last) < move_thresh &&
       fabs(y-y_last) < move_thresh)
    {
        return;
    }
    x_last = x;
    y_last = y;
    
    controlRect* touchedControl = gameInterfaceFindControl(x, y);
    if(touchedControl)
    {
        touchedControl->touch_rx = (x - touchedControl->x) / touchedControl->xw;
        touchedControl->touch_ry = (y - touchedControl->y) / touchedControl->yw;
    }
    
    gameInterfaceControls.help.visible = 0;
    
    if(touchedControl == &gameInterfaceControls.accelerator)
    {
        float m = touchedControl->touch_rx;
        if(m <= 0.10) m = 0;
        targetSpeed = minSpeed + ((maxSpeed-minSpeed) * m);
    }
    else if(touchedControl == &gameInterfaceControls.radar)
    {
        radar_mode = !radar_mode;
        gameInterfaceControls.radar.tex_id = radar_mode? 4: 20;
    }
    else if(touchedControl == &gameInterfaceControls.trim)
    {
        gameInputTrimBegin();
        gyro_calibrate_log(100);
        gameInterfaceControls.trim.blinking = 0;
        controlsCalibrated = 1;
    }
    else if(touchedControl == &gameInterfaceControls.look)
    {
        /*
         viewRotationDegrees += 90;
         if(viewRotationDegrees >= 360) viewRotationDegrees = 0;
         */
    }
    else if(touchedControl == &gameInterfaceControls.fire)
    {
        //fireAction = ACTION_FIRE_BULLET;
    }
    else if(touchedControl == &gameInterfaceControls.fireRectMissle)
    {
        //fireAction = ACTION_FIRE_MISSLE;
    }
    else if(touchedControl == &gameInterfaceControls.action)
    {
        gameInterfaceControls.textMenuControl.visible = !gameInterfaceControls.textMenuControl.visible;
        //gameInterfaceControls.consoleHidden = gameInterfaceControls.textMenuControl.visible;
    }
    else if(touchedControl == &gameInterfaceControls.menuControl)
    {
    }
    else if(touchedControl == &gameInterfaceControls.textMenuControl)
    {
    }
    else if(touchedControl == &gameInterfaceControls.help)
    {
    }
    else if(touchedControl == &gameInterfaceControls.fireRectMisc)
    {
    }
    else
    {
        gameInterfaceControls.touchUnmappedX = x;
        gameInterfaceControls.touchUnmappedY = y;
    }
}

void
gameInterfaceEditString(char *ptr)
{
    gameInterfaceControls.textMenuControl.visible = 0;
    
    char *p = ptr;
    char *pd = gameInterfaceControls.textEditControl.text;
    while(*p)
    {
        int i = 0;
        while(charMap[i] != tolower(*p) && i < strlen(charMap)) i++;
        
        if(i < strlen(charMap))
        {
            *pd = charMap[i];
            pd++;
        }
        p++;
    }
    *pd = '\0';
    gameInterfaceControls.textEditControl.textDest = ptr;
    gameInterfaceControls.textEditControl.visible = 1;
}

void
gameInterfaceEditInt(int *ptr)
{
    gameInterfaceControls.textMenuControl.visible = 0;
    
    char *pd = gameInterfaceControls.textEditControl.text;
    sprintf(pd, "%d", *ptr);
    gameInterfaceControls.textEditControl.textDestInt = ptr;
    gameInterfaceControls.textEditControl.visible = 1;
}

void
gameInterfaceEditDone()
{
    gameInterfaceControls.textMenuControl.visible = 1;
    
    // re-init with new values
    gameNetwork_init(0, gameSettingsGameName, gameSettingsPlayerName, gameSettingsNetworkFrequency,
                     gameSettingsDirectoryServerName,
                     gameSettingsPortNumber,
                     gameSettingsLocalIPOverride);
}

void
gameInterfaceHandleTouchBegin(float x, float y)
{
    controlRect* touchedControl = gameInterfaceFindControl(x, y);
    if(touchedControl)
    {
        touchedControl->touched = 1;
        gameInterfaceHandleTouchMove(x, y);
    }
    
    if(touchedControl == &gameInterfaceControls.textMenuControl ||
       touchedControl == &gameInterfaceControls.fireRect2 ||
       touchedControl == &gameInterfaceControls.textEditControl)
    {
        int a[3][3] =
        {
            {0, 1, 0},
            {2, 0, 3},
            {5, 4, 0}
        };
        
        int rx = ((float) (1 - touchedControl->touch_rx) * 3.0); // inverted
        int ry = ((float) (1 - touchedControl->touch_ry) * 3.0); // inverted
        int rgnv = a[rx][ry];
        
        if(touchedControl == &gameInterfaceControls.fireRect2)
        {
            touchedControl = &gameInterfaceControls.textMenuControl;
            rgnv = 5;
        }
        
        if(touchedControl == &gameInterfaceControls.textEditControl)
        {
            char *p = &(touchedControl->text[strlen(touchedControl->text)-1]);
            
            int i = 0;
            while(charMap[i] != *p) i++;
            
            switch(rgnv)
            {
                case 1:
                    i++;
                    break;
                case 4:
                    i--;
                    break;
                case 2:
                    *(p+1) = *p;
                    break;
                case 3:
                    *p = '\0';
                    break;
                case 5:
                    /* save */
                    if(touchedControl->textDestInt)
                    {
                        char*p = touchedControl->text;
                        int invalid = 0;
                        while(*p)
                        {
                            if(*p < '0' || *p > '9') invalid = 1;
                            p++;
                        }
                        
                        if(!invalid)
                        {
                            *touchedControl->textDestInt = atoi(touchedControl->text);
                        }
                    }
                    else if(touchedControl->textDest)
                    {
                        strcpy(touchedControl->textDest, touchedControl->text);
                    }
                    touchedControl->textDestInt = NULL;
                    touchedControl->textDest = NULL;
                    touchedControl->visible = 0;
                    gameInterfaceEditDone();
                    break;
            default:
                    gameAudioPlaySoundAtLocation("bump", gameCamera_getX(), gameCamera_getY(), gameCamera_getZ());
                    console_write("use menu arrows/ok");
                break;
            }
            
            if(*p)
            {
                if(i < 0) i = strlen(charMap)-1;
                if(i >= strlen(charMap)) i = 0;
                *p = charMap[i];
            }
            
        }
        else
        {
            switch(rgnv)
            {
                case 0:
                    break;
                case 1:
                    action_prev();
                    break;
                case 2:
                    action_sub_next();
                    break;
                case 3:
                    action_sub_prev();
                    break;
                case 4:
                    action_next();
                    break;
                case 5:
                    switch(fireAction)
                {
                    case ACTION_SETTING_PLAYER_NAME:
                        gameInterfaceEditString(gameSettingsPlayerName);
                        break;
                    case ACTION_SETTING_GAME_NAME:
                        gameInterfaceEditString(gameSettingsGameName);
                        break;
                    case ACTION_SETTING_UPDATE_FREQUENCY:
                        gameInterfaceEditInt(&gameSettingsNetworkFrequency);
                        break;
                    case ACTION_SETTING_PORT_NUMBER:
                        gameInterfaceEditInt(&gameSettingsPortNumber);
                        break;
                    case ACTION_SETTING_DIRECTORY_SERVER:
                        gameInterfaceEditString(gameSettingsDirectoryServerName);
                        break;
                    case ACTION_SETTING_LOCAL_IP_OVERRIDE:
                        gameInterfaceEditString(gameSettingsLocalIPOverride);
                        break;
                        
                    case ACTION_SETTING_SHIP_MODEL:
                    {
                        int model_new = MODEL_SHIP1;
                        
                        if(!gameSettingsRatingGiven)
                        {
                            gameDialogRating();
                            break;
                        }
                            
                        
                        switch(model_my_ship)
                        {
                            case MODEL_SHIP1:
                                model_new = MODEL_SHIP2;
                                break;
                            case MODEL_SHIP2:
                                model_new = MODEL_SHIP3;
                                break;
                            default:
                                model_new = MODEL_SHIP1;
                                break;
                        }
                        model_my_ship = model_new;
                        console_write("model %d (on next respawn)", model_my_ship);
                    }
                        break;
                        
                    case ACTION_SETTING_CONTROL_TYPE:
                        gameSettingsSimpleControls = !gameSettingsSimpleControls;
                        break;
                        
                    case ACTION_SETTING_LOCK_CAMERA:
                        if(camera_chase_frames >= GAME_CAMERA_CHASE_LAG_FRAMES_MAX)
                        {
                            camera_chase_frames = 0;
                            camera_locked_frames = 60 * 30;
                        }
                        else if(camera_chase_frames == 0)
                        {
                            camera_chase_frames = 1;
                            camera_locked_frames = 0;
                        }
                        else camera_chase_frames *= 2;
                        
                        console_write("camera chase:%d locked:%d",
                                      camera_chase_frames,
                                      camera_locked_frames? 1: 0);
                        break;
                        
                    case ACTION_SETTING_SHOW_DEBUG:
                        console_write("enemy_intelligence:%f,myPlayerId:%d\nbuild date/time:%s - %s\n",
                                      gameStateSinglePlayer.enemy_intelligence,
                                      gameNetworkState.my_player_id, __DATE__, __TIME__);
                        break;
                        
                    case ACTION_SETTING_TWEAK_PHYSICS:
                        C_FRICTION *= 1.5;
                        if(C_FRICTION >= 0.5) C_FRICTION = 0.005;
                        console_write("C_FRICTION:%f", C_FRICTION);
                        break;
                        
                    case ACTION_SETTING_GYRO_FEEDBACK:
                        if(GYRO_FEEDBACK == 0.0) GYRO_FEEDBACK = 0.0005;
                        GYRO_FEEDBACK *= 2;
                        if(GYRO_FEEDBACK >= 0.2) GYRO_FEEDBACK = GYRO_FEEDBACK_DEFAULT;
                        console_write("GYRO_FEEDBACK:%f", GYRO_FEEDBACK);
                        break;
                        
                    case ACTION_SETTING_SHIP_TEXTURE:
                        switch(texture_id_playership)
                    {
                        case TEXTURE_ID_SHIP1:
                            texture_id_playership = TEXTURE_ID_SHIP2;
                            break;
                        case TEXTURE_ID_SHIP2:
                            texture_id_playership = TEXTURE_ID_SHIP3;
                            break;
                        case TEXTURE_ID_SHIP3:
                            texture_id_playership = TEXTURE_ID_SHIP4;
                            break;
                        case TEXTURE_ID_SHIP4:
                            texture_id_playership = TEXTURE_ID_SHIP5;
                            break;
                        case TEXTURE_ID_SHIP5:
                            texture_id_playership = TEXTURE_ID_SHIP6;
                            break;
                        case TEXTURE_ID_SHIP6:
                            texture_id_playership = TEXTURE_ID_SHIP7;
                            break;
                        case TEXTURE_ID_SHIP7:
                            texture_id_playership = TEXTURE_ID_SHIP8;
                            break;
                        default:
                            texture_id_playership = TEXTURE_ID_SHIP1;
                            break;
                    }
                        
                        WorldElemListNode* pShip = world_elem_list_find(my_ship_id, &gWorld->elements_list);
                        if(pShip)
                        {
                            pShip->elem->texture_id = texture_id_playership;
                        }
                        
                        console_write("texture:%d\r", texture_id_playership);
                        gameInterfaceControls.textMenuControl.hide_frames = 120;
                        break;
                        
                    case ACTION_SETTING_INPUT_SENSITIVITY:
                        GYRO_DC += 0.001;
                        if(GYRO_DC >= 0.01) GYRO_DC = 0.001;
                        console_write("Input sensitivity: %f\n", GYRO_DC);
                        break;
                        
                    case ACTION_SETTING_BOTINTELLIGENCE:
                        gameStateSinglePlayer.setting_bot_intelligence++;
                        if(gameStateSinglePlayer.setting_bot_intelligence > 10)
                            gameStateSinglePlayer.setting_bot_intelligence = 1;
                        console_write("Bot skill:%d\n", gameStateSinglePlayer.setting_bot_intelligence);
                        break;
                        
                    case ACTION_SETTING_AUDIO:
                        gameAudioMuted = !gameAudioMuted;
                        console_write("Audio muted:%d\n", gameAudioMuted);
                        break;
                        
                    default:
                        gameInterfaceControls.menuAction = fireAction;
                        break;
                }
                    break;
                default:
                    break;
            }
        }
    }
    
    if(touchedControl == &gameInterfaceControls.dialogRect)
    {
        if(touchedControl->touch_ry <= 0.5)
        {
            touchedControl->visible = 0;
            gameInterfaceControls.consoleHidden = 0;
            if(gameInterfaceControls.dialogRectActionLeft) gameInterfaceControls.dialogRectActionLeft();
        }
        else
        {
            touchedControl->visible = 0;
            gameInterfaceControls.consoleHidden = 0;
            if(gameInterfaceControls.dialogRectActionRight) gameInterfaceControls.dialogRectActionRight();
        }
    }
    
    if(touchedControl == &gameInterfaceControls.fireRectBoost)
    {
        speedBoost = SPEED_BOOST_FRAMES;
        gameInterfaceControls.fireRectBoost.visible = 0;
        gameAudioPlaySoundAtLocation("speedboost", my_ship_x, my_ship_y, my_ship_z);
    }
    
    // update what controls are visible based on menu conditions
    gameInterfaceControls.fireRect2.visible = 0;
    
    if(fireAction == ACTION_SHOOT_BLOCK ||
       fireAction == ACTION_BLOCK_TEXTURE ||
       fireAction == ACTION_DROP_BLOCK ||
       fireAction == ACTION_REPLACE_OBJECT ||
       fireAction == ACTION_SCALE_OBJECT)
    {
        gameInterfaceControls.fireRect2.visible = 1;
    }
    gameInterfaceControls.fireRectMissle.visible = !gameInterfaceControls.fireRect2.visible;
    
    gameInterfaceControls.touchCount++;
    
    if(touchedControl != &gameInterfaceControls.trim) game_paused = 0;
}

void
gameInterfaceHandleTouchEnd(float x, float y)
{
    controlRect* touchedControl = gameInterfaceFindControl(x, y);
    
    x_last = y_last = -1;
    
    if(touchedControl)
    {
        touchedControl->touched = 0;
        //gameInterfaceHandleTouchMove(x, y);
        touchedControl->touch_end_last = time_ms;
    }
    
    if(touchedControl == &gameInterfaceControls.trim)
    {
        needTrim = 0;
    }
    
    gameInterfaceControls.touchUnmappedX = gameInterfaceControls.touchUnmappedY = -1;
    
    gameInterfaceControls.touchCount--;
}

void
gameInterfaceHandleAllTouchEnd()
{
    controlRect** controlSet = gameInterfaceControls.controlArray;
    
    for(int i = 0; controlSet[i] != NULL; i++)
    {
        controlSet[i]->touched = 0;
    }
    
    gameInterfaceControls.touchUnmappedX = gameInterfaceControls.touchUnmappedY = -1;
}

void
gameInterfaceModalDialogWithRect(char* msg, char *buttonLeft, char *buttonRight, void (*cbLeft)(void), void (*cbRight)(void),
                         controlRect* overrideRect, unsigned long life_frames)
{
    gameInterfaceControls.dialogLifeFrames = life_frames;
    
    gameInterfaceControls.dialogRect = *overrideRect;
    
    sprintf(gameInterfaceControls.dialogRect.text,
            "%s",
            msg? msg: "");
    sprintf(gameInterfaceControls.dialogRect.textLeft,
            "%s",
            buttonLeft? buttonLeft: "");
    sprintf(gameInterfaceControls.dialogRect.textRight,
            "%s",
            buttonRight? buttonRight: "");
    gameInterfaceControls.dialogRectActionLeft = cbLeft;
    gameInterfaceControls.dialogRectActionRight = cbRight;
    gameInterfaceControls.dialogRect.visible = 1;
    gameInterfaceControls.dialogRect.modal = 1;
}

void
gameInterfaceModalDialog(char* msg, char *buttonLeft, char *buttonRight, void (*cbLeft)(void), void (*cbRight)(void))
{
    gameInterfaceModalDialogWithRect(msg, buttonLeft, buttonRight, cbLeft, cbRight, &gameInterfaceControls.dialogRectDefault, 999999);
}

void
gameInterfaceSetInterfaceState(InterfaceMiscState state)
{
    switch(state)
    {
        case INTERFACE_STATE_BOOST_AVAIL:
            gameInterfaceControls.fireRectBoost.visible = 1;
            break;
            
        case INTERFACE_STATE_BOOST_AVAIL_NO:
            gameInterfaceControls.fireRectBoost.visible = 0;
            break;
            
        case INTERFACE_STATE_TOWING:
            gameInterfaceControls.fireRectMisc.visible = 1;
            break;
            
        case INTERFACE_STATE_TOWING_NONE:
            gameInterfaceControls.fireRectMisc.visible = 0;
            break;
            
        case INTERFACE_STATE_TRIM_BLINKING:
            gameInterfaceControls.trim.blinking = 1;
            break;
            
        case INTERFACE_STATE_ONSCREEN_INPUT_CONTROL:
            gameInterfaceControls.altControl.visible = 1;
            gameInterfaceControls.fireRectBoost.visible = 0;
            break;
            
        case INTERFACE_STATE_CLOSE_MENU:
            gameInterfaceControls.textMenuControl.visible = 0;
            gameInterfaceControls.menuControl.visible = 0;
            break;
            
        default:
            break;
    }
}