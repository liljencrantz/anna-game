#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <GL/glew.h>	
#include <math.h>	

#include "scene.h"
#include "render.h"
#include "util.h"
#include "ball.h"

#define BALL_COUNT(levels) (2*(0x55555555 & ((1<<(2*(levels+1)))-1)))
#define BALL_SCALE_THRESHOLD 1.05

float ball_normal[BALL_COUNT(BALL_LEVEL_MAX)][3];

int ball_p=0;
int ball_i=0;

static inline void plain_vertex(
    scene_t *s,
    int level, int x, int y, 
    ball_type_t *b,
    float *view_dir)
{
    int idx = ball_idx(level, x, y);
    ball_point_t *p=&b->data[idx];
    
    glColor3f(0.2,0.4,0.1);
    
    glNormal3f(
	ball_normal[idx][0],
	ball_normal[idx][1],
	ball_normal[idx][2]
	);    
    glVertex3f(
	ball_normal[idx][0]*p->radius,
	ball_normal[idx][1]*p->radius,
	ball_normal[idx][2]*p->radius
	);    
}

static inline void scale_vertex_sub(
    float f1, float f2, 
    int idx1, int idx2, int idx3,
    ball_type_t *b)
{
    float ff2 = 0.5*f2;

    float *n1 = ball_normal[idx1];
    float *n2 = ball_normal[idx2];
    float *n3 = ball_normal[idx3];

    float r1 = b->data[idx1].radius;
    float r2 = b->data[idx2].radius;
    float r3 = b->data[idx3].radius;
    
    glColor3f(0.2,0.4,0.1);
    glNormal3f(
	n1[0]*f1 + ff2*(n2[0]+n3[0]),
	n1[1]*f1 + ff2*(n2[1]+n3[1]),
	n1[2]*f1 + ff2*(n2[2]+n3[2]));
    
    glVertex3f(
	n1[0]*f1*r1 + ff2*(n2[0]*r2+n3[0]*r3),
	n1[1]*f1*r1 + ff2*(n2[1]*r2+n3[1]*r3),
	n1[2]*f1*r1 + ff2*(n2[2]*r2+n3[2]*r3)
	);    
}


static inline void scale_vertex1(
    float f1, float f2, 
    int level, int x, int y, 
    ball_type_t *b)
{
    int idx1 = ball_idx(level, x, y);
    int idx2 = ball_idx(level-1, x/2, y/2);
    
    float *n1 = ball_normal[idx1];
    float *n2 = ball_normal[idx2];
    
    float r1 = b->data[idx1].radius;
    float r2 = b->data[idx2].radius;
    
    glColor3f(0.2,0.4,0.1);
    glNormal3f(
	n1[0]*f1 + f2*n2[0],
	n1[1]*f1 + f2*n2[1],
	n1[2]*f1 + f2*n2[2]);
    glVertex3f(
	n1[0]*f1*r1 + n2[0]*f2*(r2),
	n1[1]*f1*r1 + n2[1]*f2*(r2),
	n1[2]*f1*r1 + n2[2]*f2*(r2)
	);
}

static inline void scale_vertex2(
    float f1, float f2, 
    int level, int x, int y, 
    ball_type_t *b)
{
    int idx1 = ball_idx(level, x, y);
    int idx2 = ball_idx(level-1, x/2, y/2);
    int idx3 = ball_idx(level-1, x/2, (y/2+1)%(1<<(level-1)));
    scale_vertex_sub(f1,f2,idx1,idx2,idx3,b);
}

static inline void scale_vertex3(
    float f1, float f2, 
    int level, int x, int y, 
    ball_type_t *b)
{    
    int idx1 = ball_idx(level, x, y);
    int idx2 = ball_idx(level-1, (x/2+1)%(1<<level), y/2);
    int idx3 = ball_idx(level-1, x/2, (y/2+1)%(1<<(level-1)));
    scale_vertex_sub(f1,f2,idx1,idx2,idx3,b);
}

static inline void scale_vertex4(
    float f1, float f2, 
    int level, int x, int y, 
    ball_type_t *b)
{
    int idx1 = ball_idx(level, x, y);
    int idx2 = ball_idx(level-1, x/2, y/2);
    int idx3 = ball_idx(level-1, (x/2+1)%(1<<level), y/2);
    scale_vertex_sub(f1,f2,idx1,idx2,idx3,b);    
}

void render_ball_at_level(ball_type_t *t, int level)
{
    int i, j;
    
    glColor4f(1,0,0,1);
    glBegin(GL_TRIANGLE_STRIP);
    
    int side_size = 1<<level;
    int side_size2 = 2<<level;

    for(j=1;j<side_size;j++)
    {
	for(i=0;i<side_size2;i++)
	{
	    plain_vertex(
		0, level, i, j-1,
		t, 0);
	    plain_vertex(
		0, level, i, j,
		t, 0);
	}	
    }
    plain_vertex(
	0, level, 0, side_size-1,
	t, 0);
    
    glEnd();
}


void render_ball(scene_t *s, ball_t *t)
{
    int level;

    int i, j;
    
    glPushMatrix();
    
    GLfloat view_dir[3];
    subtract(t->pos, s->camera.pos, view_dir, 3);

    GLfloat corr = render_height_correct(
	view_dir[0],
	view_dir[1]);
    float prev_error=2.0;
    float error;
    float distance_sq = dot_prod(view_dir, view_dir, 3);
    
    for(level=2; level < t->type->levels; level++)
    {
	error = t->scale*s->render_quality * t->type->error[level] / (0.0001 + sqrtf(distance_sq))*4;
	if(error < 1.0)
	{
	    break;
	}
	prev_error = error;
    }

    glTranslatef( t->pos[0], t->pos[1], t->pos[2]+corr);
    glRotatef( t->angle1, 0.0f, 0.0f, 1.0f );
    glRotatef( t->angle2, 0.0f, 1.0f, 0.0f );
    glRotatef( t->angle3, 1.0f, 0.0f, 0.0f );
    glTranslatef( t->offset[0], t->offset[1], t->offset[2]);
    glScalef(t->scale, t->scale, t->scale);
    glPointSize(5.0);
    
    glColor4f(1,0,0,1);
    
    int side_size = 1<<level;
    int side_size2 = 2<<level;

    s->triangle_count += 2*(1<<level)*(2<<level);
    
    if(prev_error < BALL_SCALE_THRESHOLD)
    {
	glBegin(GL_TRIANGLE_STRIP);
	float f1 = (prev_error - 1.0)/(BALL_SCALE_THRESHOLD-1.0);
	float f2 = (1.0 - f1);
	ball_i++;


	for(j=1;j<(side_size-1);j++)
	{    
	    for(i=0;i<side_size2;i+=2)
	    {		
		scale_vertex1(
		    f1, f2, level, i, j-1, t->type);
		scale_vertex2(
		    f1, f2, level, i, j, t->type);
		scale_vertex4(
		    f1, f2, level, i+1, j-1, t->type);
		scale_vertex3(
		    f1, f2, level, i+1, j, t->type);
	    }
	    j++;
	    for(i=0;i<side_size2;i+=2)
	    {
		scale_vertex2(
		    f1, f2, level, i, j-1, t->type);
		scale_vertex1(
		    f1, f2, level, i, j, t->type);
		scale_vertex3(
		    f1, f2, level, i+1, j-1, t->type);
		scale_vertex4(
		    f1, f2, level, i+1, j, t->type);
	    }
	}	
	glEnd();
    }
    else
    {
	ball_p++;
	glCallList(t->type->list_index + level);
    }
    
    glPopMatrix();

}

void render_balls(scene_t *s)
{
    int i;
    glEnable( GL_CULL_FACE );	
    glEnable(GL_LIGHTING);
    glEnable(GL_RESCALE_NORMAL);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHT0);


// Create light components
    float ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    float diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
    
// Assign created components to GL_LIGHT0
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    //glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
    glEnable ( GL_COLOR_MATERIAL ) ;

    int count = 0;    
    for(i=0;; i++)
    {
	ball_t *ball = scene_ball_get(s, i);
	if(ball)
	{
	    count++;
	    ball->visible = scene_is_visible(s,ball->pos, ball->scale);
	    if(ball->visible)
		render_ball(s, ball);
	    if(count >= scene_ball_get_count(s))
		break;
	}
    }
    
    glDisable(GL_LIGHT0);
    glDisable( GL_CULL_FACE );
    glDisable(GL_LIGHTING);

//    printf("%.2f\n", (float)ball_i/(ball_i+ball_p));
    
}

void render_balls_init()
{
    
    int i, j, level;
    for(level=0; level < BALL_LEVEL_MAX; level++)
    {
	int side_size = 1<<level;
	int side_size2 = 2<<level;
	printf("Wee prepare level %d, with size %d x %d\n", level, side_size2, side_size);
//	printf("Level %d starts at offset %d, ends at offset %d\n", ball_point_count(level-1), ball_point_count(level));
	
	
	for(i=0;i<side_size2;i++)
	{
	    for(j=0;j<side_size;j++)
	    {
		ball_normal[ball_idx(level, i, j)][0] = sin(M_PI * j / (side_size-1)) * cos(M_PI * 2 * i / side_size2);
		ball_normal[ball_idx(level, i, j)][1] = sin(M_PI * j / (side_size-1)) * sin(M_PI * 2 * i / side_size2);
		ball_normal[ball_idx(level, i, j)][2] = cos(M_PI * j / (side_size-1));
	    }	
	}
    }    
    
}


