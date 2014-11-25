//
//  gameUtils.h
//  gl_flight
//
//  Created by jbrady on 8/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef gl_flight_gameUtils_h
#define gl_flight_gameUtils_h

#include "models.h"
#include "worldElem.h"
#include "gameTimeval.h"

#define MIN(x, y) ((x) < (y)? (x): (y))
#define MAX(x, y) ((x) > (y)? (x): (y))

extern int n_elements_compared;
extern unsigned long n_elements_out_of_order;
extern int do_visibility_test;
extern int nearest_first;
extern game_timeval_t console_write_time;

float
distance(float x1, float y1, float z1, float x2, float y2, float z2);

float
approx_distance(float x1, float y1, float z1, float x2, float y2, float z2);

float
cam_distance(float x1, float y1, float z1);

int
sort_elem_coordinates_furthest_first(WorldElem* pElem, model_coord_t coords_sorted[]);

int
element_visible(WorldElem* pElem, float visibleDistance, float min_dot);

int
element_dist_compare(WorldElem* pElemA, WorldElem* pElemB);

float
get_time_ms();

unsigned long
get_time_sec();

void
conv_3d_to_2d(float vec1[3], float vec2[3], float pt_unit_vec[3], float* v1dot, float* v2dot);

float
dot(float v1x, float v1y, float v1z, float v2x, float v2y, float v2z);

float
dot2(float v[3], float u[3]);

void
unit_vector_ab(float a[3], float b[3], float out[3]);

void
console_write(char* fmt, ...);

void
console_clear();

void
console_display_menu(char *buf);

void
console_pop();

void
console_push();

void
console_append(char* fmt, ...);

float
rand_in_range(float b, float e);

void
rand_seed(long l);

void
position_predict(float loc_a[3][3], float loc_b[3], float loc_a1[3], float loc_b1);

float
predict_a1_at_b1_for_a_over_b(float loc_a[3], float loc_b[3], float loc_b1);

#endif
