#include <stdio.h>

#include "engine.h"
#include "SDL2\SDL_timer.h"

#include <float.h>


int engine_state;

unsigned long long int performance_frequency;

float delta_time;
unsigned long long start_delta;
unsigned long long end_delta;

void (*engine_StartUp)();

void (*engine_GameMain)(float );


int b_init_properly = 0;

void engine_Init(int width, int height, int init_mode)
{
	mat3_t r;
	int camera_index;
	
	log_Init("log");
	
	log_LogMessage(LOG_MESSAGE_NOTIFY, "ENGINE START");
	
	if(renderer_Init(width, height, init_mode) &&
	   shader_Init("shaders") &&
	   input_Init() &&
	   gpu_Init() &&
	   mesh_Init() &&
	   light_Init() &&
	   world_Init() &&
	   camera_Init() &&
	   player_Init() &&
	   sound_Init() &&
	   gui_Init() && 
	   physics_Init() &&
	   material_Init() &&
	   texture_Init() && 
	   font_Init() &&
	   bsp_Init()
	   )
	{
		b_init_properly = 1;
	}
	
	
	if(b_init_properly)
	{		
		engine_state = ENGINE_PLAYING;
		
		performance_frequency = SDL_GetPerformanceFrequency();
		
		start_delta = 0;
		end_delta = 0;
		delta_time = 0.0;
		
		log_LogMessage(LOG_MESSAGE_NOTIFY, "Massacre engine started properly!");
	}
	else
	{
		log_LogMessage(LOG_MESSAGE_NOTIFY, "Massacre engine has found problems during initialization...");
	}
	
	
}

void engine_Finish()
{
	if(b_init_properly)
	{
		editor_Finish();
		shader_Finish();
		material_Finish();
		input_Finish();
		mesh_Finish();
		world_Finish();
		camera_Finish();
		light_Finish();
		texture_Finish();
		sound_Finish();
		//collision_Finish();
		gui_Finish();
		bsp_Finish();
		player_Finish();
		font_Finish();
		//projectile_Finish();
		physics_Finish();
		brush_Finish();
		gpu_Finish();
		renderer_Finish();
		log_LogMessage(LOG_MESSAGE_NOTIFY, "Massacre engine finished properly!");
	}


	log_LogMessage(LOG_MESSAGE_NOTIFY, "ENGINE FINISH");
	
	log_Finish();
}

void engine_MainLoop()
{
	
	if(!b_init_properly)
		return;
	
	if(engine_StartUp)
	{
		engine_StartUp();
	}
	
	float s;
	float e;
	
	while(engine_state)
	{
		engine_UpdateDeltaTime();
		
		renderer_OpenFrame();
		input_GetInput();
		gui_ProcessGUI();
		engine_GameMain(delta_time);
		

		if(engine_state == ENGINE_PLAYING)
		{
			player_ProcessActivePlayer(delta_time);
			player_ProcessAI(delta_time);
			player_UpdatePlayers(delta_time);
			physics_ProcessCollisions(*(float *)&delta_time);
			//projectile_UpdateProjectiles();
		}
		
		
		sound_ProcessSound();
		//s = engine_GetDeltaTime();
		light_UpdateLights();	
		world_VisibleWorld();
		light_VisibleLights();
		
		//e = engine_GetDeltaTime();
		
		
		//s = engine_GetDeltaTime();
		renderer_DrawFrame();
		//e = engine_GetDeltaTime();
		renderer_CloseFrame();
		
		
		
		//printf("CPU: %f\n", e - s);
	}
}

void engine_SetGameStartupFunction(void (*startup_fn)(void))
{
	engine_StartUp = startup_fn;
}

void engine_SetGameMainFunction(void (*game_main_fn)(float ))
{
	//game_main = game_main_fn;
	engine_GameMain = game_main_fn;
}

void engine_SetEngineState(int state)
{
	switch(state)
	{
		case ENGINE_QUIT:
		case ENGINE_PAUSED:
		case ENGINE_PLAYING:
		case ENGINE_EDITING:
			
			
			/* once the engine is set to quit, nothing should stop it... */
			//if(state == ENGINE_QUIT && engine_state == ENGINE_QUIT)
			//	return;
			
			if(engine_state == ENGINE_QUIT)
				return;	
				
			/*if(engine_state == ENGINE_PLAYING && state == ENGINE_PAUSED)
			{
				engine_state |= ENGINE_JUST_PAUSED;
			}
			else if(engine_state == ENGINE_PAUSED && state == ENGINE_PLAYING)
			{
				engine_state |= ENGINE_JUST_RESUMED;
			}*/
			engine_state = state;
		break;
	}
}

void engine_UpdateDeltaTime()
{
	end_delta = SDL_GetPerformanceCounter();
	delta_time = (float)((end_delta - start_delta) * 1000) / performance_frequency;
	start_delta = end_delta;
}

float engine_GetDeltaTime()
{
	unsigned long long cur = SDL_GetPerformanceCounter();
	return (float)((cur - start_delta) * 1000) / (float)performance_frequency;
}















