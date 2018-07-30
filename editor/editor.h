#ifndef EDITOR_H
#define EDITOR_H

#include "ed_common.h"
#include "ed_in.h"
#include "brush.h"



void editor_Init();

void editor_RestartEditor();

void editor_Finish();

void editor_Main(float delta_time);


/*
===============================================================
===============================================================
===============================================================
*/


int editor_PickOnBrush(brush_t *brush);

void editor_Set3dHandleTransformMode(int mode);

void editor_Set3dHandlePivotMode(int mode);

void editor_SetEditingMode(int mode);

void editor_ToggleBrushEditing();


/*
===============================================================
===============================================================
===============================================================
*/

void editor_RegisterEditor(char *name, void (*init_callback)(), void (*finish_callback)(), void (*restart_callback)(), void (*setup_callback)(), void (*shutdown_callback)(), void (*main_callback)(float));

void editor_UnregisterEditor(char *name);

editor_t *editor_GetEditor(char *name);

void editor_InitializeEditors();

void editor_FinishEditors();

void editor_StartEditor(char *name);





#endif 








