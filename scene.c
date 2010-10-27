#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include <GL/gl.h>	

#include "util.h"
#include "scene.h"

//#include "generate.h"

void scene_init( scene_t *s, int lb, float scene_size )
{
    memset(s, 0, sizeof(scene_t));
    
    s->level_base = lb;
    s->sun_pos[0]=-sqrt(0.85);
    s->sun_pos[1]=0.0;
    s->sun_pos[2]=sqrt(0.15);
    
    s->ambient_light = 0.5;
    s->sun_light = 1.0;

    s->scene_size = scene_size;
    s->render_quality=60.0;
}

float scene_hid_x_coord(scene_t *s, hid_t hid)
{
    int idx = HID_GET_X_POS(hid);
    int max = (2<<HID_GET_LEVEL(hid));
    return (s->scene_size * idx) / max;
}

float scene_hid_y_coord(scene_t *s, hid_t hid)
{
    int idx = HID_GET_Y_POS(hid);
    int max = (2<<HID_GET_LEVEL(hid));
    return (s->scene_size * idx) / max;
}

static void scene_get_slope_level( scene_t *s, int level, float xf, float yf, float *slope )
{
    int points_at_level = (2 << level);
    float scaled_x = xf*points_at_level/s->scene_size;
    float scaled_y = yf*points_at_level/s->scene_size;
    int base_x = (int)floor(scaled_x);
    int base_y = (int)floor(scaled_y);

    hid_t hid;
    int y_inc = (base_y < (points_at_level-2));
    int x_inc = (base_x < (points_at_level-2));
    float w_inv = points_at_level/s->scene_size;
    
    if((base_x >= points_at_level-1) ||
       (base_y >= points_at_level-1) ||
       !x_inc ||
       !y_inc)
    {
	slope[0]=0;
	slope[1]=0;
	return;
    }
        
    HID_SET(hid, level, base_x, base_y);
    float h1 = scene_hid_lookup(s, hid)->height;
    /*
    printf(
	"LALA2 %.2f %.2f\n",
	(1.0-f1)*(1.0-f2)*scene_hid_lookup(s, hid)->height,
	height);
*/  
    //return scene_hid_lookup(s, hid)->height;
    
    HID_SET(hid, level, base_x+1, base_y);
    float h2 = scene_hid_lookup(s, hid)->height;
    
    HID_SET(hid, level, base_x, base_y+1);
    
    float h3 = scene_hid_lookup(s, hid)->height;
    
    slope[0] = (h2-h1)*w_inv;
    slope[1] = (h3-h1)*w_inv;
}


float scene_get_height( scene_t *s, float xf, float yf )
{
    return scene_get_height_level(s, s->level_base-1, xf, yf);    
}

void scene_get_slope( scene_t *s, float xf, float yf, float *slope )
{
    scene_get_slope_level(s, s->level_base-1, xf, yf, slope);
}

float scene_get_height_level( scene_t *s, int level, float xf, float yf )
{
    int points_at_level = (2 << level);
    float scaled_x = xf*points_at_level/s->scene_size;
    float scaled_y = yf*points_at_level/s->scene_size;
    int base_x = (int)floor(scaled_x);
    int base_y = (int)floor(scaled_y);
    float f1 = scaled_x-base_x;
    float f2 = scaled_y-base_y;
    float height=0.0;
    hid_t hid;
    int y_inc = (base_y < (points_at_level-2))?1:0;
    int x_inc = (base_x < (points_at_level-2))?1:0;
    
    if((base_x >= points_at_level-1) ||
       (base_y >= points_at_level-1))
    {
	return 0.0;
    }
        
    //printf("Base %d %d %d\n", level, base_x, base_y);
    //printf("Figure out height at %.2f %.2f\n", xf, yf);
    if(f1 >= 1.0)
    {
	printf("OOOOPS, lvl %d, xf %.5f, yf %.5f, f %.2f %.2f\n", level, xf, yf, f1, f2);
	
    }
    
    assert(f1<1.0);
    assert(f2<1.0);
    //printf("Factors: %.2f %.2f\n", f1,f2);
    
    HID_SET(hid, level, base_x, base_y);
    //  printf("LALA1 %.2f\n",height);
    height = (1.0-f1)*(1.0-f2)*scene_hid_lookup(s, hid)->height;
    /*
    printf(
	"LALA2 %.2f %.2f\n",
	(1.0-f1)*(1.0-f2)*scene_hid_lookup(s, hid)->height,
	height);
*/  
    //return scene_hid_lookup(s, hid)->height;
    
    HID_SET(hid, level, base_x+x_inc, base_y);
    height += (f1)*(1.0-f2)*scene_hid_lookup(s, hid)->height;
    
    HID_SET(hid, level, base_x, base_y+y_inc);
    
    height += (1.0-f1)*(f2)*scene_hid_lookup(s, hid)->height;
    
    HID_SET(hid, level, base_x+x_inc, base_y+y_inc);
    height += (f1)*(f2)*scene_hid_lookup(s, hid)->height;
    
    return height;
}

void scene_nid_coord(scene_t *s, nid_t nid, float *dst)
{
    float points_at_level = (1 << NID_GET_LEVEL(nid));
    float x = 0.5 + NID_GET_X_POS(nid);
    float y = 0.5 + NID_GET_Y_POS(nid);
    float w = s->scene_size;
    dst[0] = w*x/points_at_level;
    dst[1] = w*y/points_at_level;
    
}

float scene_nid_min_distance(scene_t *s, nid_t nid, view_t *pos)
{
    float npos[2];
    scene_nid_coord(s, nid, npos);
    //printf("Node is %.2f %.2f. Camera is %.2f %.2f\n", npos[0], npos[1], pos->pos[0], pos->pos[1]);
    npos[0]-= pos->pos[0];
    npos[1]-= pos->pos[1];
    
    return maxf(0.0, sqrt(npos[0]*npos[0]+npos[1]*npos[1]) - scene_nid_radius(s, nid));
}

float scene_nid_radius(scene_t *s, nid_t nid)
{
    int level = NID_GET_LEVEL(nid);
    int points_at_level = 1 << level;
    float side = s->scene_size / points_at_level;
    return side * 0.7082;
    
}


int scene_nid_is_visible(scene_t *s, nid_t nid, view_t *pos)
{
    float p[2];
    scene_nid_coord(s, nid, p);
    return scene_is_visible(s, p, scene_nid_radius(s,nid));
}

int scene_is_visible(scene_t *s, float *pos, float radius)
{
    int i;
    int visible;

    float diff[] = 
	{
	    s->camera.pos[0]-pos[0],
	    s->camera.pos[1]-pos[1]
	}
    ;
    float dst_sq = diff[0]*diff[0] + diff[1]*diff[1];
    if(dst_sq > ((80+radius)*(80+radius)))
	return 0;
    


//    printf("Check if node %d %d %d is visible.\n", NID_GET_LEVEL(nid), NID_GET_X_POS(nid), NID_GET_Y_POS(nid));
    for( i=2; i<3; i++ ){
	float k = s->camera.k[i];
	float m = s->camera.m[i];
	int side = s->camera.side[i];
	visible=0;
	float proj_y = pos[0]*k + m + radius*(1+fabs(k))*(side?-1.0:1.0);
	if( side )
	    visible |=  proj_y <= pos[1];
	else
	    visible |=  proj_y >= pos[1];
	
	if( !visible )
	{
	    return 0;
	}
    }

    return 1;
    

}
