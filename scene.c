#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include <GL/glew.h>	

#include "util.h"
#include "scene.h"

#define RENDER_DISTANCE 130
#define BUFF_SZ 512

void scene_init( scene_t *s, char *name, int load )
{
    memset(s, 0, sizeof(scene_t));

    s->load = load;
    assert(strlen(name) < SCENE_NAME_MAX);
    
    strcpy(s->name, name);
    s->sun_pos[0]=-sqrt(0.85);
    s->sun_pos[1]=0.0;
    s->sun_pos[2]=sqrt(0.15);
    
    s->ambient_light = 0.5;
    s->sun_light = 1.0;
    
    s->render_quality=40.0;
    
}

static size_t scene_allocate_tile(tile_t *t, int lvl)
{
    if(!lvl)
	return 0;
    int i;

    size_t res=0;
    
    for(i=0; i<TILE_SUBTILE_COUNT; i++)
    {
	size_t sz = sizeof(tile_t);
	if(lvl == 1)
	{
	    sz -= sizeof(tile_t *) * TILE_SUBTILE_COUNT;
	}
	
	res += sz;
	t->subtile[i] = calloc(1, sz);	
    }
    for(i=0; i<TILE_SUBTILE_COUNT; i++)
    {
	res += scene_allocate_tile(t->subtile[i], lvl-1);
    }
    return res;
}


void scene_configure( scene_t *s, int lb, float scene_size )
{
    s->level_base = lb * TILE_LEVELS;
    s->scene_size = scene_size;

    printf(
	"Each tile uses %d bytes for nodes, %d bytes for heightmaps and %d bytes for pointers\n",
	sizeof(t_node_t) * TILE_NODE_COUNT(TILE_LEVELS),
	sizeof(heightmap_element_t) * TILE_HM_COUNT(TILE_LEVELS),
	sizeof(tile_t *) * TILE_SUBTILE_COUNT
	);
    

    
    tile_t *t = calloc(1, sizeof(tile_t));
    size_t res = scene_allocate_tile(t, lb-1);
    s->root_tile = t;
    printf(
	"%d tiles of %d bytes each allocated. Used a total of %.2f megabytes of memory\n", 
	res/sizeof(tile_t), sizeof(tile_t),
	((double)res)/1024/1024);
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

static void scene_get_slope_level( 
    scene_t *s, int level, 
    float xf, float yf, 
    float *slope )
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
    
    if((base_x >= (points_at_level-1)) ||
       (base_y >= (points_at_level-1)) ||
       !x_inc ||
       !y_inc ||
       (base_x < 0) ||
       (base_y < 0))
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
    
    if((base_x < 0) ||
       (base_y < 0) ||
       (base_x >= points_at_level-1) ||
       (base_y >= points_at_level-1))
    {
	return 0.0;
    }
    
    //fprintf(stderr, "%d %.2f %.2f\n", level, xf, yf);
    
    //printf("Base %d %d %d\n", level, base_x, base_y);
    //printf("Figure out height at %.2f %.2f\n", xf, yf);

    if(f1 >= 1.0)
    {
	printf(
	    "OOOOPS, lvl %d, xf %.5f, yf %.5f, f %.2f %.2f\n", 
	    level, xf, yf, f1, f2);
    }
    
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
    if(dst_sq > ((RENDER_DISTANCE+radius)*(RENDER_DISTANCE+radius)))
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


void scene_tree_destroy(
    scene_t *s, 
    int tid)
{
    s->tree_used[tid/32] &= ~(1<<(tid%32));
    s->tree_search_start = mini(tid, s->tree_search_start);
    s->tree_count--;
}

size_t scene_tree_get_count(scene_t *s)
{
    return s->tree_count;
}

static inline int scene_tree_free(scene_t *s, size_t tid)
{
    return !(s->tree_used[tid/32] & (1<<(tid%32)));
}

tree_t *scene_tree_get(scene_t *s, size_t idx)
{
    return scene_tree_free(s, idx)?0:&s->tree[idx];
}


int scene_tree_create(
    scene_t *s,
    char *tree_type_name,
    float *pos,
    float angle,
    float scale)
{
    int idx = s->tree_search_start;
    while(!scene_tree_free(s, idx))
	idx++;
    
    tree_t *t = &s->tree[idx];
    
    t->pos[0] = pos[0];
    t->pos[1] = pos[1];
    t->pos[2] = scene_get_height(s, pos[0], pos[1]);
    //printf("Create tree at %f %f %f with angle %.2f\n", pos[0], pos[1], t->pos[2], angle);
    t->type = tree_type_get(tree_type_name);
    t->angle = angle;
    t->radius = 3.0;
    t->scale = 1.0;
    
    s->tree_count++;
    s->tree_search_start = idx+1;
    s->tree_used[idx/32] |= (1<<(idx%32));
    
    return idx;
}





void scene_ball_destroy(
    scene_t *s, 
    int tid)
{
    s->ball_used[tid/32] &= ~(1<<(tid%32));
    s->ball_search_start = mini(tid, s->ball_search_start);
    s->ball_count--;
}

size_t scene_ball_get_count(scene_t *s)
{
    return s->ball_count;
}

static inline int scene_ball_free(scene_t *s, size_t tid)
{
    return !(s->ball_used[tid/32] & (1<<(tid%32)));
}

ball_t *scene_ball_get(scene_t *s, size_t idx)
{
    return scene_ball_free(s, idx)?0:&s->ball[idx];
}

int scene_ball_create(
    scene_t *s,
    char *ball_type_name,
    float scale)
{
    int idx = s->ball_search_start;
    while(!scene_ball_free(s, idx))
	idx++;
    
    ball_t *t = &s->ball[idx];
    
    //printf("Create ball at %f %f %f with angle %.2f\n", pos[0], pos[1], t->pos[2], angle);
    t->type = ball_type_get(ball_type_name);
    t->scale = scale;
    
    s->ball_count++;
    s->ball_search_start = idx+1;
    s->ball_used[idx/32] |= (1<<(idx%32));
    
    return idx;
}





void scene_boid_set_destroy(
    scene_t *s, 
    int tid)
{
    s->boid_set_used[tid/32] &= ~(1<<(tid%32));
    s->boid_set_search_start = mini(tid, s->boid_set_search_start);
    s->boid_set_count--;
}

size_t scene_boid_set_get_count(scene_t *s)
{
    return s->boid_set_count;
}

static inline int scene_boid_set_is_free(scene_t *s, size_t tid)
{
    return !(s->boid_set_used[tid/32] & (1<<(tid%32)));
}

boid_set_t *scene_boid_set_get(scene_t *s, size_t idx)
{
    return scene_boid_set_is_free(s, idx)?0:s->boid_set[idx];
}

int scene_boid_set_create(
    scene_t *s,
    int count,
    float x,
    float y)
{
    int idx = s->boid_set_search_start;
    while(!scene_boid_set_is_free(s, idx))
	idx++;
    
    boid_set_t *t = boid_set_init(count, x, y);
    s->boid_set[idx] = t;
    s->boid_set_count++;
    s->boid_set_search_start = idx+1;
    s->boid_set_used[idx/32] |= (1<<(idx%32));
    
    return idx;
}

void scene_save(scene_t *s)
{
    char dir_name[BUFF_SZ];
    if(sprintf(dir_name, BUFF_SZ, "data/%s/terrain/tile_0") >= BUFF_SZ)
    {
	printf("Name too long.\n");
	exit(1);
    }
    //tile_save(s->root_node, dir_name);
    
}
