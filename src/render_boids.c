#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <GL/glew.h>	
#include <math.h>	

#include "scene.h"
#include "render.h"
#include "util.h"
#include "boid.h"


void render_boids(scene_t *s)
{

    
    int i, j;
    glPointSize(8);
    glColor3f(0,0,1);    
    glBegin(GL_POINTS);
    int count = 0;
    
    for(i=0;; i++)
    {
	boid_set_t *boid_set = scene_boid_set_get(s, i);
	if( boid_set)
	{
	    count++;
	    
	    for(j=0; j<boid_set->count; j++)
	    {
		float corr = render_height_correct(
		    boid_set->data[j].pos[0]-s->camera.pos[0],
		    boid_set->data[j].pos[1]-s->camera.pos[1]
		    );
		glVertex3f(
		    boid_set->data[j].pos[0],
		    boid_set->data[j].pos[1],
		    boid_set->data[j].pos[2]+corr
		    );
	    }
	    if(count >= scene_boid_set_get_count(s))
		break;
	}	
    }
    glEnd();
    
}
