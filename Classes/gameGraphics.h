//
//  gameGraphics.h
//  gl_flight
//
//  Created by Justin Brady on 2/28/13.
//
//

#ifndef gl_flight_gameGraphics_h
#define gl_flight_gameGraphics_h

#include "models.h"
#include "world.h"

#define GAME_GRAPHICS_DRAW_FPS 60

extern float viewWidth;
extern float viewHeight;
extern int radar_mode;

const static int TERRAIN_BG_MULT = 2;

typedef struct
{
    model_coord_t *coords;
    model_texcoord_t *texcoords;
    model_index_t *indices;
    int n_indices;
    int tex_id;
    
    struct {
        struct tess_storage_t* S;
        struct tess_storage_t S_;
        model_coord_t *coords;
        model_texcoord_t *texcoords;
        model_index_t *indices;
    } tess;
    
} DrawBackgroundData;

typedef struct
{
    model_coord_t coords256[6194];
    model_texcoord_t txcoords256[2048];
    model_index_t indices256[2048];
    int count;
} DrawBoundingData;

typedef struct
{
    model_coord_t coords[12];
    model_texcoord_t texcoords[8];
    model_index_t indices[6];
    int tex_id;
} gameGraphics_drawState2d;

void
drawState2dSetCoords(gameGraphics_drawState2d* state);

void
drawState2dSet(gameGraphics_drawState2d* state);

void
drawState2dDraw();

void
drawText(char* text , float x, float y, float scale);

void
drawControls();

int
drawRadar();

void
drawBackground();

void
drawBounding();

void
drawBillboardInit(float xVec[3], float yVec[3]);

void
drawBillboard(WorldElem* pElem);

void
drawElem_newFrame();

void
drawElem(WorldElem* pElem);

void
drawElemStart(WorldElemListNode* pVisibleList);

void
drawElemEnd();

void
drawLineBegin();

void
drawLineEnd();

void
drawLineWithColorAndWidth(float a[3], float b[3], float color[3], float width);

void
drawLine(float a[3], float b[3]);

void
drawLinesElemTriangles(WorldElem* pElem);

void
drawLineGrid(float start[3], float u[3], float v[3], float nu, float nv);

void
drawBoundingLineGrid();

void
drawTriangleMesh(struct mesh_opengl_t* glmesh, int tex_id);

void
drawLines(float start[3], float vend[3], float step_v[3], int step_n);

void
visible_list_remove(WorldElem* elem, unsigned int* n_visible, WorldElemListNode** pVisibleCheck);

void
gameGraphicsInit();

void
gameGraphicsUninit();

int
bindTexture(unsigned int tex_id);

#endif
