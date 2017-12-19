#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix.h"
#include "gui.h"
#include "r_main.h"
#include "input.h"


/* from renderer.h */
extern int window_width;
extern int window_height;

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

button_t *gui_AddButtonToWidget(widget_t *widget, char *name, short x, short y, short w, short h, short bm_flags)
{
	button_t *button = NULL;
	
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
	
	int widget_stack_top = -1;
	widget_t *widget_stack[128];
	
	float screen_mouse_x = (window_width * 0.5) * normalized_mouse_x;
	float screen_mouse_y = (window_height * 0.5) * normalized_mouse_y;

	float relative_screen_mouse_x;
	float relative_screen_mouse_y;
	
	float relative_mouse_x;
	float relative_mouse_y;
	
	int b_do_rest = 0;
	short x = 0;
	short y = 0;
	
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
			
			if(!(top_widget->bm_flags & WIDGET_MOUSE_OVER))
			{
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
		/*else
		{
			w->bm_flags &= ~WIDGET_MOUSE_OVER;
		}*/
		
		//_update_specific_flags:
		
		
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
						}
					}
				}
				else
				{
					if(w->bm_flags & WIDGET_HAS_LEFT_MOUSE_BUTTON)
					{
						button->bm_button_flags |= BUTTON_PRESSED;
					}
					else
					{
						button->bm_button_flags &= ~BUTTON_PRESSED;
					}
				}
			break;
			
			case WIDGET_CHECKBOX:
			
			break;
		}
		
		/* if all the nestled widgets of the top widgets have been 
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
	float right = window_width / 2;
	float left = -right;
	float top = window_height / 2;
	float bottom = -top;
	CreateOrthographicMatrix(&gui_projection_matrix, left, right, top, bottom, -10.0, 10.0, NULL);
}



