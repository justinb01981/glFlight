#ifndef __GAMEGRAPHICSTERRAIN__
#define __GAMEGRAPHICSTERRAIN__


#include "models.h"
#include "world.h"
#include "gameGlobals.h"
#include "gameGraphics.h"
#include "gameIncludes.h"
#include "textures.h"

#define N 4096
#define VN(x, y) (pI[ x*3+(y)])
#define TN(x, y) (pT[ x*2+(y)])

typedef struct Terrain_ {
    model_coord_t vertices[N];
    model_texcoord_t texvertices[N];
    model_index_t indices[N];
    size_t nIndices, nVertices;
    model_coord_t yOff;
} Terrain;

static Terrain terrain;

void terrainBuildSanity(void) {
    GLfloat R = 50;
    int i, ti;

    terrain.nIndices = 3;

    i = 0; ti = 0;
    terrain.vertices[i++] = 0;
    terrain.vertices[i++] = 0;
    terrain.vertices[i++] = -R;
    terrain.texvertices[ti++] = 0.0;
    terrain.texvertices[ti++] = 0.0;

    terrain.vertices[i++] = R;
    terrain.vertices[i++] = 0;
    terrain.vertices[i++] = R;
    terrain.texvertices[ti++] = 1.0;
    terrain.texvertices[ti++] = 1.0;

    terrain.vertices[i++] = -R;
    terrain.vertices[i++] = 0;
    terrain.vertices[i++] = R;
    terrain.texvertices[ti++] = 0.0;
    terrain.texvertices[ti++] = 1.0;

    i = 0;
    terrain.indices[i++] = 0;
    terrain.indices[i++] = 2;
    terrain.indices[i++] = 1;

    terrain.indices[i++] = 0;   // BOTH sides
    terrain.indices[i++] = 1;
    terrain.indices[i++] = 2;

    terrain.yOff = 0;

}

typedef struct Vector_ {
    float x, y, z;
    float dx, dy, dz;
} Vector;

static void terrainBuildHelper(
        Vector u,
        Vector v,
        model_index_t vIndex,
        int depth
        ) {
    
    Terrain* T = &terrain;
    int i, a, b, c, d;

    unsigned off = 3;
    GLfloat* pI =  &T->vertices[T->nVertices*3 - off*3];
    GLfloat* pT = &T->texvertices[T->nVertices * 2 - off * 2];
 
    if (depth >= 3) return;

    // growing down/left(?) - in a single direction anyway
    /*
     *    d'
     *       \
     *         \
     *       ____ a
     * e'----   /   \
     *         /     \
     *        /       \
     *       /         \
     *      /           \
     *     c-------------b
     *
     *
     *
     *
     */


    a = 0, b = 3, c = 6;    // vertex positions for t1

    GLfloat bax = (pI[b * 3 + 0] - pI[a * 3 + 0]); 
    GLfloat baz = (pI[b * 3 + 2] - pI[a * 3 + 2]);

    GLfloat X = pI[a * 3 + 0] - bax;
    GLfloat Z = pI[a * 3 + 2] - baz;
    GLfloat bcx = (pI[b * 3 + 0] - pI[a * 3 + 0]);
    GLfloat bcz = (pI[b * 3 + 2] - pI[a * 3 + 2]);

    GLfloat X1 = X - bcx;
    GLfloat Z1 = Z - bcz;
    GLfloat Y = T->yOff;

    int i3 = terrain.nVertices * 3, t2 = terrain.nVertices * 2;

    // triangle from indices N,N-1,N-2
    int iD = terrain.nVertices, iB = terrain.nVertices-2, iE = terrain.nVertices + 1, iA = terrain.nVertices - 3, iC =
            terrain.nVertices - 1;   // indices
    GLfloat *pD = &terrain.vertices[iD*3], *pE = &terrain.vertices[iE*3], *pC = &terrain.vertices[iC*3], *pA = &terrain.vertices[iA*3];
    GLfloat *pDt = &terrain.texvertices[iD*2], *pEt = &terrain.texvertices[iE*2], *pCt = &terrain.texvertices[iC*2], *pAt = &terrain.texvertices[iA*2];
    
    // new modelview vertex d
    T->vertices[i3 + 0] = X;
    T->vertices[i3 + 1] = Y;
    T->vertices[i3 + 2] = Z;
    // new texture vertex (u,v) for d
    T->texvertices[t2 + 0] = pT[a * 2 + 0] - (pT[b * 2 + 0] - pT[a * 2 + 0]);
    T->texvertices[t2 + 1] = pT[a * 2 + 1] - (pT[b * 2 + 1] - pT[a * 2 + 1]);
    terrain.nVertices += 1;  // done

    terrain.indices[terrain.nIndices] = iD;
    terrain.indices[terrain.nIndices + 1] = iA;
    terrain.indices[terrain.nIndices + 2] = iE;// pt e forthcoming
    terrain.nIndices += 3;

    // advance counters for next triangle

    int i4 = i3 + 3;
    // new modelview vertex e
    T->vertices[i4 + 0] = X1;
    T->vertices[i4 + 1] = Y;
    T->vertices[i4 + 2] = Z1;
    int t3 = t2 + 2;    // new texture vertex (u,v) for e
    T->texvertices[t3 + 0] = T->texvertices[t2 + 0] - (pT[c * 2 + 0] - pT[a * 2 + 0]);
    T->texvertices[t3 + 1] = T->texvertices[t2 + 1] - (pT[c * 2 + 1] - pT[a * 2 + 1]);
    terrain.nVertices += 1;  // add vertex e

    terrain.indices[terrain.nIndices] = iE;
    terrain.indices[terrain.nIndices + 1] = iA;
    terrain.indices[terrain.nIndices + 2] = iB; // pt e forthcoming
    terrain.nIndices += 3;
    
    // rotate vectors and continue
    terrainBuildHelper(depth + 1);
    terrainBuildHelper(depth + 1);


}

void terrainBuild(void) {

    GLfloat R = 1;
    int i, ti;

    terrain.nVertices = 3;
    terrain.nIndices = 3;

    i = 0; ti = 0;
    terrain.vertices[i++] = R;
    terrain.vertices[i++] = 0;
    terrain.vertices[i++] = 0;
    terrain.texvertices[ti++] = 0.1;
    terrain.texvertices[ti++] = 0.0;

    terrain.vertices[i++] = R*2;
    terrain.vertices[i++] = 0;
    terrain.vertices[i++] = R;
    terrain.texvertices[ti++] = 0.2;
    terrain.texvertices[ti++] = 0.1;

    terrain.vertices[i++] = 0;
    terrain.vertices[i++] = 0.0;
    terrain.vertices[i++] = R;
    terrain.texvertices[ti++] = 0.0;
    terrain.texvertices[ti++] = 0.1;

    i = 0;
    terrain.indices[i++] = 0;
    terrain.indices[i++] = 2;
    terrain.indices[i++] = 1;

//    terrain.indices[i++] = 0;   // BOTH sides!?
//    terrain.indices[i++] = 1;
//    terrain.indices[i++] = 2;

    GLfloat *ptrVtx = (GLfloat*) &terrain.vertices;
    GLfloat *ptrTtx = (GLfloat*) &terrain.texvertices;
    terrainBuildHelper(0);
}

void terrainDraw(void) {

    glVertexPointer(3, GL_FLOAT, 0, terrain.vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, terrain.texvertices);

    // background (skybox) drawing (disabled now in favor of bounding textures
    bindTexture(TEXTURE_ID_TERRAIN);
    glDrawElements(GL_TRIANGLES, terrain.nIndices,
                   index_type_enum, terrain.indices);

}


#endif