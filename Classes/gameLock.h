//
//  gameLock.h
//  gl_flight
//
//  Created by Justin Brady on 6/6/13.
//
//

#ifndef gl_flight_gameLock_h
#define gl_flight_gameLock_h

#ifndef _NOT_POSIX
#include <pthread.h>
#else
#include <windows.h>
#endif

typedef struct
{
#ifndef _NOT_POSIX
    pthread_mutex_t
#else
	HANDLE
#endif
	m;
} game_lock_t;

inline static void
game_lock_init(game_lock_t* g)
{
#ifndef _NOT_POSIX
    pthread_mutex_init(&g->m, NULL);
#else
	g->m = CreateMutex(NULL, FALSE, NULL);
#endif
}

inline static void
game_lock_uninit(game_lock_t* g)
{
#ifndef _NOT_POSIX
    pthread_mutex_destroy(&g->m);
#else
	CloseHandle(g->m);
#endif
}

inline static void
game_lock_lock(game_lock_t* g)
{
#ifndef _NOT_POSIX
    pthread_mutex_lock(&g->m);
#else
	WaitForSingleObject(g->m, INFINITE);
#endif
}

inline static void
game_lock_unlock(game_lock_t* g)
{
#ifndef _NOT_POSIX
    pthread_mutex_unlock(&g->m);
#else
	ReleaseMutex(g->m);
#endif
}


#endif
