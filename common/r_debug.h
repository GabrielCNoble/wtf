#ifndef R_DEBUG_H
#define R_DEBUG_H

#include "vector.h"


#ifdef R_DBG_NO_DEBUG
	#define R_DBG_PUSH_FUNCTION_NAME()
	#define R_DBG_POP_FUNCTION_NAME() 
#else
	#define R_DBG_PUSH_FUNCTION_NAME() renderer_PushFunctionName(__func__)
	#define R_DBG_POP_FUNCTION_NAME() renderer_PopFunctionName()
#endif


enum DEBUG_COMMAND_TYPES
{
	DEBUG_COMMAND_TYPE_DRAW_POINT,
	DEBUG_COMMAND_TYPE_DRAW_LINE,
	DEBUG_COMMAND_TYPE_DRAW_2D_LINE,
	DEBUG_COMMAND_TYPE_DRAW_VERTS,
	DEBUG_COMMAND_TYPE_DRAW_VERTS_INDEXED,
};

typedef struct
{
	short type;
	short persistent;
	void *data;
}dbg_command_t;

typedef struct
{
	unsigned char depth_test;
	unsigned char stencil_test;
	unsigned char depth_mask;
	unsigned char stencil_ref;
	
	unsigned short stencil_func;
	unsigned short stencil_op_fail;
	unsigned short stencil_op_zfail;
	unsigned short stencil_op_zpass;
}dbg_draw_params_t;

typedef struct
{
	vec3_t position;
	vec3_t color;
	int smooth;
	float size;
	
	dbg_draw_params_t draw_params;
}point_dbg_draw_data_t;

typedef struct
{
	vec3_t from;
	vec3_t to;
	vec3_t color;
	float width;
}line_dbg_draw_data_t;

typedef struct
{
	vec2_t from;
	vec2_t to;
	vec3_t color;
	float width;
}line_2d_dbg_draw_data_t;

typedef struct
{
	unsigned char depth_test;
	unsigned char stencil_test;
	unsigned char depth_mask;
	unsigned char stencil_ref;
	
	unsigned short stencil_func;
	unsigned short stencil_op_fail;
	unsigned short stencil_op_zfail;
	unsigned short stencil_op_zpass;
	
	unsigned short fill_mode;
	
}polygon_dbg_draw_params_t;


typedef struct
{
	int size;
	short stride;
	short mode;
	int count;
	
	polygon_dbg_draw_params_t params;
	
	void *verts;
}verts_dbg_draw_data_t;


#ifdef __cplusplus
extern "C"
{
#endif

void renderer_InitDebug();

void renderer_FinishDebug();

void renderer_Debug(int enable, int verbose);

void renderer_VerboseDebugOutput(int enable);

void renderer_DrawPoint(vec3_t position, vec3_t color, float size, int smooth, int persistent, int depth_test);

void renderer_DrawLine(vec3_t from, vec3_t to, vec3_t color, float width, int persistent);

void renderer_Draw2dLine(vec2_t from, vec2_t to, vec3_t color, float width, int persistent);

/*
==============================================================
==============================================================
==============================================================
*/

void renderer_DrawPortalsOulines();

void renderer_DrawPortalViews();

void renderer_DrawViews();

void renderer_DrawWaypoints();

void renderer_DrawColliders();

void renderer_DrawEntities();

/*
==============================================================
==============================================================
==============================================================
*/

void renderer_DrawDebug();

void renderer_PushFunctionName(const char *name);

void renderer_PopFunctionName();

#ifdef __cplusplus
}
#endif



#endif