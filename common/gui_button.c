#include <stdlib.h>
#include <string.h>

#include "gui.h"
#include "gui_button.h"
#include "c_memory.h"

#ifdef __cplusplus
extern "C"
{
#endif

button_t *gui_CreateButton(char *name, short x, short y, short w, short h, short bm_flags, void (*button_callback)(widget_t *widget))
{
	button_t *button = NULL;

	button = (button_t *)gui_CreateWidget(name, x, y, w, h, WIDGET_BUTTON);

	button->widget.bm_flags = WIDGET_RENDER_TEXT;
	button->widget.widget_callback = button_callback;

	button->bm_button_flags = bm_flags & (~BUTTON_PRESSED);
	button->rendered_text = NULL;
	button->button_text = NULL;

	return button;
}

button_t *gui_AddButton(widget_t *widget, char *name, short x, short y, short w, short h, short bm_flags, void (*button_callback)(widget_t *widget))
{
	button_t *button = NULL;
	widget_t *wdgt;

	if(widget)
	{

		button = gui_CreateButton(name, x, y, w, h, bm_flags, button_callback);

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

void gui_SetButtonText(button_t *button, char *text)
{
	int prev_text_len = 0;
	int new_text_len = 0;

	if(button)
	{
		if(text)
		{
			if(button->button_text)
			{
				prev_text_len = strlen(button->button_text) + 1;
				new_text_len = strlen(text) + 1;

				if(new_text_len > prev_text_len)
				{
					memory_Free(button->button_text);
					button->button_text = memory_Malloc(new_text_len, "gui_SetButtonText");
				}

				strcpy(button->button_text, text);
			}
			else
			{
				button->button_text = memory_Strdup(text, "gui_SetButtonText");
			}

			button->widget.bm_flags |= WIDGET_RENDER_TEXT;
		}
	}
}

void gui_UpdateButton(widget_t *widget)
{
	button_t *button = (button_t *)widget;


	if(button->bm_button_flags & BUTTON_DONT_RECEIVE_CLICK)
	{
		widget->bm_flags &= ~(WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON | WIDGET_HAS_LEFT_MOUSE_BUTTON | WIDGET_JUST_RECEIVED_LEFT_MOUSE_DOUBLE_CLICK | WIDGET_MOUSE_OVER);
	}

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

#ifdef __cplusplus
}
#endif




