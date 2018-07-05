#ifndef GUI_OPTION_LIST_H
#define GUI_OPTION_LIST_H

#include "gui_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

option_list_t *gui_CreateOptionList(char *name, short x, short y, short w, short bm_flags, short max_visible_options, void (*option_list_callback)(widget_t *));

option_list_t *gui_AddOptionList(widget_t *widget, char *name, short x, short y, short w, short bm_flags, short max_visible_options, void (*option_list_callback)(widget_t *));

option_t *gui_AddOptionToList(option_list_t *option_list, char *name, char *text);

void gui_RemoveOptionFromList(option_list_t *option_list, int option_index);

void gui_RemoveAllOptions(option_list_t *option_list);

/*void gui_NestleOption(option_list_t *option_list, int option_index, char *name, char *text);*/

option_list_t *gui_NestleOptionList(option_list_t *option_list, int option_index, char *name);

option_list_t *gui_GetNestledOptionList(option_list_t *option_list, int option_index);

option_t *gui_GetOptionAt(option_list_t *option_list, int option_index);

void gui_InvalidOption(option_list_t *option_list, int option_index);

void gui_ValidOption(option_list_t *option_list, int option_index);

int gui_GetOptionUniqueIndex(option_list_t *option_list, int option_index);

void gui_UpdateOptionList(widget_t *widget);

void gui_PostUpdateOptionList(widget_t *widget);

#ifdef __cplusplus
}
#endif

#endif
