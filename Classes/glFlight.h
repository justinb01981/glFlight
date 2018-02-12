//
//  glFlight.h
//  gl_flight
//
//  Created by Justin Brady on 2/28/13.
//
//

#ifndef gl_flight_glFlight_h
#define gl_flight_glFlight_h

extern int world_inited;
extern int game_paused;
extern int game_terminated_gracefully;
extern char* world_data;

extern void (*glFlightDrawframeHook)(void);

void
glFlightFrameStage1();

void
glFlightFrameStage2();

void
glFlightCamFix(int elem_id);

#endif
