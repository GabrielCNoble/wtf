#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include "gui_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

button_t *gui_CreateButton(char *name, short x, short y, short w, short h, short bm_flags, void (*button_callback)(widget_t *widget));

button_t *gui_AddButton(widget_t *widget, char *name, short x, short y, short w, short h, short bm_flags, void (*button_callback)(widget_t *widget));

void gui_SetButtonText(button_t *button, char *text);

void gui_UpdateButton(widget_t *widget);

#ifdef __cplusplus
}
#endif

#endif
