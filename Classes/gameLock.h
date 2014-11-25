//
//  gameLock.h
//  gl_flight
//
//  Created by Justin Brady on 6/6/13.
//
//

#ifndef gl_flight_gameLock_h
#define gl_flight_gameLock_h

#include <pthread.h>

typedef struct
{
    pthread_mutex_t m;
} game_lock_t;

inline static void
game_lock_init(game_lock_t* g)
{
    pthread_mutex_init(&g->m, NULL);
}

inline static void
game_lock_uninit(game_lock_t* g)
{
    pthread_mutex_destroy(&g->m);
}

inline static void
game_lock_lock(game_lock_t* g)
{
    pthread_mutex_lock(&g->m);
}

inline static void
game_lock_unlock(game_lock_t* g)
{
    pthread_mutex_unlock(&g->m);
}


#endif
