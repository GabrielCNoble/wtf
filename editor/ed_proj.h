#ifndef ED_PROJ_H
#define ED_PROJ_H

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

typedef struct
{
	int version;
	int brush_count;
	int light_count;
	int camera_count;
	int material_count;
	int texture_count;
	int spawn_point_count;
	char *active_camera;
}proj_header_t;

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
}brush_lump_t;

typedef struct
{
	int vert_count;
	int first_index_offset;
	vec3_t normal;
	char material_name[MAX_NAME_LEN];
}polygon_record_t;

typedef struct
{
	mat3_t orientation;
	vec3_t position;
	float fovy;
	float width;
	float height;
	char name[MAX_NAME_LEN];
}camera_lump_t;

typedef struct
{
	vec3_t position;
	char name[MAX_NAME_LEN];
}spawn_point_record_t;



void editor_SaveProject();

int editor_OpenProject(char *file_name);

void editor_SetProjectName(char *name);

void editor_CloseProject();

void editor_ExportBsp(char *file_name);

int editor_IsProjectDirty();

void editor_DirtyProject();

void editor_CleanProject();
 
#endif




