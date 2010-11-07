#ifndef TREE_H
#define TREE_H

#include <GL/glew.h>	

#define TREE_SECTION_REGULAR 0
#define TREE_SECTION_JOINT 1
#define TREE_SECTION_POINTS 2

typedef struct 
{
    int count;
    float data[][3];
}
    tree_section_type_t;

typedef struct 
{
    tree_section_type_t *type;
    GLfloat width;
    GLfloat length;
    GLfloat pos[3];
    GLfloat normal[3];
}
    tree_section_t;


typedef struct
{
    size_t section_count;
    char *name;
    tree_section_t section[];
}
    tree_type_t;

struct tree
{
    tree_type_t *type;

    GLfloat pos[3];
    GLfloat angle;
    GLfloat radius;
    
    GLfloat scale;
    int visible;    
};

typedef struct tree tree_t;

void tree_load_init(void);

tree_type_t *tree_type_get(char *name);


#endif
