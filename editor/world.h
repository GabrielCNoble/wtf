#ifndef WORLD_H
#define WORLD_H

#include "w_common.h"
#include "bsp_common.h"

void world_Init();

void world_Finish();

void world_LoadWorldModel(char *file_name);

void world_LoadBsp(char *file_name);

void world_BuildBatches();

void world_VisibleLeaves();

void world_VisibleWorld();

void world_Update();



void world_Move(vec3_t *position, vec3_t *velocity);

void world_TryStepUp(vec3_t *position, vec3_t *velocity, trace_t *trace);

void world_TryStepDown(vec3_t *position, vec3_t *velocity, trace_t *trace);




#endif
