#pragma once

struct MenuRenderItem {
	char *Name;
	char *Image;
	char *Id;
	char *Src;
	char *Body;
	int X;
	int Y;
	int W;
	int H;
	struct MenuRenderFont* Font;
	struct MenuRenderItem** DirectChildren;
	struct MenuRenderItem* Parent;
};

struct GenericListNode {
	void* Value;
	struct GenericListNode* Next;
	struct GenericListNode* Last;
};

struct GenericList {
	struct GenericListNode* Head;
	struct GenericListNode* Tail;
}
Fonts;

struct MenuRenderFont {
	char* Name;
	struct image_s* FontImage;
	int Height;
	int Width;
	int Offset;
};

struct MenuRenderQueueNode {
	struct MenuRenderItem* Value;
	struct MenuRenderQueueNode* Next;
	struct MenuRenderQueueNode* Last;
};

struct MenuRenderQueue {
	struct MenuRenderQueueNode* Bottom;
	struct MenuRenderQueueNode* Top;
};

/*Model Dict*/
struct ModelDictionaryEntry {
	char* Key;
	char* Value;
};

struct ModelDictionary {
	int Size;
	int BufferSize;
	int Top;	//BAD
	struct ModelDictionaryEntry* Dictionary;
};

//Generic list; must cast from void* to whatever the expected value is and vice-versa
void GenericPush(struct GenericList* List, void* Value);
void* GenericPop(struct GenericList* List);
void* GenericPeek(struct GenericList* List);
void* GenericFind(struct GenericList* List, void* Argument, void* (*Finder)(void*, void*));

/**Pushes a menurenderItem*/
void MenuPush(struct MenuRenderQueue* Queue, struct MenuRenderItem* Next);
struct MenuRenderItem* MenuPop(struct MenuRenderQueue *Queue);
struct MenuRenderItem* MenuPeek(struct MenuRenderQueue *Queue);
struct MenuRenderItem* MenuBottom(struct MenuRenderQueue *Queue);
struct MenuRenderItem* MenuFindBottom(struct MenuRenderQueue *Queue, char* Name);
struct MenuRenderItem* MenuFindTop(struct MenuRenderQueue *Queue, char* Name);

void OpenMenuFromFile(struct MenuRenderQueue *Queue, char *File);
void OpenMenuFromString(struct MenuRenderQueue *Queue, char* XML);\

//void InterpolateMenu(char* MenuXML, )

struct MenuRenderItem* PushFromXmlNode(struct _xmlNode *Node);
void CallAllOnLoads(struct MenuRenderQueue *Queue);
void BreadthFirst(struct MenuRenderItem *Item, void(*Function)(struct MenuRenderItem*));
void DepthFirst(struct MenuRenderItem *Item, void(*Function)(struct MenuRenderItem*));
void CloseMenu(struct NenuRenderQueueNode *Node);
void CloseMenuItem(struct MenuRenderItem *Item);
void CloseAllMenus(struct MenuRenderQueue *Queue);

void DrawItem(struct MenuRenderItem *Item);
void DrawStack(struct MenuRenderQueue *Queue);
void DrawMenuText(struct MenuRenderItem *Item);

//Tree Operations
void OnLoad(struct MenuRenderItem* Item);

char* Interpolate(char* InXML, struct ModelDictionary* Model);

char* FindInDictionary(struct ModelDictionary* Model, char* Key);
void InsertInDictionary(struct ModelDictionary* Model, char* Key, char* Value);
void DestroyDictionary(struct ModelDictionary* Model);

//Fonts
struct MenuRenderFont* LoadFont(char* Name);

struct MenuRenderFont* OpenFontFileConfiguration(char* Name);