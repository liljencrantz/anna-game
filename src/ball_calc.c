
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "scene.h"
#include "heightmap_element.h"
#include "node.h"
#include "util.h"
#include "assert.h"

void render_ball_at_level(ball_type_t *t, int level);


#define BALL_TYPE_CALC_NORM 15.0

static float height_factor(int lvl, float fx, float fy)
{
    float f = 0.5 - sqrt(pow(0.5-fx,2) + pow(0.5-fy,2)) / sqrt(2);
    
    int n = 2 << lvl;
    float alpha = 2.0 * M_PI/n;
    float h = cos(alpha);
    return 1*(1.0-f) + f*h;
}

static inline void interpolate( ball_type_t *b, int level, int x, int y )
{
    int i, j;
    int tot_factor = 0;
    float radius=0.0f;
    int factor_arr[]=
	{
	    -1,
	    2,
	    4,
	    2,
	    -1
	}
    ;
	
    int color[]={0,0,0};
    
    int side_x = 4 << level;
    int side_y = 2 << level;
    ball_point_t *dst = &b->data[ball_idx(level, x, y)];

    float height_factor_arr[] =
	{
	    0,0.5,0,0.5,0
	}
    ;
    

    for( i=0; i<5; i++ )
    {
	int x_new = (2*x+side_x-2+i)%side_x;
	for( j=0; j<5; j++ )
	{
	    int y_new = (2*y-2+i+side_y)%side_y;
	    ball_point_t *src = &b->data[ball_idx(level+1, x_new, y_new)];

	    int next_factor = factor_arr[i]*factor_arr[j];
	    tot_factor += next_factor;
		
	    radius += (float)next_factor* src->radius / height_factor(level, height_factor_arr[i], height_factor_arr[j]);
	    color[0] += next_factor * src->color[0];
	    color[1] += next_factor * src->color[1];
	    color[2] += next_factor * src->color[2];
		
	    
	}
    }
    
    radius /= (float)tot_factor;		
    dst->radius=radius;
    dst->color[0] = color[0]/tot_factor;
    dst->color[1] = color[1]/tot_factor;
    dst->color[2] = color[2]/tot_factor;
} 

/**
   For every non-leaf heightmap element in the scene, calculate the
   interpolated height of that heightmap element based on the height
   of the underlying elements.
*/
void ball_type_calc_lod(ball_type_t *b)
{
    
    int i, j, k;
    for( i=b->levels-1; i>=0; i-- )
    {
	int nw = 1<<i;
	int nw2 = 2<<i;
	for( j=0; j<nw2; j++ )
	{
	    for( k=0; k<nw; k++ )
	    {		
		interpolate( b, i, j, k );
	    }
	}
    }
    
}



/**
   For every non-leaf node in the scene, calculate the distortion
   coefficient to use when calculating the error obtained by using the
   specified node instead of the leaf node.
 */
static void ball_type_calc_node_distortion(ball_type_t *b)
{

    int i, j, lvl;
    
    for(i=0; i<b->levels; i++)
    {
	b->error[i] = 0;
    }
    int count[]={0,0,0,0,0,0,0,0,0};
    

    for(i=0; i< (2<<b->levels); i++)
    {
	for(j=0; j< (1<<(b->levels-1)); j++)
	{
	    GLfloat org_height = b->data[ball_idx(b->levels-1, i, j)].radius;
	    for(lvl=b->levels-2; lvl >=0; lvl--)
	    {
		int ldiff = b->levels - lvl-1;
		
		int x_trunc = (i) >> ldiff;
		float f1x = ((float)(i - (x_trunc << ldiff)) / (1<<ldiff));
		float f2x = 1.0-f1x;
		
		int y_trunc = (j) >> ldiff;
		float f1y = ((float)(j - (y_trunc << ldiff)) / (1<<ldiff));
		float f2y = 1.0-f1y;

		int max_x = 2<<lvl;
		int max_y = 1<<lvl;
		

//		printf("%d %d %d %.2f\n", j, lvl, y_trunc, f1y);
		
		assert(f1x>=0);
		assert(f2x>=0);
		assert(f1y>=0);
		assert(f2y>=0);
		
		GLfloat approx_height = 
		    b->data[ball_idx(lvl, x_trunc, y_trunc)].radius*f1x*f1y;
		approx_height += 
		    b->data[ball_idx(lvl, (x_trunc+1)%(max_x), y_trunc)].radius*f2x*f1y;
		approx_height += 
		    b->data[ball_idx(lvl, x_trunc, (y_trunc+1)%max_y)].radius*f1x*f2y;
		approx_height += 
		    b->data[ball_idx(lvl, (x_trunc+1)%max_x, (y_trunc+1)%max_y)].radius*f2x*f2y;
//		approx_height = 
//		    b->data[ball_idx(lvl, x_trunc, y_trunc)].radius;


//		approx_height = height_factor(lvl, f1x, f1y)*org_height;
		approx_height *= height_factor(lvl, f1x, f1y);
/*

		if(fabs(approx_height-org_height) > 2.01 && lvl>=4)
		{
		    printf(
			"LALA %d %d=>%d, %d=>%d, %.2f %.2f\n", 
			lvl, i, x_trunc, j, y_trunc, org_height, approx_height);
		}
*/		
//		b->error[lvl] = maxf(b->error[lvl], fabs(approx_height-org_height));//pow(fabs(approx_height-org_height),2);
		b->error[lvl] += pow(fabs(approx_height-org_height),2);
		count[lvl]++;
	    }
	}
    }

    for(i=b->levels-2; i>=0; i--)
    {
	b->error[i]= maxf(b->error[i], b->error[i+1]*1.4);
    }
    for(i=0; i<b->levels-1; i++)
    {
	b->error[i]/= count[i];
	b->error[i]= pow(b->error[i], 1.0/2.0);

//	printf("Level %d, error %.3f\n", i, b->error[i]);
    }

}
/*
static void ball_type_calc_normal_hid(scene_t *s, hid_t hid)
{
    int lvl = HID_GET_LEVEL(hid);
    int x = HID_GET_X_POS(hid);
    int y = HID_GET_Y_POS(hid);
    int x1, x2, y1, y2;
    x1 = x==0?0:x-1;
    y1 = y==0?0:y-1;
    x2 = x==(BALL_SUBBALL_HM_PER_BALL(lvl)-1)?x:x+1;
    y2 = y==(BALL_SUBBALL_HM_PER_BALL(lvl)-1)?y:y+1;
    hid_t xhid1, xhid2, yhid1, yhid2;
    HID_SET(xhid1, lvl, x1, y);
    HID_SET(xhid2, lvl, x2, y);
    HID_SET(yhid1, lvl, x, y1);
    HID_SET(yhid2, lvl, x, y2);
    
    GLfloat xv[3], yv[3], normal[3];
    
    xv[0] = scene_hid_x_coord(s, xhid1)- scene_hid_x_coord(s, xhid2);    
    xv[1] = 0.0;
    xv[2] = scene_hid_lookup(s, xhid1)->height - scene_hid_lookup(s,xhid2)->height;
    
    yv[0] = 0.0;
    yv[1] = scene_hid_y_coord(s, yhid1)- scene_hid_y_coord(s, yhid2);    
    yv[2] = scene_hid_lookup(s, yhid1)->height - scene_hid_lookup(s,yhid2)->height;
    
    cross_prod(xv, yv, normal);
    normalize(normal, normal, 3);
    heightmap_element_t *he = scene_hid_lookup(s, hid);
    memcpy(he->normal, normal, sizeof(GLfloat)*3);
    //printf("Normal is %f %f %f\n", normal[0], normal[1], normal[2]);
    
}
*/
void ball_type_calc(ball_type_t *b)
{
    int i;
    
    ball_type_calc_lod(b);
    ball_type_calc_node_distortion(b);

    render_ball_type_prerender(b);
}
