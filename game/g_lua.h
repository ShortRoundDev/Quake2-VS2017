#pragma once
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

//quake includes
#include "g_local.h"

struct _EntityList {
	struct EntityListNode* Head;
	struct EtntityListNode* Tail;
} EntityList;

struct EntityListNode {
	edict_t* Value;
	struct EntityListNode* Next;
};

lua_State* state;

void LuaInit(void);
void LuaDestroy(void);
void LuaCreateDom(char* ElementName);
void LuaLoadScript(char* ScriptName);
void LuaRunInitScript(char* ElementName);
void __LuaStackDump (lua_State *L);
int LuaInitEnt(char* Name, lua_State *L);

//Registered Functions
static int Reg_CloseMenu(lua_State *L);

//Library functions
int sp_LuaSpawn(edict_t* ent);

//Entity List functions
void EntityListAdd(edict_t* ent);
edict_t* EntityListFind(char* Id);
void EntityListRemove(edict_t* ent);