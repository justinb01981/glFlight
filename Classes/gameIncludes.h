//
//  gameIncludes.h
//  gl_flight
//
//  Created by Justin Brady on 7/5/14.
//
//

#ifndef gl_flight_gameIncludes_h
#define gl_flight_gameIncludes_h

#include "gamePlatform.h"

#if GAME_PLATFORM_ANDROID
#include <GLES/glext.h>
#include <GLES/glplatform.h>
#include "GLES/gl.h"
#include "GLES/glext.h"
#else /* iOS */
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#define BSD_SOCKETS
#endif

#endif
