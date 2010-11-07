#ifndef RENDER_H
#define RENDER_H

#include "scene.h"

#define DRAW 1
#define RENDER_DIST 1000

#define RENDER_PASS_SOLID 0
#define RENDER_PASS_TRANSPARENT 1
#define RENDER_PASS_COUNT 2

typedef struct
{
    float view_direction[3];
}
    render_state_t;

void render_init();

void render( scene_t *s );

void render_terrain_init();
void render_trees_init();
void render_balls_init();
void render_balls(scene_t *s);
void render_trees_trunk(scene_t *s);
void render_trees_leaves(scene_t *s);
void render_terrain_start(scene_t *s);
void render_terrain_finish(scene_t *s);
void render_boids(scene_t *s);


static inline float render_height_correct(GLfloat a, GLfloat b)
{
    return -0.002*(a*a+b*b);
}


#endif
