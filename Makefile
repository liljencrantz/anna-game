#COMPILERFLAGS = -Wall -funroll-loops -pg #-ffast-math -fprofile-arcs -ftest-coverage 
#COMPILERFLAGS = -Wall -funroll-loops -ffast-math -fprofile-arcs -ftest-coverage 
#COMPILERFLAGS = -Wall -O3 -mcpu=athlon-tbird -funroll-loops -ffast-math 
PROF_FLAGS := -g -pg

CFLAGS := -rdynamic -Wall -std=c99 -D_ISO99_SOURCE=1 -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=199309L $(PROF_FLAGS) -O3 -ffast-math -msse4 -mfpmath=sse -march=core2 
LDFLAGS := -lm -lpthread -rdynamic -ll -lSDL -lSDL_image -lGL -lGLU $(PROF_FLAGS) -ffast-math liblua/src/liblua.a

RENDER_OBJS = render.o render_terrain.o render_trees.o tree.o render_actors.o
GENERATE_OBJS = tile_calc.o 
ANNA_OBJS = $(GENERATE_OBJS) $(RENDER_OBJS) actor.o main.o screen.o util.o scene.o tile.o node.o heightmap_element.o anna_lua.o
TILE_OBJS = tile_test.o tile.o
PROGRAMS := anna

all: $(PROGRAMS)
.PHONY: all

#########################################################
#            BEGIN DEPENDENCY TRACKING                  #
#########################################################
%.d: %.c
	gcc -MM -MT "$@ $*.o" -MG $*.c >> $@ || rm $@ 
include $(ANNA_OBJS:.o=.d)
#########################################################
#             END DEPENDENCY TRACKING                   #
#########################################################

anna: $(ANNA_OBJS)  
	$(CC) $(ANNA_OBJS) $(LDFLAGS) -o $@ #-fprofile-arcs -ftest-coverage -pg

tile_test: $(TILE_OBJS) 
	gcc -Wall $(TILE_OBJS) -o $@

clean:
	rm -f *.o *.d anna tile_test gmon.out
.PHONY: clean
