


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




static inline void vd_add_vertex(
    vertex_data_t *vd, 
    float *pos, float *normal, GLubyte *color)
{
    
    memcpy(vd->vertex[vd->vertex_count].pos, pos, sizeof(float)*3);
    memcpy(vd->vertex[vd->vertex_count].normal, normal, sizeof(float)*3);
    memcpy(vd->vertex[vd->vertex_count].color, color, sizeof(GLubyte)*4);
    vd->vertex_count++;
    
}


static inline void vd_add_index(
    vertex_data_t *vd, 
    int idx)
{
    vd->index[vd->idx_count++] = idx;
}



