#ifndef HEIGHTMAP_ELEMENT_H
#define HEIGHTMAP_ELEMENT_H

#include <GL/gl.h>

/**
   Data structure representing a heightmap element identifier. A
   heightmap element identifier encodes the level of detail (LOD) as
   well as the x and y coordinate of a terrain node. Treat this as an
   opaque data structure and use the macros below to access parts of
   the identifier.
 */
typedef struct
{
    unsigned int level;
    unsigned int id;
}
    hid_t;

#include "node.h"

/**
   Return the LOD level portion of the specified hid.
*/
#define HID_GET_LEVEL(hid) ((hid).level)

/**
   Set the LOD level portion of the specified hid.
*/
#define HID_SET_LEVEL(hid, l) ((hid).level=l)

#define HID_GET_X_POS(hid) ((hid).id % (2<<(hid).level))
#define HID_GET_Y_POS(hid) ((hid).id / (2<<(hid).level))

#define HID_SET(hid, lvl, x, y) do{(hid).level=lvl;((hid).id=((x)+((2<<(hid).level)*(y))));}while(0)
#define HID_SET_POS(hid, x, y) ((hid).id=((x)+((2<<(hid).level)*(y))))
#define HID_GET_IDX(hid) ((hid).id)

typedef struct
{
    GLfloat height;
    GLfloat normal[3];
    GLubyte color[3];
/*	GLfloat shadow_height; */
}
heightmap_element_t;

int hid_get_nid_at_level(hid_t hid, nid_t *nid, int level);


#endif
