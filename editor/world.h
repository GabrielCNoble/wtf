#ifndef WORLD_H
#define WORLD_H

#include "w_common.h"

void world_Init();

void world_Finish();

void world_LoadWorldModel(char *file_name);

void world_LoadBsp(char *file_name);

void world_BuildBatches();

void world_VisibleLeaves();

void world_VisibleWorld();

void world_Update();




#endif
