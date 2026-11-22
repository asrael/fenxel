#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <SDL3/SDL.h>

#include "host.h"

typedef struct FXL_Script {
    lua_State* L;
} FXL_Script;

internal int script_traceback(lua_State* L) {
    luaL_traceback(L, L, lua_tostring(L, 1), 1);
    return 1;
}

internal bool script_run(FXL_Script* script, const char* src, const char* name);

internal bool script_init(FXL_Script* script) {
    script->L = luaL_newstate();

    if (!script->L) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "could not the LuaJIT state");
        return false;
    }

    luaL_openlibs(script->L);
    script_run(script, "print('script runtime initialized with ' ..jit.version)", "=luajit_version");

    return true;
}

internal bool script_run(FXL_Script* script, const char* src, const char* name) {
    lua_State* L = script->L;

    lua_pushcfunction(L, script_traceback);

    int handler = lua_gettop(L);
    bool ok = luaL_loadbuffer(L, src, strlen(src), name) == 0 && lua_pcall(L, 0, 0, handler) == 0;

    if (!ok) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    lua_remove(L, handler);

    return ok;
}

internal void script_quit(FXL_Script* script) {
    if (script->L) lua_close(script->L);

    script->L = nullptr;
}
