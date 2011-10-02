#COMPILERFLAGS = -Wall -funroll-loops -pg #-ffast-math -fprofile-arcs -ftest-coverage 
#COMPILERFLAGS = -Wall -funroll-loops -ffast-math -fprofile-arcs -ftest-coverage 
#COMPILERFLAGS = -Wall -O3 -mcpu=athlon-tbird -funroll-loops -ffast-math 
PROF_FLAGS := -g -pg

CFLAGS := -fPIC -rdynamic -Wall -std=c99 -D_ISO99_SOURCE=1 -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=199309L $(PROF_FLAGS)  -I include 
# -O3 -ffast-math -msse4 -mfpmath=sse -march=core2 
LDFLAGS := -lm -lpthread -rdynamic -lSDL -lSDL_image -lGL -lGLU  $(PROF_FLAGS) -ffast-math -lGLEW

RENDER_OBJS = src/render.o src/render_terrain.o src/render_trees.o src/tree.o src/render_balls.o src/ball.o src/ball_calc.o src/boid.o src/render_boids.o
GENERATE_OBJS = src/tile_calc.o 
ANNA_OBJS = $(GENERATE_OBJS) $(RENDER_OBJS) src/main.o src/screen.o src/scene.o src/tile.o src/node.o src/heightmap_element.o src/vertex_data.o src/thread.o autogen/annaGame.o src/util.o

all: lib/annaGame.so
.PHONY: all

autogen/annaGame.c: bindings/annaGame.bind
	annabind bindings/annaGame.bind >autogen/annaGame.c

lib/annaGame.so: $(ANNA_OBJS)
	$(CC) -shared $(ANNA_OBJS) -o $@ $(LDFLAGS) 

#########################################################
#            BEGIN DEPENDENCY TRACKING                  #
#########################################################
%.d: %.c
	echo -n $@ " " >$@; $(CC) -I include -MT $(@:.d=.o)  -MM -MG $*.c >> $@ || rm $@ 
ifneq "$(MAKECMDGOALS)" "clean"
include $(ANNA_OBJS:.o=.d)
endif
#########################################################
#             END DEPENDENCY TRACKING                   #
#########################################################

clean:
	rm -f src/*.o src/*.d gmon.out autogen/*.c autogen/*.o lib/*.so
.PHONY: clean

