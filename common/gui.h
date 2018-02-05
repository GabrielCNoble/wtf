#ifndef GUI_H
#define GUI_H

#include "gui_common.h"
#include "gui_dropdown.h"
#include "gui_option_list.h"
#include "gui_option.h"
#include "gui_button.h"
#include "gui_widget_bar.h"
#include "gui_checkbox.h"
#include "gui_text_field.h"
#include "gui_slider.h"

int gui_Init();

void gui_Finish();

widget_t *gui_CreateWidget(char *name, short x, short y, short w, short h);

void gui_NestleWidget(widget_t *parent, widget_t *widget);

void gui_DestroyWidget(widget_t *widget);

void gui_GetAbsolutePosition(widget_t *widget, short *x, short *y);

void gui_SetAsTop(widget_t *widget);

void gui_SetVisible(widget_t *widget);

void gui_SetInvisible(widget_t *widget);

void gui_RenderText(widget_t *widget);

gui_var_t *gui_CreateVar(char *name, short type, void *addr);

void gui_TrackVar(gui_var_t *var, widget_t *widget);

void gui_DeleteVar(gui_var_t *var);

void gui_UpdateVars();

void gui_ProcessGUI();

void gui_UpdateGUIProjectionMatrix();

gui_var_t gui_MakeUnsignedCharVar(unsigned char value);

gui_var_t gui_MakeIntVar(int value);

gui_var_t gui_MakeFloatVar(float value);

gui_var_t gui_MakeDoubleVar(double value);

gui_var_t gui_MakeVec2Var(vec2_t value);

gui_var_t gui_MakeVec3Var(vec3_t value);


#endif








