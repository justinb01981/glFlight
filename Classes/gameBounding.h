//
//  gameBounding.h
//  gl_flight
//
//  Created by jbrady on 12/9/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef gl_flight_gameBounding_h
#define gl_flight_gameBounding_h

struct boundingRegionVector
{
    float f[6];
    float perpVec[3];
};

struct boundingRegion
{
    int nVectorsInited;
    struct boundingRegionVector v[1];
};
typedef struct boundingRegion boundingRegion;

boundingRegion*
boundingRegionInit(int numVectors);

void
boundingRegionUninit(boundingRegion* rgn);

// takes point, and unit-vector, and perpendicular-vector
void boundingRegionAddVec(boundingRegion* rgn, float x, float y, float z, float ux, float uy, float uz, float pvec[3]);

int boundingRegionCheckPointInside(boundingRegion* rgn, float x, float y, float z, float vout[3]);

#endif
