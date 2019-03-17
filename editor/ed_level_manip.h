#ifndef ED_LEVEL_MANIP_H
#define ED_LEVEL_MANIP_H




#include "ed_level_common.h"
#include "brush.h"




void editor_LevelEditorTransformOp(struct selection_state_t *selection_state, int op);

void editor_LevelEditorTransformFrame(struct selection_state_t *selection_state, int frame);

void editor_LevelEditorTransformPivot(struct selection_state_t *selection_state, int pivot);

void editor_LevelEditorTransformAxis(struct selection_state_t *selection_state, int axis_constraint);

vec3_t editor_LevelEditorGetCenterOfSelections(struct selection_state_t *selection_state);

vec3_t editor_LevelEditorGetSelectionPosition(struct pick_record_t *record);

void editor_LevelEditorCalculateMouseDelta(struct selection_state_t *selection_state);





void editor_LevelEditorTranslateSelections(struct selection_state_t *selection_state, vec3_t translation);

void editor_LevelEditorRotateSelections(struct selection_state_t *selection_state, vec3_t axis, float angle);

void editor_LevelEditorScaleSelections(struct selection_state_t *selection_state, vec3_t axis, float scaling);

void editor_LevelEditorCopySelections(struct selection_state_t *selection_state);



void editor_LevelEditorTranslateBrush(struct pick_record_t *pick_record, vec3_t translation);

void editor_LevelEditorRotateBrush(struct pick_record_t *pick_record, vec3_t axis, float angle);

void editor_LevelEditorScaleBrush(struct pick_record_t *pick_record_t, vec3_t axis, float scaling);



void editor_LevelEditorTranslateLight(struct pick_record_t *pick_record, vec3_t translation);

void editor_LevelEditorRotateLight(struct pick_record_t *pick_record, vec3_t axis, float angle);












#endif // MANIPULATION
