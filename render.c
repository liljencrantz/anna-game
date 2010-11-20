#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

#include <GL/glew.h>	
#include <GL/glu.h>	

#include "util.h"
#include "common.h"
#include "render.h"
#include "screen.h"
#include "thread.h"

void render_init()
{
    thread_set_render();    
    glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
    glDepthFunc(GL_LEQUAL);				// The Type Of Depth Test To Do
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glMatrixMode(GL_PROJECTION);
    
    gluPerspective(
	70.0*16/10,
	16.0/10.0,
	1.0f,
	3000.0f);	// Calculate The Aspect Ratio Of The Window
    
    glMatrixMode(GL_MODELVIEW);
    
    glClearColor(0.3,0.3,5.0,1.0);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); 
    glDisable(GL_BLEND);
//  glBlendFunc( GL_ONE, GL_ONE);
    
    render_terrain_init();
//    render_trees_init();    
//    render_markers_init();    
    render_balls_init();    
}

static GLfloat sign(GLfloat v)
{
    return v>0.0?1.0:-1.0;
}


static void calc_pov( scene_t *s )
{
	int i;
	GLfloat angle_1=s->camera.lr_rot+85;
	GLfloat angle_2=s->camera.lr_rot-85;
	GLfloat angle_3=s->camera.lr_rot+90;
	GLfloat plane[3][3];
	
	GLfloat mpos[3];
//	printf("%.2f\n", s->camera.lr_rot);
	
	mpos[0] = s->camera.pos[0] - 1.0*cos(s->camera.lr_rot*M_PI/180.0f )*15.0;
	mpos[1] = s->camera.pos[1] - 1.0*sin(s->camera.lr_rot*M_PI/180.0f )*15.0;
	mpos[2] = s->camera.pos[2];
	
	plane[0][0]=mpos[0] + cos( angle_1*M_PI/180.0f );
	plane[0][1]=mpos[1] + sin( angle_1*M_PI/180.0f );
	plane[0][2]=mpos[2];
	
	plane[1][0]=mpos[0] + cos( angle_2*M_PI/180.0f );
	plane[1][1]=mpos[1] + sin( angle_2*M_PI/180.0f );
	plane[1][2]=mpos[2];
	
	plane[2][0]=mpos[0] + cos( angle_3*M_PI/180.0f );
	plane[2][1]=mpos[1] + sin( angle_3*M_PI/180.0f );
	plane[2][2]=mpos[2];
	
	for( i=0; i<3; i++ )
	{
	    float divisor = (plane[i][0]-mpos[0]);
	    divisor = fabs(divisor)<0.0000001?0.01*sign(divisor):divisor;
	    s->camera.k[i] = (plane[i][1]-mpos[1])/divisor;
	    s->camera.m[i] = mpos[1] -mpos[0]*s->camera.k[i];
	    s->camera.side[i] = plane[i][0]>mpos[0];
	}
	s->camera.side[0] = !s->camera.side[0];
	s->camera.side[1] = s->camera.side[1];
	s->camera.side[2] = !s->camera.side[2];
//	s->camera.side[2] = !s->camera.side[2];
	

}

static void render_setup_camera(scene_t *s)
{
    GLfloat view_dir[4];
    view_dir[0] = 0;//s->camera.pos[0];
    view_dir[1] = 0;//s->camera.pos[1];
    view_dir[2] = 0;//s->camera.pos[2];
    view_dir[3] = 1;
    glLightfv(GL_LIGHT0, GL_POSITION, view_dir);
    #ifdef DRAW
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glLoadIdentity();
//	render_hud( s );
	if(0)
	{
	    /* 
	       Alternative camera view, shows much more overhead view,
	       but rendered with detail suitable for normal
	       view. Suitable for render debugging.
	    */
	    glRotatef( -90.0f, 1.0f, 0.0f, 0.0f );
	    glRotatef( 90, 1.0f, 0.0f, 0.0f );
//	    glRotatef( s->camera.s_rot, 0.0f, 1.0f, 0.0f );
//	    glRotatef( s->camera.lr_rot, 0.0f, 0.0f, 1.0f );
//	    glTranslatef( -s->camera.pos[0], -s->camera.pos[1], -(s->camera.pos[2]+150) );
	    glTranslatef( -s->camera.pos[0], -s->camera.pos[1], -(s->camera.pos[2])-30 );
//	    glTranslatef( -100, -100, -50 );
	}
	else
	{
	    /* Normal view */
	    glRotatef( -90.0f, 1.0f, 0.0f, 0.0f );
	    glRotatef( s->camera.ud_rot, 1.0f, 0.0f, 0.0f );
	    //glRotatef( s->camera.s_rot, 0.0f, 1.0f, 0.0f );
	    glRotatef( -s->camera.lr_rot+90, 0.0f, 0.0f, 1.0f );	    
	    glTranslatef( -s->camera.pos[0], -s->camera.pos[1], -s->camera.pos[2] );
/*	    
   printf( "LALALA angle %.2f, pos %.2f %.2f\n", 
		    s->camera.lr_rot,
		    s->camera.pos[0], s->camera.pos[1]);
/**/	    
	}
#endif	
	calc_pov( s );
}


void render( scene_t *s )
{
    
    scene_update(s);

    s->triangle_count = 0;

    glPushMatrix();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_setup_camera(s);
    render_terrain_start(s);

    render_trees_trunk(s);
    render_boids(s);
    render_terrain_finish(s);
    render_balls(s);
    
    glPopMatrix();
//    printf("Rendered %d tiangles\n", s->triangle_count);
   
}

