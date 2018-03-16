#ifndef R_MAIN_H
#define R_MAIN_H

#include "vector.h"
#include "matrix.h"
#include "w_common.h"
#include <stdint.h>

#define RENDERER_MIN_WIDTH 800
#define RENDERER_MAX_WIDTH 1920

#define RENDERER_MIN_HEIGHT 600
#define RENDERER_MAX_HEIGHT 1080

#define RENDERER_MIN_MSAA_SAMPLES 1
#define RENDERER_MAX_MSAA_SAMPLES 16

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

enum RENDERER_STAGE
{
	RENDERER_DRAW_SHADOW_MAPS_STAGE = 0,
	RENDERER_Z_PREPASS_STAGE,
	RENDERER_DRAW_WORLD_STAGE,
	RENDERER_SWAP_BUFFERS_STAGE,
	RENDERER_BIND_LIGHT_CACHE,
	RENDERER_BIND_GPU_CACHE,
	RENDERER_DRAW_FRAME,
	RENDERER_DRAW_GUI,
	RENDERER_STAGE_COUNT
};

enum VERTEX_ATTRIB
{
	VERTEX_ATTRIB_POSITION,
	VERTEX_ATTRIB_NORMAL,
	VERTEX_ATTRIB_TEX_COORDS,
	VERTEX_ATTRIB_TANGENT,
};


typedef struct
{
	mat4_t *transform;
	unsigned int vert_start;
	unsigned int vert_count;
	unsigned int draw_mode;
}draw_command_t;

typedef struct
{
	int material_index;
	unsigned short max_draw_cmds;
	unsigned short draw_cmds_count;
	draw_command_t *draw_cmds;
}draw_group_t;




int renderer_Init(int width, int height, int init_mode);

void renderer_Finish();

void renderer_SetDiffuseTexture(int texture_index);

void renderer_SetNormalTexture(int texture_index);

void renderer_SetClusterTexture();

void renderer_SetShadowTexture();

void renderer_SetTexture(int texture_unit, int texture_target, int texture_index);

void renderer_BindTexture(int texture_unit, int texture_target, int texture);

//void renderer_SetViewProjectionMatrix(float *matrix);


void renderer_SetProjectionMatrix(mat4_t *matrix);

void renderer_SetViewMatrix(mat4_t *matrix);

void renderer_SetModelMatrix(mat4_t *matrix);

void renderer_UpdateMatrices();




void renderer_SetShader(int shader_index);

void renderer_SetUniform1i(int uniform, int value);

void renderer_SetUniform4fv(int uniform, float *value);

void renderer_SetUniformMatrix4fv(int uniform, float *value);

void renderer_SetVertexAttribPointer(int attrib, int size, int offset, int stride);

void renderer_SetMaterial(int material_index);

void renderer_UpdateDrawGroups();

void renderer_SubmitDrawCommand(mat4_t *transform, unsigned short draw_mode, unsigned int vert_start, unsigned int vert_count, int material_index);






void renderer_SetWindowSize(int width, int height);

void renderer_Backbuffer(int width, int height, int samples);

void renderer_BindBackbuffer();

void renderer_SetRendererResolution(int width, int height);

void renderer_Fullscreen(int enable);

void renderer_ToggleFullscreen();

void renderer_Multisample(int enable);

void renderer_ToggleMultisample();

void renderer_RegisterCallback(void (*r_fn)(void), int type);

void renderer_OpenFrame();

void renderer_DrawFrame();

void renderer_ZPrePass();

void renderer_BlitBackbuffer();

void renderer_DrawShadowMaps();

void renderer_DrawGUI();

void renderer_DrawPlayers();

void renderer_DrawActivePlayer();

void renderer_DrawOpaque();

void renderer_DrawTranslucent();

void renderer_DrawWorld();

void renderer_DrawBloom();

void renderer_Tonemap();

void renderer_DrawSkyBox();

void renderer_DrawParticles();

void renderer_CloseFrame();

void renderer_BeginTimeElapsedQuery();

void renderer_EndTimeElapsedQuery(int stage_index);

void renderer_ReportQueryResults();

char *renderer_GetGLEnumString(int name);







#endif
