#include <stdlib.h>
#include <string.h>
#include <menu.h>
#include "libxml/HTMLparser.h"
#include "libxml/parser.h"

#include "g_lua.h"

void MenuPush(struct MenuRenderQueue *Queue, struct MenuRenderItem* Next){
	struct MenuRenderQueueNode* Top;

	Top = calloc(1, sizeof(struct MenuRenderQueueNode));
	Top->Value = Next;
	Top->Next = NULL;
	Top->Last = Queue->Top;
	//First Item in Queue
	if(Top->Last == NULL) {
		Queue->Top = Top;
		Queue->Bottom = Top;
		return;
	}
	Top->Last->Next = Top;
}

struct MenuRenderItem* MenuPop(struct MenuRenderQueue *Queue){
	struct MenuRenderItem* Item;
	struct MenuRenderQueueNode* NewTop;
	
	Item = Queue->Top->Value;
	NewTop = Queue->Top->Last;

	//End of Queue
	if(NewTop != NULL)
		NewTop->Next = NULL;

	free(Queue->Top);
	Queue->Top = NewTop;

	return Item;
}

struct MenuRenderItem* MenuPeek(struct MenuRenderQueue *Queue){
	if(Queue == NULL) {
		return NULL;
	}
	if(Queue->Top == NULL) {
		return NULL;
	}
	return Queue->Top->Value;
}

struct MenuRenderItem* MenuBottom(struct MenuRenderQueue *Queue){
	if(Queue == NULL) {
		return NULL;
	}
	if(Queue->Bottom == NULL) {
		return NULL;
	}
	return Queue->Bottom->Value;
}

struct MenuRenderItem* MenuFindBottom(struct MenuRenderQueue *Queue, char* Name){
	struct MenuRenderQueueNode* Cursor;
	Cursor = Queue->Bottom;
	if(Cursor == NULL) {
		return NULL;
	}
	do {
		if(strcmp(Cursor->Value->Name, Name) == 0) {
			return Cursor->Value;
		}
		Cursor = Cursor->Next;
	} while(Cursor != NULL);

	return NULL;
}

struct MenuRenderItem* MenuFindTop(struct MenuRenderQueue *Queue, char* Name){
	struct MenuRenderQueueNode* Cursor;
	Cursor = Queue->Top;
	if(Cursor == NULL) {
		return NULL;
	}
	do {
		if(strcmp(Cursor->Value->Name, Name) == 0) {
			return Cursor->Value;
		}
		Cursor = Cursor->Last;
	} while(Cursor != NULL);
}

void OpenMenuFromFile(struct MenuRenderQueue *Queue, char *File) {
	htmlDocPtr Document;
	struct MenuRenderItem* MenuItem;

	Document = htmlParseFile(File, NULL);
	if(Document == NULL) {
		return;
	}
	
	MenuItem = PushFromXmlNode(Document->root, NULL);
	MenuPush(Queue, MenuItem);

	CallAllOnLoads(Queue);
}

struct MenuRenderItem* PushFromXmlNode(struct _xmlNode *Node, struct MenuRenderItem *Parent) {
	struct MenuRenderItem* MenuItem;
	struct _xmlNode *Child;
	size_t NumChildren = 0;
	char* Tmp;
	char* IdTmp;

	//Script tag
	MenuItem = calloc(1, sizeof(struct MenuRenderItem));
	MenuItem->Parent = Parent;
	
	//Allocate and assign attributes
	//Name
	if(Node->name != NULL) {
		MenuItem->Name = calloc(strlen(Node->name) + 1, sizeof(char));
		strcpy(MenuItem->Name, Node->name);

		if(strcmp(Node->name, "script") == 0) {
			IdTmp = xmlGetProp(Node, "Src");
			if(IdTmp != NULL) {
				LuaLoadScript(IdTmp);
			}
			return MenuItem;
		}
	}

	//Id
	IdTmp = xmlGetProp(
		Node,
		"Id"
	);
	if(IdTmp != NULL) {
		MenuItem->Id = calloc(strlen(IdTmp) + 1, sizeof(char));
		strcpy(MenuItem->Id, IdTmp);
		LuaCreateDom(IdTmp);
	}

	//Path
	Tmp = xmlGetProp(
		Node,
		"Image"
	);

	if(Tmp != NULL) {
		MenuItem->Image = calloc(strlen(Tmp) + 1, sizeof(char));
		strcpy(MenuItem->Image, Tmp);
	}

	Tmp = Node->content;
	if(Tmp != NULL) {
		MenuItem->Body = calloc(strlen(Tmp) +1, sizeof(char));
		strcpy(MenuItem->Body, Tmp);
	}

	//X, Y, W, H
	Tmp = xmlGetProp(Node, "X");
	if(Tmp != NULL)
		MenuItem->X = atoi(Tmp);

	Tmp = xmlGetProp(Node, "Y");
	if(Tmp != NULL)
		MenuItem->Y = atoi(Tmp);

	Tmp = xmlGetProp(Node, "W");
	if(Tmp != NULL)
		MenuItem->W = atoi(Tmp);

	Tmp = xmlGetProp(Node, "H");
	if(Tmp != NULL)
		MenuItem->H = atoi(Tmp);

	for(Child = Node->xmlChildrenNode; Child != NULL; Child = Child->next) {
		NumChildren++;
	}

	if(NumChildren == 0) {
		MenuItem->DirectChildren = NULL;
		return MenuItem;
	}
	MenuItem->DirectChildren = calloc(sizeof(struct MenuRenderItem*), NumChildren + 1);
	NumChildren = 0;

	Child = Node->childs;
	while(Child != NULL){
		MenuItem->DirectChildren[NumChildren++] = PushFromXmlNode(Child, MenuItem);
		Child = Child->next;
	}
	
	return MenuItem;
}

void CallAllOnLoads(struct MenuRenderQueue *Queue) {
	struct MenuRenderItem* Item;
	Item = MenuPeek(Queue);
	if(Item == NULL) {
		return;
	}
	BreadthFirst(Item, OnLoad);
}
void BreadthFirst(struct MenuRenderItem *Item, void (*Function)(struct MenuRenderItem*)) {
	struct MenuRenderItem** Children;

	if(Item == NULL)
		return;

	Function(Item);

	Children = Item->DirectChildren;
	if(Children == NULL) {
		return;
	}
	while(*Children != NULL) {
		BreadthFirst(*Children, Function);
		Children++;
	}

}
void DepthFirst(struct MenuRenderItem *Item, void (*Function)(struct MenuRenderItem*)) {
	struct MenuRenderItem** Children;

	if(Item == NULL)
		return;

	Children = Item->DirectChildren;
	if(Children == NULL) {
		return;
	}
	while(*Children != NULL) {
		BreadthFirst(*Children, Function);
		Children++;
	}
	Function(Item);
}

void CloseTopLevelMenu(struct MenuRenderQueue *Queue) {
	struct MenuRenderQueueNode *Node = Queue->Top;
	struct MenuRenderQueueNode* NewTop;
	
	NewTop = Node->Last;
	CloseMenu(Node);

	//End of Queue
	if(NewTop != NULL)
		NewTop->Next = NULL;
	else
		Queue->Bottom = NULL;
	
	Queue->Top = NewTop;
}

void CloseMenu(struct MenuRenderQueueNode *Node) {
	if(Node == NULL)
		return;
	if(Node->Value != NULL){
		CloseMenuItem(Node->Value);
	}
	free(Node);
}

void CloseMenuItem(struct MenuRenderItem *Item) {
	struct MenuRenderItem** Children;
	
	if(Item == NULL)
		return;

	if(Item->Name != NULL)
		free(Item->Name);
	if(Item->Image != NULL)
		free(Item->Image);
	if(Item->Src != NULL)
		free(Item->Src);
	if(Item->Id != NULL)
		free(Item->Id);
	

	Children = Item->DirectChildren;
	if(Children == NULL)
		goto FreeChild;

	while(*Children != NULL) {
		CloseMenuItem(*Children);
		Children++;
	}
	free(Item->DirectChildren);

	FreeChild:
	free(Item);
}
void CloseAllMenus(struct MenuRenderQueue *Queue) {
	struct MenuRenderNode *Top = Queue->Top;
	while(Top != NULL) {
		CloseTopLevelMenu(Queue);
		Top = Queue->Top;
	}
}

//Tree Operations
void OnLoad(struct MenuRenderItem* Item) {
	if(Item == NULL)
		return;
	if(Item->Id == NULL)
		return;

	LuaRunInitScript(Item->Id);
}

struct MenuRenderItem* FindItemXY(struct MenuRenderItem* Item, long X, long Y){
	struct MenuRenderItem **Children;
	struct MenuRenderItem *FoundItem;
	if(Item == NULL)
		return NULL;
	
	Children = Item->DirectChildren;
	if(Children == NULL)
		goto CheckSelf;

	while(*Children != NULL) {
		FoundItem = FindItemXY(*Children, X, Y);
		Children++;
		if(FoundItem != NULL)
			return FoundItem;
	}

	CheckSelf:
	if(X >= Item->X && Y > Item->Y && X <= Item->X + Item->W && Y < Item->Y + Item->H) {
		return Item;
	}
	return NULL;
}

struct MenuRenderItem* FindItemFromAllXY(struct MenuRenderQueue* Queue, long X, long Y){
	return NULL;
}

void DispatchEvent(char* Id, char* Event){
	lua_getglobal(state, "Document");
	lua_getfield(state, -1, Id);
	if(lua_isnil(state, -1)) {
		printf("ElementName not found!\n");
		lua_settop(state, -2);
		return;
	}
	lua_getfield(state, -1, Event);
	if(lua_isnil(state, -1)) {
		printf("Onload not found!\n");
		lua_settop(state, -3);
		return;
	}

	lua_pcall(state, 0, 0, 0);
	lua_settop(state, -3);
	
}
