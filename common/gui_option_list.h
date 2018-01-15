#ifndef GUI_OPTION_LIST_H
#define GUI_OPTION_LIST_H

#include "gui_common.h"

option_list_t *gui_CreateOptionList(char *name, short x, short y, short w, short bm_flags, void (*option_list_callback)(widget_t *));

void gui_AddOptionToList(option_list_t *option_list, char *name, char *text);

void gui_NestleOption(option_list_t *option_list, int option_index, char *name, char *text);

void gui_UpdateOptionList(widget_t *widget);


#endif
