#ifndef __CPTHRAD__
#define __CPTHREAD__

/* a simple thread class just to encapsulate all the necessary thread variables and functions into one class,
 * across all platforms. 
 */

#ifdef _BWINDOWS_
#include <windows.h>
#endif

#ifdef _UNIX_
#include <pthread.h>

#define HANDLE unsigned long
#define DWORD pthread_t
#endif

struct ThreadHelperStruct
{
  bool running;
  void* funcptr;
  void* params;
};

class CPThread {

 public:
  CPThread(void* nfuncptr, void* nparams);     /* setup the thread, but dont start it */

  int start();    /* start the thread off */

  /* HEY! this function will wait for the thread to return on it's own, NOT kill it!
   * you have to implement some kind of a global variable to let the thread know when it should return
   */
  int stop();  /* stop the thread */

  bool done();    /* is the thread done yet? */

  ~CPThread();

 private:

 void helper();

  HANDLE handle1;
  DWORD threadid;

#ifdef _UNIX_
  struct ThreadHelperStruct hs;
#endif

  void* funcptr;
  void* params;

};

#endif
