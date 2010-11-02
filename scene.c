#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

#include <GL/glew.h>	

#include "util.h"
#include "scene.h"

#define RENDER_DISTANCE 130
#define BUFF_SZ 512

#define SCENE_FORMAT_VERSION 0

#define SUBTILE_LEVEL_MAX 10

typedef struct 
{
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t convar;
}
    scene_load_t;

typedef struct
{
    int version;
    int level_base;
    float scene_size;
}
    scene_head_t;

typedef struct
{
    int subtile[SUBTILE_LEVEL_MAX];
    int subtile_level;    
}
    subtile_t;


static tile_t *scene_tile_load(scene_t *s, subtile_t *st)
{
    tile_t *t = malloc(sizeof(tile_t));
    char fn[BUFF_SZ];
    size_t used=0;
    int i;
    
    used += snprintf(
	fn+used, 
	BUFF_SZ-used,
	"data/%s/terrain/tile",
	s->name);
    
    for(i=0; i<st->subtile_level; i++)
    {
	used += snprintf(
	    fn+used, 
	    BUFF_SZ-used,
	    "_%d",
	    st->subtile[i] );
    }
        
    used += snprintf(
	fn+used, 
	BUFF_SZ-used,
	".atd");
    
    assert(used < BUFF_SZ);
    
    FILE *f = fopen(fn, "w");
    assert(f);
    size_t read = fread(t, sizeof(tile_t), 1, f);
    int cl = fclose(f);
    assert((cl==0) && (read == 1));
    return t;
}

static tile_t **get_tile_address(tile_t *pos, subtile_t *sub, int idx)
{
    if( idx == sub->subtile_level-1)
    {
	return &pos->subtile[sub->subtile[idx]];
    }
    return get_tile_address(pos->subtile[sub->subtile[idx]], sub, idx+1);
}

static void scene_tile_insert(scene_t *s, tile_t *t, subtile_t *sub)
{
    if(sub->subtile_level == 0)
    {
	s->root_tile = t;
    }
    else
    {
	tile_t **destination = get_tile_address(s->root_tile, sub, 0);
	*destination = t;
    }
}

static tile_t *scene_tile_remove(scene_t *s, subtile_t *sub)
{
    tile_t *t;
    if(sub->subtile_level == 0)
    {
	t = s->root_tile;
	s->root_tile = t;
    }
    else
    {
	tile_t **tp = get_tile_address(s->root_tile, sub, 0);
	t = *tp;
	*tp = 0;
    }
    return t;
}

static int scene_find_missing_tile(scene_t *s, subtile_t *sub)
{
    return 0;
}

static int scene_find_unneeded_tile(scene_t *s, subtile_t *sub)
{
    return 0;
}

/*
  If a tile needs loading, load it, wait for the main thread to allow
  updates, and insert it, return true.
  
  If a tile can be unloaded, wait for the main thread to allow updates
  and remove it, return true.
  
  Otherwise, return false.
*/
static int scene_try_load_tile(scene_t *s)
{
    scene_load_t *sl = (scene_load_t *)s->load_state;
    subtile_t sub;
    if(scene_find_missing_tile(s, &sub))
    {
	tile_t *t = scene_tile_load(s, &sub);
	
	pthread_mutex_lock(&sl->mutex);
	pthread_cond_wait(&sl->convar, &sl->mutex);	    
	scene_tile_insert(s, t, &sub);
	pthread_mutex_unlock(&sl->mutex);	
	return 1;
    }

    if(scene_find_unneeded_tile(s, &sub))
    {
	pthread_mutex_lock(&sl->mutex);
	pthread_cond_wait(&sl->convar, &sl->mutex);	    
	tile_t *t = scene_tile_remove(s, &sub);
	pthread_mutex_unlock(&sl->mutex);	
	free(t);
	return 1;
    }
    
    return 0;
}

void scene_update(scene_t *s)
{
    scene_load_t *sl = (scene_load_t *)s->load_state;
    pthread_mutex_lock(&sl->mutex);
    pthread_cond_signal(&sl->convar);
    pthread_mutex_unlock(&sl->mutex);
}

static void *scene_load_runner(void *arg)
{
    scene_t *s = (scene_t *)arg;
    scene_load_t *sl = (scene_load_t *)s->load_state;

    set_current_thread_name("anna (loader)");
    
    /*
      If we have nothing to do, go to sleep for ~half a second, then
      recheck if we need to load anything.
      
      If we actually had something to do, check at once if we can do
      more.
    */
    struct timespec ts = {
	0,
	500000
    };
    
    while(1)
    {
	if(!scene_try_load_tile(s))
	{
	    nanosleep(&ts, 0);
	}	
    }
    return 0;
    
}


static void scene_load_init(scene_t *s)
{
    char fname[BUFF_SZ];
    if(snprintf(fname, BUFF_SZ, "data/%s/scene.asd", s->name) < BUFF_SZ)
    {
	FILE *f = fopen(fname, "r");
	if(f)
	{
	    size_t read = fread(s, sizeof(scene_head_t), 1, f);
	    if((fclose(f)==0) && (read == 1))
	    {
		s->load_state = malloc(sizeof(scene_load_t));
		scene_load_t *sl = (scene_load_t *)s->load_state;
		
		pthread_mutex_init(&sl->mutex, 0);
		pthread_cond_init(&sl->convar, 0);
		
		int rc = pthread_create(
		    &sl->thread, 
		    0,
		    scene_load_runner, s);
		
		if(rc)
		{
		    printf("ERROR; return code from pthread_create() is %d\n", rc);
		    exit(1);
		}
		return;
	    }
	}
    }
    printf("Failed to load scene %s\n", s->name);
    exit(1);
}

void scene_destroy(scene_t *s)
{
    if(s->load)
    {
	scene_load_t *sl = (scene_load_t *)s->load_state;
    }
    
}


void scene_init( scene_t *s, char *name, int load )
{
    memset(s, 0, sizeof(scene_t));

    s->version = SCENE_FORMAT_VERSION;
    
    s->load = load;
    assert(strlen(name) < SCENE_NAME_MAX);
    
    strcpy(s->name, name);
    s->sun_pos[0]=-sqrt(0.85);
    s->sun_pos[1]=0.0;
    s->sun_pos[2]=sqrt(0.15);
    
    s->ambient_light = 0.5;
    s->sun_light = 1.0;
    
    s->render_quality=60.0;

    if(load)
    {
	scene_load_init(s);
    }
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
    size_t res = scene_allocate_tile(t, lb-1) + sizeof(tile_t);
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

static void scene_tile_save(tile_t *t, char *dst)
{
    char fname[BUFF_SZ];
    if(snprintf(fname, BUFF_SZ, "%s.atd", dst) < BUFF_SZ)
    {
	FILE *f = fopen(fname, "w");
	if(f)
	{
	    size_t written = fwrite(t, sizeof(tile_t), 1, f);
	    if((fclose(f)==0) && (written == 1))
	    {
		int i;
		for(i=0; i<TILE_SUBTILE_COUNT; i++)
		{
		    tile_t *st = t->subtile[i];
		    if(st)
		    {
			snprintf(fname, BUFF_SZ, "%s_%d", dst, i);
			scene_tile_save(st, fname);			
		    }
		}
		return;
	    }
	}
    }
    
    printf("Failed to save tile %s\n", dst);
    exit(1);

}


void scene_save(scene_t *s)
{
    char fname[BUFF_SZ];
    if(snprintf(fname, BUFF_SZ, "data/%s/scene.asd", s->name) < BUFF_SZ)
    {

	FILE *f = fopen(fname, "w");
	if(f)
	{
	    size_t written = fwrite(s, sizeof(scene_head_t), 1, f);
	    if((fclose(f)==0) && (written == 1))
	    {
		if(snprintf(fname, BUFF_SZ, "data/%s/terrain/tile_0", s->name) < BUFF_SZ)
		{
		    
		    scene_tile_save(s->root_tile, fname);
		    return;
		}
	    }
	}
    }
    printf("Failed to save scene %s\n", s->name);
    exit(1);
    
}

