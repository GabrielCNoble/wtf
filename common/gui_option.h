#ifndef GUI_OPTION_H
#define GUI_OPTION_H


#include "gui_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

option_t *gui_CreateOption(char *name, char *text);

void gui_SetOptionText(option_t *option, char *text);

void gui_UpdateOption(widget_t *widget);

#ifdef __cplusplus
}
#endif

#endif
