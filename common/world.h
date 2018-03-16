#ifndef WORLD_H
#define WORLD_H

#include "w_common.h"
#include "bsp_common.h"

int world_Init();

void world_Finish();

int world_LoadBsp(char *file_name);

void world_VisibleLeaves();

void world_VisibleWorld();

void world_Update();




#endif
