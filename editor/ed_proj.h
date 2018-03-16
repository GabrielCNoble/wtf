#ifndef ED_PROJ_H
#define ED_PROJ_H

#include <stdint.h>

#include "vector.h"
#include "matrix.h"
#include "camera.h"
#include "brush.h"
#include "bsp_file.h"

#define PROJ_EXT ".wtf"
#define PROJ_VERSION 0

/************************************************

	structure of the project file (.wtf)...
	
	proj_header (proj_header_t)
	
	
	material0 (material_lump_t)
	diffuse_texture_name (if present)
	normal_texture_name (if present)
	
	
	material1 (material_lump_t)
	diffuse_texture_name (if present)
	normal_texture_name (if present)
	.
	.
	.
	
	
	
	brush0 (brush_lump_t)
	brush0_triangle_group0_material_name
	brush0_triangle_group0_vertex_count
	brush0_triangle_group0_vertices
	brush0_triangle_group1_material_name
	brush0_triangle_group1_vertex_count
	brush0_triangle_group1_vertices
	
	
	brush1 (brush_lump_t)
	brush1_triangle_group0_material_name
	brush1_triangle_group0_vertex_count
	brush1_triangle_group0_vertices
	brush1_triangle_group1_material_name
	brush1_triangle_group1_vertex_count
	brush1_triangle_group1_vertices
	.
	.
	.
	
	
	light0 (light_lump_t)
	light1 (light_lump_t)
	.
	.
	.
	
	
	camera0 (camera_lump_t)
	camera1 (camera_lump_t)
	.
	.
	.
	
	
	
	
	
		


*************************************************/

#define WTF_CONSTANT0 0x00667477
#define WTF_CONSTANT1 0x77746600

enum PROJ_FILE_TAGS
{
	POLYGON_RECORD = 0,
	BRUSH_RECORD,
	MATERIAL_RECORD,
	TRIANGLE_GROUP_RECORD,
	SPAWN_POINT_RECORD,
	TEXTURE_RECORD,
	LIGHT_RECORD,
	ENTITY_DEF_RECORD,
	ENTITY_RECORD,
	MODEL_RECORD,
};

static char *proj_file_tags_str[] = 
{
	"[polygon record]",
	"[brush record]",
	"[material record]",
	"[triangle group record]",
	"[spawn point record]",
	"[texture record]",
	"[light record]",
	"[entity definition record]",
	"[entity record]",
	"[model record]",
};


/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	uint32_t wtf0;
	uint32_t wtf1;
	uint32_t version;
	uint32_t field_count;
	
	
	uint32_t brush_count;
	uint32_t light_count;
	uint32_t camera_count;
	uint32_t material_count;
	uint32_t texture_count;
	uint32_t spawn_point_count;
	uint32_t model_count;
	uint32_t entity_def_count;
	uint32_t entity_count;
	uint32_t sound_emitter_count;
	uint32_t particle_emitter_count;
	
	/* reserved for future use... */
	/* NOTE: yeah, this should do... */
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
	
}proj_header_t;

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
	int vertex_count;
	//int index_count;
	int triangle_group_count;
	int polygon_count;
	short type;
	short bm_flags;
}brush_record_t;

static struct
{
	brush_record_t record;
}brush_record_size_t;


/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	int vert_count;
	int first_index_offset;
	vec3_t normal;
	char material_name[1];
	//char material_name[MAX_NAME_LEN];
}polygon_record_t;


static struct
{
	polygon_record_t record;
	char max_material_name[BSP_FILE_MAX_NAME_LEN - 1];
}polygon_record_size_t;


/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	mat3_t orientation;
	vec3_t position;
	float fovy;
	float width;
	float height;
	char name[MAX_NAME_LEN];
}camera_record_t;

/*
===================================================================
===================================================================
===================================================================
*/


void editor_NewProject();

void editor_SaveProject();

int editor_OpenProject(char *file_name);

void editor_CopyProjectResources(char *base_path);

void editor_SetProjectName(char *name);

char *editor_GetProjectName();

void editor_CloseProject();

void editor_ExportBsp(char *file_name);

int editor_IsProjectDirty();

void editor_DirtyProject();

void editor_CleanProject();

char *editor_GetAbsolutePathToProject();
 
#endif




