#ifndef GUI_DROPDOWN_H
#define GUI_DROPDOWN_H

#include "gui_common.h"

dropdown_t *gui_CreateDropdown(char *name, char *text, short x, short y, short w, short bm_flags, void (*dropdown_callback)(widget_t *widget));

dropdown_t *gui_AddDropdown(widget_t *widget, char *name, char *text, short x, short y, short w, short bm_flags, void (*dropdown_callback)(widget_t *widget));

void gui_AddOption(dropdown_t *dropdown, char *name, char *text);

void gui_UpdateDropDown(widget_t *widget);

void gui_DropDownBarUpdate(widget_t *bar);


#endif
