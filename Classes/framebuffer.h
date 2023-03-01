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
#define FRAMEBUFFER_H_INCLUDED 1

#include <assert.h>
#include <math.h>
#define GLES_SILENCE_DEPRECATION
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#define TEXTURE_ID_BOUNDING (104) // TODO:  HEY NUMBNUTS THIS COLLIDES WITH TEXTURES.H

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

struct FrameBufInst {
    GLfloat coords[512];
    GLuint indices[256];
    GLfloat colors[256];
    size_t count;
};

// initialized from eaglView.m
static GLuint gFramebufferId, gColorRenderbufferId;
static GLuint FramebufferName;
static GLuint renderedTexture;

static GLuint frameBufGen(struct FrameBufInst* s)
{
    int width = 128, height = 128;
    int tex_id = 3;
    
    glGenFramebuffers(1, &FramebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    
    // The texture we're going to render to
    glGenTextures(1, &renderedTexture);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, renderedTexture);

    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

    // adding binding/drawing here
    glClearColor(0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);
    
    // Render to our framebuffer
    glViewport(0, 0, width, height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
    
    /*
  0  ____  1
     |  /
     | /
     |/
    2
     */
    
    // draw triangles
    GLfloat o3[] = {-width/2, -height/2, 0};
    int c = 0;
    s->coords[c++] = o3[0];//
    s->coords[c++] = o3[1];
    s->coords[c++] = o3[2];
    s->coords[c++] = o3[0]+width;//
    s->coords[c++] = o3[1];
    s->coords[c++] = o3[2];
    s->coords[c++] = o3[0];//
    s->coords[c++] = o3[1]+height;
    s->coords[c++] = o3[2];
    s->coords[c++] = o3[0]+width;//
    s->coords[c++] = o3[1]+height;
    s->coords[c++] = o3[2];
    
    int i = 0;
    s->indices[i++] = 0;
    s->indices[i++] = 1;
    s->indices[i++] = 2;
    s->indices[i++] = 3;
    s->indices[i++] = 2;
    s->indices[i++] = 1;
    
    int t = 0;
    GLfloat m = 1.0;
    /*
    s->colors[t++] = m;
    s->colors[t++] = 0.0;
    s->colors[t++] = 0.0;
    s->colors[t++] = 1.0;
     
    s->colors[t++] = 0.0;
    s->colors[t++] = m;
    s->colors[t++] = 0.0;
    s->colors[t++] = 1.0;
     
    s->colors[t++] = 0.0;
    s->colors[t++] = 0.0;
    s->colors[t++] = m;
    s->colors[t++] = 1.0;
     
    s->colors[t++] = m;
    s->colors[t++] = 0.0;
    s->colors[t++] = 0.0;
    s->colors[t++] = 1.0;
    */
    
    // lets gradiate...
    static float mg = 1.0;
    for(int t = 0; t <= 4; t++) for(int ti = 0; ti < 4; ti++) {
        s->colors[t*4+ti] = sin(t) - cos(t) * mg;
    }
    mg *= -1;
//    static_fb_phase = fmod(static_fb_phase+0.1, M_PI*2);
    s->count = i;
    
    glEnableClientState(GL_COLOR_ARRAY);
    
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glOrthof(-width/2, width/2,
             -height/2, height/2,
             1, -1);
    glTranslatef(0, 0, 0);
    
    glVertexPointer(3, GL_FLOAT, 0, s->coords);

    glColorPointer(4, GL_FLOAT, 0, s->colors);
    glDrawElements(GL_TRIANGLES, s->count, GL_UNSIGNED_INT, s->indices);
    
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    glDisableClientState(GL_COLOR_ARRAY);
    
    // restore original screen framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return renderedTexture;
}

static void frameBufCleanup(struct FrameBufInst* s)
{
    if(gFramebufferId)
    {
        glDeleteFramebuffers(1, gFramebufferId);
        glDeleteTextures(1, renderedTexture);
    }
}

#endif /* framebuffer_h */
