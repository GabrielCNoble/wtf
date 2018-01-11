#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL2\SDL_ttf.h"

#include "matrix.h"
#include "gui.h"
#include "font.h"
#include "r_main.h"
#include "input.h"


/* from r_main.h */
extern int r_width;
extern int r_height;

/* from input.c */
extern float normalized_mouse_x;
extern float normalized_mouse_y;
extern int bm_mouse;

widget_t *widgets;
widget_t *last_widget;
widget_t *top_widget;
mat4_t gui_projection_matrix;

static char formated_str[8192];


extern font_t *gui_font;

void gui_Init()
{
	widgets = NULL;
	gui_UpdateGUIProjectionMatrix();
}

void gui_Finish()
{
	
}

widget_t *gui_CreateWidget(char *name, short x, short y, short w, short h)
{
	widget_t *widget;
	
	widget = malloc(sizeof(widget_t));
	
	if(w < WIDGET_MIN_SIZE) w = WIDGET_MIN_SIZE;
	if(h < WIDGET_MIN_SIZE) h = WIDGET_MIN_SIZE;
	
	widget->last_nestled = NULL;
	widget->nestled = NULL;
	widget->next = NULL;
	widget->prev = NULL;
	widget->x = x;
	widget->y = y;
	widget->w = w / 2;
	widget->h = h / 2;
	widget->bm_flags = 0;
	widget->type = WIDGET_NONE;
	widget->parent = NULL;
	//widget->first_parent = NULL;
	widget->widget_callback = NULL;
	
	widget->name = strdup(name);
	
	if(!widgets)
	{
		widgets = widget;
		last_widget = widget;
		top_widget = widget;
	}
	else
	{
		last_widget->next = widget;
		widget->prev = last_widget;
		last_widget = widget;
	}
	
	return widget;
}


button_t *gui_AddButton(widget_t *widget, char *name, short x, short y, short w, short h, short bm_flags, void (*button_callback)(widget_t *widget))
{
	button_t *button = NULL;
	widget_t *wdgt;
	
	if(widget)
	{
		button = malloc(sizeof(button_t));
	
		if(w < WIDGET_MIN_SIZE) w = WIDGET_MIN_SIZE;
		if(h < WIDGET_MIN_SIZE) h = WIDGET_MIN_SIZE;
		
		button->widget.last_nestled = NULL;
		button->widget.nestled = NULL;
		button->widget.next = NULL;
		button->widget.prev = NULL;
		button->widget.x = x;
		button->widget.y = y;
		button->widget.w = w / 2;
		button->widget.h = h / 2;
		button->widget.bm_flags = WIDGET_RENDER_TEXT;
		button->widget.type = WIDGET_BUTTON;
		button->widget.name = strdup(name);
		button->widget.parent = widget;
		button->widget.widget_callback = button_callback;
			
		button->bm_button_flags = bm_flags & (~BUTTON_PRESSED);
		button->rendered_text = NULL;
		
		if(!widget->nestled)
		{
			widget->nestled = (widget_t *)button;
			widget->last_nestled = (widget_t *)button;
		}
		else
		{
			widget->last_nestled->next = (widget_t *)button;
			button->widget.prev = widget->last_nestled;
			widget->last_nestled = (widget_t *)button;
		}
	}
	
	return button;
}


checkbox_t *gui_AddCheckBox(widget_t *widget, short x, short y, short w, short h, short bm_flags, void (*checkbox_callback)(widget_t *widget))
{
	checkbox_t *checkbox = NULL;
	widget_t *wdgt;
	
	if(widget)
	{
		checkbox = malloc(sizeof(checkbox_t));
	
		if(w < CHECKBOX_MIN_SIZE) w = CHECKBOX_MIN_SIZE;
		if(h < CHECKBOX_MIN_SIZE) h = CHECKBOX_MIN_SIZE;
		
		checkbox->widget.last_nestled = NULL;
		checkbox->widget.nestled = NULL;
		checkbox->widget.next = NULL;
		checkbox->widget.prev = NULL;
		checkbox->widget.x = x;
		checkbox->widget.y = y;
		checkbox->widget.w = w / 2;
		checkbox->widget.h = h / 2;
		checkbox->widget.bm_flags = WIDGET_RENDER_TEXT;
		checkbox->widget.type = WIDGET_CHECKBOX;
		//checkbox->widget.name = strdup(name);
		checkbox->widget.parent = widget;
		checkbox->widget.widget_callback = checkbox_callback;
			
		checkbox->bm_checkbox_flags = bm_flags;
		
		if(!widget->nestled)
		{
			widget->nestled = (widget_t *)checkbox;
			widget->last_nestled = (widget_t *)checkbox;
		}
		else
		{
			widget->last_nestled->next = (widget_t *)checkbox;
			checkbox->widget.prev = widget->last_nestled;
			widget->last_nestled = (widget_t *)checkbox;
		}
	}
	
	return checkbox;
}



dropdown_t *gui_AddDropDown(widget_t *widget, char *name, char *text, short x, short y, short w, short bm_flags, void (*dropdown_callback)(widget_t *widget))
{
	dropdown_t *dropdown = NULL;
	widget_t *wdgt;
	if(widget)
	{
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
		dropdown->widget.parent = widget;
		dropdown->widget.next = NULL;
		dropdown->widget.prev = NULL;
		dropdown->widget.nestled = NULL;
		dropdown->widget.last_nestled = NULL;
		dropdown->widget.widget_callback = dropdown_callback;
		

		
		//dropdown->max_options = 4;
		//dropdown->options = malloc(sizeof(dropdown_option_t) * dropdown->max_options);
		
		//dropdown->option_count = 0;
		dropdown->bm_dropdown_flags = bm_flags;
		
		if(text)
			dropdown->dropdown_text = strdup(text);
		else 
			dropdown->dropdown_text = NULL;
			
		dropdown->rendered_text = NULL;	
		
	/*	dropdownT_->x_closed = x;
		dropdown->y_closed = y;
		dropdown->w_closed = w / 2;
		dropdown->h_closed = DROPDOWN_HEIGHT / 2.0;
		
		dropdown->x_dropped = x;
		dropdown->y_dropped = y; 
		dropdown->w_dropped = w / 2;
		dropdown->h_dropped = DROPDOWN_HEIGHT / 2.0;*/
		
		
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


void gui_SetAsTop(widget_t *widget)
{
	widget_t **first;
	widget_t **last;
	
	if(widget)
	{
		if(widget->parent)
		{			
			first = &widget->parent->nestled;
			last = &widget->parent->last_nestled;
		}
		else
		{
			first = &widgets;
			last = &last_widget;
		}
		
		/* widget is already first in the list (or the only one in the list)... */
		if(!widget->prev)
		{
			return;
		}
			
		/* by here there will be at least two widgets in the list, 
		and it will be either in the middle or the last... */
		widget->prev->next = widget->next;
			
		if(widget->next)
		{
			/* middle... */
			widget->next->prev = widget->prev;
		}
		else
		{
			/* last... */
			*last = widget->prev;
		}
			
		widget->prev = NULL;
		widget->next = *first;
		(*first)->prev = widget;
		*first = widget;		
	}
}


void gui_RenderText(widget_t *widget)
{
	option_t *option;
	dropdown_t *dropdown;
	SDL_Color foreground = {255, 255, 255, 255};
	SDL_Color background = {0, 0, 0, 0};
	
	if(!gui_font)
		return;
	
	switch(widget->type)
	{
		case WIDGET_OPTION:
			option = (option_t *)widget;
			
			if(!option->option_text)
				return;
			
			if(option->rendered_text)
			{
				SDL_FreeSurface(option->rendered_text);
			}
			
			sprintf(formated_str, option->option_text);
			
			option->rendered_text = TTF_RenderUTF8_Blended_Wrapped(gui_font->font, formated_str, foreground, option->widget.w * 2.0);
		break;
		
		case WIDGET_DROPDOWN:
			dropdown = (dropdown_t *)widget;
			if(!dropdown->dropdown_text)
				return;
				
			if(dropdown->rendered_text)
				SDL_FreeSurface(dropdown->rendered_text);
			
			sprintf(formated_str, dropdown->dropdown_text);
			
			dropdown->rendered_text = TTF_RenderUTF8_Blended_Wrapped(gui_font->font, formated_str, foreground, dropdown->widget.w * 2.0);
					
		break;
	}
	
	
	widget->bm_flags &= ~WIDGET_RENDER_TEXT;
}


#if 0
/* this function scrambles the links
of nestled widgets, so DO NOT CALL IT! */
void gui_SetTopWidget(widget_t *widget)
{
	
	/* Only do something if there's more than one
	widget on the list... */
	if(widgets->next)
	{
		
		/* Only do something if this widget is not already the
		top widget (and hence, the first on the list) */
		if(widget != widgets)
		{
			widget->prev->next = widget->next;
			
			/* If this widget is not the last in the list, do pointer
			business... */
			if(widget->next)
			{
				widget->next->prev = widget->prev;
			}
			
			/* Old first points to this widget... */
			widgets->prev = widget;
			/* This widget points to old first... */
			widget->next = widgets;
			/* This widget is now first... */
			widgets = widget;
			/* Self explanatory... */
			widgets->prev = NULL;
		}
	}
	
	top_widget = widget;
}
#endif

#define WIDGET_STACK_SIZE 128

#define push_widget(w) if(widget_stack_top+1>=WIDGET_STACK_SIZE)		\\
					   {												\\
							printf("cannot push widget!\n");			\\
					   }												\\
					   else												\\
					   {												\\
					   		x += w->x;									\\
							y += w->y;									\\
							widget_stack_top++;							\\
							widget_stack[widget_stack_top] = w;			\\
							w = w->nestled;								\\
					   }





void gui_ProcessGUI()
{
	widget_t *w;
	widget_t *new_top;
	widget_t *top = top_widget;
	widget_t *r;
	button_t *button;
	dropdown_t *dropdown;
	checkbox_t *checkbox;
	option_list_t *option_list;
	option_t *option;
	
	int widget_stack_top = -1;
	widget_t *widget_stack[WIDGET_STACK_SIZE];
	
	float screen_mouse_x = (r_width * 0.5) * normalized_mouse_x;
	float screen_mouse_y = (r_height * 0.5) * normalized_mouse_y;

	float relative_screen_mouse_x;
	float relative_screen_mouse_y;
	
	float relative_mouse_x;
	float relative_mouse_y;
	
	int b_do_rest = 0;
	int x = 0;
	int y = 0;
	int call_callback;
	int mouse_over_top;
	
	short top_y;
	short y_increment;
	short bottom_y;
	
	new_top = NULL;
	
	//w = top_widget;
	w = widgets;
	
	_do_rest:
		
	while(w)
	{
		relative_screen_mouse_x = screen_mouse_x - (w->x + x);
		relative_screen_mouse_y = screen_mouse_y - (w->y + y);
		
		w->relative_mouse_x = relative_screen_mouse_x / (float)w->w;
		w->relative_mouse_y = relative_screen_mouse_y / (float)w->h;
		
		/* ignore invisible widgets... */
		if(w->bm_flags & WIDGET_INVISIBLE)
			goto _advance_widget;
		
		if(w->parent)
		{
			top = w->parent->nestled;
		}
		else
		{
			top = widgets;
		}
				
		if(w == top)
		{	
			/* this enables manipulating widgets like sliders. This
			only clears the WIDGET_HAS_LEFT_MOUSE_BUTTON if the left
			mouse button is not pressed, given that the top widget will
			conserve the flag even if the mouse is not over it... */
			if(!(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
			{
				w->bm_flags &= ~WIDGET_HAS_LEFT_MOUSE_BUTTON;
			}
		}
		else
		{
			w->bm_flags &= ~(WIDGET_HAS_LEFT_MOUSE_BUTTON | WIDGET_HAS_RIGHT_MOUSE_BUTTON);
		}
		
		w->bm_flags &= ~(WIDGET_MOUSE_OVER | WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON | WIDGET_JUST_RECEIVED_RIGHT_MOUSE_BUTTON);
		
		
		if(w->relative_mouse_x >= -1.0 && w->relative_mouse_x <= 1.0 &&
		   w->relative_mouse_y >= -1.0 && w->relative_mouse_y <= 1.0)
		{
			if(!(top->bm_flags & WIDGET_MOUSE_OVER))
			{
								
				r = top->parent;
				/* propagate the flag upwards... */
				while(r)
				{
					r->bm_flags |= WIDGET_MOUSE_OVER;
					r = r->parent;
				}
				
				w->bm_flags |= WIDGET_MOUSE_OVER;
				
				if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
				{
					w->bm_flags |= WIDGET_HAS_LEFT_MOUSE_BUTTON | WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON;
					
					//top->bm_flags &= ~(WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON | WIDGET_JUST_RECEIVED_RIGHT_MOUSE_BUTTON | WIDGET_HAS_LEFT_MOUSE_BUTTON | WIDGET_HAS_RIGHT_MOUSE_BUTTON);
					
					gui_SetAsTop(w);
				}
				
			}
		}
		
		call_callback = 0;
		
		
		if(w->bm_flags & WIDGET_RENDER_TEXT)
		{
			gui_RenderText(w);
		}
		
		
		switch(w->type)
		{
			case WIDGET_NONE:
				if(w->nestled)
				{
					x += w->x;
					y += w->y;
					widget_stack_top++;		
					widget_stack[widget_stack_top] = w;
					w = w->nestled;
					continue;
				}
			break;
			
			case WIDGET_BUTTON:
				
				button = (button_t *)w;
				
				if(button->bm_button_flags & BUTTON_TOGGLE)
				{
					if(w->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
					{
						if(button->bm_button_flags & BUTTON_PRESSED)
						{
							button->bm_button_flags &= ~BUTTON_PRESSED;
						}
						else
						{
							button->bm_button_flags |= BUTTON_PRESSED;
							
							if(w->widget_callback)
							{
								call_callback = 1;
							}
								
						}
					}
				}
				else
				{
					if(w->bm_flags & WIDGET_HAS_LEFT_MOUSE_BUTTON)
					{
						if(w->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
						{
							if(w->widget_callback)
							{
								call_callback = 1;
							}		
						}
							
						button->bm_button_flags |= BUTTON_PRESSED;
					}
					else
					{
						button->bm_button_flags &= ~BUTTON_PRESSED;
					}
				}
			break;
			
			case WIDGET_CHECKBOX:
				checkbox = (checkbox_t *)w;
				
				if(w->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
				{
					if(checkbox->bm_checkbox_flags & CHECKBOX_CHECKED)
					{
						checkbox->bm_checkbox_flags &= ~CHECKBOX_CHECKED;
					}
					else
					{
						checkbox->bm_checkbox_flags |= CHECKBOX_CHECKED;
					}
					
					if(w->widget_callback)
					{
						call_callback = 1;
					}
					
				}
				
			break;
			
			case WIDGET_DROPDOWN:
				
				dropdown = (dropdown_t *)w;
				
				if(w->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
				{
					if(dropdown->bm_dropdown_flags & DROPDOWN_DROPPED)
					{
						dropdown->bm_dropdown_flags &= ~DROPDOWN_DROPPED;
					
						if(w->nestled)
						{
							w->nestled->bm_flags |= WIDGET_INVISIBLE;
						}
					}
					else
					{
						dropdown->bm_dropdown_flags |= DROPDOWN_DROPPED;
						
						if(w->nestled)
						{
							w->nestled->bm_flags &= ~WIDGET_INVISIBLE;
						}
					}
				}
				
				#if 0
				/* if the user clicks somewhere else, close this dropdown
				box... */
				if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
				{
					if(!(w->bm_flags & WIDGET_MOUSE_OVER))
					{
						dropdown->bm_dropdown_flags &= ~DROPDOWN_DROPPED;
						if(w->nestled)
						{
							w->nestled->bm_flags |= WIDGET_INVISIBLE;
						}
					}
				}
				#endif
				
				
				if(w->nestled)
				{
					if(!(w->nestled->bm_flags & WIDGET_INVISIBLE))
					{
						x += w->x;
						y += w->y;
						widget_stack_top++;		
						widget_stack[widget_stack_top] = w;
						w = w->nestled;
						continue;
					}		
				}			
			break;
			
			case WIDGET_OPTION_LIST:
				
				option_list = (option_list_t *)w;
				
				if(option_list->bm_option_list_flags & OPTION_LIST_UPDATE_EXTENTS)
				{
					top_y = (OPTION_HEIGHT * option_list->option_count) * 0.5;
					
					
					w->h = top_y;
					w->y = -top_y;
					
					/* if this option list is nestled within a option, 
					make it align correctly... */
					if(option_list->widget.parent->type == WIDGET_OPTION)
					{
						w->y += OPTION_HEIGHT * 0.5;
					}
					else
					{
						w->y -= OPTION_HEIGHT * 0.5;
					}
					
					/* - OPTION_HEIGHT * 0.5;*/
					
					
					top_y -=  OPTION_HEIGHT * 0.5;
					r = w->nestled;
					
					while(r)
					{
						r->y = top_y;
						top_y -= OPTION_HEIGHT;
						r = r->next;
					}
					
					option_list->bm_option_list_flags &= ~OPTION_LIST_UPDATE_EXTENTS;
				}
				
				if(w->nestled)
				{
					x += w->x;
					y += w->y;
					widget_stack_top++;		
					widget_stack[widget_stack_top] = w;
					w = w->nestled;
					continue;
				}
			break;
			
			case WIDGET_OPTION:
				
				option = (option_t *)w;
				option_list = (option_list_t *)w->parent;
				
				if(w->bm_flags & WIDGET_MOUSE_OVER)
				{
					/* if this option has the mouse over it, make
					it the active option of this option list... */
					option_list->active_option_index = option->index;
					option_list->active_option = (struct option_t *)option;	
					
					if(w->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
					{
						call_callback = 1;
					}
					
				}
				else
				{
					if((option_t *)option_list->active_option == option)
					{
						/* only keep the option as active when the mouse
						is away only if this option has any nestled options... */
						if(!option->widget.nestled)
						{
							option_list->active_option_index = -1;
							option_list->active_option = NULL;
						}
					}
					
				}
				
				if(w->nestled)
				{
					//if(w->bm_flags & WIDGET_MOUSE_OVER)
					/* only recurse down if this option is the active one... */
					if(option == (option_t *)option_list->active_option)
					{
						w->nestled->bm_flags &= ~WIDGET_INVISIBLE;
						
						x += w->x;
						y += w->y;
						widget_stack_top++;		
						widget_stack[widget_stack_top] = w;
						w = w->nestled;
						continue;
					}
					else
					{
						w->nestled->bm_flags |= WIDGET_INVISIBLE;
					}
				}
			break;
		}
		
		if(call_callback)
		{
			w->widget_callback(w);
		}
		
		
		_advance_widget:
		
		w = w->next;
		
		/* this will keep poping from this stack until
		something not null appears to be processed or 
		until the stack is empty. The latter means
		the work is done and we can go home... */
		if(!w)
		{
			if(widget_stack_top >= 0)
			{
				w = widget_stack[widget_stack_top];
				widget_stack_top--;
				
				x -= w->x;
				y -= w->y;
				
				goto _advance_widget;
			}
		}
		
	}
		
}

void gui_UpdateGUIProjectionMatrix()
{
	float right = r_width / 2;
	float left = -right;
	float top = r_height / 2;
	float bottom = -top;
	CreateOrthographicMatrix(&gui_projection_matrix, left, right, top, bottom, -10.0, 10.0, NULL);
}



