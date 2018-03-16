#ifndef GUI_OPTION_H
#define GUI_OPTION_H


#include "gui_common.h"

option_t *gui_CreateOption(char *name, char *text);

void gui_UpdateOption(widget_t *widget);


#endif
