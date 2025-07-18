#include "build_config_defines.h"

#include "lua.hpp"

extern "C"{
    #include "lfs.h"
}

extern "C" {
    #include "luasocket/socket.h"
    #include "luasocket/luasocket.h"
}

//#include "Libs.h"
#include "script_additional_libs.h"

static const struct luaL_reg R[] =
{
	{ NULL,	    NULL },
};

//extern "C" __declspec(dllexport)
extern "C" int luaopen_marshal(lua_State* L);
int luaopen_lua_extensions(lua_State *L, bool IsDebug = false){

    open_additional_libs(L);

    luaopen_lfs(L);
    //open_string(L);
    //open_math(L);
    //open_table(L);
    luaopen_marshal(L);
    //open_kb(L);
    //open_log(L); 

    if (IsDebug)
    {
        luaopen_jit(L);
        luaopen_ffi(L);
        luaopen_debug(L);
    }

	luaL_register(L, "lua_extensions", R);
	return 0;
}

lua_CFunction luaopen_socket_core_init() {
	return luaopen_socket_core;
}

extern "C" void pdebug_init(lua_State* L);
void pdebug_init_init(lua_State* L) {
    pdebug_init(L);
}