#ifndef BOID_H
#define BOID_H


typedef struct 
{
    float pos[3];
    float vel[3];
}
    boid_t;

typedef struct
{
    int count;
    float speed;
    float target[3];
    boid_t data[];
}
    boid_set_t;

boid_set_t *boid_set_init(int count, float x, float y);
void boid_step(boid_set_t *b, float dt);

#endif
