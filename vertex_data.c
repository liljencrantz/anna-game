#include <assert.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <GL/glew.h>

#include "vertex_data.h"

void vd_init(
    vertex_data_t *vd, 
    int vertex_count,
    int index_count)
{
    vd->vertex = malloc(sizeof(vertex_element_t)*vertex_count);
    vd->index = malloc(sizeof(GLushort)*index_count);
    vd->idx_count=0;
    vd->vertex_count=0;
}

void vd_reset(
    vertex_data_t *vd )
{
    vd->idx_count=0;
    vd->vertex_count=0;
}

void vd_stream(
    vertex_data_t *vd,
    int type)
{
    // bind the buffer object to use
    glBindBuffer(GL_ARRAY_BUFFER, vd->vbo);
    const GLsizeiptr vertex_size = vd->vertex_count*sizeof(vertex_element_t);
    glBufferData(GL_ARRAY_BUFFER, vertex_size, vd->vertex, GL_STREAM_DRAW);

    // Describe to OpenGL where the vertex data is in the buffer 
    glVertexPointer(3, GL_FLOAT, sizeof(vertex_element_t), 0); 
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex_element_t), (void *)offsetof(vertex_element_t, color));
    glNormalPointer(GL_FLOAT, sizeof(vertex_element_t), (void *)offsetof(vertex_element_t, normal));
    
    // create index buffer 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vd->ibo); 
    glBufferData(
	GL_ELEMENT_ARRAY_BUFFER,
	vd->idx_count*sizeof(GLuint), 
	vd->index, 
	GL_STREAM_DRAW);
    

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vd->ibo); 

    // Activate the VBOs to draw 
    //   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); 
    glEnableClientState(GL_VERTEX_ARRAY); 
    glEnableClientState(GL_COLOR_ARRAY); 
    glEnableClientState(GL_NORMAL_ARRAY); 
    
    // This is the actual draw command 
    glDrawElements(
	type,
	vd->idx_count,
	GL_UNSIGNED_SHORT,
	(GLvoid*)((char*)NULL));
    
}

void vd_generate(
    vertex_data_t *vd)
{
    // bind the buffer object to use
    glBindBuffer(GL_ARRAY_BUFFER, vd->vbo);
    const GLsizeiptr vertex_size = vd->vertex_count*sizeof(vertex_element_t);
    glBufferData(GL_ARRAY_BUFFER, vertex_size, vd->vertex, GL_STREAM_DRAW);

    // Describe to OpenGL where the vertex data is in the buffer 
    glVertexPointer(3, GL_FLOAT, sizeof(vertex_element_t), 0); 
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex_element_t), (void *)offsetof(vertex_element_t, color));
    glNormalPointer(GL_FLOAT, sizeof(vertex_element_t), (void *)offsetof(vertex_element_t, normal));
    
    // create index buffer 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vd->ibo); 
    glBufferData(
	GL_ELEMENT_ARRAY_BUFFER,
	vd->idx_count*sizeof(GLuint), 
	vd->index, 
	GL_STREAM_DRAW);
    
    free(vd->vertex);
    free(vd->index);
}

void vd_draw(
    vertex_data_t *vd,
    int type)
{
    // bind the buffer object to use
    glBindBuffer(GL_ARRAY_BUFFER, vd->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vd->ibo); 

    glEnableClientState(GL_VERTEX_ARRAY); 
    glEnableClientState(GL_COLOR_ARRAY); 
    glEnableClientState(GL_NORMAL_ARRAY); 
    
    // This is the actual draw command 
    glDrawElements(
	type,
	vd->idx_count,
	GL_UNSIGNED_SHORT,
	(GLvoid*)((char*)NULL));
    
}
