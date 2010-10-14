#ifndef VIEW_H
#define VIEW_H

#include "SDL/SDL_opengl.h"

typedef struct
{
    GLfloat lr_rot, ud_rot, s_rot;
    GLfloat pos[3];

    GLfloat k[4];
    GLfloat m[4];
    int side[4];
    int planes;
}
view_t;

#endif
