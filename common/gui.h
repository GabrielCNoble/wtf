#ifndef GUI_H
#define GUI_H

#include "gui_common.h"
#include "gui_imgui.h"

#ifdef __cplusplus
extern "C"
{
#endif

int gui_Init();

void gui_Finish();

void gui_OpenGuiFrame();

void gui_CloseGuiFrame();

void gui_DrawGUI();




#ifdef __cplusplus
}
#endif


#endif








