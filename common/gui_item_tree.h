#ifndef GUI_ITEM_TREE_H
#define GUI_ITEM_TREE_H

#include "gui_common.h"

#ifdef __cplusplus
extern "C"
{
#endif



item_tree_t *gui_CreateItemTree(char *name, short x, short y, short w, short h, short flags, void (*item_tree_callback)(widget_t *widget));

item_tree_t *gui_AddItemTree(widget_t *parent, char *name, short x, short y, short w, short h, short flags, void (*item_tree_callback)(widget_t *widget));

void gui_UpdateItemTree(widget_t *tree);

void gui_PostUpdateItemTree(widget_t *tree);




#ifdef __cplusplus
}
#endif






#endif
