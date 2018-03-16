#ifndef BSP_FILE_H
#define BSP_FILE_H


#define BSP_FILE_VERSION 0
#define BSP_FILE_MAX_NAME_LEN 64
#define MAX_NAME_LEN 64
#define BSP_MAX_NAME_LEN 512

#include <stdint.h>
#include "material.h"
#include "matrix.h"

/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	uint32_t version;
	uint32_t world_vertice_count;
	uint32_t world_triangle_group_count;
	uint32_t world_nodes_count;
	uint32_t world_leaves_count;
	uint32_t collision_nodes_count;
	uint32_t light_count;
	uint32_t material_count;
	uint32_t model_count;
	uint32_t entity_def_count;
	uint32_t entity_count;
	uint32_t sound_emitter_count;
	uint32_t particle_emitter_count;
	
	
	uint32_t reserved0;
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t reserved3;
	uint32_t reserved4;
	uint32_t reserved5;
	uint32_t reserved6;
	uint32_t reserved7;
	uint32_t reserved8;
	uint32_t reserved9;
	uint32_t reserved10;
	uint32_t reserved11;
	uint32_t reserved12;
	uint32_t reserved13;
	uint32_t reserved14;
	uint32_t reserved15;
	
}bsp_header_t;

/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	mat3_t orientation;
	vec3_t position;
	vec3_t color;
	float radius;
	float energy;
	short type;
	short bm_flags;
	char name[1];
}light_record_t;

static struct
{
	light_record_t record;
	char name[BSP_MAX_NAME_LEN - 1];
}light_record_size_t;

/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	vec4_t base;
	int bm_flags;
	char name[1];
}material_record_t;

static struct
{
	material_record_t record;
	char max_name[BSP_MAX_NAME_LEN - 1];
	char diffuse_texture_name[BSP_MAX_NAME_LEN];
	char normal_texture_name[BSP_MAX_NAME_LEN];
}material_record_size_t;

/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	int vertice_count;
	char material_name[1];
}triangle_group_record_t;

/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	vec3_t position;
	char name[1];
}spawn_point_record_t;

static struct
{
	spawn_point_record_t record;
	char max_name[BSP_MAX_NAME_LEN - 1];
}spawn_point_record_size_t;

/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	int bm_texture_flags;
	/* char file_name[1]; */
	/* char name[1]; */
}texture_record_t;

static struct
{
	texture_record_t record;
	char max_file_name[BSP_MAX_NAME_LEN];
	char max_name[BSP_MAX_NAME_LEN];
}texture_record_size_t;

/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	int flags;
	/* char file_name[1]; */
	/* char name[1]; */
}model_record_t;

static struct
{
	model_record_t record;
	char max_file_name[BSP_FILE_MAX_NAME_LEN];
	char max_name[BSP_FILE_MAX_NAME_LEN];
}model_record_size_t;

/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	int type;
	int flags;
	/* char name[1]; */
	/* char model[1]; */
}entity_def_record_t;


static struct
{
	entity_def_record_t record;
	char max_name[BSP_FILE_MAX_NAME_LEN];
	char max_model_name[BSP_FILE_MAX_NAME_LEN];
}entity_def_record_size_t;

/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	mat3_t orientation;
	vec3_t position;
	vec3_t scale;
	int flags;
	/* char name[1]; */
	/* char def[1]; */
}entity_record_t;

static struct
{
	entity_record_t record;
	char max_name[BSP_FILE_MAX_NAME_LEN];
	char max_def_name[BSP_FILE_MAX_NAME_LEN];
}entity_record_size_t;


/*
===================================================================
===================================================================
===================================================================
*/



#endif








