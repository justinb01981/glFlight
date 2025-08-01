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

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRenderer_onSurfaceCreated(JNIEnv *e, jclass o);
JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRenderer_onSurfaceChanged(JNIEnv *e, jclass o, jfloatArray arr);
JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRenderer_onDrawFrame(JNIEnv *e, jclass o);

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightInit(JNIEnv *e, jclass o);
JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightUninit(JNIEnv *e, jclass o);
JNIEXPORT jint JNICALL Java_com_domain17_glflight_GameRunnable_glFlightResourcesInit(JNIEnv *e, jclass o);
JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightRunBGThread(JNIEnv *e, jclass o);
JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightRunTimerThread(JNIEnv *e, jobject o);

JNIEXPORT jstring JNICALL Java_com_domain17_glflight_GameRunnable_glFlightNextAudioEvent(JNIEnv *e, jclass o, jfloatArray arr);

JNIEXPORT void JNICALL Java_com_domain17_glflight_GameRunnable_glFlightSensorInput(JNIEnv *e, jclass o, jfloatArray arr);

JNIEXPORT void JNICALL Java_com_example_glflight_GameRunnable_glFlightTouchInput(JNIEnv *e, jclass o, jstring str);

extern jstring openUrlRequest;
JNIEXPORT jstring JNICALL Java_com_domain17_glflight_GameRunnable_glFlightOpenURL(JNIEnv *e, jclass o);

#ifdef __cplusplus
}
#endif

#endif /* GLFLIGHT_H_ */
