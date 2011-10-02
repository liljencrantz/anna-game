
#include "thread.h"


static pthread_t thread_render;

int thread_is_render()
{
    return pthread_equal(thread_render, pthread_self());    
}

void thread_set_render(pthread_t r)
{
    thread_render = pthread_self();
}

