//
//  framebuffer.c
//  gl_flight
//
//  Created by Justin Brady on 5/16/23.
//

#include <stdio.h>
#include <string.h>

#include "gameIncludes.h"
#include "framebuffer.h"
#include "textures.h"

int gTextureFramebuffer = -1;

GLuint FramebufferName = GL_NONE;
GLuint renderedTexture = GL_NONE;
GLuint FramebufferNameInitial = GL_NONE;

void frameBufUpdate(struct FrameBufInst* s) {

    goto abort_this;
    
    setupGLFramebufferView();

    glBindTexture(GL_TEXTURE_2D, renderedTexture);



    // Render to our framebuffer
    glViewport(0, 0, s->width, s->height); // Render on the whole framebuffer, complete from the lower left corner to the upper right

    // adding binding/drawing here
    glClearColor(0.0, 0.4, 0.0, 1.0);
    //glClear(GL_COLOR_BUFFER_BIT);

    float width = s->width;
    float height = s->height;

    // draw triangles
    GLfloat o3[] = {-width/2, -height/2, -1};

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

    // lets gradiate...
//    static float mg = 1.0;
//    for(int t = 0; t <= 4; t++) for(int ti = 0; ti < 4; ti++) {
//        s->colors[t*4+ti] = ti*0.5;
//    }
//    mg *= -1;

    s->count = i;

    glEnableClientState(GL_COLOR_ARRAY);

    // see gameGraphics.c - setting up view matrix already done

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);

    glVertexPointer(3, GL_FLOAT, 0, s->coords);
    glColorPointer(4, GL_FLOAT, 0, s->colors);

    glDrawElements(GL_TRIANGLES, (int) s->count, GL_UNSIGNED_INT, s->indices);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    abort_this:
    glDisableClientState(GL_COLOR_ARRAY);
    
    setupGLFramebufferViewDone();
}

void frameBufCleanup(struct FrameBufInst* s)
{
    glDeleteFramebuffers(1, &FramebufferName); FramebufferName = GL_NONE;

    glDeleteTextures(1, &renderedTexture); renderedTexture = GL_NONE;

    gTextureFramebuffer = -1;

    memset(s, 0, sizeof(*s));

    textures_hack_framebuffer_cleanup();
}

void frameBufInit(struct FrameBufInst* s)
{
    s->width = s->height = 128;

    glGenFramebuffers(1, &FramebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);

    // The texture we're going to render to
    glGenTextures(1, &renderedTexture);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, renderedTexture);

    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s->width, s->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);


    /*
  0  ____  1
     |  /
     | /
     |/
    2
     */

    // TODO: relocate to draw loop
    frameBufUpdate(s);

    textures_hack_framebuffer(renderedTexture);

    // restore our initial framebuffer outside
}
