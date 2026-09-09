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


extern GLuint fragmentShader;
extern GLuint vertexShader;
extern unsigned int shaderProgram;
extern unsigned int samplerLoc, uniformS, uniformPos;
extern int UniformEnvironment;
extern GLuint BufferName[], VAONames[];
extern GLint UniformBufferOffset;
extern GLuint TextureName;
extern GLuint *txData;
extern float MVP[4][4];

const static int TERRAIN_BG_MULT = 2;

enum {
    VERTXATTRIB_XFORM,
    VERTXATTRIB_TXPOS,
    VERTXATTRIB_COLOR,
    VERTXATTRIB_MAX
};

enum {
    BUFFERNAME_VTX,
    BUFFERNAME_ELX,
    BUFFERNAME_UTX,
    BUFFERNAME_MAX
};

typedef struct vec2 {
    GLfloat f[2];
} vec2;
typedef struct vec3 {
    GLfloat f[3];
} vec3;
typedef struct vertex_v2fv2f
{
    vec3 Position;
    vec2 Texcoord;
} vertex_v2fv2f;

typedef struct mat4 { GLfloat f[4][4]; } mat4;
typedef struct mat4u { mat4 f, p; } mat4u;
typedef struct vec4 { GLfloat f[4]; } vec4;

#define MAT4IDENT() \
{                   \
1,0,0,0,            \
0,1,0,0,            \
0,0,1,0,            \
0,0,0,1             \
}

#define MAT4SCALE(S) \
{                   \
S,0,0,0,            \
0,S,0,0,            \
0,0,S,0,            \
0,0,0,1             \
}

#define ZeePfoc 1.5

#define MAT4PERSPECTIVE(z, Pfoc) \
{                   \
(-z)/Pfoc  ,0.0,                0.0,        0.0,            \
0.0,            (-z)/Pfoc,      0.0,        0.0,            \
0.0,            0.0,                0.0,        Pfoc,           \
0.0,            0.0,                0.0,        0.0             \
}

#define ORTHOFAR ZeePfoc
#define MAT4ORTHO(w, h) \
{                   \
2.0/w,          0.0,                0.0,            -w,           \
0.0,            2.0/h,              0.0,            -h,           \
0.0,            0.0,                -2.0/ORTHOFAR,   -ORTHOFAR,   \
0.0,            0.0,                0.0,            1             \
}

#define MAT4TRANSLATE(X, Y, Z) \
{                   \
X ,           0.0,                0.0,        0,            \
0.0,            Y,                0.0,        0,            \
0.0,            0.0,                Z,        0,            \
0.0,            0.0,                0.0,        1.0           \
}

#define MAT4ROTY(th)                                      \
{                                                         \
1.0,        0.0,            0.0,    0,            \
0.0,       cos(th),         -sin(th),        0,            \
0.0,       sin(th),         cos(th),    0,            \
0.0,       0.0,             0.0,        1.0           \
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define MAT4MUL_inplace(o, x) \
{   \
int r, c;                   \
for(r = 0; r < 4; r++) { \
for(c = 0; c < 4; c++) { \
    o.f[r][c] *= x.f[r][c];    \
}   \
}   \
}

#define MAT4XLT_inplace(o, x) \
{                               \
int r, c;                       \
for(r = 0; r < 4; r++) {        \
for(c = 0; c < 4; c++) {        \
    o.f[r][c] += x.f[r][c];    \
}                               \
}                               \
}

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
drawBounding(int);

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

void glPrepareShaderAttributesVertices(const GLfloat* src, const size_t len);

void glPrepareShaderAttributesTexUV(const GLfloat* src, const size_t len);

void glPrepareShaderAttributesDraw(const GLushort* src, const size_t len);

#endif
