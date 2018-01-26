#ifndef GUI_TEXT_FIELD
#define GUI_TEXT_FIELD

#include "gui_common.h"



typedef struct
{
	widget_t widget;
	short text_buffer_size;
	short text_cursor;
	short text_buffer_cursor;
	char *text;
	SDL_Surface *rendered_text;
	short cursor_blink_timer;
	short bm_text_field_flags;
}text_field_t;


text_field_t *gui_AddTextField(widget_t *widget, char *name, short x, short y, short w, short bm_flags, void (*text_field_callback)(widget_t *));

void gui_SetText(widget_t *text_field, char *text);

void gui_UpdateTextField(widget_t *widget);

void gui_PostUpdateTextField(widget_t *widget);



#endif

