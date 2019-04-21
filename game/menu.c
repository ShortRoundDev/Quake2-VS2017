#include <stdlib.h>
#include <string.h>
#include <menu.h>
#include "libxml/HTMLparser.h"
#include "libxml/parser.h"

#include "g_lua.h"
#include "game.h"
extern game_import_t gi;

void GenericPush(struct GenericList* List, void* Value) {
	if (List == NULL)
		return;
	struct GenericListNode* NewNode = calloc(1, sizeof(struct GenericListNode));
	NewNode->Value = Value;
	
	//First in list
	if (List->Head == NULL) {
		List->Head = NewNode;
		List->Tail = NewNode;
		return;
	}

	struct GenericListNode* Tail = List->Tail;
	Tail->Next = NewNode;
	NewNode->Last = Tail;
	List->Tail = NewNode;
}
void* GenericPop(struct GenericList* List) {
	if (List == NULL)
		return NULL;
	if (List->Tail == NULL)
		return NULL;
	struct GenericListNode* Tmp = List->Tail->Last;
	struct GenericListNode* Popped = List->Tail;

	//Last element in list
	if (Tmp == NULL) {
		List->Head = NULL;
		List->Tail = NULL;
		return Popped;
	}

	Tmp->Next = NULL;
	List->Tail = Tmp;
	return Popped;
}
void* GenericPeek(struct GenericList* List) {
	if (List == NULL)
		return NULL;
	return List->Tail;
}

void* GenericFind(struct GenericList* List, void* Argument, void* (*Finder)(void*, void*)) {
	if (List == NULL)
		return NULL;
	if (List->Head == NULL)
		return NULL;
	struct GenericListNode* Cursor = List->Head;
	while (Cursor != NULL) {
		void* Out = Finder(Cursor->Value, Argument);
		if (Out)
			return Out;
	}
	return NULL;
}

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

void OpenMenuFromString(struct MenuRenderQueue *Queue, char *XML) {
	htmlDocPtr Document = htmlParseDoc(XML, NULL);
	if (Document == NULL) {
		return;
	}
	struct MenuRenderItem* MenuItem = PushFromXmlNode(Document->root, NULL);
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

		if (strcmp(Node->name, "script") == 0) {
			IdTmp = xmlGetProp(Node, "Src");
			if (IdTmp != NULL) {
				LuaLoadScript(IdTmp);
			}
			return MenuItem;
		}

		char* FontVal = xmlGetProp(Node, "Font");
		if (FontVal) {
			printf("Font == NULL: %d", MenuItem->Font == NULL);
			MenuItem->Font = LoadFont(FontVal);
			printf("Font == NULL: %d", MenuItem->Font == NULL);
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

char* FindInDictionary(struct ModelDictionary* Model, char* Key) {
	//For now, just linear lookup.
	//TODO: Hash instead
	int i = 0;
	for (struct ModelDictionaryEntry* Cursor = Model->Dictionary; i < Model->Size; Cursor++, i++) {
		if (Cursor->Key == NULL)
			continue;
		if (!strcmp(Cursor->Key, Key)) {
			return Cursor->Value;
		}
	}
	return NULL;
}

void InsertInDictionary(struct ModelDictionary* Model, char* Key, char* Value) {
	if (Model->Top >= Model->Size) {		//BAD
		printf("ERROR, Dictionary out of room\n");
		return;
	}
	Model->Dictionary[Model->Top].Key = malloc((strlen(Key) + 1) * sizeof(char));
	strcpy(Model->Dictionary[Model->Top].Key, Key);

	Model->Dictionary[Model->Top].Value = malloc((strlen(Value) + 1) * sizeof(char));
	strcpy(Model->Dictionary[Model->Top].Value, Value);
	Model->Top++;
}

void DestroyDictionary(struct ModelDictionary* Model) {
	int i = 0;
	for (struct ModelDictionaryEntry* Cursor = Model->Dictionary; i < Model->Size; Cursor++, i++) {
		if (Cursor->Key != NULL)
			free(Cursor->Key);
		if (Cursor->Value != NULL)
			free(Cursor->Value);
	}
	free(Model->Dictionary);
	free(Model);
}

char* Interpolate(char* InXML, struct ModelDictionary* Model) {
	if (Model == NULL)
		return InXML;

	//Traverse until you reach something like {{Key}}
	//Replace that with Model[Key]
	//Continue until end of string
	//return new String

	char* Buffer = calloc(
		(strlen(InXML) + Model->BufferSize + 1),
		sizeof(char)
	);

	//My shit way of parsing stuff
	int NumBrackets = 0;
	int StartString = 0;
	int EndString = 0;
	int EndLastString = 0;
	size_t XMLSize = strlen(InXML);
	for (int i = 0; i < strlen(InXML); i++) {
		if (InXML[i] == '{') {
			NumBrackets++;
			if (NumBrackets == 2) {
				StartString = i + 1;
			}
		}
		if (InXML[i] == '}') {
			if (NumBrackets <= 1) {
				NumBrackets = 0;
				continue;
			}
			NumBrackets++;
			if (NumBrackets == 4) {
				EndString = i - 2;

				char* Key = calloc(EndString - StartString + 2, sizeof(char));
				strncpy(Key, InXML + StartString, EndString - StartString + 1);
				printf("Key: %s\n", Key);
				char* Value = FindInDictionary(Model, Key);
				if (Value == NULL) {
					strncat(Buffer, InXML + EndLastString, i - EndLastString);
				}
				else {
					strncat(Buffer, InXML + EndLastString, (StartString - 2) - EndLastString);
					strncat(Buffer, Value, strlen(Value));
				}
				EndLastString = i + 1;
				NumBrackets = 0;
			}
		}
		if (InXML[i] == '\\') {
			if (i + 1 != XMLSize) {
				if (InXML[i + 1] == '{' || InXML[i + 1] == '}')
					i++;
			}
		}
	}
	strncat(Buffer, InXML + EndLastString, strlen(InXML) - EndLastString);
	return Buffer;
}

inline void* FindFontByName(void* _Font, void* _Name) {
	struct MenuRenderFont* Font = (struct MenuRenderFont*)_Font;
	char* Name = (char*)_Name;

	return !strcmp(Font->Name, Name) ? (void*)Font : NULL;
}

struct MenuRenderFont* LoadFont(char* Name) {
	printf("FONT: Loading Font...");
	struct MenuRenderFont* Font = (struct MenuRenderFont*) GenericFind(&Fonts, Name, FindFontByName);
	if (Font == NULL) {
		printf("Font not found...Loading from File...\n");
		Font = OpenFontFileConfiguration(Name);
		GenericPush(&Fonts, (void*)Font);
	}
	printf("Done Loading Font\n");
	return Font;
}

struct MenuRenderFont* OpenFontFileConfiguration(char* Name) {
	printf("Opening font configuration file...");
	struct MenuRenderFont* Font = calloc(1, sizeof(struct MenuRenderFont));
	Font->Name = calloc(strlen(Name) + 1, sizeof(char));
	strcpy(Font->Name, Name);
	char* FontFileName = calloc(
		strlen("baseq2/fonts/") +
		strlen(Name) +
		strlen(".txt") + 1,
		sizeof(char)
	);
	strcpy(FontFileName, "baseq2/fonts/");
	strcat(FontFileName, Name);
	strcat(FontFileName, ".txt");

	FILE* fp = fopen(FontFileName, "r");
	if (!fp) {
		printf("Configuration not found. Returning NULL\n");
		return NULL;
	}
	printf("Found configuration. Configuring...");
	fseek(fp, 0, SEEK_END);
	size_t Size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* Text = calloc(Size + 1, sizeof(char));

	fread(Text, sizeof(char), Size, fp);
	fclose(fp);
	free(FontFileName);
	char* Token = strtok(Text, "=\n");
	int i = 1;
	char* Key;
	char* Value;
	while (Token != NULL) {
		if (i++ & 1) {
			Key = Token;
			printf("Got Key: %s\n", Key);
		}
		else {
			Value = Token;
			printf("Got value: %s\n", Value);
			if (strcmp(Key, "Height") == 0) {
				Font->Height = atoi(Value);
				printf("Got Height: %d\n", Font->Height);
			}
			else if (strcmp(Key, "Width") == 0) {
				Font->Width = atoi(Value);
				printf("Got Width: %d\n", Font->Width);
			}
			else if (strcmp(Key, "Offset") == 0) {
				Font->Offset = atoi(Value);
				printf("Got Offset: %d", Font->Offset);
			}
			else if (strcmp(Key, "FontImage") == 0) {
				Font->FontImage = gi.re->RegisterPic(Value);
				printf("Got FontImage Filename: %s\n", Value);
			}
		}
		Token = strtok(NULL, "=\n");
	}
	printf("Done tokenizing!\n");
	free(Text);
	return Font;
}