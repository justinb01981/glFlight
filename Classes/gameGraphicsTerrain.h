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
    float R = 50;
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

// 2d data types
typedef struct Point_ {
    float X, Y;
}Point;

typedef struct Vector_ {
    Point A, B;
} Vector;

Terrain* T = &terrain;

model_index_t allocVertex3(Point p, float Y, float Stex)
{
    model_index_t inext = T->nVertices, tnext = T->nVertices;

    // new modelview vertex d
    T->vertices[inext*3 + 0] = p.X;
    T->vertices[inext*3 + 1] = Y;
    T->vertices[inext*3 + 2] = p.Y;

    // new textureview vertex
    T->texvertices[tnext*2 + 0] = p.X*Stex;
    T->texvertices[tnext*2 + 1] = p.Y*Stex;

    T->nVertices += 1;
    return inext;
}

Point castPoint(Point origin, Vector vec, float Xs, float Ys) {
    Point P = { 
        origin.X + (vec.B.X - vec.A.X)*Xs,
        origin.Y + (vec.B.Y - vec.A.Y)*Ys
    };

    return P;
}

static void terrainBuildHelper(
        Vector U,
        Vector V,
        model_index_t vIndex,
        int depth
        ) {

    float Y = 0.0;
    

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

    // vertex A passed in already
    // add b = a+U
    // add c = a+V

    Point A = 
    { 
        U.A.X,
        U.A.Y 
    };

    Point B = castPoint(A, U, 1, 1), C = castPoint(A, V, 1, 1); // advancing counters for next triangle
    Point D = castPoint(A, U, -1, -1), E = castPoint(A, V, -1, -1);

    model_index_t vnextB = allocVertex3(B, Y, 0.1);
    model_index_t vnextC = allocVertex3(C, Y, 0.1);
    model_index_t vnextD = allocVertex3(D, Y, 0.1);
    model_index_t vnextE = allocVertex3(E, Y, 0.1);

    model_index_t ord[] = {vnextC, vnextB, vnextE, vnextD};

    // add indices to new points designated as triangle
    terrain.indices[terrain.nIndices] = vIndex;
    terrain.indices[terrain.nIndices + 1] = ord[1];
    terrain.indices[terrain.nIndices + 2] = ord[0];
    terrain.nIndices += 3;
    terrain.indices[terrain.nIndices] = vIndex;
    terrain.indices[terrain.nIndices + 1] = ord[3];
    terrain.indices[terrain.nIndices + 2] = ord[2];
    terrain.nIndices += 3;
    terrain.indices[terrain.nIndices] = vIndex;
    terrain.indices[terrain.nIndices + 1] = ord[2];
    terrain.indices[terrain.nIndices + 2] = ord[1];
    terrain.nIndices += 3;
    terrain.indices[terrain.nIndices] = vIndex;
    terrain.indices[terrain.nIndices + 1] = ord[0];
    terrain.indices[terrain.nIndices + 2] = ord[3];
    terrain.nIndices += 3;

    // flip vectors and continue

   
    //terrainBuildHelper(depth + 1);

}

void terrainBuild(void) {

    float R = 5;
    int i, ti;

    terrain.nVertices = 0;
    terrain.nIndices = 0;

    Vector U = { {0,0},{5,5} }, V = { {0,0},{5,-5} };

    model_index_t m = allocVertex3((Point) {U.A.X, U.A.Y}, 0, 0.1);

    terrainBuildHelper(U,V, m, 0);
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