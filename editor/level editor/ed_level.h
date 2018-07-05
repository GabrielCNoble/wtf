#ifndef ED_LEVEL_H
#define ED_LEVEL_H

#include "..\ed_common.h"
#include "..\brush.h"



void editor_LevelEditorInit();

void editor_LevelEditorFinish();

void editor_LevelEditorSetup();

void editor_LevelEditorShutdown();

void editor_LevelEditorRestart();

void editor_LevelEditorMain(float delta_time);


pick_record_t editor_LevelEditorPickObject(float mouse_x, float mouse_y);

int editor_LevelEditorPickBrushFace(brush_t *brush, float mouse_x, float mouse_y);

/*
====================================================================
====================================================================
====================================================================
*/
  
void editor_LevelEditorCheck3dHandle(float mouse_x, float mouse_y);

void editor_LevelEditorSet3dCursorPosition(float mouse_x, float mouse_y);

void editor_LevelEditorUpdate3dHandlePosition();

void editor_LevelEditorSet3dHandleTransformMode(int mode);

void editor_LevelEditorSetEditingMode(int mode);

/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorEdit(float delta_time);

void editor_LevelEditorFly(float delta_time);

/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorAddSelection(pick_record_t *record);

void editor_LevelEditorDropSelection(pick_record_t *record);

void editor_LevelEditorClearSelections();

void editor_LevelEditorCopySelections();

void editor_LevelEditorDestroySelections();

pick_record_t editor_LevelEditorGetLastSelection();

/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorTranslateSelections(vec3_t direction, float amount);

void editor_LevelEditorRotateSelections(vec3_t axis, float amount);

void editor_LevelEditorScaleSelections(vec3_t axis, float amount);

/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorStartPIE();

void editor_LevelEditorStopPIE();

/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorCopyLevelData();

void editor_LevelEditorClearLevel();

void editor_LevelEditorRestoreLevelData();

/*
====================================================================
====================================================================
====================================================================
*/






#endif






