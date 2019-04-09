#include "g_lua.h"
#include "g_local.h"
//lua_State* state;

void LuaInit(void) {
	state = luaL_newstate();
	luaL_openlibs(state);

	//Register functions
	lua_pushcfunction(state, Reg_CloseMenu);
	lua_setglobal(state, "CloseMenu");

	lua_pushcfunction(state, LuaPrint);
	lua_setglobal(state, "print");

	lua_pushcfunction(state, LuaSetModel);
	lua_setglobal(state, "SetModel");

	LuaLoadScript("baseq2/scripts/main.lua");

	EntityList.Head = NULL;
	EntityList.Tail = NULL;
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

void LuaCreateEntityType(char* EntityName) {
	lua_getglobal(state, "Entity");
	lua_createtable(state, 0, 1);
	lua_setfield(state, -2, EntityName);
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

void LuaInitEntity(char* EntityName, edict_t* ent) {
	printf("Loading %s\n", EntityName);
	lua_getglobal(state, "Entity");
	if (lua_isnil(state, -1)) {
		printf("Global table `Entity` has not been initialized!\n");
		lua_settop(state, -1);
		return;
	}
	lua_getfield(state, -1, EntityName);
	if (lua_isnil(state, -1)) {
		printf("ElementName not found!\n");
		lua_settop(state, -2);
		return;
	}
	lua_getfield(state, -1, "Init");
	if (lua_isnil(state, -1)) {
		printf("Init not found!\n");
		lua_settop(state, -3);
		return;
	}
	if (!lua_isfunction(state, -1)) {
		printf("Init is not a function for some reason...");
		lua_settop(state, -3);
		return;
	}
	LuaPushEnt(ent);
	int err = lua_pcall(state, 1, 0, 0);
	if (err == LUA_ERRRUN)
		printf("ERRRUN\n");
	if (err == LUA_ERRMEM)
		printf("ERRMEM\n");
	if (err == LUA_ERRERR)
		printf("ERRERR\n");
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

void LuaSetFieldUint32_t(const char* Field, uint32_t Value) {
	lua_pushinteger(state, Value);
	lua_setfield(state, -2, Field);
}
void LuaSetFieldString(const char* Field, const char* Value) {
	if (Value == NULL)
		LuaSetFieldNil(Field);
	lua_pushstring(state, Value);
	lua_setfield(state, -2, Field);
}
void LuaSetFieldBoolean(const char* Field, int Value) {
	lua_pushboolean(state, Value);
	lua_setfield(state, -2, Field);
}
void LuaSetFieldFloat(const char* Field, float Value) {
	lua_pushnumber(state, Value);
	lua_setfield(state, -2, Field);
}
void LuaSetFieldNil(const char* Field) {
	lua_pushnil(state);
	lua_setfield(state, -2, Field);
}

void LuaPushEnt(edict_t* ent) {
	lua_createtable(state, 0, 20);

	//Custom fields. TODO: Add non-custom fields as well
	LuaSetFieldString("entId", ent->entId);
	LuaSetFieldString("luaClassName", ent->luaClassName);
	LuaSetFieldUint32_t("movetype", ent->movetype);
	LuaSetFieldUint32_t("flags", ent->flags);
	LuaSetFieldString("model", ent->model);
	LuaSetFieldFloat("freetime", ent->freetime);
	LuaSetFieldString("message", ent->message);
}

//Registered Functions
static int Reg_CloseMenu(lua_State *L) {
	CloseTopLevelMenu(&Queue);
}

//Library functions
int sp_LuaSpawn(edict_t* ent) {
	//generic, inline lua entity
	if (!strcmp(ent->classname, "lua_ent")) {
		if (ent->luaClassName == NULL) {
			printf("ERROR: Missing Lua Class Name in lua_ent! Skipping\n");
			return;
		}
		ent->classname = ent->luaClassName;
		printf("Setting lua classname: %s\n", ent->classname);
	}
	printf("Loading Lua Entity: %s\n, {%s}", ent->classname, ent->entId);
	char* FilePathName = calloc(
		strlen("scripts/entities/") +
		strlen(ent->classname) +
		strlen("/entity.lua") + 1,
		sizeof(char)
	); //+1 for null terminator

	strcpy(FilePathName, "baseq2/scripts/entities/");
	strcat(FilePathName, ent->classname);
	strcat(FilePathName, "/entity.lua");

	//Initialize class type
	LuaCreateEntityType(ent->classname);

	LuaLoadScript(FilePathName);
	LuaInitEntity(ent->classname, ent);
	free(FilePathName);
	ent->think = LuaThink;
	gi.linkentity(ent);
}

static int LuaThink(edict_t* p) {
	printf("THINKING!\n");
}

static int LuaPrint(lua_State* L) {
	int nargs = lua_gettop(L);
	for (int i = 1; i <= nargs; i++) {
		if (lua_isstring(L, i)) {
			printf("%s", lua_tostring(L, i));
		}
	}
	printf("\n");
}

static int LuaSetModel(lua_State* L) {
	char *Path = lua_tostring(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, -1, "entId");
	char* entId = lua_tostring(L, -1);
	lua_pop(L, 2);
	
	edict_t* ent = EntityListFind(entId);
	if (ent == NULL) {
		printf("Couldn't find the entity\n");
	}
	else {
		printf("Path: %s\n", Path);
		ent->s.modelindex = gi.modelindex(Path);
	}
		
}

//Entity List functions

void EntityListAdd(edict_t* ent) {
	if (ent == NULL) {
		printf("ERROR! ent must not be null!\n");
	}
	struct EntityListNode* Node = calloc(1, sizeof(struct EntityListNode));
	Node->Value = ent;
	Node->Next = NULL;
	
	if (EntityList.Head == NULL) {
		EntityList.Head = Node;
		EntityList.Tail = Node;
		return;
	}

	struct EntityListNode* Cursor = EntityList.Head;
	if (Cursor == NULL) {
		printf("ERROR: NULL HEAD ON ADD!\n");
		return;
	}
	while (Cursor->Next != NULL) {
		Cursor = Cursor->Next;
	}

	Cursor->Next = Node;
	EntityList.Tail = Node;
}

edict_t* EntityListFind(char* Id) {

	if (Id == NULL) {
		printf("ERROR: Id must not be null!\n");
		return NULL;
	}

	struct EntityListNode* Cursor = EntityList.Head;
	while (Cursor != NULL) {
		if (Cursor->Value == NULL) {
			printf("ERROR! Null values in entity list! Check spawn logic\n");
			return NULL;
		}
		if (Cursor->Value->entId == NULL) {
			printf("ERROR! Null ID In entity! Check spawn logic\n");
			return NULL;
		}
		if (strcmp(Cursor->Value->entId, Id) == 0) {
			return Cursor->Value;
		}
		Cursor = Cursor->Next;
	}
	return NULL;
}

void EntityListRemove(edict_t* ent) {
	if (ent == NULL) {
		printf("ERROR: ent must not be null!\n");
		return;
	}
	if (ent->entId == NULL) {
		printf("This is not a removable entity! entID must not be null. You are either using a native entity or your entity was not properly initialized\n");
	}
	struct EntityListNode* Cursor = EntityList.Head;
	if (Cursor == NULL)
		return;
	if (Cursor->Value == NULL) {
		printf("ERROR: Null values in entity list! check spawn logic\n");
		return;
	}
	if (Cursor->Value->entId == NULL) {
		printf("ERROR! Null ID In entity! Check spawn logic\n");
		return;
	}
	if (strcmp(Cursor->Value->entId, ent->entId) == 0) {
		EntityList.Head = Cursor->Next;

		//only item in list
		if (EntityList.Tail == Cursor) {
			EntityList.Tail = NULL;
		}

		//TODO: Remove head logic
	}

	while (Cursor->Next != NULL) {
		if (Cursor->Next->Value == NULL) {
			printf("ERROR! Null values in entity list! Check spawn logic\n");
			return;
		}
		if (Cursor->Next->Value->entId == NULL) {
			printf("ERROR! Null ID In entity! Check spawn logic\n");
			return;
		}
		if (strcmp(Cursor->Next->Value->entId, ent->entId) == 0) {
			break;
		}
	}
	
	if (Cursor->Next == NULL)
		return;

	struct edict_t* ToRemove = Cursor->Next->Value;
	struct EntityListNode* Tmp = Cursor->Next;
	Cursor->Next = Cursor->Next->Next;
	free(Tmp);
	G_FreeEdict(ToRemove);
}