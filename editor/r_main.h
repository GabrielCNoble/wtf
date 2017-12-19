#ifndef R_MAIN_H
#define R_MAIN_H

enum INIT_MODE
{
	INIT_FULLSCREEN = 1,
	INIT_WINDOWED
};


typedef struct
{
	int start;
	int count;
}command_buffer_t;

void renderer_Init(int width, int height, int init_mode);;

void renderer_Finish();

void renderer_RegisterFunction(void (*r_fn)(void));

void renderer_OpenFrame();

void renderer_DrawFrame();

void renderer_ZPrePass();

void renderer_DrawShadowMaps();

void renderer_DrawGUI();

void renderer_DrawWorld();

void renderer_DrawSkyBox();

void renderer_CloseFrame();

void renderer_GetWindowSize(int *w, int *h);

void renderer_SubmitCommandBuffer();





#endif
