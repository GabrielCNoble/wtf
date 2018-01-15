#include <stdlib.h>
#include <string.h>

#include "gui_option_list.h"

extern widget_t *widgets;
extern widget_t *last_widget;

option_list_t *gui_CreateOptionList(char *name, short x, short y, short w, short bm_flags, void (*option_list_callback)(widget_t *))
{
	option_list_t *options;
	
	options = malloc(sizeof(option_list_t));
	options->widget.name = strdup(name);
	options->widget.next = NULL;
	options->widget.prev = NULL;
	options->widget.nestled = NULL;
	options->widget.last_nestled = NULL;
	options->widget.parent = NULL;
	options->widget.type = WIDGET_OPTION_LIST;
	options->widget.w = w;	
	options->widget.x = x;
	options->widget.y = y;
	options->option_count = 0;
	options->active_option_index = -1;
	options->active_option = NULL;
	options->widget.bm_flags = 0;
	options->bm_option_list_flags = OPTION_LIST_UPDATE_EXTENTS;
	options->widget.widget_callback = option_list_callback;
	
	if(!widgets)
	{
		widgets = (widget_t *)options;
	}
	else
	{
		options->widget.prev = last_widget;
		last_widget->next = (widget_t *)options;
	}
	
	last_widget = (widget_t *) options;

	return options;
	
}

void gui_AddOptionToList(option_list_t *option_list, char *name, char *text)
{
	option_t *option;
	
	option = malloc(sizeof(option_t));
	option->widget.name = strdup(name);
	option->widget.w = option_list->widget.w;
	option->widget.h = OPTION_HEIGHT / 2.0;
	option->widget.x = 0;
	option->widget.bm_flags = WIDGET_RENDER_TEXT;
	option->widget.next = NULL;
	option->widget.prev = NULL;
	option->widget.nestled = NULL;
	option->widget.last_nestled = NULL;
	option->widget.parent = (widget_t *)option_list;
	option->widget.type = WIDGET_OPTION;
	option->index = option_list->option_count;
	option->widget.widget_callback = option_list->widget.widget_callback;
	option->rendered_text = NULL;
		
	if(text)
		option->option_text = strdup(text);
	else
		option->option_text = NULL;
		
	
	if(!option_list->widget.nestled)
	{
		option_list->widget.nestled = (widget_t *)option;
	}	
	else
	{
		option->widget.prev = option_list->widget.last_nestled;
		option_list->widget.last_nestled->next = (widget_t *)option;
	}
	
	option_list->widget.last_nestled = (widget_t *)option;
	option_list->option_count++;
		
}

void gui_NestleOption(option_list_t *option_list, int option_index, char *name, char *text)
{
	option_t *option;
	widget_t *wdg;
	int i;
	int w;

	for(i = 0, wdg = option_list->widget.nestled; i < option_index && wdg; i++, wdg = wdg->next);
		
	gui_AddOption((dropdown_t *)wdg, name, text);
	
	w = wdg->w;
	wdg = wdg->nestled;
	
	wdg->x = w * 2.0;
	wdg->y += OPTION_HEIGHT;
}

void gui_UpdateOptionList(widget_t *widget)
{
	option_list_t *option_list = (option_list_t *)widget;
	int top_y;
	widget_t *r;
				
	if(option_list->bm_option_list_flags & OPTION_LIST_UPDATE_EXTENTS)
	{
		top_y = (OPTION_HEIGHT * option_list->option_count) * 0.5;
				
					
		widget->h = top_y;
		widget->y = -top_y;
		
		if(option_list->widget.parent)
		{
			/* if this option list is nestled within a option, 
			make it align correctly... */
			if(option_list->widget.parent->type == WIDGET_OPTION)
			{
				widget->y += OPTION_HEIGHT * 0.5;
			}
			else
			{
				widget->y -= OPTION_HEIGHT * 0.5;
			}
		}
					
		/* - OPTION_HEIGHT * 0.5;*/
					
					
		top_y -=  OPTION_HEIGHT * 0.5;
		r = widget->nestled;
					
		while(r)
		{
			r->y = top_y;
			top_y -= OPTION_HEIGHT;
			r = r->next;
		}
		
		if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
		{
			r = widget->parent;
			
			while(r)
			{
				r->bm_flags |= WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON;
				
				r = r->parent;
			}
			
		}
					
		option_list->bm_option_list_flags &= ~OPTION_LIST_UPDATE_EXTENTS;
	}
}






