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

void load_temp_tile_data(scene_t *s)
{
    tile_t *t = calloc(1, sizeof(tile_t));
    s->root_tile=t;
    int i, j;
    
    for(i=0; i<TILE_SUBTILE_COUNT; i++)
    {
	t->subtile[i] = calloc(1, sizeof(tile_t));
    }
    for(i=0; i<(2<<7); i++)
    {
	for(j=0; j<(2<<7); j++)
	{
	    hid_t hid;
	    HID_SET(hid, 7, i, j);
	    //printf("%d %d\n", i,j);
	    assert(i == HID_GET_X_POS(hid));
	    assert(j == HID_GET_Y_POS(hid));
	    
	    tile_hid_lookup(t,hid)->height = 1.5*sin(0.3*i)+1.5*sin(0.3*j);
	    tile_hid_lookup(t,hid)->color[0] = 42;
	    tile_hid_lookup(t,hid)->color[1] = 84;
	    tile_hid_lookup(t,hid)->color[2] = 42;
	    
	    //printf("%d\n", hid.id, fabs(tile_hid_lookup(t,hid)->height - (0.0*sin(0.1*i)+0*sin(0.2*j)+0.1*i)));
	    
//	    assert(fabs(tile_hid_lookup(t,hid)->height - (0.0*sin(0.1*i)+0*sin(0.2*j)+0.1*i)) < 0.001);
	}
	
    }
    for(i=0; i<(2<<7); i++)
    {
	for(j=0; j<(2<<7); j++)
	{
	    hid_t hid;
	    HID_SET(hid, 7, i, j);
//	    printf("%d %d %f %f\n", i, j, tile_hid_lookup(t,hid)->height, (0.0*sin(0.1*i)+0*sin(0.2*j)+0.1*i));

//	    assert(fabs(tile_hid_lookup(t,hid)->height - (0.0*sin(0.1*i)+0*sin(0.2*j)+0.1*i)) < 0.001);
	}
	
    }
    tile_calc(s, 8);
}

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

