#ifndef ENGINE_H
#define ENGINE_H

#include "r_main.h"
#include "path.h"
#include "shader.h"
#include "input.h"
#include "model.h"
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
#include "log.h"
#include "entity.h"
#include "portal.h"
#include "particle.h"
#include "script.h"
#include "navigation.h"
#include "event.h"
#include "resource.h"



//#define engine_PrintError(__VA_ARGS__) printf("%s:", __func__, __VA_ARGS__);




enum ENGINE_STATE
{
	ENGINE_QUIT = 0,
	ENGINE_PAUSED = 1,
	ENGINE_PLAYING = 1 << 1,
	ENGINE_EDITING = 1 << 2,
	ENGINE_JUST_PAUSED = 1 << 3,
	ENGINE_JUST_RESUMED = 1 << 4,
};



#ifdef __cplusplus
extern "C"
{
#endif

void engine_Init(int width, int height, int init_mode, int argc, char *argv[]);

void engine_Finish();

void engine_MainLoop();

void engine_SetGameStartupFunction(void (*startup_fn)(int, char *[]));

void engine_SetGameMainFunction(void (*game_main_fn)(float ));

void engine_SetGameShutdownFunction(void (*shutdown_fn)());

void engine_ReadConfig();

void engine_WriteConfig();

void engine_SetEngineState(int state);

int engine_GetEngineState();

void engine_UpdateDeltaTime();

float engine_GetCurrentDeltaTime();

float engine_GetDeltaTime();

void engine_BackTrace();

void engine_BreakPoint();

#ifdef __cplusplus
}
#endif


#endif
