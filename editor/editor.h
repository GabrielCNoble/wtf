#ifndef EDITOR_H
#define EDITOR_H

#include "ed_common.h"
#include "ed_in.h"


void editor_Init();

void editor_Finish();

void editor_Main(float delta_time);

/*void editor_Input(float delta_time);

void editor_ProcessMouse(float delta_time);

void editor_ProcessKeyboard(float delta_time);*/

void editor_TranslateSelections(vec3_t direction, float amount);

void editor_RotateSelections(vec3_t axis, float amount, int individual_origins);

void editor_DeleteSelections();

void editor_AddToWorld(int type, vec3_t position, mat3_t *orientation);

void editor_EnablePicking();

void editor_DisablePicking();

int editor_Pick(pick_record_t *record);

int editor_Check3dHandle();

void editor_Position3dCursor();

void editor_AddSelection(pick_record_t *record);

void editor_DropSelection(pick_record_t *record);

void editor_ClearSelection();

void editor_ExportMap(char *file_name);

void editor_WindowResizeCallback();


#endif 
