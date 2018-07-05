#ifndef GUI_DROPDOWN_H
#define GUI_DROPDOWN_H

#include "gui_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

dropdown_t *gui_CreateDropdown(char *name, char *text, short x, short y, short w, short bm_flags, void (*dropdown_callback)(widget_t *));

dropdown_t *gui_AddDropdown(widget_t *widget, char *name, char *text, short x, short y, short w, short bm_flags, void (*dropdown_callback)(widget_t *));

void gui_RemoveDropdown(dropdown_t *dropdown, int free_dropdown);

option_t *gui_AddOption(dropdown_t *dropdown, char *name, char *text);

void gui_UpdateDropdown(widget_t *widget);

void gui_PostUpdateDropdown(widget_t *widget);

void gui_UpdateDropdownBar(widget_t *bar);

#ifdef __cplusplus
}
#endif

#endif
