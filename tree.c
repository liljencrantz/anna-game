

#include <GL/gl.h>	
#include <GL/glu.h>    

#include "SDL/SDL_image.h"

#include "tree.h"
#include "util.h"

tree_type_t *tree_type;


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

void add_section(
    tree_type_t *t, GLuint tid, GLfloat w,
    GLfloat p1x, GLfloat p1y, GLfloat p1z, 
    GLfloat p2x, GLfloat p2y, GLfloat p2z)
{
    t->section[t->section_count].type = TREE_SECTION_REGULAR;
    t->section[t->section_count].texture_id = tid;
    t->section[t->section_count].width = w;
    t->section[t->section_count].pos1[0] = p1x;
    t->section[t->section_count].pos1[1] = p1y;
    t->section[t->section_count].pos1[2] = p1z;
    t->section[t->section_count].pos2[0] = p2x;
    t->section[t->section_count].pos2[1] = p2y;
    t->section[t->section_count].pos2[2] = p2z;
    
    subtract(
	t->section[t->section_count].pos1,
	t->section[t->section_count].pos2,
	t->section[t->section_count].normal,
	3);

    normalize(
	t->section[t->section_count].normal,
	t->section[t->section_count].normal,
	3);
    t->section_count++;
}

void add_jsection(
    tree_type_t *t, GLuint tid1, GLuint tid2, GLfloat w1, GLfloat w2,
    GLfloat p1x, GLfloat p1y, GLfloat p1z)
{
    t->section[t->section_count].type = TREE_SECTION_JOINT;
    t->section[t->section_count].texture_id = tid1;
    t->section[t->section_count].width = w1;
    t->section[t->section_count].pos1[0] = 
	t->section[t->section_count-1].pos2[0];
    t->section[t->section_count].pos1[1] = 
	t->section[t->section_count-1].pos2[1];
    t->section[t->section_count].pos1[2] = 
	t->section[t->section_count-1].pos2[2];
    t->section_count++;


    t->section[t->section_count].type = TREE_SECTION_REGULAR;
    t->section[t->section_count].texture_id = tid2;
    t->section[t->section_count].width = w2;
    t->section[t->section_count].pos1[0] = 
	t->section[t->section_count-2].pos2[0];
    t->section[t->section_count].pos1[1] = 
	t->section[t->section_count-2].pos2[1];
    t->section[t->section_count].pos1[2] = 
	t->section[t->section_count-2].pos2[2];
    t->section[t->section_count].pos2[0] = p1x;
    t->section[t->section_count].pos2[1] = p1y;
    t->section[t->section_count].pos2[2] = p1z;
    subtract(
	t->section[t->section_count].pos1,
	t->section[t->section_count].pos2,
	t->section[t->section_count].normal,
	3);

    normalize(
	t->section[t->section_count].normal,
	t->section[t->section_count].normal,
	3);
    t->section_count++;
}

void add_joint(
    tree_type_t *t, GLuint tid, GLfloat w,
    GLfloat p1x, GLfloat p1y, GLfloat p1z)
{
    t->section[t->section_count].type = TREE_SECTION_JOINT;
    t->section[t->section_count].texture_id = tid;
    t->section[t->section_count].width = w;
    t->section[t->section_count].pos1[0] = p1x;
    t->section[t->section_count].pos1[1] = p1y;
    t->section[t->section_count].pos1[2] = p1z;
    t->section_count++;
}

void add_points(
    tree_type_t *t, GLfloat pw, GLfloat cw,
    GLfloat p1x, GLfloat p1y, GLfloat p1z,
    GLubyte *color, char count)
{
    tree_section_points_t *tsp = 
	(tree_section_points_t *)&(t->section[t->section_count]);
    tsp->type = TREE_SECTION_POINTS;
    
    tsp->point_width=pw;
    tsp->cloud_width=cw*2;
    tsp->pos[0]=p1x;
    tsp->pos[1]=p1y;
    tsp->pos[2]=p1z;
    normalize(tsp->pos, tsp->normal, 2);
    tsp->color[0] = color[0];
    tsp->color[1] = color[1];
    tsp->color[2] = color[2];
    tsp->count = count;
    t->section_count++;
}

tree_type_t *tree_load(char *name)
{
    tree_type_t *res = malloc(sizeof(tree_type_t) + 300 * sizeof(tree_section_t));
    
    res->section_count=0;
    GLuint tid = load_texture("textures/branch5.png");
    GLuint stem1_tid = load_texture("textures/stem1.png");
    GLuint stem2_tid = load_texture("textures/stem2.png");
    GLuint stem3_tid = load_texture("textures/stem3.png");
    GLuint branch1_tid = load_texture("textures/branch1.png");
    GLuint branch2_tid = load_texture("textures/branch2.png");
    GLuint branch3_tid = load_texture("textures/branch3.png");
    GLuint branch4_tid = load_texture("textures/branch4.png");
    GLuint root1_tid = load_texture("textures/root1.png");
    GLuint connection1_tid = load_texture("textures/connection1.png");
    GLuint jtid = load_texture("textures/connection2.png");

    GLubyte col[]=
	{
	    45,
	    70,
	    35
	}
    ;
    
    /*
      Stem
     */
    add_joint(
	res, jtid, 0.7, 
	0.0, 0.0, 0.35);
    add_section(
	res, stem1_tid, 0.77, 
	0.0, 0.0, 0.0,
	-0.1, -0.06, 1.99);
    add_jsection(
	res, jtid, stem3_tid, 0.50, 0.55, 
	-0.1, -1.09, 2.89);
    add_jsection(
	res, jtid, stem2_tid, 0.29, 0.33, 
	0.1, 0.0, 4.21);
    add_jsection(
	res, jtid, stem3_tid, 0.19, 0.21, 
	0.2, 1.38, 3.10);
    add_jsection(
	res, jtid, stem3_tid, 0.16, 0.18, 
	0.3, 0.65, 2.31);
    add_jsection(
	res, jtid, stem3_tid, 0.12, 0.15, 
	0.4, -0.11, 3.22);
    add_jsection(
	res, jtid, branch2_tid, 0.1, 0.1, 
	0.5, 0.13, 3.6);
    add_jsection(
	res, jtid, branch4_tid, 0.05, 0.07, 
	0.53, 0.32, 3.41);
    add_jsection(
	res, jtid, branch4_tid, 0.02, 0.05, 
	0.55, 0.25,3.34);
    add_points(
	res, 0.2, 0.3, 
	0.55, 0.35, 3.34, col, 12);


    /* Branch 1 */
    add_section(
	res, branch1_tid, 0.15, 
	0.13, 0.0, 0.91, 
	0.84, 0.0, 2.0);
    add_jsection(
	res, jtid, branch2_tid, 0.04, 0.05, 
	0.55, 0.0, 2.08);
    add_jsection(
	res, jtid, branch2_tid, 0.04, 0.05, 
	0.39, 0.0, 1.83);
    add_jsection(
	res, jtid, tid, 0.04, 0.05, 
	0.53, 0.0, 1.81);
    add_points(
	res, 0.2, 0.3, 
	0.53, 0.0, 1.81, col, 16);
    

    add_section(
	res, tid, 0.05, 
	0.75, 0.0, 1.84, 
	1.26, 0.0, 1.59);
    add_jsection(
	res, jtid, tid, 0.03, 0.04, 
	0.95, 0.0, 1.11 );
    add_jsection(
	res, jtid, tid, 0.03, 0.04, 
	0.7, 0.0, 1.2);
    add_jsection(
	res, jtid, tid, 0.025, 0.03, 
	0.75, 0.0, 1.3);
    add_points(
	res, 0.2, 0.3, 
	0.75, 0.0, 1.3, col, 16);


    add_section(
	res, branch3_tid, 0.04, 
	0.91, 0.0, 1.75, 
	1.22, 0.01, 1.85);
    add_jsection(
	res, jtid, tid, 0.025, 0.03, 
	1.25, 0.02, 1.75);
    add_points(
	res, 0.2, 0.3, 
	1.25, 0.02, 1.75, col, 16);



    /* Branch 2 */
    add_section(
	res, branch1_tid, 0.1, 
	-0.13, 0.0, 1.21, 
	-0.84, 0.0, 2.5);
    add_jsection(
	res, jtid, branch2_tid, 0.02, 0.03, 
	-0.55, 0.0, 2.58);
    add_jsection(
	res, jtid, branch2_tid, 0.01, 0.025, 
	-0.39, 0.1, 1.83+0.5);
    add_points(
	res, 0.2, 0.3, 
	-0.39, 0.1, 1.83+0.5, col, 8);


    

    add_section(
	res, tid, 0.035, 
	-0.75, 0.0, 1.84+0.5, 
	-1.26, 0.0, 1.59+0.5);
    add_jsection(
	res, jtid, tid, 0.025, 0.03, 
	-0.95, 0.0, 1.11+0.5 );
    add_jsection(
	res, jtid, tid, 0.025, 0.03, 
	-0.7, 0.0, 1.2+0.5);
    add_jsection(
	res, jtid, tid, 0.02, 0.025, 
	-0.75, 0.0, 1.3+0.5);
    add_points(
	res, 0.2, 0.3, 
	-0.75, -0.3, 1.3+0.5, col, 8);


    add_section(
	res, branch3_tid, 0.04, 
	-0.91, 0.0, 1.75+0.5, 
	-1.22, 0.01, 1.85+0.5);
    add_jsection(
	res, jtid, tid, 0.025, 0.03, 
	-1.25, 0.02, 1.75+0.5);
    add_points(
	res, 0.2, 0.3, 
	-1.25, 0.3, 1.75+0.5, col, 12);



    /* Branch 3 */
    
    add_section(
	res, branch2_tid, 0.1, 
	0.0, -0.48, 2.23, 
	0.0, -1.44, 2.35);
    add_jsection(
	res, jtid, branch3_tid, 0.08, 0.1, 
	0.0, -1.5, 2.6);
    add_jsection(
	res, jtid, branch2_tid, 0.04, 0.05, 
	0.0, -1.29, 2.63);
    add_jsection(
	res, jtid, branch2_tid, 0.03, 0.04, 
	0.0, -1.23, 2.53
	);
    add_points(
	res, 0.2, 0.3, 
	0.0, -1.23, 2.53, col, 16);
        
    /* Branch 4*/

    add_section(
	res, branch1_tid, 0.15, 
	0.0, -0.69, 3.4, 
	0.05, -0.8, 4.64);
    add_jsection(
	res, jtid, branch2_tid, 0.05, 0.06, 
	0.05, -0.28, 4.52);    
    add_jsection(
	res, jtid, tid, 0.03, 0.04, 
	0.05, -0.35, 4.26);
    add_jsection(
	res, jtid, tid, 0.025, 0.03, 
	0.05, -0.53, 4.24);
    add_points(
	res, 0.2, 0.3, 
	0.05, -0.53, 4.24, col, 16);

    /* Branch 5 */
    add_section(
	res, branch1_tid, 0.10, 
	0.0, -1.08, 2.86, 
	-0.3, -1.59, 3.56);
    add_jsection(
	res, jtid, tid, 0.03, 0.04, 
	-0.4, -1.38, 3.79);
    add_jsection(
	res, jtid, tid, 0.025, 0.03, 
	-0.45, -1.22, 3.5);

    add_joint(
	res, jtid, 0.08, 
	0.035, -0.80, 4.27);
    add_section(
	res, branch1_tid, 0.10, 
	0.035, -0.80, 4.27, 
	0.035, -1.65, 4.24);
    add_jsection(
	res, jtid, tid, 0.03, 0.04, 
	0.05, -1.7, 4.46);
    add_jsection(
	res, jtid, tid, 0.025, 0.03, 
	0.08, -1.53, 4.46);
    add_points(
	res, 0.2, 0.3, 
	0.08, -1.53, 4.46, col, 16);

    add_section(
	res, branch4_tid, 0.15, 
	0.35, 0.39, 2.62, 
	0.15, -0.3, 2.66);
    add_jsection(
	res, jtid, tid, 0.03, 0.03, 
	0.05, -0.41, 2.99);
    add_jsection(
	res, jtid, tid, 0.02, 0.02, 
	0.05, -0.08, 2.99);

    add_points(
	res, 0.2, 0.3, 
	0.05, -0.08, 2.99, col, 16);

/*
    add_jsection(
	res, jtid, tid, 0.025, 0.03, 
	0.08, -1.53, 4.46);
*/

    /* Branch 6 */
    add_section(
	res, branch1_tid, 0.1, 
	0.15, 0.06, 4.19, 
	0.15, 1.08, 4.22);
    add_jsection(
	res, jtid, tid, 0.03, 0.02, 
	0.15, 1.07, 4.34);

    add_points(
	res, 0.2, 0.3, 
	0.15, 1.07, 4.34, col, 8);
    
    add_section(
	res, tid, 0.05, 
	0.15, 0.70, 4.21, 
	0.40, 0.78, 4.22);
    add_jsection(
	res, jtid, tid, 0.02, 0.02, 
	0.45, 0.27, 4.18);
    add_jsection(
	res, jtid, tid, 0.02, 0.02, 
	0.65, 0.31, 4.17);
    
    add_points(
	res, 0.2, 0.3, 
	0.65, 0.31, 4.17, col, 4);

    /* Branch 7 */
    add_section(
	res, branch4_tid, 0.15, 
	0.2, 1.35, 3.16, 
	0.2, 1.55, 2.5);
    add_jsection(
	res, jtid, tid, 0.03, 0.02, 
	0.2, 1.33, 2.31);

    add_points(
	res, 0.2, 0.3, 
	0.2, 1.33, 2.31, col, 12);

    

    
    /*
      Roots
     */

    add_section(
	res, root1_tid, 0.16, 
	0.0, 0.5* -0.29, 0.16,
	0.0, -0.86, 0.08
	);
    add_section(
	res, root1_tid, 0.16, 
	0.5*-0.27, 0.5*-0.08, 0.16,
	-0.81, -0.26, 0.08
	);
    add_section(
	res, root1_tid, 0.16, 
	0.5*-0.17, 0.5*0.234, 0.16,
	-0.5, 0.69, 0.08
	);
    add_section(
	res, root1_tid, 0.16, 
	0.5*0.17, 0.5*0.234, 0.16,
	0.5, 0.69, 0.08
	);
    add_section(
	res, root1_tid, 0.16, 
	0.5*0.27, 0.5*-0.08, 0.16,
	0.81, -0.69, 0.08
	);
    
    /*

    res->section[0].width=0.5;
    res->section[0].pos1[0] = 0.0;
    res->section[0].pos1[1] = 0.0;
    res->section[0].pos1[2] = 0.0;

    res->section[0].pos2[0] = 0.0;
    res->section[0].pos2[1] = -0.06;
    res->section[0].pos2[2] = 1.99;
    
    
    res->section[1].texture_id = res->section[0].texture_id;
    res->section[1].width=0.3;

    res->section[1].pos1[0] = 0.0;
    res->section[1].pos1[1] = 0.13;
    res->section[1].pos1[2] = 1.76;

    res->section[1].pos2[0] = 0.0;
    res->section[1].pos2[1] = 0.8;
    res->section[1].pos2[2] = 2.2;
    

    res->section[2].texture_id = res->section[0].texture_id;
    res->section[2].width=0.25;

    res->section[2].pos1[0] = 0.0;
    res->section[2].pos1[1] = 0.8;
    res->section[2].pos1[2] = 2.2;

    res->section[2].pos2[0] = 0.0;
    res->section[2].pos2[1] = 0.15;
    res->section[2].pos2[2] = 3.0;

    
    res->section[3].texture_id = res->section[0].texture_id;
    res->section[3].width=0.2;

    res->section[3].pos1[0] = 0.0;
    res->section[3].pos1[1] = 0.15;
    res->section[3].pos1[2] = 3.0;

    res->section[3].pos2[0] = 0.0;
    res->section[3].pos2[1] = -0.8;
    res->section[3].pos2[2] = 2.5;
    */
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
	tree_type = tree_load(0);
    
    return tree_type;
}
