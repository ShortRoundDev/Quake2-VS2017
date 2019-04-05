#include "game/menu.h"

#include <stdlib.h>
#include <string.h>

void MenuPush(struct MenuRenderQueue *Queue, struct MenuRenderItem* Next) {
	struct MenuRenderQueueNode* Top;

	Top = malloc(sizeof(struct MenuRenderQueueNode));
	Top->Value = Next;
	Top->Next = NULL;
	Top->Last = Queue->Top;
	//First Item in Queue
	if (Top->Last == NULL) {
		Queue->Top = Top;
		Queue->Bottom = Top;
		return;
	}
	Top->Last->Next = Top;
}

struct MenuRenderItem* MenuPop(struct MenuRenderQueue *Queue) {
	struct MenuRenderItem* Item;
	struct MenuRenderQueueNode* NewTop;

	Item = Queue->Top->Value;
	NewTop = Queue->Top->Last;

	//End of Queue
	if (NewTop != NULL)
		NewTop->Next = NULL;

	free(Queue->Top);
	Queue->Top = NewTop;

	return Item;
}

struct MenuRenderItem* MenuPeek(struct MenuRenderQueue *Queue) {
	return Queue->Top->Value;
}

struct MenuRenderItem* MenuBottom(struct MenuRenderQueue *Queue) {
	return Queue->Bottom->Value;
}

struct MenuRenderItem* MenuFindBottom(struct MenuRenderQueue *Queue, char* Name) {
	struct MenuRenderQueueNode* Cursor;
	Cursor = Queue->Bottom;
	if (Cursor == NULL) {
		return NULL;
	}
	do {
		if (strcmp(Cursor->Value->Name, Name) == 0) {
			return Cursor->Value;
		}
		Cursor = Cursor->Next;
	} while (Cursor != NULL);

	return NULL;
}

struct MenuRenderItem* MenuFindTop(struct MenuRenderQueue *Queue, char* Name) {
	struct MenuRenderQueueNode* Cursor;
	Cursor = Queue->Top;
	if (Cursor == NULL) {
		return NULL;
	}
	do {
		if (strcmp(Cursor->Value->Name, Name) == 0) {
			return Cursor->Value;
		}
		Cursor = Cursor->Last;
	} while (Cursor != NULL);
}
