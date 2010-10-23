#include <stdlib.h>     
#include <stdio.h>     
#include <math.h>
#include <GL/gl.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
#include <unistd.h>     // Header file for sleeping.
#include <sys/time.h>
#include <string.h>
#include <float.h>
#include <assert.h>

#include "util.h"
#include "render.h"
#include "screen.h"
#include "actor.h"
#include "tree.h"
#include "anna_lua.h" 

void init()
{
    tree_load_init();
    
    if(0)
	screen_init(1440, 900, 1);
    else
	screen_init(800, 600, 0);

    render_init();
    anna_lua_init();
}

int main(int argc, char **argv) 
{  
    init();	
    anna_lua_run();
    return 1;
}

