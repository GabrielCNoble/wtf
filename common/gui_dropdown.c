#include <stdlib.h>
#include <string.h>

#include "gui_dropdown.h"

dropdown_t *gui_CreateDropdown(char *name, char *text, short x, short y, short w, short bm_flags, void (*dropdown_callback)(widget_t *widget))
{
	dropdown_t *dropdown = NULL;

	dropdown = malloc(sizeof(dropdown_t));
		
	dropdown->widget.bm_flags = WIDGET_RENDER_TEXT;
	dropdown->widget.x = x;
	dropdown->widget.y = y;
	dropdown->widget.h = DROPDOWN_HEIGHT / 2.0;
		
	if(w < WIDGET_MIN_SIZE)
		w = WIDGET_MIN_SIZE;
		
	dropdown->widget.w = w / 2;
	dropdown->widget.type = WIDGET_DROPDOWN;
	dropdown->widget.name = strdup(name);
	dropdown->widget.parent = NULL;
	dropdown->widget.next = NULL;
	dropdown->widget.prev = NULL;
	dropdown->widget.nestled = NULL;
	dropdown->widget.last_nestled = NULL;
	dropdown->widget.widget_callback = dropdown_callback;

	dropdown->bm_dropdown_flags = bm_flags;
		
	if(text)
		dropdown->dropdown_text = strdup(text);
	else 
		dropdown->dropdown_text = NULL;
		
	dropdown->rendered_text = NULL;	

	
	return dropdown;
}

dropdown_t *gui_AddDropdown(widget_t *widget, char *name, char *text, short x, short y, short w, short bm_flags, void (*dropdown_callback)(widget_t *widget))
{
	dropdown_t *dropdown = NULL;
	widget_t *wdgt;
	if(widget)
	{
		dropdown = gui_CreateDropdown(name, text, x, y, w, bm_flags, dropdown_callback);
		
		
		if(!widget->nestled)
		{
			widget->nestled = (widget_t *)dropdown;
			widget->last_nestled = (widget_t *)dropdown;
		}
		else
		{
			widget->last_nestled->next = (widget_t *)dropdown;
			dropdown->widget.prev = widget->last_nestled;
			widget->last_nestled = (widget_t *)dropdown;
		}
	}
	
	return dropdown;
}

void gui_AddOption(dropdown_t *dropdown, char *name, char *text)
{
	option_list_t *options;
	option_t *option;
	
	short x;
	short y;
	if(dropdown)
	{
		if(!dropdown->widget.nestled)
		{
			options = malloc(sizeof(option_list_t));
			options->widget.next = NULL;
			options->widget.prev = NULL;
			options->widget.nestled = NULL;
			options->widget.last_nestled = NULL;
			options->widget.parent = (widget_t *)dropdown;
			options->widget.type = WIDGET_OPTION_LIST;
			options->widget.w = dropdown->widget.w;	
			options->widget.x = 0;
			options->option_count = 0;
			options->active_option_index = -1;
			options->active_option = NULL;
			options->widget.bm_flags = WIDGET_INVISIBLE;
			options->widget.w = options->widget.w;
			options->bm_option_list_flags = OPTION_LIST_UPDATE_EXTENTS;
			options->widget.widget_callback = dropdown->widget.widget_callback;
			
			dropdown->widget.nestled = (widget_t *)options;
			dropdown->widget.last_nestled = (widget_t *)options;
		}
		
		options = (option_list_t *)dropdown->widget.nestled;
		
		
		option = malloc(sizeof(option_t));
		option->widget.name = strdup(name);
		option->widget.w = options->widget.w;
		option->widget.h = OPTION_HEIGHT / 2.0;
		option->widget.x = 0;
		option->widget.bm_flags = WIDGET_RENDER_TEXT;
		option->widget.next = NULL;
		option->widget.prev = NULL;
		option->widget.nestled = NULL;
		option->widget.last_nestled = NULL;
		option->widget.parent = (widget_t *)options;
		option->widget.type = WIDGET_OPTION;
		option->index = options->option_count;
		option->widget.widget_callback = options->widget.widget_callback;
		option->rendered_text = NULL;
		
		if(text)
			option->option_text = strdup(text);
		else
			option->option_text = NULL;
		
		if(!options->widget.nestled)
		{
			options->widget.nestled = (widget_t *)option;
		}
		else
		{
			options->widget.last_nestled->next = (widget_t *)option;
			option->widget.prev = options->widget.last_nestled;
		}
		
		options->widget.last_nestled = (widget_t *)option;
		options->bm_option_list_flags |= OPTION_LIST_UPDATE_EXTENTS;
		options->option_count++;
	}
}

void gui_UpdateDropDown(widget_t *widget)
{
	dropdown_t *dropdown = (dropdown_t *)widget;
				
	if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
	{
		if(dropdown->bm_dropdown_flags & DROPDOWN_DROPPED)
		{
			dropdown->bm_dropdown_flags &= ~DROPDOWN_DROPPED;
					
			if(widget->nestled)
			{
				widget->nestled->bm_flags |= WIDGET_INVISIBLE;
			}
		}
		else
		{
			dropdown->bm_dropdown_flags |= DROPDOWN_DROPPED;
			
			if(widget->nestled)
			{
				widget->nestled->bm_flags &= ~WIDGET_INVISIBLE;
			}
		}
	}
}





