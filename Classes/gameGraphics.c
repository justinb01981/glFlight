//
//  gameGraphics.c
//  gl_flight
//
//  Created by Justin Brady on 2/28/13.
//
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "gameIncludes.h"

#include "gameGraphics.h"
#include "gameCamera.h"
#include "models.h"
#include "gameInterface.h"
#include "world.h"
#include "gameGlobals.h"
#include "gameUtils.h"
#include "gameShip.h"
#include "textures.h"
#include "gameUtils.h"
#include "gameNetwork.h"
#include "mesh.h"

#define BACKGROUND_MODEL model_background
#define BACKGROUND_MODEL_TEXCOORDS /*model_cube_texcoords_alt*/ model_background_texcoords
#define BACKGROUND_MODEL_INDICES1 model_background_indices1

extern int isLandscape;

extern int game_target_missle_id;
extern int game_target_objective_id;

float viewWidth;
float viewHeight;
DrawBackgroundData* bgData = NULL;

gameGraphics_drawState2d drawState_2d;
gameGraphics_drawState2d drawControls_ds;

void* gl_vertex_ptr_last = 0;
void* gl_texcoord_ptr_last = 0;

int texture_id_playership;
int texture_id_background = BACKGROUND_TEXTURE;

int background_init_needed = 1;

/*
model_coord_t overlay_coords[12];
model_texcoord_t overlay_texcoords[8];
model_index_t overlay_indices[6];
 */

int radar_mode = 0;

model_index_t* drawElem_indicesBatchBuffer = NULL, *drawElem_indicesBatchBufferCur;
model_coord_t* drawElem_vertexBatchBuffer = NULL, *drawElem_vertexBatchBufferCur;
model_texcoord_t* drawElem_textCoordBatchBuffer = NULL, *drawElem_textCoordBatchBufferCur;
unsigned int drawElem_BatchBuffer_max = 0;
unsigned int drawElem_indicesBatchBuffer_count = 0;
unsigned int drawElem_vertexBatchBuffer_count = 0;
unsigned int drawElem_textCoordBatchBuffer_count = 0;
unsigned int drawElem_batchTotal[3];
int drawElemAnimationIdx = 0;
model_coord_t tex_coord_adjust[128] =
{
    0.0, 0.01,
    0.0, 0.01,
    0.0, 0.01,
    0.0, 0.01,
    
    0.0, 0.01,
    0.0, 0.01,
    0.0, 0.01,
    0.0, 0.01,
    
    0.0, 0.01,
    0.0, 0.01,
    0.0, 0.01,
    0.0, 0.01,
    
    0.0, 0.01,
    0.0, 0.01,
    0.0, 0.01,
    0.0, 0.01
};

typedef struct
{
    float x, y;
    float xw, yw;
    int tex_id;
} drawSubElement;

void radarDrawTextDone()
{
    glMatrixMode(GL_TEXTURE); // hack
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static inline void
setupGLModelView(float scrWidth, float scrHeight)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    float sw = scrWidth;
    float sh = scrHeight;
    /*
     * 0,0 starts in center of view
     * remember: matrix stack is applied in-reverse!
     */
    
    glOrthof(-sw/2, sw/2,
             -sh/2, sh/2,
             1, -1);

    glScalef((sw/sh), 1, 1);
    glScalef((sw/sh), sh/sw, 1);
    glRotatef(90, 0, 0, 1);
    glTranslatef(-sw/2, -sh/2, 0);
    
    //glScalef(sw/sh, 1, 1);
    //glTranslatef(sw, 0, 0);
    //glRotatef(90, 0, 0, 1);
}

static void
setupGLModelViewDone()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static inline void
setupGLTextureView()
{
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glRotatef(180, 0, 1, 0);
    glRotatef(90, 0, 0, 1);
}

void
setupGLTextureViewDone()
{
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
}

static unsigned int drawn_texture_last;

int
bindTexture(unsigned int tex_id)
{
    if(!bindTextureRequest(tex_id))
    {
        glBindTexture(GL_TEXTURE_2D, TEXTURE_ID_UNKNOWN);
        return 0;
    }
    
    if(tex_id != drawn_texture_last && tex_id < n_textures)
    {
        glBindTexture(GL_TEXTURE_2D, texture_list[tex_id]);
        drawn_texture_last = tex_id;
        return 1;
    }
    
    return 0;
}

static gameGraphics_drawState2d drawText_ds;

void drawText(char *str, float x, float y, float scale)
{
    float screenWidth = viewWidth;
    float screenHeight = viewHeight;
    unsigned long len = strlen(str);
    float fscreenwidth = /*8*/ gameInterfaceControls.textWidth * scale;
    float fscreenheight = /*12*/ gameInterfaceControls.textHeight * scale;
    float scalechar = 1.0;
    float fmapwidth = 512; // width of the font bitmap
    float fmapheight = 512; // height of the font bitmap
    float rows = 6;
    float cols = 10;
    float z = -2;
    int maxLines = 20;
    gameGraphics_drawState2d *ds = &drawText_ds;
    
    controlRect r;
    
    setupGLModelView(screenWidth, screenHeight);
    
    float m = /*480 / gameInterfaceControls.interfaceWidth*/ 0.85;
    fscreenwidth *= m;
    fscreenheight *= m;
    
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glScalef(-(fmapwidth/cols) / fmapwidth, -(fmapheight/rows) / (fmapheight), 1);
    glRotatef(90, 0, 0, 1);
    
    // enable alpha-blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float line = 0;
    
    int j = 0;
    for(int i = 0; i < len; i++)
    {
        if(str[i] == '\n')
        {
            line++;
            if(line > maxLines) line = 0;
            
            j = 0;
            if(i >= len) break;
            else continue;
        }
        
        if(str[i] == '\r')
        {
            j = 0;
            if(i >= len) break;
            else continue;
        }
        
        if(str[i] == ' ')
        {
            j++;
            continue;
        }
        
        ds->tex_id = TEXTURE_ID_FONTMAP;
        
        if(str[i] == '^' && i < len-1)
        {
            i++;
            if(str[i] == '2' && i < len-1)
            {
                scalechar *= 2;
                continue;
            }
            else if(str[i] == '1' && i < len-1)
            {
                scalechar /= 2;
                j++;
                continue;
            }
            else
            {
                ds->tex_id = TEXTURE_ID_FONTMAP2;
            }
        }
        
        int c = toupper(str[i]);
        //  above transforms cause this orientation:
        //     x-->
        //  3-------2
        //  |       | y
        //  |       | |
        //  |       | \/
        //  1-------0
        int irow = (c-' ') / 10;
        int icol = (c-' ') % 10;
        float col = icol;
        float row = irow;
        
        // 0
        ds->texcoords[0] = row+1;
        ds->texcoords[1] = col+1;
        // 1
        ds->texcoords[2] = row;
        ds->texcoords[3] = col+1;
        // 2
        ds->texcoords[4] = row+1;
        ds->texcoords[5] = col;
        // 3
        ds->texcoords[6] = row;
        ds->texcoords[7] = col;
        
        r.x = x - (line*fscreenheight) - fscreenheight*(scalechar);
        r.y = y + j*fscreenwidth;
        r.xw = fscreenheight * scalechar;
        r.yw = fscreenwidth * scalechar;
        
        // HACK: to flip instead of glRotatef
        r.y = screenHeight - r.y-r.yw;
        
        ds->coords[0] = r.x;
        ds->coords[1] = r.y;
        ds->coords[2] = z;
        
        ds->coords[3] = r.x+r.xw;
        ds->coords[4] = r.y;
        ds->coords[5] = z;
        
        ds->coords[6] = r.x;
        ds->coords[7] = r.y+r.yw;
        ds->coords[8] = z;
        
        ds->coords[9] = r.x+r.xw;
        ds->coords[10] = r.y+r.yw;
        ds->coords[11] = z;

        // {0,1,2, 1,3,2};
        ds->indices[0] = 0; ds->indices[1] = 1; ds->indices[2] = 2;
        ds->indices[3] = 1; ds->indices[4] = 3; ds->indices[5] = 2;
        
        drawState2dSet(ds);
        drawState2dDraw();
        
        j++;
    }
    
    glDisable(GL_BLEND);
    
    glPopMatrix();
    
    setupGLModelViewDone();
}

static gameGraphics_drawState2d drawRadar_ds;

int drawRadar()
{
    float screenWidth = viewWidth;
    float screenHeight = viewHeight;
    float z = -2;
    gameGraphics_drawState2d *ds = &drawRadar_ds;
    int i;
    int minimal_overlay = 1;
    int objective_marker = 0;
    
    WorldElemListNode* cur = gWorld->elements_moving.next;
    
    // iterate over objects-moving list
    while(cur)
    {
        int visible = 0;
        float dist;
        
        if(cur->elem->stuff.radar_visible)
        {
            visible = 1;
        }
        
        if(cur->elem->stuff.nametag) visible = 1;
        if(cur->elem->elem_id == game_target_objective_id) visible = 1;
        
        dist = distance(cur->elem->physics.ptr->x,
                        cur->elem->physics.ptr->y,
                        cur->elem->physics.ptr->z,
                        my_ship_x, my_ship_y, my_ship_z);
        
        if(dist <= 2) visible = 0;
        
        if(visible)
        {
            setupGLModelView(screenWidth, screenHeight);
            
            // rotate textures
            glMatrixMode(GL_TEXTURE);
            glPushMatrix();
            glLoadIdentity();
            glRotatef(180, 0, 1, 0);
            glRotatef(90, 0, 0, 1);
            
            // enable alpha-blending
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            float yawdot;
            float pitchdot;
            float xvec[3], yvec[3], zvec[3];
            float ev[3];
            float zdot;
            controlRect er;
            float x, y;
            unsigned int tex_id = cur->elem->texture_id;
            float scale = 1;
            char *textLabel = NULL;
            int camera_relative = 1;
            int tex_icon = -1;
            float leadv = dist / bulletVel;
            
            if(cur->elem->elem_id == game_target_missle_id) camera_relative = 1;
            
            float elemPos[3] = {cur->elem->physics.ptr->x, cur->elem->physics.ptr->y, cur->elem->physics.ptr->z};
            
            // calculate lead
            if(cur->elem->physics.ptr->velocity > 0)
            {
                elemPos[0] += cur->elem->physics.ptr->vx*leadv;
                elemPos[1] += cur->elem->physics.ptr->vy*leadv;
                elemPos[2] += cur->elem->physics.ptr->vz*leadv;
            }
            
            if(camera_relative)
            {
                ev[0] = (elemPos[0] - gameCamera_getX()) / dist;
                ev[1] = (elemPos[1] - gameCamera_getY()) / dist;
                ev[2] = (elemPos[2] - gameCamera_getZ()) / dist;
                gameShip_getXVector(xvec);
                gameShip_getYVector(yvec);
                gameShip_getZVector(zvec);
            }
            else
            {
                ev[0] = (elemPos[0] - my_ship_x) / dist;
                ev[1] = (elemPos[1] - my_ship_y) / dist;
                ev[2] = (elemPos[2] - my_ship_z) / dist;
                gameShip_getXVector(xvec);
                gameShip_getYVector(yvec);
                gameShip_getZVector(zvec);
            }
            
            zdot = -dot2(ev, zvec);
            pitchdot = dot2(ev, yvec);
            yawdot = dot2(ev, xvec);
            
            float vw = viewWidth;
            float vh = viewHeight;
            
            if(!isLandscape)
            {
                vw = viewHeight;
                vh = viewWidth;
            }
            
            er = (controlRect) {0, 0, vw, vh};
            
            float ar = vw/vh;
            float xs = /*(vw / 640) * (1/ar)*/ 1/ar;
            float ys = /*vh / 480*/ 1;
            
            /* see our glFrustum */
            float xfr = 1;
            float yfr = xfr * (viewWidth/viewHeight);
            x = 0.5 - ((yawdot/2) * xfr);
            y = 0.5 + ((pitchdot/2) * yfr);
            
            /* JB: values after rotation into iOS orientation has happened */
            er.y = x * vw;
            er.x = y * vh;
            
            er.xw = 8 * xs;
            er.yw = 8 * ys;
            
            // TODO: when aiming at target, display its name
            if(/*cur->elem->object_type == OBJ_PLAYER*/ zdot >= 0)
            {
                // skip drawing when too close
                if(dist <= 20 && zdot > 0.5 &&
                   cur->elem->elem_id != game_target_missle_id &&
                   cur->elem->elem_id != game_target_objective_id)
                    goto radar_draw_end;
                
                if(cur->elem->stuff.nametag)
                {
                    textLabel = cur->elem->stuff.nametag;
                }
            }
            else
            {
                tex_id = TEXTURE_ID_RADAR_BEHIND;
            }
            
            if(cur->elem->stuff.nametag)
            {
                if(strcmp(cur->elem->stuff.nametag, "Firewall") == 0)
                {
                    tex_id = TEXTURE_ID_OBJECTIVE_FIREWALL;
                    scale = 4;
                }
            }
            
            if(cur->elem->elem_id == game_target_objective_id /* && zdot > 0*/ && objective_marker)
            {
                tex_id = TEXTURE_ID_RADAR_OBJECTIVE;
                if(!minimal_overlay) tex_icon = zdot >= 0? cur->elem->texture_id: TEXTURE_ID_RADAR_BEHIND;
                
                scale = 4;
            }
            else if(cur->elem->elem_id == game_target_missle_id && zdot > 0)
            {
                // currently missle-targeted
                tex_id = TEXTURE_ID_LOCKEDON;
                scale = (4.0*50) / dist;
                if(scale > 6) scale = 6;
            }
            else
            {
                // cleanup overlay: skip these
                if(minimal_overlay) goto radar_draw_end;
            }
            
            er.x -= er.xw/2*scale;
            er.y -= er.yw/2*scale;
            er.xw *= scale;
            er.yw *= scale;
            
            ds->coords[0] = er.x;
            ds->coords[1] = er.y;
            ds->coords[2] = z;
            
            ds->coords[3] = er.x+er.xw;
            ds->coords[4] = er.y;
            ds->coords[5] = z;
            
            ds->coords[6] = er.x;
            ds->coords[7] = er.y+er.yw;
            ds->coords[8] = z;
            
            ds->coords[9] = er.x+er.xw;
            ds->coords[10] = er.y+er.yw;
            ds->coords[11] = z;
            
            //model_texcoord_t overlay_texcoords_t[] = {0,1, 1,1, 0,0, 1,0};
            i = 0;
            ds->texcoords[i++] = 0; ds->texcoords[i++] = 1;
            ds->texcoords[i++] = 1; ds->texcoords[i++] = 1;
            ds->texcoords[i++] = 0; ds->texcoords[i++] = 0;
            ds->texcoords[i++] = 1; ds->texcoords[i++] = 0;
            
            //model_index_t overlay_indices_t[] = {0,1,2, 1,3,2};
            i = 0;
            ds->indices[i++] = 0; ds->indices[i++] = 1; ds->indices[i++] = 2;
            ds->indices[i++] = 1; ds->indices[i++] = 3; ds->indices[i++] = 2;
            
            ds->tex_id = tex_id;
            
            drawState2dSet(ds);
            drawState2dDraw();
            
            if(tex_icon >= 0)
            {
                float icon_wo = er.yw*0.25;
                scale = 0.25;
                
                er.x -= er.xw/2*scale;
                er.y -= (er.yw/2*scale)+icon_wo;
                er.xw *= scale;
                er.yw *= scale;
                
                ds->coords[0] = er.x;
                ds->coords[1] = er.y;
                ds->coords[2] = z;
                
                ds->coords[3] = er.x+er.xw;
                ds->coords[4] = er.y;
                ds->coords[5] = z;
                
                ds->coords[6] = er.x;
                ds->coords[7] = er.y+er.yw;
                ds->coords[8] = z;
                
                ds->coords[9] = er.x+er.xw;
                ds->coords[10] = er.y+er.yw;
                ds->coords[11] = z;
                ds->tex_id = tex_icon;
                
                drawState2dSetCoords(ds);
                drawState2dDraw();
                tex_icon = -1;
            }
            
            radar_draw_end:
            
            glDisable(GL_BLEND);
            glPopMatrix();
            setupGLModelViewDone();
            
            if(textLabel && zdot > 0)
            {
                drawText(textLabel, er.x+20, viewHeight - er.y, 1);
            }
        }

        cur = cur->next;
    }
    
    return 0;
}

int drawControlsRadar(drawSubElement subElemRadarList[], int max)
{
    WorldElemListNode* cur = gWorld->elements_moving.next;
    int subElementsN = 0;
    static drawSubElement fadeSubElements[10];
    static int fadeSubElements_n = 0;
    const int fadeSubElements_frames = 60;
    
    // iterate over objects-moving list
    while(cur && subElementsN < max)
    {
        int visible = 0;
        
        if(cur->elem->object_type == OBJ_PLAYER)
        {
            if(cur->elem->physics.ptr->velocity >= RADAR_MIN_VELOCITY)
                visible = 1;
        }
        else if(cur->elem->stuff.radar_visible)
        {
            visible = 1;
        }
        
        if(radar_mode) // render on map x/z (top-down)
        {
            if(visible)
            {
                subElemRadarList[subElementsN].x = 0.5 + cur->elem->physics.ptr->x / gWorld->bound_radius/2;
                subElemRadarList[subElementsN].y = 0.5 + cur->elem->physics.ptr->z / gWorld->bound_radius/2;
                subElemRadarList[subElementsN].xw = 1.0/20;
                subElemRadarList[subElementsN].yw = 1.0/20;
                subElemRadarList[subElementsN].tex_id = cur->elem->texture_id;
                
                subElementsN++;
            }
        }
        else // render on player x/y plane
        {
            int tex_id_infront = cur->elem->texture_id;
            
            if(visible)
            {
                float yawdot;
                float pitchdot;
                float xvec[3], yvec[3], zvec[3];
                float elemVec[3];
                float zdot;
                
                float dist = distance(cur->elem->physics.ptr->x,
                                      cur->elem->physics.ptr->y,
                                      cur->elem->physics.ptr->z,
                                      my_ship_x, my_ship_y, my_ship_z);
                elemVec[0] = (cur->elem->physics.ptr->x - my_ship_x) / dist;
                elemVec[1] = (cur->elem->physics.ptr->y - my_ship_y) / dist;
                elemVec[2] = (cur->elem->physics.ptr->z - my_ship_z) / dist;
                gameShip_getXVector(xvec);
                gameShip_getYVector(yvec);
                gameShip_getZVector(zvec);
                
                conv_3d_to_2d(xvec, yvec, elemVec, &yawdot, &pitchdot);
                
                subElemRadarList[subElementsN].x = 0.5 + pitchdot/2; //1-fabs(pitchdot-0.5);
                subElemRadarList[subElementsN].xw = 1.0/20.0;
                subElemRadarList[subElementsN].y = 0.5 - yawdot/2; //fabs(yawdot-0.5);
                subElemRadarList[subElementsN].yw = 1.0/20.0;
                zdot = elemVec[0]*zvec[0]+elemVec[1]*zvec[1]+elemVec[2]*zvec[2];
                subElemRadarList[subElementsN].tex_id = zdot <= 0? tex_id_infront: TEXTURE_ID_RADAR_BEHIND;
                
                subElementsN++;
            }
        }
        cur = cur->next;
    }
    
    if(radar_mode)
    {
        if(tex_pass % fadeSubElements_frames != 0)
        {
            for(int i = 0; i < fadeSubElements_n && subElementsN < max; i++)
            {
                subElemRadarList[subElementsN++] = fadeSubElements[i];
            }
        }
        else
        {
            int i;
            
            for(i = 0; i < sizeof(fadeSubElements)/sizeof(drawSubElement) && i < subElementsN; i++)
            {
                fadeSubElements[i] = subElemRadarList[i];
            }
            
            fadeSubElements_n = i;
        }
    }
    
    return subElementsN;
}

void drawControls()
{
    float screenWidth = viewWidth;
    float screenHeight = viewHeight;
    int tex_id_controls = 1;
    float z = -2;
    static game_timeval_t blink_time_last = 0;
    int ti;
    drawSubElement subElements[64];
    int subElementsN;
    gameGraphics_drawState2d* ds = &drawControls_ds;
    float yphase = 0;
    
    controlRect** controls = gameInterfaceGetControlArray();
    
    int i = 0;
    while(controls[i])
    {
        controlRect r;
        subElementsN = 0;
        
        setupGLModelView(screenWidth, screenHeight);
        
        // rotate textures
        setupGLTextureView();
        /*
         glMatrixMode(GL_TEXTURE);
         glPushMatrix();
         glLoadIdentity();
         glRotatef(180, 0, 1, 0);
         glRotatef(90, 0, 0, 1);
         */
        
        // enable alpha-blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        r = *controls[i];
        tex_id_controls = controls[i]->tex_id;
        
        if(r.hide_frames > 0)
        {
            controls[i]->hide_frames--;
        }
        else if(r.visible)
        {
            game_timeval_t blink_d = time_ms - blink_time_last;
            if(blink_d < 200 && controls[i]->blinking)
            {
                tex_id_controls = 0;
            }
            if(blink_d > 400) blink_time_last = time_ms;
            
            if(controls[i] == &gameInterfaceControls.accelerator)
            {
                //yphase = speed / maxSpeed;
                subElements[subElementsN].xw = 0.1;
                subElements[subElementsN].x = (targetSpeed / maxSpeed) + 0.05;
                subElements[subElementsN].y = 0;
                subElements[subElementsN].yw = 1;
                subElements[subElementsN].tex_id = 21;
                subElementsN++;
            }
            else if(controls[i] == &gameInterfaceControls.trim && tex_id_controls != 0)
            {
                float trimR;
                
                trimR = gameShip_calcRoll();
                
                float m[16] =
                {
                    sin(trimR), -cos(trimR), 1, 0, // landscape y-axis
                    cos(trimR), sin(trimR), 1, 0,  // landscape x-axis
                    0, 0, 1, 0,
                    0, 0, 0, 1
                };
                
                glMatrixMode(GL_TEXTURE);
                
                glPushMatrix();
                glLoadIdentity();
                
                glTranslatef(0.5, 0.5, 0);
                
                //glRotatef(RADIANS_TO_DEGREES(trimR), 0, 0, 1);
                glMultMatrixf(m);
                glTranslatef(-0.5, -0.5, 0);
            }
            else if(controls[i] == &gameInterfaceControls.radar)
            {
                subElementsN = drawControlsRadar(subElements, sizeof(subElements)/sizeof(drawSubElement));
            }
            else if(controls[i] == &gameInterfaceControls.fireRect2)
            {
                switch(fireAction)
                {
                    case ACTION_SHOOT_BLOCK:
                    case ACTION_DROP_BLOCK:
                    case ACTION_BLOCK_TEXTURE:
                        tex_id_controls = texture_id_block;
                        break;
                    default:
                        break;
                }
            }
            else if(controls[i] == &gameInterfaceControls.dialogRect)
            {
                if(gameInterfaceControls.dialogRect.dialogLifeFrames > 0)
                {
                    gameInterfaceControls.dialogRect.dialogLifeFrames--;
                    if(gameInterfaceControls.dialogRect.dialogLifeFrames <= GAME_FRAME_RATE)
                    {
                        gameInterfaceControls.dialogRect.x += (gameInterfaceControls.interfaceWidth / GAME_FRAME_RATE) * gameInterfaceControls.dialogRect.xm;
                        gameInterfaceControls.dialogRect.y += (gameInterfaceControls.interfaceHeight / GAME_FRAME_RATE) * gameInterfaceControls.dialogRect.ym;
                    }
                }
                else
                {
                    gameInterfaceControls.dialogRect.visible = 0;
                    gameInterfaceModalDialogDequeue();
                }
            }
            else if(controls[i] == &gameInterfaceControls.calibrateRect)
            {
                if(bindTexture(TEXTURE_ID_CALIBRATE_CURSOR))
                {
                    extern double motionRollMotion, motionPitchMotion, motionYawMotion, roll_m, pitch_m, yaw_m;
                    extern int gyroStableCount, gyroStableCountThresh;
                    float S = 0.2;
                    float g1 = gyroStableCount, g2 = gyroStableCountThresh;
                    subElements[0].tex_id = TEXTURE_ID_CALIBRATE_CURSOR;
                    subElements[0].x = 0.5 - S/2 + ((motionRollMotion) / M_PI)*(1.0 - ((float) 1.0/g2) * (float) g1);
                    subElements[0].y = 0.5 - S/2 + ((motionPitchMotion) / M_PI)*(1.0 - ((float) 1.0/g2) * (float) g1);
                    subElements[0].xw = S;
                    subElements[0].yw = S;
                    subElementsN++;
                }
            }
            
            // HACK: to flip instead of glRotatef
            r.y = screenHeight - r.y-r.yw;
            
            ti = 0;
            ds->coords[ti++] = r.x;
            ds->coords[ti++] = r.y;
            ds->coords[ti++] = z;
            
            ds->coords[ti++] = r.x+r.xw;
            ds->coords[ti++] = r.y;
            ds->coords[ti++] = z;
            
            ds->coords[ti++] = r.x;
            ds->coords[ti++] = r.y+r.yw;
            ds->coords[ti++] = z;
            
            ds->coords[ti++] = r.x+r.xw;
            ds->coords[ti++] = r.y+r.yw;
            ds->coords[ti++] = z;
            
            //model_texcoord_t overlay_texcoords_t[] = {0,(1-yphase), 1,(1-yphase), 0,0-yphase, 1,0-yphase};
            ti = 0;
            ds->texcoords[ti++] = 0; ds->texcoords[ti++] = 1-yphase;
            ds->texcoords[ti++] = 1; ds->texcoords[ti++] = 1-yphase;
            ds->texcoords[ti++] = 0; ds->texcoords[ti++] = 0-yphase;
            ds->texcoords[ti++] = 1; ds->texcoords[ti++] = 0-yphase;
            
            //model_index_t overlay_indices_t[] = {0,1,2, 1,3,2};
            ti = 0;
            ds->indices[ti++] = 0; ds->indices[ti++] = 1; ds->indices[ti++] = 2;
            ds->indices[ti++] = 1; ds->indices[ti++] = 3; ds->indices[ti++] = 2;
            
            ds->tex_id = tex_id_controls;
            
            if(ds->tex_id >= 0)
            {
                drawState2dSet(ds);
                drawState2dDraw();
            }
            
            // draw sub-elements within this element
            for(int j = 0; j < subElementsN; j++)
            {
                controlRect er = r;
                
                er.y += ((float) er.yw*subElements[j].y);
                er.x += ((float) er.xw*subElements[j].x);
                er.xw = (subElements[j].xw * (float) er.xw);
                er.yw = (subElements[j].yw * (float) er.yw);
                
                ti = 0;
                ds->coords[ti++] = er.x;
                ds->coords[ti++] = er.y;
                ds->coords[ti++] = z;
                
                ds->coords[ti++] = er.x+er.xw;
                ds->coords[ti++] = er.y;
                ds->coords[ti++] = z;
                
                ds->coords[ti++] = er.x;
                ds->coords[ti++] = er.y+er.yw;
                ds->coords[ti++] = z;
                
                ds->coords[ti++] = er.x+er.xw;
                ds->coords[ti++] = er.y+er.yw;
                ds->coords[ti++] = z;
                
                ds->tex_id = subElements[j].tex_id;
                
                drawState2dSet(ds);
                drawState2dDraw();
            }
        }
        
        if(controls[i] == &gameInterfaceControls.textMenuControl &&
           r.visible && r.hide_frames == 0)
        {
            char menuBuf[1024];
            
            console_display_menu(menuBuf);
            
            strcpy(controls[i]->text, menuBuf);
        }
        
        if(controls[i] == &gameInterfaceControls.trim)
        {
            glPopMatrix();
        }
        
        glDisable(GL_BLEND);
        setupGLTextureViewDone();
        setupGLModelViewDone();
        
        if(controls[i]->visible &&
           (controls[i]->text[0] || controls[i] == &gameInterfaceControls.keyboardEntry))
        {
            static char displayText[1024];
            int td = 30;
            float textLineW = 0, textLineN = 0;
            
            strncpy(displayText, controls[i]->text, sizeof(displayText)-1);
            char *pCur = strtok(displayText, "\n");
            
            do
            {
                if(!pCur) pCur = displayText;
                textLineN++;
                char *p = pCur;
                unsigned long l = 0;
                while(*p)
                {
                    if(*p != '^') l++;
                    p++;
                }
                if(l > textLineW) textLineW = l;
                
                pCur = strtok(NULL, "\n");
            } while(pCur);
                
            strncpy(displayText, controls[i]->text, sizeof(displayText)-1);

            if(controls[i] == &gameInterfaceControls.keyboardEntry)
            {
                long dl = strlen(displayText);
                if((tex_pass % td) < (td/2))
                {
                    displayText[dl] = '^';
                    displayText[dl+1] = '@';
                    displayText[dl+2] = '\0';
                }

                float scale = 2.0;
                drawText(displayText,
                         controls[i]->x + controls[i]->xw+32+gameInterfaceControls.textWidth*2,
                         controls[i]->y + controls[i]->yw*0.5 - (gameInterfaceControls.textWidth * scale * (dl/2)), scale);
            }
            else
            {
                float Tw = gameInterfaceControls.textWidth * textLineW;
                float Th = gameInterfaceControls.textHeight * textLineN;
                float Ox = controls[i]->x + controls[i]->xw - (controls[i]->xw-Th)/2;
                float Oy = (controls[i]->y + (controls[i]->yw/2)) - (Tw/2);
                if(controls[i]->text_align_topleft)
                {
                    Ox = controls[i]->x + controls[i]->xw - gameInterfaceControls.textHeight;
                    Oy = controls[i]->y;
                }
                drawText(displayText,
                         // lines drawn along x
                         Ox,
                         // characters drawn along y
                         Oy , 1);
            }
        }
        
        i++;
    }
}

void
drawElem_newFrame()
{
    gl_vertex_ptr_last = gl_texcoord_ptr_last = NULL;
    
    drawElemAnimationIdx = floor((fmodf(time_ms, 1000) / 1000) * (float) TEXTURE_ANIMATION_LEN);
}

void
drawState2dSetCoords(gameGraphics_drawState2d* state)
{
    memcpy(drawState_2d.coords, state->coords, sizeof(drawState_2d.coords));
    memcpy(drawState_2d.texcoords, state->texcoords, sizeof(drawState_2d.texcoords));
    memcpy(drawState_2d.indices, state->indices, sizeof(drawState_2d.indices));
    drawState_2d.tex_id = state->tex_id;
    
    bindTexture(drawState_2d.tex_id);
}

void
drawState2dSet(gameGraphics_drawState2d* state)
{
    drawState2dSetCoords(state);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glVertexPointer(3, GL_FLOAT, 0, drawState_2d.coords);
    glTexCoordPointer(2, GL_FLOAT, 0, drawState_2d.texcoords);
}

void
drawState2dDraw()
{
    glDrawElements(GL_TRIANGLES, sizeof(drawState_2d.indices)/sizeof(model_index_t),
                   index_type_enum, drawState_2d.indices);
}

void
drawElemBatch()
{
    if(drawElem_indicesBatchBuffer_count > 0)
    {
        glVertexPointer(3, GL_FLOAT, 0, drawElem_vertexBatchBufferCur);
        gl_vertex_ptr_last = drawElem_vertexBatchBufferCur;
        glTexCoordPointer(2, GL_FLOAT, 0, drawElem_textCoordBatchBufferCur);
        gl_texcoord_ptr_last = drawElem_textCoordBatchBufferCur;
        glDrawElements(GL_TRIANGLES, drawElem_indicesBatchBuffer_count, index_type_enum, drawElem_indicesBatchBufferCur);
        
        drawElem_indicesBatchBufferCur = drawElem_indicesBatchBufferCur + drawElem_indicesBatchBuffer_count;
        drawElem_vertexBatchBufferCur = drawElem_vertexBatchBufferCur + drawElem_vertexBatchBuffer_count;
        drawElem_textCoordBatchBufferCur = drawElem_textCoordBatchBufferCur + drawElem_textCoordBatchBuffer_count;
        drawElem_batchTotal[0] += drawElem_indicesBatchBuffer_count;
        drawElem_batchTotal[1] += drawElem_vertexBatchBuffer_count;
        drawElem_batchTotal[2] += drawElem_textCoordBatchBuffer_count;
        
        drawElem_indicesBatchBuffer_count = drawElem_vertexBatchBuffer_count = drawElem_textCoordBatchBuffer_count = 0;
    }
}

void
drawElem(WorldElem* pElem)
{
    const int coord_size = 3;
    const int texcoord_size = 2;
    
    if(pElem->type == MODEL_SPRITE)
    {
        drawElemBatch();
        
        // draw billboards
        drawBillboard(pElem);
        
        gl_vertex_ptr_last = NULL;
        gl_texcoord_ptr_last = NULL;
    }
//    else if(pElem->type == MODEL_CONTRAIL)
//    {
//        // TODO: probably hurting performance by committing a batch every time this model is drawn
//        drawElemBatch();
//
//        drawLineBegin();
//
//        gl_vertex_ptr_last = NULL;
//        gl_texcoord_ptr_last = NULL;
//
//        drawLineWithColorAndWidth(&pElem->coords[0], &pElem->coords[3], &pElem->texcoords[0], 2.0);
//        drawLineEnd();
//    }
    else
    {
        if(pElem->renderInfo.tex_adjust)
        {
            for(int x = 0; x < pElem->n_texcoords; x+=2)
            {
                pElem->texcoords[x] += tex_coord_adjust[x];
            }
        }
        
        if(pElem->renderInfo.tex_adjust)
        {
            for(int x = 1; x < pElem->n_texcoords; x+=2)
            {
                pElem->texcoords[x] += tex_coord_adjust[x];
            }
        }
        
        int texture_id = pElem->texture_id;
        
        int texture_id_animated = texture_animated(texture_id, drawElemAnimationIdx);
        
        if(drawn_texture_last != texture_id_animated)
        {
            // dispatch batched drawElements and restart a new batch
            drawElemBatch();
        }
        
        if(bindTexture(texture_id_animated))
        {
        }
        
        model_coord_t* pElemCoords = pElem->coords;
        model_texcoord_t* pElemTexCoords = pElem->texcoords;
        model_index_t* pElemIndices = pElem->indices;
        size_t pElemIndicesCount = pElem->n_indices;
        
        if(drawElem_indicesBatchBuffer_count+drawElem_batchTotal[0]+pElemIndicesCount < drawElem_BatchBuffer_max &&
           drawElem_vertexBatchBuffer_count+drawElem_batchTotal[1]+/*pElem->n_coords*/(pElemIndicesCount*coord_size) < drawElem_BatchBuffer_max &&
           drawElem_textCoordBatchBuffer_count+drawElem_batchTotal[2]+(pElemIndicesCount*texcoord_size) < drawElem_BatchBuffer_max)
        {
            int idx;
            
            unsigned long indicesBatchOffset = drawElem_indicesBatchBuffer_count;
            
            const int coord_inval = -1;

            // TODO: CRASH HERE in malloc
            int *coord_table = malloc(pElemIndicesCount * 4 * sizeof(int));
            if(!coord_table) return; 
            
            model_index_t** face_table = malloc((pElemIndicesCount) * sizeof(model_index_t*));
            if(!face_table)
            {
                free(coord_table);
                return;
            }
            
            int iFace = 0;
            while(iFace < pElemIndicesCount/3)
            {
                face_table[iFace] = &(pElemIndices[iFace*3]);
                iFace++;
            }
            
            for(idx = 0; idx < pElemIndicesCount * 4; idx++)
            {
                coord_table[idx] = coord_inval;
            }
            
            // TODO: only sort faces on "complex" objects
            if(pElem->renderInfo.priority ||
               pElem->renderInfo.concavepoly)
            {
                int sorted = 0;
                
                while(sorted < (pElemIndicesCount/3))
                {
                    iFace = sorted+1;
                    int iMin = sorted;
                    while(iFace < pElemIndicesCount/3)
                    {
                        model_index_t *Ta = face_table[iFace];
                        model_coord_t *Ca = &pElem->coords[*Ta * coord_size];
                        model_index_t *Tb = face_table[iMin];
                        model_coord_t *Cb = &pElem->coords[*Tb * coord_size];
                        
                        model_coord_t a[] = {
                            Ca[0]+(Ca[3]-Ca[0])*0.5, Ca[1]+(Ca[4]-Ca[1])*0.5, Ca[2]+(Ca[5]-Ca[2])*0.5
                        };
                        model_coord_t b[] = {
                            Cb[0]+(Cb[3]-Cb[0])*0.5, Cb[1]+(Cb[4]-Cb[1])*0.5, Cb[2]+(Cb[5]-Cb[2])*0.5
                        };
                        if(cam_distance(a[0], a[1], a[2]) > cam_distance(b[0], b[1], b[2]))
                        {
                            iMin = iFace;
                        }
                        
                        iFace++;
                    }
                    
                    model_index_t* tmp = face_table[sorted];
                    face_table[sorted] = face_table[iMin];
                    face_table[iMin] = tmp;
                    sorted ++;
                }
            }

            iFace = 0;
            idx = 0;
            while(iFace < pElemIndicesCount/3)
            {
                model_index_t *pTriangle = face_table[iFace];
            
                for(int i = 0; i < 3; i++)
                {
                    int coord_unmapped = pTriangle[i];
                    
                    if(coord_table[coord_unmapped] == coord_inval)
                    {
                        coord_table[coord_unmapped] = (int) drawElem_vertexBatchBuffer_count/coord_size;
                        
                        // copy coordinates
                        drawElem_vertexBatchBufferCur[drawElem_vertexBatchBuffer_count+0] = pElemCoords[pTriangle[i]*coord_size];
                        drawElem_vertexBatchBufferCur[drawElem_vertexBatchBuffer_count+1] = pElemCoords[pTriangle[i]*coord_size+1];
                        drawElem_vertexBatchBufferCur[drawElem_vertexBatchBuffer_count+2] = pElemCoords[pTriangle[i]*coord_size+2];
                        
                        // copy texture coordinates
                        drawElem_textCoordBatchBufferCur[drawElem_textCoordBatchBuffer_count+0] = pElemTexCoords[pTriangle[i]*texcoord_size];
                        drawElem_textCoordBatchBufferCur[drawElem_textCoordBatchBuffer_count+1] = pElemTexCoords[pTriangle[i]*texcoord_size+1];
                        
                        drawElem_vertexBatchBuffer_count += coord_size;
                        drawElem_textCoordBatchBuffer_count += texcoord_size;
                    }
                    
                    // copy index
                    drawElem_indicesBatchBufferCur[indicesBatchOffset+idx] = coord_table[pTriangle[i]];
                    idx++;
                }
                
                iFace++;
            }
            drawElem_indicesBatchBuffer_count += idx;
            
            free(coord_table);
            coord_table = NULL;
            
            free(face_table);
            face_table = NULL;
            
            //if(gl_vertex_ptr_last != drawElem_vertexBatchBuffer)
            //{
            //    glVertexPointer(3, GL_FLOAT, 0, drawElem_vertexBatchBuffer);
            //    gl_vertex_ptr_last = drawElem_vertexBatchBuffer;
            //}
            
            
            //if(gl_texcoord_ptr_last != drawElem_textCoordBatchBuffer)
            //{
            //    glTexCoordPointer(2, GL_FLOAT, 0, drawElem_textCoordBatchBuffer);
            //    gl_texcoord_ptr_last = drawElem_textCoordBatchBuffer;
            //}
        }
        else
        {
            drawElemBatch();
        }
    }
}

void
drawElemStart(WorldElemListNode* pVisibleList)
{
    /* setup that happens every frame */
    
    float camXVec[3], camYVec[3];
    
    gameCamera_getXVector(camXVec);
    gameCamera_getYVector(camYVec);
    
    drawBillboardInit(camXVec, camYVec);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    drawElem_newFrame();
    
    if(!drawElem_indicesBatchBuffer)
    {
        drawElem_BatchBuffer_max = 500000;
        drawElem_indicesBatchBuffer = malloc(drawElem_BatchBuffer_max * sizeof(model_index_t));
        drawElem_vertexBatchBuffer = malloc(drawElem_BatchBuffer_max * sizeof(model_coord_t));
        drawElem_textCoordBatchBuffer = malloc(drawElem_BatchBuffer_max * sizeof(model_texcoord_t));
    }
    
    drawElem_indicesBatchBuffer_count = drawElem_vertexBatchBuffer_count = drawElem_textCoordBatchBuffer_count = 0;
    
    drawElem_batchTotal[0] = drawElem_batchTotal[1] = drawElem_batchTotal[2] = 0;
    
    drawElem_indicesBatchBufferCur = drawElem_indicesBatchBuffer;
    drawElem_vertexBatchBufferCur = drawElem_vertexBatchBuffer;
    drawElem_textCoordBatchBufferCur = drawElem_textCoordBatchBuffer;
}

void
drawElemEnd()
{
    drawElemBatch();
    
    glDisable(GL_BLEND);
}

void
visible_list_remove(WorldElem* elem, unsigned int* n_visible, WorldElemListNode** pVisCheckPtr)
{
    // visible-list may contain linked_elems so remove those (because clear-pending will)
    WorldElem* pLinkedElem = elem;
    while(pLinkedElem)
    {
        if(*pVisCheckPtr && pLinkedElem == (*pVisCheckPtr)->elem) *pVisCheckPtr = NULL;
        
        pLinkedElem->in_visible_list = 0;
        // HACK: temporarily invisible to prevent adding back to list below
        pLinkedElem->invisible = 1;
        world_elem_list_remove(pLinkedElem, &gWorld->elements_visible);
        (*n_visible)--;
        pLinkedElem = pLinkedElem->linked_elem;
    }
}

static void
drawBackgroundBuildTerrain(DrawBackgroundData* bgData)
{
    float I_Mk = 25; // model - step size
    float Mk = I_Mk;
    float Tk = 8 / (25/Mk); // texture - step-size multiplier
    float terrain_height_y = 0;
    
    float Ub = -gWorld->bound_radius*1.5;
    float Vb = -gWorld->bound_radius*1.5;
    float Ue = gWorld->bound_radius*1.5;
    float Ve = gWorld->bound_radius*1.5;
    
    int allocated = 0;
    size_t n_coords = 0;
    size_t n_indices = 0;
    size_t n_tcoords = 0;
    size_t n_indices_last;
    
    
    
    do{
        float Vi = Vb;
        float Ui = Ub;
        
        float M[] = {Ui, 0, Vi};
        float T[] = {0, 0};
        
        n_indices_last = n_indices;
        
        if(allocated)
        {
            tess_begin(M, T, bgData->tess.S);
        }
        
        while(Vi <= Ve)
        {
            while(Ui+Mk >= Ub && Ui+Mk <= Ue)
            {
                Ui += Mk;

                if(allocated)
                {
                    M[0] = Ui;
                    M[1] = terrain_height_y;
                    M[2] = Vi;
                    T[0] = Ui/Ue * Tk;
                    T[1] = Vi/Ve * Tk;
                    
                    //terrain_height_y += ((int)floor(rand_in_range(0, 100)) % 2 == 0) ? 0.5: -0.5;
                    
                    tess_step(M, T, bgData->tess.S);
                }
                else
                {
                    n_coords += 3 * 2;
                    n_tcoords += 2 * 2;
                    n_indices += 3 * 2;
                }
            }
            
            Vi += fabs(Mk);
            if(allocated)
            {
                M[0] = Ui;
                M[2] = Vi;
                T[0] = Ui/Ue * Tk;
                T[1] = Vi/Ve * Tk;

                tess_step_row(M, T, bgData->tess.S);
            }
            
            // TODO: if a row is out-of-bounds, invalidate its triangle
            Mk *= -1;
        }
        
        if(!allocated)
        {
            allocated = 1;
            bgData->tess.coords = malloc(sizeof(model_coord_t) * n_coords);
            bgData->tess.indices = malloc(sizeof(model_index_t) * n_indices);
            bgData->tess.texcoords = malloc(sizeof(model_texcoord_t) * n_tcoords);
            
            // tesselate - init
            bgData->tess.S = &bgData->tess.S_;
            bgData->tess.S->Icur = bgData->tess.S->Is = bgData->tess.indices;
            bgData->tess.S->Mcur = bgData->tess.S->Ms = bgData->tess.coords;
            bgData->tess.S->Tcur = bgData->tess.S->Ts = bgData->tess.texcoords;
            
            Mk = I_Mk;
        }
    } while(n_indices != n_indices_last);
    
    tess_end(bgData->tess.S);
    
    // tesselate - done
}

static void
drawBackgroundInit(int tex_id,
                   float alpha, float beta, float gamma,
                   float scale,
                   model_index_t* indices,
                   size_t indices_n)
{
    int i;
    
    if(!bgData)
    {
        bgData = malloc(sizeof(*bgData));
        
        if(bgData)
        {
            memset(bgData, 0, sizeof(*bgData));
            
            bgData->tex_id = tex_id;
            
            bgData->coords = malloc(sizeof(BACKGROUND_MODEL));
            bgData->texcoords = malloc(sizeof(BACKGROUND_MODEL_TEXCOORDS));
            bgData->indices = malloc(sizeof(model_index_t) * indices_n);
            
            // swap triangle indices
            for(i = 0; i < indices_n; i += 3)
            {
                bgData->indices[i] = indices[i+2];
                bgData->indices[i+1] = indices[i+1];
                bgData->indices[i+2] = indices[i];
                bgData->n_indices += 3;
            }
            
            memcpy(bgData->texcoords, BACKGROUND_MODEL_TEXCOORDS, sizeof(BACKGROUND_MODEL_TEXCOORDS));
            
            float alpha = 0;
            float beta = 0;
            float gamma = 0;
            
            for(i = 0; i < sizeof(BACKGROUND_MODEL)/sizeof(model_coord_t); i += 3)
            {
                quaternion_t pt = {0, BACKGROUND_MODEL[i+0] * scale, BACKGROUND_MODEL[i+1] * scale, BACKGROUND_MODEL[i+2] * scale};
                quaternion_t xq = {0, 1, 0, 0};
                quaternion_t yq = {0, 0, 1, 0};
                quaternion_t zq = {0, 0, 0, 1};
                
                // yaw
                if(alpha != 0)
                {
                    quaternion_rotate_inplace(&pt, &zq, alpha);
                    quaternion_rotate_inplace(&xq, &zq, alpha);
                    quaternion_rotate_inplace(&yq, &zq, alpha);
                }
                
                // pitch
                if(beta != 0)
                {
                    quaternion_rotate_inplace(&pt, &xq, beta);
                    quaternion_rotate_inplace(&yq, &xq, beta);
                    quaternion_rotate_inplace(&zq, &xq, beta);
                }
                
                // roll
                if(gamma != 0)
                {
                    quaternion_rotate_inplace(&pt, &zq, gamma);
                    quaternion_rotate_inplace(&xq, &zq, gamma);
                    quaternion_rotate_inplace(&yq, &zq, gamma);
                }
                
                bgData->coords[i+0] =  pt.x;
                bgData->coords[i+1] =  pt.y;
                bgData->coords[i+2] =  pt.z;
            }
            
            drawBackgroundBuildTerrain(bgData);
        }
    }
}

static void
drawBackgroundUninit()
{
    if(bgData)
    {
        if(bgData->coords) free(bgData->coords);
        if(bgData->indices) free(bgData->indices);
        if(bgData->texcoords) free(bgData->texcoords);
        if(bgData->tess.coords) free(bgData->tess.coords);
        if(bgData->tess.indices) free(bgData->tess.indices);
        if(bgData->tess.texcoords) free(bgData->tess.texcoords);
        free(bgData);
        bgData = NULL;
    }
}

void
drawBackground_tess(float* modelC, float* textureC, unsigned int* indicesC, unsigned long indicesN)
{
    glVertexPointer(3, GL_FLOAT, 0, modelC);
    glTexCoordPointer(2, GL_FLOAT, 0, textureC);
    bindTexture(TEXTURE_ID_TERRAIN);
    glDrawElements(GL_TRIANGLES, (int) indicesN, index_type_enum, indicesC);
}

void
drawBackgroundCore()
{
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glOrthof(-1, 1,
             -1, 1,
             1, -1);
    
    if(isLandscape)
    {
        glRotatef(-90, 0, 0, 1);
    }
    
    glRotatef(RADIANS_TO_DEGREES(gameCamera_getEulerGamma()), 0, 0, 1);
    // X'
    glRotatef(RADIANS_TO_DEGREES(gameCamera_getEulerBeta()), 1, 0, 0);
    // Z
    glRotatef(RADIANS_TO_DEGREES(gameCamera_getEulerAlpha()), 0, 0, 1);
    
    glVertexPointer(3, GL_FLOAT, 0, bgData->coords);
    glTexCoordPointer(2, GL_FLOAT, 0, bgData->texcoords);
    bindTexture(bgData->tex_id);
    glDrawElements(GL_TRIANGLES, bgData->n_indices,
                   index_type_enum, bgData->indices);
    
    glPopMatrix();
    
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    
    tess_walk(bgData->tess.S, drawBackground_tess);
}

void drawBackground()
{
    if(background_init_needed)
    {
        drawBackgroundUninit();
        drawBackgroundInit(texture_id_background, 0, 0, 0, 100, BACKGROUND_MODEL_INDICES1, sizeof(BACKGROUND_MODEL_INDICES1) / sizeof(model_index_t));
        background_init_needed = 0;
    }
    drawBackgroundCore();
}

struct
{
    float coord_offsets[4][3];
} drawBillboardData;

void
drawBillboardInit(float xVec[3], float yVec[3])
{
    float coords[4][3] =
    {
        {0, 0, 0},
        {xVec[0], xVec[1], xVec[2]},
        {yVec[0], yVec[1], yVec[2]},
        {xVec[0]+yVec[0], xVec[1]+yVec[1], xVec[2]+yVec[2]}
    };
    
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            coords[i][j] -= xVec[j]/2;
        }
    }
    
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            //coords[i][j] += yVec[j]/2;
            coords[i][j] -= yVec[j]/2;
        }
    }
    
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 3; j++) drawBillboardData.coord_offsets[i][j] = coords[i][j];
}

static gameGraphics_drawState2d drawBillboard_ds;

void
drawBillboard(WorldElem* pElem)
{
    float x = pElem->physics.ptr->x;
    float y = pElem->physics.ptr->y;
    float z = pElem->physics.ptr->z;
    float s = pElem->scale;
    gameGraphics_drawState2d *ds = &drawBillboard_ds;
    int ti;
    
    // TODO: this is putting coordinates below and offset of physics position (instead of equal on all sides)
    /*
    model_coord_t coords[4][3] =
    {
        {x + s*drawBillboardData.coord_offsets[0][0], y + s*drawBillboardData.coord_offsets[0][1], z + s*drawBillboardData.coord_offsets[0][2]},
        {x + s*drawBillboardData.coord_offsets[1][0], y + s*drawBillboardData.coord_offsets[1][1], z + s*drawBillboardData.coord_offsets[1][2]},
        {x + s*drawBillboardData.coord_offsets[2][0], y + s*drawBillboardData.coord_offsets[2][1], z + s*drawBillboardData.coord_offsets[2][2]},
        {x + s*drawBillboardData.coord_offsets[3][0], y + s*drawBillboardData.coord_offsets[3][1], z + s*drawBillboardData.coord_offsets[3][2]}
    };
     */

    ti = 0;
    ds->coords[ti++] = x + s*drawBillboardData.coord_offsets[0][0];
    ds->coords[ti++] = y + s*drawBillboardData.coord_offsets[0][1];
    ds->coords[ti++] = z + s*drawBillboardData.coord_offsets[0][2];
    
    ds->coords[ti++] = x + s*drawBillboardData.coord_offsets[1][0];
    ds->coords[ti++] = y + s*drawBillboardData.coord_offsets[1][1];
    ds->coords[ti++] = z + s*drawBillboardData.coord_offsets[1][2];
    
    ds->coords[ti++] = x + s*drawBillboardData.coord_offsets[2][0];
    ds->coords[ti++] = y + s*drawBillboardData.coord_offsets[2][1];
    ds->coords[ti++] = z + s*drawBillboardData.coord_offsets[2][2];
    
    ds->coords[ti++] = x + s*drawBillboardData.coord_offsets[3][0];
    ds->coords[ti++] = y + s*drawBillboardData.coord_offsets[3][1];
    ds->coords[ti++] = z + s*drawBillboardData.coord_offsets[3][2];
    
    /*
    model_texcoord_t texcoords[4][2] =
    {
        {0, 0},
        {1, 0},
        {0, 1},
        {1, 1}
    };
     */
    ti = 0;
    ds->texcoords[ti++] = 0; ds->texcoords[ti++] = 0;
    ds->texcoords[ti++] = 1; ds->texcoords[ti++] = 0;
    ds->texcoords[ti++] = 0; ds->texcoords[ti++] = 1;
    ds->texcoords[ti++] = 1; ds->texcoords[ti++] = 1;
    
    /*
    model_index_t indices[6] =
    {
        1,0,2,
        1,2,3
    };
     */
    ti = 0;
    ds->indices[ti++] = 1; ds->indices[ti++] = 0; ds->indices[ti++] = 2;
    ds->indices[ti++] = 1; ds->indices[ti++] = 2; ds->indices[ti++] = 3;
    
    ds->tex_id = pElem->texture_id;
    
    drawState2dSet(ds);
    drawState2dDraw();
}

struct
{
    model_coord_t coords[6];
    model_index_t indices[2];
    float colorLast[3];
    float widthLast;
} drawLineData;

void
drawLineBegin()
{
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.0, 1.0, 0.0, 1.0);
}

void
drawLineEnd()
{
    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    gl_vertex_ptr_last = NULL;
    gl_texcoord_ptr_last = NULL;
}

void
drawLineWithColorAndWidth(float a[3], float b[3], float color[3], float width)
{
    int i;
    
    for(i = 0; i < 3; i++) drawLineData.coords[i] = a[i];
    for(i = 0; i < 3; i++) drawLineData.coords[i+3] = b[i];
    
    for(i = 0; i < 2; i++) drawLineData.indices[i] = i;
    
    if(drawLineData.widthLast != width)
    {
        glLineWidth(width);
        drawLineData.widthLast = width;
    }
    
    glColor4f(color[0], color[1], color[2], 1.0);
    /*
    if(drawLineData.colorLast[0] != color[0] ||
       drawLineData.colorLast[1] != color[1] ||
       drawLineData.colorLast[2] != color[2])
    {
        glColor4f(color[0], color[1], color[2], 1.0);
        
        for(int c = 0; c < 3; c++) drawLineData.colorLast[c] = color[c];
    }
     */

    glVertexPointer(3, GL_FLOAT, 0, drawLineData.coords);
    glDrawElements(GL_LINES, 2, index_type_enum, drawLineData.indices);
}

void
drawLine(float a[3], float b[3])
{
    float color[] = {0, 1.0, 0};
    
    drawLineWithColorAndWidth(a, b, color, 1);
}

void
drawLinesElemTriangles(WorldElem *pElem)
{
    int i;
    for(i = 0; i*3 < pElem->n_indices; i++)
    {
        if(pElem->indices[i*3+2]*3+2 >= pElem->n_coords) break;
        
        {
            float a[] =
            {
                pElem->coords[(pElem->indices[i*3]*3)],
                pElem->coords[(pElem->indices[i*3]*3)+1],
                pElem->coords[(pElem->indices[i*3]*3)+2]
            };
            
            float b[] =
            {
                pElem->coords[(pElem->indices[i*3+1]*3)],
                pElem->coords[(pElem->indices[i*3+1])*3+1],
                pElem->coords[(pElem->indices[i*3+1])*3+2],
            };
            
            glColor4f(1.0, 0.0, 0.0, 1.0);
            drawLine(a, b);
        }
        
        {
            float a[] =
            {
                pElem->coords[(pElem->indices[i*3+1]*3)],
                pElem->coords[(pElem->indices[i*3+1]*3)+1],
                pElem->coords[(pElem->indices[i*3+1]*3)+2]
            };
            
            float b[] =
            {
                pElem->coords[(pElem->indices[i*3+2]*3)],
                pElem->coords[(pElem->indices[i*3+2]*3)+1],
                pElem->coords[(pElem->indices[i*3+2]*3)+2],
            };
            
            glColor4f(0.0, 1.0, 0.0, 1.0);
            drawLine(a, b);
        }
        
        {
            float a[] =
            {
                pElem->coords[(pElem->indices[i*3+2]*3)],
                pElem->coords[(pElem->indices[i*3+2]*3)+1],
                pElem->coords[(pElem->indices[i*3+2]*3)+2]
            };
            
            float b[] =
            {
                pElem->coords[(pElem->indices[i*3]*3)],
                pElem->coords[(pElem->indices[i*3]*3)+1],
                pElem->coords[(pElem->indices[i*3]*3)+2],
            };
            
            glColor4f(0.0, 0.0, 1.0, 1.0);
            drawLine(a, b);
        }
    }
}

void
drawLineGrid(float start[3], float u[3], float v[3], float nu, float nv)
{
    float x;
    for(x = 0; x <= nu; x++)
    {
        float k[3];
        float j[3];
        
        for(int i = 0; i < 3; i++)
        {
            j[i] = start[i] + (u[i]*x);
            k[i] = j[i] + (v[i]*nv);
        }
        
        drawLine(j, k);
    }
    
    for(x = 0; x <= nv; x++)
    {
        float k[3];
        float j[3];
        
        for(int i = 0; i < 3; i++)
        {
            j[i] = start[i] + (v[i]*x);
            k[i] = j[i] + (u[i]*nu);
        }
        
        drawLine(j, k);
    }
}

void
drawBoundingLineGrid()
{
    /*
    #define VECBOUND(x) gWorld->boundingRegion->v[(x)].f
    
    // TODO: texture-fill a triangle defined by nearest bounding vector plane when approaching boundary
    int i = 0, j;
    
    int N = WORLD_BOUNDING_SPHERE_STEPS+1;
    for(i = 1; i <= N/2; i += 1)
    for(j = N; j < N*N; j += N)
    {
        float D = sin(M_PI*2 / WORLD_BOUNDING_SPHERE_STEPS) * (gWorld->bound_radius / M_PI*2*i);
        
        {
            float result[3];
            
            float c[] = {
                VECBOUND(j+i)[0],
                VECBOUND(j+i)[1],
                VECBOUND(j+i)[2]
            };
            float v1[] = {
                VECBOUND(j+i)[3],
                VECBOUND(j+i)[4],
                VECBOUND(j+i)[5]
            };
            float v2[] = {
                VECBOUND(j+i-1)[0],
                VECBOUND(j+i-1)[1],
                VECBOUND(j+i-1)[2]
            };

            vector_cross_product(v1, v2, result);
            
            int d;
            for(d = 0; d < 3; d++)
            {
                result[d] = result[d] * D + c[d] - (result[d]*(D/2));
            }
                
            drawLine(c, v2);
            
            //vector_cross_product(u1, u3, result);
            
            //for(d = 0; d < 3; d++)
            //{
            //    result[d] = result[d]*D + u1[d];
            //}
            //drawLine(u1, u3);
        }
    }
    */
    
    /*
    int linter = 20;
    float lstart[] = {0, 0, 0};
    float u[] = {linter, 0, 0};
    float v[] = {0, linter, 0};
    
    // grids along x, y
    lstart[0] = 0; lstart[1] = 0; lstart[2] = 0;
    u[0] = linter; u[1] = 0; u[2] = 0;
    v[0] = 0; v[1] = linter; v[2] = 0;
    drawLineGrid(lstart, u, v, gWorld->bound_x / linter, gWorld->bound_y / linter);
    lstart[2] = gWorld->bound_z;
    drawLineGrid(lstart, u, v, gWorld->bound_x / linter, gWorld->bound_y / linter);
    
    // grids along z, y
    lstart[0] = 0; lstart[1] = 0; lstart[2] = 0;
    u[0] = 0; u[1] = 0; u[2] = linter;
    v[0] = 0; v[1] = linter; v[2] = 0;
    drawLineGrid(lstart, u, v, gWorld->bound_z / linter, gWorld->bound_y / linter);
    lstart[0] = gWorld->bound_x;
    drawLineGrid(lstart, u, v, gWorld->bound_z / linter, gWorld->bound_y / linter);
    
    // grids along , y
    lstart[0] = 0; lstart[1] = 0; lstart[2] = 0;
    u[0] = linter; u[1] = 0; u[2] = 0;
    v[0] = 0; v[1] = 0; v[2] = linter;
    //drawLineGrid(lstart, u, v, gWorld->bound_x / linter, gWorld->bound_z / linter);
    lstart[1] = gWorld->bound_y;
    drawLineGrid(lstart, u, v, gWorld->bound_z / linter, gWorld->bound_z / linter);
     */
    
    WorldElemListNode* pCur = gWorld->drawline_list_head.next;
    while(pCur)
    {
        drawLine(&pCur->elem->coords[0], &pCur->elem->coords[3]);
        pCur = pCur->next;
    }
}

void
drawTriangleMesh(struct mesh_opengl_t* glmesh, int tex_id)
{
    glVertexPointer(3, GL_FLOAT, 0, glmesh->coords);
    
    glTexCoordPointer(2, GL_FLOAT, 0, glmesh->tex_coords);
    
    bindTexture(tex_id);
    
    glDrawElements(GL_TRIANGLES, glmesh->n_indices, index_type_enum, glmesh->indices);
}

void
gameGraphicsInit()
{
}

void
gameGraphicsUninit()
{
    int i;
    void** freeBuffers[] = {
        (void**) &drawElem_indicesBatchBuffer,
        (void**) &drawElem_vertexBatchBuffer,
        (void**) &drawElem_textCoordBatchBuffer
    };
    
    for(i = 0; i < 3; i++) { if(*(freeBuffers[i])) { free(*(freeBuffers[i])); *freeBuffers[i] = NULL; } }
    
    drawBackgroundUninit();
    
    background_init_needed = 1;
}
