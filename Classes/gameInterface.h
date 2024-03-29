//
//  gameInterface.h
//  gl_flight
//
//  Created by jbrady on 5/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef gl_flight_gameInterface_h
#define gl_flight_gameInterface_h

#include "action.h"
#include "gameTimeval.h"

#define CONTROLARRAY_LEN 64

typedef struct
{
    double x, y;
    double xw, yw;
    double xm, ym;
    int tex_id;
    int touch_began;
    int touch_id;
    int visible;
    int ignore_touch;
    int hide_frames;
    int blinking;
    double touch_rx, touch_ry;
    game_timeval_t touch_end_last;
    char text[1024], textLeft[64], textRight[64];
    char *textDest;
    int *textDestInt;
    int modal:1, text_align_topleft:1;
    void *pnext;
    unsigned long dialogLifeFrames;
    
    struct {
        void (*dialogRectActionLeft)(void);
        void (*dialogRectActionRight)(void);
    } d;
} controlRect;

typedef struct {
    controlRect mainMenu;
    controlRect accelerator;
    controlRect trim;
    controlRect fire;
    controlRect look;
    controlRect textMenuButton;
    controlRect accelIndicator;
    controlRect radar;
    controlRect fireRect2;
    controlRect fireRectMisc;
    controlRect reticleRect;
    controlRect fireRectMissle;
    controlRect fireRectBoost;
    
    controlRect menuControl;
    controlRect textMenuControl;
    //controlRect help;
    controlRect graphicDialog;
    controlRect graphicDialogHelp;
    controlRect dialogRect;
    controlRect consoleTextRect;
    controlRect statsTextRect;
    controlRect keyboardEntry;
    controlRect calibrateRect;
    
    controlRect dialogRectDefault;
    
    controlRect altControl;
    
    //int menuAction;
    
    float textWidth;
    float textHeight;
    
    struct {
		int unused;
    } dialogMessages;

    int consoleHidden;
    
    controlRect* controlArray[CONTROLARRAY_LEN];
    
    int touchCount;
    int touchId;

    float interfaceWidth;
    float interfaceHeight;
    
    float touchUnmappedX, touchUnmappedY;
    float touchUnmapped;
    
    controlRect* pTouchedLastControl;

} controls;

extern controls gameInterfaceControls;

extern int texture_id_block;

void
gameInterfaceInit(double screenWidth, double screenHeight);

void

gameInterfaceReset();

static controlRect*
gameInterfaceFindControl(float x, float y);

void
gameInterfaceTouchIDSet(int id);

void
gameInterfaceHandleTouchMove(float x, float y);

void
gameInterfaceHandleTouchBegin(float x, float y);

void
gameInterfaceHandleTouchEnd(float x, float y);

void
gameInterfaceHandleAllTouchEnd();

void
gameInterfaceModalDialog(const char* msg, const char *buttonLeft, const char *buttonRight, void (*cbLeft)(void), void (*cbRight)(void));

void
gameInterfaceModalDialogEnqueue(const char* msg, const char *buttonLeft, const char *buttonRight,
                                void (*cbLeft)(void), void (*cbRight)(void),
                                controlRect* overrideRect,
                                unsigned long life_frames);

void
gameInterfaceModalDialogDequeue();

controlRect*
gameInterfaceModalDialogPeek();

static controlRect**
gameInterfaceGetControlArray()
{
    return gameInterfaceControls.controlArray;
}

typedef enum
{
    INTERFACE_STATE_NONE = 0,
    INTERFACE_STATE_BOOST_AVAIL,
    INTERFACE_STATE_BOOST_AVAIL_NO,
    INTERFACE_STATE_TOWING,
    INTERFACE_STATE_TOWING_NONE,
    INTERFACE_STATE_TRIM_BLINKING,
    INTERFACE_STATE_ONSCREEN_INPUT_CONTROL,
    INTERFACE_STATE_CLOSE_MENU
} InterfaceMiscState;

void
gameInterfaceSetInterfaceState(InterfaceMiscState state);

void
gameInterfaceEditString(char *ptr);

void
gameInterfaceProcessAction(void);

void
gameInterfaceActivateShip(void);

#endif
