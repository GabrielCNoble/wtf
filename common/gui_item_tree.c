#include "gui.h"
#include "gui_item_tree.h"


extern widget_t *widgets;
extern widget_t *last_widget;


#ifdef __cplusplus
extern "C"
{
#endif


item_tree_t *gui_CreateItemTree(char *name, short x, short y, short w, short h, short flags, void (*tree_callback)(widget_t *widget))
{
	item_tree_t *tree = (item_tree_t *)gui_CreateWidget(name, x, y, w, h, WIDGET_ITEM_TREE);
	
	tree->x_offset = 0;
	tree->y_offset = 0;
}

item_tree_t *gui_AddTree(widget_t *parent, char *name, short x, short y, short w, short h, short flags, void (*tree_callback)(widget_t *widget))
{
	item_tree_t *tree = gui_CreateItemTree(name, x, y, w, h, flags, tree_callback);
	
	
	if(parent)
	{
		if(!parent->nestled)
		{
			parent->nestled = (widget_t *)tree;
		}
		else
		{
			parent->last_nestled->next = (widget_t *)tree;
			tree->widget.prev = parent->last_nestled;
		}
		
		parent->last_nestled = (widget_t *)tree;
	}
	else
	{
		if(!widgets)
		{
			widgets = (widget_t *)tree;
		}
		else
		{
			last_widget->next = (widget_t *)tree;
			tree->widget.prev = last_widget;
		}
		
		last_widget = (widget_t *)tree;
	}
	
	tree->widget.parent = parent;
	
	
	return tree;
}

void gui_UpdateItemTree(widget_t *tree)
{
	item_tree_t *item_tree;
	
	if(tree)
	{
		item_tree = (item_tree_t *)tree;
		
		
		
		
		
	}
}

void gui_PostUpdateItemTree(widget_t *tree)
{
	item_tree_t *item_tree;
	
	if(tree)
	{
		item_tree = (item_tree_t *)tree;
		
		
		
		
		
	}
}




#ifdef __cplusplus
}
#endif
