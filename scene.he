#ifndef SCENE_H
#define SCENE_H

#include <stdio.h>
#include <stdlib.h>

#include "tile.h"
#include "view.h"
#include "tree.h"
#include "ball.h"
#include "boid.h"

/**
   The scene object is rather large, because it contains statically
   allocated memory used for storing trees, balls, boids, etc. The
   reason for statically allocating these is to allow adding of simple
   objects but avoiding memory allocations in the main thread.
 */

typedef struct
{
    tile_t *root_tile;
    int level_base;
    float sun_pos[3];
    float ambient_light;
    float sun_light;
    float scene_size;
    double time;
    float render_quality;
    
    view_t camera;	

    float leaf_offset;
    float grass_offset;
    
    struct actor *player;
    
    void *lua_state;
    void *terrain_state;
        
    tree_t tree[4096];
    size_t tree_count;
    
    ball_t ball[4096];
    size_t ball_count;
    
    boid_set_t *boid_set[16];
    size_t boid_set_count;
    
}
scene_t;

#define scene_nid_lookup(s,nid) (tile_nid_lookup((s)->root_tile, nid))
#define scene_hid_lookup(s,hid) (tile_hid_lookup((s)->root_tile, hid))

heightmap_element_t *scene_heighmap_get(
    scene_t *s, 
    hid_t hid);

t_node_t *scene_node_get(
    scene_t *s, 
    nid_t nid);

float scene_get_height( scene_t *s, float xf, float yf );
void scene_get_slope( scene_t *s, float xf, float yf, float *slope);
float scene_get_height_level( scene_t *s, int level, float xf, float yf );

void scene_init(scene_t *s, int lb, float scene_size);

float scene_hid_x_coord(scene_t *s,hid_t hid);
float scene_hid_y_coord(scene_t *s,hid_t hid);

/**
   Returns the position of the middle of the specified node.
 */
void scene_nid_coord(scene_t *s, nid_t nid, float *dst);
/**
  Returns the (possibly approximated) distance from the specified
  position to the closespoint inside of the node specified.
 */
float scene_nid_min_distance(scene_t *s, nid_t nid, view_t *pos);

float scene_nid_radius(scene_t *s, nid_t nid);


int scene_nid_is_visible(scene_t *s, nid_t nid, view_t *pos);

int scene_is_visible(scene_t *s, float *pos, float radius);

//void get_nabla_height( scene_t *s, float xf, float yf, float *arr );

//tile_t *get_tile( scene_t *s, int x, int y );
//int get_tile_count( scene_t *s );
//void get_all_tiles( scene_t *s, tile_t ** );

static inline void scene_tree_destroy(
    scene_t *s, 
    int tid)
{
    s->tree[tid] = s->tree[--s->tree_count];
}


static inline tree_t *scene_tree_get(scene_t *s, size_t idx)
{
    return &s->tree[idx];
}

static inline size_t scene_tree_get_count(scene_t *s)
{
    return s->tree_count;
}

static inline int scene_tree_create(
    scene_t *s,
    char *tree_type_name,
    float *pos,
    float angle,
    float scale)
{
    tree_t *t = &s->tree[s->tree_count];
    
    t->pos[0] = pos[0];
    t->pos[1] = pos[1];
    t->pos[2] = scene_get_height(s, pos[0], pos[1]);
    //printf("Create tree at %f %f %f with angle %.2f\n", pos[0], pos[1], t->pos[2], angle);
    t->type = tree_type_get(tree_type_name);
    t->angle = angle;
    t->radius = 3.0;
    t->scale = 1.0;
    return s->tree_count++;
}

static inline void scene_ball_destroy(
    scene_t *s, 
    int tid)
{
    s->ball[tid] = s->ball[--s->ball_count];
}

static inline ball_t *scene_ball_get(scene_t *s, size_t idx)
{
    return &s->ball[idx];
}

static inline size_t scene_ball_get_count(scene_t *s)
{
    return s->ball_count;
}

static inline int scene_ball_create(
    scene_t *s,
    char *ball_type_name,
    float scale)
{
    ball_t *t = &s->ball[s->ball_count];
    
    t->pos[0] = 0;
    t->pos[1] = 0;
    t->pos[2] = 0;

//    printf("Create ball at %f %f %f, scale is %.2f\n", pos[0], pos[1], t->pos[2], scale);
    t->type = ball_type_get(ball_type_name);
    t->angle1 = 0.0;
    t->angle2 = 0.0;
    t->scale = scale;
    return s->ball_count++;
}


static inline void scene_boid_set_destroy(
    scene_t *s, 
    int tid)
{
    free(s->boid_set[tid]);
    s->boid_set[tid] = s->boid_set[--s->boid_set_count];
}


static inline boid_set_t *scene_boid_set_get(scene_t *s, size_t idx)
{
    return s->boid_set[idx];
}

static inline size_t scene_boid_set_get_count(scene_t *s)
{
    return s->boid_set_count;
}

static inline int scene_boid_set_create(
    scene_t *s,
    int count,
    float x,
    float y)
{
    s->boid_set[s->boid_set_count] = boid_set_init(count, x, y);
    return s->boid_set_count++;
}

#endif

