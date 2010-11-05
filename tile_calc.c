
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "scene.h"
#include "heightmap_element.h"
#include "node.h"
#include "util.h"
#include "assert.h"

#define TILE_CALC_NORM 15.0

static inline void interpolate( scene_t * s,
				hid_t hid )
{
    heightmap_element_t *hm = scene_hid_lookup(s, hid);
    if(!hm)
    {
	printf(
	    "Tried to interpolate on element %d %d %d\n",
	    HID_GET_LEVEL(hid), HID_GET_X_POS(hid), HID_GET_Y_POS(hid));
	
	exit(1);
    }
    

//	int col_arr[4];
/*
  colour_type col;
  colour_type sub_col;
*/	
    int i, j;
    int tot_factor = 0;
    GLfloat height=0.0f;
    int factor_arr[]=
	{
	    -1,
	    2,
	    4,
	    2,
	    -1
	}
    ;
//	col_arr[0]=col_arr[1]=col_arr[2]=col_arr[3]=0;
	
    int x_pos = HID_GET_X_POS(hid);
    int y_pos = HID_GET_Y_POS(hid);
    int color[3];
    
    
    for( i=0; i<5; i++ )
    {
	int x_new = 2*x_pos-2+i;
	if(x_new < 0 || x_new >= TILE_SUBTILE_HM_PER_TILE(HID_GET_LEVEL(hid)+1))
	{
	    continue;
	}
	for( j=0; j<5; j++ )
	{
	    int y_new = 2*y_pos-2+i;
	    if(y_new < 0 || y_new >= TILE_SUBTILE_HM_PER_TILE(HID_GET_LEVEL(hid)+1))
	    {
		continue;
	    }
	    
	    hid_t hid2;
	    HID_SET(hid2, HID_GET_LEVEL(hid)+1, x_new, y_new);
	    heightmap_element_t *hm2 = scene_hid_lookup(s, hid2);
	    if(hm2)
	    {
		int next_factor = factor_arr[i]*factor_arr[j];
		tot_factor += next_factor;
		
		height += (float)next_factor* hm2->height;
		color[0] += next_factor * hm2->color[0];
		color[1] += next_factor * hm2->color[1];
		color[2] += next_factor * hm2->color[2];
		
	    }
	}
    }
    if( tot_factor <= 0 )
    {
	return;
    }	
    
    height /= (float)tot_factor;		
    hm->height = height;
    hm->color[0] = color[0]/tot_factor;
    hm->color[1] = color[1]/tot_factor;
    hm->color[2] = color[2]/tot_factor;
} 

/**
   For every non-leaf heightmap element in the scene, calculate the
   interpolated height of that heightmap element based on the height
   of the underlying elements.
*/
void tile_calc_lod(scene_t *s, int level_count)
{
    
    int i, j, k;
    for( i=level_count-2; i>=0; i-- )
    {
	int nw = 2<<i;
	for( j=0; j<nw; j++ )
	{
	    for( k=0; k<nw; k++ )
	    {		
		hid_t hid;
		HID_SET(hid, i, j, k);
		interpolate( s, hid );
	    }
	}
    }
    
}

static void tile_calc_add_node_error(scene_t *s, int level, hid_t ohid, GLfloat err)
{
    int hid_x = HID_GET_X_POS(ohid);
    int hid_y = HID_GET_Y_POS(ohid);
    int hid_level = HID_GET_LEVEL(ohid);
    int w_diff = hid_level-level+1;
    int nid_x_width=1;
    int nid_x = hid_x >> w_diff;
    int nid_y_width=1;
    int nid_y = hid_y >> w_diff;
    if((nid_x<<w_diff == hid_x) && nid_x != 0)
    {
	nid_x--;
	nid_x_width=2;
    }
    if((nid_y<<w_diff == hid_y) && nid_y != 0)
    {
	nid_y--;
	nid_y_width=2;
    }
    int i,j;
    
    for(i=0; i<nid_x_width; i++)
    {
	for(j=0; j<nid_y_width; j++)
	{
	    nid_t nid;
	    NID_SET(nid, level, nid_x+i, nid_y+j);
	    t_node_t *n = scene_nid_lookup(s, nid);
	    n->distortion += powf(err,TILE_CALC_NORM);
//	    n->distortion = maxf(err, n->distortion);
	}
	
    }
    
}


/**
   For every non-leaf node in the scene, calculate the distortion
   coefficient to use when calculating the error obtained by using the
   specified node instead of the leaf node.
 */
static void tile_calc_node_distortion(scene_t *s, int level_count)
{
    int i, j, lvl, k;
    GLfloat avg[]={0,0,0,0,0,0,0,0};
    int avg_count[]={0,0,0,0,0,0,0,0};
    int count=0;
    
    for(i=0; i< (1<<level_count); i++)
    {
	for(j=0; j< (1<<level_count); j++)
	{
	    hid_t ohid;
	    HID_SET(ohid, level_count-1, i, j);
	    GLfloat org_height = scene_hid_lookup(s, ohid)->height;
	    GLfloat x_pos = scene_hid_x_coord(s, ohid);
	    GLfloat y_pos = scene_hid_y_coord(s, ohid);
	    
	    for(lvl=level_count-2; lvl >=0; lvl--)
	    {
		GLfloat approx_height = scene_get_height_level(s, lvl, x_pos, y_pos);
		tile_calc_add_node_error(s, lvl, ohid, fabs(approx_height-org_height));
//		printf("%d %d %d\n", lvl, i, j);
		assert(!isnan(approx_height));
		assert(!isnan(org_height));
		
		avg[lvl] += fabs(approx_height-org_height);
		assert(!isnan(avg[lvl]));
//		printf("%.2f\n", avg[lvl]);
	    }
	    count++;
	}
    }
    for(i=0; i<8; i++)
    {
	printf("Level %d, avg error %.2f\n", i, avg[i]/count);
	avg[i]=0;
    }
    printf("\n");
    
    for( i=level_count-2; i>=0; i-- )
    {
	int nw = 1<<i;
	for( j=0; j<nw; j++ )
	{
	    for( k=0; k<nw; k++ )
	    {
		nid_t nid;
		NID_SET(nid, i, j, k);
		t_node_t *n = scene_nid_lookup(s, nid);
		//printf("%.2f => %.2f\n", n->distortion, powf(n->distortion,1.0/3.0));
		n->distortion = powf(n->distortion,1.0/TILE_CALC_NORM);
		avg[i] += n->distortion;
		avg_count[i]++;
	    }
	}
    }

    for(i=0; i<7; i++)
    {
//	printf("Level %d, avg node distortion %.2f\n", i, avg[i]/avg_count[i]);
    }
}

static void tile_calc_normal_hid(scene_t *s, hid_t hid)
{
    int lvl = HID_GET_LEVEL(hid);
    int x = HID_GET_X_POS(hid);
    int y = HID_GET_Y_POS(hid);
    int x1, x2, y1, y2;
    x1 = x==0?0:x-1;
    y1 = y==0?0:y-1;
    x2 = x==(TILE_SUBTILE_HM_PER_TILE(lvl)-1)?x:x+1;
    y2 = y==(TILE_SUBTILE_HM_PER_TILE(lvl)-1)?y:y+1;
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

static void tile_calc_normal(scene_t *s, int level_count)
{
    int i, j, k;
/*
    for( i=level_count-1; i>=0; i-- )
    {
	int nw = 2<<i;
	for( j=0; j<nw; j++ )
	{
	    for( k=0; k<nw; k++ )
	    {		
		hid_t hid;
		HID_SET(hid, i, j, k);
		heightmap_element_t *he = scene_hid_lookup(s, hid);
		he->height = level_count-i;
		he->color[1] = i*255/level_count;
		
	    }
	}
    }
*/
    for( i=level_count-1; i>=0; i-- )
    {
	int nw = 2<<i;
	for( j=0; j<nw; j++ )
	{
	    for( k=0; k<nw; k++ )
	    {		
		hid_t hid;
		HID_SET(hid, i, j, k);
		tile_calc_normal_hid( s, hid );
	    }
	}
    }
    
}

static void tile_calc_validate(scene_t *s)
{
    int i, j, k;
/*
    for( i=level_count-1; i>=0; i-- )
    {
	int nw = 2<<i;
	for( j=0; j<nw; j++ )
	{
	    for( k=0; k<nw; k++ )
	    {		
		hid_t hid;
		HID_SET(hid, i, j, k);
		heightmap_element_t *he = scene_hid_lookup(s, hid);
		he->height = level_count-i;
		he->color[1] = i*255/level_count;
		
	    }
	}
    }
*/
    int lvl = s->level_base-1;
    
    int nw = 1<<lvl;
    for( j=0; j<nw; j++ )
    {
	for( k=0; k<nw; k++ )
	{		
	    nid_t nid;
	    NID_SET(nid, lvl, j, k);
	    t_node_t *n = scene_nid_lookup(s, nid);
	    if(n->distortion != 0)
	    {
		printf("Oops, distortion is %.4f at %d %d %d\n", n->distortion, lvl, j, k);
		exit(1);

	    }
	    
	}
    }
    
    
}

void tile_calc(scene_t *s)
{
    tile_calc_validate(s);
    printf("Inbound data is valid\n");
    tile_calc_lod(s, s->level_base);   



    printf("LOD data generated\n");
    tile_calc_node_distortion(s, s->level_base);
    printf("Distortions calculated\n");

    tile_calc_validate(s);
    printf("Midpoint data is valid\n");


    tile_calc_normal(s, s->level_base);
    printf("Normals calculated\n");
    tile_calc_validate(s);
    printf("Outbound data is valid\n");
}
