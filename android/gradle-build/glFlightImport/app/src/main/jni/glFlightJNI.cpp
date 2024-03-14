#include <jni.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <EGL/egl.h>
#include <map>

#include <android/log.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "gameIncludes.h"
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
    
#include "stubs.h"

void dbtrace(const char* x, int y) { __android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", "%s:%d\n", x, y); }
void dbtracelog(const char* x, int y, const char* s) { __android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", "%s:%d %s\n", x, y, s); }
extern void (*trimDoneCallback)(void);

#include "glFlight.h"

// globals
double sumroll, sumpitch, sumyaw, sumrolloff, sumpitchoff, sumyawoff;
game_timeval_t sensorInputNext = 0;
float trimRoll, trimPitch, trimYaw;
char glFlightDefaultPlayerName_[256];

static int
env_float_copy(JNIEnv* e, jfloatArray arr, float dest[])
{
	jboolean b = false;
	jfloat* jf = e->GetFloatArrayElements(arr, &b);

	int len = e->GetArrayLength(arr);
	int i;
	for(i = 0; i < len; i++) dest[i] = jf[i];

	e->ReleaseFloatArrayElements(arr, jf, false);

	return len;
}

/*** globals ***/
typedef struct
{
	char pathPrefix[1024];
} glFlightGameResourceInfo_t;

glFlightGameResourceInfo_t glFlightGameResourceInfo;

glFlightPrefs prefs;

bool glFlightInited = false;

std::map<int,int> touchIDMap;
int touchStateTouchCount = 0;

static char settingsPath[255];
static char glFlightDefaultGameName_[256];

inline static const char*
glFlightSettingsPath()
{
	sprintf(settingsPath, "%s/settings.txt", glFlightGameResourceInfo.pathPrefix);
	return settingsPath;
}

inline static const char*
glFlightDefaultGameName()
{
	sprintf(glFlightDefaultGameName_, "android%d", rand() % 1024);
	return glFlightDefaultGameName_;
}

inline static const char*
glFlightDefaultPlayerName()
{
	sprintf(glFlightDefaultPlayerName_, "player%d", rand() % 1024);
	return glFlightDefaultPlayerName_;
}

inline static void
glFlightJNIInit()
{
	DBPRINTF(("glFlightJNIInit called\n"));

	gameCamera_init(0, 0, 0, 0, 0, 0);

	game_init();
	gameAudioInit();

    get_time_ms_init();
    rand_seed(time_ms);

	world_lock_init();

    world_lock();

	gameSettingsPlatformInit(glFlightDefaultPlayerName(), glFlightDefaultGameName());

	if(!gameSettingsRead(glFlightSettingsPath()))
	{
		gameSettingsDefaults();
	}

	// TODO: hack!
	gameSettingsPortNumber = 52000;

	game_ai_init();

	console_init();

	initTextures(glFlightGameResourceInfo.pathPrefix);

    gameNetwork_init_mem();
	gameNetwork_init(0, gameSettingsGameNameDefault, gameSettingsPlayerName,
			gameSettingsNetworkFrequency,
			gameSettingsPortNumber, gameSettingsLocalIPOverride);

	gameMapFilePrefix(glFlightGameResourceInfo.pathPrefix);
	gameMapFileName("temp");
	gameMapSetMap(initial_map);

    // HACK: touch inputs are in portrait mode

    gameInterfaceInit(viewWidth, viewHeight);

	DBPRINTF(("glFlightJNIInit done!\n"));

    world_unlock();

    glFlightInited = true;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	/* http://developer.android.com/training/articles/perf-jni.html */
	DBPRINTF(("JNI_OnLoad (%d)\n", 0));

	return JNI_VERSION_1_6;
}

/*** native methods ***/
JNIEXPORT
void JNICALL Java_com_domain17_glflight_GameRenderer_onSurfaceCreated(JNIEnv *e, jobject o)
{
	// surface doesnt have bounds yet
}


JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRenderer_onSurfaceChanged(JNIEnv *e, jobject o, jfloatArray arr)
{
	// surface bounds have changed and we can init

	float f[16];
	int l = env_float_copy(e, arr, f);

	viewWidth = f[0];
	viewHeight = f[1];

    glViewport(0, 0, viewWidth, viewHeight);

}

static game_timeval_t gameInputTimeLast = 0;
static game_timeval_t gameDrawTimeLast = 0;

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRenderer_onDrawFrame(JNIEnv *e, jobject o)
{
    // TODO: draw a 'wait...loading' status string each frame until load
	if(!glFlightInited) {
		glFlightJNIInit();
		return;
	}

    assert(eglGetCurrentContext() != NULL);

    // NOTE: 10-25 - removing timing for input events since drawframe is being paced at 60fps // ??
    gameInput();

	glFlightFrameStage1();
	glFlightFrameStage2();

    //DBPRINTF(("drawFrame time delta:%f", time_ms_wall - gameDrawTimeLast));

    gameDrawTimeLast = time_ms_wall;
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightInit(JNIEnv *e, jobject o)
{
//
}

JNIEXPORT jint JNICALL Java_com_domain17_glflight_GameRunnable_glFlightResourcesInit(JNIEnv *e, jobject o)
{
	return 0;
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightPause(JNIEnv *e, jobject o)
{
    gameNetwork_disconnect();
    if(save_map)
    {
        world_lock();
        gameMapWrite();
        world_unlock();
    }
	gameSettingsWrite(glFlightSettingsPath());
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightUninit(JNIEnv *e, jobject o)
{
    glFlightInited = false;

    gameNetwork_disconnect();
    if(save_map)
    {
        world_lock();
        gameMapWrite();
        world_unlock();
    }
	gameSettingsWrite(glFlightSettingsPath());

    // HACK: reinitialize some important globals

    gameInputUninit();
    tex_pass = 0;
    glFlightDrawframeHook = gameDialogInitialCountdownDrawCallback;

    DBPRINTF(("Java_com_domain17_glflight_GameRunnable_glFlightUninit called + exiting"));
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightRunBGThread(JNIEnv *e, jobject o)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	if(glFlightInited)
	{
		do_game_network_read();
	}
}

JNIEXPORT jstring JNICALL Java_com_domain17_glflight_GameRunnable_glFlightNextAudioEvent(JNIEnv *e, jobject o, jchar str)
{
	char audioStr[255];
	cocoaMessage msgNext;

	audioStr[0] = '\0';

	if(glFlightInited)
	{
		gameAudioLock();
		if(cocoaMessageAudioList.next)
		{
			sprintf(audioStr, "%s:%f:%f", cocoaMessageAudioList.next->str,
			        cocoaMessageAudioList.next->f[0],
			        cocoaMessageAudioList.next->f[1]);
			cocoaMessageListPop(&cocoaMessageAudioList);
		}

		gameAudioUnlock();
	}

	jstring s = e->NewStringUTF(audioStr);
    return s;
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightSensorInput(JNIEnv *e, jobject o, jfloatArray arr)
{
	float jf[16];

	int l = env_float_copy(e, arr, jf);

	// see FullScreenActivity.java

    sumroll = fmod(sumrolloff - jf[0], M_PI);
    sumpitch = fmod(sumpitchoff - jf[2], M_PI);
    sumyaw = fmod(sumyawoff - jf[1], M_PI);

    //DBPRINTF(("sensor: %03f %03f %03f", jf[0], jf[1], jf[2]));

	// Z reversed
	gameInputGyro(-sumroll, sumpitch, -sumyaw);
}

JNIEXPORT jboolean JNICALL Java_com_domain17_glflight_GameRunnable_glFlightSensorNeedsCalibrate(JNIEnv *e, jobject o)
{
    return needTrim;
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightTouchInput(JNIEnv *e, jobject o, jfloatArray arr)
{
	float f[16];
	int l = env_float_copy(e, arr, f);

    // HACK: to match iOS, treat touch coordinates as portrait-mode
	float x = f[1];
	float y = f[0];
	float action = f[2];
	float pointerID = f[3];

	//assert(pointerID > 0);

	x = viewHeight - x; // invert

	x = x * (viewWidth/viewHeight); // scale for aspect

	y = y * (viewHeight/viewWidth); // scale for aspect

    DBPRINTF(("action=%f", action));
	DBPRINTF(("x=%f", x));
	DBPRINTF((" y=%f\n", y));
	DBPRINTF((" touchMap: %d", touchIDMap.size()));

	if(action == 0)
	{
        int touchID = touchIDMap.size()+1;
        touchIDMap[pointerID] = touchID;
    	gameInterfaceTouchIDSet(touchID);

		gameInterfaceHandleTouchBegin(x, y);
	}
	else if(action == 1 || action == 3)
	{
	    gameInterfaceTouchIDSet(touchIDMap[pointerID]);
        touchIDMap.erase(pointerID);
		gameInterfaceHandleTouchEnd(x, y);
	}
	else if(action == 2)
	{
	    gameInterfaceTouchIDSet(touchIDMap[pointerID]);
	    gameInterfaceHandleTouchMove(x, y);
	}
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameResources_glFlightGameResourceInit(JNIEnv *e, jobject o, jstring pathPrefix)
{
	jboolean b = false;
	const char* p = e->GetStringUTFChars(pathPrefix, &b);

	memset(&glFlightGameResourceInfo, 0, sizeof(glFlightGameResourceInfo));

	strcpy(glFlightGameResourceInfo.pathPrefix, p);

	DBPRINTF(("resourcePrefix: %s\n", glFlightGameResourceInfo.pathPrefix));

	e->ReleaseStringUTFChars(pathPrefix, p);
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameResources_glFlightGameResourceRead(JNIEnv *e, jobject o, jstring name)
{
	jboolean b = false;
	const char* filename = e->GetStringUTFChars(name, &b);

	FILE* fp = fopen(filename, "r");
	DBPRINTF(("reading file: %s\n", filename));
	if(fp)
	{
		char buf[64];
		while(1)
		{
			int l = fread(buf, sizeof(buf)-1, sizeof(char), fp);
			if(l > 0)
			{
				buf[l] = '\0';
				DBPRINTF(("read bytes: %d\n", strlen(buf)));
			}
			else
				break;
		}
		fclose(fp);
	}
	else
	{
		DBPRINTF(("failed to open %s\n", filename));
	}

	e->ReleaseStringUTFChars(name, filename);
}

int main(int argc, char** argv)
{
	DBPRINTF(("main %d\n", 0));
	return 0;
}
    
void appWriteSettings()
{
    gameSettingsWrite(glFlightSettingsPath());
}

#ifdef __cplusplus
}
#endif


