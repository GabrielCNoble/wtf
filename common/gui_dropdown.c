#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gui_dropdown.h"
#include "gui_option_list.h"
#include "input.h"

/* from input.c */
extern int bm_mouse;

/* from r_main.c */
extern int r_window_width;
extern int r_window_height;

/* from gui.c */
extern char formated_str[];
extern widget_t *widgets;
extern widget_t *last_widget;
extern int gui_widget_unique_index;

 

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
	dropdown->widget.rendered_name = NULL;
	dropdown->widget.unique_index = gui_widget_unique_index++;
	dropdown->widget.process_callback = NULL;

	dropdown->bm_dropdown_flags = bm_flags;
		
	if(text)
		dropdown->dropdown_text = strdup(text);
	else 
		dropdown->dropdown_text = NULL;
		
	dropdown->rendered_text = NULL;	

	
	return dropdown;
}

dropdown_t *gui_AddDropdown(widget_t *widget, char *name, char *text, short x, short y, short w, short bm_flags, void (*dropdown_callback)(widget_t *))
{
	dropdown_t *dropdown = NULL;
	widget_t *wdgt;
	
	dropdown = gui_CreateDropdown(name, text, x, y, w, bm_flags, dropdown_callback);
	
	if(widget)
	{
		dropdown->widget.parent = widget;
		
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
	else
	{
		if(!widgets)
		{
			widgets = (widget_t *)dropdown;
		}
		else
		{
			last_widget->next = (widget_t *)dropdown;
			dropdown->widget.prev = last_widget;
		}
		last_widget = (widget_t *)dropdown;
		
		
		dropdown->widget.parent = NULL;
		
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
			gui_AddOptionList((widget_t *)dropdown, "dropdown option list", 0, 0, dropdown->widget.w * 2.0, 0, 32, dropdown->widget.widget_callback);
		}
		options = (option_list_t *)dropdown->widget.nestled;
		
		gui_AddOptionToList(options, name, text);
			
	}
}

void gui_UpdateDropdown(widget_t *widget)
{
	dropdown_t *dropdown = (dropdown_t *)widget;
	gui_var_t *var;
	
	dropdown->bm_dropdown_flags &= ~DROPDOWN_JUST_DROPPED;
				
	if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
	{
		
		/* make the dropdown that received the left click
		consume the flag, to avoid other stuff getting
		clicked on the same frame... */
		bm_mouse &= ~MOUSE_LEFT_BUTTON_JUST_CLICKED;
		
		if(dropdown->bm_dropdown_flags & DROPDOWN_DROPPED)
		{
			dropdown->bm_dropdown_flags &= ~DROPDOWN_DROPPED;
		}
		else
		{
			dropdown->bm_dropdown_flags |= DROPDOWN_DROPPED | DROPDOWN_JUST_DROPPED;			
		}
		
		if(dropdown->widget.widget_callback)
		{
			dropdown->widget.widget_callback(widget);
		}
		
	}
	
	if(widget->bm_flags & WIDGET_TRACK_VAR)
	{
		var = widget->var;
		
		if(var->bm_flags & GUI_VAR_VALUE_HAS_CHANGED)
		{
			switch(var->type)
			{
				case GUI_VAR_FLOAT:
				
				break;
				
				case GUI_VAR_INT:
				
				break;
				
				case GUI_VAR_STRING:
					
					if(!(char **)var->addr)
						break;
					
					if(!(*((char **)var->addr)))
						break;
					
					if(dropdown->dropdown_text)
						free(dropdown->dropdown_text);
					
					sprintf(formated_str, "%s", *((char **)var->addr));
					dropdown->dropdown_text = strdup(formated_str);
					
					widget->bm_flags |= WIDGET_RENDER_TEXT;
					
				break;
				
				default:
					
				break;
			}
		}
		
		
		
	}
	
}

void gui_PostUpdateDropdown(widget_t *widget)
{
	dropdown_t *dropdown = (dropdown_t *)widget;
	if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED || (input_GetKeyStatus(SDL_SCANCODE_ESCAPE) & KEY_JUST_PRESSED))
	{
		if(!(dropdown->bm_dropdown_flags & DROPDOWN_JUST_DROPPED))
		{
			dropdown->bm_dropdown_flags &= ~DROPDOWN_DROPPED;
		}
	}
}

void gui_UpdateDropdownBar(widget_t *bar)
{
	widget_t *w;
	widget_t *active;
	widget_bar_t *wbar;
	dropdown_t *dropdown;
	dropdown_t *active_dropdown;
	widget_t *r;
	
	w = bar->nestled;
	wbar = (widget_bar_t *)bar;
	
	if(wbar->active_widget)
	{
		if(wbar->active_widget->bm_flags & WIDGET_MOUSE_OVER)
		{
			return;
		}
	}
	
	while(w)
	{
		if(w->bm_flags & WIDGET_MOUSE_OVER)
		{
			
			if(wbar->active_widget)
			{
				if(wbar->active_widget != w)
				{
					dropdown = (dropdown_t *)w;
					active_dropdown = (dropdown_t *)wbar->active_widget;
					dropdown->bm_dropdown_flags |= active_dropdown->bm_dropdown_flags & DROPDOWN_DROPPED;
					active_dropdown->bm_dropdown_flags &= ~DROPDOWN_DROPPED;
					
					if(dropdown->bm_dropdown_flags & DROPDOWN_DROPPED)
					{
						dropdown->bm_dropdown_flags |= DROPDOWN_JUST_DROPPED;
					}
				}
			}
			
			wbar->active_widget = w;
			active_dropdown = (dropdown_t *)wbar->active_widget;
		}
		
		w = w->next;
	}
	
}


















