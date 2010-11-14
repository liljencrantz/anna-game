#define lua_lock(L) annalua_lock(L)
#define lua_unlock(L) annalua_unlock(L)
#define lua_userstateopen(L) annalua_userstateopen(L)
#define lua_userstatethread(L,L1) annalua_userstatethread(L,L1)

void annalua_lock(lua_State * L);
void annalua_unlock(lua_State * L);
void annalua_userstateopen(lua_State * L);
void annalua_userstatethread(lua_State * L, lua_State * L1);
