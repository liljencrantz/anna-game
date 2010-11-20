#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <GL/glew.h>	
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

void render_tree_trunk(scene_t *s, tree_t *t)
{

    glPushMatrix();

    GLfloat view_dir[3];
    GLfloat *t_pos = &t->transform[12];
    subtract(t_pos, s->camera.pos, view_dir, 3);
    GLfloat corr = render_height_correct(view_dir[0],
					 view_dir[1]);
    
//    view_dir[2]-= corr;
    
    glMultMatrixf(t->transform);
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
    return;
/*    
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
*/      
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
    int count=0;
    
    if(scene_tree_get_count(s))
    {
	for(i=0;; i++)
	{
	    tree_t *tree = scene_tree_get(s, i);
	    
	    if(tree)
	    {
		count++;
		if(tree->type)
		{
//		    printf("We have a tree to render!\n");

		    GLfloat *t_pos = &tree->transform[12];
		    tree->visible = scene_is_visible(s,t_pos, tree->scale);
		    if(tree->visible)
			render_tree_trunk(s, tree);
		}
		if(count >= scene_tree_get_count(s))
		    break;
		
	    }
	}
	
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
    
}

