#include <stdlib.h>     
#include <stdio.h>     
#include <GL/glew.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
#include <string.h>
#include <assert.h>
#include <math.h>

#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>

#include "scene.h"
#include "tree.h"
#include "ball.h"
#include "anna_lua.h"
#include "screen.h"
#include "render.h"
#include "tile.h"
#include "boid.h"
#include "util.h"


static lua_State *L;    

void tile_calc(scene_t *s);
/*
static int get_int (lua_State *L, void *v)
{
    lua_pushnumber(L, *(int*)v);
    return 1;
}

static int set_int (lua_State *L, void *v)
{
    *(int*)v = luaL_checkint(L, 3);
    return 0;
}
*/

static int get_float (lua_State *L, void *v, off_t off)
{
    lua_pushnumber(L, *(float*)(v+off));
    return 1;
}

static int set_float (lua_State *L, void *v, off_t off)
{
    *((float*)(v+off)) = luaL_checknumber(L, 3);
    return 0;
}

static int get_ptr_float (lua_State *L, void *v, off_t off)
{
    lua_pushnumber(L, *(float*)(*(void **)v+off));
    return 1;
}

static int set_ptr_float (lua_State *L, void *v, off_t off)
{
    *((float*)(*(void**)v+off)) = luaL_checknumber(L, 3);
    return 0;
}

static int get_double (lua_State *L, void *v, off_t off)
{
    lua_pushnumber(L, *(double*)(v+off));
    return 1;
}

static int set_double (lua_State *L, void *v, off_t off)
{
    *(double*)(v+off) = luaL_checknumber(L, 3);
    return 0;
}

static int set_pointer (lua_State *L, void *v, off_t off)
{
    *(void **)(v+off) = (void *)lua_topointer(L, 3);
    return 0;
}

typedef int (*Xet_func) (lua_State *L, void *v, off_t off);

typedef const struct{
    const char *name;  /* member name */
    Xet_func func;     /* get or set function for type of member */
    size_t offset;     /* offset of member within your_t */
} register_member_t;
//Xet_reg_pre;

static void register_type_add (lua_State *L, register_member_t *l)
{
    for (; l->name; l++) {
	lua_pushstring(L, l->name);
	lua_pushlightuserdata(L, (void*)l);
	lua_settable(L, -3);
    }
}

static int Xet_call (lua_State *L)
{
  /* for get: stack has userdata, index, lightuserdata */
  /* for set: stack has userdata, index, value, lightuserdata */
  register_member_t *m = (register_member_t *)lua_touserdata(L, -1);  /* member info */
  lua_pop(L, 1);                               /* drop lightuserdata */
  luaL_checktype(L, 1, LUA_TUSERDATA);
  return m->func(L, (void *)((char *)lua_touserdata(L, 1)), m->offset);
}


static int index_handler (lua_State *L)
{
  /* stack has userdata, index */
  lua_pushvalue(L, 2);                     /* dup index */
  lua_rawget(L, lua_upvalueindex(1));      /* lookup member by name */
  if (!lua_islightuserdata(L, -1)) {
    lua_pop(L, 1);                         /* drop value */
    lua_pushvalue(L, 2);                   /* dup index */
    lua_gettable(L, lua_upvalueindex(2));  /* else try methods */
    if (lua_isnil(L, -1))                  /* invalid member */
      luaL_error(L, "cannot get member '%s'", lua_tostring(L, 2));
    return 1;
  }
  return Xet_call(L);                      /* call get function */
}

static int newindex_handler (lua_State *L)
{
  /* stack has userdata, index, value */
  lua_pushvalue(L, 2);                     /* dup index */
  lua_rawget(L, lua_upvalueindex(1));      /* lookup member by name */
  if (!lua_islightuserdata(L, -1))         /* invalid member */
    luaL_error(L, "cannot set member '%s'", lua_tostring(L, 2));
  return Xet_call(L);                      /* call set function */
}

static int register_type (
    lua_State *L, 
    char *type_name,
    const luaL_reg *type_methods,
    const luaL_reg *type_meta_methods,
    const register_member_t *getters,
    const register_member_t *setters
    )
{
    int metatable, methods;
    
    /* create methods table, & add it to the table of globals */
    luaL_openlib(L, type_name, type_methods, 0);
    methods = lua_gettop(L);
    
    /* create metatable for the type and add it to the registry */
    luaL_newmetatable(L, type_name);
    luaL_openlib(L, 0, type_meta_methods, 0);  /* fill metatable */
    metatable = lua_gettop(L);
    
    lua_pushliteral(L, "__metatable");
    lua_pushvalue(L, methods);    /* dup methods table*/
    lua_rawset(L, metatable);     /* hide metatable:
                                   metatable.__metatable = methods */
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, metatable);  /* upvalue index 1 */
    register_type_add(L, getters);     /* fill metatable with getters */
    lua_pushvalue(L, methods);    /* upvalue index 2 */
    lua_pushcclosure(L, index_handler, 2);
    lua_rawset(L, metatable);     /* metatable.__index = index_handler */
    
    lua_pushliteral(L, "__newindex");
    lua_newtable(L);              /* table for members you can set */
    register_type_add(L, setters);     /* fill with setters */
    lua_pushcclosure(L, newindex_handler, 1);
    lua_rawset(L, metatable);     /* metatable.__newindex = newindex_handler */
    
    lua_pop(L, 1);                /* drop metatable */
    return 1;                     /* return methods on the stack */
}

static void *check_item (lua_State *L, int index, char *type_name)
{
    void *yd;
    luaL_checktype(L, index, LUA_TUSERDATA);
    yd = luaL_checkudata(L, index, type_name);
    if (yd == NULL){
	luaL_typerror(L, index, type_name);
    }
    return yd;
}

static int lua_tree_create (lua_State *L)
{
    int *res = (int *)lua_newuserdata(L, sizeof(int));
    luaL_getmetatable(L, "TreeStem");
    lua_setmetatable(L, -2);
    float pos[] = 
	{
	    luaL_checknumber(L, 3),
	    luaL_checknumber(L, 4)
	}
    ;
    
    *res = scene_tree_create(
	(scene_t *)lua_topointer(L, 1),
	(char *)luaL_checkstring(L, 2),
	pos,
	luaL_checknumber(L, 5),
	luaL_checknumber(L, 6));

    return 1;
}


static int lua_ball_create (lua_State *L)
{
    int *res = (int *)lua_newuserdata(L, sizeof(int));
    luaL_getmetatable(L, "BallPeer");
    lua_setmetatable(L, -2);    
    
    *res = scene_ball_create(
	(scene_t *)check_item(L, 1, "Scene"),
	(char *)luaL_checkstring(L, 2),
	luaL_checknumber(L, 3));

    return 1;
}

static int lua_ball_type_create (lua_State *L)
{
    ball_type_t **res = (ball_type_t **)lua_newuserdata(L, sizeof(ball_type_t *));
    luaL_getmetatable(L, "BallType");
    lua_setmetatable(L, -2);    
    
    *res = ball_type_create(
	maxi(0,mini(BALL_LEVEL_MAX,luaL_checknumber(L, 1))),
	luaL_checkstring(L, 2),
	maxf(0.0,minf(1.0,luaL_checknumber(L, 3)))*255,
	allocfn_calloc);
    
    return 1;
}

static int lua_ball_type_set_element(lua_State *L)
{
    ball_type_t **t = (ball_type_t **)check_item(L, 1, "BallType");

    int x = luaL_checkint(L, 2)-1;
    int y = luaL_checkint(L, 3)-1;

    float h = luaL_checknumber(L, 4);

    float r = luaL_checknumber(L, 5);
    float g = luaL_checknumber(L, 6);
    float b = luaL_checknumber(L, 7);
    
    int l = (*t)->levels;
    int idx = ball_idx(l,x,y);

//    printf("Set %d %d -> %d\n", x,y,idx);
    

    (*t)->data[idx].radius = h;
    (*t)->data[idx].color[0] = r*255;
    (*t)->data[idx].color[1] = g*255;
    (*t)->data[idx].color[2] = b*255;
        
    return 0;
}

static int lua_ball_type_calc(lua_State *L)
{
    ball_type_t **t = (ball_type_t **)check_item(L, 1, "BallType");
    ball_type_calc(*t);
    return 0;
}


static int lua_ball_type_save(lua_State *L)
{
    ball_type_t **t = (ball_type_t **)check_item(L, 1, "BallType");
    scene_t *s = (scene_t *)check_item(L, 2, "Scene");
    char *dname = s->name;
    char *name = (*t)->name;
    ball_type_save(*t, dname, name);
    return 0;
}


static int lua_boid_set_create (lua_State *L)
{
    scene_t *s = (scene_t *)lua_topointer(L, 1);
    boid_set_t **res = (boid_set_t **)lua_newuserdata(L, sizeof(boid_set_t *));
    luaL_getmetatable(L, "BoidSetPeer");
    lua_setmetatable(L, -2);    
    *res = scene_boid_set_get(
	s, 
	scene_boid_set_create(
	    s,
	    luaL_checkint(L, 2),
	    luaL_checknumber(L, 3),
	    luaL_checknumber(L, 4)));
/*    printf("Wee, we have created a boid set with index %d at address %d\n", *res,
      scene_boid_set_get(s, *res));
*/  
    return 1;
}

static int lua_boid_step(lua_State *L)
{
    boid_set_t **t = (boid_set_t **)check_item(L, 1, "BoidSetPeer");
    scene_t *s = (scene_t *)check_item(L, 2, "Scene");
    float dt = luaL_checknumber(L, 3);
/*    printf(
	"Wee, do step on boid set with index %d, address %d\n", 
	*t, scene_boid_set_get(s, *t));*/
    boid_step(*t, dt);
    
    return 0;
}

static int lua_ball_set_transform(lua_State *L)
{
    int *t = (int *)check_item(L, 1, "BallPeer");
    scene_t *s = (scene_t *)check_item(L, 2, "Scene");
    ball_t *b = scene_ball_get(s, *t);

    b->transform[0] = luaL_checknumber(L, 3);
    b->transform[1] = luaL_checknumber(L, 4);	
    b->transform[2] = luaL_checknumber(L, 5);
    b->transform[3] = luaL_checknumber(L, 6);
    b->transform[4] = luaL_checknumber(L, 7);
    b->transform[5] = luaL_checknumber(L, 8);
    b->transform[6] = luaL_checknumber(L, 9);
    b->transform[7] = luaL_checknumber(L, 10);
    b->transform[8] = luaL_checknumber(L, 11);
    b->transform[9] = luaL_checknumber(L, 12);
    b->transform[10] = luaL_checknumber(L, 13);
    b->transform[11] = luaL_checknumber(L, 14);
    b->transform[12] = luaL_checknumber(L, 15);
    b->transform[13] = luaL_checknumber(L, 16);
    b->transform[14] = luaL_checknumber(L, 17);
    b->transform[15] = luaL_checknumber(L, 18);

    return 0;
}



static int lua_ball_destroy(lua_State *L)
{
    scene_t *s = (scene_t *)lua_topointer(L, 2);
    int *t = (int *)check_item(L, 1, "BallPeer");
    if(t)
    {
	scene_ball_destroy(s, *t);
    }
    return 0;
}


static int lua_scene_create (lua_State *L)
{
    scene_t *res = (scene_t*)lua_newuserdata(L, sizeof(scene_t));
    luaL_getmetatable(L, "Scene");
    lua_setmetatable(L, -2);
    
    const char *name = luaL_checkstring(L,1);    scene_init(res, name, lua_toboolean(L, 2));
//    load_temp_tile_data(res);
    
//    camera_move(res);    
    res->camera.lr_rot=0;
    res->camera.ud_rot = 65;
    
    return 1;
}

static int lua_scene_configure(lua_State *L)
{
    scene_t *s = (scene_t *)check_item(L, 1, "Scene");
    scene_configure(
	s, 
	luaL_checkint(L, 2),
	luaL_checknumber(L, 3),
	luaL_checkint(L, 4),
	luaL_checkint(L, 5));
        
    return 0;
}

static int lua_scene_create_tree(lua_State *L)
{
    scene_t *s = (scene_t *)check_item(L, 1, "Scene");
    char *name = luaL_checkstring(L, 2);
    float x = luaL_checknumber(L,3);
    float y = luaL_checknumber(L,4);
    float pos[] = 
	{
	    x, y
	}
    ;
    float angle = luaL_checknumber(L,5);
    float scale = luaL_checknumber(L,6);
    
    scene_tree_create(
	s,
	name,
	pos,
	angle, scale
	);
    
    return 0;
}

static int lua_scene_save_items(lua_State *L)
{
    scene_t *s = (scene_t *)check_item(L, 1, "Scene");
    scene_save_items(s);
    return 0;
}

static int lua_scene_set_terrain_element(lua_State *L)
{
    scene_t *s = (scene_t *)check_item(L, 1, "Scene");
    hid_t hid;

    int lvl = luaL_checkint(L, 2)-1;
    int x = luaL_checkint(L, 3)-1;
    int y = luaL_checkint(L, 4)-1;
    if((x < 0) || (y < 0) << (x >= (2<<lvl)) || (y >= (2<<lvl)))
    {
	return 0;
    }
    HID_SET(hid, lvl, x, y);
    
//    printf("LALALA %d %d %d\n", luaL_checkint(L, 2), luaL_checkint(L, 3), luaL_checkint(L, 4));
    
    heightmap_element_t *he = scene_hid_lookup(s, hid);
    he->height = luaL_checknumber(L, 5);
    he->color[0] = 255*luaL_checknumber(L, 6);
    he->color[1] = 255*luaL_checknumber(L, 7);
    he->color[2] = 255*luaL_checknumber(L, 8);
    return 0;
}

static int lua_scene_get_terrain_element(lua_State *L)
{
    scene_t *s = (scene_t *)check_item(L, 1, "Scene");
    hid_t hid;
    int lvl = luaL_checkint(L, 2)-1;
    int x = luaL_checkint(L, 3)-1;
    int y = luaL_checkint(L, 4)-1;
    if((x < 0) || (y < 0) << (x >= (2<<lvl)) || (y >= (2<<lvl)))
    {
	return 0;
    }
    HID_SET(hid, lvl, x, y);
        
    heightmap_element_t *he = scene_hid_lookup(s, hid);
    lua_pushnumber(L, he->height);    
    lua_pushnumber(L, (float)he->color[0]/255);    
    lua_pushnumber(L, (float)he->color[1]/255);    
    lua_pushnumber(L, (float)he->color[2]/255);    
    return 4;
}


static int lua_scene_generate_lod(lua_State *L)
{
    scene_t *s = (scene_t *)check_item(L, 1, "Scene");
    tile_calc(s);    
    return 0;
}


static int lua_scene_save_terrain(lua_State *L)
{
    scene_t *s = (scene_t *)check_item(L, 1, "Scene");
    scene_save_terrain(s);
    return 0;
}


static int lua_scene_render(lua_State *L)
{
    scene_t *s = (scene_t *)check_item(L, 1, "Scene");
    render(s);
    return 0;
}

static int lua_scene_get_height(lua_State *L)
{
    scene_t *s = (scene_t *)check_item(L, 1, "Scene");
    
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    lua_pushnumber(L, scene_get_height(s, x, y));    
        
    return 1;
}

static int lua_scene_get_slope(lua_State *L)
{
    scene_t *s = (scene_t *)check_item(L, 1, "Scene");
    
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float res[2];
    
    scene_get_slope(s, x, y, res);    
    lua_pushnumber(L, res[0]);
    lua_pushnumber(L, res[1]);
    
    return 2;
}

static int lua_scene_get_real_time(lua_State *L)
{
    lua_pushnumber(L, get_time());    
    return 1;
}


static int lua_screen_init(lua_State *L)
{
    int w = luaL_checkint(L, 1);
    int h = luaL_checkint(L, 2);
    int fs = luaL_checkint(L, 3);
    screen_init(w, h, fs);
    return 0;
}

static int lua_screen_check_input(lua_State *L)
{
    screen_check_input();
    return 0;
}

static int lua_screen_key_get(lua_State *L)
{
    int key;
    if(!lua_isnumber(L,1))
    {
	const char *s = lua_tolstring(L, 1, 0);
	key = s[0];
    }
    else
    {
	key = luaL_checkint(L, 1);
	
    }
    lua_pushboolean(L, screen_key_get(key));    
    return 1;
}

static int lua_screen_destroy(lua_State *L)
{
    screen_destroy();
    return 0;
}

static int lua_screen_swap_buffers(lua_State *L)
{
    screen_swap_buffers();
    return 0;
}



void register_types(
    lua_State *L)
{
    
    
    static const register_member_t scene_getters[] = {
	{"size",          get_float, offsetof(scene_t,scene_size)   },
	{"time",          get_double, offsetof(scene_t,time)   },
	{"cameraX",       get_float, offsetof(scene_t,camera)+offsetof(view_t,pos)},
	{"cameraY",       get_float, offsetof(scene_t,camera)+offsetof(view_t,pos)+sizeof(float)},
	{"cameraZ",       get_float, offsetof(scene_t,camera)+offsetof(view_t,pos)+2*sizeof(float)},
	{"cameraAngle",   get_float, offsetof(scene_t,camera)+offsetof(view_t,lr_rot)},
	{"renderQuality", get_float, offsetof(scene_t,render_quality)   },
	{0,0}
    };
    
    static const register_member_t scene_setters[] = {
	{"time",          set_double, offsetof(scene_t,time)   },
	{"renderQuality",   set_float,    offsetof(scene_t,render_quality)   },
	{"cameraX",   set_float, offsetof(scene_t,camera)+offsetof(view_t,pos)},
	{"cameraY",   set_float, offsetof(scene_t,camera)+offsetof(view_t,pos)+sizeof(float)},
	{"cameraZ",   set_float, offsetof(scene_t,camera)+offsetof(view_t,pos)+2*sizeof(float)},
	{"cameraAngle", set_float, offsetof(scene_t,camera)+offsetof(view_t,lr_rot)},
	{0,0}
    };
    
    static const luaL_reg scene_methods[] = {
	{"getRealTime", lua_scene_get_real_time},
	{"create", lua_scene_create},
	{"configure", lua_scene_configure},
	{"setTerrainElement", lua_scene_set_terrain_element},
	{"getTerrainElement", lua_scene_get_terrain_element},
	{"generateLod", lua_scene_generate_lod},
	{"saveTerrain", lua_scene_save_terrain},
	{"render", lua_scene_render},
	{"getHeight", lua_scene_get_height},
	{"getSlope", lua_scene_get_slope},
	{"createTree", lua_scene_create_tree},
	{"saveItems", lua_scene_save_items},
	{0,0}
    };

    static const luaL_reg scene_meta_methods[] = {
	{0,0}
    };

    register_type(
	L,
	"Scene", 
	scene_methods, 
	scene_meta_methods,
	scene_getters,
	scene_setters);
    
    static const register_member_t tree_getters[] = {
	{0,0}
    };
    
    static const register_member_t tree_setters[] = {
	{0,0}
    };
    
    static const luaL_reg tree_methods[] = {
	{"create", lua_tree_create},	
	{0,0}
    };

    static const luaL_reg tree_meta_methods[] = {
	{0,0}
    };
    
    register_type(
	L,
	"TreeStem", 
	tree_methods, 
	tree_meta_methods,
	tree_getters,
	tree_setters);


    static const register_member_t ball_getters[] = {
	{0,0}
    };

    static const register_member_t ball_setters[] = {
	{0,0}
    };
    
    static const luaL_reg ball_methods[] = {
	{"create", lua_ball_create},	
	{"destroy", lua_ball_destroy},	
	{"setTransform", lua_ball_set_transform},	
	{0,0}
    };

    static const luaL_reg ball_meta_methods[] = {
	{0,0}
    };
    
    register_type(
	L,
	"BallPeer", 
	ball_methods, 
	ball_meta_methods,
	ball_getters,
	ball_setters);

    static const register_member_t ball_type_getters[] = {
	{0,0}
    };

    static const register_member_t ball_type_setters[] = {
	{0,0}
    };
    
    static const luaL_reg ball_type_methods[] = {
	{"create", lua_ball_type_create},	
	{"setElement", lua_ball_type_set_element},	
	{"save", lua_ball_type_save},	
	{"calc", lua_ball_type_calc},	
	{0,0}
    };

    static const luaL_reg ball_type_meta_methods[] = {
	{0,0}
    };
    
    register_type(
	L,
	"BallType", 
	ball_type_methods, 
	ball_type_meta_methods,
	ball_type_getters,
	ball_type_setters);

    static const register_member_t screen_getters[] = {
	{0,0}
    };

    static const register_member_t screen_setters[] = {
	{0,0}
    };
    
    static const luaL_reg screen_methods[] = {
	{"init", lua_screen_init},	
	{"checkInput", lua_screen_check_input},	
	{"keyGet", lua_screen_key_get},	
	{"destroy", lua_screen_destroy},	
	{"swapBuffers", lua_screen_swap_buffers},	
	{0,0}
    };

    static const luaL_reg screen_meta_methods[] = {
	{0,0}
    };
    
    register_type(
	L,
	"Screen", 
	screen_methods, 
	screen_meta_methods,
	screen_getters,
	screen_setters);
    
    static const register_member_t boid_set_getters[] = {
	{"targetX",   get_ptr_float, offsetof(boid_set_t,target)},
	{"targetY",   get_ptr_float, offsetof(boid_set_t,target)+sizeof(float)},
	{"targetZ",   get_ptr_float, offsetof(boid_set_t,target)+2*sizeof(float)},
	{0,0}
    };
    
    static const register_member_t boid_set_setters[] = {
	{"targetX",   set_ptr_float, offsetof(boid_set_t,target)},
	{"targetY",   set_ptr_float, offsetof(boid_set_t,target)+sizeof(float)},
	{"targetZ",   set_ptr_float, offsetof(boid_set_t,target)+2*sizeof(float)},
	{0,0}
    };
    
    static const luaL_reg boid_set_methods[] = {
	{"create", lua_boid_set_create},	
	{"step", lua_boid_step},	
	{0,0}
    };

    static const luaL_reg boid_set_meta_methods[] = {
	{0,0}
    };
    
    register_type(
	L,
	"BoidSetPeer", 
	boid_set_methods, 
	boid_set_meta_methods,
	boid_set_getters,
	boid_set_setters);


}

    
void anna_lua_run()
{
    char *buff = "anna.run()";
    int error;
    
    error = luaL_loadbuffer(L, buff, strlen(buff), "Timestep tick") || lua_pcall(L, 0, LUA_MULTRET, 0);
    if (error) {
	fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
	lua_pop(L, 1);  /* pop error message from the stack */
    }

}

void anna_lua_init()
{
    L = lua_open();
    luaL_openlibs(L);
    
    register_types(L);
    int status = luaL_loadfile(L, "lua/anna/main.lua");
    if (status) {
        /* If something went wrong, error message is at the top of */
        /* the stack */
        fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
        exit(1);
    }
    
    int result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result) {
        fprintf(stderr, "Failed to run script: %s\n", lua_tostring(L, -1));
        exit(1);
    }
    
    
}
