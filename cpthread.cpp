
#include "cpthread.h"

#ifdef _BWINDOWS_
#include <windows.h>
#endif

#ifdef _UNIX_
#include <pthread.h>
#include <signal.h>
#endif

typedef void funcptr(void*);

/* nasty hack */
void mythreadhelper(void* params)
{
#ifdef _BWINDOWS_
#endif

#ifdef _UNIX_
  struct ThreadHelperStruct *hs = (struct ThreadHelperStruct*) params;

  hs->running = true;
  (*((funcptr*) hs->funcptr))(hs->params);
  hs->running = false;
#endif
}

CPThread::CPThread(void* nfuncptr, void* nparams)
{

  funcptr = nfuncptr;
  params = nparams;

#ifdef _BWINDOWS_
#endif

#ifdef _UNIX_
#endif
}

int CPThread::start()
{

#ifdef _BWINDOWS_
  handle1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) funcptr, params, 0x0, &threadid);
  if(handle1 == NULL)
    return -1;

  return 0;
#endif

#ifdef _UNIX_
  int thread_ret;

  hs.params = params;
  hs.funcptr = funcptr;
  thread_ret = pthread_create(&threadid, NULL, (void* (*)(void*)) mythreadhelper,  &hs);
  if(thread_ret != 0)
    return -1;

  return 0;
#endif

}

int CPThread::stop()
{
#ifdef _BWINDOWS_
    DWORD r;

    /* wait for the thread to finish */
    while(GetExitCodeThread(handle1, &r) == STILL_ACTIVE)
    {
        Sleep(1);
    }
    CloseHandle(handle1);    /*TerminateThread(handle1, 0);*/
    
#endif

#ifdef _UNIX_
  pthread_join(threadid, (void**) NULL); /* wait for the thread to die */
#endif

  return 0;
}

bool CPThread::done()
{
#ifdef _BWINDOWS_
    DWORD r;
    if(!GetExitCodeThread(handle1, &r)) return false;

    if(r == STILL_ACTIVE)
        return false;
    else
        return true;
#endif

#ifdef _UNIX_
    /* is the thread joinable yet? */
        return !hs.running;
#endif

}




CPThread::~CPThread()
{
  stop();
}
