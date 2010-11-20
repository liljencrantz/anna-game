#COMPILERFLAGS = -Wall -funroll-loops -pg #-ffast-math -fprofile-arcs -ftest-coverage 
#COMPILERFLAGS = -Wall -funroll-loops -ffast-math -fprofile-arcs -ftest-coverage 
#COMPILERFLAGS = -Wall -O3 -mcpu=athlon-tbird -funroll-loops -ffast-math 
PROF_FLAGS := -g -pg

CFLAGS := -rdynamic -Wall -std=c99 -D_ISO99_SOURCE=1 -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=199309L $(PROF_FLAGS) 
# -O3 -ffast-math -msse4 -mfpmath=sse -march=core2 
LDFLAGS := -lm -lpthread -rdynamic -lSDL -lSDL_image -lGL -lGLU  $(PROF_FLAGS) -ffast-math luasrc/lua-5.1.4/src/liblua.a -lGLEW

RENDER_OBJS = render.o render_terrain.o render_trees.o tree.o render_balls.o ball.o ball_calc.o boid.o render_boids.o
GENERATE_OBJS = tile_calc.o 
ANNA_OBJS = $(GENERATE_OBJS) $(RENDER_OBJS) main.o screen.o util.o scene.o tile.o node.o heightmap_element.o anna_lua.o hash_table.o vertex_data.o thread.o luauser.o
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

anna: $(ANNA_OBJS) luasrc/lua-5.1.4/src/liblua.a
	$(CC) $(ANNA_OBJS) $(LDFLAGS) -o $@ #-fprofile-arcs -ftest-coverage -pg

tile_test: $(TILE_OBJS) 
	gcc -Wall $(TILE_OBJS) -o $@

clean:
	rm -rf *.o *.d anna tile_test gmon.out luasrc
.PHONY: clean

tree_branch_data.c: branch.lua
	lua branch.lua > tree_branch_data.c

luasrc/lua-5.1.4:
	mkdir luasrc
	cd luasrc; wget http://www.lua.org/ftp/lua-5.1.4.tar.gz
	cd luasrc; tar -xvzf lua-5.1.4.tar.gz

luasrc/lua-5.1.4/src/liblua.a: luasrc/lua-5.1.4
	cd luasrc/lua-5.1.4/src; $(MAKE) CFLAGS="-O2 -Wall -DLUA_USE_LINUX -DLUA_USER_H=\"\\\"$(shell pwd)/luauser.h\\\"\"" liblua.a
