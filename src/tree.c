

#include <GL/glew.h>	
#include <GL/glu.h>    
#include <assert.h>    

#include "SDL/SDL_image.h"

#include "tree.h"
#include "util.h"

tree_type_t *tree_type;

/*
GLuint load_texture(const char* file)
{
   SDL_Surface* surface = IMG_Load(file);
   if(!surface)
   {
       printf("Error while loading texture %s\n", file);
       exit(1);
   }
   
   GLuint texture;
   glPixelStorei(GL_UNPACK_ALIGNMENT,4);
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   SDL_PixelFormat *format = surface->format;
   if (format->Amask)
   {
      gluBuild2DMipmaps(GL_TEXTURE_2D, 4,
         surface->w, surface->h, GL_RGBA,GL_UNSIGNED_BYTE, surface->pixels);
   }
   else
   {
      gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
         surface->w, surface->h, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
   }
   SDL_FreeSurface(surface);
   return texture;
}
*/
void add_section(
    tree_type_t *t, tree_section_type_t *type, GLfloat w,
    GLfloat p1x, GLfloat p1y, GLfloat p1z)
{
    tree_section_t *s = &t->section[t->section_count];
    s->type = type;
    s->width = w;
    s->pos[0] = p1x;
    s->pos[1] = p1y;
    s->pos[2] = p1z;

    if(t->section_count > 0 && s->type)
    {
	tree_section_t *p = &t->section[t->section_count-1];
	subtract(
	    p->pos,
	    s->pos,
	    s->normal,
	    3);
	
	s->length = sqrt(dot_prod(s->normal, s->normal, 3));
	
	normalize(
	    s->normal,
	    s->normal,
	    3);

	for(int i=0; i<3; i++)
	{
	    assert(fabs(s->pos[i]+s->normal[i]*s->length - p->pos[i])< 0.01);
	    
	}
	
    }
    t->section_count++;
}

tree_type_t *tree_load(char *name)
{
    tree_type_t *res = malloc(sizeof(tree_type_t) + 300 * sizeof(tree_section_t));
    
    assert(name && strlen(name) < TREE_NAME_SZ);
    strcpy(res->name, name);
    
    res->section_count=0;

    float branch_data[][3]=
	{
	    {0.00, 1.00, 1.00},
	    {0.06, 1.33, 1.03},
	    {0.11, 0.90, 1.20},
	    {0.18, 1.33, 0.57},
	    {0.28, 0.57, 0.93},
	    {0.53, 0.57, 0.84},
	    {0.62, 0.31, 1.07},
	    {0.74, 0.64, 0.80},
	    {0.91, 0.38, 0.80},
	    {1.00, 0.70, 0.70}
	}
    ;

    tree_section_type_t *t1 = malloc(sizeof(tree_section_type_t) + 10*sizeof(float[3]));
    t1->count=10;
    memcpy(t1->data, branch_data, 10*sizeof(float[3]));

#include "tree_branch_data.c"

    /*
      Stem
     */
    add_section(
	res, 0, 0, 
	0.0, 0.0, 0.0);    
    add_section(
	res, stem1_tid, 0.77, 
	-0.1, -0.06, 1.99);
    add_section(
	res,  stem3_tid, 0.55, 
	-0.1, -1.09, 2.89);
    add_section(
	res, stem2_tid, 0.33, 
	0.1, 0.0, 4.21);
    add_section(
	res, stem3_tid, 0.21, 
	0.2, 1.38, 3.10);
    add_section(
	res,  stem3_tid, 0.18, 
	0.3, 0.65, 2.31);
    add_section(
	res,  stem3_tid, 0.15, 
	0.4, -0.11, 3.22);
    add_section(
	res,  branch2_tid, 0.09, 
	0.5, 0.13, 3.6);
    add_section(
	res,  branch4_tid, 0.08, 
	0.53, 0.32, 3.41);
    add_section(
	res,  branch4_tid, 0.07, 
	0.55, 0.25,3.34);


    /* Branch 1 */
    add_section(
	res, 0, 0,
	0.13, 0.0, 0.91);
    add_section(
	res, branch1_tid, 0.15,     
	0.84, 0.0, 2.0);
    add_section(
	res, branch2_tid, 0.05, 
	0.55, 0.0, 2.08);
    add_section(
	res, branch2_tid, 0.06, 
	0.39, 0.0, 1.83);
    add_section(
	res, branch5_tid, 0.05, 
	0.53, 0.0, 1.81);

    add_section(
	res, 0, 0, 
	0.75, 0.0, 1.87);
    add_section(
	res, branch2_tid, 0.05, 
	1.26, 0.0, 1.59);
    add_section(
	res, branch2_tid, 0.04, 
	0.95, 0.0, 1.11 );
    add_section(
	res, branch2_tid, 0.04, 
	0.7, 0.0, 1.2);

    add_section(
	res, 0,0,
	0.91, 0.0, 1.75);
    
    add_section(
	res, branch3_tid, 0.04, 
	1.22, 0.01, 1.85);
    add_section(
	res, branch2_tid, 0.03, 
	1.25, 0.02, 1.75);

    /* Branch 2 */
    add_section(
	res, 0,0,
	-0.13, 0.0, 1.21);
    
    add_section(
	res, branch1_tid, 0.1, 
	-0.84, 0.0, 2.5);
    add_section(
	res, branch2_tid, 0.03, 
	-0.55, 0.0, 2.58);
    add_section(
	res, branch2_tid, 0.025, 
	-0.39, 0.1, 1.83+0.5);


    

    add_section(
	res, 0,0,
	-0.75, 0.0, 1.84+0.5);
    add_section(
	res, branch2_tid, 0.035, 
	-1.26, 0.0, 1.59+0.5);
    add_section(
	res, branch2_tid, 0.03, 
	-0.95, 0.0, 1.11+0.5 );
    add_section(
	res, branch2_tid, 0.03, 
	-0.7, 0.0, 1.2+0.5);
    add_section(
	res, branch2_tid, 0.025, 
	-0.75, 0.0, 1.3+0.5);


    add_section(
	res, 0,0,
	-0.91, 0.0, 1.75+0.5);
    add_section(
	res, branch3_tid, 0.04, 
	-1.22, 0.01, 1.85+0.5);
    add_section(
	res, branch2_tid, 0.03, 
	-1.25, 0.02, 1.75+0.5);

    /* Branch 3 */
    
    add_section(
	res, 0,0,
	0.0, -0.48, 2.23);
    
    add_section(
	res, branch2_tid, 0.1, 
	0.0, -1.44, 2.35);
    add_section(
	res, branch3_tid, 0.1, 
	0.0, -1.5, 2.6);
    add_section(
	res, branch2_tid, 0.05, 
	0.0, -1.29, 2.63);
    add_section(
	res, branch2_tid, 0.04, 
	0.0, -1.23, 2.53
	);
        
    /* Branch 4*/

    add_section(
	res, 0,0,
	0.0, -0.69, 3.4);
    
    add_section(
	res, branch1_tid, 0.15, 
	0.05, -0.8, 4.64);
    add_section(
	res, branch2_tid, 0.06, 
	0.05, -0.28, 4.52);    
    add_section(
	res, branch2_tid, 0.04, 
	0.05, -0.35, 4.26);
    add_section(
	res, branch2_tid, 0.03, 
	0.05, -0.53, 4.24);

    /* Branch 5 */
    add_section(
	res, 0, 0,
	0.0, -1.08, 2.86);
    
    add_section(
	res, branch1_tid, 0.10, 
	-0.3, -1.59, 3.56);
    add_section(
	res, branch2_tid, 0.04, 
	-0.4, -1.38, 3.79);
    add_section(
	res, branch2_tid, 0.03, 
	-0.45, -1.22, 3.5);

    add_section(
	res, 0,0,
	0.035, -0.70, 4.27);
    add_section(
	res, branch1_tid, 0.10, 
	0.035, -1.65, 4.24);
    add_section(
	res, branch2_tid, 0.04, 
	0.05, -1.7, 4.46);
    add_section(
	res, branch2_tid, 0.03, 
	0.08, -1.53, 4.46);


    add_section(
	res, 0,0,
	0.35, 0.39, 2.62);
    add_section(
	res, branch4_tid, 0.15, 
	0.15, -0.3, 2.66);
    add_section(
	res, branch2_tid, 0.03, 
	0.05, -0.41, 2.99);
    add_section(
	res, branch2_tid, 0.02, 
	0.05, -0.08, 2.99);

/*
    add_section(
	res, branch2_tid, 0.025, 0.03, 
	0.08, -1.53, 4.46);
*/

    /* Branch 6 */
    add_section(
	res, 0,0,
	0.15, 0.06, 4.19);
    add_section(
	res, branch1_tid, 0.1, 
	0.15, 1.08, 4.22);
    add_section(
	res, branch2_tid, 0.02, 
	0.15, 1.07, 4.34);

    add_section(
	res, 0,0,
	0.15, 0.70, 4.21);
    
    add_section(
	res, branch2_tid, 0.05, 
	0.40, 0.78, 4.22);
    add_section(
	res, branch2_tid, 0.02, 
	0.45, 0.27, 4.18);
    add_section(
	res, branch2_tid, 0.02, 
	0.65, 0.31, 4.17);

    /* Branch 7 */
    add_section(
	res, 0,0,
	0.2, 1.35, 3.16);
    
    add_section(
	res, branch4_tid, 0.15, 
	0.2, 1.55, 2.5);
    add_section(
	res, branch2_tid, 0.02, 
	0.2, 1.33, 2.31);

    
    /*
      Roots
     */

    add_section(
	res, 0,0,
	0.0, 0.5* -0.29, 0.16);
    add_section(
	res, root1_tid, 0.16, 
	0.0, -0.86, 0.08
	);

    add_section(
	res, 0,0,
	0.5*-0.27, 0.5*-0.08, 0.16);
    add_section(
	res, root1_tid, 0.16, 
	-0.81, -0.26, 0.08
	);

    add_section(
	res, 0,0,
	0.5*-0.17, 0.5*0.234, 0.16);
    add_section(
	res, root1_tid, 0.16, 
	-0.5, 0.69, 0.08
	);

    add_section(
	res, 0,0,
	0.5*0.17, 0.5*0.234, 0.16);
    add_section(
	res, root1_tid, 0.16, 
	0.5, 0.69, 0.08
	);

    add_section(
	res, 0,0,
	0.5*0.27, 0.5*-0.08, 0.16);
    add_section(
	res, root1_tid, 0.16, 
	0.81, -0.69, 0.08
	);

    
    return res;    
}

void tree_load_init()
{
    
    int flags=IMG_INIT_JPG|IMG_INIT_PNG;
    int initted=IMG_Init(flags);
    if((initted&flags) != flags) {
	printf("IMG_Init: Failed to init required jpg and png support!\n");
	printf("IMG_Init: %s\n", IMG_GetError());
	exit(1);
	
    }
    
}


tree_type_t *tree_type_get(char *name)
{
    if(tree_type == 0)
	tree_type = tree_load(name);
    
    return tree_type;
}
