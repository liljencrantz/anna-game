#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <assert.h>

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
    s->render_quality=75.0;
}
/*

float get_height( scene_t *s, float xf, float yf )
{
	int tile_x, tile_y, idx_x, idx_y, heightmap_side;
	double sub_x, sub_y, factor_x1, factor_x2, factor_y1, factor_y2;
	float res;
		
	tile_t *tile;

	tile_x = (int)xf/s->tile_width;
	tile_y = (int)yf/s->tile_width;

	tile = get_tile( s, tile_x, tile_y );
	if( (tile == 0) || (tile->state <= TILE_ACTIVE ) )
	{
		return 0.0f;
	}	

	heightmap_side = (1<<tile->lod)+1;

	sub_x = (xf - s->tile_width*tile_x)*heightmap_side/s->tile_width;
	sub_y = (yf - s->tile_width*tile_y)*heightmap_side/s->tile_width;
	
	factor_x2 = sub_x - floor(sub_x);
	factor_x1 = 1.0f-factor_x2;
	factor_y2 = sub_y - floor(sub_y);
	factor_y1 = 1.0f-factor_y2;
	
	idx_x = floor(sub_x);
	idx_y = floor(sub_y);

	res =  tile->heightmap[tile->lod-1][idx_x   + idx_y     * heightmap_side ].height *factor_x1*factor_y1;
	res += tile->heightmap[tile->lod-1][idx_x+1 + idx_y     * heightmap_side ].height *factor_x2*factor_y1;
	res += tile->heightmap[tile->lod-1][idx_x   + (idx_y+1) * heightmap_side ].height *factor_x1*factor_y2;
	res += tile->heightmap[tile->lod-1][idx_x+1 + (idx_y+1) * heightmap_side ].height *factor_x2*factor_y2;
	
	return res;
}

void get_nabla_height( scene_t *s, float xf, float yf, float *arr )
{
    int tile_x, tile_y, idx_x, idx_y, heightmap_side;
    double sub_x, sub_y;
    
    
    tile_t *tile;
    
    
    tile_x = (int)xf/s->tile_width;
    tile_y = (int)yf/s->tile_width;
    
    tile = get_tile( s, tile_x, tile_y );
    if( (tile == 0) || (tile->state <= TILE_ACTIVE ) )
    {
	arr[0]=arr[1]=0;
	return;
    }
    
    heightmap_side = (1<<tile->lod)+1;
    
    sub_x = (xf - s->tile_width*tile_x)*heightmap_side/s->tile_width;
    sub_y = (yf - s->tile_width*tile_y)*heightmap_side/s->tile_width;
    
    idx_x = floor(sub_x);
    idx_y = floor(sub_y);
    
    arr[0] = tile->heightmap[tile->lod-1][idx_x+1   + idx_y     * heightmap_side ].height - 
	tile->heightmap[tile->lod-1][idx_x   + idx_y     * heightmap_side ].height;
    
    arr[1] = tile->heightmap[tile->lod-1][idx_x   + (idx_y+1)     * heightmap_side ].height - 
	tile->heightmap[tile->lod-1][idx_x   + idx_y     * heightmap_side ].height;
}
*/
/*
tile_t *get_tile( scene_t *s, int x, int y )
{
    if(s->tile_cash_x == x && s->tile_cash_y == y)
    {
	return s->tile_cash;
    }
    else
    {
	tile_t *t = (tile_t *)hash_get(
	    &s->tile_hash, 
	    (void *)hash_key(x, y ) );
	s->tile_cash_x=x;
	s->tile_cash_y=y;
	s->tile_cash=t;
	return t;
    }
}
*/
 /*
float get_corner_height( scene_t *s, int x, int y )
{
//	printf( "%d %d\n", x, y );	
	float *foo = hash_get( &s->height_hash, (void *)hash_key(x, y ) );
	if( foo == 0 )
	{		
		foo=&s->height_arr[s->height_arr_pos++];
		*foo = generate_corner_height( s, x, y );
		hash_put( &s->height_hash, (void *)hash_key(x, y ), foo );
	}
	return *foo;
}
 */

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

float scene_get_height( scene_t *s, float xf, float yf )
{
    return scene_get_height_level(s, s->level_base-1, xf, yf);
    
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
    
    return sqrt(npos[0]*npos[0]+npos[1]*npos[1]);
}


int scene_nid_is_visible(scene_t *s, nid_t nid, float distance, view_t *pos)
{
    int i, j;
    hid_t hid[10];
    int visible;
//    return 1;

    if(NID_GET_LEVEL(nid) < 5)
	return 1;
    
    nid_get_hid(nid, hid);
//    printf("Check if node %d %d %d is visible.\n", NID_GET_LEVEL(nid), NID_GET_X_POS(nid), NID_GET_Y_POS(nid));
    for( i=0; i<3; i++ ){
	float k = s->camera.k[i];
	float m = s->camera.m[i];
	int side = s->camera.side[i];
	visible=0;
	for( j=1; j<8; j+= 2 )
	{
	    float node_x = scene_hid_x_coord(s, hid[j]);
	    float node_y = scene_hid_y_coord(s, hid[j]);
	    float proj_y = node_x*k + m;		 
	    if( side )
		visible |=  proj_y <= node_y ;
	    else
		visible |=  proj_y >= node_y ;
	}
	if( !visible )
	{
//	    printf("No, plane %d\n", i);
	    return 0;
	}
    }
//    printf("Yes\n");

    return 1;
}

int scene_is_visible(scene_t *s, float *pos, float radius)
{
    int i, j;
    int visible;
//    return 1;

//    printf("Check if node %d %d %d is visible.\n", NID_GET_LEVEL(nid), NID_GET_X_POS(nid), NID_GET_Y_POS(nid));
    for( i=0; i<3; i++ ){
	float k = s->camera.k[i];
	float m = s->camera.m[i];
	int side = s->camera.side[i];
	visible=0;
	float proj_y = pos[0]*k + m;		 
	    if( side )
		visible |=  proj_y <= pos[1];
	    else
		visible |=  proj_y >= pos[1];
	
	if( !visible )
	{
//	    printf("No, plane %d\n", i);
	    return 0;
	}
    }
//    printf("Yes\n");

    return 1;
    

}
