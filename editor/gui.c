#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix.h"
#include "gui.h"
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
		button->widget.bm_flags = 0;
		button->widget.type = WIDGET_BUTTON;
		button->widget.name = strdup(name);
		button->widget.parent = widget;
		button->widget.widget_callback = button_callback;
			
		button->bm_button_flags = bm_flags & (~BUTTON_PRESSED);
		
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
		checkbox->widget.bm_flags = 0;
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



dropdown_t *gui_AddDropDown(widget_t *widget, char *name, short x, short y, short w, short bm_flags)
{
	dropdown_t *dropdown;
	widget_t *wdgt;
	if(widget)
	{
		dropdown = malloc(sizeof(dropdown_t));
		
		dropdown->widget.bm_flags = 0;
		dropdown->widget.x = x;
		dropdown->widget.y = y;
		dropdown->widget.h = DROPDOWN_HEIGHT;
		
		if(w < WIDGET_MIN_SIZE)
			w = WIDGET_MIN_SIZE;
		
		dropdown->widget.w = w / 2;
		dropdown->widget.type = WIDGET_DROPDOWN;
		dropdown->widget.name = strdup(name);
		dropdown->widget.parent = widget;
		
		dropdown->max_options = 4;
		dropdown->options = malloc(sizeof(dropdown_option_t) * dropdown->max_options);
		
		dropdown->option_count = 0;
		dropdown->bm_dropdown_flags = bm_flags;
		
		dropdown->x_closed = x;
		dropdown->y_closed = y;
		dropdown->w_closed = w / 2;
		dropdown->h_closed = DROPDOWN_HEIGHT;
		
		dropdown->x_dropped = x;
		dropdown->y_dropped = y; 
		dropdown->w_dropped = w / 2;
		dropdown->h_dropped = DROPDOWN_HEIGHT;
		
		
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
		
		/*wdgt = widget;
		
		while(wdgt->parent)
		{
			wdgt = wdgt->parent;
		}
		
		dropdown->widget.first_parent = wdgt;*/
	}
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

void gui_ProcessGUI()
{
	widget_t *w = widgets;
	widget_t *new_top;
	button_t *button;
	dropdown_t *dropdown;
	checkbox_t *checkbox;
	
	int widget_stack_top = -1;
	widget_t *widget_stack[128];
	
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
	
	new_top = NULL;
	
	w = top_widget;
	
	_do_rest:
		
	while(w)
	{
		relative_screen_mouse_x = screen_mouse_x - (w->x + x);
		relative_screen_mouse_y = screen_mouse_y - (w->y + y);
		
		w->relative_mouse_x = relative_screen_mouse_x / (float)w->w;
		w->relative_mouse_y = relative_screen_mouse_y / (float)w->h;
		
		if(w == top_widget)
		{
			/* if the top widget was already processed, skip
			reprocessing it when updating the rest of the list... */
			if(b_do_rest)
			{
				w = w->next;
				continue;
			}
			
			
			/* this enables manipulating widgets like sliders. This
			only clears the WIDGET_HAS_LEFT_MOUSE_BUTTON if the left
			mouse button is not pressed... */
			if(!(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
			{
				w->bm_flags &= ~WIDGET_HAS_LEFT_MOUSE_BUTTON;
			}
			
		}
		else
		{
			w->bm_flags &= ~(WIDGET_HAS_LEFT_MOUSE_BUTTON | 
							 WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON | 
							 WIDGET_HAS_RIGHT_MOUSE_BUTTON |
							 WIDGET_JUST_RECEIVED_RIGHT_MOUSE_BUTTON);
		}
		
		
		/* every widget that is not the top widget
		must have its WIDGET_MOUSE_OVER flag cleared,
		to avoid other keeping this flag when
		the mouse just hovered over the top widget... */
		w->bm_flags &= ~WIDGET_MOUSE_OVER;
		
		
		
		
		/* if the top widget has the mouse over itself,
		avoid updating anything else from the rest of the
		list... */
		//if(top_widget->bm_flags & WIDGET_MOUSE_OVER && b_do_rest)
		//{
		//	w = w->next;
		//	goto _update_specific_flags;
			//continue;
	//	}
		
		
		
		if(w->relative_mouse_x >= -1.0 && w->relative_mouse_x <= 1.0 &&
		   w->relative_mouse_y >= -1.0 && w->relative_mouse_y <= 1.0)
		{
			
			if((!(top_widget->bm_flags & WIDGET_MOUSE_OVER)) || (w->parent->bm_flags & WIDGET_MOUSE_OVER) /* w->first_parent == top_widget*/)
			{
				
				if(w->parent)
				{
					if(!(w->parent->bm_flags & WIDGET_MOUSE_OVER))
					{
						goto _skip_mouse_over;
					}
				}
				
				w->bm_flags |= WIDGET_MOUSE_OVER;
				
				if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
				{
					w->bm_flags |= WIDGET_HAS_LEFT_MOUSE_BUTTON | WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON;
					
					/* latch the change until everything is done... */
					if(!new_top)
					{
						new_top = w;
					}
				}
				
			}
		}
		
		_skip_mouse_over:
		
		/*else
		{
			w->bm_flags &= ~WIDGET_MOUSE_OVER;
		}*/
		
		//_update_specific_flags:
		
		call_callback = 0;
		
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
				
				//printf("%d\n", w->bm_flags & WIDGET_MOUSE_OVER);
				
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
								//w->widget_callback();
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
								//w->widget_callback();
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
				
				if(dropdown->bm_dropdown_flags & DROPDOWN_DROPPED)
				{
					
				}
				
				if(w->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
				{
					if(dropdown->bm_dropdown_flags & DROPDOWN_DROPPED)
					{
						dropdown->bm_dropdown_flags &= ~DROPDOWN_DROPPED;
						
						w->x = dropdown->x_closed;
						w->y = dropdown->y_closed;
						w->w = dropdown->w_closed;
						w->h = dropdown->h_closed;
						printf("nope!\n");
					}
					else
					{
						dropdown->bm_dropdown_flags |= DROPDOWN_DROPPED;
						
						w->x = dropdown->x_dropped;
						w->y = dropdown->y_dropped;
						w->w = dropdown->w_dropped;
						w->h = dropdown->h_dropped;
						
						printf("dropped!\n");
					}
				}
				
				
				
				
				
				
			break;
		}
		
		if(call_callback)
		{
			w->widget_callback(w);
		}
		
		/* if all the nestled widgets of the top widget have been 
		properly processed, quit the loop and allow the rest
		of the list to be updated... */
		if(widget_stack_top < 0 && (!b_do_rest))
		{
			break;
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
	
	
	if(!b_do_rest)
	{
		b_do_rest = 1;
		w = widgets;
		goto _do_rest;
	}
	
	if(new_top)
	{
		top_widget = new_top;
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



