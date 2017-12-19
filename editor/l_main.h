#ifndef L_MAIN_H
#define L_MAIN_H

#include "matrix_types.h"
#include "vector_types.h"

#include "l_common.h"
#include "bsp_common.h"




enum LIGHT_FLAGS
{
	LIGHT_CACHED = 1,
	LIGHT_MOVED = 1 << 1
};


typedef struct
{
	mat4_t light_to_world_matrix;
	mat3_t orientation;
	vec3_t position;
}light_position_t;


typedef struct
{
	struct bsp_dleaf_t *leaf;					/* in which leaf this light is in (updated every time it moves)... */
	
	unsigned char r;
	unsigned char g;
	unsigned char b;
	char cache;
	
	unsigned short radius;
	unsigned short energy;
	
	unsigned char bm_flags;
	unsigned char align0;
	unsigned char align1;
	unsigned char alignd2;
	
	
	
	//unsigned short first_cluster_id;
	//unsigned short last_cluster_id;
}light_params_t;

typedef struct
{
	//mat4_t projection_matrix;
	//mat4_t world_to_light_matrix;
	vec4_t forward_axis;
	vec4_t position;
	vec4_t color;
	float radius;
	float energy;
	float align0;
	float align1;
}gpu_lamp_t;

typedef struct light_cache_slot_t
{
	unsigned int index_buffer;
	unsigned int last_touched;
	unsigned short light_index;
	unsigned short gpu_index;
}light_cache_slot_t;

void light_Init();

void light_Finish();

int light_CreateLight(char *name, mat3_t *orientation, vec3_t position, vec3_t color, float radius, float energy);

void light_UpdateLights();

//void light_CacheLight(int light_index);

//void light_DropLight(int light_index);

//void light_EvictOld();

//void light_Update();

//void light_BindLightCache();

//void light_UnbindLightCache();

void light_VisibleLights();

void light_ClearLightLeaves();

void light_TranslateLight(int light_index, vec3_t direction, float amount);

void light_SetLight(int light_index);




#endif
