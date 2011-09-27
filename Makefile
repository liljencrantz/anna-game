#COMPILERFLAGS = -Wall -funroll-loops -pg #-ffast-math -fprofile-arcs -ftest-coverage 
#COMPILERFLAGS = -Wall -funroll-loops -ffast-math -fprofile-arcs -ftest-coverage 
#COMPILERFLAGS = -Wall -O3 -mcpu=athlon-tbird -funroll-loops -ffast-math 
PROF_FLAGS := -g -pg

CFLAGS := -fPIC -rdynamic -Wall -std=c99 -D_ISO99_SOURCE=1 -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=199309L $(PROF_FLAGS) 
# -O3 -ffast-math -msse4 -mfpmath=sse -march=core2 
LDFLAGS := -lm -lpthread -rdynamic -lSDL -lSDL_image -lGL -lGLU  $(PROF_FLAGS) -ffast-math -lGLEW

RENDER_OBJS = render.o render_terrain.o render_trees.o tree.o render_balls.o ball.o ball_calc.o boid.o render_boids.o
GENERATE_OBJS = tile_calc.o 
ANNA_OBJS = $(GENERATE_OBJS) $(RENDER_OBJS) main.o screen.o scene.o tile.o node.o heightmap_element.o vertex_data.o thread.o annaGame.o util.o
TILE_OBJS = tile_test.o tile.o

all: lib/annaGame.so
.PHONY: all

lib/annaGame.so: $(ANNA_OBJS)
	$(CC) -shared $(ANNA_OBJS) -o $@ $(LDFLAGS) 

#########################################################
#            BEGIN DEPENDENCY TRACKING                  #
#########################################################
%.d: %.c
	gcc -MM -MT "$@ $*.o" -MG $*.c >> $@ || rm $@ 
ifneq "$(MAKECMDGOALS)" "clean"
include $(ANNA_OBJS:.o=.d)
endif
#########################################################
#             END DEPENDENCY TRACKING                   #
#########################################################

tile_test: $(TILE_OBJS) 
	gcc -Wall $(TILE_OBJS) -o $@

clean:
	rm -rf *.o *.d anna tile_test gmon.out
.PHONY: clean

