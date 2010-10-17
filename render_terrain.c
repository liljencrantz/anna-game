#include <assert.h>
#include <string.h>
#include <math.h>

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


typedef struct
{
    GLfloat vertex[3];
    GLfloat normal[3];
    GLfloat shade;
    GLubyte color[3];
        
    int use;
    
}
    render_element_t;
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


static void render_prepare_node(scene_t *s, nid_t nid)
{
    t_node_t *n = scene_nid_lookup(s, nid);
    nid_t child_id[4];
    float error;
    int i;
    
    if(!n)
    {
	return;
    }
    
    if(scene_nid_is_visible(s, nid, &s->camera))
    {
	n->distance = scene_nid_min_distance(s, nid, &s->camera);
	error = node_get_error(n, &s->camera)*s->render_quality;
	//printf("Distance: %.2f, distortion is %.2f, error is %.2f\n", n->distance, n->distortion, error);
	
	if(error < 1.0)
	{
	    n->scale = maxf(0.00001, minf(1.0, (1.0/error-1.0)*5.0));
	}
	else
	{
	    n->scale = 0.0;
	    nid_get_children(nid, child_id);
	    for(i=0; i<4; i++)
	    {
		render_prepare_node(s, child_id[i]);
	    }
	}	    
    }
    else
    {
	n->scale = -1.0;
    }
}


static void vertex_shade( render_element_t *el, float *pos)
{
    
//    printf("Vertex %.2f %.2f %.2f \n", vertex[0], vertex[1], vertex[2]);
    GLfloat direction[3];
    subtract(el->vertex, pos, direction, 3);
    direction[2] -= 2.0*render_height_correct(direction[0], direction[1]);
    
    normalize(direction, direction, 3);
    GLfloat l = 0.4 + 0.6*fabs(dot_prod(direction, el->normal, 3));
    el->shade = l;
}

static void vertex_perspective( render_element_t *el, float *pos)
{
    vertex_shade(el, pos);
    
    float a = el->vertex[0]-pos[0];
    float b = el->vertex[1]-pos[1];
    if (DRAW)
    {
	glColor3ub( el->color[0]*el->shade, el->color[1]*el->shade, el->color[2]*el->shade);    
//	printf( "%f %f %f\n", el->color[0]*el->shade, el->color[1]*el->shade, el->color[2]*el->shade);    
	glVertex3f( el->vertex[0], el->vertex[1], el->vertex[2]+render_height_correct(a,b));
//	printf("%f %f %f\n", el->vertex[0], el->vertex[1], el->vertex[2]+render_height_correct(a,b));
    }
}


#define MIDDLE_ELEMENT 8

static void interpolate(scene_t *s, hid_t main_hid, GLfloat scale, render_element_t *dest)
{
    heightmap_element_t *el = scene_hid_lookup(s, main_hid);

    dest->vertex[0] = scene_hid_x_coord(s, main_hid);
    dest->vertex[1] = scene_hid_y_coord(s, main_hid);
    dest->vertex[2] = el->height;
    dest->color[0] = el->color[0];
    dest->color[1] = el->color[1];
    dest->color[2] = el->color[2];

    memcpy(dest->normal, el->normal, sizeof(GLfloat)*3);	
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
	    if( tmp->scale > 0.0 && (!*n || tmp->scale > (*n)->scale))
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

static void render_calculate_elements(
    scene_t *s, 
    nid_t nid,
    float scale,
    render_element_t *element)
{
    hid_t hid[9];
    nid_get_hid(nid, hid);
    int i;
    
    for(i=1;i<9;i+=2)
    {
	//printf("Lookup he %d %d %d\n", HID_GET_LEVEL(hid[i]), HID_GET_X_POS(hid[i]), HID_GET_Y_POS(hid[i]));
	
	hid_t el;
	t_node_t *n;
	int level;
	max_used(s, hid[i], &level, &el, &n);
	interpolate(s, el, n->scale, &element[i]);
    }
    for(i=0; i<10; i+=2)
    {
	element[i].use=0;
	hid_t el;
	t_node_t *n;
	int level;	
	max_used(s, hid[i], &level, &el, &n);
	if(level == NID_GET_LEVEL(nid))
	{
	    interpolate(s, el, n->scale, &element[i]);
	}
	else
	{
	    element[i].vertex[2] = 0.5 * (element[(i+7)%8].vertex[2] + element[i+1].vertex[2]);	    
	    element[i].shade = 0.5 * (element[(i+7)%8].shade + element[i+1].shade);		    element[i].normal[0] = 0.5 * (element[(i+7)%8].normal[0] + element[i+1].normal[0]);	    
	    element[i].normal[1] = 0.5 * (element[(i+7)%8].normal[1] + element[i+1].normal[1]);	    
	    element[i].normal[2] = 0.5 * (element[(i+7)%8].normal[2] + element[i+1].normal[2]);	    
    
	}
	
    }
    
    interpolate(s, hid[MIDDLE_ELEMENT], 1.0, &element[MIDDLE_ELEMENT]);
    
}


static void render_node(scene_t *s, nid_t nid)
{
    t_node_t *n = scene_nid_lookup(s, nid);
    render_element_t render_element[10];
    nid_t child_id[4];
    int i;

    if(!n || n->scale < 0.0)
    {
	/*
	  Don't render non-existing nodes or nodes outside of the viewable area.
	*/
	return;
    }
    
    if(n->scale > 0.0 )
    {
	if(n->distance > 80)
	{
	    return;
	}
	
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
	
	render_calculate_elements( s, nid, n->scale, render_element);
	
	glBegin( GL_TRIANGLE_FAN );
	
	//glColor3f( 0.0, 0.0, 0.4);//0.1*nid.level );
	vertex_perspective( &render_element[MIDDLE_ELEMENT], s->camera.pos );
	vertex_perspective( &render_element[7], s->camera.pos );

	for( i=0; i<4; i++ )
	{
	    if( render_element[2*i].use )
	    {
		vertex_perspective( &render_element[2*i], s->camera.pos );
	    }
	    else
	    {
		vertex_shade( &render_element[2*i], s->camera.pos );
	    }
	    vertex_perspective( &render_element[2*i+1], s->camera.pos );
	    
	}
	glEnd();

#define SHADE_MAX 0.6
	if(n->distance < GRASS_MAX_DISTANCE)
	{
	    
	    if(render_element[1].shade < SHADE_MAX ||
	       render_element[3].shade < SHADE_MAX ||
	       render_element[5].shade < SHADE_MAX ||
	       render_element[6].shade < SHADE_MAX)
	    {
		
		glDisable( GL_CULL_FACE );
		glBegin( GL_TRIANGLES );
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
	    
		glEnd();
		glEnable( GL_CULL_FACE );	
	    }
	    
	}
	
    }
    else
    {
	nid_get_children(nid, child_id);
	for( i=0; i<4; i++ )
	{
	    render_node( s, child_id[i]);
	}
    }
}

void render_terrain( scene_t *s )
{
    s->grass_offset=(0.5+0.5*cos(s->time*0.1))*sin(s->time*GRASS_WIND_FREQUENCY)*GRASS_WIND_AMPLITUDE;
    nid_t root = node_get_root();
    render_prepare_node(s, root);
    glEnable( GL_CULL_FACE );	
    render_node(s, root);    
    glDisable( GL_CULL_FACE );
//    printf("AAA\n");
}

void render_terrain_init()
{
    render_register(render_terrain, RENDER_PASS_SOLID);
}

