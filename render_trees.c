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
    glBindTexture( GL_TEXTURE_2D, sec->texture_id );
    glBegin( GL_TRIANGLE_STRIP );

//    glColor3f(0.0,0.0,0.0);
	
    glTexCoord2f( 0.0, 1.0 );
    glVertex3f(
	sec->pos1[0]+side_dir[0], 
	sec->pos1[1]+side_dir[1], 
	sec->pos1[2]+side_dir[2]);

    glTexCoord2f( 1.0, 1.0 );
    glVertex3f(
	sec->pos1[0]-side_dir[0], 
	sec->pos1[1]-side_dir[1], 
	sec->pos1[2]-side_dir[2]);
    
    glTexCoord2f( 0.0, 0.0 );
    glVertex3f(
	sec->pos2[0]+side_dir[0], 
	sec->pos2[1]+side_dir[1],
	sec->pos2[2]+side_dir[2]);

    glTexCoord2f( 1.0, 0.0 );
    glVertex3f(
	sec->pos2[0]-side_dir[0],
	sec->pos2[1]-side_dir[1],
	sec->pos2[2]-side_dir[2]);
    glEnd();
    }
    
}

void render_joint(tree_section_t *sec, GLfloat *view_dir, int simple)
{
    GLfloat dir1[3];
    GLfloat dir2[3];

    if(simple)
    {
	dir1[0]=dir1[1]=0.0;
	dir1[2] = 1.0;
    }
    else
    {
	if( view_dir[0] < view_dir[1])
	{
	    dir1[0] = 0.0;
	    dir1[1] = view_dir[2];
	    dir1[2] = -view_dir[1];
	}
	else
	{
	    dir1[0] = -view_dir[2];
	    dir1[1] = 0.0;
	    dir1[2] = view_dir[0];
	}
	
	normalize(dir1, dir1, 3);
    }
    
    multiply_s(dir1, 0.5*sec->width, dir1, 3);
    cross_prod(view_dir, dir1, dir2);
    
    if(DRAW)
    {
    glBindTexture( GL_TEXTURE_2D, sec->texture_id );
    glBegin( GL_TRIANGLE_STRIP );
    
    glTexCoord2f( 0.0, 0.0 );
    glVertex3f(
	sec->pos1[0]+dir1[0]-dir2[0], 
	sec->pos1[1]+dir1[1]-dir2[1], 
	sec->pos1[2]+dir1[2]-dir2[2]);

    glTexCoord2f( 0.0, 1.0 );
    glVertex3f(
	sec->pos1[0]-dir1[0]-dir2[0], 
	sec->pos1[1]-dir1[1]-dir2[1], 
	sec->pos1[2]-dir1[2]-dir2[2]);
    
    glTexCoord2f( 1.0, 0.0 );
    glVertex3f(
	sec->pos1[0]+dir1[0]+dir2[0], 
	sec->pos1[1]+dir1[1]+dir2[1], 
	sec->pos1[2]+dir1[2]+dir2[2]);
    
    glTexCoord2f( 1.0, 1.0 );
    glVertex3f(
	sec->pos1[0]-dir1[0]+dir2[0], 
	sec->pos1[1]-dir1[1]+dir2[1], 
	sec->pos1[2]-dir1[2]+dir2[2]);
    
    glEnd();
    }
    
}

#define LEAF_LENGTH 0.3
#define LEAF_MID 0.05
#define LEAF_MID2 0.13
#define LEAF_WIDTH 0.15
#define LEAF_WIDTH2 0.18

void render_leaves(
    tree_section_points_t *sec, 
    GLfloat distance_sq, 
    GLfloat offset)
{
    
    
    glDisable( GL_TEXTURE_2D );
    int i, j;
    sec->cloud_width = 0.2;
    
    int high_count=0, mid_count=0, low_count = sec->count;;
    
#define DISTANCE_HIGH_FULL 20.0
#define DISTANCE_HIGH_SCALE 45.0
#define DISTANCE_MID_FULL 300.0
#define DISTANCE_MID_SCALE 600.0

    if (distance_sq < DISTANCE_HIGH_SCALE)
    {
	high_count = (1.0 - maxf(0.0, distance_sq-DISTANCE_HIGH_FULL)/(DISTANCE_HIGH_SCALE - DISTANCE_HIGH_FULL)) * sec->count;
	mid_count = sec->count;
    }
    else 
    {
	mid_count = (1.0 - maxf(0.0, distance_sq-DISTANCE_MID_FULL)/(DISTANCE_MID_SCALE - DISTANCE_MID_FULL)) * sec->count;
    }
	
    for(i=0; i<high_count; i++)
    {
		
	glBegin( GL_TRIANGLE_STRIP );

	GLfloat f1[]=
	    {
		0.0, 0.5*LEAF_WIDTH, -0.5*LEAF_WIDTH,
		0.5*LEAF_WIDTH2, -0.5*LEAF_WIDTH2, 0.0
	    };
	
	GLfloat f2[]=
	    {
		0.0, LEAF_MID, LEAF_MID, LEAF_MID2, LEAF_MID2, LEAF_LENGTH
	    };
	
	GLfloat zf[]=
	    {
		0.0, -0.05, -0.05, -0.2, -0.2, -1.0
	    };


	GLfloat base_pos[] = 
	    {
		sec->pos[0]+point_offset[i][0]*sec->cloud_width,
		sec->pos[1]+point_offset[i][1]*sec->cloud_width,
		sec->pos[2]+point_offset[i][2]*sec->cloud_width
	    }
	;
	
	for(j=0; j<6; j++)
	{
	    GLfloat zfactor = zf[j]*offset;
	    
	    glColor3ub(
		(1.0 + 0.4*zfactor)*sec->color[0]+color_offset[i][0], 
		(1.0 + 0.4*zfactor)*sec->color[1]+color_offset[i][1], 
		(1.0 + 0.4*zfactor)*sec->color[2]+color_offset[i][2]);
    if(DRAW)
    {

	    glVertex3f(
		base_pos[0]+sec->normal[0]*f2[j] + sec->normal[1]*f1[j], 
		base_pos[1]+sec->normal[0]*f1[j] + sec->normal[1]*f2[j], 
		base_pos[2]+zfactor*0.4
		);
    }
    
	}
	glEnd();

    }

    if(mid_count > high_count)
    {
	glBegin( GL_TRIANGLES );
	for(; i<mid_count; i++)
	{
	
	    GLfloat f1[]=
		{
		    0.7*LEAF_WIDTH, -0.7*LEAF_WIDTH, 0.0
		};
	    
	    GLfloat f2[]=
		{
		    0.1*LEAF_LENGTH, 0.1*LEAF_LENGTH, LEAF_LENGTH
		};
	    
	    GLfloat zf[]=
		{
		    0.0, 0.0, -1.0
		};
	    
	    
	    GLfloat base_pos[] = 
		{
		    sec->pos[0]+point_offset[i][0]*sec->cloud_width,
		    sec->pos[1]+point_offset[i][1]*sec->cloud_width,
		    sec->pos[2]+point_offset[i][2]*sec->cloud_width
		}
	    ;
	    
	    for(j=0; j<3; j++)
	    {
		GLfloat zfactor = zf[j]*offset;
		
		glColor3ub(
		    (1.0 + 0.4*zfactor)*sec->color[0]+color_offset[i][0], 
		    (1.0 + 0.4*zfactor)*sec->color[1]+color_offset[i][1], 
		    (1.0 + 0.4*zfactor)*sec->color[2]+color_offset[i][2]);
		if(DRAW)
		{
		    
		    glVertex3f(
			base_pos[0]+sec->normal[0]*f2[j] + sec->normal[1]*f1[j], 
			base_pos[1]+sec->normal[0]*f1[j] + sec->normal[1]*f2[j], 
			base_pos[2]+zfactor*0.4
			);	    
		}
	    }
	    
	}
	glEnd();
    }
    
    if(low_count > mid_count)
    {
	
	glPointSize(3.0);
	glBegin( GL_POINTS );
	for(; i<low_count; i++)
	{	
	
	    GLfloat base_pos[] = 
		{
		    sec->pos[0]+point_offset[i][0]*sec->cloud_width,
		    sec->pos[1]+point_offset[i][1]*sec->cloud_width,
		    sec->pos[2]+point_offset[i][2]*sec->cloud_width
		}
	    ;
	    if(DRAW)
	    {
		glColor3ub(
		    0.8*sec->color[0]+color_offset[i][0], 
		    0.8*sec->color[1]+color_offset[i][1], 
		    0.8*sec->color[2]+color_offset[i][2]);
		glVertex3fv(base_pos);
	    }
	    
	}
	glEnd();
    }
    

//    printf("%.2f\n", sec->pos[2]);
    
    
    glEnable( GL_TEXTURE_2D );
    
}




void render_tree_leaves(scene_t *s, tree_t *t)
{
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
    
    glTranslatef( t->pos[0], t->pos[1]-0.6, t->pos[2]+corr+ 3.3);
    glRotatef(t->angle, 0,0,1);
    rotate_z(view_dir, -t->angle*M_PI/180);
    glColor4f(0.2,0.4,0.1,1);
    
    glBegin( GL_TRIANGLE_FAN );
    
    int i;
    glVertex3f(0.4,0,0);

    for(i=0; i<vec_count; i++)
    {
	float x = 0.4;
	float y = -(vec[i][0]-middle[0])*scale;
	float z = (middle[1]-vec[i][1])*scale;
	glVertex3f(x,y,z);
    }    
    glEnd();
    
    glPopMatrix();
    
}

void render_tree_leavesaaa(scene_t *s, tree_t *t)
{
    
    glPushMatrix();

    GLfloat view_dir[3];
    subtract(t->pos, s->camera.pos, view_dir, 3);
    GLfloat corr = render_height_correct(view_dir[0],
					 view_dir[1]);
    
//    view_dir[2]-= corr;
    
    glTranslatef( t->pos[0], t->pos[1], t->pos[2]+corr);
    glRotatef(t->angle, 0,0,1);
    rotate_z(view_dir, -t->angle*M_PI/180);
    
    GLfloat distance_sq = view_dir[0]*view_dir[0] + view_dir[1]*view_dir[1];
//    w/distance > k * quality
//    w > k*q*dst
//    W*w > k*k*q*q*dst*dst
    int i;
    for(i=0; i<t->type->section_count; i++)
    {
#define TREE_QUALITY_FACTOR 0.05
	GLfloat view_dir2[3];
	GLfloat width_sq = t->type->section[i].width*t->type->section[i].width;
	if(t->type->section[i].type == TREE_SECTION_JOINT)
	    width_sq /= 30;

	//printf("%f < %f ?\n",width_sq*s->render_quality*s->render_quality,distance_sq*TREE_QUALITY_FACTOR);
    
	if(width_sq*s->render_quality*s->render_quality < distance_sq*TREE_QUALITY_FACTOR)
	    continue;
	
	switch(t->type->section[i].type)
	{
	    case TREE_SECTION_REGULAR:
		break;
	    case TREE_SECTION_JOINT:
		break;

	    case TREE_SECTION_POINTS:
		
		render_leaves((tree_section_points_t *)&t->type->section[i], distance_sq, s->leaf_offset );
		break;

	    default:
		printf("Don't know how to render tree section of type %d\n", 
		       t->type->section[i].type);
		exit(1);
		
	}
	
    }
    
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

#define NORMALIZE_SECTION_DIST 200.0

    
    GLfloat distance_sq = view_dir[0]*view_dir[0] + view_dir[1]*view_dir[1];
    if(distance_sq >= NORMALIZE_SECTION_DIST )
    {
	view_dir[2] += 2.5;	
	normalize(view_dir, view_dir, 3);
	rotate_z(view_dir, -t->angle*M_PI/180);
    }
    
//    w/distance > k * quality
//    w > k*q*dst
//    W*w > k*k*q*q*dst*dst
    int i;
    for(i=0; i<t->type->section_count; i++)
    {
#define TREE_QUALITY_FACTOR 0.05
	GLfloat view_dir2[3];
	GLfloat width_sq = t->type->section[i].width*t->type->section[i].width;
	if(t->type->section[i].type == TREE_SECTION_JOINT)
	    width_sq /= 30;

	//printf("%f < %f ?\n",width_sq*s->render_quality*s->render_quality,distance_sq*TREE_QUALITY_FACTOR);
    
	if(width_sq*s->render_quality*s->render_quality < distance_sq*TREE_QUALITY_FACTOR)
	    continue;
	if(distance_sq < NORMALIZE_SECTION_DIST)
	{
	    switch(t->type->section[i].type)
	    {
		case TREE_SECTION_REGULAR:
		    add(view_dir, t->type->section[i].pos2, view_dir2, 3);
		    normalize(view_dir2, view_dir2, 3);
		    rotate_z(view_dir, -t->angle*M_PI/180);
		    render_section(&t->type->section[i], view_dir2);
		    break;
		case TREE_SECTION_JOINT:
		    add(view_dir, t->type->section[i].pos1, view_dir2, 3);
		    normalize(view_dir2, view_dir2, 3);
		    rotate_z(view_dir, -t->angle*M_PI/180);
		    render_joint(&t->type->section[i], view_dir2, 0);
		    break;
		    
		case TREE_SECTION_POINTS:
		    break;
		    
		default:
		    printf("Don't know how to render tree section of type %d\n", 
			   t->type->section[i].type);
		    exit(1);
		    
	    }
	}
	else
	{
	    switch(t->type->section[i].type)
	    {
		case TREE_SECTION_REGULAR:
		    render_section(&t->type->section[i], view_dir);
		    break;
		case TREE_SECTION_JOINT:
		    render_joint(&t->type->section[i], view_dir, 1);
		    break;
		    
		case TREE_SECTION_POINTS:
		    break;
		    
		default:
		    printf("Don't know how to render tree section of type %d\n", 
			   t->type->section[i].type);
		    exit(1);
		    
	    }
	    
	}
	
	
    }
    
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

    glEnable( GL_TEXTURE_2D );
    glEnable( GL_ALPHA_TEST );
    glEnable( GL_BLEND );

    glDepthMask( GL_FALSE );
    glShadeModel( GL_FLAT );

    glColor3f(0.0,0.0,0.0);

    int i;
    
    for(i=0; i<scene_tree_get_count(s); i++)
    {
	tree_t *tree = scene_tree_get(s, i);
	if(tree->visible)
	    render_tree_trunk(s, tree);
    }
    
    glDepthMask( GL_TRUE );
    glDisable(GL_TEXTURE_2D);
    glDisable( GL_ALPHA_TEST );
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
    
    render_register(render_trees_leaves, RENDER_PASS_SOLID);    
    render_register(render_trees_trunk, RENDER_PASS_TRANSPARENT);    
    
}

