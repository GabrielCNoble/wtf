#include <stdlib.h>
#include <string.h>

#include "gui_button.h"

button_t *gui_CreateButton(char *name, short x, short y, short w, short h, short bm_flags, void (*button_callback)(widget_t *widget))
{
	button_t *button = NULL;
		
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
	button->widget.parent = NULL;
	button->widget.widget_callback = button_callback;
	
			
	button->bm_button_flags = bm_flags & (~BUTTON_PRESSED);
	button->rendered_text = NULL;
	
	return button;
}

button_t *gui_AddButton(widget_t *widget, char *name, short x, short y, short w, short h, short bm_flags, void (*button_callback)(widget_t *widget))
{
	button_t *button = NULL;
	widget_t *wdgt;
	
	if(widget)
	{
		
		button = gui_CreateButton(name, x, y, w, h, bm_flags, button_callback);
		/*button = malloc(sizeof(button_t));
	
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
		button->rendered_text = NULL;*/
		
		button->widget.parent = widget;
		
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

void gui_UpdateButton(widget_t *widget)
{
	button_t *button = (button_t *)widget;
				
	if(button->bm_button_flags & BUTTON_TOGGLE)
	{
		if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
		{
			if(button->bm_button_flags & BUTTON_PRESSED)
			{
				button->bm_button_flags &= ~BUTTON_PRESSED;
			}
			else
			{
				button->bm_button_flags |= BUTTON_PRESSED;
				
				if(widget->widget_callback)
				{
					widget->widget_callback(widget);
				}
						
			}
		}
	}
	else
	{
		if(widget->bm_flags & WIDGET_HAS_LEFT_MOUSE_BUTTON)
		{
			if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
			{
				if(widget->widget_callback)
				{
					widget->widget_callback(widget);
				}		
			}
							
			button->bm_button_flags |= BUTTON_PRESSED;
		}
		else
		{
			button->bm_button_flags &= ~BUTTON_PRESSED;
		}
	}
}
