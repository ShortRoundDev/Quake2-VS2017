#ifndef __menu_h
#define __menu_h

struct MenuRenderItem {
	char *Name;
	int X;
	int Y;
	int W;
	int H;
	struct MenuRenderItem* DirectChildren;
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
void MenuPush(struct MenuRenderQueue *Queue, struct MenuRenderItem* Next);
struct MenuRenderItem* MenuPop(struct MenuRenderQueue *Queue);
struct MenuRenderItem* MenuPeek(struct MenuRenderQueue *Queue);
struct MenuRenderItem* MenuBottom(struct MenuRenderQueue *Queue);
struct MenuRenderItem* MenuFind(struct MenuRenderQueue *Queue, char* Name);

#endif