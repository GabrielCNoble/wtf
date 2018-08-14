#include <stdio.h>
#include <signal.h>


#include "engine.h"
#include "memory.h"
#include "SDL2\SDL_timer.h"

#include <float.h>
#include <malloc.h>


int command_line_arg_count;
char **command_line_args;

int engine_state;

unsigned long long int performance_frequency;

float delta_time;
unsigned long long start_delta;
unsigned long long end_delta;

void (*engine_StartUp)(int, char *[]);
void (*engine_GameMain)(float );
void (*engine_Shutdown)();

int b_init_properly = 0;

#define FPS_COLLECTION_TIME 1000.0

float accum_frame_time = 0.0;
float collection_delta = 0.0;
int collection_frame_count = 0;
float fps = 0.0;


#ifdef __cplusplus
extern "C"
{
#endif

void engine_SigSegHandler(int signal)
{
	log_LogMessage(LOG_MESSAGE_ERROR, "engine_SigSegHandler: Forced log flush due a segmentation fault!");
	//engine_BackTrace();
	log_Finish();
	exit(-1);
}

void engine_Init(int width, int height, int init_mode, int argc, char *argv[])
{
	mat3_t r;
	int camera_index;
	int *i = 0;


	/*i = malloc(sizeof(int));

	_HEAPINFO heap_info;
	heap_info._pentry = -1;
	heap_info._size = 0;
	heap_info._useflag = 0;

	printf("%d\n", _heapwalk(&heap_info));*/


	signal(SIGSEGV, engine_SigSegHandler);

	//*i = 5;


	log_Init();
	log_LogMessage(LOG_MESSAGE_NOTIFY, "ENGINE START");


	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		log_LogMessage(LOG_MESSAGE_ERROR, "engine_Init: SDL didn't init!");
		return;
	}

	memory_Init(0);

	path_Init(argv[0]);
	//path_AddSearchPath("fonts");

	b_init_properly = 1;

	b_init_properly &= resource_Init();
	b_init_properly &= renderer_Init(width, height, init_mode);
	b_init_properly &= shader_Init();
	b_init_properly &= script_Init();
	b_init_properly &= input_Init();
	b_init_properly &= gpu_Init();
	b_init_properly &= model_Init();
	b_init_properly &= particle_Init();
	b_init_properly &= entity_Init();
	b_init_properly &= light_Init();
	b_init_properly &= event_Init();
	b_init_properly &= world_Init();
	b_init_properly &= camera_Init();
	b_init_properly &= sound_Init();
	//b_init_properly &= player_Init();
	b_init_properly &= gui_Init();
	b_init_properly &= physics_Init();
	b_init_properly &= material_Init();
	b_init_properly &= texture_Init();
	b_init_properly &= font_Init();
	b_init_properly &= navigation_Init();
	b_init_properly &= bsp_Init();
	//b_init_properly &= portal_Init();
	memory_CheckCorrupted();

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

	command_line_arg_count = argc;
	command_line_args = argv;


}

void engine_Finish()
{
	//memory_Report();
	if(b_init_properly)
	{

		memory_CheckCorrupted();
		//portal_Finish();
		shader_Finish();
		navigation_Finish();
		material_Finish();
		input_Finish();
		particle_Finish();
		model_Finish();
		world_Finish();
		event_Finish();
		camera_Finish();
		light_Finish();
		texture_Finish();
		sound_Finish();
		entity_Finish();
		gui_Finish();
		bsp_Finish();
		//player_Finish();
		script_Finish();
		font_Finish();
		physics_Finish();
		gpu_Finish();
		renderer_Finish();
		resource_Finish();
		log_LogMessage(LOG_MESSAGE_NOTIFY, "Massacre engine finished properly!");
	}

	path_Finish();

	memory_CheckCorrupted();
	memory_Report();
	memory_Finish();

	log_LogMessage(LOG_MESSAGE_NOTIFY, "ENGINE FINISH");
	log_Finish();
	SDL_Quit();


}


float accum_cpu_time = 0.0;
float accum_gpu_time = 0.0;
float capture_time = 0.0;
int captured_frames = 0;


void engine_MainLoop()
{

	if(!b_init_properly)
		return;

	if(engine_StartUp)
	{
		engine_StartUp(command_line_arg_count, command_line_args);
	}

	memory_CheckCorrupted();

	float s;
	float e;

	engine_UpdateDeltaTime();

	while(engine_state)
	{

		engine_UpdateDeltaTime();

		s = engine_GetDeltaTime();
		renderer_StartGpuTimer();

		renderer_OpenFrame();
		input_GetInput(delta_time);
		//gui_ProcessGUI();

		gui_OpenGuiFrame();

		if(engine_GameMain)
		{
			engine_GameMain(delta_time);
		}

		/*gui_ImGuiSetNextWindowPos(vec2(0.0, 0.0), ImGuiCond_Once, vec2(0.0, 0.0));
		gui_ImGuiBegin("test", NULL, 0);
		gui_ImGuiEnd();*/


		if(engine_state & ENGINE_PLAYING)
		{
			entity_UpdateScriptComponents();
			world_ExecuteWorldScript();
			physics_ProcessCollisions(delta_time);
		}

        entity_UpdatePhysicsComponents();
		entity_UpdateTransformComponents();
		entity_UpdateCameraComponents();

		script_ExecuteScripts(delta_time);


		entity_ClearMarkedEntities();


		sound_ProcessSound();
		particle_UpdateParticleSystems(delta_time);

		world_MarkLightsOnLeaves();
		world_MarkEntitiesOnLeaves();

		world_VisibleWorld();
		world_VisibleEntities();
		world_VisibleLights();


		gui_CloseGuiFrame();

		renderer_DrawFrame();

		renderer_CloseFrame();
		e = engine_GetDeltaTime();


		accum_cpu_time += e - s;
		accum_gpu_time += renderer_StopGpuTimer();
		capture_time += delta_time;



		if(capture_time >= 1000.0)
		{
			accum_cpu_time /= captured_frames;
			accum_gpu_time /= captured_frames;
			printf("frame time - |gpu: %0.3f| |cpu: %0.3f| |fps(gpu): %0.3f| |fps(cpu): %0.3f|\n", accum_gpu_time, accum_cpu_time, 1.0 / (accum_gpu_time * 0.001), 1.0 / (accum_cpu_time * 0.001));

			accum_cpu_time = 0.0;
			accum_gpu_time = 0.0;
			capture_time = 0.0;
			captured_frames = 0;
		}
		else
		{
			captured_frames++;
		}

		engine_state &= ~(ENGINE_JUST_PAUSED | ENGINE_JUST_RESUMED);
	}


	if(engine_Shutdown)
	{
		engine_Shutdown();
	}
}

void engine_SetGameStartupFunction(void (*startup_fn)(int, char *[]))
{
	engine_StartUp = startup_fn;
}

void engine_SetGameMainFunction(void (*game_main_fn)(float ))
{
	//game_main = game_main_fn;
	engine_GameMain = game_main_fn;
}

void engine_SetGameShutdownFunction(void (*shutdown_fn)())
{
	engine_Shutdown = shutdown_fn;
}

void engine_ReadConfig()
{

}

void engine_WriteConfig()
{

}

void engine_SetEngineState(int state)
{
	/* once the engine is set to quit, nothing should stop it... */
	if(engine_state == ENGINE_QUIT)
		return;

	switch(state)
	{
		case ENGINE_QUIT:
			engine_state = ENGINE_QUIT;
		break;

		case ENGINE_PAUSED:
			if(engine_state & ENGINE_PLAYING)
			{
				engine_state = ENGINE_PAUSED | ENGINE_JUST_PAUSED;
			}
		break;

		case ENGINE_PLAYING:
			if(engine_state & ENGINE_PAUSED)
			{
				engine_state = ENGINE_PLAYING | ENGINE_JUST_RESUMED;
			}
		break;
	}
}

int engine_GetEngineState()
{
	return engine_state;
}

void engine_UpdateDeltaTime()
{
	end_delta = SDL_GetPerformanceCounter();
	delta_time = (float)((float)(end_delta - start_delta) * 1000) / performance_frequency;
	start_delta = end_delta;
}

float engine_GetDeltaTime()
{
	unsigned long long cur = SDL_GetPerformanceCounter();
	return (float)((cur - start_delta) * 1000) / (float)performance_frequency;
}


#ifdef __cplusplus
}
#endif










