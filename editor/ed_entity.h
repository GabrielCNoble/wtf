#ifndef ED_ENTITY_H
#define ED_ENTITY_H

#include "..\ed_common.h"
#include "..\common\ent_common.h"

void editor_EntityEditorInit();

void editor_EntityEditorFinish();

void editor_EntityEditorSetup();

void editor_EntityEditorShutdown();

void editor_EntityEditorRestart();

void editor_EntityEditorMain(float delta_time);

/*
===============================================================
===============================================================
===============================================================
*/

void editor_EntityEditorSetCurrentEntityDef(struct entity_handle_t entity_def);

/*
===============================================================
===============================================================
===============================================================
*/


void editor_EntityEditorCheck3dHandle(float mouse_x, float mouse_y);

void editor_EntityEditorSet3dCursorPosition(float mouse_x, float mouse_y);

void editor_EntityEditorUpdate3dHandlePosition();

void editor_EntityEditorSet3dHandleTransformMode(int mode);

/*
===============================================================
===============================================================
===============================================================
*/

void editor_EntityEditorMoveCamera();

void editor_EntityEditorUpdateCamera();

void editor_EntityEditorUpdatePreviewEntity();

void editor_EntityEditorEdit();

/*
===============================================================
===============================================================
===============================================================
*/

//pick_record_t editor_EntityEditorPickColliderPrimitive(float mouse_x, float mouse_y);

//void editor_EntityEditorAddSelection(pick_record_t *record);
//
//void editor_EntityEditorDropSelection(pick_record_t *record);
//
//void editor_EntityEditorClearSelections();
//
//void editor_EntityEditorCopySelections();
//
//void editor_EntityEditorDestroySelections();
//
//void editor_EntityEditorTranslateSelections(vec3_t direction, float amount);
//
//void editor_EntityEditorRotateSelections(vec3_t axis, float amount);
//
//void editor_EntityEditorScaleSelections(vec3_t axis, float amount);


/*
===============================================================
===============================================================
===============================================================
*/


int editor_EntityEditorLoadEntityFileCallback(char *path, char *file_name);

int editor_EntityEditorSaveEntityFileCallback(char *file_path, char *file_name, void **out_buffer, int *out_buffer_size);


#endif
