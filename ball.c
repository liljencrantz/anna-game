#include <stdio.h>    
#include <stdlib.h>    


#include <GL/glew.h>	
#include <GL/glu.h>    
#include <assert.h>    

#include "SDL/SDL_image.h"

#include "ball.h"
#include "util.h"

ball_type_t *ball_type;

ball_type_t *ball_type_create(size_t level)
{
    size_t sz = sizeof(ball_type_t) + ball_point_count(level)* sizeof(ball_point_t) + level*sizeof(float);
//    printf("Ball has %d points\n",  ball_point_count(level));
    printf("Created new ball type, size is %.2f kB\n",  (float)sz/1024);
    
    ball_type_t *res = calloc(1,sz);
    
    res->error = (void *)(res) + sizeof(ball_type_t)+ ball_point_count(level)* sizeof(ball_point_t);
    assert(res->error < res+sz);
    res->levels = level;

    return res;    
}

ball_type_t *ball_load(char *name)
{
    int level=6;
    ball_type_t *res = ball_type_create(level);
    int side_size = 1<<level;
    int side_size2 = 2<<level;
    int i, j;
    
    for(i=0;i<side_size2;i++)
    {
	for(j=0;j<side_size;j++)
	{
//	    res->data[ball_idx(level, i, j)].radius = (2.0 + sin((float)i/side_size*M_PI*4)*sin((float)j/(side_size-1)*M_PI*4))/3.0;
	    res->data[ball_idx(level, i, j)].radius = (9.0 + fabs(sin(4.0*(float)i/side_size2*M_PI*4)+sin(2.0*(float)j/(side_size-1)*M_PI*4)))/10.0;
//	    res->data[ball_idx(level, i, j)].radius = (4.0 + cos(1.0*(float)j/(side_size-1)*M_PI*2))/6.0;
	}
    }

    ball_calc(res);

    return res;
    
}


ball_type_t *ball_type_get(char *name)
{
    if(ball_type == 0)
	ball_type = ball_load(0);
    
    return ball_type;
}
