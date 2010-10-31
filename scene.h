#ifndef SCENE_H
#define SCENE_H

#include <stdio.h>
#include <stdlib.h>

#include "tile.h"
#include "view.h"
#include "tree.h"
#include "ball.h"
#include "boid.h"

#define SCENE_TREE_MAX 4096
#define SCENE_BALL_MAX 8192
#define SCENE_BOID_SET_MAX 16

#define SCENE_NAME_MAX 31
#define SCENE_NAME_SZ (SCENE_NAME_MAX+1)

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
    char name[SCENE_NAME_SZ];
    int load;
    
    view_t camera;	
    
    float leaf_offset;
    float grass_offset;
    
    struct actor *player;
    
    void *lua_state;
    void *terrain_state;
    
    tree_t tree[SCENE_TREE_MAX];
    int tree_used[SCENE_TREE_MAX/32];
    size_t tree_search_start;
    size_t tree_count;
    
    ball_t ball[SCENE_BALL_MAX];
    int ball_used[SCENE_BALL_MAX/32];
    size_t ball_search_start;
    size_t ball_count;
    
    boid_set_t *boid_set[SCENE_BOID_SET_MAX];
    int boid_set_used[SCENE_BOID_SET_MAX/32];
    size_t boid_set_search_start;
    size_t boid_set_count;    
}
scene_t;

/**
   Return the terrain node with the specified nid
 */
#define scene_nid_lookup(s,nid) (tile_nid_lookup((s)->root_tile, nid))
/**
   Return the heightmap element with the specified hid
 */
#define scene_hid_lookup(s,hid) (tile_hid_lookup((s)->root_tile, hid))

/**
   Returns the height of the world at the specified position
 */
float scene_get_height( scene_t *s, float xf, float yf );
/**
   Returns the slope of the world at the specified position.

   \param slope is the destination value, a two element vector.
 */
void scene_get_slope( scene_t *s, float xf, float yf, float *slope);
float scene_get_height_level( scene_t *s, int level, float xf, float yf );

/**
   Initialize a new scene. If the load flag is set, a background
   thread will be created that autoloades the relevant bits of scene
   depending on the camera point.
 */
void scene_init(scene_t *s, char *name, int load);

void scene_configure(scene_t *s, int tile_levels, float scene_size);
void scene_save(scene_t *s);


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

void scene_tree_destroy(
    scene_t *s, 
    int tid);
tree_t *scene_tree_get(scene_t *s, size_t idx);
size_t scene_tree_get_count(scene_t *s);
int scene_tree_create(
    scene_t *s,
    char *tree_type_name,
    float *pos,
    float angle,
    float scale);

void scene_ball_destroy(
    scene_t *s, 
    int tid);
ball_t *scene_ball_get(scene_t *s, size_t idx);
size_t scene_ball_get_count(scene_t *s);
int scene_ball_create(
    scene_t *s,
    char *ball_type_name,
    float scale);

void scene_boid_set_destroy(
    scene_t *s, 
    int tid);
boid_set_t *scene_boid_set_get(scene_t *s, size_t idx);
size_t scene_boid_set_get_count(scene_t *s);
int scene_boid_set_create(
    scene_t *s,
    int count,
    float x,
    float y);

#endif

