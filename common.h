#ifndef COMMON_H
#define COMMON_H

//#include "SDL/SDL_opengl.h"
#include "node.h"
#include "heightmap_element.h"

#define TILE_EMPTY 0
#define TILE_SEEDED 1
#define TILE_ACTIVE 2
#define TILE_HEIGHTMAP 3
#define TILE_READY 4

/*  Directions. 3-direction is the opposite direction  */
#define NORTH 0
#define EAST 1
#define WEST 2
#define SOUTH 3

struct actor;

typedef struct
{
    GLfloat pos[3];
    GLfloat direction;
    GLfloat velocity[3];
}
object_t;


/* Global variables */

extern int alt_camera;
extern int rendered_objs;

/* Code for the simulation (simulator.c) */
//void scene_step( scene_t *s, float dt );
//void move_camera( scene_t *s );

int randi( int min, int max );
GLfloat randf();
GLfloat gaussian();

#endif
