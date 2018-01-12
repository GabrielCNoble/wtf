#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include "gui_common.h"

button_t *gui_CreateButton(char *name, short x, short y, short w, short h, short bm_flags, void (*button_callback)(widget_t *widget));

button_t *gui_AddButton(widget_t *widget, char *name, short x, short y, short w, short h, short bm_flags, void (*button_callback)(widget_t *widget));

void gui_UpdateButton(widget_t *widget);



#endif
