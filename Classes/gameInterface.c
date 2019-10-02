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

extern void appWriteSettings(void);
extern void GameNetworkBonjourManagerBrowseBegin(void);
extern void game_add_network_portal(char* name);
extern void load_map_and_host_game(void);

controls gameInterfaceControls;

int texture_id_block = TEXTURE_ID_BLOCK;

const char *charMap = "abcdefghijklmnopqrstuvwxyz0123456789. _-@!$%^&*";

// variables scoped for fireAction menu handling
unsigned int maps_list_idx = 0;
int game_map_custom_loaded = 0;
int game_start_difficulty = 2;
int game_start_score = 0;

static void trimThenHideCalibrateRect(void)
{
    gameInterfaceControls.calibrateRect.visible = 0;
}

void
gameInterfaceInit(double screenWidth, double screenHeight)
{
    int i;
    controlRect tmpRect = {
        screenWidth-(screenWidth * 0.15),
        screenHeight-(screenHeight * 0.15),
        screenWidth * 0.15,
        screenHeight * 0.15
    };
    
    gameInterfaceControls.interfaceHeight = screenHeight;
    gameInterfaceControls.interfaceWidth = screenWidth;
    
    gameInterfaceControls.textWidth = ceil(screenWidth / 164);
    gameInterfaceControls.textHeight = ceil(gameInterfaceControls.textWidth * 2.2 * (screenWidth/screenHeight));
    
    // origin (in non-landscape mode) is upper left
    // (in landscape mode, screenWidth = Y-axis, screenHeight = X-axis
    controlRect accelRect = {0, 0, screenWidth-(screenWidth*0.30), (screenHeight * 0.4507)/4};
    gameInterfaceControls.accelerator = accelRect;
    gameInterfaceControls.accelerator.tex_id = /*37*/ 5;
    gameInterfaceControls.accelerator.visible = 1;
    
    gameInterfaceControls.trim.tex_id = 7;
    gameInterfaceControls.trim.visible = 0;
    gameInterfaceControls.trim.x = screenWidth - (screenWidth*0.15);
    gameInterfaceControls.trim.y = screenHeight * 0.0;
    gameInterfaceControls.trim.xw = 0.15*screenWidth;
    gameInterfaceControls.trim.yw = 0.15*screenHeight;
    
    gameInterfaceControls.fire = (controlRect) {
        screenWidth * 0.0,
        screenHeight-(screenHeight*0.20),
        screenWidth * 0.20,
        screenHeight * 0.20};
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
    gameInterfaceControls.textMenuButton = actionRect;
    gameInterfaceControls.textMenuButton.tex_id = TEXTURE_ID_CONTROLS_MENUBUTTON;
    gameInterfaceControls.textMenuButton.visible = 0;
    
    gameInterfaceControls.accelIndicator = accelRect;
    gameInterfaceControls.accelIndicator.xw = 10;
    gameInterfaceControls.accelIndicator.tex_id = 37;
    gameInterfaceControls.accelIndicator.visible = 1;
    
    gameInterfaceControls.radar = (controlRect) {
        screenWidth-screenWidth*0.15,
        screenHeight,
        screenWidth*0.15,
        screenHeight*0.15
    };
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
    
    gameInterfaceControls.textMenuControl.x = screenWidth*0.1;
    gameInterfaceControls.textMenuControl.y = screenHeight*0.2;
    gameInterfaceControls.textMenuControl.xw = screenWidth*0.8;
    gameInterfaceControls.textMenuControl.yw = screenHeight*0.6;
    gameInterfaceControls.textMenuControl.tex_id = TEXTURE_ID_CONTROLS_TEXTMENU;
    gameInterfaceControls.textMenuControl.visible = 0;
    
    float helpWidth = 1.0;
    float helpHeight = 1.0;
    gameInterfaceControls.graphicDialog.x = ((1.0-helpWidth)/2) * screenWidth;
    gameInterfaceControls.graphicDialog.y = ((1.0-helpHeight)/2) * screenHeight;
    gameInterfaceControls.graphicDialog.xw = helpWidth*screenWidth;
    gameInterfaceControls.graphicDialog.yw = helpHeight*screenHeight;
    gameInterfaceControls.graphicDialog.tex_id = TEXTURE_ID_DIALOG_GAMEOVER;
    gameInterfaceControls.graphicDialog.visible = 0;
    
    gameInterfaceControls.graphicDialogHelp.x = 0;
    gameInterfaceControls.graphicDialogHelp.y = 0;
    gameInterfaceControls.graphicDialogHelp.xw = screenWidth;
    gameInterfaceControls.graphicDialogHelp.yw = screenHeight;
    gameInterfaceControls.graphicDialogHelp.tex_id = TEXTURE_ID_HELPOVERLAY;
    gameInterfaceControls.graphicDialogHelp.visible = 0;
    
    float dialogWidth = /*0.48*/ 0.60;
    float dialogHeight = 0.512;
    gameInterfaceControls.dialogRectDefault.x = gameInterfaceControls.interfaceWidth/2 - (dialogWidth*gameInterfaceControls.interfaceWidth)/2;
    gameInterfaceControls.dialogRectDefault.y = gameInterfaceControls.interfaceHeight/2 - (dialogHeight*gameInterfaceControls.interfaceHeight)/2;
    gameInterfaceControls.dialogRectDefault.xw = (dialogWidth*gameInterfaceControls.interfaceWidth);
    gameInterfaceControls.dialogRectDefault.yw = (dialogHeight*gameInterfaceControls.interfaceHeight);
    gameInterfaceControls.dialogRectDefault.tex_id = TEXTURE_ID_CONTROLS_DIALOG_BOX;
    gameInterfaceControls.dialogRectDefault.visible = 0;
    gameInterfaceControls.dialogRect = gameInterfaceControls.dialogRectDefault;
    
    gameInterfaceControls.keyboardEntry.xw = screenWidth * 0.64;
    gameInterfaceControls.keyboardEntry.yw = screenHeight * 0.5;
    gameInterfaceControls.keyboardEntry.x = screenWidth*0.1;
    gameInterfaceControls.keyboardEntry.y = (screenHeight - gameInterfaceControls.keyboardEntry.yw)/2;
    gameInterfaceControls.keyboardEntry.tex_id = /*29*/ 108;
    gameInterfaceControls.keyboardEntry.visible = 0;
    
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
    
    gameInterfaceControls.fireRectMissle = (controlRect) {
        screenWidth*0.20,
        screenHeight-(screenHeight*0.15),
        screenWidth*0.15,
        screenHeight*0.15
    };
    gameInterfaceControls.fireRectMissle.tex_id = TEXTURE_ID_CONTROLS_MISSLE;
    gameInterfaceControls.fireRectMissle.visible = 1;
    
    gameInterfaceControls.fireRectMisc = gameInterfaceControls.fireRectMissle;
    gameInterfaceControls.fireRectMisc.y -= gameInterfaceControls.fireRectMisc.yw;
    gameInterfaceControls.fireRectMisc.tex_id = TEXTURE_ID_CONTROLS_TOW;
    
    gameInterfaceControls.fireRectBoost.x = screenWidth - (screenWidth*0.15);
    gameInterfaceControls.fireRectBoost.y = screenHeight;
    gameInterfaceControls.fireRectBoost.xw = 0.15*screenWidth;
    gameInterfaceControls.fireRectBoost.yw = 0.15*screenHeight;
    gameInterfaceControls.fireRectBoost.tex_id = TEXTURE_ID_CONTROLS_BOOST;
    gameInterfaceControls.fireRectBoost.visible = 0;
    
    gameInterfaceControls.consoleTextRect.x = screenWidth - gameInterfaceControls.textHeight*(console_lines_max-1);
    gameInterfaceControls.consoleTextRect.y = screenHeight * 0.15;
    gameInterfaceControls.consoleTextRect.xw = 0.1*screenWidth;
    gameInterfaceControls.consoleTextRect.yw = 0.5*screenHeight;
    gameInterfaceControls.consoleTextRect.visible = 1;
    gameInterfaceControls.consoleTextRect.tex_id = -1;
    gameInterfaceControls.consoleTextRect.text_align_topleft = 1;
    
    gameInterfaceControls.statsTextRect.x = screenWidth * 0.15;
    gameInterfaceControls.statsTextRect.y = screenHeight * 0.15;
    gameInterfaceControls.statsTextRect.xw = 0.05*screenWidth;
    gameInterfaceControls.statsTextRect.yw = screenHeight/3;
    gameInterfaceControls.statsTextRect.visible = 1;
    gameInterfaceControls.statsTextRect.tex_id = -1;
    gameInterfaceControls.statsTextRect.text_align_topleft = 1;
    
    gameInterfaceControls.altControl = gameInterfaceControls.fireRectBoost;
    gameInterfaceControls.altControl.tex_id = TEXTURE_ID_CONTROLS_ROLL;
    gameInterfaceControls.altControl.visible = 0;
    
    gameInterfaceControls.calibrateRect = gameInterfaceControls.dialogRectDefault;
    gameInterfaceControls.calibrateRect.tex_id = TEXTURE_ID_CONTROLS_RADAR;
    gameInterfaceControls.calibrateRect.visible = 0;
    gameInterfaceControls.calibrateRect.ignore_touch  = 1;
    
    i = 0;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.mainMenu;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.accelerator;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.trim;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.fire;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.look;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.textMenuButton;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.accelIndicator;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.radar;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.menuControl;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.statsTextRect;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.fireRect2;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.fireRectBoost;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.fireRectMisc;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.reticleRect;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.textMenuControl;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.fireRectMissle;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.dialogRect;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.graphicDialog;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.graphicDialogHelp;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.consoleTextRect;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.altControl;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.keyboardEntry;
    gameInterfaceControls.controlArray[i++] = &gameInterfaceControls.calibrateRect;
    gameInterfaceControls.controlArray[i++] = NULL;
    
    i = 0;
    while(gameInterfaceControls.controlArray[i])
    {
        gameInterfaceControls.controlArray[i]->text[0] = '\0';
        gameInterfaceControls.controlArray[i]->textLeft[0] = '\0';
        gameInterfaceControls.controlArray[i]->textRight[0] = '\0';
        i++;
    }
    
    gameInterfaceControls.menuControl.visible = 0;
    
    gameInterfaceControls.consoleHidden = 0;
    
    gameInterfaceControls.touchCount = 0;
}

void
gameInterfaceReset()
{
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
        if(!controlSet[i]->ignore_touch && controlSet[i]->visible && controlSet[i] != &gameInterfaceControls.mainMenu &&
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
    if(gameInterfaceControls.touchId-1 >= TOUCHES_MAX)
    {
        DBPRINTF(("gameInterfaceControls.touchId %d > TOUCHES_MAX (%d), ignoring", gameInterfaceControls.touchId, TOUCHES_MAX));
        return;
    }
    
    controlRect* touchedControl = gameInterfaceFindControl(x, y);
    if(touchedControl)
    {
        touchedControl->touch_rx = (x - touchedControl->x) / touchedControl->xw;
        touchedControl->touch_ry = (y - touchedControl->y) / touchedControl->yw;
        //DBPRINTF(("touch_rx: %f touch_ry: %f", touchedControl->touch_rx, touchedControl->touch_ry));
    }
    
    if(touchedControl == &gameInterfaceControls.accelerator)
    {
        float m = touchedControl->touch_rx;
        if(m <= 0.10) m = 0;
        targetSpeed = minSpeed + ((maxSpeed-minSpeed) * m);
    }
    else if(touchedControl == &gameInterfaceControls.fire)
    {
        //fireAction = ACTION_FIRE_BULLET;
    }
    else
    {
        gameInterfaceControls.touchUnmappedX = x;
        gameInterfaceControls.touchUnmappedY = y;
        gameInterfaceControls.touchUnmapped = 1;
    }
    
    if(gameInterfaceControls.pTouchedLastControl == touchedControl &&
       touchedControl != &gameInterfaceControls.accelerator)
    {
        return;
    }
    gameInterfaceControls.pTouchedLastControl = touchedControl;
    
    // controls after this ignore move events
    if(!touchedControl || !touchedControl->touch_began) return;
    
    if(touchedControl == &gameInterfaceControls.radar)
    {
        radar_mode = !radar_mode;
        gameInterfaceControls.radar.tex_id = radar_mode? 4: 20;
    }
    else if(touchedControl == &gameInterfaceControls.trim)
    {
        gameInputTrimBegin(trimThenHideCalibrateRect);
        gyro_calibrate_log(100);
        gameInterfaceControls.trim.blinking = 0;
        gameInterfaceControls.calibrateRect.visible = 1;
        //controlsCalibrated = 1;
    }
    else if(touchedControl == &gameInterfaceControls.look)
    {
        /*
         viewRotationDegrees += 90;
         if(viewRotationDegrees >= 360) viewRotationDegrees = 0;
         */
    }
    else if(touchedControl == &gameInterfaceControls.fireRectMissle)
    {
        //fireAction = ACTION_FIRE_MISSLE;
    }
    else if(touchedControl == &gameInterfaceControls.textMenuButton)
    {
        gameInterfaceControls.textMenuControl.visible = !gameInterfaceControls.textMenuControl.visible;
    }
    else if(touchedControl == &gameInterfaceControls.menuControl)
    {
    }
    else if(touchedControl == &gameInterfaceControls.textMenuControl)
    {
    }
    else if(touchedControl == &gameInterfaceControls.graphicDialog)
    {
    }
    else if(touchedControl == &gameInterfaceControls.fireRectMisc)
    {
    }
}

void
gameInterfaceEditString(char *ptr)
{
    gameInterfaceControls.textMenuControl.visible = 0;
    
    char *p = ptr;
    char *pd = gameInterfaceControls.keyboardEntry.text;
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
    gameInterfaceControls.keyboardEntry.textDest = ptr;
    gameInterfaceControls.keyboardEntry.visible = 1;
}

void
gameInterfaceEditInt(int *ptr)
{
    gameInterfaceControls.textMenuControl.visible = 0;
    
    char *pd = gameInterfaceControls.keyboardEntry.text;
    sprintf(pd, "%d", *ptr);
    gameInterfaceControls.keyboardEntry.textDestInt = ptr;
    gameInterfaceControls.keyboardEntry.visible = 1;
}

void
gameInterfaceEditDone()
{
    gameInterfaceControls.keyboardEntry.visible = 0;
    
    // re-init with new values
    gameNetwork_init(0, gameSettingGameTitle, gameSettingsPlayerName, gameSettingsNetworkFrequency,
                     gameSettingsPortNumber,
                     gameSettingsLocalIPOverride);
    
    gameInterfaceControls.keyboardEntry.text[0] = '\0';
    
    fireAction = fireActionQueuedAfterEdit;
    fireActionQueuedAfterEdit = ACTION_INVALID;
    gameInterfaceProcessAction();
    
    appWriteSettings();
}

void
gameInterfaceTouchIDSet(int id)
{
    gameInterfaceControls.touchId = id;
}

void
gameInterfaceHandleTouchBegin(float x, float y)
{
    int rgnv = 0;

    controlRect* touchedControl = gameInterfaceFindControl(x, y);
    
    gameDialogGraphicCancel();
    
    if(touchedControl)
    {
        touchedControl->touch_began = 1;
        touchedControl->touch_id = gameInterfaceControls.touchId;
        gameInterfaceHandleTouchMove(x, y);
        
        gameInterfaceControls.textMenuControl.tex_id = TEXTURE_ID_CONTROLS_TEXTMENU;
    }
    else
    {
        gameInterfaceControls.textMenuControl.tex_id = TEXTURE_ID_CONTROLS_TEXTMENU_OUTOFBOUNDS;
    }
    
    if(touchedControl == &gameInterfaceControls.textMenuControl ||
       touchedControl == &gameInterfaceControls.fireRect2 ||
       touchedControl == &gameInterfaceControls.keyboardEntry)
    {
        float touchRegionsXY2048[][3] = {
            {650,1446, 1}, // up
            {650,1844, 4}, // down
            {460,1652, 3}, // left
            {844,1654, 2}, // right
            {1320,1654, 5} // enter
        };
        float touchRadius = 200;
        float RMin = touchRadius;
        
        if(touchedControl == &gameInterfaceControls.textMenuControl)
        {
            int outofbounds = 1;
            int r;
            for(r = 0; r < sizeof(touchRegionsXY2048)/(sizeof(float)*2); r++)
            {
                float dx = ((touchedControl->touch_ry * 2048) - touchRegionsXY2048[r][0]);
                float dy = (((1-touchedControl->touch_rx) * 2048) - touchRegionsXY2048[r][1]);
                float R = sqrt(dx*dx + dy*dy);
                if(R < RMin)
                {
                    RMin = R;
                    rgnv = touchRegionsXY2048[r][2];
                    outofbounds = 0;
                }
            }
            
            gameInterfaceControls.textMenuControl.tex_id = outofbounds ? TEXTURE_ID_CONTROLS_TEXTMENU_OUTOFBOUNDS : TEXTURE_ID_CONTROLS_TEXTMENU;
        }
        else if(touchedControl == &gameInterfaceControls.keyboardEntry)
        {
            int col = (int) floor((float) (touchedControl->touch_ry) * 10); // inverted
            int row = (int) floor((float) (1 - touchedControl->touch_rx) * 5); // inverted
            if(row < 5 && col < 10)
            {
                char KB[5][10] = {
                    {'1','2','3','4','5','6','7','8','9','0'},
                    {'q','w','e','r','t','y','u','i','o','p'},
                    {'a','s','d','f','g','h','j','k','l','@'},
                    {':','z','x','c','v','b','n','m',',','.'},
                    {'D','D',' ',' ',' ',' ',' ',' ',' ','\n'}
                };
                
                char key = KB[(int)roundf(row)][(int)roundf(col)];
                size_t l = strlen(touchedControl->text);
                char *p = &(touchedControl->text[l]);
                
                if(key == '\n')
                {
                    /* save */
                    *p = '\0';
                    
                    if(touchedControl->textDestInt)
                    {
                        char* p = touchedControl->text;
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
                    
                    /*
                    if(fireAction == ACTION_CONNECT_TO_GAME ||
                       fireAction == ACTION_HOST_GAME)
                    {
                        // re-perform the action that brought up the keyvboard
                        rgnv = 5;
                    }
                     */
                }
                else if(key == 'D')
                {
                    // delete
                    if(l > 0) *(p-1) = '\0';
                }
                else
                {
                    *p = key;
                    *(p+1) = '\0';
                }
            }
        }

        switch(rgnv)
        {
            case 0:
                break;
            case 1:
                gameAudioPlaySoundAtLocation("menubeep", gameCamera_getX(), gameCamera_getY(), gameCamera_getZ());
                action_prev();
                break;
            case 2:
                gameAudioPlaySoundAtLocation("menubeep", gameCamera_getX(), gameCamera_getY(), gameCamera_getZ());
                action_sub_next();
                break;
            case 3:
                gameAudioPlaySoundAtLocation("menubeep", gameCamera_getX(), gameCamera_getY(), gameCamera_getZ());
                action_sub_prev();
                break;
            case 4:
                gameAudioPlaySoundAtLocation("menubeep", gameCamera_getX(), gameCamera_getY(), gameCamera_getZ());
                action_next();
                break;
            case 5:
                gameAudioPlaySoundAtLocation("menubeep", gameCamera_getX(), gameCamera_getY(), gameCamera_getZ());
                gameInterfaceProcessAction();
                break;
                
            default:
                break;
        }
    }
    
    if(touchedControl == &gameInterfaceControls.dialogRect)
    {
        if(touchedControl->touch_ry <= 0.5)
        {
            touchedControl->visible = 0;
            gameInterfaceControls.consoleHidden = 0;
            if(gameInterfaceControls.dialogRect.d.dialogRectActionLeft) gameInterfaceControls.dialogRect.d.dialogRectActionLeft();
        }
        else
        {
            touchedControl->visible = 0;
            gameInterfaceControls.consoleHidden = 0;
            if(gameInterfaceControls.dialogRect.d.dialogRectActionRight) gameInterfaceControls.dialogRect.d.dialogRectActionRight();
        }
        
        if(glFlightDrawframeHook == NULL) glFlightDrawframeHook = gameDialogMuteCountdown;
    }
    
    if(touchedControl == &gameInterfaceControls.fireRectBoost)
    {
        speedBoost = SPEED_BOOST_FRAMES;
        gameInterfaceControls.fireRectBoost.visible = 0;
        gameAudioPlaySoundAtLocation("speedboost", my_ship_x, my_ship_y, my_ship_z);
    }
    
    if(touchedControl == &gameInterfaceControls.fire)
    {
        gameInterfaceHandleTouchMove(x, y);
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
}

static void
gameInterfaceBrowseFoundGame(char* name)
{
    if(gameNetworkState.hostInfo.bonjour_lan)
    {
        glFlightDrawframeHook = NULL;
        gameNetworkState.gameNetworkHookGameDiscovered = NULL;
        gameNetwork_connect(name, load_map_and_host_game);
    }
    else
    {
        if(glFlightDrawframeHook)
        {
            // add a portal which hosts a new game anyway
            console_write("To host new game enter *HOST* portal\n");
            game_add_network_portal(GAME_NETWORK_HOST_PORTAL_NAME);
            // cancel countdown
            glFlightDrawframeHook = NULL;
        }
        
        game_add_network_portal(name);
    }
}

static int
gameInterfaceMultiplayerConfigure(int action)
{
    switch(action)
    {
        case ACTION_CONNECT_TO_GAME:
        case ACTION_HOST_GAME_LAN:
        {
            gameNetwork_disconnectSignal();
            
            gameMapSetMap(map_portal_lobby);
            
            glFlightDrawframeHook = gameDialogNetworkCountdownEnded;
            gameNetworkState.gameNetworkHookGameDiscovered = gameInterfaceBrowseFoundGame;
            
            if(action == ACTION_HOST_GAME_LAN)
            {
                gameNetworkState.hostInfo.bonjour_lan = 1;
                GameNetworkBonjourManagerBrowseBegin();
                
                return 1;
            }
            else
            {
                // browse directory
                if(gameNetwork_connect(gameSettingGameTitle, load_map_and_host_game) != GAME_NETWORK_ERR_NONE)
                {
                    console_write("failed to connect to server");
                    glFlightDrawframeHook = NULL;
                    gameNetworkState.gameNetworkHookGameDiscovered = NULL;
                    return 0;
                }
                return 1;
            }
        }
        break;
        
        case ACTION_HOST_GAME_OR_COMMIT:
        {
            gameNetwork_host(gameSettingGameTitle, load_map_and_host_game);
            // find an available port and register
            glFlightDrawframeHook = gameDialogNetworkHostRegister;
        }
        break;
    }
    
    /*
    switch(gameDialogState.networkGameNameEntered)
    {
            // pop dialog
        case 0:
            if(action == ACTION_CONNECT_TO_GAME)
            {
                gameNetwork_disconnectSignal();
                gameMapSetMap(map_portal_lobby);
                glFlightDrawframeHook = gameDialogBrowseGamesCountdown;
                extern void game_add_network_portal(char* name);
                gameNetworkState.gameNetworkHookGameDiscovered = game_add_network_portal;
                gameNetwork_connect("SEARCHING", gameDialogBrowseGamesCountdown);
                return 1;
            }
            
            gameDialogState.networkGameNameEntered++;
            gameDialogEnterGameName(action);
            return 0;
            
            // dialog closed - pop keyboard
        case 1:
            gameNetwork_disconnectSignal();
            gameDialogState.networkGameNameEntered++;
            
            fireActionQueuedAfterEdit = action;
            fireAction = ACTION_SETTING_GAME_NAME;
            gameInterfaceProcessAction();
            return 0;
            
            // ready to connect - clean up
        case 2:
            if(action == ACTION_HOST_GAME_LAN)
            {
                gameDialogClose();
                gameNetworkState.hostInfo.bonjour_lan = 1;
                GameNetworkBonjourManagerBrowseBegin();
                glFlightDrawframeHook = gameDialogBrowseGamesCountdown;
            }
            else if(action == ACTION_HOST_GAME)
            {
                gameNetwork_connect("", load_map_and_host_game);
                gameNetwork_directoryRegister(gameSettingGameTitle);
                glFlightDrawframeHook = gameDialogRegisterGameCountdown;
            }
            else
            {
                gameDialogClose();
                glFlightDrawframeHook = gameDialogBrowseGamesCountdown;
                gameNetwork_connect(gameSettingGameTitle, gameDialogBrowseGamesCountdown);
            }
            
            gameDialogState.networkGameNameEntered = 0;
            
            return 1;
            
        default:
            return 0;
    }
     */
    
    return 0;
}

void gameInterfaceProcessAction()
{
    char *mapBuffer = NULL;
    char strTmp[256];
    
    if(fireAction > ACTION_HOST_GAME_LAN && fireAction <= ACTION_CONNECT_TO_GAME)
    {
        gameNetworkState.hostInfo.bonjour_lan = 0;
    }

    switch(fireAction)
    {
        case ACTION_MAP_EDIT:
            gameNetwork_disconnectSignal();
            game_map_custom_loaded = 1;
            gameInterfaceControls.mainMenu.visible = 0;
            gameMapFileName("custom");
            mapBuffer = gameMapRead();
            gameMapSetMap(mapBuffer);
            //save_map = 1; // save map when done
            gameStateSinglePlayer.map_use_current = 1;
            console_write("custom map loaded\nchanges will be saved");
            break;
            
        case ACTION_MAP_SAVE:
            gameInterfaceControls.mainMenu.visible = 0;
            gameMapFileName("custom");
            gameMapWrite();
            gameStateSinglePlayer.map_use_current = 1;
            console_write("custom map saved\n");
            break;
            
        case ACTION_CLEAR_MAP:
            gameMapFileName("custom");
            maps_list_idx++;
            if(maps_list[maps_list_idx] == NULL) maps_list_idx = 0;
            gameMapSetMap(maps_list[maps_list_idx]);
            gameMapWrite();
            gameStateSinglePlayer.map_use_current = 1;
            console_write("map loaded:%s", maps_list_names[maps_list_idx]);
            break;
        
        case ACTION_NETWORK_MULTIPLAYER_MENU:
            actions_menu_set(ACTION_HOST_GAME_LAN);
            break;
        
        case ACTION_HOST_GAME_LAN:
        case ACTION_CONNECT_TO_GAME:
        case ACTION_HOST_GAME_OR_COMMIT:
        {
            if(gameInterfaceMultiplayerConfigure(fireAction))
            {
                gameInterfaceControls.mainMenu.visible = gameInterfaceControls.textMenuControl.visible = 0;
            }
            break;
        }
            
        case ACTION_NETWORK_DISCONNECT:
            gameNetwork_disconnectSignal();
            gameMapSetMap(initial_map);
            break;
            
        case ACTION_DISPLAY_SCORES:
            if(gameNetworkState.connected)
            {
                gameDialogState.hideNetworkStatus = 0;
                gameDialogNetworkGameStatus();
            }
            else
            {
                gameDialogScores();
            }
            break;
            
        case ACTION_HELP:
#if !GAME_PLATFORM_ANDROID
            gameDialogVisitHomepage();
#endif
            gameDialogGraphic(TEXTURE_ID_CONTROLS_HELP);
            break;
            
        case ACTION_OPEN_RATING_URL:
            gameDialogRating();
            break;
            
        case ACTION_RESUME_GAME:
            game_start_difficulty = gameStateSinglePlayer.last_game.difficulty;
            game_start_score = gameStateSinglePlayer.last_game.score;
            
        case ACTION_START_GAME:
            gameNetwork_disconnectSignal();
            actions_menu_reset();
            gameInterfaceControls.mainMenu.visible = 0;
            game_start(game_start_difficulty, GAME_TYPE_COLLECT);
            gameStateSinglePlayer.stats.score = game_start_score;
            gameInterfaceControls.mainMenu.visible = gameInterfaceControls.textMenuControl.visible = 0;
            save_map = 0;
            break;
            
        case ACTION_NETWORK_GAME_SCORE:
            gameDialogState.hideNetworkStatus = 0;
            gameDialogNetworkGameStatus();
            break;
            
        case ACTION_START_SURVIVAL_GAME:
            gameNetwork_disconnectSignal();
            gameInterfaceControls.mainMenu.visible = 0;
            sprintf(strTmp, "survival started!\n");
            gameStateSinglePlayer.started = 1;
            game_start(1, GAME_TYPE_SURVIVAL);
            gameInterfaceControls.mainMenu.visible = gameInterfaceControls.textMenuControl.visible = 0;
            save_map = 0;
            break;
            
        case ACTION_START_SPEEDRUN_GAME:
            gameNetwork_disconnectSignal();
            gameInterfaceControls.mainMenu.visible = 0;
            sprintf(strTmp, "survival started!\n");
            gameStateSinglePlayer.started = 1;
            game_start(1, GAME_TYPE_SPEEDRUN);
            gameInterfaceControls.mainMenu.visible = gameInterfaceControls.textMenuControl.visible = 0;
            save_map = 0;
            break;
            
        case ACTION_START_LOBBALL_GAME:
            gameNetwork_disconnectSignal();
            gameInterfaceControls.mainMenu.visible = 0;
            sprintf(strTmp, "lob started!\n");
            gameStateSinglePlayer.started = 1;
            game_start(1, GAME_TYPE_LOBBALL);
            gameInterfaceControls.mainMenu.visible = gameInterfaceControls.textMenuControl.visible = 0;
            save_map = 0;
            break;
            
        case ACTION_START_DEFEND_GAME:
            gameNetwork_disconnectSignal();
            gameInterfaceControls.mainMenu.visible = 0;
            sprintf(strTmp, "defend started!\n");
            gameStateSinglePlayer.started = 1;
            game_start(1, GAME_TYPE_DEFEND);
            gameInterfaceControls.mainMenu.visible = gameInterfaceControls.textMenuControl.visible = 0;
            save_map = 0;
            break;
            
        case ACTION_START_TURRET_GAME:
            gameNetwork_disconnectSignal();
            gameInterfaceControls.mainMenu.visible = 0;
            sprintf(strTmp, "turret started!\n");
            gameStateSinglePlayer.started = 1;
            game_start(1, GAME_TYPE_TURRET);
            gameInterfaceControls.mainMenu.visible = gameInterfaceControls.textMenuControl.visible = 0;
            save_map = 0;
            break;
            
        case ACTION_SETTING_PLAYER_NAME:
            gameInterfaceEditString(gameSettingsPlayerName);
            break;
            
        case ACTION_SETTING_GAME_NAME:
        fire_action_setting_game_name:
            gameInterfaceEditString(gameSettingGameTitle);
            break;
        case ACTION_SETTING_UPDATE_FREQUENCY:
            gameInterfaceEditInt(&gameSettingsNetworkFrequency);
            break;
        case ACTION_SETTING_PORT_NUMBER:
            gameInterfaceEditInt(&gameSettingsPortNumber);
            break;
        case ACTION_SETTING_LOCAL_IP_OVERRIDE:
            gameInterfaceEditString(gameSettingsLocalIPOverride);
            break;
            
        case ACTION_SETTING_SHIP_MODEL:
            {
                int model_new = MODEL_SHIP1;
                
                switch(model_my_ship)
                {
                    case MODEL_SHIP1:
                        model_new = MODEL_SHIP2;
                        texture_id_playership = TEXTURE_ID_ENEMYSHIP;
                        console_write("respawn: virus-fighter");
                        break;
                    case MODEL_SHIP2:
                        model_new = MODEL_SHIP3;
                        texture_id_playership = TEXTURE_ID_ENEMYSHIP_ACE;
                        console_write("respawn: heavy-fighter");
                        break;
                    default:
                        model_new = MODEL_SHIP1;
                        texture_id_playership = TEXTURE_ID_SHIP1;
                        console_write("respawn: defense-fighter");
                        break;
                }
                model_my_ship = model_new;
                
                appWriteSettings();
            }
            break;
            
        case ACTION_SETTING_CONTROL_MODE:
            gameSettingsComplexControls = !gameSettingsComplexControls;
            appWriteSettings();
            console_write("%s", gameSettingsComplexControls ? "enabled" : "disabled");
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
            appWriteSettings();
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
            appWriteSettings();
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
            appWriteSettings();
            break;
            
        case ACTION_SETTING_INPUT_SENSITIVITY:
            GYRO_DC += 1;
            if(GYRO_DC >= 10) GYRO_DC = 3;
            console_write("Input sensitivity: %f\n", GYRO_DC);
            appWriteSettings();
            break;
            
        case ACTION_SETTING_BOTINTELLIGENCE:
            gameStateSinglePlayer.setting_bot_intelligence++;
            if(gameStateSinglePlayer.setting_bot_intelligence > 10)
                gameStateSinglePlayer.setting_bot_intelligence = 1;
            console_write("Bot skill:%d\n", gameStateSinglePlayer.setting_bot_intelligence);
            appWriteSettings();
            break;
            
        case ACTION_SETTING_AUDIO:
            gameAudioMuted = !gameAudioMuted;
            console_write("Audio muted:%d\n", gameAudioMuted);
            appWriteSettings();
            break;
            
        case ACTION_SETTING_RESET_SCORE:
            gameDialogResetScores();
            break;
            
        case ACTION_SETTING_RESET_DEFAULT:
            gameSettingsDefaults();
            break;
            
        default:
            break;
    }
}

void
gameInterfaceHandleTouchEnd(float x, float y)
{
    controlRect *touchedControl = gameInterfaceFindControl(x, y);

    if (touchedControl) {
        touchedControl->touch_began = 0;
        //gameInterfaceHandleTouchMove(x, y);
        touchedControl->touch_end_last = time_ms;
    }

    if (touchedControl == &gameInterfaceControls.trim)
    {
        //needTrim = 0;
    }

    // clear controls touched by this touchid
    for (int i = 0; gameInterfaceControls.controlArray[i] != NULL; i++)
    {
        if (gameInterfaceControls.controlArray[i]->touch_id == gameInterfaceControls.touchId)
        {
            gameInterfaceControls.controlArray[i]->touch_began = 0;
            gameInterfaceControls.controlArray[i]->touch_end_last = time_ms;
        }
    }
    
    gameInterfaceControls.touchUnmapped = 0;
    gameInterfaceControls.touchUnmappedX = gameInterfaceControls.touchUnmappedY = -1;
    
    gameInterfaceControls.touchCount--;
    
    gameInterfaceControls.pTouchedLastControl = NULL;
}

void
gameInterfaceHandleAllTouchEnd()
{
    controlRect** controlSet = gameInterfaceControls.controlArray;
    
    for(int i = 0; controlSet[i] != NULL; i++)
    {
        controlSet[i]->touch_began = 0;
    }
    
    gameInterfaceControls.touchUnmapped = 0;
    gameInterfaceControls.touchUnmappedX = gameInterfaceControls.touchUnmappedY = -1;
}

void
gameInterfaceModalDialogEnqueue(const char* msg, const char *buttonLeft, const char *buttonRight, void (*cbLeft)(void), void (*cbRight)(void),
                         controlRect* overrideRect, unsigned long life_frames)
{
    controlRect *pcur;
    controlRect *pdialog = (controlRect*) malloc(sizeof(controlRect));
    if(!pdialog) return;
    
    if(life_frames != GAME_DIALOG_LIFE_MAX)
    {
        if(gameInterfaceModalDialogPeek() != NULL)
        {
            free(pdialog);
            return;
        }
    }
    
    *pdialog = *overrideRect;
    pdialog->pnext = NULL;
    pdialog->dialogLifeFrames = life_frames;
    
    sprintf(pdialog->text,
            "%s",
            msg? msg: "");
    sprintf(pdialog->textLeft,
            "%s",
            buttonLeft? buttonLeft: "");
    sprintf(pdialog->textRight,
            "%s",
            buttonRight? buttonRight: "");
    pdialog->d.dialogRectActionLeft = cbLeft;
    pdialog->d.dialogRectActionRight = cbRight;
    pdialog->visible = 1;
    pdialog->modal = 1;
    
    pcur = &gameInterfaceControls.dialogRect;
    while(pcur->pnext) pcur = pcur->pnext;
    
    pcur->pnext = pdialog;
    
    gameInterfaceModalDialogDequeue();
}

void
gameInterfaceModalDialogDequeue()
{
    if(gameInterfaceControls.dialogRect.visible) return;
    
    // dequeue next dialog
    controlRect* pqueued = gameInterfaceControls.dialogRect.pnext;
    if(pqueued)
    {
        gameInterfaceControls.dialogRect = *pqueued;
        free(pqueued);
    }
}

void
gameInterfaceModalDialog(const char* msg, const char *buttonLeft, const char *buttonRight, void (*cbLeft)(void), void (*cbRight)(void))
{
    gameInterfaceModalDialogEnqueue(msg, buttonLeft, buttonRight, cbLeft, cbRight, &gameInterfaceControls.dialogRectDefault, GAME_DIALOG_LIFE_MAX);
}

controlRect*
gameInterfaceModalDialogPeek()
{
    return gameInterfaceControls.dialogRect.pnext;
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

void
gameInterfaceCalibrateDone()
{
    gameInterfaceControls.textMenuButton.visible = 1;
    gameInterfaceControls.trim.visible = 1;
}
