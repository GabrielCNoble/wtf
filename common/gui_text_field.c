#include "gui_text_field.h"

#include <string.h>
#include <stdlib.h>

#include "input.h"

/* from input.c */
extern int bm_mouse;
extern widget_t *widgets;
extern widget_t *last_widget;

text_field_t *gui_AddTextField(widget_t *widget, char *name, short x, short y, short w, short bm_flags, void (*text_field_callback)(widget_t *))
{
	text_field_t *field;
	
	field = malloc(sizeof(text_field_t));
	
	
	field->widget.x = x;
	field->widget.y = y;
	field->widget.w = w / 2.0;
	field->widget.h = OPTION_HEIGHT / 2.0;
	field->widget.name = strdup(name);
	field->widget.next = NULL;
	field->widget.prev = NULL;
	field->widget.nestled = NULL;
	field->widget.type = WIDGET_TEXT_FIELD;
	field->widget.widget_callback = text_field_callback;
	field->widget.bm_flags = 0;
	
	
	field->bm_text_field_flags = bm_flags;
	field->cursor_blink_timer = TEXT_FIELD_CURSOR_BLINK_TIME;
	field->text_field_len = TEXT_FIELD_BUFFER_SIZE;
	field->text_field_char_count = 0;
	field->text = malloc(TEXT_FIELD_BUFFER_SIZE);
	
	if(!widget)
	{
		if(!widgets)
		{
			widgets = (widget_t *)field;
		}
		else
		{
			last_widget->next = (widget_t *)field;
			field->widget.prev = last_widget;
		}
		
		last_widget = (widget_t *)field;
	}
	else
	{
		if(!widget->nestled)
		{
			widget->nestled = (widget_t *)field;
		}
		else
		{
			widget->last_nestled->next = (widget_t *)field;
			field->widget.prev = widget->last_nestled;
		}
		
		widget->last_nestled = (widget_t *)field;
	}
	
	return field;
}

void gui_UpdateTextField(widget_t *widget)
{
	text_field_t *field = (text_field_t *)widget;
	
	
	
	if(field->bm_text_field_flags & TEXT_FIELD_RECEIVING_TEXT)
	{
		if(field->cursor_blink_timer > 0)
		{
			field->cursor_blink_timer--;
		}
		else
		{
			field->cursor_blink_timer = TEXT_FIELD_CURSOR_BLINK_TIME;
			field->bm_text_field_flags ^= TEXT_FIELD_CURSOR_VISIBLE;			
		}
		
	}
	
	/* left button click outside a text field will disable text receiving... */
	if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
	{
		if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
		{	
			if(!(field->bm_text_field_flags & TEXT_FIELD_READ_ONLY))
			{
				
				if(!(field->bm_text_field_flags & TEXT_FIELD_NO_WRITE))
				{			
					field->bm_text_field_flags |= TEXT_FIELD_CURSOR_VISIBLE | TEXT_FIELD_RECEIVING_TEXT;
					field->cursor_blink_timer = TEXT_FIELD_CURSOR_BLINK_TIME;
					
					input_EnableTextInput(1);
					
				}
				
			}
		}
		else
		{
			field->bm_text_field_flags &= ~(TEXT_FIELD_RECEIVING_TEXT | TEXT_FIELD_CURSOR_VISIBLE);
		}
	}
	
	
}

void gui_PostUpdateTextField(widget_t *widget)
{
	
}






