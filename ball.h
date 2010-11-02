#ifndef BALL_H
#define BALL_H

#define BALL_LEVEL_MAX 8

typedef struct
{
    float radius;
    GLubyte color[3];
}
    ball_point_t;

typedef struct
{
    size_t levels;
    float *error;
    GLuint list_index;
    ball_point_t data[];
}
    ball_type_t;

typedef struct 
{
    GLfloat pos[3];
    GLfloat offset[3];
    GLfloat angle1;
    GLfloat angle2;
    GLfloat angle3;
    GLfloat radius;
    
    ball_type_t *type;
    GLfloat scale;
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
ball_type_t *ball_type_create(size_t level);
void ball_type_set(ball_type_t *ball, int x, int y, float h, float r, float g, float b);
void ball_type_save(ball_type_t *b, char *fn);
ball_type_t *ball_type_get(char *name);

#endif
