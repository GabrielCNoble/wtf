#ifndef ED_PICKING_H
#define ED_PICKING_H

#include "brush.h"
#include "ed_picklist.h"

void editor_EnablePicking();

void editor_SamplePickingBuffer(float mouse_x, float mouse_y, int *sample);

void editor_DisablePicking();

vec3_t editor_3DCursorPosition(float mouse_x, float mouse_y);

struct pick_record_t editor_PickObject(float mouse_x, float mouse_y);

void editor_DrawPickableBrush(struct brush_t *brush, int brush_id);

void editor_DrawPickableLight(int light_index);



#endif // ED_PICKING_H
