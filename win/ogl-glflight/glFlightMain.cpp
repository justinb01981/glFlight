//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on Simple_Texture2D.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com


#include "gameFramework.hpp"
#include "gameWinTypes.h"
#include "gameAudioOpenAL.h"

#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <al.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "textures.h"
#include "gameCamera.h"
#include "gameSettings.h"
#include "gameAi.h"
#include "gameInput.h"
#include "world_file.h"
#include "gameGraphics.h"
#include "gameInterface.h"
#include "gameAudio.h"
#include "gameNetwork.h"
#include "maps.h"
#include "gameDebug.h"
#include "gameDialogs.h"

#include "glFlight.h"

#include "gameWinStubs.h"

#ifdef __cplusplus
};
#endif

/*** globals ***/

glFlightPrefs prefs;

class GLFlightGame : public gameFramework
{
private:
    bool glFlightInited = false;

    std::map<int, int> touchIDMap;
    int touchStateTouchCount = 0;

    unsigned long bgThreadId;

    WSAData wsaData;

public:

    GLFlightGame(int argc, char* argv[]) :
        gameFramework(argc, argv, "glFlightMain", gameFramework::CORE, 3, 0, 2, MATCH_TEMPLATE, glm::uvec2(1280, 640))
    {
    }

    void glFlightInitialize()
    {
        gameCamera_init(0, 0, 0, 0, 0, 0);

        game_init();
        gameAudioInit();

        get_time_ms_init();
        rand_seed(time_ms);

        world_lock_init();

        gameSettingsPlatformInit(glFlightDefaultPlayerName(), glFlightDefaultGameName());

        if (!gameSettingsRead(glFlightSettingsPath()))
        {
            gameSettingsDefaults();
        }

        game_ai_init();

        console_init();

        sprintf(glFlightGameResourceInfo.pathPrefix, "resources/");
        initTextures(glFlightGameResourceInfo.pathPrefix);

        gameNetwork_init_mem();
        gameNetwork_init(0, gameSettingsGameNameDefault, gameSettingsPlayerName,
            gameSettingsNetworkFrequency,
            gameSettingsPortNumber, gameSettingsLocalIPOverride);

        gameMapFilePrefix(glFlightGameResourceInfo.pathPrefix);
        gameMapFileName((char*) "temp");
        gameMapSetMap(initial_map);

        // HACK: touch inputs are in portrait mode
        gameInterfaceInit(viewWidth, viewHeight);
        gameInterfaceControls.trim.blinking = 1;
    }

    bool begin()
    {
        // Set the display viewport
        glm::uvec2 WindowSize = this->getWindowSize();
        glViewport(0, 0, WindowSize.x, WindowSize.y);

        viewWidth = WindowSize.x;
        viewHeight = WindowSize.y;

        // set up of framebuffers (and depth buffer) has happened in subclasses
        GLint depthSize = 0;
        glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthSize);

        // set up texture units
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);

        WSAStartup(MAKEWORD(2, 2), &wsaData);

        glFlightInitialize();

        glFlightInited = true;

        openALInit();

        CreateThread(NULL, 1024 * 64, GLFlightGame::backgroundWorker, &glFlightInited, 0, &bgThreadId);

        return true;
    }

    bool end() 
    {

        glFlightInited = false;

        // TODO: poll for thread exit code
        Sleep(1000);

        gameNetwork_disconnect();
        if (save_map)
        {
            world_lock();
            gameMapWrite();
            world_unlock();
        }
        gameSettingsWrite(glFlightSettingsPath());

        // HACK: reinitialize some important globals

        gameInputUninit();
        tex_pass = 0;
        glFlightDrawframeHook = gameDialogInitialCountdown;

        openALUninit();

        DBPRINTF(("Java_com_domain17_glflight_GameRunnable_glFlightUninit called + exiting"));

        return true;
    }

    bool render()
	{
		if (!glFlightInited) return true;

        glfwPollEvents();
        
		glFlightFrameStage1();

        gameInputTranslate();
        gameAudioProcessEvents();

        glFlightFrameStage2();

        return true;
	}

    void gameInputTranslate()
    {
        static int mouseFlagsLast = 0;
        static int keysLast[GLFW_KEY_LAST] = { 0 };
        static float eulerO[3] = { 0, 0, 0 };
        static float sSense = 0.2;
        static float mouseDownRightLoc[2] = { 0, 0 };

        static int KEY_ROLL_ADD = GLFW_KEY_Q;
        static int KEY_ROLL_SUB = GLFW_KEY_E;
        static int KEY_FIRE_LASER = GLFW_KEY_SPACE;
        static int KEY_FIRE_MISSLE = GLFW_KEY_LEFT_SHIFT;
        static int KEY_ACCEL = GLFW_KEY_W;
        static int KEY_DECEL = GLFW_KEY_S;

        gameInterfaceTouchIDSet(1);

        float x = (viewHeight - MouseCurrent.y) * (viewWidth/viewHeight);
        float y = (MouseCurrent.x) * (viewHeight/viewWidth);

        // x/y are swapped
        if ((MouseButtonFlags & gameFramework::MOUSE_BUTTON_LEFT))
        {
            if(!(mouseFlagsLast & gameFramework::MOUSE_BUTTON_LEFT))
            {
                gameInterfaceHandleTouchBegin(x, y);
            }
            gameInterfaceHandleTouchMove(x, y);
        }

        if (!(MouseButtonFlags & gameFramework::MOUSE_BUTTON_LEFT) && (mouseFlagsLast & gameFramework::MOUSE_BUTTON_LEFT))
        {
            gameInterfaceHandleTouchMove(x, y);
            gameInterfaceHandleTouchEnd(x, y);
        }

        if (MouseButtonFlags & gameFramework::MOUSE_BUTTON_RIGHT)
        {
            if (!(mouseFlagsLast & gameFramework::MOUSE_BUTTON_RIGHT))
            {
                mouseDownRightLoc[0] = MouseCurrent.y;
                mouseDownRightLoc[1] = MouseCurrent.x;
            }
            eulerO[1] = -1 * M_PI * (MouseCurrent.y - mouseDownRightLoc[0])/400 * sSense;
            eulerO[2] = -1 * M_PI * (MouseCurrent.x - mouseDownRightLoc[1])/400 * sSense;
        }
        else
        {
            eulerO[1] = eulerO[2] = 0.0;
        }

        if (isKeyPressed(KEY_ROLL_ADD))
        {
            eulerO[0] = -1.0 * sSense;
        } 
        else if (isKeyPressed(KEY_ROLL_SUB))
        {
            eulerO[0] = 1.0 * sSense;
        }
        else
        {
            eulerO[0] = 0.0;
        }

        if (isKeyPressed(KEY_FIRE_LASER) && !keysLast[KEY_FIRE_LASER])
        {
            gameInterfaceHandleTouchBegin(gameInterfaceControls.fire.x, gameInterfaceControls.fire.y);
        }
        if (!isKeyPressed(KEY_FIRE_LASER) && keysLast[KEY_FIRE_LASER])
        {
            gameInterfaceHandleTouchEnd(gameInterfaceControls.fire.x, gameInterfaceControls.fire.y);
        }

        if (isKeyPressed(KEY_FIRE_MISSLE))
        {
            gameInterfaceHandleTouchBegin(gameInterfaceControls.fireRectMissle.x, gameInterfaceControls.fireRectMissle.y);
        }
        if (!isKeyPressed(KEY_FIRE_MISSLE) && keysLast[KEY_FIRE_MISSLE])
        {
            gameInterfaceHandleTouchEnd(gameInterfaceControls.fireRectMissle.x, gameInterfaceControls.fireRectMissle.y);
        }

        const float speedInc = 1.0;

        if (isKeyPressed(KEY_ACCEL) && !keysLast[KEY_ACCEL])
        {
            if(targetSpeed + speedInc <= maxSpeed) targetSpeed += speedInc;
        }
        if (isKeyPressed(KEY_DECEL) && !keysLast[KEY_DECEL])
        {
            if(targetSpeed - speedInc >= minSpeed) targetSpeed -= speedInc;
        }

        gameInputGyro(eulerO[0], eulerO[1], eulerO[2]);

        mouseFlagsLast = MouseButtonFlags;
        keysLast[KEY_FIRE_LASER] = isKeyPressed(KEY_FIRE_LASER);
        keysLast[KEY_FIRE_MISSLE] = isKeyPressed(KEY_FIRE_MISSLE);
        keysLast[KEY_ACCEL] = isKeyPressed(KEY_ACCEL);
        keysLast[KEY_DECEL] = isKeyPressed(KEY_DECEL);

        gameInput();
    }

    void gameAudioProcessEvents()
    {
        char audioStr[255];
        cocoaMessage msgNext;

        audioStr[0] = '\0';

        if (glFlightInited)
        {
            gameAudioLock();
            if (cocoaMessageAudioList.next)
            {
                
                sprintf(audioStr, "%s%s", glFlightGameResourceInfo.pathPrefix, cocoaMessageAudioList.next->str);

                openALPlay(audioStr, cocoaMessageAudioList.next->f[0], cocoaMessageAudioList.next->f[1]);

                cocoaMessageListPop(&cocoaMessageAudioList);
            }

            gameAudioUnlock();
        }
    }

    static unsigned long 
    backgroundWorker(void* arg)
    {
        bool* glFlightInited = (bool*) arg;

        while (glFlightInited)
        {
            do_game_network_read();
        }

        return 0;
    }
};

int main(int argc, char** argv)
{
    GLFlightGame app(argc, argv);
    return app();
}

