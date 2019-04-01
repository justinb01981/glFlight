/*
 * justin1.h
 *
 *  Created on: May 15, 2014
 *      Author: justin
 */

#include <jni.h>

#ifndef GLFLIGHT_H_
#define GLFLIGHT_H_

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRenderer_onSurfaceCreated(JNIEnv *e, jobject o);
JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRenderer_onSurfaceChanged(JNIEnv *e, jobject o, , jfloatArray arr);
JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRenderer_onDrawFrame(JNIEnv *e, jobject o);

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightInit(JNIEnv *e, jobject o);
JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightUninit(JNIEnv *e, jobject o);
JNIEXPORT jint JNICALL Java_com_domain17_glflight_GameRunnable_glFlightResourcesInit(JNIEnv *e, jobject o);
JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightRunBGThread(JNIEnv *e, jobject o);
JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightRunTimerThread(JNIEnv *e, jobject o);

JNIEXPORT jstring JNICALL Java_com_domain17_glflight_GameRunnable_glFlightNextAudioEvent(JNIEnv *e, jobject o, jfloatArray arr);

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightSensorInput(JNIEnv *e, jobject o, jfloatArray arr);

JNIEXPORT jint JNICALL Java_com_domain17_glflight_GameRunnable_glFlightSensorNeedsCalibrate(JNIEnv *e, jobject o);

JNIEXPORT void JNICALL Java_com_example_glflight_GameRunnable_glFlightTouchInput(JNIEnv *e, jobject o, jfloatArray arr);

#ifdef __cplusplus
}
#endif

#endif /* GLFLIGHT_H_ */
