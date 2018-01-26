#include "gui_text_field.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "input.h"

/* from input.c */
extern int bm_mouse;
extern int text_buffer_in;
extern int text_buffer_out;
extern char text_buffer[];

/* from gui.c */
extern widget_t *widgets;
extern widget_t *last_widget;

text_field_t *gui_AddTextField(widget_t *widget, char *name, short x, short y, short w, short bm_flags, void (*text_field_callback)(widget_t *))
{
	text_field_t *field;
	
	int i;
	
	field = malloc(sizeof(text_field_t));
	
	
	field->widget.x = x;
	field->widget.y = y;
	field->widget.w = w / 2.0;
	field->widget.h = OPTION_HEIGHT / 2.0;
	field->widget.name = strdup(name);
	field->widget.next = NULL;
	field->widget.prev = NULL;
	field->widget.nestled = NULL;
	field->widget.parent = widget;
	field->widget.type = WIDGET_TEXT_FIELD;
	field->widget.widget_callback = text_field_callback;
	field->widget.bm_flags = 0;
	
	
	field->bm_text_field_flags = bm_flags;
	field->cursor_blink_timer = TEXT_FIELD_CURSOR_BLINK_TIME;
	field->text_buffer_size = TEXT_FIELD_BUFFER_SIZE;
	field->text_buffer_cursor = 0;
	field->text_cursor = 0;
	field->text = malloc(TEXT_FIELD_BUFFER_SIZE);
	field->rendered_text = NULL;
	
	
	for(i = 0; i < TEXT_FIELD_BUFFER_SIZE; i++)
	{
		field->text[i] = '\0';
	}
	
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

void gui_SetText(widget_t *widget, char *text)
{
	text_field_t *field;
	int i = 0;
	
	if(widget->type == WIDGET_TEXT_FIELD)
	{
		field = (text_field_t *)widget;
		
		field->text_buffer_cursor = 0;
		
		while(text[i])
		{
			field->text[field->text_buffer_cursor] = text[i];
			i++;
			field->text_buffer_cursor++;
		}
		
		field->text[field->text_buffer_cursor] = '\0';
	}
}

void gui_UpdateTextField(widget_t *widget)
{
	text_field_t *field = (text_field_t *)widget;
	gui_var_t *var;
	int i;
	char *temp;
	
	
	if(!(widget->bm_flags & WIDGET_TRACK_VAR))
	{
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
			
			while(text_buffer_out != text_buffer_in)
			{
				widget->bm_flags |= WIDGET_RENDER_TEXT;
				
				if(field->bm_text_field_flags & TEXT_FIELD_DRAW_TEXT_SELECTED)
				{
					field->text_buffer_cursor = 0;
					field->text[0] = ' ';
					for(i = 1; i < field->text_buffer_size; i++)
					{
						field->text[i] = '\0';
					}
				
					field->bm_text_field_flags &= ~TEXT_FIELD_DRAW_TEXT_SELECTED;		
				}
				
				switch(text_buffer[text_buffer_out])
				{
					case SDLK_BACKSPACE:
						if(field->text_buffer_cursor > 0)
						{
							field->text[field->text_buffer_cursor] = '\0';
							field->text_buffer_cursor--;
						}
						field->text[field->text_buffer_cursor] = ' ';
					break;
					
					case SDLK_RETURN:
					//case SDLK_RETURN2:
						text_buffer_out = (text_buffer_out + 1) % TEXT_BUFFER_SIZE;
						goto _disable_text_receiving;
					break;
					
					
					default:
						field->text[field->text_buffer_cursor] = text_buffer[text_buffer_out];
						if(field->text_buffer_cursor + 1 >= field->text_buffer_size)
						{
							temp = malloc(field->text_buffer_size + 32);
							memcpy(temp, field->text, field->text_buffer_cursor);
							free(field->text);
							field->text = temp;
							field->text_buffer_size += 32;
						}
						
						field->text[field->text_buffer_cursor + 1] = '\0';
						field->text_buffer_cursor++;
					break;
				}
				
				text_buffer_out = (text_buffer_out + 1) % TEXT_BUFFER_SIZE;
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
						
						if(field->text_buffer_cursor)
						{
							field->bm_text_field_flags |= TEXT_FIELD_DRAW_TEXT_SELECTED;
							widget->bm_flags |= WIDGET_RENDER_TEXT;
						}
						//field->text_buffer_cursor = 0;
						
						input_EnableTextInput(1);
						
					}
					
				}
			}
			else
			{
				_disable_text_receiving:
					
				field->bm_text_field_flags &= ~(TEXT_FIELD_RECEIVING_TEXT | TEXT_FIELD_CURSOR_VISIBLE | TEXT_FIELD_DRAW_TEXT_SELECTED);
				widget->bm_flags |= WIDGET_RENDER_TEXT;
			}
		}
	}
	else
	{
		var = widget->var;
		
		if(var->bm_flags & GUI_VAR_VALUE_HAS_CHANGED)
		{
			switch(var->type)
			{
				case GUI_VAR_FLOAT:
					sprintf(field->text, "%.02f", *((float *)var->addr));
				break;
				
				case GUI_VAR_INT:
					sprintf(field->text, "%d", *((int *)var->addr));
				break;
				
				case GUI_VAR_STRING:
					sprintf(field->text, "%s", *((char **)var->addr));
				break;
			}
				
			var->bm_flags &= ~GUI_VAR_VALUE_HAS_CHANGED;
			widget->bm_flags |= WIDGET_RENDER_TEXT;
		}
	}
	
	
	
	
	
}

void gui_PostUpdateTextField(widget_t *widget)
{
	
}






