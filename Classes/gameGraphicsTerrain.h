#ifndef __GAMEGRAPHICSTERRAIN__
#define __GAMEGRAPHICSTERRAIN__


#include "models.h"
#include "world.h"
#include "gameGlobals.h"
#include "gameGraphics.h"
#include "gameIncludes.h"
#include "textures.h"

#define N 327680 

typedef struct Terrain_ {
    model_coord_t vertices[N];
    model_texcoord_t texvertices[N];
    model_index_t indices[N];
    size_t nIndices, nVertices;
    model_coord_t yOff;
} Terrain;

static Terrain terrain;

// 2d data types
typedef struct Point_ {
    float X, Y, Z;
}Point;

typedef struct Vector_ {
    Point A, B;
} Vector;



// MARK: -- API exposed global defines here
Terrain* T = &terrain;
int cols = 96;
int rows = 96;
float Xt = 0;
float Zt = 0;
float Tsc = 0.05;


model_index_t allocVertex3(Point p, float Stex)
{
    model_index_t inext = T->nVertices, tnext = T->nVertices;

    // new modelview vertex d
    T->vertices[inext*3 + 0] = p.X;
    T->vertices[inext*3 + 1] = p.Y;
    T->vertices[inext*3 + 2] = p.Z;

    // new textureview vertex
    T->texvertices[tnext*2 + 0] = p.X*Stex;
    T->texvertices[tnext*2 + 1] = p.Z*Stex;

    T->nVertices += 1;
    return inext;
}

float YcalculateFromXZ(float X, float Z) {

    //return sin((X - Xt) / 3.14) - cos((Z - Zt) / 3.14) + 5;
    return 0; ///sin(X-Xt)* log(fabs(Z)); // ignoring Zt
}

Point castPoint(Point origin, Vector vec, float Sx, float Sy) {
    float X = origin.X + (vec.B.X - vec.A.X) * Sx;
    float Z = origin.Z + (vec.B.Z - vec.A.Z) * Sy;

    float Ycalc = YcalculateFromXZ(X, Z);

    Point P = { 
        X,
        Ycalc,//origin.Y + (vec.B.Y - vec.A.Y)*S,
        Z
    };

    return P;
}

static void terrainBuildHelper(
                               Vector U,
                               Vector V,
                               model_index_t vIndex,
                               const float R
        )
{
    // THIS CALL ALLOCATES 4 VERTICES(?) MAKE SURE SPACE IS AVAILABLE IN TERRAIN

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
        U.A.Y,
        U.A.Z
    };

    float Ks = R;
    Point B = castPoint(A, U, Ks, Ks), C = castPoint(A, V, Ks, Ks); // advancing counters for next triangle
    Point D = castPoint(A, U, -Ks, -Ks), E = castPoint(A, V, -Ks, -Ks);

    model_index_t vnextB = allocVertex3(B, Tsc);
    model_index_t vnextC = allocVertex3(C, Tsc);

    model_index_t vnextD = allocVertex3(D, Tsc);
    model_index_t vnextE = allocVertex3(E, Tsc);

    model_index_t ord[] = {vnextC, vnextB, vnextE, vnextD};

    // add indices to new points designated as triangle

    model_index_t PAIRS[] = {
        1,0,
        3,2,
        2,1,
        0,3
    };

    for (int i = 0; i < sizeof(PAIRS) / sizeof(model_index_t); i += 2) 
    {
        terrain.indices[terrain.nIndices] = vIndex;
        terrain.indices[terrain.nIndices + 1] = ord[PAIRS[i]];
        terrain.indices[terrain.nIndices + 2] = ord[PAIRS[i+1]];
        terrain.nIndices += 3;
        // triangles on both sides
        terrain.indices[terrain.nIndices] = vIndex;
        terrain.indices[terrain.nIndices + 1] = ord[PAIRS[i+1]];
        terrain.indices[terrain.nIndices + 2] = ord[PAIRS[i]];
        terrain.nIndices += 3;
    }


    // flip vectors and continue

}

void terrainBuild(void) {
        
    assert(cols * rows * 4 < N);

    float R = gWorld->bound_radius / 10;    // 1/2 width of triangles

    terrain.nVertices = 0;
    terrain.nIndices = 0;

    // steps are 2R apart bc this is width of triangles terrainBuildHelper creates
    for (float col = cols/-2; col < cols/2; col += R) {

        for (float row = rows/-2; row < rows/2; row += R) {

            Vector U = { {col,YcalculateFromXZ(col,row),row},{col+R,YcalculateFromXZ(col+R,row+R),row+R} }, V = { {col,YcalculateFromXZ(col,row),row},{col+R,YcalculateFromXZ(col+R,row-R),row-R} };

            model_index_t m = allocVertex3((Point) { U.A.X, U.A.Y, U.A.Z }, /* tex scale */ Tsc);

            terrainBuildHelper(U, V, m, R);
        }
    }
}

void terrainDraw(void) {

    //terrainBuild();// dude try moving this to async without locking and see what it looks like drawing

    glVertexPointer(3, GL_FLOAT, 0, terrain.vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, terrain.texvertices);

    // background (skybox) drawing (disabled now in favor of bounding textures
    bindTexture(TEXTURE_ID_TERRAIN);
    glDrawElements(GL_TRIANGLES,
                   (GLsizei) terrain.nIndices,
                   index_type_enum,
                   terrain.indices);

    Xt += 3.141 / 240.0;
    Zt += 0.3;
}


#endif
