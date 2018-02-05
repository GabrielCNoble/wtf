#ifndef GUI_SLIDER_H
#define GUI_SLIDER_H


#include "gui_common.h"





slider_t *gui_AddSlider(widget_t *widget, char *name, short x, short y, short w, short bm_flags, void (*slider_callback)(widget_t *), gui_var_t *var, gui_var_t max, gui_var_t min);

void gui_UpdateSlider(widget_t *widget);

#endif
