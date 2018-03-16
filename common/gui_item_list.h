#ifndef GUI_ITEM_LIST_H
#define GUI_ITEM_LIST_H

#include "gui_common.h"

item_list_t *gui_CreateItemList(char *name, short x, short y, short w, short h, unsigned short flags, void (*item_list_callback)(widget_t *));

item_list_t *gui_AddItemList(widget_t *parent, char *name, short x, short y, short w, short h, unsigned short flags, void (*item_list_callback)(widget_t *));

void gui_ClearListType(item_list_t *list);

widget_t *gui_GetItemAt(item_list_t *list, int item_index);

widget_t *gui_AddItemToList(widget_t *item, item_list_t *list);

void gui_RemoveItemFromList(int item_index, item_list_t *list);

void gui_RemoveAllItems(item_list_t *list);

void gui_UpdateListExtents(item_list_t *list);

void gui_UpdateItemList(item_list_t *list);

void gui_PostUpdateItemList(item_list_t *list);



#endif
