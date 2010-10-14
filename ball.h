#ifndef BALL_H
#define BALL_H

typedef struct
{
    float radius;
    float normal[3];
    GLubyte color[3];
}
    ball_point_t;

typedef struct
{
    float *error;
    ball_point_t *data;
}
    ball_type_t;

typedef struct 
{
    GLfloat pos[3];
    GLfloat angle;
    GLfloat radius;
    
    ball_type_t *type;
    GLfloat scale;
    int visible;    
}
    ball_t;

void ball_load_init();
ball_type_t *ball_type_get(char *name);


#endif
