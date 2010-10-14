#ifndef TREE_H
#define TREE_H

#include <GL/gl.h>	

#define TREE_SECTION_REGULAR 0
#define TREE_SECTION_JOINT 1
#define TREE_SECTION_POINTS 2

typedef struct 
{
    int type;
    
    GLuint texture_id;
    GLfloat width;
    GLfloat pos1[3];
    GLfloat pos2[3];
    GLfloat normal[3];
}
    tree_section_t;

typedef struct
{
    int type;
    GLfloat point_width;
    GLfloat cloud_width;
    GLfloat pos[3];
    GLfloat normal[2];
    GLubyte color[3];
    char count;
}
    tree_section_points_t;


typedef struct
{
    size_t section_count;
    char *name;
    tree_section_t section[];
}
    tree_type_t;

struct tree
{
    GLfloat pos[3];
    GLfloat angle;
    GLfloat radius;
    
    tree_type_t *type;
    GLfloat scale;
    int visible;    
};

typedef struct tree tree_t;

void tree_load_init(void);

tree_type_t *tree_type_get(char *name);


#endif
