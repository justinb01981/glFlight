//
//  framebuffer.h
//  gl_flight
//
//  Created by Justin Brady on 12/5/22.
//
//  see http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
//

#ifndef framebuffer_h
#define framebuffer_h

#include <assert.h>
#include <math.h>

#include "gameGraphics.h"
#include "textures.h"

#define FB_TEXTURE_WIDTHHEIGHT (512)

struct FrameBufInst {
    GLfloat coords[512];
    GLuint indices[256];
    GLfloat colors[256];
    float width, height;
    size_t count;
    
    GLint name;
    GLint texture;
};

void frameBufUpdate(struct FrameBufInst* s);

void frameBufCleanup(struct FrameBufInst* s);

void frameBufInit(struct FrameBufInst* s);

#endif /* framebuffer_h */
