#include <stdlib.h>     
#include <stdio.h>     
#include <math.h>
#include <GL/glew.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
#include <unistd.h>     // Header file for sleeping.
#include <sys/time.h>
#include <string.h>
#include <float.h>
#include <assert.h>
#include <GL/glew.h>

#include "util.h"
#include "render.h"
#include "screen.h"
#include "tree.h"
#include "anna_lua.h" 
#include "ior.h" 

#define IOR_MEM_USAGE 1024*1024*16

void init()
{
    tree_load_init();
    
    if(0)
	screen_init(1440, 900, 1);
    else
	screen_init(960, 600, 0);

    render_init();    
    set_current_thread_name("anna (main)");
}
/*
int main(int argc, char **argv) 
{  
    init();	
    anna_lua_run();
    return 0;
}

*/
