#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gui_option_list.h"
#include "input.h"

/* from gui.c */
extern widget_t *widgets;
extern widget_t *last_widget;


extern int r_window_width;
extern int r_window_height;


int gui_option_unique_index = 0;

/* from input.c */
extern int bm_mouse;
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
	options->widget.bm_flags = WIDGET_IGNORE_EDGE_CLIPPING | WIDGET_JUST_CREATED;
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
	option->widget.bm_flags = WIDGET_RENDER_TEXT | WIDGET_IGNORE_EDGE_CLIPPING | WIDGET_JUST_CREATED; 
	option->widget.next = NULL;
	option->widget.prev = NULL;
	option->widget.nestled = NULL;
	option->widget.last_nestled = NULL;
	option->widget.parent = (widget_t *)option_list;
	option->widget.type = WIDGET_OPTION;
	option->index = option_list->option_count;
	option->unique_index = gui_option_unique_index++;
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
	
	option_list->bm_option_list_flags |= OPTION_LIST_UPDATE_EXTENTS;
	
	option_list->widget.last_nestled = (widget_t *)option;
	option_list->option_count++;
		
}

/*void gui_NestleOption(option_list_t *option_list, int option_index, char *name, char *text)
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
}*/

option_list_t *gui_NestleOptionList(option_list_t *option_list, int option_index, char *name)
{
	int i;
	widget_t *wdg;
	option_t *option;
	option_list_t *options = NULL;
	
	for(i = 0, wdg = option_list->widget.nestled; i < option_index && wdg; i++, wdg = wdg->next);
	
	if(wdg)
	{
		/* only nestle this option list if the option doesn't has any already nestled option list... */
		if(!wdg->nestled)
		{	
			options = malloc(sizeof(option_list_t));
			options->widget.name = strdup(name);
			options->widget.next = NULL;
			options->widget.prev = NULL;
			options->widget.nestled = NULL;
			options->widget.last_nestled = NULL;
			options->widget.parent = NULL;
			options->widget.type = WIDGET_OPTION_LIST;
			options->widget.w = wdg->w;	
			options->widget.x = wdg->w * 2.0;
			options->widget.y = 0;
			options->option_count = 0;
			options->active_option_index = -1;
			options->active_option = NULL;
			options->widget.bm_flags = WIDGET_IGNORE_EDGE_CLIPPING | WIDGET_JUST_CREATED;
			options->bm_option_list_flags = OPTION_LIST_UPDATE_EXTENTS;
			options->widget.widget_callback = wdg->widget_callback;
			
			wdg->nestled = (widget_t *)options;
			wdg->last_nestled = (widget_t *)options;
			options->widget.parent = wdg;
		}
		
	}
		
	return options;
}

option_list_t *gui_GetNestledOptionList(option_list_t *option_list, int option_index)
{
	int i;
	widget_t *r;
	
	for(r = option_list->widget.nestled, i = 0; r && i < option_index; r = r->next, i++);
	return (option_list_t *)r;
}

int gui_GetOptionUniqueIndex(option_list_t *option_list, int option_index)
{
	int i;
	widget_t *r;
	option_t *option;
	
	for(r = option_list->widget.nestled, i = 0; r && i < option_index; r = r->next, i++);
	
	if(r)
	{
		option = (option_t *)r;
		return option->unique_index;
	}
	
	return -1;
	
}

void gui_UpdateOptionList(widget_t *widget)
{
	option_list_t *option_list = (option_list_t *)widget;
	int top_y;
	widget_t *r;
	widget_t *parent;
	
	short x;
	short y;
				
	if(option_list->bm_option_list_flags & OPTION_LIST_UPDATE_EXTENTS)
	{
		top_y = (OPTION_HEIGHT * option_list->option_count) * 0.5;
				
					
		widget->h = top_y;
		widget->y = -top_y;
		
		if(widget->parent)
		{
			/* if this option list is nestled within a option, 
			make it align correctly... */
			if(widget->parent->type == WIDGET_OPTION)
			{
				widget->y += OPTION_HEIGHT * 0.5;
			}
			else
			{
				if(widget->parent->type == WIDGET_NONE)
				{
					
				}
				/* dropdown... */
				else
				{
					widget->y -= OPTION_HEIGHT * 0.5;
				}
				
			}
		}
					
		/* - OPTION_HEIGHT * 0.5;*/
					
					
		top_y -= OPTION_HEIGHT * 0.5;
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
	
	if(widget->parent)
	{
		parent = widget->parent;
		
		if(parent->type == WIDGET_OPTION)
		{
			
			gui_GetAbsolutePosition(parent, &x, &y);
		
			if(x + parent->w + widget->w > r_window_width * 0.5) /*|| x + widget->w > r_window_width * 0.5)*/
			{
				widget->x = -parent->w - widget->w;
			}
			else
			{
				widget->x = +parent->w + widget->w;
			}
			
		}
		else if(parent->type == WIDGET_DROPDOWN)
		{
			gui_GetAbsolutePosition(parent, &x, &y);
			
			if(y + parent->h + widget->h > r_window_height * 0.5)
			{
				widget->y = -parent->h - widget->h;
			}
			else
			{
				widget->y = parent->h + widget->h;
			}
			
		}
		
		
	}
	
}


void gui_PostUpdateOptionList(widget_t *widget)
{
	//if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
	if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED || (input_GetKeyStatus(SDL_SCANCODE_ESCAPE) & KEY_JUST_PRESSED))
	{
		/* pop-up menu... */
		if(!widget->parent)
		{
			gui_SetInvisible(widget);
		}
		else
		{
		
		}
	}
}





