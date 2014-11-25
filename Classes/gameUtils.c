
//
//  gameUtils.c
//  gl_flight
//
//  Created by jbrady on 8/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <sys/time.h>
#include "worldElem.h"
#include "gameCamera.h"
#include "gameGlobals.h"
#include "gameUtils.h"
#include "action.h"
#include "gameInterface.h"


typedef int BOOL;
const static int TRUE = 1;
const static int FALSE = 0;

int n_elements_compared = 0;
volatile float time_ms = 0;

float drawDistanceFar = 200;

float 
distance(float x1, float y1, float z1, float x2, float y2, float z2)
{
    float a = (x2-x1);
    float b = (y2-y1);
    float c = (z2-z1);
    
    float s = sqrt(a*a + b*b + c*c);
    return (s);
}

float 
approx_distance(float x1, float y1, float z1, float x2, float y2, float z2)
{
    float a = (x2-x1);
    float b = (y2-y1);
    float c = (z2-z1);
    
    //float s = sqrt(pow(a, 2) + pow(b, 2) + pow(c, 2));
    // faster version, good enough for measuring sorting
    float s = a*a + b*b + c*c;
    return (s);
}

inline static float 
cam_distance_2(float x1, float y1, float z1)
{
    return approx_distance(gameCamera_getX(), gameCamera_getY(), gameCamera_getZ(), x1, y1, z1);
}

float
cam_distance(float x1, float y1, float z1)
{
    return distance(gameCamera_getX(), gameCamera_getY(), gameCamera_getZ(), x1, y1, z1);
}

int 
sort_elem_coordinates_furthest_first(WorldElem* pElem, model_coord_t coords_sorted[])
{
    //float max = cam_distance(pElem->coords[0], pElem->coords[1], pElem->coords[2]);
    int max_i = 0;
    int sorted = 0;
    float inval_dist = -1000;
    int skip[MAX_ELEM] = {FALSE};
    
    while(sorted < pElem->n_coords)
    {
        float max = inval_dist;
        
        for(int i = 0; i < pElem->n_coords; i += 3)
        {
            if(!skip[i])
            {
                float d = cam_distance_2(pElem->coords[i], pElem->coords[i+1], pElem->coords[i+2]);
                if(d > max || max == inval_dist)
                {
                    max = d;
                    max_i = i;
                }
            }
        }
        
        if(max == inval_dist) break;
        
        coords_sorted[sorted] = max_i;
        skip[max_i] = TRUE;
        sorted++;
    }
    return sorted;
}

BOOL
element_visible(WorldElem* pElem, float visibleDistance, float min_dot)
{
    int do_visibility_test = 1;
    
    if(pElem->invisible) return FALSE;
    
    if(pElem->renderInfo.priority > 0) return TRUE;
    
    float cam_x = gameCamera_getX();
    float cam_y = gameCamera_getY();
    float cam_z = gameCamera_getZ();
    
    // distance test
    /*
    pElem->renderInfo.distance =
        distance(cam_x,
                 cam_y,
                 cam_z,
                 pElem->physics.ptr->x,
                 pElem->physics.ptr->y,
                 pElem->physics.ptr->z);
     */
    
    if(pElem->renderInfo.distance > visibleDistance)
    {
        return FALSE;
    }

    
    /*
     * a better visibility test involves checking all the points
     * in the object, but that's pretty expensive
     * so for now a problem exists where large objects may disappear
     * from view even though the object is partially visible
     */
    if(do_visibility_test)
    {   
        // visibility test
        // Skip drawing elements behind us:
        // find dot product of body-z vector and object
        float vec_x = (pElem->physics.ptr->x - (cam_x));
        float vec_y = (pElem->physics.ptr->y - (cam_y));
        float vec_z = (pElem->physics.ptr->z - (cam_z));
        
        if(min_dot > 0) // more complex test
        {
            // convert vec to a unit-vector
            vec_x = vec_x / pElem->renderInfo.distance;
            vec_y = vec_y / pElem->renderInfo.distance;
            vec_z = vec_z / pElem->renderInfo.distance;
        }
        
        float vec_zview[3];
        gameCamera_getZVector(vec_zview);
        
        // TODO: convert vec_x to unit-vector if you want to do this..
        float dot = vec_x*vec_zview[0] + vec_y*vec_zview[1] + vec_z*vec_zview[2];
        
        if(dot < min_dot)
        {
            return FALSE;
        }
    }
    
    return TRUE;
}

unsigned long n_elements_out_of_order = 0;
unsigned int visibility_test_freq = 10;

int
element_dist_compare(WorldElem* pElemA, WorldElem* pElemB)
{
    float distA = 0;
    float distB = 0;
    
    WorldElem* pElem = pElemA;
    
    // if A > B return true
    if(pElemA->head_elem || pElemA->linked_elem)
    {
        // find nearest point in this primitive
        for(int i = 0; i < pElem->n_indices; i++)
        {
            unsigned int c = pElem->indices[i] * 3;
            
            model_coord_t x = pElem->coords[c];
            model_coord_t y = pElem->coords[c+1];
            model_coord_t z = pElem->coords[c+2];
            //model_coord_t d = cam_distance_2(x, y, z);
            model_coord_t d = cam_distance(x, y, z);
            
            if(distA == 0 || d < distA) distA = d;
        }
    }
    else
    {
        //distA = cam_distance_2(pElem->physics.ptr->x, pElem->physics.ptr->y, pElem->physics.ptr->z);
        distA = pElem->renderInfo.distance;
    }
    
    pElem = pElemB;
    
    if(pElemB->head_elem || pElemB->linked_elem)
    {
        // find nearest point in this primitive
        for(int i = 0; i < pElem->n_indices; i++)
        {
            unsigned int c = pElem->indices[i] * 3;
            
            model_coord_t x = pElem->coords[c];
            model_coord_t y = pElem->coords[c+1];
            model_coord_t z = pElem->coords[c+2];
            //model_coord_t d = cam_distance_2(x, y, z);
            model_coord_t d = cam_distance(x, y, z);
            
            if(distB == 0 || d < distB) distB = d;
        }
    }
    else
    {
        //distB = cam_distance_2(pElem->physics.ptr->x, pElem->physics.ptr->y, pElem->physics.ptr->z);
        distB = pElem->renderInfo.distance;
    }
    
    n_elements_compared++;
    
    if(distA > distB)
    {
        n_elements_out_of_order++;
        return 1;
    }
    return 0;
}

game_timeval_t
get_time_ms()
{
    struct timeval tv;
    static struct timeval tv_start = {0};

    /* Warning: unsigned long cannot contain this value */
    gettimeofday(&tv, NULL);
    
    if(tv_start.tv_sec == 0)
    {
        tv_start = tv;
    }
    
    game_timeval_t tvs = tv.tv_sec - tv_start.tv_sec;
    game_timeval_t tvus = tv.tv_usec / 1000;

    time_ms = tvs * 1000 + tvus;
    
    return time_ms;
}

unsigned long
get_time_sec()
{
    return time(NULL);
}

void
conv_3d_to_2d(float vec1[3], float vec2[3], float pt_unit_vec[3], float* v1dot, float* v2dot)
{
    *v1dot = vec1[0]*pt_unit_vec[0] + vec1[1]*pt_unit_vec[1] + vec1[2]*pt_unit_vec[2];
    *v2dot = vec2[0]*pt_unit_vec[0] + vec2[1]*pt_unit_vec[1] + vec2[2]*pt_unit_vec[2];
}

float dot(float v1x, float v1y, float v1z, float v2x, float v2y, float v2z)
{
    return v1x*v2x + v1y*v2y + v1z*v2z;
}

float dot2(float v[3], float u[3])
{
    return dot(v[0], v[1], v[2], u[0], u[1], u[2]);
}

void unit_vector_ab(float a[3], float b[3], float out[3])
{
    float v[3];
    
    for(int i = 0; i < 3; i++) v[i] = b[i]-a[i];
    float d = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    
    for(int i = 0; i < 3; i++) out[i] = v[i] / d;
}

char consoleMessage[2048] = {0};
char consoleMessageTemp[2048] = {0};
char *consoleMessageLast = NULL;

game_timeval_t console_write_time = 0;


void
console_clear()
{
    consoleMessage[0] = '\0';
}

void
console_append(char* fmt, ...)
{
    int i = strlen(consoleMessage);
    
    //if(i > 0) consoleMessage[i++] = '\n';
    consoleMessage[i] = '\0';
    
    if(i > sizeof(consoleMessage)/2) consoleMessage[0] = '\0';
    
    va_list list;
    
    va_start(list, fmt);
    
    /*
    if(i > 0)
    {
        i++;
        strcat(consoleMessage, "\n");
    }
     */
    
    vsnprintf(consoleMessage+i, sizeof(consoleMessage)-i, fmt, list);
    
    va_end(list);
}

void
console_write(char* fmt, ...)
{
    int append = 0;
    char *msgPtr;
  
    if(time_ms - console_write_time < 5000) append = 1;
    msgPtr = append? consoleMessageTemp: consoleMessage;
    
    if(append) strcat(consoleMessage, "\n");
    
    console_write_time = time_ms;
    
    va_list list;
     
    if(strlen(msgPtr) > sizeof(consoleMessage)/2) msgPtr[0] = '\0';
     
    va_start(list, fmt);
    
    vsnprintf(msgPtr, sizeof(consoleMessage), fmt, list);
     
    int l = strlen(msgPtr);
    //if(l > 0 && msgPtr[l-1] == '\n') msgPtr[l-1] = '\0';
     
    va_end(list);
    
    if(append) console_append(consoleMessageTemp);
}

void
console_display_menu(char *buf)
{
    actions_display_menu(buf);
}

void
console_push()
{
    if(consoleMessageLast) free(consoleMessageLast);
    consoleMessageLast = strdup(consoleMessage);
}

void
console_pop()
{
    if(consoleMessageLast)
    {
        strcpy(consoleMessage, consoleMessageLast);
        free(consoleMessageLast);
        consoleMessageLast = NULL;
    }
}

float
rand_in_range(float b, float e)
{
    float range = fabs(e-b);
    if(range == 0) return b;
    
    float m = RAND_MAX / range;
    float k = rand() / m;
    float r = roundf(fmod(k, range));
    
    return b + r;
}

void
rand_seed(long l)
{
    srand(l);
}

void
position_predict(float loc_a[3][3], float loc_b[3], float loc_a1[3], float loc_b1)
{
    float d[3];
    float v[3];
    
    // calculate derivative
    for(int i = 0; i < 3; i++) d[i] = (loc_a[i][0] - loc_a[i][1])/(loc_b[0]-loc_b[1]) -
                                      (loc_a[i][1] - loc_a[i][2])/(loc_b[1]-loc_b[2]);
    
    // calculate velocity
    for(int i = 0; i < 3; i++) v[i] = (loc_a[i][0] - loc_a[i][1]) / (loc_b[0] - loc_b[1]);
    
    // future pos[i] at t = loc_a[i] + (velocity*t) + (derviative*t)
    float t = loc_b1 - loc_b[0];
    
    for(int i = 0; i < 3; i++) loc_a1[i] = loc_a[i][0] + (v[i] * t) + (d[i] * t);
}

float
predict_a1_at_b1_for_a_over_b(float loc_a[3], float loc_b[3], float loc_b1)
{
    float d;
    float v;
    
    // calculate derivative
    d = (loc_a[0] - loc_a[1])/(loc_b[0]-loc_b[1]) -
        (loc_a[1] - loc_a[2])/(loc_b[1]-loc_b[2]);
    
    // calculate velocity
    v = (loc_a[0] - loc_a[1]) / (loc_b[0] - loc_b[1]);
    
    // future pos[i] at t = loc_a[i] + (velocity*t) + (derviative*t)
    float t = loc_b1 - loc_b[0];
    
    return loc_a[0] + (v * t) + (d * t);
}