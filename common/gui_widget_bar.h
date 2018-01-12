#ifndef GUI_WIDGET_BAR_H
#define GUI_WIDGET_BAR_H

#include "gui_common.h"

widget_bar_t *gui_AddWidgetBar(widget_t *widget, char *name, short x, short y, short w, short h, short bm_flags);

void gui_AddWidgetToBar(widget_t *widget, widget_bar_t *bar);

void gui_AdjustBar(widget_t *widget);

void gui_UpdateWidgetBar(widget_t *widget);


#endif
