#ifndef ED_SELECTION_H
#define ED_SELECTION_H

#include "vector.h"
#include "ed_common.h"
#include "brush.h"

void editor_EnablePicking();

void editor_SamplePickingBuffer(float mouse_x, float mouse_y, int *sample);

void editor_DisablePicking();

/*
===============================================================
===============================================================
===============================================================
*/

pick_record_t editor_PickObject(float mouse_x, float mouse_y);

pick_record_t editor_PickBrushFace(brush_t *brush, float mouse_x, float mouse_y);

/*
===============================================================
===============================================================
===============================================================
*/

int editor_Check3dHandle(double mouse_x, double mouse_y, vec3_t handle_position, int mode);

vec3_t editor_3dCursorPosition(float mouse_x, float mouse_y);

float editor_GetMouseOffsetFrom3dHandle(float mouse_x, float mouse_y, vec3_t handle_position, vec3_t axis, int transform_mode, float linear_snap_value, float angular_snap_value);


/*
===============================================================
===============================================================
===============================================================
*/


void editor_AddSelection(pick_record_t *record, pick_list_t *pick_list);

void editor_DropSelection(pick_record_t *record, pick_list_t *pick_list);

void editor_ClearSelection(pick_list_t *pick_list);

pick_record_t editor_GetLastSelection();

mat3_t editor_GetLastSelectionOrientation();

vec3_t editor_GetLastSelectionPosition();

/*
===============================================================
===============================================================
===============================================================
*/

void editor_TranslateSelections(pick_list_t *pick_list, vec3_t direction, float amount);

void editor_RotateSelections(pick_list_t *pick_list, vec3_t axis, float amount, vec3_t handle_position);

void editor_ScaleSelections(pick_list_t *pick_list, vec3_t axis, float amount);

void editor_CopySelections(pick_list_t *pick_list);

void editor_DestroySelection(pick_list_t *pick_list);

#endif
