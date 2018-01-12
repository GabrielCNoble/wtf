#ifndef GUI_CHECKBOX_H
#define GUI_CHECKBOX_H

#include "gui_common.h"

checkbox_t *gui_AddCheckbox(widget_t *widget, short x, short y, short w, short h, short bm_flags, void (*checkbox_callback)(widget_t *widget));

void gui_UpdateCheckbox(widget_t *widget);



#endif