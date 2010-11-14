#ifndef THREAD_HH
#define THREAD_HH

#include <pthread.h>

/**
   Returns true of the current thread is the rendering thread
*/
int thread_is_render();
void thread_set_render();

#endif
