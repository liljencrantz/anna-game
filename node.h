#ifndef NODE_H
#define NODE_H

#include <GL/glew.h>
#include "view.h"

/**
   Data structure representing a node identifier. A node identifier
   encodes the level of detail (LOD) as well as the x and y coordinate
   of a terrain node. Treat this as an opaque data structure and use
   the macros below to access parts of the identifier.
 */
typedef struct
{
    unsigned int level;
    unsigned int id;
}
    nid_t;

#include "heightmap_element.h"

/**
   Return the LOD level portion of the specified nid.
*/
#define NID_GET_LEVEL(nid) ((nid).level)

/**
   Set the LOD level portion of the specified nid.
*/
#define NID_SET_LEVEL(nid, l) ((nid).level=l)

#define NID_GET_X_POS(nid) ((nid).id % (1<<(nid).level))
#define NID_GET_Y_POS(nid) ((nid).id / (1<<(nid).level))

#define NID_SET(nid, lvl, x, y) do{(nid).level=lvl;((nid).id=((x)+((1<<(nid).level)*(y))));}while(0)

#define NID_SET_POS(nid, x, y) ((nid).id=((x)+((1<<(nid).level)*(y))))
#define NID_GET_IDX(nid) ((nid).id)

typedef struct
{
    /**
       An estimate of the error level from using this node instead of
       using the most detailed level. Must be 0 for leaf nodes. (Leaf
       nodes in the context of the entire node tree, not a specific
       rendering pass, which will use some subset of the whole node tree)
       This value remains constant unless the heightmap changes.

       The distortion value is calculated during the lod generation pass.
    */
    GLfloat distortion;
    
    /**
       Distance to closest position on node. Usually updated on every render.
    */
    GLfloat distance;

    /**
       How to render the specified node.
       
       The value of the scale variable is undefined for unused nodes,
       must be zero for non-leaf nodes, negative for nodes outside of
       the viewing area, and positive but less than or equal to 1.0
       for leaf nodes. (Leaf nodes in the context of a specific
       rendering pass, not in the context of the entire node tree)
       
       A scale of 1.0 means that the node will be rendered
       normally. If the render strength is less than 1.0, it means the
       node will be partially flattened to make a continous animation
       from this node level and the parent node.

    */
    GLfloat scale;    
}
t_node_t;

nid_t node_get_root();

GLfloat node_get_error(t_node_t *n, view_t *pos);
void nid_get_children(nid_t nid, nid_t *child_nid);
void nid_get_hid(nid_t nid, hid_t *hid);
int nid_is_edge(nid_t nid);


#endif
