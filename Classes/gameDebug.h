//
//  gameDebug.h
//  gl_flight
//
//  Created by Justin Brady on 9/8/14.
//
//

#ifndef gl_flight_gameDebug_h
#define gl_flight_gameDebug_h

#include <stdio.h>
#include "gamePlatform.h"

#if GAME_PLATFORM_ANDROID
#include <jni.h>
#include <android/log.h>

static inline void
dbprintf_helper(char* fmt, ...)
{
    va_list v;
    va_start(v,fmt);
    __android_log_vprint(ANDROID_LOG_DEBUG, "LOG_TAG", fmt, v);
    va_end(v);
}
#define DBPRINTF(x) dbprintf_helper x

#else
#define DBPRINTF(x) { printf("%s:%d", __FILE__, __LINE__); printf x; }
#endif

#endif
