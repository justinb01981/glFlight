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

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "framebuffer.h"
#include "textures.h"

#define FB_TEXTURE_WIDTHHEIGHT (128)

extern int gTextureFramebuffer;

extern void glDrawBuffers(GLuint, GLenum*);
extern int bindTexture(unsigned int);
extern void glBlitFramebuffer(
     GLint srcX0,
     GLint srcY0,
     GLint srcX1,
     GLint srcY1,
     GLint dstX0,
     GLint dstY0,
     GLint dstX1,
     GLint dstY1,
     GLbitfield mask,
     GLenum filter);
extern void setupGLFramebufferView(void);
extern void setupGLFramebufferViewDone(void);


struct FrameBufInst {
    GLfloat coords[512];
    GLuint indices[256];
    GLfloat colors[256];
    float width, height;
    size_t count;
};

extern GLuint FramebufferName;
extern GLuint renderedTexture;

void frameBufUpdate(struct FrameBufInst* s);

void frameBufCleanup(struct FrameBufInst* s);

void frameBufInit(struct FrameBufInst* s);

#endif /* framebuffer_h */
