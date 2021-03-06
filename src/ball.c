#include <stdio.h>    
#include <stdlib.h>    


#include <GL/glew.h>	
#include <GL/glu.h>    
#include <assert.h>    

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "SDL/SDL_image.h"

#include "ball.h"
#include "scene.h"
#include "util.h"
#include "thread.h"

ball_type_t *ball_type;

ball_type_t *ball_type_create(size_t level, char *name, GLubyte alpha, allocfn_t alloc)
{
    size_t sz = sizeof(ball_type_t) + ball_point_count(level)* sizeof(ball_point_t);
    assert(strlen(name) < BALL_NAME_MAX);
//    printf("Ball has %d points\n",  ball_point_count(level));
    wprintf(L"Created new ball type, size is %.2f kB\n",  (float)sz/1024);
    ball_type_t *res = alloc.fn(alloc.data, sz);
    strcpy(res->name, name);
    res->levels = level;
    res->alpha=alpha;
    return res;    
}

ball_type_t *ball_type_load(char *dir, char *name)
{
    char cbuff[BUFF_SZ];
    
    if(snprintf(cbuff, BUFF_SZ, "data/%s/ball_type/%s.abt", dir, name) < BUFF_SZ)
    {

	struct stat sbuff;
	
	if(!stat(cbuff, &sbuff))
	{
	    
	    FILE *f = fopen(cbuff, "r");
	    if(f)
	    {
		ball_type_t *res = malloc(sbuff.st_size);
		if( res )
		{
		    size_t r = fread(res, sbuff.st_size, 1, f);
		    if((fclose(f)==0) && (r == 1))
		    {
			if(thread_is_render())
			    render_ball_type_prerender(res);
			return res;
		    }
		}
	    }
	}
    }
    wprintf(L"Failed to load ball type %s\n", cbuff);
    exit(1);
}

void ball_type_save(ball_type_t *b, char *dir, char *fn)
{
    char buff[BUFF_SZ];
    
    if(snprintf(buff, BUFF_SZ, "data/%s/ball_type/%s.abt", dir, fn) < BUFF_SZ)
    {
	FILE *f = fopen(buff, "w");
	if(f)
	{
	    size_t sz = sizeof(ball_type_t) + ball_point_count(b->levels)* sizeof(ball_point_t);
	    size_t written = fwrite(b, sz, 1, f);
	    if((fclose(f)==0) && (written == 1))
	    {
		return;
	    }
	}
    }
    wprintf(L"Failed to save ball type %s\n", fn);
    exit(1);
    
}

