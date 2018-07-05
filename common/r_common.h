#ifndef R_COMMON_H
#define R_COMMON_H

#include "vector.h"
#include "matrix.h"


#define RENDERER_MIN_WIDTH 800
#define RENDERER_MAX_WIDTH 1920

#define RENDERER_MIN_HEIGHT 600
#define RENDERER_MAX_HEIGHT 1080

#define RENDERER_MIN_MSAA_SAMPLES 1
#define RENDERER_MAX_MSAA_SAMPLES 16

#define R_DRAW_COMMAND_LIST_RESIZE_INCREMENT 128


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

enum RENDERER_TEXTURES
{
	RENDERER_TEXTURE0,
	RENDERER_TEXTURE1,
	RENDERER_TEXTURE2,
	RENDERER_TEXTURE3,
	RENDERER_TEXTURE4,
	RENDERER_TEXTURE5,
};

enum RENDERER_TEXTURE_TARGETS
{
	RENDERER_TEXTURE_1D,
	RENDERER_TEXTURE_1D_ARRAY,
	RENDERER_TEXTURE_2D,
	RENDERER_TEXTURE_2D_ARRAY,
	RENDERER_TEXTURE_3D,
};

enum VERTEX_ATTRIB
{
	VERTEX_ATTRIB_POSITION,
	VERTEX_ATTRIB_NORMAL,
	VERTEX_ATTRIB_TEX_COORDS,
	VERTEX_ATTRIB_TANGENT,
	VERTEX_ATTRIB_COLOR,
};

enum VERTEX_FORMAT
{
	VERTEX_FORMAT_V3F_N3F_T3F_TC2F,
	VERTEX_FORMAT_V3F_N4IREV_T4IREV_TC2F,
	VERTEX_FORMAT_V3F_N3F,
	VERTEX_FORMAT_V3F,
	VERTEX_FORMAT_CUSTOM,
};

enum DRAW_COMMAND_FLAGS
{
	DRAW_COMMAND_FLAG_INDEXED_DRAW = 1,
};


typedef struct
{
	mat4_t *transform;
	unsigned int start;
	unsigned int count;
	unsigned char draw_mode;
	unsigned char flags;
	//unsigned short draw_mode;
	short material_index;			/* this could be a char... */
}draw_command_t;

typedef struct
{
	int material_index;
	unsigned short max_draw_cmds;
	unsigned short draw_cmds_count;
	draw_command_t *draw_cmds;
}draw_command_group_t;


typedef struct
{
	int start;
	int next;
	int material_index;
}batch_t;


typedef struct
{
	unsigned int framebuffer_id;
	unsigned int color_attachment;
	unsigned int depth_attachment;
	unsigned int stencil_attachment;
}framebuffer_t;

typedef struct
{
	
}tex_unit_t;

#endif











