#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <GL/gl.h>	
#include <math.h>	

#include "scene.h"
#include "render.h"
#include "util.h"
#include "ball.h"

/*
Before: 
31.3 fps
render_ball: 42.9% CPU

 */
#define BALL_COUNT(levels) (2*(0x55555555 & ((1<<(2*(levels+1)))-1)))
#define SCALE_THRESHOLD 1.1

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
    
    float c = fabs(dot_prod(view_dir, ball_normal[idx], 3));
    c = s->ambient_light + c*(s->sun_light-s->ambient_light);
    
    glColor3f(c*0.2,c*0.4,c*0.1);
    
    glVertex3f(
	ball_normal[idx][0]*p->radius,
	ball_normal[idx][1]*p->radius,
	ball_normal[idx][2]*p->radius
	);    
}

static inline void scale_vertex_sub(
    scene_t *s,
    float f1, float f2, 
    int idx1, int idx2, int idx3,
    ball_type_t *b,
    float *view_dir)
{
    float ff2 = 0.5*f2;

    float *n1 = ball_normal[idx1];
    float *n2 = ball_normal[idx2];
    float *n3 = ball_normal[idx3];

    float r1 = b->data[idx1].radius;
    float r2 = b->data[idx2].radius;
    float r3 = b->data[idx3].radius;
    
    float c1 = fabs(dot_prod(view_dir, ball_normal[idx1], 3));
    float c2 = fabs(dot_prod(view_dir, ball_normal[idx2], 3));
    float c3 = fabs(dot_prod(view_dir, ball_normal[idx3], 3));

    float c = f1*c1 + ff2*(c2+c3);
    c = s->ambient_light + c*(s->sun_light-s->ambient_light);
    glColor3f(c*0.2,c*0.4,c*0.1);
    
    glVertex3f(
	n1[0]*f1*r1 + ff2*(n2[0]*r2+n3[0]*r3),
	n1[1]*f1*r1 + ff2*(n2[1]*r2+n3[1]*r3),
	n1[2]*f1*r1 + ff2*(n2[2]*r2+n3[2]*r3)
	);    
}


static inline void scale_vertex1(
    scene_t *s,
    float f1, float f2, 
    int level, int x, int y, 
    ball_type_t *b,
    float *view_dir)
{
    int idx1 = ball_idx(level, x, y);
    int idx2 = ball_idx(level-1, x/2, y/2);


    float *n1 = ball_normal[idx1];
    float *n2 = ball_normal[idx2];

    float r1 = b->data[idx1].radius;
    float r2 = b->data[idx2].radius;
    
    float c1 = fabs(dot_prod(view_dir, ball_normal[idx1], 3));
    float c2 = fabs(dot_prod(view_dir, ball_normal[idx2], 3));

    float c = f1*c1 + f2*(c2);
    c = s->ambient_light + c*(s->sun_light-s->ambient_light);
    glColor3f(c*0.2,c*0.4,c*0.1);
    
    glVertex3f(
	n1[0]*f1*r1 + n2[0]*f2*(r2),
	n1[1]*f1*r1 + n2[1]*f2*(r2),
	n1[2]*f1*r1 + n2[2]*f2*(r2)
	);
}

static inline void scale_vertex2(
    scene_t *s,
    float f1, float f2, 
    int level, int x, int y, 
    ball_type_t *b,
    float *view_dir)
{
    int idx1 = ball_idx(level, x, y);
    int idx2 = ball_idx(level-1, x/2, y/2);
    int idx3 = ball_idx(level-1, x/2, (y/2+1)%(1<<(level-1)));
    
    scale_vertex_sub(s, f1,f2,idx1,idx2,idx3,b,view_dir);
}

static inline void scale_vertex3(
    scene_t *s,
    float f1, float f2, 
    int level, int x, int y, 
    ball_type_t *b,
    float *view_dir)
{
    
    int idx1 = ball_idx(level, x, y);
    int idx2 = ball_idx(level-1, (x/2+1)%(1<<level), y/2);
    int idx3 = ball_idx(level-1, x/2, (y/2+1)%(1<<(level-1)));

    scale_vertex_sub(s, f1,f2,idx1,idx2,idx3,b,view_dir);
}

static inline void scale_vertex4(
    scene_t *s,
    float f1, float f2, 
    int level, int x, int y, 
    ball_type_t *b,
    float *view_dir)
{
    int idx1 = ball_idx(level, x, y);
    int idx2 = ball_idx(level-1, x/2, y/2);
    int idx3 = ball_idx(level-1, (x/2+1)%(1<<level), y/2);

    scale_vertex_sub(s, f1,f2,idx1,idx2,idx3,b,view_dir);    
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
    float distance_sq = dot_prod(view_dir, view_dir, 3);    
    float prev_error=2.0;
    float error;
    
    for(level=2; level < t->type->levels; level++)
    {
	error = t->scale*s->render_quality * t->type->error[level] / (0.0001 + sqrtf(distance_sq))*2;
//	printf("%.2f\n", error);
	
	if(error < 1.0)
	{
	    break;
	}
	prev_error = error;
    }
    
    normalize(view_dir, view_dir, 3);
    rotate_z(view_dir, -t->angle1*M_PI/180);
    rotate_y(view_dir, -t->angle2*M_PI/180);
//    rotate_x(view_dir, -t->angle3*M_PI/180);
    
    glTranslatef( t->pos[0], t->pos[1], t->pos[2]+corr);
    glRotatef( t->angle1, 0.0f, 0.0f, 1.0f );
    glRotatef( t->angle2, 0.0f, 1.0f, 0.0f );
    glRotatef( t->angle3, 1.0f, 0.0f, 0.0f );
    glTranslatef( t->offset[0], t->offset[1], t->offset[2]);
    glScalef(t->scale, t->scale, t->scale);
    glPointSize(5.0);
    
    glColor4f(1,0,0,1);
    glBegin(GL_TRIANGLE_STRIP);
    
    int side_size = 1<<level;
    int side_size2 = 2<<level;

    if(prev_error < SCALE_THRESHOLD)
    {
	float f1 = (prev_error - 1.0)/(SCALE_THRESHOLD-1.0);
	float f2 = (1.0 - f1);
	ball_i++;
	
	for(j=1;j<(side_size-1);j++)
	{

	    for(i=0;i<side_size2;i+=2)
	    {		
		scale_vertex1(
		    s, f1, f2, level, i, j-1, t->type, view_dir);
		scale_vertex2(
		    s, f1, f2, level, i, j, t->type, view_dir);
		scale_vertex4(
		    s, f1, f2, level, i+1, j-1, t->type, view_dir);
		scale_vertex3(
		    s, f1, f2, level, i+1, j, t->type, view_dir);
	    }	

	    j++;
	    for(i=0;i<side_size2;i+=2)
	    {
		scale_vertex2(
		    s, f1, f2, level, i, j-1, t->type, view_dir);
		scale_vertex1(
		    s, f1, f2, level, i, j, t->type, view_dir);
		scale_vertex3(
		    s, f1, f2, level, i+1, j-1, t->type, view_dir);
		scale_vertex4(
		    s, f1, f2, level, i+1, j, t->type, view_dir);
	    }
	}	
    }
    else
    {
	ball_p++;
	for(j=1;j<side_size;j++)
	{
	    for(i=0;i<side_size2;i++)
	    {
		plain_vertex(
		    s, level, i, j-1,
		    t->type, view_dir);
		plain_vertex(
		    s, level, i, j,
		    t->type, view_dir);
	    }	
	}
	plain_vertex(
	    s, level, 0, side_size-1,
	    t->type, view_dir);

    }
    
    glEnd();    
    
    glPopMatrix();

}

void render_balls(scene_t *s)
{

    int i;
    glEnable( GL_CULL_FACE );	
    
    for(i=0; i<scene_ball_get_count(s); i++)
    {
	ball_t *ball = scene_ball_get(s, i);
	ball->visible = scene_is_visible(s,ball->pos, ball->scale);
	if(ball->visible)
	    render_ball(s, ball);
    }
    glDisable( GL_CULL_FACE );

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
    
    render_register(render_balls, RENDER_PASS_SOLID);    
    
}


