#include <math.h>

#include "node.h"
//#include "scene.h"
#include "heightmap_element.h"


nid_t node_get_root()
{
    nid_t root = 
	{
	    0, 0
	}
    ;
    return root;
}


GLfloat node_get_error(t_node_t *n, view_t *pos)
{
    return n->distortion/(0.0001+n->distance);
}

void nid_get_children(nid_t nid, nid_t *child_nid)
{
    int x = 2*NID_GET_X_POS(nid);
    int y = 2*NID_GET_Y_POS(nid);
    int level = NID_GET_LEVEL(nid)+1;
    NID_SET(child_nid[0], level, x, y);
    NID_SET(child_nid[1], level, x+1, y);
    NID_SET(child_nid[2], level, x, y+1);
    NID_SET(child_nid[3], level, x+1, y+1);    
}

void nid_get_hid(nid_t nid, hid_t *hid)
{
    int x = 2*NID_GET_X_POS(nid);
    int y = 2*NID_GET_Y_POS(nid);
    int level = NID_GET_LEVEL(nid);
    int i, j;
    HID_SET(hid[0], level, x, y+1);
    HID_SET(hid[1], level, x, y);
    HID_SET(hid[2], level, x+1, y);
    HID_SET(hid[3], level, x+2, y);
    HID_SET(hid[4], level, x+2, y+1);
    HID_SET(hid[5], level, x+2, y+2);
    HID_SET(hid[6], level, x+1, y+2);
    HID_SET(hid[7], level, x, y+2);
    HID_SET(hid[8], level, x+1, y+1);

}

int nid_is_edge(nid_t nid)
{
    int x = NID_GET_X_POS(nid);
    int y = NID_GET_Y_POS(nid);
    int level = NID_GET_LEVEL(nid);    
    int w = (1<<level)-1;
    return (x==w)||(y==w);
}


