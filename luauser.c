#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
#include <pthread.h>

static pthread_mutex_t annalualock;

void annalua_initlock() {
    if (pthread_mutex_init(&annalualock, NULL) != 0) {
        printf("ERROR: pthread_mutex_init");
	fflush(stdout);
        exit(1);
    }
}

void annalua_lock(lua_State * L) {
    if (pthread_mutex_lock(&annalualock) != 0) {
        printf("ERROR: pthread_mutex_lock\n");
	fflush(stdout);
        exit(1);
    }
}

void annalua_unlock(lua_State * L) {
    if (pthread_mutex_unlock(&annalualock) != 0) {
        printf("ERROR: pthread_mutex_lock\n");
	fflush(stdout);
        exit(1);
    }
}

void annalua_userstateopen(lua_State * L) {
}

void annalua_userstatethread(lua_State * L, lua_State * L1) {
}

