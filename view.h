#ifndef VIEW_H
#define VIEW_H

//#include "SDL/SDL_opengl.h"

typedef struct
{
    float lr_rot, ud_rot, s_rot;
    float pos[3];

    float k[4];
    float m[4];
    int side[4];
    int planes;
}
view_t;

#endif
