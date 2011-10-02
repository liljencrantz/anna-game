#ifndef HEIGHTMAP_ELEMENT_H
#define HEIGHTMAP_ELEMENT_H

#include <GL/glew.h>

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

static inline int hid_get_nid_at_level(hid_t hid, nid_t *nid, int level)
{
    int hid_x = HID_GET_X_POS(hid);
    int hid_y = HID_GET_Y_POS(hid);
    int hid_level = HID_GET_LEVEL(hid);
    int w_diff = hid_level-level+1;
    int nid_x_width=1;
    int nid_x = hid_x >> w_diff;
    int nid_y_width=1;
    int nid_y = hid_y >> w_diff;
    if((nid_x<<w_diff == hid_x) && nid_x != 0)
    {
	nid_x--;
	nid_x_width=2;
    }
    if((nid_y<<w_diff == hid_y) && nid_y != 0)
    {
	nid_y--;
	nid_y_width=2;
    }
    int i,j;
    int count = 0;
    
    for(i=0; i<nid_x_width; i++)
    {
	for(j=0; j<nid_y_width; j++)
	{
	    NID_SET(nid[count], level, nid_x+i, nid_y+j);
	    count++;
	}
	
    }

    return count;
    
}


static inline hid_t hid(int lvl, int x, int y)
{
    hid_t h;
    h.level=lvl;
    h.id=x+((2<<h.level)*y);
    return h;
}

#endif
