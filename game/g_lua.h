#pragma once
#include "lua.h"
#include "lualib.h"
#include <inttypes.h>
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
int LuaCreateEntityType(char* EntityName);
void LuaCreateGlobalTable(char* Global, char* Table);
void LuaLoadScript(char* ScriptName);
void LuaRunInitScript(char* ElementName);
void LuaInitEntity(edict_t* ent);
void __LuaStackDump (lua_State *L);

//Table Manipulation
void LuaSetFieldUint32_t(const char* Field, uint32_t Value);
void LuaSetFieldString(const char* Field, const char* Value);
void LuaSetFieldBoolean(const char* Field, int Value);
void LuaSetFieldFloat(const char* Field, float Value);
void LuaSetFieldNil(const char* Field);
void LuaPushEnt(edict_t* ent);

//Registered Functions
static int Reg_CloseMenu(lua_State *L);

//Library functions
int sp_LuaSpawn(edict_t* ent);
static int LuaPrint(lua_State* L);
static int LuaGetEntByName(lua_State* L);
static int LuaSetModel(lua_State* L);
static int LuaCallGlobalObjectFunction(const char* Global, const char* Object, const char* Function);
static int LuaCallGlobalObjectFunctionWithEnt(const char* Global, const char* Object, const char* Function, edict_t* ent);
static int LuaCallEntityFunction(edict_t* ent, const char* Function);
static int LuaThink(edict_t* p);
static int LuaMove(lua_State* L);
static int LuaLook(lua_State* L);
static int LuaMenuBindWithModel(lua_State* L);
void LuaUse(edict_t *ent);

//Entity List functions
void EntityListAdd(edict_t* ent);
edict_t* EntityListFind(char* Id);
void EntityListRemove(edict_t* ent);

//Forward declarations:
qboolean M_walkmove(edict_t *ent, float yaw, float dist);