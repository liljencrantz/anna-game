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

typedef void(*render_function_t)(scene_t *s);
//typedef void(*render_element_t)(void *element, scene_t *scene, render_state_t *state)

void render_init();

void render_register(render_function_t f, int pass);

void render( scene_t *s );

void render_terrain_init();
void render_trees_init();
void render_actors_init();

GLfloat render_height_correct(GLfloat a, GLfloat b);

#endif
