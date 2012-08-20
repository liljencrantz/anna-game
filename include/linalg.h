#ifndef ANNA_GAME_LINALG_H
#define ANNA_GAME_LINALG_H

struct transform
{
    float arr[16];
};

struct vec
{
    float arr[4];
};

static inline void transform_init(struct transform *t)
{
    memset(t, 0, sizeof(struct transform));
    t->arr[0] = 1.0;
    t->arr[5] = 1.0;
    t->arr[10] = 1.0;
    t->arr[15] = 1.0;
}

static inline float transform_get(struct transform *t, int idx1, int idx2)
{
    return t->arr[4*idx1+idx2];
}

static inline float transform_set(struct transform *t, int idx1, int idx2, float value)
{
    return t->arr[4*idx1+idx2] = value;
}

#endif

