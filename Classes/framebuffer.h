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
#define GLES_SILENCE_DEPRECATION
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#define TEXTURE_ID_BOUNDING (104)

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
    GLfloat texcoords[256];
    size_t count;
};

// initialized from eaglView.m
static GLuint gFramebufferId, gColorRenderbufferId;
static GLuint FramebufferName;
static GLuint renderedTexture;

static void frameBufPrepareProgram(struct FrameBufInst* s)
{
    //glCreateProgram -- see eaglview.h
}

static GLuint frameBufGen(struct FrameBufInst* s)
{
    int width = 1024, height = 1024;
    float a = width, b = -height;
    int tex_id = 3;
    
    frameBufPrepareProgram(s);
    
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
    GLfloat o3[] = {-484, height/2, 0};
    int c = 0;
    s->coords[c++] = o3[0];//
    s->coords[c++] = o3[1];
    s->coords[c++] = o3[2];
    s->coords[c++] = o3[0]+a;//
    s->coords[c++] = o3[1];
    s->coords[c++] = o3[2];
    s->coords[c++] = o3[0];//
    s->coords[c++] = o3[1]+b;
    s->coords[c++] = o3[2];
    
    int i = 0;
    s->indices[i++] = 0;
    s->indices[i++] = 2;
    s->indices[i++] = 1;
    
    int t = 0;
    s->texcoords[t++] = 0.0;
    s->texcoords[t++] = 0.0;
    s->texcoords[t++] = 1.0;
    s->texcoords[t++] = 1.0;
    s->texcoords[t++] = 0.0;
    s->texcoords[t++] = 1.0;
    s->texcoords[t++] = 1.0;
    s->texcoords[t++] = 0.5;
    s->texcoords[t++] = 1.0;
    s->texcoords[t++] = 1.0;
    s->texcoords[t++] = 1.0;
    s->texcoords[t++] = 0.5;
    s->texcoords[t++] = 1.0;
    s->texcoords[t++] = 1.0;
    s->texcoords[t++] = 1.0;
    s->texcoords[t++] = 0.5;
    s->texcoords[t++] = 1.0;
    s->texcoords[t++] = 1.0;
    s->texcoords[t++] = 1.0;
    
    
    s->count = i;
    
//    glEnableClientState(GL_COLOR_ARRAY);
//    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
//    glOrthof(-width/2, width/2,
//             -height/2, height/2,
//             1, -1);
    
    glVertexPointer(2, GL_FLOAT, 0, s->coords);
    glTexCoordPointer(2, GL_FLOAT, 0, s->texcoords);

    glColorPointer(4, GL_FLOAT, 0, s->texcoords);
    glDrawElements(GL_TRIANGLES, s->count, GL_UNSIGNED_INT, s->indices);
    
    extern void drawLineWithColorAndWidth(float a[3], float b[3], float color[3], float width);
    GLfloat line[3][3] = {
        {-width/2-1, -height/2-1, 0},
        {width/2+1, height/2+1, 0},
        {1.0, 0.0, 0.0}
    };
    drawLineWithColorAndWidth(&line[0], &line[1], &line[2], 8);
    
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
//    glDisableClientState(GL_COLOR_ARRAY);
//    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
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
