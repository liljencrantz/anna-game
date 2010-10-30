#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <GL/glew.h>	
#include <math.h>	

#include "scene.h"
#include "render.h"
#include "util.h"
#include "boid.h"
#include "actor.h"


void render_boids(scene_t *s)
{

    
    int i, j;
    glPointSize(8);
    glColor3f(0,0,1);    
    glBegin(GL_POINTS);
    
    for(i=0; i<scene_boid_set_get_count(s); i++)
    {
	//printf("A\n");
	
	boid_set_t *boid_set = scene_boid_set_get(s, i);
	boid_set->target[0] = 100 - 60 * cos(s->time*0.01);
	boid_set->target[1] = 40;//80 + 60 * sin(s->time*0.05);
	boid_set->target[2] =  scene_get_height(s, boid_set->target[0],
						   boid_set->target[1]) + 5;
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

    }
    glEnd();
    
}
