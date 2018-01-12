#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL2\SDL_ttf.h"

#include "matrix.h"
#include "gui.h"
#include "font.h"
#include "r_main.h"
#include "input.h"


//#include "gui_dropdown.h"


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
	widget_bar_t *bar;
	
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
		
		if(w->parent)
		{
			top = w->parent->nestled;
			//top = w->parent->top;
		}
		else
		{
			top = widgets;
		}
		
		relative_screen_mouse_x = screen_mouse_x - (w->x + x);
		relative_screen_mouse_y = screen_mouse_y - (w->y + y);
		
		w->relative_mouse_x = relative_screen_mouse_x / (float)w->w;
		w->relative_mouse_y = relative_screen_mouse_y / (float)w->h;
		
		w->bm_flags &= ~(WIDGET_MOUSE_OVER | WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON | WIDGET_JUST_RECEIVED_RIGHT_MOUSE_BUTTON);
		
		/* ignore invisible widgets... */
		if(w->bm_flags & WIDGET_INVISIBLE)
			goto _advance_widget;
		
		
				
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
		
		
		
		
		/*if(w->type == WIDGET_BUTTON)
		{
			printf("%f %f\n", w->relative_mouse_x, w->relative_mouse_y);
			
			
		}*/
		
		
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
					/*if(w->parent)
					{
						w->parent->top = w;
					}*/
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
				gui_UpdateButton(w);
			break;
			
			case WIDGET_CHECKBOX:
				gui_UpdateCheckbox(w);
				/*checkbox = (checkbox_t *)w;
				
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
					
				}*/
				
			break;
			
			case WIDGET_DROPDOWN:
				
				gui_UpdateDropdown(w);
				
				dropdown = (dropdown_t *)w;
					
				if(w->nestled)
				{
					//if(!(w->nestled->bm_flags & WIDGET_INVISIBLE))
					if(dropdown->bm_dropdown_flags & DROPDOWN_DROPPED)
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
				
				gui_UpdateOptionList(w);
				
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
				
				gui_UpdateOption(w);
				
				if(w->nestled)
				{
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
			
			case WIDGET_BAR:
				
				gui_UpdateWidgetBar(w);
				
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
		}
		
		/*if(call_callback)
		{
			w->widget_callback(w);
		}*/
		
		
		_advance_widget:
		
		/* we'll get here after recursing through all the
		widgets contained within this bar, and so we have
		all the information needed to properly decide things... */
		if(w->type == WIDGET_BAR)
		{
			bar = (widget_bar_t *)w;
			if(bar->process_fn)
			{
				bar->process_fn(w);
			}
		}
		
		
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



