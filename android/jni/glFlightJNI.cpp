#include <jni.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <android/log.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "game/gameIncludes.h"

#include "game/textures.h"
#include "game/gameCamera.h"
#include "game/gameSettings.h"
#include "game/gameAi.h"
#include "game/gameInput.h"
#include "game/world_file.h"
#include "game/gameGraphics.h"
#include "game/gameInterface.h"
#include "game/gameAudio.h"
#include "game/gameNetwork.h"
#include "game/maps.h"
#include "game/gameDebug.h"
#include "game/gameDialogs.h"
    
#include "stubs.h"

void dbtrace(const char* x, int y) { __android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", "%s:%d\n", x, y); }
void dbtracelog(const char* x, int y, const char* s) { __android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", "%s:%d %s\n", x, y, s); }

extern void update_time_ms_frame_tick();

#include "game/glFlight.h"

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
int glFlightRenderIgnore = 0;

static char settingsPath[255];

inline static const char*
glFlightSettingsPath()
{
	sprintf(settingsPath, "%s/settings.txt", glFlightGameResourceInfo.pathPrefix);
	return settingsPath;
}

inline static const char*
glFlightDefaultGameName()
{
	static char glFlightDefaultGameName_[256];

	sprintf(glFlightDefaultGameName_, "android%d", rand() % 1024);
	return glFlightDefaultGameName_;
}

inline static const char*
glFlightDefaultPlayerName()
{
	static char glFlightDefaultPlayerName_[256];

	sprintf(glFlightDefaultPlayerName_, "player%d", rand() % 1024);
	return glFlightDefaultPlayerName_;
}

inline static void
glFlightJNIInit()
{
	DBPRINTF(("glFlightJNIInit (%d)\n", 0));

	gameCamera_init(0, 0, 0, 0, 0, 0);

	game_init();
	gameAudioInit();

    get_time_ms_init();
    rand_seed(time_ms);

	world_lock_init();

	gameSettingsPlatformInit(glFlightDefaultPlayerName(), glFlightDefaultGameName());
	if(!gameSettingsRead(glFlightSettingsPath()))
	{
		gameSettingsDefaults();
	}

	game_ai_init();

	gameInputInit();

	console_init();

	initTextures(glFlightGameResourceInfo.pathPrefix);

    gameNetwork_init_mem();
	gameNetwork_init(0, gameSettingsGameNameDefault, gameSettingsPlayerName,
			gameSettingsNetworkFrequency,
			gameSettingsPortNumber, gameSettingsLocalIPOverride);

	gameMapFilePrefix(glFlightGameResourceInfo.pathPrefix);
	gameMapFileName("temp");
	gameMapSetMap(maps_list[0]);

    // HACK: touch inputs are in portrait mode
	gameInterfaceInit(viewWidth, viewHeight);
    gameInterfaceControls.trim.blinking = 1;
    
    gameDialogCalibrate();

    //gameDialogLoading();
}

static float xscale = 1;
static float yscale = 1;

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
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRenderer_onSurfaceChanged(JNIEnv *e, jobject o, jfloatArray arr)
{
	float f[16];
	int l = env_float_copy(e, arr, f);

	viewWidth = f[0]*xscale;
	viewHeight = f[1]*yscale;

	DBPRINTF(("Java_com_example_glflight_GameRenderer_onSurfaceChanged: "
			"viewWidth:%f",
			viewWidth));
	DBPRINTF(("Java_com_example_glflight_GameRenderer_onSurfaceChanged: "
			"viewHeight:%f",
			viewHeight));
}

static game_timeval_t gameInputTimeLast = 0;
static game_timeval_t gameDrawTimeLast = 0;

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRenderer_onDrawFrame(JNIEnv *e, jobject o)
{
    // TODO: draw a 'wait...loading' status string each frame until load
    // complete
	if(!glFlightInited && viewWidth > 0 && viewHeight > 0)
	{
		DBPRINTF(("Java_com_example_glflight_GameRenderer_onDrawFrame calling glFlightJNIInit()"));

		if(glFlightRenderIgnore > 0) { glFlightRenderIgnore--; return; }

		glFlightJNIInit();

		glFlightInited = true;
		return;
	}

	glFlightFrameStage1();
	glFlightFrameStage2();

    if(time_ms_wall - gameInputTimeLast >= (1000/GYRO_SAMPLE_RATE))
	{
		gameInput();

		gameInputTimeLast = time_ms_wall;
		//DBPRINTF(("gameInput"));
	}

    //DBPRINTF(("drawFrame time delta:%f", time_ms_wall - gameDrawTimeLast));

    gameDrawTimeLast = time_ms_wall;
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightInit(JNIEnv *e, jobject o)
{
    glFlightInited = false;
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightPause(JNIEnv *e, jobject o)
{
    glFlightRenderIgnore = 100;

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
    DBPRINTF(("Java_com_domain17_glflight_GameRunnable_glFlightUninit called"));

    glFlightInited = false;

    gameNetwork_disconnect();
    if(save_map)
    {
        world_lock();
        gameMapWrite();
        world_unlock();
    }
	gameSettingsWrite(glFlightSettingsPath());
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightRunTimerThread(JNIEnv *e, jobject o)
{
	//update_time_ms_frame_tick();
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
			sprintf(audioStr, "%s:%f", cocoaMessageAudioList.next->str, cocoaMessageAudioList.next->f[0]);
			cocoaMessageListPop(&cocoaMessageAudioList);
			dbtrace((char*)"sound\n",0);
		}
		gameAudioUnlock();
	}

	/* TODO: leak here? */
	jstring s = e->NewStringUTF(audioStr);
    return s;
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightSensorInput(JNIEnv *e, jobject o, jfloatArray arr)
{
	float jf[16];
	static game_timeval_t sensorInputLast = 0;
	static float trimRoll, trimPitch, trimYaw;

	int l = env_float_copy(e, arr, jf);

	/* based off the inputs in landscape mode on my nexus 7... */
	float roll = -jf[2];
	float pitch = -jf[1];
	float yaw = -jf[0];

	if(gameInputTrimPending())
	{
		trimRoll = roll;
		trimPitch = pitch;
		trimYaw = yaw;
		gameInputGyro(roll, pitch, yaw);
	}

	if(/*get_time_ms() - sensorInputLast > 1000/GAME_TICK_RATE*/1)
	{
		gameInputGyro2(roll, pitch, yaw, 0.08);
		sensorInputLast = get_time_ms();
	}
}

JNIEXPORT jint JNICALL Java_com_domain17_glflight_GameRunnable_glFlightSensorNeedsCalibrate(JNIEnv *e, jobject o)
{
	return 0;
}

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightTouchInput(JNIEnv *e, jobject o, jfloatArray arr)
{
	float f[16];
	int l = env_float_copy(e, arr, f);

    // HACK: to match iOS, treat touch coordinates as portrait-modes
	float x = f[1];
	float y = f[0];
	float action = f[2];

	x = viewHeight - x; // invert

	x = x * (viewWidth/viewHeight); // scale for aspect

	y = y * (viewHeight/viewWidth); // scale for aspect

    DBPRINTF(("action=%f", action));
	DBPRINTF(("x=%f", x));
	DBPRINTF((" y=%f\n", y));

	if(action == 0)
	{
		touchStateTouchCount += 1;
    	gameInterfaceControls.touchId = touchStateTouchCount;

		gameInterfaceHandleTouchBegin(x, y);
	}
	else if(action == 1 || action == 3)
	{
	    gameInterfaceControls.touchId = touchStateTouchCount; // wrong!
		gameInterfaceHandleTouchEnd(x, y);
		touchStateTouchCount -= 1;
	}
	else if(action == 2)
	{
	    gameInterfaceControls.touchId = touchStateTouchCount;
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


