#ifndef VERTEX_DATA_H
#define VERTEX_DATA_H

/**
   This file contains a library for using VBOs for drawing things on
   screen. It is very limited in that it only sdupports the types of
   drawing that Anna is currently interested in. Specifically:

   * Only one vertex format is currently supported, drawing with a color (GLubyte), a normal (GLfloat) and a position (GLfloat) per vertex, and specifying indices as GLushort. In the future, a small number of additional formats, such as supplying a texture coordinate in stead of/in additon to a color is likely to be added.

   * Two operating modes are currently supported, streaming or static. For static drawing, use vd_generate to create the vbo data, and then draw it as often as you wish using vd_draw. For streaming mode, call vd_stream to both send the data to the gpu and draw it. 
 */


typedef struct
{
    GLfloat pos[3];
    GLfloat normal[3];
    GLubyte color[4];
}
    vertex_element_t;

typedef struct 
{
    vertex_element_t *vertex;
    GLushort *index;
    int idx_count;
    int vertex_count;
    GLuint vbo;
    GLuint ibo;
}
    vertex_data_t;

/**
   Initialize a vd with room for the specified number of verices and indices.
*/
void vd_init(
    vertex_data_t *vd, 
    int vertex_count,
    int index_count);

/**
   Reset index/vertex counters. This only makes sense when reusing a
   vd, e.g. when using vd_stream.
*/
void vd_reset(
    vertex_data_t *vd );

/**
   Generate the VBO without freeing the temp buffers and then draw it
   using the specified element type
*/
void vd_stream(
    vertex_data_t *vd,
    int type);

/**
   Generate the VBO and free temp buffers
*/
void vd_generate(
    vertex_data_t *vd);

/**
   Draw the VBO using the specified element type
*/
void vd_draw(
    vertex_data_t *vd,
    int type);



/**
   Add a vertex to the list using vector arguments
*/
static inline void vd_add_vertex_v(
    vertex_data_t *vd, 
    float *pos, float *normal, GLubyte *color)
{    
    memcpy(vd->vertex[vd->vertex_count].pos, pos, sizeof(float)*3);
    memcpy(vd->vertex[vd->vertex_count].normal, normal, sizeof(float)*3);
    memcpy(vd->vertex[vd->vertex_count].color, color, sizeof(GLubyte)*4);
    vd->vertex_count++;   
}

/**
   Add a vertex to the list using separate arguments for each component
*/
static inline void vd_add_vertex_a(
    vertex_data_t *vd, 
    float p_x, float p_y, float p_z,
    float n_x,
    float n_y,
    float n_z,
    GLubyte r,
    GLubyte g,
    GLubyte b,
    GLubyte a)
{
/*
    printf("%f %f %f    %f %f %f      %d %d %d %d\n",
	   p_x, p_y, p_z,
	   n_x,n_y,n_z,
	   r,g,b,a);
*/
    vd->vertex[vd->vertex_count].pos[0] = p_x;
    vd->vertex[vd->vertex_count].pos[1] = p_y;
    vd->vertex[vd->vertex_count].pos[2] = p_z;
    vd->vertex[vd->vertex_count].normal[0] = n_x;
    vd->vertex[vd->vertex_count].normal[1] = n_y;
    vd->vertex[vd->vertex_count].normal[2] = n_z;
    vd->vertex[vd->vertex_count].color[0] = r;
    vd->vertex[vd->vertex_count].color[1] = g;
    vd->vertex[vd->vertex_count].color[2] = b;
    vd->vertex[vd->vertex_count].color[3] = a;
    vd->vertex_count++;
}

/**
   Add a vertex indefto the list
*/

static inline void vd_add_index(
    vertex_data_t *vd, 
    int idx)
{
    vd->index[vd->idx_count++] = idx;
}

#endif
