#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <GL/glew.h>	
#include <math.h>	

#include "scene.h"
#include "render.h"
#include "util.h"

void render_markers( scene_t *s )
{
//    glDisable(GL_DEPTH_TEST);
    glColor3f(1.0,0.0,0.0);
    glLineWidth(1.0);

    float normal[2];
    normal[0] = cos(M_PI/180.0 * s->player->angle);
    normal[1] = sin(M_PI/180.0 * s->player->angle);
    
    
    glBegin(GL_LINES);
    glVertex3f(s->player->pos[0]+1, s->player->pos[1]-1, s->player->pos[2]+0.3);
    glVertex3f(s->player->pos[0]-1, s->player->pos[1]+1, s->player->pos[2]+0.3);
    glVertex3f(s->player->pos[0]+1, s->player->pos[1]+1, s->player->pos[2]+0.3);
    glVertex3f(s->player->pos[0]-1, s->player->pos[1]-1, s->player->pos[2]+0.3);

    glVertex3f(s->player->pos[0], s->player->pos[1], s->player->pos[2]+0.3);
    glVertex3f(s->player->pos[0]+normal[0], s->player->pos[1]+normal[1], s->player->pos[2]+0.3);


/*
    glVertex3fv(s->player->pos);
    glVertex3f(s->player->pos[0], s->player->pos[1], s->player->pos[2]+100);

    glVertex3f(0,0,3);
    glVertex3f(0,0,100);
*/  
    glEnd();
    
    glEnable(GL_DEPTH_TEST);
/*
    glPointSize(16);
    glBegin(GL_POINTS);
    glVertex3fv(s->player->pos);
    glVertex3f(40, 40, 0);
*/
/*
    glVertex3f(s->player->pos[0], s->player->pos[1], s->player->pos[2]+100);

    glVertex3f(0,0,3);
    glVertex3f(0,0,100);

    glVertex3f(40, 40, 5);
    glVertex3f(40, 40, 10);
  
    glEnd();
*/    

}

void render_markers_init()
{
    render_register(render_markers, RENDER_PASS_SOLID);    
}

