#include "g_lua.h"
#include "menu.h"
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

	lua_pushcfunction(state, LuaMove);
	lua_setglobal(state, "Move");

	lua_pushcfunction(state, LuaLook);
	lua_setglobal(state, "Look");

	lua_pushcfunction(state, LuaGetEntByName);
	lua_setglobal(state, "GetByName");

	lua_pushcfunction(state, LuaMenuBindWithModel);
	lua_setglobal(state, "OpenMenu");

	LuaLoadScript("baseq2/scripts/main.lua");

	EntityList.Head = NULL;
	EntityList.Tail = NULL;
}

void LuaDestroy(void) {
	lua_close(state);
}

void LuaCreateDom(char* ElementName) {
	LuaCreateGlobalTable("Document", ElementName);
}

int LuaCreateEntityType(char* EntityName) {
	LuaCreateGlobalTable("Entity", EntityName);
}

void LuaCreateGlobalTable(char* Global, char* Table) {
	lua_getglobal(state, Global);
	lua_getfield(state, -1, Table);

	//Entity Definition already exists
	if (!lua_isnil(state, -1)) {
		lua_settop(state, -2);
		return 0;
	}
	lua_pop(state, 1);

	lua_createtable(state, 0, 1);
	lua_setfield(state, -2, Table);
	lua_pop(state, 1);
	return 1;
}

void LuaLoadScript(char* ScriptName) {
	int err = 0;
	err = luaL_dofile(state, ScriptName);
}

void LuaRunInitScript(char* ElementName) {
	LuaCallGlobalObjectFunction("Document", ElementName, "OnLoad");
}

void LuaInitEntity(edict_t* ent) {
	LuaCallEntityFunction(ent, "Init");
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
	LuaSetFieldFloat("y", ent->s.origin[0]);
	LuaSetFieldFloat("x", ent->s.origin[1]);

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
	if (LuaCreateEntityType(ent->classname))
		LuaLoadScript(FilePathName);

	LuaInitEntity(ent);
	free(FilePathName);
	ent->think = LuaThink;
	ent->monsterinfo.scale = 10.0f;
	VectorSet(ent->mins, -16, -16, -24);
	VectorSet(ent->maxs, 16, 16, 32);
	ent->movetype = MOVETYPE_STEP;
	ent->solid = SOLID_BBOX;
gi.linkentity(ent);
}

static int LuaCallGlobalObjectFunctionWithEnt(const char* Global, const char* Object, const char* Function, edict_t* ent) {
	_Bool Kill = false;
	if (Global == NULL) {
		printf("ERROR: Global cannot be NULL!\n");
		Kill = true;
	}

	if (Object == NULL) {
		printf("ERROR: Object cannot be NULL!\n");
		Kill = true;
	}

	if (Object == NULL) {
		printf("ERROR: Function cannot be NULL!\n");
		Kill = true;
	}

	if (Kill)
	return -1;

	lua_getglobal(state, Global);
	if (lua_isnil(state, -1)) {
		printf("Global table `%s` has not been initialized!\n", Global);
		lua_settop(state, -1);
		return;
	}
	lua_getfield(state, -1, Object);
	if (lua_isnil(state, -1)) {
		printf("`%s` not found in Global table `%s`!\n", Object, Global);
		lua_settop(state, -2);
		return;
	}
	lua_getfield(state, -1, Function);
	if (lua_isnil(state, -1)) {
		printf("Function `%s` not found in Object `%s` under Global table `%s`!\n", Function, Object, Global);
		lua_settop(state, -3);
		return;
	}
	if (!lua_isfunction(state, -1)) {
		printf("`%s` is not a function!\n", Function);
		lua_settop(state, -3);
		return;
	}
	if (ent)
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

static int LuaCallGlobalObjectFunction(const char* Global, const char* Object, const char* Function) {
	LuaCallGlobalObjectFunctionWithEnt(Global, Object, Function, NULL);
}

static int LuaCallEntityFunction(edict_t* ent, const char* FunctionName) {
	LuaCallGlobalObjectFunctionWithEnt("Entity", ent->classname, FunctionName, ent);
}

static int LuaThink(edict_t* p) {
	LuaCallEntityFunction(p, "Think");
}

static int LuaMove(lua_State* L) {
	double Dist = lua_tonumber(L, -1);
	lua_pop(L, 1);
	double Direction = lua_tonumber(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, -1, "entId");
	char* entId = lua_tostring(L, -1);
	lua_pop(L, 2);
	edict_t* ent = EntityListFind(entId);
	M_walkmove(ent, Direction, Dist);
}

static int LuaLook(lua_State* L) {
	double Direction = lua_tonumber(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, -1, "entId");
	char* entId = lua_tostring(L, -1);
	lua_pop(L, 2);
	edict_t* ent = EntityListFind(entId);
	ent->s.angles[YAW] = Direction;
}

static int LuaMenuBindWithModel(lua_State* L) {
	struct ModelDictionary* Model = NULL;
	if (lua_istable(L, -1)) {
		int ModelSize = 0;
		lua_pushnil(L);

		while (lua_next(L, -2 - (ModelSize * 2)) != 0) {
			char* Key = lua_tostring(L, -2);
			lua_pushstring(L, Key);
			ModelSize++;
		}
		printf("Modelsize: %d\n", ModelSize);
		__LuaStackDump(L);


		Model = calloc(sizeof(struct ModelDictionary), 1);
		Model->Dictionary = calloc(sizeof(struct ModelDictionaryEntry), ModelSize);
		Model->Size = ModelSize;

		for(int i = ModelSize; i > 0; i--) {
			if (lua_istable(L, -1)) {
				break;
			}
			char* Value = lua_tostring(L, -1);
			printf("Value: %s\n", Value);
			InsertInDictionary(
				Model,
				lua_tostring(L, -2),
				Value
			);

			Model->BufferSize += strlen(Value);

			lua_pop(L, 2);
		}
	}
	else if (!lua_isnil(L, -1)) {
		printf("ERROR: To open a menu without binding a model, pass `nil` for the model\n");
		return 0;
	}
	//Pop Table or Nil, whatever
	lua_pop(L, 1);

	char* XML = lua_tostring(L, -1);
	char* Interpolated = Interpolate(XML, Model);
	DestroyDictionary(Model);
	OpenMenuFromString(&Queue, Interpolated);
	free(Interpolated);
}

void LuaUse(edict_t *p) {
	LuaCallEntityFunction(p, "Use");
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

static int LuaGetEntByName(lua_State* L) {
	const char* Name = lua_tostring(L, -1);
	lua_pop(L, 1);
	edict_t* Ent = EntityListFind(Name);
	LuaPushEnt(Ent);
	return 1;
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