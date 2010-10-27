#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <GL/gl.h>	
#include <math.h>	

#include "scene.h"
#include "render.h"
#include "util.h"
#include "tree.h"


GLfloat point_offset[32][3];
int color_offset[32][3];
tree_t tree[32];



void render_section(tree_section_t *sec, GLfloat *view_dir)
{
    GLfloat side_dir[3];

    cross_prod(view_dir, sec->normal, side_dir);
    multiply_s(side_dir, 0.5*sec->width, side_dir, 3);
    
    if(DRAW)
    {

//    glBindTexture( GL_TEXTURE_2D, sec->texture_id );

	int i;
	
	for(i=0; i<sec->type->count; i++)
	{
	    float f = 1.0-sec->type->data[i][0];
	    float px = sec->pos[0] + 
		sec->length*sec->normal[0]*f;

	    float py = sec->pos[1] + 
		sec->length*sec->normal[1]*f;
	    
	    float pz = sec->pos[2] +
		sec->length*sec->normal[2]*f;
	    
	    glVertex3f(
		px+side_dir[0]*sec->type->data[i][1], 
		py+side_dir[1]*sec->type->data[i][1], 
		pz+side_dir[2]*sec->type->data[i][1]);
	    
	    glVertex3f(
		px-side_dir[0]*sec->type->data[i][2], 
		py-side_dir[1]*sec->type->data[i][2], 
		pz-side_dir[2]*sec->type->data[i][2]);
	}

    }
    
}

void render_tree_leaves(scene_t *s, tree_t *t)
{
    return;
    
    float middle[]={988,843};
    
    float vec[][2]={
	{682,1285},
	{597,1183},
	{514,1192},
	{423,1140},
	{406,1014},
	{462,984},
	{388,898},
	{436,859},
	{412,775},
	{465,730},
	{471,669},
	{406,634},
	{393,532},
	{513,484},
	{574,528},
	{636,528},
	{699,444},
	{822,459},
	{903,567},
	{912,493},
	{1039,480},
	{1047,523},
	{1158,537},
	{1158,570},
	{1219,570},
	{1234,639},
	{1260,636},
	{1369,802},
	{1329,847},
	{1363,955},
	{1327,1008},
	{1365,1120},
	{1269,1245},
	{1327,1294},
	{1212,1405},
	{1206,1495},
	{1066,1518},
	{973,1464},
	{898,1345},
	{909,1203},
	{790,1104}
    };
    

    int vec_count = sizeof(vec)/sizeof(*vec);
    float scale = 0.0042;
    
    
    glPushMatrix();
    GLfloat view_dir[3];
    subtract(t->pos, s->camera.pos, view_dir, 3);
    GLfloat corr = render_height_correct(view_dir[0],
					 view_dir[1]);
    
//    view_dir[2]-= corr;
    
    glTranslatef( t->pos[0], t->pos[1]-0.0, t->pos[2]+corr+ 3.3);
    glRotatef(t->angle, 0,0,1);
    rotate_z(view_dir, -t->angle*M_PI/180);
    //normalize(view_dir, view_dir, 3);
    glColor4f(0.2,0.4,0.1,1);
    
    glBegin( GL_TRIANGLE_FAN );
    float up[]=
	{
	    0,0,1
	}
    ;
    float side[3];
    cross_prod(view_dir, up, side);
    normalize(side,side,3);
    
    int i;
    glVertex3f(-3.8*side[1],3.8*side[0],0);

#define LALA 0.0
    for(i=0; i<vec_count; i++)
    {
	float y = LALA * side[0] - (vec[i][0]-middle[0])*scale*side[1];
	float x = -LALA * side[1] - (vec[i][0]-middle[0])*scale*side[0];
	float z = (middle[1]-vec[i][1])*scale;
	glVertex3f(x,y,z);
    }    
    {
	float y = LALA * side[0] - (vec[0][0]-middle[0])*scale*side[1];
	float x = -LALA * side[1] - (vec[0][0]-middle[0])*scale*side[0];
	float z = (middle[1]-vec[0][1])*scale;
	glVertex3f(x,y,z);
    }
    glEnd();
    
    glPopMatrix();
    
}


void render_tree_trunk(scene_t *s, tree_t *t)
{

    glPushMatrix();

    GLfloat view_dir[3];
    subtract(t->pos, s->camera.pos, view_dir, 3);
    GLfloat corr = render_height_correct(view_dir[0],
					 view_dir[1]);
    
//    view_dir[2]-= corr;
    
    glTranslatef( t->pos[0], t->pos[1], t->pos[2]+corr);
    glRotatef(t->angle, 0,0,1);
    glColor3f(0.2,0.2,0.05);

    //printf("LALA %.2f\n", t->angle);
    
#define NORMALIZE_SECTION_DIST 00.0
    
    GLfloat distance_sq = view_dir[0]*view_dir[0] + view_dir[1]*view_dir[1];
    if( distance_sq >= NORMALIZE_SECTION_DIST )
    {
	view_dir[2] += 2.5;	
	normalize(view_dir, view_dir, 3);
	rotate_z(view_dir, -t->angle*M_PI/180);
    }
    
//    w/distance > k * quality
//    w > k*q*dst
//    W*w > k*k*q*q*dst*dst
    int i;
    glBegin( GL_TRIANGLE_STRIP );
    for(i=0; i<t->type->section_count; i++)
    {
#define TREE_QUALITY_FACTOR 0.05
	if(!t->type->section[i].type)
	{
	    glEnd();	    
	    glBegin( GL_TRIANGLE_STRIP );
	    continue;
	}
	
	GLfloat view_dir2[3];
	GLfloat width_sq = t->type->section[i].width*t->type->section[i].width;

	//printf("%f < %f ?\n",width_sq*s->render_quality*s->render_quality,distance_sq*TREE_QUALITY_FACTOR);
    
	if(width_sq*s->render_quality*s->render_quality < distance_sq*TREE_QUALITY_FACTOR)
	{
	    continue;
	}
	
	if( distance_sq < NORMALIZE_SECTION_DIST)
	{
	    add(view_dir, t->type->section[i].pos, view_dir2, 3);
	    normalize(view_dir2, view_dir2, 3);
	    rotate_z(view_dir, -t->angle*M_PI/180);
	    render_section(&t->type->section[i], view_dir2);
	}
	else
	{
	    render_section(&t->type->section[i], view_dir);
	}
    }
	    glEnd();
    
    glPopMatrix();

}

GLfloat rnd()
{
    return sqrt((float)rand() / RAND_MAX) *  ((rand() > (RAND_MAX/2))?1.0:-1.0);    
}

int rndc()
{
    return 0;//rand()%16 + 8;
}



void render_trees_leaves(scene_t *s)
{
        
    glShadeModel( GL_FLAT );
    
    int i;

    s->leaf_offset = 0.8+0.2*sin(2.0 * s->time);
    
    for(i=0; i<scene_tree_get_count(s); i++)
    {
	tree_t *tree = scene_tree_get(s, i);

	tree->visible = scene_is_visible(s,tree->pos, 3);
	if(tree->visible)
	{
	   render_tree_leaves(s, tree);
	}
    }

    glShadeModel(GL_SMOOTH);
        
}

void render_trees_trunk(scene_t *s)
{

//    glEnable( GL_TEXTURE_2D );
//    glEnable( GL_ALPHA_TEST );
    glDisable( GL_BLEND );
//    glEnable( GL_BLEND );

//    glDepthMask( GL_FALSE );
    glShadeModel( GL_FLAT );


    int i;
    
    for(i=0; i<scene_tree_get_count(s); i++)
    {
	tree_t *tree = scene_tree_get(s, i);
	if(tree->visible)
	    render_tree_trunk(s, tree);
    }
    
    //  glDepthMask( GL_TRUE );
//    glDisable(GL_TEXTURE_2D);
//    glDisable( GL_ALPHA_TEST );
    glDisable( GL_BLEND );
    glShadeModel(GL_SMOOTH);
    
}

void render_trees_init()
{
    
    int i;
    for(i=0; i<32; i++)
    {
	point_offset[i][0] = rnd();
	point_offset[i][1] = rnd();
	point_offset[i][2] = rnd();
	
	color_offset[i][0] = rndc();
	color_offset[i][1] = rndc();
	color_offset[i][2] = rndc();
//	printf("%d\n", color_offset[i][2]);
    }
    
    render_register(render_trees_trunk, RENDER_PASS_SOLID);    
    render_register(render_trees_leaves, RENDER_PASS_SOLID);    
    
}

