#ifndef __menu_h
#define __menu_h

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
	struct MenuRenderItem** DirectChildren;
	struct MenurenderItem* Parent;
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

/**Pushes a menurenderItem*/
void MenuPush(struct MenuRenderQueue* Queue, struct MenuRenderItem* Next);
struct MenuRenderItem* MenuPop(struct MenuRenderQueue *Queue);
struct MenuRenderItem* MenuPeek(struct MenuRenderQueue *Queue);
struct MenuRenderItem* MenuBottom(struct MenuRenderQueue *Queue);
struct MenuRenderItem* MenuFindBottom(struct MenuRenderQueue *Queue, char* Name);
struct MenuRenderItem* MenuFindTop(struct MenuRenderQueue *Queue, char* Name);

void OpenMenuFromFile(struct MenuRenderQueue *Queue, char *File);
struct MenuRenderItem* PushFromXmlNode(struct _xmlNode *Node);
void CallAllOnLoads(struct MenuRenderQueue *Queue);
void BreadthFirst(struct MenuRenderItem *Item, void (*Function)(struct MenuRenderItem*));
void DepthFirst(struct MenuRenderItem *Item, void (*Function)(struct MenuRenderItem*));
void CloseTopLevelMenu(struct MenuRenderQueue *Queue);
void CloseMenu(struct NenuRenderQueueNode *Node);
void CloseMenuItem(struct MenuRenderItem *Item);
void CloseAllMenus(struct MenuRenderQueue *Queue);

struct MenuRenderItem* FindItemXY(struct MenuRenderItem* Item, long X, long Y);
struct MenuRenderItem* FindItemFromAllXY(struct MenuRenderQueue* Queue, long X, long Y);
void DispatchEvent(char* Id, char* Event);

//Tree Operations
void OnLoad(struct MenuRenderItem* Item);
#endif