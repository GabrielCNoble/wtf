#ifndef ENGINE_H
#define ENGINE_H

#include "r_main.h"
#include "shader.h"
#include "input.h"
#include "mesh.h"
#include "camera.h"
#include "gpu.h"
//#include "player.h"
//#include "physics.h"
#include "material.h"
#include "l_main.h"
//#include "projectile.h"
#include "sound.h"
//#include "collision.h"
#include "gui.h"
#include "bsp.h"
#include "font.h"

enum ENGINE_STATE
{
	ENGINE_QUIT = 0,
	ENGINE_PAUSED = 1,
	ENGINE_PLAYING = 1 << 1,
	ENGINE_EDITING = 1 << 2,
	ENGINE_JUST_PAUSED = 1 << 3,
	ENGINE_JUST_RESUMED = 1 << 4
};



void engine_Init(int width, int height, int init_mode);

void engine_Finish();

void engine_MainLoop();

void engine_SetGameStartupFunction(void (*startup_fn)(void));

void engine_SetGameMainFunction(void (*game_main_fn)(float ));

void engine_SetEngineState(int state);

void engine_UpdateDeltaTime();

float engine_GetDeltaTime();




#endif 
