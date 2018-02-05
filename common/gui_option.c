#include "gui_option.h"


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
			if(widget->widget_callback)
				widget->widget_callback(widget);
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
