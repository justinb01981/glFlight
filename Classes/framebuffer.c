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
#include "gameDebug.h"

void
setupGLFramebufferView()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    
    glOrthof(-FB_TEXTURE_WIDTHHEIGHT/2, FB_TEXTURE_WIDTHHEIGHT/2,
             -FB_TEXTURE_WIDTHHEIGHT/2, FB_TEXTURE_WIDTHHEIGHT/2,
             1, -1);

    glTranslatef(0, 0, 0);
    
    // see framebuffer.h
}

void
setupGLFramebufferViewDone()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
}

int colorBg = 1;
void frameBufUpdate(struct FrameBufInst* s) {

    setupGLFramebufferView();

    DBPRINTF(("frameBufUpdate: renderedTexture = %d\n", s->texture));

    // adding binding/drawing here
    glClearColor(colorBg / 3, colorBg % 2, 0.0, 1.0);
    colorBg += 1;
    glClear(GL_COLOR_BUFFER_BIT);

    float width = s->width;
    float height = s->height;
    
    const int T = 4;
    int idx = 0;

    while (idx < (width-T) * (height-T)  )
    {
        // set origin

        GLfloat o3[] = {-width/2, -height/2, 0};
        o3[0] += idx % (int) width;
        o3[1] += idx / width;

        float width = T;
        float height = T;
        
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

        s->colors[t++] = fmod(rand(), 1000) / 1000.0;
        s->colors[t++] = 0;
        s->colors[t++] = 1.0;
        s->colors[t++] = 1.0;

        s->colors[t++] = 0.0;
        s->colors[t++] = 1.0;
        s->colors[t++] = 0.0;
        s->colors[t++] = 1.0;

        s->colors[t++] = 1.0;
        s->colors[t++] = 0.0;
        s->colors[t++] = 0.0;
        s->colors[t++] = 1.0;

        s->colors[t++] = 0.5;
        s->colors[t++] = 0.5;
        s->colors[t++] = 0.5;
        s->colors[t++] = 1.0;

        s->count = i;

        glEnableClientState(GL_COLOR_ARRAY);

        // see gameGraphics.c - setting up view matrix already done

        // Set the list of draw buffers.
        GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, DrawBuffers);

        glVertexPointer(3, GL_FLOAT, 0, s->coords); // ??
        glColorPointer(4, GL_FLOAT, 0, s->colors);

        glDrawElements(GL_TRIANGLES, (int) s->count, GL_UNSIGNED_INT, s->indices);

        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

        if(idx % (int) width == 0)
        {
            if((idx / (int) width) % 2 == 0)
            {
            // skip odd
                idx += width-1;
            }
        }
        idx += 1;

        glDisableClientState(GL_COLOR_ARRAY);
    }
    
    setupGLFramebufferViewDone();
}

void frameBufCleanup(struct FrameBufInst* s)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &s->name);
    s->name = 0;

    glBindTexture(GL_TEXTURE_2D, 0);

    // TODO: having problems on 2nd draw into texture
//    glDeleteTextures(1, &s->texture);
//    s->texture = 0;

//    textures_hack_framebuffer_cleanup();
}

void frameBufInit(struct FrameBufInst* s)
{
    s->width = s->height = FB_TEXTURE_WIDTHHEIGHT;

    s->name = 0;
    glGenFramebuffers(1, &s->name);

    glBindFramebuffer(GL_FRAMEBUFFER, s->name);
    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);

    // The texture we're going to render to
    glGenTextures(1, &s->texture);
    glBindTexture(GL_TEXTURE_2D, s->texture);

    // Give an empty image to OpenGL ( the last "0"  )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s->width, s->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    
    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s->texture, 0);


    /*
  0  ____  1
     |  /
     | /
     |/
     */
    textures_hack_framebuffer(s->texture);

//    // TODO: relocate to draw loop
//    frameBufUpdate(s);

    // TODO: how to release texture binding to framebuffer? 2nd draw is not visible in new renderedTexture
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // ToDO: can we release framebuffer here now that tex is rendered?
    // release

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
}
