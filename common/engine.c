#include <stdio.h>
#include <signal.h>


#include "engine.h"
#include "memory.h"
#include "SDL2\SDL_timer.h"

#include <float.h>


int command_line_arg_count;
char **command_line_args;

int engine_state;

unsigned long long int performance_frequency;

float delta_time;
unsigned long long start_delta;
unsigned long long end_delta;

void (*engine_StartUp)(int, char *[]);

void (*engine_GameMain)(float );

int b_init_properly = 0;

#define FPS_COLLECTION_TIME 1000.0

float accum_frame_time = 0.0;
float collection_delta = 0.0;
int collection_frame_count = 0;
float fps = 0.0;


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
	
	signal(SIGSEGV, engine_SigSegHandler);
	
	//*i = 5;
	
	log_Init();
	
	log_LogMessage(LOG_MESSAGE_NOTIFY, "ENGINE START");
	
	path_Init(argv[0]);
	path_AddSearchPath("shaders", SEARCH_PATH_SHADER);
	path_AddSearchPath("fonts", SEARCH_PATH_FONT);
	
	b_init_properly = 1;
	
	//if(renderer_Init(width, height, init_mode))
	//printf("start\n");
	b_init_properly &= renderer_Init(width, height, init_mode);
	//printf("a\n");
	b_init_properly &= shader_Init();
	//printf("b\n");
	b_init_properly &= input_Init();
	//printf("c\n");
	b_init_properly &= gpu_Init();
	//printf("d\n");
	b_init_properly &= model_Init();
	//printf("e\n");
	b_init_properly &= entity_Init();
	//printf("f\n");
	b_init_properly &= light_Init();
	//printf("g\n");
	b_init_properly &= world_Init();
	//printf("h\n");
	b_init_properly &= camera_Init();
	//printf("i\n");
	b_init_properly &= sound_Init();
	//printf("j\n");
	b_init_properly &= player_Init();
	//printf("k\n");
	b_init_properly &= gui_Init();
	//printf("l\n");
	b_init_properly &= physics_Init();
	//printf("m\n");
	b_init_properly &= material_Init();
	//printf("n\n");
	b_init_properly &= texture_Init();
	//printf("o\n");
	b_init_properly &= font_Init();
	//printf("p\n");
	b_init_properly &= bsp_Init();
	//printf("q\n");
	
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
	if(b_init_properly)
	{
		//editor_Finish();
		shader_Finish();
		material_Finish();
		input_Finish();
		model_Finish();
		world_Finish();
		camera_Finish();
		light_Finish();
		texture_Finish();
		sound_Finish();
		entity_Finish();
		gui_Finish();
		bsp_Finish();
		player_Finish();
		font_Finish();
		physics_Finish();
		gpu_Finish();
		renderer_Finish();
		log_LogMessage(LOG_MESSAGE_NOTIFY, "Massacre engine finished properly!");
	}

	path_Finish();
	
	memory_Report();
	log_LogMessage(LOG_MESSAGE_NOTIFY, "ENGINE FINISH");
	
	log_Finish();
	
	
}

void engine_MainLoop()
{
	
	if(!b_init_properly)
		return;

	if(engine_StartUp)
	{
		engine_StartUp(command_line_arg_count, command_line_args);
	}
	
	float s;
	float e;
	
	while(engine_state)
	{
		engine_UpdateDeltaTime();		
		renderer_OpenFrame();
		input_GetInput(delta_time);
		gui_ProcessGUI();
		
		engine_GameMain(delta_time);
		
		if(engine_state == ENGINE_PLAYING)
		{
			player_ProcessActivePlayer(delta_time);
			player_ProcessAI(delta_time);
			player_UpdatePlayers(delta_time);
			physics_ProcessCollisions(delta_time);
			player_PostUpdatePlayers(delta_time);
			//projectile_UpdateProjectiles();
		} 

		sound_ProcessSound();
		light_UpdateLights();
		entity_UpdateEntities();
			
		world_VisibleWorld();
		light_VisibleLights();
		

		renderer_DrawFrame();		
		renderer_CloseFrame();
		
		/*if(collection_delta < FPS_COLLECTION_TIME)
		{
			accum_frame_time += 1.0 / (engine_GetDeltaTime() * 0.001);
			collection_delta += engine_GetDeltaTime();
			collection_frame_count++;
		}
		else
		{
			fps = accum_frame_time / collection_frame_count;
			
			collection_frame_count = 0;
			collection_delta = 0.0;
			accum_frame_time = 0.0;
		}*/
		
		
		
		
		
		//printf("%f\n", 1.0 / (engine_GetDeltaTime() * 0.001));
		
		
		//printf("CPU: %f\n", e - s);
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

void engine_ReadConfig()
{
	
}

void engine_WriteConfig()
{
	
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
	delta_time = (float)((float)(end_delta - start_delta) * 1000) / performance_frequency;
	start_delta = end_delta;
}

float engine_GetDeltaTime()
{
	unsigned long long cur = SDL_GetPerformanceCounter();
	return (float)((cur - start_delta) * 1000) / (float)performance_frequency;
}

#if defined(__WIN32__) || defined(__WINRT__)
#define WINDOWS_BACKTRACE
#include <Windows.h>
#include <winnt.h>
#include <Dbghelp.h>
#include <imagehlp.h>

#else
#define LINUX_BACKTRACE
#include <execinfo.h>

#endif

void engine_BackTrace()
{
	#ifdef WINDOWS_BACKTRACE
	
	CONTEXT context;
	STACKFRAME frame;
	HANDLE process;
	HANDLE thread;
	HINSTANCE module;
	IMAGEHLP_SYMBOL *symbol;
	
	void *(*exception_information)();
	int d = 0;
	char symbol_buffer[sizeof(IMAGEHLP_SYMBOL) + 255];
	
	memset(symbol_buffer, 0, sizeof(IMAGEHLP_SYMBOL) + 255);
	symbol = (IMAGEHLP_SYMBOL *)symbol_buffer;
	
	
	symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL) + 255;
	symbol->MaxNameLength = 254;
	

	RtlCaptureContext(&context); 
	
	//context = ((GetExceptionInformation())->ContextRecord);
	process = GetCurrentProcess();
	thread = GetCurrentThread();
	
	SymInitialize(process, 0, 1);
	
	
	frame.AddrPC.Offset = context.Eip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrStack.Offset = context.Esp;
	frame.AddrStack.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = context.Ebp;
	frame.AddrFrame.Mode = AddrModeFlat;
	
	while (StackWalk(IMAGE_FILE_MACHINE_I386, process, thread, &frame, &context, 0, SymFunctionTableAccess, SymGetModuleBase, 0))
  	{
  		if(SymGetSymFromAddr(process, frame.AddrPC.Offset, (DWORD *)&d, symbol))
		{
			printf("%s\n", symbol->Name);  	
		}
  		//addr2line("wow", (void*)frame.AddrPC.Offset);
  		//printf("%x\n", (void *)frame.AddrPC.Offset);
  	}
	
	/*HANDLE h_process;
	HANDLE h_thread;
	DWORD machine_type;
	SYMBOL_INFO *info;
	
	int i;
	int captured_frames = 0;
	
	void **trace = malloc(sizeof(void *) * MAXSHORT);	
	captured_frames = RtlCaptureStackBackTrace(0, MAXSHORT, trace, NULL);
	
	for(i = 0; i < captured_frames; i++)
	{
		printf("%x\n", trace[i]);
	}
	
	free(trace);*/
	
	SymCleanup(GetCurrentProcess());
	
	#else 
	
	#endif
}













