#ifndef ACTOR_H
#define ACTOR_H

#include "SDL/SDL_opengl.h"

#include "scene.h"

#define ACTION_WALK_FORWARD 1
#define ACTION_WALK_BACKWARD 2
#define ACTION_TURN_LEFT 4
#define ACTION_TURN_RIGHT 8
#define ACTION_RUN_FORWARD 16
#define ACTION_STRAFE_LEFT 32
#define ACTION_STRAFE_RIGHT 64
#define ACTION_PHYSICAL_ATTACK 128
#define ACTION_DODGE 256
#define ACTION_MAGICAL_ATTACK 512

struct actor
{
    float pos[3];
    float angle;
    float width;
    float velocity[3];
    long long action;
    scene_t *scene;
    tile_t *tile;
    char *name;
};

typedef struct actor actor_t;

void actor_set_action(actor_t *p, int action, int val);
void actor_do_step(actor_t *p, scene_t *s, float timestep);
void actor_set_pos(actor_t *p, float x, float y);

#endif
