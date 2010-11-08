#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

#include <GL/glew.h>	

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"
#include "hash_table.h"
#include "scene.h"

#define SCENE_FORMAT_VERSION 0

#define SUBTILE_LEVEL_MAX 10

#define SCENE_NONE -1

/**
   Time, in seconds, from when a piece of data is no longer deemed
   needed by the renderer until it will be unloaded.
 */
#define SCENE_UNLOAD_GRACE_PERIOD 30

typedef struct 
{
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t convar;
    /**
       A hash table storing needed, loadable data.
     */
    hash_table_t used;
    int current_lap;    
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
    tile_t *t = calloc(1,sizeof(tile_t));
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
    
    FILE *f = fopen(fn, "r");
    assert(f);
    size_t read = fread(t, sizeof(tile_t)- sizeof(tile_t *) * TILE_SUBTILE_COUNT, 1, f);
    int cl = fclose(f);
    if(!((cl==0) && (read == 1)))
    {
	printf(
	    "Error reading file %s. Read %d units, close status was %d\n",
	    fn, read, cl);
	
	exit(1);
    }
    
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
	tile_t **destination = get_tile_address(s->root_tile, sub, 1);
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

static int scene_subtile_x_idx(scene_t *s, int lvl, float pos)
{
    int tile_side = 1 << (lvl*TILE_LEVELS);
    return (pos/s->scene_size)*tile_side;
}

static int scene_subtile_y_idx(scene_t *s, int lvl, float pos)
{
    int tile_side = 1 << (lvl*TILE_LEVELS);
    return (pos/s->scene_size)*tile_side;
}

static int scene_subtile_side(scene_t *s, int lvl)
{
    return 1 << (lvl*TILE_LEVELS);    
}

static int scene_subtile_idx(scene_t *s, int lvl, int x, int y)
{
    return x + scene_subtile_side(s,lvl)*y;
    
}

static int scene_find_missing_tile_from_position(
    scene_t *s, subtile_t *sub, float *pos, float priority)
{
    int lvl, x, y;
    scene_load_t *sl = (scene_load_t *)s->load_state;
    sub->subtile[0]=0;
    for(lvl=1; lvl < (s->level_base/TILE_LEVELS); lvl++)
    {
	int subtile_radius = 8 * priority;
	int x_base = scene_subtile_x_idx(s, lvl, pos[0]);
	int y_base = scene_subtile_y_idx(s, lvl, pos[1]);
	int subtile_side = scene_subtile_side(s, lvl);
	sub->subtile_level = lvl+1;
//	printf("Wop di doo, check level %d, base is %d %d, radius %d\n",
//	       lvl, x_base, y_base, subtile_radius);
		
	for(
	    x=maxi(0, x_base-subtile_radius);
	    x <= mini(x_base +subtile_radius, subtile_side-1);
	    x++)
	{
	    for(
		y=maxi(0, y_base-subtile_radius);
		y <= mini(y_base +subtile_radius, subtile_side-1);
		y++)
	    {
		//	printf("Check tile %d %d\n", x, y);
		
		int idx = scene_subtile_idx(s, lvl, x, y);
		sub->subtile[lvl] = idx;
		tile_t ** tile_ptr = get_tile_address(s->root_tile, sub, 1);
		
		if(!*tile_ptr)
		{
		    //  printf("Yes\n");
		    
		    return 1;
		}
		else
		{
		    hash_put(
			&sl->used, 
			*tile_ptr,
			(void *)sl->current_lap);
		}
		
	    }
	    
	}
    }
    
    return 0;
    
}

static int scene_find_missing_tile(
    scene_t *s, subtile_t *sub)
{
    return scene_find_missing_tile_from_position(
	s, sub, s->camera.pos, 1);
    
}

static int scene_find_unneeded_tile_recursive(
    scene_t *s, tile_t *t, subtile_t *sub)
{
    int i;
    int has_subtile=0;
    
    if(!t)
	return 0;

    scene_load_t *sl = (scene_load_t *)s->load_state;
    
    sub->subtile_level++;
    
    if(sub->subtile_level < s->level_base/TILE_LEVELS)
    {
	for(i=0; i<TILE_SUBTILE_COUNT; i++)
	{
	    tile_t *st = t->subtile[i];
	    has_subtile |= !!st;
	    sub->subtile[sub->subtile_level-1]=i;
	    
	    if(scene_find_unneeded_tile_recursive(s, st, sub))
		return 1;
	}
    }
    
    sub->subtile_level--;
    
    if(sub->subtile_level > 0) 
    {
	if(!has_subtile)
	{
	    if(sl->current_lap - (int)hash_get(&sl->used, t) > SCENE_UNLOAD_GRACE_PERIOD * s->target_fps)
	    {
		hash_remove(&sl->used, t, 0, 0);
		return 1;
	    }
	}
    }
    
    return 0;
}

static ssize_t scene_find_unneeded_tile(
    scene_t *s, subtile_t *sub)
{
    sub->subtile_level=0;
    return scene_find_unneeded_tile_recursive(
	s, s->root_tile, sub);
}

static tree_tile_t *scene_tree_tile_load(scene_t *s, ssize_t idx)
{
    char fname[BUFF_SZ];
    size_t item_tile_side = ceilf(s->scene_size/ITEM_TILE_SIZE);
    int i, j;
    tree_tile_t *res;
    
    if(snprintf(fname, BUFF_SZ, "data/%s/tree/tile_%d.atd", s->name, idx) < BUFF_SZ)
    {

	struct stat sbuff;
	if(!stat(fname, &sbuff))
	{
	    size_t item_count = sbuff.st_size / (sizeof(tree_t) - sizeof(tree_type_t *) + TREE_NAME_SZ);
	    printf("Load tile with %d items\n", item_count);
	    
	    size_t sz = sizeof(tree_tile_t) + sizeof(tree_t)*item_count;
	    res = calloc(sz,1);
	    res->count = item_count;
	    
	    FILE *f = fopen(fname, "r");
	    if(f)
	    {
		for(j=0; j<item_count; j++)
		{
		    char bnam[TREE_NAME_SZ];
		    
		    size_t r1 = fread(bnam, TREE_NAME_SZ, 1, f);
		    tree_t *tree = &res->tree[j];
		    size_t r2 = fread(
			((void *)tree) + offsetof(tree_t, pos), 
			sizeof(tree_t) - sizeof(tree_type_t *),
			1,
			f);
		    tree->type = tree_type_get(bnam);
		    
		    if((r1 != 1) || (r2 != 1))
		    {
			printf("Failed to load item\n");
			exit(1);
		    }
		}
		if(fclose(f)!=0)
		{
		    printf(
			"Failed to close file after loading item tile %d of scene %s\n",
			idx, s->name);
		    exit(1);
		}
		return res;
		
		
	    }
	}
    }    
    printf("Failed to load item tile %d of scene %s\n", idx, s->name);
    exit(1);
}

static ball_tile_t *scene_ball_tile_load(scene_t *s, ssize_t idx)
{
    return 0;
}

static int scene_find_missing_item_tile(scene_t *s)
{
    int my_x = s->camera.pos[0]/ITEM_TILE_SIZE;
    int my_y = s->camera.pos[1]/ITEM_TILE_SIZE;
    int x, y;
    int steps = 2*RENDER_DISTANCE/ITEM_TILE_SIZE;
    
    int side = ceilf(s->scene_size/ITEM_TILE_SIZE);

    for( x = my_x-steps; x <= my_x+steps; x++)
    {
	if( x < 0 || x >= side)
	    continue;
	
	for( y = my_y-steps; y <= my_y+steps; y++)
	{
	    if( y < 0 || y >= side)
	    continue;
	
	    int my_idx = x + y * side;
	    if(!s->tree_tile[my_idx])
		return my_idx;
	}
    }
    return SCENE_NONE;
}

/*
  If a tile needs loading, load it, wait for the main thread to allow
  updates, and insert it, return true.
  
  If a tile can be unloaded, wait for the main thread to allow updates
  and remove it, return true.
  
  Otherwise, return false.
*/
static int scene_try_load_tile(
    scene_t *s, int do_lock)
{
    scene_load_t *sl = (scene_load_t *)s->load_state;
    subtile_t sub;
    if(scene_find_missing_tile(s, &sub))
    {
	tile_t *t = scene_tile_load(s, &sub);
//	printf("Found tile to load %d\n", t);
	if(do_lock)
	{
	    pthread_mutex_lock(&sl->mutex);
	    pthread_cond_wait(&sl->convar, &sl->mutex);	    
	}
	
	scene_tile_insert(s, t, &sub);
	if(do_lock)
	{
	    pthread_mutex_unlock(&sl->mutex);	
	}	
	return 1;
    }
    
    int idx = scene_find_missing_item_tile(s);
    if(idx != SCENE_NONE)
    {
	printf("Found item tile to load at index %d\n", idx);
	
	tree_tile_t *tt = scene_tree_tile_load(s, idx);
	ball_tile_t *bt = scene_ball_tile_load(s, idx);
	
	if(do_lock)
	{
	    pthread_mutex_lock(&sl->mutex);
	    pthread_cond_wait(&sl->convar, &sl->mutex);	    
	}
	s->ball_tile[idx] = bt;
	s->tree_tile[idx] = tt;
	
	if(do_lock)
	{
	    pthread_mutex_unlock(&sl->mutex);	
	}	
	return 1;
    }
    
//	printf("No tiles needed\n");

    if(scene_find_unneeded_tile(s, &sub))
    {
	if(do_lock)
	{
	    pthread_mutex_lock(&sl->mutex);
	    pthread_cond_wait(&sl->convar, &sl->mutex);	    
	}
	
	tile_t *t = scene_tile_remove(s, &sub);
	
	if(do_lock)
	{
	    pthread_mutex_unlock(&sl->mutex);	
	}
	free(t);
	return 1;
    }
    
    return 0;
}

void scene_update(scene_t *s)
{
    if(s->load)
    {
	scene_load_t *sl = (scene_load_t *)s->load_state;
	pthread_mutex_lock(&sl->mutex);
	pthread_cond_signal(&sl->convar);
	pthread_mutex_unlock(&sl->mutex);
    }
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
	500000000
    };
    
    while(1)
    {
	sl->current_lap++;
	
	if(!scene_try_load_tile(s,1))
	{
	    nanosleep(&ts, 0);
	}	
    }
    return 0;
    
}


static void scene_load_init(scene_t *s)
{
    char fname[BUFF_SZ];
    subtile_t st = 
	{
	    {
		0
	    },
	    1
	}
    ;
    
    if(snprintf(fname, BUFF_SZ, "data/%s/scene.asd", s->name) < BUFF_SZ)
    {
	FILE *f = fopen(fname, "r");
	if(f)
	{
	    size_t read = fread(s, sizeof(scene_head_t), 1, f);
	    if((fclose(f)==0) && (read == 1))
	    {

		size_t item_tile_side = ceilf(s->scene_size/ITEM_TILE_SIZE);
		s->ball_tile = calloc(item_tile_side*item_tile_side, sizeof(ball_tile_t *));
		s->tree_tile = calloc(item_tile_side*item_tile_side, sizeof(tree_tile_t *));

		s->root_tile = scene_tile_load(s, &st);
		assert(s->root_tile);

		s->load_state = malloc(sizeof(scene_load_t));
		scene_load_t *sl = (scene_load_t *)s->load_state;
		hash_init(&sl->used, &hash_ptr_func, &hash_ptr_cmp);
		sl->current_lap = 0;
		
		while(scene_try_load_tile(s, 0))
		    ;
		
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
	hash_destroy( &sl->used );
	
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
    
    hash_init( &s->ball_type, &hash_str_func, &hash_str_cmp);

    s->ambient_light[0]=0.5;
    s->ambient_light[1]=0.5;
    s->ambient_light[2]=0.5;
    s->ambient_light[3]=1;
    s->camera_light[0] = 0.5;
    s->camera_light[1] = 0.5;
    s->camera_light[2] = 0.5;
    s->camera_light[3] = 1.0;
    
    s->render_quality=60.0;
    s->target_fps = 60;

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


void scene_configure(
    scene_t *s,
    int lb, float scene_size, 
    int max_tree_count, int max_ball_count )
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
	"%d tiles of %.2f kB each allocated. Used a total of %.2f megabytes of memory\n", 
	res/sizeof(tile_t), ((float)sizeof(tile_t))/1024,
	((double)res)/1024/1024);

    size_t item_tile_side = ceil(s->scene_size/ITEM_TILE_SIZE);
    s->ball_tile = calloc(item_tile_side*item_tile_side, sizeof(ball_tile_t *));
    s->tree_tile = calloc(item_tile_side*item_tile_side, sizeof(tree_tile_t *));
    int i;
    for(i=0; i<item_tile_side*item_tile_side; i++)
    {
	s->ball_tile[i] = calloc(sizeof(ball_tile_t) + max_ball_count*sizeof(ball_t), 1);
	
	s->tree_tile[i] = calloc(sizeof(tree_tile_t) + max_tree_count*sizeof(tree_t), 1);
    }
    printf(
	"Allocated %.2f MB item tile data in %d tiles\n", 
	((float)item_tile_side)*item_tile_side*
	(max_ball_count*sizeof(ball_t) + max_tree_count*sizeof(tree_t)) / 1024 / 1024,
	item_tile_side*item_tile_side);

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
    heightmap_element_t *he1 = scene_hid_lookup(s, hid);
    if(!he1)
    {
	slope[0]=0;
	slope[1]=0;
	return;	
    }
    
    float h1 = he1->height;
    /*
    printf(
	"LALA2 %.2f %.2f\n",
	(1.0-f1)*(1.0-f2)*scene_hid_lookup(s, hid)->height,
	height);
*/  
    //return scene_hid_lookup(s, hid)->height;
    
    HID_SET(hid, level, base_x+1, base_y);
    heightmap_element_t *he2 = scene_hid_lookup(s, hid);
    if(!he2)
    {
	slope[0]=0;
	slope[1]=0;
	return;	
    }
    float h2 = he2->height;
    
    HID_SET(hid, level, base_x, base_y+1);
    
    heightmap_element_t *he3 = scene_hid_lookup(s, hid);
    if(!he3)
    {
	slope[0]=0;
	slope[1]=0;
	return;	
    }
    float h3 = he3->height;
    
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
    heightmap_element_t *he = scene_hid_lookup(s, hid);
    //  printf("LALA1 %.2f\n",height);
    if(!he)
    {
	return 0;
    }    
    height = (1.0-f1)*(1.0-f2)*he->height;
    /*
    printf(
	"LALA2 %.2f %.2f\n",
	(1.0-f1)*(1.0-f2)*scene_hid_lookup(s, hid)->height,
	height);
*/  
    //return scene_hid_lookup(s, hid)->height;
    
    HID_SET(hid, level, base_x+x_inc, base_y);
    he = scene_hid_lookup(s, hid);
    if(!he)
    {
	return 0;
    }    
    height += (f1)*(1.0-f2)*he->height;
    
    HID_SET(hid, level, base_x, base_y+y_inc);
    
    he = scene_hid_lookup(s, hid);
    if(!he)
    {
	return 0;
    }    
    height += (1.0-f1)*(f2)*he->height;
    
    HID_SET(hid, level, base_x+x_inc, base_y+y_inc);
    he = scene_hid_lookup(s, hid);
    if(!he)
    {
	return 0;
    }    
    height += (f1)*(f2)*he->height;
    
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


void scene_save_items(scene_t *s)
{
    char fname[BUFF_SZ];
    size_t item_tile_side = ceilf(s->scene_size/ITEM_TILE_SIZE);
    int i, j;

    for(i=0; i<item_tile_side*item_tile_side; i++)
    {
	if(snprintf(fname, BUFF_SZ, "data/%s/tree/tile_%d.atd", s->name, i) < BUFF_SZ)
	{
	    assert(s->tree_tile[i]);

	    FILE *f = fopen(fname, "w");
	    if(f)
	    {
		for(j=0; j<s->tree_tile[i]->count; j++)
		{
		    
		    size_t w1 = fwrite(s->tree_tile[i]->tree[j].type->name, TREE_NAME_SZ, 1, f);
		    tree_t *tree = &s->tree_tile[i]->tree[j];
/*		    
		    printf("Saving item %d of %d...\n", j, s->tree_tile[i]->count);
		    printf("Name is %s\n", s->tree_tile[i]->tree[j].type->name);
		    printf("Address is %d\n", tree);
*/
		    size_t w2 = fwrite(
			((void *)tree) + offsetof(tree_t, pos), 
			sizeof(tree_t) - sizeof(tree_type_t *),
			1,
			f);
		    

		    if((w1 != 1) || (w2 != 1))
		    {
			printf("Failed to save item %d in item tile %d of scene %s\n", j, i, s->name);
			exit(1);
		    }
		}
		if(fclose(f)!=0)
		{
		    printf("Failed to close file after saving item tile %d of scene %s\n", i, s->name);
		    exit(1);
		}
		continue;
		
	    }
	}
	printf("Failed to save item tile %d of scene %s\n", i, s->name);
	exit(1);
    }    
}


int scene_tree_create(
    scene_t *s,
    char *tree_type_name,
    float *pos,
    float angle,
    float scale)
{
    int x_idx = pos[0] / ITEM_TILE_SIZE;
    int y_idx = pos[1] / ITEM_TILE_SIZE;
    
    int idx = x_idx + y_idx * ceilf(s->scene_size/ITEM_TILE_SIZE);
    
    tree_t *t = &(s->tree_tile[idx]->tree[s->tree_tile[idx]->count++]);
    
    t->pos[0] = pos[0];
    t->pos[1] = pos[1];
    t->pos[2] = scene_get_height(s, pos[0], pos[1]);
    //printf("Create tree at %f %f %f with angle %.2f\n", pos[0], pos[1], t->pos[2], angle);
    t->type = tree_type_get(tree_type_name);
    t->angle = angle;
    t->radius = 3.0;
    t->scale = 1.0;
    
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

void scene_ball_type_get(scene_t *s, char *n, ball_type_t ** pos)
{
    ball_type_t *t = hash_get(&s->ball_type, n);
    if(t)
    {
	*pos = t;
    }
    else
    {
	t = ball_type_load(s->name, n);
	hash_put(&s->ball_type, n, t);
	*pos = t;
    }
    
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
    
    scene_ball_type_get(s, ball_type_name, &(t->type));
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
	    size_t written = fwrite(t, sizeof(tile_t) - sizeof(tile_t *) * TILE_SUBTILE_COUNT, 1, f);
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

void scene_save_terrain(scene_t *s)
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

