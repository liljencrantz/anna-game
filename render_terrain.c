#include <assert.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <GL/glew.h>
//#include <GL/glew.h>

#include "scene.h"
#include "node.h"
#include "util.h"
#include "render.h"
#include "heightmap_element.h"

#define GRASS_MAX_DISTANCE 15.0
#define GRASS_HEIGHT 0.25
#define GRASS_WIDTH 0.25
#define GRASS_INTERVAL 0.20
#define GRASS_WIND_AMPLITUDE 0.05
#define GRASS_WIND_FREQUENCY 5.0

#define MIDDLE_ELEMENT 8
#define TERRAIN_SCALE_THRESHOLD 1.2

typedef struct
{
    GLfloat pos[3];
//    GLfloat normal[3];
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


typedef struct
{
    GLfloat direction[3];
    GLfloat vertex[3];
    GLfloat normal[3];
    GLfloat shade;
    GLubyte color[3];
        
    int use;
    
}

    render_element_t;

typedef struct 
{
    scene_t *s;
    vertex_data_t vd;
}
    thread_data_t;

static pthread_t render_terrain_thread;
static pthread_mutex_t render_terrain_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t render_terrain_convar = PTHREAD_COND_INITIALIZER;

/*
static GLfloat node_norm_factor(nid_t nid, scene_t *s, GLfloat *pos)
{
    hid_t hid;
    HID_SET(hid, NID_GET_LEVEL(nid), NID_GET_X_POS(nid)*2, NID_GET_Y_POS(nid)*2);
    heightmap_element_t *el = scene_hid_lookup(s, hid);

    GLfloat coord[]=
	{
	    scene_hid_x_coord(s, hid),
	    scene_hid_y_coord(s, hid),
	    el->height
	}
    ;
    subtract(coord, pos, coord, 3);
    normalize(coord, coord, 3);
    GLfloat angle = fabs(dot_prod(coord, el->normal, 3));
//    printf("%f\n", angle);

    
    return angle < 0.25 ? 1.2:1.0;    
}
*/


static int render_prepare_node(scene_t *s, nid_t nid, float parent_level)
{
    t_node_t *n = scene_nid_lookup(s, nid);
    nid_t child_id[4];
    float error;
    int i;
    
    if(!n)
    {
	return 0;
    }
    
    if(scene_nid_is_visible(s, nid, &s->camera))
    {
	n->distance = scene_nid_min_distance(s, nid, &s->camera);
	error = node_get_error(n, &s->camera)*s->render_quality;
	//printf("Distance: %.2f, distortion is %.2f, error is %.2f\n", n->distance, n->distortion, error);
	
	if(error < 1.0)
	{
	    n->scale = parent_level;
	    return 1;
	}
	else
	{
	    int res = 0;
	    
	    float my_level = error;//maxf(0.00001, minf(1.0, (1.0/error-1.0)*5.0));
	    n->scale = 0.0;
	    nid_get_children(nid, child_id);
	    for(i=0; i<4; i++)
	    {
		res += render_prepare_node(s, child_id[i], my_level);
	    }
	    return res;
	}	    
    }
    else
    {
	n->scale = -1.0;
    }
    return 0;
}

static void vertex_perspective( scene_t *s, render_element_t *el, float *pos, vertex_data_t *vd, int idx0)
{
    float a = el->vertex[0]-pos[0];
    float b = el->vertex[1]-pos[1];
    if (DRAW)
    {
	vd->vertex[vd->vertex_count].pos[0] = el->vertex[0];
	vd->vertex[vd->vertex_count].pos[1] = el->vertex[1];
	vd->vertex[vd->vertex_count].pos[2] = el->vertex[2]+render_height_correct(a,b);
	
	vd->vertex[vd->vertex_count].color[0] = el->color[0]*el->shade;
	vd->vertex[vd->vertex_count].color[1] = el->color[1]*el->shade;
	vd->vertex[vd->vertex_count].color[2] = el->color[2]*el->shade;
	vd->vertex[vd->vertex_count].color[3] = 255;
	
	if(vd->vertex_count > idx0+1)
	{
	    vd->index[vd->idx_count++] = idx0;
	    vd->index[vd->idx_count++] = vd->vertex_count-1;
	    vd->index[vd->idx_count++] = vd->vertex_count;
	}
	
	vd->vertex_count++;
/*
	glColor3ub( el->color[0]*el->shade, el->color[1]*el->shade, el->color[2]*el->shade);    
//	printf( "%f %f %f\n", el->color[0]*el->shade, el->color[1]*el->shade, el->color[2]*el->shade);    
	glVertex3f( el->vertex[0], el->vertex[1], el->vertex[2]+render_height_correct(a,b));
//	printf("%f %f %f\n", el->vertex[0], el->vertex[1], el->vertex[2]+render_height_correct(a,b));
*/
    }
}



static void interpolate_corner(
    scene_t *s,
    nid_t nid,
    hid_t *hid_arr, 
    t_node_t *node, 
    t_node_t **node_arr, 
    render_element_t *dest_arr, 
    heightmap_element_t **el_arr, 
    int idx)
{
    heightmap_element_t *el = el_arr[idx];
    render_element_t *dest = &dest_arr[idx];
    hid_t main_hid = hid_arr[idx];

    if(node_arr[idx]->scale < TERRAIN_SCALE_THRESHOLD)
    {
	float f = (node_arr[idx]->scale - 1.0)/(TERRAIN_SCALE_THRESHOLD-1.0);
	
	assert(f>=0);
	assert(f<=1);

	if((HID_GET_X_POS(main_hid) & 1) ||
	   (HID_GET_Y_POS(main_hid) & 1)
	    )
	{

	    heightmap_element_t *el1;
	    heightmap_element_t *el2;

	    if((HID_GET_X_POS(main_hid) & 1) &&
	       (HID_GET_Y_POS(main_hid) & 1))
		{
		    if((NID_GET_X_POS(nid)+NID_GET_Y_POS(nid))&1)
		    {
			el1 = el_arr[3];
			el2 = el_arr[7];	   
		    }
		    else
		    {
			el1 = el_arr[1];
			el2 = el_arr[5];			
		    }
		    
		}
	       else
	       {
		   el1 = el_arr[(idx+1)%8];
		   el2 = el_arr[(idx+7)%8];	   
	       }
	       
	    dest->vertex[0] = scene_hid_x_coord(s, main_hid);
	    dest->vertex[1] = scene_hid_y_coord(s, main_hid);
	    dest->vertex[2] = el->height*f + 0.5*(el1->height+el2->height)*(1.0-f);

	    dest->color[0] = el->color[0]*f + 0.5*(el1->color[0]+el2->color[0])*(1.0-f);
	    dest->color[1] = el->color[1]*f + 0.5*(el1->color[1]+el2->color[1])*(1.0-f);
	    dest->color[2] = el->color[2]*f + 0.5*(el1->color[2]+el2->color[2])*(1.0-f);
	    
	    dest->normal[0] = el->normal[0]*f + 0.5*(el1->normal[0]+el2->normal[0])*(1.0-f);
	    dest->normal[1] = el->normal[1]*f + 0.5*(el1->normal[1]+el2->normal[1])*(1.0-f);
	    dest->normal[2] = el->normal[2]*f + 0.5*(el1->normal[2]+el2->normal[2])*(1.0-f);	    
	}
	else 
	{
	    hid_t parent_hid;
	    HID_SET(
		parent_hid,
		HID_GET_LEVEL(main_hid)-1, 
		HID_GET_X_POS(main_hid)/2,
		HID_GET_Y_POS(main_hid)/2);
	    heightmap_element_t *parent_el = scene_hid_lookup(s, parent_hid);
	    dest->vertex[0] = scene_hid_x_coord(s, main_hid);
	    dest->vertex[1] = scene_hid_y_coord(s, main_hid);
	    dest->vertex[2] = el->height*f + parent_el->height*(1.0-f);
	    dest->color[0] = el->color[0]*f + parent_el->color[0]*(1.0-f);
	    dest->color[1] = el->color[1]*f + parent_el->color[1]*(1.0-f);
	    dest->color[2] = el->color[2]*f + parent_el->color[2]*(1.0-f);
	    
	    dest->normal[0] = el->normal[0]*f + parent_el->normal[0]*(1.0-f);
	    dest->normal[1] = el->normal[1]*f + parent_el->normal[1]*(1.0-f);
	    dest->normal[2] = el->normal[2]*f + parent_el->normal[2]*(1.0-f);
	}
	
    }
    else
    {
	dest->vertex[0] = scene_hid_x_coord(s, main_hid);
	dest->vertex[1] = scene_hid_y_coord(s, main_hid);
	dest->vertex[2] = el->height;
	dest->color[0] = el->color[0];
	dest->color[1] = el->color[1];
	dest->color[2] = el->color[2];
	memcpy(dest->normal, el->normal, sizeof(GLfloat)*3);	
    }
    
    dest->use=1;
    
}


static void max_used( 
    scene_t *s,
    hid_t hid,
    int *level,
    hid_t *h,
    t_node_t **n
    )
{
    *n=0;
    
    int i, j;
    for(i=HID_GET_LEVEL(hid)-1; i<=HID_GET_LEVEL(hid); i++)
    {
	nid_t nid[4];
	int nid_count = hid_get_nid_at_level(hid, nid, i);
	int done=0;
	
	for(j=0; j<nid_count; j++)
	{
//	    printf("Ok, checking node (%d %d %d), number %d of %d matches at current level.\n", NID_GET_LEVEL(nid[j]), NID_GET_X_POS(nid[j]), NID_GET_Y_POS(nid[j]), j+1, nid_count);
	    
	    t_node_t *tmp = scene_nid_lookup(s, nid[j]);
	    if( tmp->scale > 0.0 && (!*n || tmp->scale < (*n)->scale))
	    {
		*n = tmp;
		done=1;
	    }
	}
	if(done)
	{
	    *level = i;
	    int hl = HID_GET_LEVEL(hid);
	    int x = HID_GET_X_POS(hid) >> (hl-*level);
	    int y = HID_GET_Y_POS(hid) >> (hl-*level);
	    HID_SET(*h, *level, x, y);
	    return;
	}
    }
}

static inline void render_calculate_elements(
    scene_t *s, 
    nid_t nid,
    t_node_t *node,
    render_element_t *element)
{
    hid_t hid[9];
    hid_t hid_max_arr[9];
    heightmap_element_t *el_arr[9];
    t_node_t *node_arr[9];
    int level_arr[9];
    
    nid_get_hid(nid, hid);
    int i, j;
    for(i=0; i<=MIDDLE_ELEMENT; i++)
    {
	max_used(s, hid[i], &level_arr[i], &hid_max_arr[i], &node_arr[i]);
	el_arr[i] = scene_hid_lookup(s, hid_max_arr[i]);
    }
    
    for(i=1;i<MIDDLE_ELEMENT;i+=2)
    {
	//printf("Lookup he %d %d %d\n", HID_GET_LEVEL(hid[i]), HID_GET_X_POS(hid[i]), HID_GET_Y_POS(hid[i]));
	interpolate_corner(s, nid, hid_max_arr, node, node_arr, element, el_arr, i);
	
	subtract(element[i].vertex, s->camera.pos, element[i].direction, 3);
	element[i].direction[2] -= 2.0*render_height_correct(element[i].direction[0], element[i].direction[1]);
	
	normalize(element[i].direction, element[i].direction, 3);
    }
    for(i=0; i<MIDDLE_ELEMENT; i+=2)
    {
	element[i].use=0;
	for(j=0; j<3;j++)
	{
	    element[i].direction[j] = (element[(i+1)%8].direction[j]+element[(i+7)%8].direction[j])*0.5;
	}
	
	if(level_arr[i] == NID_GET_LEVEL(nid))
	{
	    interpolate_corner(s, nid, hid_max_arr, node, node_arr, element, el_arr, i);
	}
	else
	{
	    element[i].vertex[2] = 0.5 * (element[(i+7)%8].vertex[2] + element[i+1].vertex[2]);	    
	    element[i].shade = 0.5 * (element[(i+7)%8].shade + element[i+1].shade);		    element[i].normal[0] = 0.5 * (element[(i+7)%8].normal[0] + element[i+1].normal[0]);	    
	    element[i].normal[1] = 0.5 * (element[(i+7)%8].normal[1] + element[i+1].normal[1]);	    
	    element[i].normal[2] = 0.5 * (element[(i+7)%8].normal[2] + element[i+1].normal[2]);
	}
    }

    for(j=0; j<3;j++)
    {
	element[MIDDLE_ELEMENT].direction[j] = (element[1].direction[j]+element[5].direction[j])*0.5;
    }
    interpolate_corner(s, nid, hid_max_arr, node, node_arr, element, el_arr, MIDDLE_ELEMENT);

    for(i=0; i<= MIDDLE_ELEMENT; i++)
    {
	float l = fabs(dot_prod(element[i].direction, element[i].normal, 3));
	l = s->ambient_light + l*(s->sun_light-s->ambient_light);	
	element[i].shade = l;	
    }
}


static void render_node(scene_t *s, nid_t nid, vertex_data_t *vd)
{
    t_node_t *n = scene_nid_lookup(s, nid);
    render_element_t render_element[10];
    nid_t child_id[4];
    int i;
    int idx0 = vd->vertex_count;
    
//    glDisable(GL_DEPTH_TEST);

    if(!n || n->scale < 0.0)
    {
	/*
	  Don't render non-existing nodes or nodes outside of the viewable area.
	*/
	return;
    }
    
    if(n->scale > 0.0 )
    {
	
	if(nid_is_edge(nid))
	{
/*
	    printf("Avoiding rendering node %d %d %d\n",
		    NID_GET_LEVEL(nid),
		    NID_GET_X_POS(nid),
		    NID_GET_Y_POS(nid));
*/
	    return;
	}
	
	render_calculate_elements( s, nid, n, render_element);

//	glBegin( GL_TRIANGLE_FAN );	
	vertex_perspective( s, &render_element[MIDDLE_ELEMENT], s->camera.pos, vd, idx0 );
	//vertex_perspective( s, &render_element[7], s->camera.pos, vd, idx0 );

	for( i=0; i<4; i++ )
	{
	    vertex_perspective( s, &render_element[(2*i+7)%8], s->camera.pos, vd, idx0 );	    
	    if( render_element[2*i].use )
	    {
		vertex_perspective( s, &render_element[2*i], s->camera.pos, vd, idx0 );
	    }
	}
	vertex_perspective( s, &render_element[7], s->camera.pos, vd, idx0 );
	/*
	vd->index[vd->idx_count++] = idx0;
	vd->index[vd->idx_count++] = vd->vertex_count-1;
	vd->index[vd->idx_count++] = idx0+1;
	*/
	
#define SHADE_MAX 0.8
	if(0&&n->distance < GRASS_MAX_DISTANCE)
	{
	    
	    if(render_element[1].shade < SHADE_MAX ||
	       render_element[3].shade < SHADE_MAX ||
	       render_element[5].shade < SHADE_MAX ||
	       render_element[6].shade < SHADE_MAX)
	    {
		
//		glDisable( GL_CULL_FACE );
//		glBegin( GL_TRIANGLES );
//	    printf("Woot %d %d %d\n", NID_GET_LEVEL(nid), NID_GET_X_POS(nid), NID_GET_Y_POS(nid));
	    
		int i=0, j=0;
		GLfloat base_x, base_y, x, y;
		GLfloat offset = s->grass_offset;
		
		GLfloat max_x = render_element[5].vertex[0];
		GLfloat max_y = render_element[5].vertex[1];
		GLfloat min_x = render_element[1].vertex[0];
		GLfloat min_y = render_element[1].vertex[1];
/*	    
	    glVertex3f(min_x, min_y, 0);
	    glVertex3f(max_x, max_y, 0);
	    glVertex3f(max_x, max_y, 100);
*/
/*
	    assert(max_x > min_x);
	    assert(max_y > min_y);
*/	    
		GLfloat x_w_component = cos(-s->camera.lr_rot * M_PI / 180.0);
		GLfloat y_w_component = sin(-s->camera.lr_rot * M_PI / 180.0);
		GLfloat wave[] = 
		    {
			0.0, 
			0.5*GRASS_INTERVAL * sin(M_PI*0.2),
			0.5*GRASS_INTERVAL * sin(M_PI*0.4),
			0.5*GRASS_INTERVAL * sin(M_PI*0.6),
			0.5*GRASS_INTERVAL * sin(M_PI*0.8),
			0.5*GRASS_INTERVAL * sin(M_PI*1.0),
			0.5*GRASS_INTERVAL * sin(M_PI*1.2),
			0.5*GRASS_INTERVAL * sin(M_PI*1.4),
			0.5*GRASS_INTERVAL * sin(M_PI*1.8),
		    }
		;
		
		GLfloat max_dst_sq = GRASS_MAX_DISTANCE*GRASS_MAX_DISTANCE;
		
		while(1)
		{
		    base_x = min_x + GRASS_INTERVAL*i;
		    
		    if(base_x >= max_x)
			break;
		    j=0;
		    GLfloat f1 = (base_x-min_x)/(max_x-min_x);
		    
		    GLfloat inv_dist = 1.0/(max_y-min_y);
		    base_y = min_y + ((i%2)?0.5*GRASS_INTERVAL:0.0);
		    
		    while(1)
		    {
			base_y += GRASS_INTERVAL*j++;
			
			if(base_y >= max_y)
			    break;
			
			x = base_x+wave[j%8];
			y = base_y+wave[i%8];
			
			GLfloat dx = x-s->camera.pos[0];
			GLfloat dy = y-s->camera.pos[1] ;
			
			GLfloat dst_sq = dx*dx + dy*dy;
			if(dst_sq > max_dst_sq)
			    continue;
			
			GLfloat f2 = (base_y-min_y)*inv_dist;		    
			
			GLfloat ff1;
			GLfloat ff2;
			int el[4];
			
			if((f1 < 0.5) && (f2 < 0.5))
			{
			    ff1 = 2.0 * f1;
			    ff2 = 2.0 * f2;
			    el[0] = 1;
			    el[1] = 2;
			    el[2] = MIDDLE_ELEMENT;
			    el[3] = 0;
			}
			else if (f1 < 0.5)
			{
			    ff1 = 2.0*f1;
			    ff2 = 2.0*f2 - 1.0;
			    el[0] = 0;
			    el[1] = MIDDLE_ELEMENT;
			    el[2] = 6;
			    el[3] = 7;
			}
			else if (f2 < 0.5)
			{
			    ff1 = 2.0*f1 - 1.0;
			    ff2 = 2.0*f2 ;
			    el[0] = 2;
			    el[1] = 3;
			    el[2] = 4;
			    el[3] = MIDDLE_ELEMENT;
			}
			else
			{
			    ff1 = 2.0*f1 - 1.0;
			    ff2 = 2.0*f2 - 1.0;
			    el[0] = MIDDLE_ELEMENT;
			    el[1] = 4;
			    el[2] = 5;
			    el[3] = 6;
			
			}

			GLfloat shade;

			shade  = render_element[el[0]].shade * (1.0-ff1) * (1.0-ff2);
			shade += render_element[el[1]].shade * (ff1) * (1.0-ff2);
			shade += render_element[el[2]].shade * (ff1) * (ff2);
			shade += render_element[el[3]].shade * (1.0-ff1) * (ff2);

			if (shade > SHADE_MAX)
			    continue;
//		    printf("%.2f\n", shade);
		    
			GLfloat h;
			
			h  = render_element[el[0]].vertex[2] * (1.0-ff1) * (1.0-ff2);
			h += render_element[el[1]].vertex[2] * (ff1) * (1.0-ff2);
			h += render_element[el[2]].vertex[2] * (ff1) * (ff2);
			h += render_element[el[3]].vertex[2] * (1.0-ff1) * (ff2);	
		    
			h += render_height_correct(dx, dy);
			
			GLfloat dz = GRASS_HEIGHT * minf(1.0, 8.0*(0.6-shade));
		    
			//printf("%.2f\n",render_element[MIDDLE_ELEMENT].shade);;
//		    glColor3f( 0,0,0);
			
			glColor3ub( 
			    render_element[MIDDLE_ELEMENT].color[0]*shade, 
			    render_element[MIDDLE_ELEMENT].color[1]*shade, 
			    render_element[MIDDLE_ELEMENT].color[2]*shade);//0.1*nid.level );
/**/		    
			
			glVertex3f(
			    x-0.5*GRASS_WIDTH*x_w_component,
			    y-0.5*GRASS_WIDTH*y_w_component,
			    h-0.1);
			glVertex3f(
			    x+0.5*GRASS_WIDTH*x_w_component,
			    y+0.5*GRASS_WIDTH*y_w_component,
			    h-0.1);
			glVertex3f(x+offset,y,h+dz);
		    }
		
		    i++;
		}
	    
//		glEnd();
//		glEnable( GL_CULL_FACE );	
	    }
	    
	}
	
    }
    else
    {
	nid_get_children(nid, child_id);
	for( i=0; i<4; i++ )
	{
	    render_node( s, child_id[i], vd);
	}
    }
}

#define NUMBER_OF_COMPONENTS_PER_VERTEX 3
#define NUMBER_OF_COMPONENTS_PER_COLOR 4

void render_data(vertex_data_t *vd)
{

   
    // bind the buffer object to use
    glBindBuffer(GL_ARRAY_BUFFER, vd->vbo);
  
    const GLsizeiptr vertex_size = vd->vertex_count*sizeof(vertex_element_t);
    
    glBufferData(GL_ARRAY_BUFFER, vertex_size, vd->vertex, GL_STREAM_DRAW);
/*
    GLvoid* vbo_buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); 
    
    printf("b %d %d\n", vbo_buffer, vd->vertex);
    
    float *b = vbo_buffer;
    b[0]=3.3;
    printf("b2\n");
    // transfer the vertex data to the VBO 
    memcpy(vbo_buffer, vd->vertex, vertex_size);
    printf("c\n");

    // append color data to vertex data. To be optimal, 
    // data should probably be interleaved and not appended 
    vbo_buffer += vertex_size; 
    memcpy(vbo_buffer, vd->color, color_size); 
    glUnmapBuffer(GL_ARRAY_BUFFER); 
*/      
    // Describe to OpenGL where the vertex data is in the buffer 
    glVertexPointer(3, GL_FLOAT, sizeof(vertex_element_t), 0); 

    // Describe to OpenGL where the color data is in the buffer
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex_element_t), (void *)offsetof(vertex_element_t, color));
    
    // create index buffer 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vd->ibo); 
        
    // For constrast, instead of glBufferSubData and glMapBuffer, 
    // we can directly supply the data in one-shot 
    glBufferData(
	GL_ELEMENT_ARRAY_BUFFER,
	vd->idx_count*sizeof(GLuint), 
	vd->index, 
	GL_STREAM_DRAW);
    
    // Activate the VBOs to draw 
    //   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); 
    glEnableClientState(GL_VERTEX_ARRAY); 
    glEnableClientState(GL_COLOR_ARRAY); 
    
    // This is the actual draw command 
    glDrawElements(
	GL_TRIANGLES,
	vd->idx_count,
	GL_UNSIGNED_SHORT,
	(GLvoid*)((char*)NULL));
    
    
}


void render_terrain( scene_t *s, vertex_data_t *vd )
{
    s->grass_offset=(0.5+0.5*cos(s->time*0.1))*sin(s->time*GRASS_WIND_FREQUENCY)*GRASS_WIND_AMPLITUDE;
    
    nid_t root = node_get_root();
    int node_count;
    while((node_count= render_prepare_node(s, root, 0.0)) > 3000)
    {
	s->render_quality *= 0.95;
    }
	
    vd->idx_count=0;
    vd->vertex_count=0;
    render_node(s, root, vd);    
/*    
    glDisable( GL_CULL_FACE );
    glBegin( GL_TRIANGLES );
    glEnd();
*/
}

static void *render_terrain_thread_runner(void *arg)
{
    set_current_thread_name("anna (terrain)");
    
    
    pthread_mutex_lock(&render_terrain_mutex);
    thread_data_t *d = arg;
    while(1)
    {
	pthread_cond_wait(&render_terrain_convar, &render_terrain_mutex);	    
	render_terrain(d->s, &d->vd);
    }
    pthread_mutex_unlock(&render_terrain_mutex);
    return 0;
    
}

void render_terrain_start(scene_t *s)
{
    if(!s->terrain_state)
    {
	thread_data_t *td = malloc(sizeof(thread_data_t));
	td->s = s;
	td->vd.vertex = malloc(sizeof(vertex_element_t)*10*3000);
	td->vd.index = malloc(sizeof(GLushort)*3000*3*8);
	td->vd.idx_count=0;
	td->vd.vertex_count=0;
	glGenBuffers(1, &(td->vd.vbo));
	glGenBuffers(1, &(td->vd.ibo));
	s->terrain_state = td;	

/*
    cpu_set_t *cpusetp;
    size_t size;
    cpusetp = CPU_ALLOC(num_cpus);
    if (cpusetp == NULL) {
	perror("CPU_ALLOC");
	exit(EXIT_FAILURE);
    }
    
    size = CPU_ALLOC_SIZE(2);
    
    CPU_ZERO_S(size, cpusetp);
    
    CPU_SET_S(cpu, size, cpusetp);
    
    pthread_attr_setaffinity_np(, size,cpusetp);
    
    
    CPU_FREE(cpusetp);
*/

	int rc = pthread_create(&render_terrain_thread, 0, render_terrain_thread_runner, s->terrain_state);

	if(rc)
	{
	    printf("ERROR; return code from pthread_create() is %d\n", rc);
	    exit(1);
	}

    }
    
    pthread_mutex_lock(&render_terrain_mutex);
    pthread_cond_signal(&render_terrain_convar);
    pthread_mutex_unlock(&render_terrain_mutex);
}

void render_terrain_finish(scene_t *s)
{
    pthread_mutex_lock(&render_terrain_mutex);
    glEnable( GL_CULL_FACE );
    thread_data_t *td = s->terrain_state;
    s->triangle_count += td->vd.idx_count/3;
    render_data(&td->vd);   
    pthread_mutex_unlock(&render_terrain_mutex);
}

void render_terrain_init()
{
    
}

void render_terrain_destroy()
{
/*
    glDeleteBuffers(1, &(vd.ibo));
    glDeleteBuffers(1, &(vd.vbo));
*/
}
