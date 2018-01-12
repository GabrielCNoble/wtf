#ifndef R_MAIN_H
#define R_MAIN_H

enum INIT_MODE
{
	INIT_FULLSCREEN_DESKTOP = 1,
	INIT_FULLSCREEN,
	INIT_WINDOWED
};

enum WINDOW_FLAGS
{
	WINDOW_FULLSCREEN = 1,
	
};

enum RENDERER_CALLBACK_TYPE
{
	PRE_SHADING_STAGE_CALLBACK = 1,
	POST_SHADING_STAGE_CALLBACK,
	RENDERER_RESOLUTION_CHANGE_CALLBACK,
	WINDOW_RESIZE_CALLBACK,
};

void renderer_Init(int width, int height, int init_mode);

void renderer_Finish();

void renderer_SetWindowSize(int width, int height);

void renderer_SetRendererResolution(int width, int height);

void renderer_Fullscreen(int enable);

void renderer_RegisterCallback(void (*r_fn)(void), int type);

void renderer_OpenFrame();

void renderer_DrawFrame();

void renderer_ZPrePass();

void renderer_DrawShadowMaps();

void renderer_DrawGUI();

void renderer_DrawPlayers();

void renderer_DrawActivePlayer();

void renderer_DrawWorld();

//void renderer_DrawWorldDeferred();

void renderer_DrawSkyBox();

//void renderer_Shade();

void renderer_CloseFrame();







#endif
