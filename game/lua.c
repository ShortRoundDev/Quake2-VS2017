#include "g_lua.h"
#include "g_local.h"
//lua_State* state;

void LuaInit(void) {
	state = luaL_newstate();
	luaL_openlibs(state);
	LuaLoadScript("baseq2/scripts/main.lua");

	//Register functions
	lua_pushcfunction(state, Reg_CloseMenu);
	lua_setglobal(state, "CloseMenu");

	__LuaStackDump(state);
}

void LuaDestroy(void) {
	lua_close(state);
}

void LuaCreateDom(char* ElementName) {
	lua_getglobal(state, "Document");
	lua_createtable(state, 0, 1);
	lua_setfield(state, -2, ElementName);
	lua_pop(state, 1);
}

void LuaLoadScript(char* ScriptName) {
	int err = 0;
	err = luaL_dofile(state, ScriptName);
}

void LuaRunInitScript(char* ElementName) {
	lua_getglobal(state, "Document");
	lua_getfield(state, -1, ElementName);
	if(lua_isnil(state, -1)) {
		printf("ElementName not found!\n");
		lua_settop(state, -2);
		return;
	}
	lua_getfield(state, -1, "OnLoad");
	if(lua_isnil(state, -1)) {
		printf("Onload not found!\n");
		lua_settop(state, -3);
		return;
	}

	lua_pcall(state, 0, 0, 0);
	lua_settop(state, -3);
}

void __LuaStackDump (lua_State *L) {
	int i;
	int t;
	i = lua_gettop(L);
    printf(" ----------------  Stack Dump ----------------\n" );
    while(i) {
		t = lua_type(L, i);
        switch (t) {
		case LUA_TSTRING:
			printf("%d:`%s'\n", i, lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:
			printf("%d: %s\n",i,lua_toboolean(L, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:
			printf("%d: %g\n",  i, lua_tonumber(L, i));
			break;
		default:
			printf("%d: %s\n", i, lua_typename(L, t));
			break;
		}
		i--;
	}
	printf("--------------- Stack Dump Finished ---------------\n" );
}


//Registered Functions
static int Reg_CloseMenu(lua_State *L) {
	CloseTopLevelMenu(&Queue);
}