#include <math.h>

#include "scene.h"
#include "actor.h"

void actor_set_action(actor_t *p, int action, int val)
{
    if(val)
	p->action |= action;
    else
	p->action &= ~action;
}

int actor_get_action(actor_t *p, int action)
{
    return !!(p->action & action);
}

void actor_set_pos(actor_t *p, float x, float y)
{
/*    
    tile_t *t = scene_get_tile(p->scene, x, y);
    if(t != p->tile)
    {
	tile_remove_actor(p->tile, p);
	tile_add_actor(p->tile, p);
    }
*/  
    p->pos[0] = x;
    p->pos[1] = y;
    p->pos[2] = scene_get_height(p->scene, x, y);
}


void actor_do_step(actor_t *p, scene_t *s ,float timestep)
{
    if(p->action&ACTION_TURN_LEFT)
    {
	p->angle += timestep;
    }
    if(p->action&ACTION_TURN_RIGHT)
    {
	p->angle -= timestep;
    }
    
    if(p->action&ACTION_WALK_FORWARD)
    {
	GLfloat dist = timestep*1.5;
	p->pos[0] += cos(p->angle)*dist;
	p->pos[1] += sin(p->angle)*dist;
	p->pos[2] = scene_get_height(s, p->pos[0], p->pos[1]);
	
    }
    if(p->action&ACTION_WALK_BACKWARD)
    {
	GLfloat dist = timestep*0.5;
	p->pos[0] -= cos(p->angle)*dist;
	p->pos[1] -= sin(p->angle)*dist;
	p->pos[2] = scene_get_height(s, p->pos[0], p->pos[1]);
    }
    
}
