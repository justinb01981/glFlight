//
//  gameBounding.c
//  gl_flight
//
//  Created by jbrady on 12/9/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>

#include "gameBounding.h"

#include <math.h>
#include <stdlib.h>

boundingRegion*
boundingRegionInit(int numVectors)
{
    boundingRegion* rgn;
    rgn = malloc(sizeof(*rgn) + sizeof(struct boundingRegionVector)*numVectors);
    rgn->nVectorsInited = 0;
    return rgn;
}

void
boundingRegionUninit(boundingRegion* rgn)
{
    free(rgn);
}

// takes point, and unit-vector
void boundingRegionAddVec(boundingRegion* rgn, float x, float y, float z, float ux, float uy, float uz)
{
    int i = 0;
    rgn->v[rgn->nVectorsInited].f[i++] = x;
    rgn->v[rgn->nVectorsInited].f[i++] = y;
    rgn->v[rgn->nVectorsInited].f[i++] = z;
    rgn->v[rgn->nVectorsInited].f[i++] = ux;
    rgn->v[rgn->nVectorsInited].f[i++] = uy;
    rgn->v[rgn->nVectorsInited].f[i++] = uz;
    rgn->nVectorsInited++;
}

int boundingRegionCheckPointInside(boundingRegion* rgn, float x, float y, float z, float vout[3])
{
    int i;
    float minDot = 0.01;
    
    for(i = 0; i < rgn->nVectorsInited; i++)
    {   
        float vx = x - rgn->v[i].f[0];
        float vy = y - rgn->v[i].f[1];
        float vz = z - rgn->v[i].f[2];
        
        float d = sqrt(vx*vx + vy*vy + vz*vz);
        
        vx /= d;
        vy /= d;
        vz /= d;
        
        float ux = rgn->v[i].f[3];
        float uy = rgn->v[i].f[4];
        float uz = rgn->v[i].f[5];
        
        float dot = ux*vx + uy*vy + uz*vz;
        
        if(dot <= minDot)
        {
            vout[0] = ux;
            vout[1] = uy;
            vout[2] = uz;
            return 0;
        }
    }
    return 1;
}
