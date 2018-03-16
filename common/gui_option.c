#include "gui_option.h"

#include "input.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern int gui_option_unique_index;
extern int gui_widget_unique_index;


extern int bm_mouse;

option_t *gui_CreateOption(char *name, char *text)
{
	
	option_t *option;
	int name_len;
 
	option = malloc(sizeof(option_t));
	option->widget.name = malloc(WIDGET_NAME_MAX_LEN);
	
	name_len = strlen(name) + 1;
	
	if(name_len > WIDGET_NAME_MAX_LEN) name_len = WIDGET_NAME_MAX_LEN; 
	
	memcpy(option->widget.name, name, name_len - 1);
	option->widget.name[name_len - 1] = '\0';
	
	option->widget.h = OPTION_HEIGHT / 2.0;
	option->widget.x = 0;
	option->widget.y = 0;
	option->widget.bm_flags = WIDGET_RENDER_TEXT | WIDGET_JUST_CREATED | WIDGET_NOT_AS_TOP;
	option->widget.next = NULL;
	option->widget.prev = NULL;
	option->widget.nestled = NULL;
	option->widget.last_nestled = NULL;
	option->widget.parent = NULL;
	option->widget.type = WIDGET_OPTION;
	option->index = -1;
	option->widget.widget_callback = NULL;
	option->rendered_text = NULL;
	option->bm_option_flags = 0;
	option->unique_index = gui_option_unique_index++;
	option->widget.unique_index = gui_widget_unique_index++;
	option->widget.process_callback = NULL;
		
	if(text)
		option->option_text = strdup(text);
	else
		option->option_text = NULL;
	
	
	return option;	
		
}

void gui_UpdateOption(widget_t *widget)
{
	option_t *option = (option_t *)widget;
	option_list_t *option_list = (option_list_t *)widget->parent;
				
	if(widget->bm_flags & WIDGET_MOUSE_OVER)
	{
		
		if(option_list->active_option_index != option->index)
		{
			/* if this option has the mouse over it, make
			it the active option of this option list... */
			option_list->active_option_index = option->index;
			option_list->active_option = (struct option_t *)option;	
			//printf("wow!\n");
			
			gui_SetVisible(widget);	
		}
							
		if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
		{
			//bm_mouse &= ~MOUSE_LEFT_BUTTON_JUST_CLICKED;
			
			if(option_list->bm_option_list_flags & OPTION_LIST_DOUBLE_CLICK_SELECTION)
			{
				if(!(option->bm_option_flags & OPTION_INVALID))
				{
					option_list->selected_option_index = option_list->active_option_index;
					option_list->selected_option = (struct option_t *)option;
				}
					
			}
			else
			{
				if(!(option->bm_option_flags & OPTION_INVALID))
					if(widget->widget_callback)
						widget->widget_callback(widget);		
			}
			
		}
		
		if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_DOUBLE_CLICK)
		{
			if(option_list->bm_option_list_flags & OPTION_LIST_DOUBLE_CLICK_SELECTION)
			{
				bm_mouse &= ~MOUSE_LEFT_BUTTON_DOUBLE_CLICKED;	
				
				if(!(option->bm_option_flags & OPTION_INVALID))
						if(widget->widget_callback)
							widget->widget_callback(widget);	
			}
		}
					
	}
	else
	{
		if((option_t *)option_list->active_option == option)
		{
			/* keep the option as active when the mouse
			is away only if this option has any nestled options... */
			if(!option->widget.nestled)
			{
				option_list->active_option_index = -1;
				option_list->active_option = NULL;
			}
		}
			
	}
}
