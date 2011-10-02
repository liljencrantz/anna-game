#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

SDL_Surface *screen=0;

static int screen_exit=0;
static int screen_keys[64];

void screen_init(int w, int h, int fullscreen)
{
    memset(screen_keys, 0, sizeof(screen_keys));
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
	printf("Unable to initialize SDL: %s\n", SDL_GetError());
	exit(1);
    }
    
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );  
    screen = SDL_SetVideoMode( w,h,0, SDL_GL_DOUBLEBUFFER | SDL_OPENGL | (fullscreen?SDL_FULLSCREEN:0) ); 
    
    if (screen == NULL) {
	printf("Unable to set video mode: %s\n", SDL_GetError());
	exit(1);
    }
/*
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
	fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	exit(1);
    }
    fprintf(stdout, "Using GLEW %s\n", glewGetString(GLEW_VERSION));
  */  
//    render_init();
}

static void screen_key_set(unsigned int key)
{
    screen_keys[key / 32] = screen_keys[key / 32] | (1<<(key %32));
}

static void screen_key_clear(unsigned int key)
{
    screen_keys[key / 32] = screen_keys[key / 32] & ~(1<<(key %32));
}

int screen_key_get(unsigned int key)
{
    return !!(screen_keys[key / 32] & (1<<(key %32)));
}

void screen_swap_buffers()
{
//    glFlush();
    //void (*fun)(int);
    //fun=glXGetProcAddress("wglGetSwapIntervalEXT");
    //glXGetSwapIntervalEXT();
//    if(DRAW)
//    {
	SDL_GL_SwapBuffers();
//    }
}

void screen_destroy()
{
    SDL_Quit();
}

int screen_do_exit()
{
    return screen_exit;
}

void screen_check_input()
{
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
	switch(event.type){
	    case SDL_KEYDOWN:
		screen_key_set(event.key.keysym.sym);
		printf("%d\n", event.key.keysym.sym);
		break;
	    case SDL_KEYUP:
		screen_key_clear(event.key.keysym.sym);
		break;
	    case SDL_QUIT:
		screen_exit=1;
		break;
	}
    }
}

