#ifndef BALL_H
#define BALL_H

#include "util.h"
#include "vertex_data.h"

#define BALL_LEVEL_MAX 8
#define BALL_NAME_MAX 30
#define BALL_NAME_SZ (BALL_NAME_MAX+1)

typedef struct
{
    float radius;
    GLubyte color[3];
}
    ball_point_t;

typedef struct
{
    size_t levels;
    float error[BALL_LEVEL_MAX];
    vertex_data_t prerender[BALL_LEVEL_MAX];
    char name[BALL_NAME_SZ];
    GLubyte alpha;
    ball_point_t data[];
}
    ball_type_t;

typedef struct 
{
    GLfloat transform[16];
    GLfloat scale;
    ball_type_t *type;
    GLfloat radius;
    int visible;    
}
    ball_t;

void ball_load_init();
ball_type_t *ball_type_get(char *name);

static inline size_t ball_point_count(int levels)
{
    return 2*(0x55555555 & ((1<<(2*(levels+1)))-1));
}

static inline size_t ball_offset(int level)
{
    return ball_point_count(level-1);
}

static inline size_t ball_idx(int level, int x, int y)
{
    return ball_offset(level) + x + (y << (level+1));
}

void ball_calc(ball_type_t *b);
void ball_type_set(ball_type_t *ball, int x, int y, float h, float r, float g, float b);
void ball_type_save(ball_type_t *b, char *dir, char *fn);

ball_type_t *ball_type_load(char *dir, char *name);
ball_type_t *ball_type_create(size_t level, char *name, GLubyte alpha, allocfn_t alloc);
void ball_type_calc(ball_type_t *b);

void render_ball_type_prerender(ball_type_t *b);


#endif
