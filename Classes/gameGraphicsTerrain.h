#ifndef __GAMEGRAPHICSTERRAIN__
#define __GAMEGRAPHICSTERRAIN__


#include "models.h"
#include "world.h"
#include "gameGlobals.h"
#include "gameGraphics.h"
#include "gameIncludes.h"
#include "textures.h"

#define N 32768000
#define TRAD 10 // triangle size
#define STRIDE 10 // spacing between triangles

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
float Xt = 0;
float Zt = 0;
float Tsc = 0.02;

Point DVertex( model_index_t idx, Point a) {
    Point r = {
        .X = a.X - T->vertices[idx*3],
        .Y = a.Y - T->vertices[idx*3+1],
        .Z = a.Z - T->vertices[idx*3+2]
    };

    return r;
}

//int substituteVertexFind(Point a) {
//    int i;
//    const float MAX = 0.0001;
//    for(i = 0; i < T->nVertices; i++)
//    {
//        Point p = DVertex(i, a);
//
//        // guess what mergy Y vertices
//        if(fabs(p.X) > MAX /* || fabs(p.Y) > MAX */ || fabs(p.Z) > MAX)
//        {
//            continue;
//        }
//        return i;
//    }
//    return -1;
//}

model_index_t allocVertex3(Point p, float Stex)
{
    model_index_t inext = T->nVertices, tnext = T->nVertices;

    // new modelview vertex d
    T->vertices[inext*3 + 0] = p.X;
    T->vertices[inext*3 + 1] = p.Y;
    T->vertices[inext*3 + 2] = p.Z;

    // new textureview vertex
    T->texvertices[tnext*2 + 0] = p.X*Stex + 0.25;
    T->texvertices[tnext*2 + 1] = p.Z*Stex + 0.25;

    T->nVertices += 1;
    return inext;
}

float YcalculateFromXZ(float X, float Z) {
//    return sin(X/8+Xt)*2 - 1.0;
//    return  16 / -(sqrt( pow(my_ship_x-X,2) + pow(my_ship_z-Z,2))) + 8;
    return 0;

}

Point projectPointVec(Point origin, Vector vec, float Sx, float Sy) {
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

static void terrainBuildHelper(Point A,
                               Vector U,
                               Vector V,
                               const float R,
                               const float width
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

    Point rowPt = A;    // mutate this


    while(distance(rowPt.X,rowPt.Y,rowPt.Z, A.X,A.Y,A.Z) <= width) {   // walk U

        Point colPt = rowPt;

        while(distance(colPt.X,colPt.Y,colPt.Z, A.X,A.Y,A.Z) <= width) {   // walk V

            float tW = R;
            Point curPt = colPt; //projectPointVec(projectPointVec(colPt,U,tW,tW), V, tW, tW);   // offset to top left

            model_index_t vIndex = allocVertex3(curPt, Tsc); // vtx of pt A + offset currently

            Point B = projectPointVec(curPt, U, tW, tW);
            Point C = projectPointVec(B, V, tW, tW); // advancing counters for next triangle
            Point D = projectPointVec(curPt, V, tW,tW);
            
            

            model_index_t vnextB = allocVertex3(B, Tsc);
            model_index_t vnextC = allocVertex3(C, Tsc);
            model_index_t vnextD = allocVertex3(D, Tsc);

            model_index_t ord[] = {vnextB, vnextC, vnextC, vnextD};

            // add indices to new points designated as triangle
            terrain.indices[terrain.nIndices++] = vIndex;
            terrain.indices[terrain.nIndices++] = ord[1];
            terrain.indices[terrain.nIndices++] = ord[0];
            terrain.indices[terrain.nIndices++] = vIndex;
            terrain.indices[terrain.nIndices++] = ord[3];
            terrain.indices[terrain.nIndices++] = ord[2];

            colPt = projectPointVec(colPt, V, R, R);
        }

        rowPt = projectPointVec(rowPt, U, R, R); // rowPt advancing along U
    }
}

void terrainUninit(void) {
    // dealloc
    terrain.nVertices = 0;
    terrain.nIndices = 0;
}

void terrainBuild(void) {

    float R = TRAD;
    float X = 2;
    float width = gWorld->bound_radius * 2.2;

    float col = -gWorld->bound_radius; //my_ship_x - width/2;
    float row = -gWorld->bound_radius; //my_ship_z - width/2;

    Point A = {col, YcalculateFromXZ(col,row), row};

    Vector
    U = { A,
        {col+X,YcalculateFromXZ(col+X,row),row} },
    V = { A,
        {col,YcalculateFromXZ(col,row+X),row+X} };

    //model_index_t iA = allocVertex3((Point) { U.A.X, U.A.Y, U.A.Z }, /* tex scale */ Tsc);

    terrainBuildHelper(A, U, V, R, width);
}

void terrainInit(void) {
    // only called on gameGraphicsInit

    terrain.nVertices = 0;
    terrain.nIndices = 0;

    terrainBuild();
}

void terrainDraw(void) {

    //terrainInit();
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
