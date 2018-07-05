#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gui_item_list.h"
#include "input.h"


extern widget_t *widgets;
extern widget_t *last_widget;

extern int gui_widget_unique_index;



extern int bm_mouse;

#ifdef __cplusplus
extern "C"
{
#endif

item_list_t *gui_CreateItemList(char *name, short x, short y, short w, short h, unsigned short flags, void (*item_list_callback)(widget_t *))
{
	item_list_t *list;
	
	
	list = malloc(sizeof(item_list_t ));
	
	//list->widget.name = strdup(name);
	list->widget.name = malloc(WIDGET_NAME_MAX_LEN);
	list->widget.name[0] = '\0';
	
	list->widget.x = x;
	list->widget.y = y;
	list->widget.w = w * 0.5;
	list->widget.h = h * 0.5;
	list->widget.bm_flags = WIDGET_JUST_CREATED;
	list->widget.next = NULL;
	list->widget.prev = NULL;
	list->widget.parent = NULL;
	list->widget.nestled = NULL;
	list->widget.last_nestled = NULL;
	list->widget.rendered_name = NULL;
	list->widget.type = WIDGET_ITEM_LIST;
	list->widget.var = NULL;
	list->widget.widget_callback = item_list_callback;
	list->widget.unique_index = gui_widget_unique_index++;
	list->widget.process_callback = NULL;
	
	list->active_item_index = 0xffff;
	list->selected_item_index = 0xffff;
	list->active_item = NULL;
	list->selected_item = NULL;
	list->flags = flags | ITEM_LIST_UPDATE;
	list->type = WIDGET_NONE;
	list->item_count = 0;
	list->first_unused = NULL;
	list->item_w = 0;
	list->item_h = 0;
	list->x_offset = 0;
	list->y_offset = 0;
	
	return list;
	
}

item_list_t *gui_AddItemList(widget_t *parent, char *name, short x, short y, short w, short h, unsigned short flags, void (*item_list_callback)(widget_t *))
{
	item_list_t *list;
	list = gui_CreateItemList(name, x, y, w, h, flags, item_list_callback);
	
	if(parent)
	{
		if(!parent->nestled)
		{
			parent->nestled = (widget_t *)list;
		}
		else
		{
			parent->last_nestled->next = (widget_t *)list;
			list->widget.prev = parent->last_nestled;
		}
		
		parent->last_nestled = (widget_t *)list;
		
		list->widget.parent = parent;
	}
	else
	{
		if(!widgets)
		{
			widgets = (widget_t *)list;
		}
		else
		{
			last_widget->next = (widget_t *)list;
			list->widget.prev = last_widget;
		}
		
		
		last_widget = (widget_t *)list;
	}
	
	return list;
}

void gui_ClearListType(item_list_t *list)
{
	widget_t *r;
	widget_t *n;
	
	if(list)
	{
		list->type = WIDGET_NONE;
		
		r = list->widget.nestled;
		
		while(r)
		{
			n = r->next;
			gui_DestroyWidget(r);
			r = n;
		}
		
		list->widget.nestled = NULL;
		list->widget.last_nestled = NULL;
		
		r = list->first_unused;
		
		while(r)
		{
			n = r->next;
			gui_DestroyWidget(r);
			r = n;
		}	
		
		list->first_unused = NULL;
		list->item_count = 0;
		list->selected_item_index = 0xffff;
		list->active_item_index = 0xffff;
		list->active_item = NULL;
		list->selected_item = NULL;
	}
}



widget_t *gui_GetItemAt(item_list_t *list, int item_index)
{
	int i;
	widget_t *r;
	
	if(list)
	{
		for(i = 0, r = list->widget.nestled; i < item_index && r; i++, r = r->next);
		return r;
	}
	
	return NULL;
}


widget_t *gui_AddItemToList(widget_t *item, item_list_t *list)
{

	widget_t *r = NULL;
		
	if(list)
	{
		if(item)
		{
			if(list->type == WIDGET_NONE || list->type == item->type)
			{
				/* first item added to the list define it's type and
				the dimensions of the next items... */
				if(list->type == WIDGET_NONE)
				{
					list->item_w = item->w;
					list->item_h = item->h;
				}
				else
				{
					item->w = list->item_w;
					item->h = list->item_h;
				}
				
				list->type = item->type;
				
				if(!list->widget.nestled)
				{
					list->widget.nestled = item;
				}
				else
				{
					list->widget.last_nestled->next = item;
					item->prev = list->widget.last_nestled;
				}
					
				list->widget.last_nestled = item;
					
				item->parent = (widget_t *)list;
				item->bm_flags |= WIDGET_NOT_AS_TOP;
				
				list->flags |= ITEM_LIST_UPDATE;
				
				r = item;
				
				
				list->item_count++;
			}
		}
		else
		{
			/* caller didn't pass a valid widget pointer, so
			check if this list already has a type... */
			if(list->type != WIDGET_NONE)
			{
				/* see if there's unused widgets... */
				if(list->first_unused)
				{
					/* yup, so just return a pointer to it... */
					r = list->first_unused;
					list->first_unused = list->first_unused->next;
					
					if(!list->widget.nestled)
					{
						list->widget.nestled = r;
					}
					else
					{
						list->widget.last_nestled->next = r;
						r->prev = list->widget.last_nestled;
					}
					
					
					list->widget.last_nestled = r;
	
					r->next = NULL;
					r->bm_flags |= WIDGET_NOT_AS_TOP;
					
					list->item_count++;
					
					/* Don't clear the nestled widgets.
					All this fuss is to enable reusing
					of already alloc'd widgets. */
					
					//r->nestled = NULL;
					//r->last_nestled = NULL;
					//r->name = NULL;
					//r->rendered_name = NULL;
					
				}		
			}
		}
		list->flags |= ITEM_LIST_UPDATE;	
	}
	
	return r;
}

void gui_RemoveItemFromList(int item_index, item_list_t *list)
{
	widget_t *w;
	
	w = gui_GetItemAt(list, item_index);
	
	if(w)
	{
		if(w == list->widget.nestled)
		{
			list->widget.nestled = w->next;
		}
		else
		{
			if(w == list->widget.last_nestled)
			{
				list->widget.last_nestled = list->widget.last_nestled->prev;
			}
			
			w->prev->next = w->next;
		}
		
		if(w->next)
		{
			w->next->prev = w->prev;
		}
		
		
		w->next = list->first_unused;
		list->first_unused = w;
		
		list->item_count--;
		
		list->flags |= ITEM_LIST_UPDATE;
	}
	
}

void gui_RemoveAllItems(item_list_t *list)
{
	widget_t *w;
	widget_t *r;
	widget_t *p;
	
	if(list)
	{
		w = list->widget.nestled;
		
		if(w)
		{
			r = w;
			
			while(r->next)r = r->next;
			
			
			r->next = list->first_unused;
			list->first_unused = w;
			
			
			list->widget.nestled = NULL;
			list->widget.last_nestled = NULL;
			list->active_item_index = 0xffff;
			list->selected_item_index = 0xffff;
			list->active_item = NULL;
			list->selected_item = NULL;
			list->item_count = 0;
			list->x_offset = 0;
			list->y_offset = 0;
		}
		
	}
}

void gui_UpdateListExtents(item_list_t *list)
{
	widget_t *w;
	float sx;
	float ex;
	//float dx;
	float sy;
	float ey;
	//float dy;
	float x;
	float y;
	
	if(list)
	{
		sx = -list->widget.w + list->item_w + list->x_offset;
		sy = list->widget.h - list->item_h + list->y_offset;
				
		w = list->widget.nestled;
		
		if(list->flags & ITEM_LIST_HORIZONTAL_ORDER)
		{
			y = sy;
			while(w)
			{
				x = sx;
				while(w && x + list->item_w <= list->widget.w)
				{
					w->x = x;
					w->y = y;
					x += list->item_w * 2.0;
					w = w->next;
				}
				y -= list->item_h * 2.0;
			}
			
		}
		else
		{
			y = sy;
			while(w)
			{
				w->y = y;
				y -= list->item_h * 2.0;
				w = w->next;
			}
		}
		
		
		list->flags &= ~ITEM_LIST_UPDATE;
	}
}

void gui_UpdateItemList(item_list_t *list)
{
	
	short items_per_row;
	short item_rows;
	
	short total_h;
	int i;
	
	widget_t *widget;
	
	if(list)
	{
		
		if(list->widget.bm_flags & WIDGET_JUST_RECEIVED_MOUSE_WHEEL_DOWN)
		{
			
			if(list->flags & ITEM_LIST_HORIZONTAL_ORDER)
			{
				items_per_row = list->widget.w / list->item_w;
				item_rows = 1 + (list->item_count / items_per_row);
			}	
			else
			{
				item_rows = list->item_count;
			}
			
			total_h = item_rows * list->item_h * 2.0;
			
			if(total_h > list->widget.h * 2.0)
			{
				if(list->widget.h * 2.0 + list->y_offset < total_h)
				{
					list->y_offset += list->item_h * 2.0;
					list->flags |= ITEM_LIST_UPDATE;
				}
				
				if(list->y_offset + list->widget.h * 2.0 > total_h)
				{
					list->y_offset = total_h - list->widget.h * 2.0;
				}
				
				
			}
		
		}
		else if(list->widget.bm_flags & WIDGET_JUST_RECEIVED_MOUSE_WHEEL_UP)
		{
			list->y_offset -= list->item_h * 2;
			
			if(list->y_offset < 0)
			{
				list->y_offset = 0;
			}
			
			list->flags |= ITEM_LIST_UPDATE;
			
		}
		
		if(list->flags & ITEM_LIST_UPDATE)
		{
			gui_UpdateListExtents(list);
		}
			
		list->active_item_index = 0xffff;
		list->active_item = NULL;
		
		widget = list->widget.nestled;
		
		i = 0;
		
		
		while(widget)
		{
			if(widget->bm_flags & WIDGET_MOUSE_OVER)
			{
				list->active_item_index = i;
				list->active_item = widget;
			}
					
			widget = widget->next;
			i++;
		}
		
		if(list->flags & ITEM_LIST_DOUBLE_CLICK_SELECTION)
		{
			if((bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED) || (input_GetKeyStatus(SDL_SCANCODE_ESCAPE) & KEY_JUST_PRESSED))
			{
				if(list->widget.bm_flags & WIDGET_MOUSE_OVER)
				{
					list->selected_item = NULL;
					list->selected_item_index = 0xffff;
				}
				
			}
			
			if(list->widget.bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
			{
				list->selected_item = list->active_item;
				list->selected_item_index = list->active_item_index;
			}
			
			if(list->widget.bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_DOUBLE_CLICK)
			{
				if(list->widget.widget_callback)
					list->widget.widget_callback((widget_t *)list);
			}
			
		}
		else
		{
			if(list->widget.bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
			{
				if(list->widget.widget_callback)
					list->widget.widget_callback((widget_t *)list);
			}
		}
		
		
		
	}
}

void gui_PostUpdateItemList(item_list_t *list)
{
	
}



#ifdef __cplusplus
}
#endif












