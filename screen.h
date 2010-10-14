
#ifndef SCREEN_H
#define SCREEN_H

#include "common.h"

/**
   Set up a window for OpenGL rendering with the specified properties
*/
void screen_init(int w, int h, int fullscreen);
void screen_check_input();
int screen_key_get(unsigned int key);
void screen_destroy();

void screen_swap_buffers();

#endif
