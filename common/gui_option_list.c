#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gui_option_list.h"
#include "gui.h"
#include "input.h"

/* from gui.c */
extern widget_t *widgets;
extern widget_t *last_widget;


extern int r_window_width;
extern int r_window_height;


int gui_option_unique_index = 0;
extern int gui_widget_unique_index;
 
/* from input.c */
extern int bm_mouse;

#ifdef __cplusplus
extern "C"
{
#endif

option_list_t *gui_CreateOptionList(char *name, short x, short y, short w, short bm_flags, short max_visible_options, void (*option_list_callback)(widget_t *))
{
	option_list_t *options;
	
	/*options = malloc(sizeof(option_list_t));
	options->widget.name = strdup(name);
	options->widget.next = NULL;
	options->widget.prev = NULL;
	options->widget.nestled = NULL;
	options->widget.last_nestled = NULL;
	options->widget.parent = NULL;
	options->widget.type = WIDGET_OPTION_LIST;
	options->widget.w = w * 0.5;	
	options->widget.x = x;
	options->widget.y = y;
	options->widget.process_callback = NULL;*/
	
	options = (option_list_t *) gui_CreateWidget(name, x, y, w, OPTION_HEIGHT, WIDGET_OPTION_LIST);
	
	options->widget.widget_callback = option_list_callback;
	options->widget.rendered_name = NULL;
		
	if(max_visible_options < OPTION_LIST_MINIMUM_MAXIMUM_VISIBLE_OPTIONS)
		max_visible_options = OPTION_LIST_MINIMUM_MAXIMUM_VISIBLE_OPTIONS;
	
	options->max_visible_options = max_visible_options;
	options->option_count = 0;
	options->active_option_index = -1;
	options->selected_option_index = -1;
	options->active_option = NULL;
	options->selected_option = NULL;
	options->widget.bm_flags = WIDGET_IGNORE_EDGE_CLIPPING | WIDGET_JUST_CREATED;
	options->bm_option_list_flags = bm_flags | OPTION_LIST_UPDATE_EXTENTS;
	
	options->first_unused = NULL;	
	
	//options->widget.unique_index = gui_widget_unique_index++;
	options->y_offset = 0;
	
	options->first_x = x;
	options->first_y = y;
	
	return options;
	
}

option_list_t *gui_AddOptionList(widget_t *widget, char *name, short x, short y, short w, short bm_flags, short max_visible_options, void (*option_list_callback)(widget_t *))
{
	option_list_t *options;
	widget_t *parent;
	
	options = gui_CreateOptionList(name, x, y, w, bm_flags, max_visible_options, option_list_callback);
	
	if(!widget)
	{
		if(!widgets)
		{
			widgets = (widget_t *)options;
		}
		else
		{
			last_widget->next = (widget_t *)options;
			options->widget.prev = last_widget;
		}
		
		last_widget = (widget_t *)options;
	}
	else
	{
		if(!widget->nestled)
		{
			widget->nestled = (widget_t *)options;
		}
		else
		{
			widget->last_nestled->next = (widget_t *)options;
			options->widget.prev = widget->last_nestled;
		}
		
		widget->last_nestled = (widget_t *)options;
		options->widget.parent = widget;
	}
	
	return options;
	
}

option_t *gui_AddOptionToList(option_list_t *option_list, char *name, char *text)
{
	option_t *option = NULL;
	int name_len;
	
	if(option_list->first_unused)
	{
		option = (option_t *)option_list->first_unused;
		option_list->first_unused = option_list->first_unused->next;
		option_list->y_offset = 0;
		//option_list->first_unused->prev = NULL;
		
		option->widget.next = NULL;
		option->widget.prev = NULL;
		option->widget.nestled = NULL;
		option->widget.last_nestled = NULL;
		option->widget.bm_flags |= WIDGET_RENDER_TEXT;
		
		name_len = strlen(name) + 1;
	
		if(name_len > WIDGET_NAME_MAX_LEN) name_len = WIDGET_NAME_MAX_LEN; 
		
		memcpy(option->widget.name, name, name_len - 1);
		option->widget.name[name_len - 1] = '\0';
		
		if(text)
			option->option_text = strdup(text);
		
	}
	else
	{
		option = gui_CreateOption(name, text);	
	}

	option->widget.w = option_list->widget.w;
	option->widget.parent = (widget_t *)option_list;
	option->index = option_list->option_count;
	option->widget.widget_callback = option_list->widget.widget_callback;
	option->widget.process_callback = option_list->widget.process_callback;
		
	if(!option_list->widget.nestled)
	{
		option_list->widget.nestled = (widget_t *)option;
		//printf("first option >>>>> %s\n", option->widget.name);
	}	
	else
	{
		option->widget.prev = option_list->widget.last_nestled;
		option_list->widget.last_nestled->next = (widget_t *)option;
	}
	
	//printf("option [%s] has parent [%s]   [%x]\n", option->widget.name, option->widget.parent->name, option_list->widget.nestled);
	
	option_list->bm_option_list_flags |= OPTION_LIST_UPDATE_EXTENTS;
	
	option_list->widget.last_nestled = (widget_t *)option;
	option_list->option_count++;
	
	return option;		
}

void gui_RemoveOptionFromList(option_list_t *option_list, int option_index)
{
	option_t *option;
	widget_t *widget;
	widget_t *nestled_widget;
	
	option = gui_GetOptionAt(option_list, option_index);
	
	if(option)
	{
		if(option->widget.nestled)
		{
			/* don't deallocate the memory for this option,
			but get rid of anything bellow it... */
			gui_DestroyWidget(option->widget.nestled);
		}
		
		widget = (widget_t *)option_list;
		nestled_widget = (widget_t *)option;
		
		/* unlink this option from the nestled list of
		the option list... */
		if((widget_t *)option == widget->nestled)
		{
			widget->nestled = widget->nestled->next;
			widget->nestled->prev = NULL;
		}
		else
		{
			nestled_widget->prev->next = nestled_widget->next;
		}
		
		if(nestled_widget->next)
		{
			nestled_widget->next->prev = nestled_widget->prev;
		}
		else
		{
			option_list->widget.last_nestled = option_list->widget.last_nestled->prev;
		}
		
		if((struct option_t *)option == option_list->active_option)
		{
			option_list->active_option = NULL;
		}
		
		if((struct option_t *)option == option_list->selected_option)
		{
			option_list->selected_option = NULL;
		}
		
		
		//free(option->widget.name);
		if(option->option_text)
		{
			free(option->option_text);
			option->option_text = NULL;
		}
			
		
		if(option->widget.rendered_name)
			SDL_FreeSurface(option->widget.rendered_name);
		
		
		/* ... and put it in an unused options list... */
		nestled_widget->next = option_list->first_unused;
		option_list->first_unused = nestled_widget;
				
		option_list->option_count--;
	}
}

void gui_RemoveAllOptions(option_list_t *option_list)
{
	widget_t *first;
	widget_t *last;
	widget_t *prev;
	option_t *option;
	
	first = option_list->widget.nestled;
	
	
	if(first)
	{
		last = first;
		prev = NULL;
		option = (option_t *)last;
		
		while(last)
		{
			option = (option_t *)last;
			if(option->option_text)
			{
				free(option->option_text);
			}
			option->option_text = NULL;
			prev = last;
			last = last->next;
		}
		last = prev;

		if(last)
		{
			last->next = option_list->first_unused;
		}
		else
		{
			first->next = option_list->first_unused;
		}
		
		option_list->first_unused = first;
		
		option_list->widget.nestled = NULL;
		option_list->widget.last_nestled = NULL;
		
		option_list->option_count = 0;
		option_list->active_option = NULL;
		option_list->selected_option = NULL;
		option_list->active_option_index = -1;
		option_list->selected_option_index = -1;
	}
	
	
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
			
			/*options = malloc(sizeof(option_list_t));
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
			options->widget.rendered_name = NULL;
			options->option_count = 0;
			options->active_option_index = -1;
			options->active_option = NULL;
			options->widget.bm_flags = WIDGET_IGNORE_EDGE_CLIPPING | WIDGET_JUST_CREATED;
			options->bm_option_list_flags = OPTION_LIST_UPDATE_EXTENTS;
			options->widget.widget_callback = wdg->widget_callback;
			options->first_unused = NULL;*/
			
			options = gui_CreateOptionList(name, wdg->w * 2.0, 0, wdg->w * 2.0, 0, 8, wdg->widget_callback);
			
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

option_t *gui_GetOptionAt(option_list_t *option_list, int option_index)
{
	int c;
	widget_t *w;
	if(option_index < 0)
		return NULL;
	
	for(c = 0, w = option_list->widget.nestled; c < option_index && w; c++, w = w->next);
	
	return (option_t *)w;
}

void gui_InvalidOption(option_list_t *option_list, int option_index)
{
	option_t *option;
	if(option_list)
	{
		option = gui_GetOptionAt(option_list, option_index);
		
		if(option)
		{
			option->bm_option_flags |= OPTION_INVALID;
			option->widget.bm_flags |= WIDGET_RENDER_TEXT;
		}
		
	}
}

void gui_ValidOption(option_list_t *option_list, int option_index)
{
	
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
	int option_w;
	int option_x;
	widget_t *r;
	widget_t *parent;
	dropdown_t *dropdown;
	int option_count = 0;
	short x;
	short y;
	
	top_y = (OPTION_HEIGHT * option_list->option_count) * 0.5;
	
	
	if(option_list->bm_option_list_flags & OPTION_LIST_DONT_RECEIVE_MOUSE)
	{
		widget->bm_flags &= ~(WIDGET_JUST_RECEIVED_MOUSE_WHEEL_DOWN | WIDGET_JUST_RECEIVED_MOUSE_WHEEL_UP |
							  WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON | WIDGET_JUST_RECEIVED_RIGHT_MOUSE_BUTTON |
							  WIDGET_JUST_RECEIVED_LEFT_MOUSE_DOUBLE_CLICK | WIDGET_MOUSE_OVER);
	}
	
	if(widget->bm_flags & WIDGET_JUST_RECEIVED_MOUSE_WHEEL_DOWN)
	{
		//if(option_list->y_offset + option_list->widget.h)
		if(option_list->y_offset * 0.5 + option_list->max_visible_options * OPTION_HEIGHT * 0.5 < top_y)
		{
			option_list->y_offset += (unsigned short)OPTION_HEIGHT;
			option_list->bm_option_list_flags |= OPTION_LIST_UPDATE_EXTENTS;
		}
		
	}
	else if(widget->bm_flags & WIDGET_JUST_RECEIVED_MOUSE_WHEEL_UP)
	{
		/*	if((unsigned short)((unsigned short)option_list->y_offset - (unsigned short)OPTION_HEIGHT) < (unsigned short)option_list->y_offset)
			{*/
			//	printf("%u %u\n", (unsigned short)option_list->y_offset, (unsigned short)((unsigned short)option_list->y_offset - (unsigned short)OPTION_HEIGHT));
		if(option_list->y_offset > 0)
		{
			option_list->y_offset -= (unsigned short)OPTION_HEIGHT;
			option_list->bm_option_list_flags |= OPTION_LIST_UPDATE_EXTENTS;
		}
			
			/*}*/
	}	
		
	if(option_list->bm_option_list_flags & OPTION_LIST_UPDATE_EXTENTS)
	{
		//top_y = (OPTION_HEIGHT * option_list->option_count) * 0.5;
		
		option_list->bm_option_list_flags &= ~OPTION_LIST_SCROLLER;
		
		option_w = option_list->widget.w;
		if(widget->bm_flags & WIDGET_DRAW_OUTLINE)
		{
			option_w -= 6;
		}
		
		option_x = 0;
		
		if(top_y > OPTION_HEIGHT * option_list->max_visible_options * 0.5)
		{
			top_y = OPTION_HEIGHT * option_list->max_visible_options * 0.5;
			option_list->bm_option_list_flags |= OPTION_LIST_SCROLLER;
			option_w -= 5;
			option_x = -5;
		}
		
		r = widget->nestled;
		
		widget->h = top_y;
		
		if(widget->bm_flags & WIDGET_DRAW_OUTLINE)
		{
			widget->h++;
		}			
		
		if(!(option_list->bm_option_list_flags & OPTION_LIST_DONT_TRANSLATE))
		{
			widget->y = option_list->first_y - top_y;
		}
	
		
		if(widget->parent)
		{
			
			if(option_list->bm_option_list_flags & OPTION_LIST_DONT_TRANSLATE)
			{
				widget->y = option_list->first_y;
			}
			
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
					widget->y += OPTION_HEIGHT * 0.5;
				}
				else if(widget->parent->type == WIDGET_DROPDOWN)
				{
					widget->y -= OPTION_HEIGHT * 0.5;
				}
				
				
			}
		}
										
		top_y -= OPTION_HEIGHT * 0.5;
		top_y += option_list->y_offset;
		r = widget->nestled;
		
		//printf("%d\n", option_w);
					
		while(r)
		{
			r->y = top_y;
			r->x = option_x;
			r->w = option_w;
			top_y -= OPTION_HEIGHT;
			r = r->next;
		}
					
		option_list->bm_option_list_flags &= ~OPTION_LIST_UPDATE_EXTENTS;
		
		if(widget->parent)
		{
			gui_GetAbsolutePosition(widget->parent, &x, &y);
		}
		else
		{
			x = 0;
			y = 0;
		}
		
		
		gui_UpdateWidgetRelativeMouse(widget, x, y);
		gui_UpdateWidgetMouseEvents(widget);
	}
	
	if(widget->parent)
	{
		parent = widget->parent;
			
		if(parent->type == WIDGET_OPTION)
		{		
			gui_GetAbsolutePosition(parent, &x, &y);
		
			if(x + parent->w + widget->w * 2.0 > r_window_width * 0.5) /*|| x + widget->w > r_window_width * 0.5)*/
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
			
			dropdown = (dropdown_t *)parent;
			
			gui_GetAbsolutePosition(parent, &x, &y);
			
			//printf("%d %d\n",y - parent->h - option_list->first_y - widget->h, -r_window_height / 2);
			
			if(y - parent->h - widget->h * 2.0 < -r_window_height / 2)
			{
				widget->y = parent->h + widget->h;
			}
			else
			{
				widget->y = -parent->h - widget->h;
			}
			
			/*if(y + parent->h + widget->h > r_window_height * 0.5)
			{
				widget->y = -parent->h - widget->h;
			}
			else
			{
				widget->y = parent->h + widget->h;
			}*/
		}	
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
	
		
	return;
	
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
			//printf("here\n");
		}
	}
	
	if(widget->process_callback)
	{
		widget->process_callback(widget);
	}
}


#ifdef __cplusplus
}
#endif




