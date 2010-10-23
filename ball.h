#ifndef BALL_H
#define BALL_H

#define BALL_LEVEL_MAX 10

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
size_t ball_idx(int level, int x, int y);
void ball_calc(ball_type_t *b);



#endif
