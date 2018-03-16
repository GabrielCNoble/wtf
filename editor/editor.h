#ifndef EDITOR_H
#define EDITOR_H

#include "ed_common.h"
#include "ed_in.h"
#include "brush.h"


void editor_Init();

void editor_RestartEditor();

void editor_Finish();

void editor_Main(float delta_time);

/*void editor_Input(float delta_time);

void editor_ProcessMouse(float delta_time);

void editor_ProcessKeyboard(float delta_time);*/

void editor_AddToWorld(int type, vec3_t position, mat3_t *orientation);

void editor_EnablePicking();

void editor_DisablePicking();

int editor_PickObject();

int editor_PickOnBrush(brush_t *brush);

int editor_Check3dHandle();

void editor_Set3dHandleMode(int mode);

void editor_Set3dHandlePivotMode(int mode);

void editor_SetEditingMode(int mode);

void editor_ToggleBrushEditing();

void editor_Position3dCursor();

void editor_Position3dHandle();




void editor_AddSelection(pick_record_t *record);

void editor_DropSelection(pick_record_t *record);

void editor_ClearSelection();

void editor_TranslateSelections(vec3_t direction, float amount);

void editor_RotateSelections(vec3_t axis, float amount);

void editor_ScaleSelections(vec3_t axis, float amount);

void editor_CopySelections();

void editor_DeleteSelection();


//void editor_ExportMap(char *file_name);

void editor_StartPIE();

void editor_StopPIE();

void editor_WindowResizeCallback();



#endif 








