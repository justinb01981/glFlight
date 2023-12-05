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
#if GAME_PLATFORM_OGL 
#define GL_GLES_PROTOTYPES 1
//#include <GLES2/gl2.h>
//#include <GL/glext.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#else
#if GAME_PLATFORM_ANDROID
#define GL_GLEXT_PROTOTYPES
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
//#include "GLES/glext.h"
#else /* iOS */
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#define BSD_SOCKETS
#endif

#endif

#endif