#include "gui_option_list.h"


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
					
		/* - OPTION_HEIGHT * 0.5;*/
					
					
		top_y -=  OPTION_HEIGHT * 0.5;
		r = widget->nestled;
					
		while(r)
		{
			r->y = top_y;
			top_y -= OPTION_HEIGHT;
			r = r->next;
		}
					
		option_list->bm_option_list_flags &= ~OPTION_LIST_UPDATE_EXTENTS;
	}
}
