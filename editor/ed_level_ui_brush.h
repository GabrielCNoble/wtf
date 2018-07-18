#ifndef ED_UI_BRUSH_H
#define ED_UI_BRUSH_H

#include "gui_common.h"

void editor_InitBrushUI();

void editor_OpenBrushPropertiesWindow();

void editor_CloseBrushPropertiesWindow();

void editor_OpenBrushFacePropertiesWindow(int brush_index, int face_index);

void editor_CloseBrushFacePropertiesWindow();

void editor_OpenBrushFaceUVWindow();

void editor_CloseBrushFaceUVWindow();




#endif
