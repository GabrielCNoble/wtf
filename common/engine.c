#include <stdio.h>
#include <signal.h>


#include "engine.h"
#include "c_memory.h"
#include "SDL2\SDL_timer.h"

#include <float.h>
#include <malloc.h>
#include <stdarg.h>

struct subsystem_t
{
    char subsystem_name[128];
    void (*subsystem_init_func)();
    void (*subsystem_shutdown_func)();
};


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

FILE *log_file;

#define MAX_ERRORS 1024
#define MAX_ERROR_STRING_LEN 1024

int e_error_stack_top = -1;
char e_error_stack[MAX_ERRORS][MAX_ERROR_STRING_LEN];


#ifdef __cplusplus
extern "C"
{
#endif

void engine_SigSegHandler(int signal)
{
	log_LogMessage(LOG_MESSAGE_ERROR, 1, "engine_SigSegHandler: Forced log flush due a segmentation fault!");
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

	b_init_properly = 1;

	signal(SIGSEGV, engine_SigSegHandler);

	//*i = 5;


	log_Init();
	//log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "LOG START");


	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		log_LogMessage(LOG_MESSAGE_ERROR, 1, "engine_Init: SDL didn't init!\nError cause:\n%s\n", SDL_GetError());
		b_init_properly = 0;
	}

	memory_Init();
	path_Init();
	shader_Init();
	renderer_InitGL();
	//path_AddSearchPath("fonts");

	if(b_init_properly)
	{
	    b_init_properly &= script_Init();
		b_init_properly &= resource_Init();
		b_init_properly &= renderer_Init(width, height, init_mode);

		if(b_init_properly)
		{
			//b_init_properly &= shader_Init();
			b_init_properly &= input_Init();
			b_init_properly &= animation_Init();
			//b_init_properly &= gpu_Init();
			b_init_properly &= model_Init();
			b_init_properly &= particle_Init();
			b_init_properly &= entity_Init();
			b_init_properly &= light_Init();
			b_init_properly &= event_Init();
			b_init_properly &= world_Init();
			b_init_properly &= sound_Init();
			b_init_properly &= gui_Init();
			b_init_properly &= physics_Init();
			b_init_properly &= material_Init();
			b_init_properly &= texture_Init();
			b_init_properly &= navigation_Init();
			b_init_properly &= bsp_Init();
		}
	}

	memory_CheckCorrupted();

	if(b_init_properly)
	{
		engine_state = ENGINE_PLAYING;

		performance_frequency = SDL_GetPerformanceFrequency();

		start_delta = 0;
		end_delta = 0;
		delta_time = 0.0;

		log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "Mayhem engine started properly!");
	}
	else
	{
		log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "Mayhem engine has found problems during initialization...");
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
		shader_Finish();
		navigation_Finish();
		animation_Finish();
		material_Finish();
		input_Finish();
		particle_Finish();
		model_Finish();
		world_Finish();
		event_Finish();
		light_Finish();
		texture_Finish();
		sound_Finish();
		entity_Finish();
		gui_Finish();
		bsp_Finish();
		script_Finish();
		physics_Finish();
		//gpu_Finish();
		renderer_Finish();
		resource_Finish();
		log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "Mayhem engine finished properly!");
	}

	path_Finish();

	memory_CheckCorrupted();
	memory_Report(0);
	memory_Finish();

	//log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "LOG END");
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
	int cpu_timer;

	engine_UpdateDeltaTime();

	while(engine_state)
	{

		engine_UpdateDeltaTime();

		//printf("a: %llu\n", start_delta);

		//printf("%f %llu\n", delta_time, end_delta - start_delta);

		//s = engine_GetDeltaTime();
		//renderer_StartGpuTimer();


		input_GetInput(delta_time);

		//printf("b: %llu\n", start_delta);
		//gui_ProcessGUI();

		gui_OpenGuiFrame();

		//printf("c: %llu\n", start_delta);

		//int cpu_timer = renderer_StartCpuTimer("engine_MainLoop");
        //cpu_timer = renderer_StartCpuTimer("engine_GameMain");
		if(engine_GameMain)
		{
			engine_GameMain(delta_time);
		}

		//printf("d: %llu\n", start_delta);

		if(engine_state & ENGINE_PLAYING)
		{
			entity_UpdateTriggers();
			entity_UpdateScriptComponents();
			animation_UpdateAnimations(delta_time);
			world_ExecuteWorldScript();
			physics_ProcessCollisions(delta_time);
		}


        //printf("e: %llu\n", start_delta);
		//entity_RemoveMarkedEntities(1);

        entity_UpdatePhysicsComponents();
		entity_UpdateTransformComponents();
		entity_UpdateCameraComponents();

		//printf("f: %llu\n", start_delta);

		sound_ProcessSound();
		particle_UpdateParticleSystems(delta_time);

		//printf("g: %llu\n", start_delta);

		world_MarkLightsOnLeaves();
		world_MarkEntitiesOnLeaves();

		//printf("h: %llu\n", start_delta);

		renderer_DrawFrame();

		//printf("i: %llu\n", start_delta);

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
	performance_frequency = SDL_GetPerformanceFrequency();
	delta_time = (float)((float)(end_delta - start_delta) * 1000) / performance_frequency;
	start_delta = end_delta;
}

float engine_GetCurrentDeltaTime()
{
	unsigned long long cur = SDL_GetPerformanceCounter();
	return (float)((cur - start_delta) * 1000) / (float)performance_frequency;
}

float engine_GetDeltaTime()
{
	return delta_time;
}


#define MAX_FUNCS 8192


const int addr_name_count = 0;
const void *addr_name[MAX_FUNCS][2];


void engine_CaptureFunction(const char *name, const void *addr)
{
	if(addr_name_count < MAX_FUNCS)
	{
        addr_name[addr_name_count][0] = addr;
		addr_name[addr_name_count][1] = name;
	}
}

void engine_ErrorCaller(const char *caller, const char *format, ...)
{
	/*va_list args;
	va_start(args, format);

	e_error_stack_top++;

	vsprintf(e_error_stack[e_error_stack_top], format, args);
	log_LogMessage(LOG_MESSAGE_ERROR, "%s: %s", caller, e_error_stack[e_error_stack_top]);*/
}

const char *engine_GetError()
{
    /*if(e_error_stack_top == -1)
	{
		return NULL;
	}

    e_error_stack_top--;
    return e_error_stack[e_error_stack_top + 1];*/
}

#ifdef __cplusplus
}
#endif










