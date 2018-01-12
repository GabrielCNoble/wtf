#ifndef GUI_H
#define GUI_H

#include "gui_common.h"
#include "gui_dropdown.h"
#include "gui_option_list.h"
#include "gui_option.h"
#include "gui_button.h"
#include "gui_widget_bar.h"
#include "gui_checkbox.h"

void gui_Init();

void gui_Finish();

widget_t *gui_CreateWidget(char *name, short x, short y, short w, short h);

void gui_SetAsTop(widget_t *widget);

void gui_RenderText(widget_t *widget);

void gui_ProcessGUI();

void gui_UpdateGUIProjectionMatrix();

#endif








