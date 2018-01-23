#ifndef ED_PROJ_H
#define ED_PROJ_H

#include "vector.h"
#include "matrix.h"
#include "camera.h"
#include "brush.h"

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
	char *active_camera;
}proj_header_t;

typedef struct
{
	mat3_t orientation;
	vec3_t position;
	vec3_t scale;
	int vertex_count;
	int triangle_group_count;
	short type;
	short bm_flags;
}brush_lump_t;

typedef struct
{
	mat3_t orientation;
	vec3_t position;
	float fovy;
	float width;
	float height;
	char name[32];
}camera_lump_t;



void editor_SaveProject(char *file_name);

void editor_OpenProject(char *file_name);

void editor_CloseProject();

void editor_ExportBsp(char *file_name);

#endif
