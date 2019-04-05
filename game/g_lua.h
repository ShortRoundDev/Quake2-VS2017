#pragma once
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

lua_State* state;

void LuaInit(void);
void LuaDestroy(void);
void LuaCreateDom(char* ElementName);
void LuaLoadScript(char* ScriptName);
void LuaRunInitScript(char* ElementName);
void __LuaStackDump (lua_State *L);

//Registered Functions
static int Reg_CloseMenu(lua_State *L);