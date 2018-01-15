#include <stdio.h>
#include <string.h>

#include "ed_proj.h"
#include "brush.h"
#include "l_main.h"
#include "l_common.h"
#include "bsp_file.h"
#include "material.h"



extern int brush_count;
extern brush_t *brushes;

extern int light_count;
extern light_position_t *light_positions;
extern light_params_t *light_params;


char current_project_name[512] = {"test_project.wtf"};

extern int world_vertices_count;
extern vertex_t *world_vertices;

extern int world_nodes_count;
extern bsp_pnode_t *world_nodes;
extern int collision_nodes_count;
extern bsp_pnode_t *collision_nodes;

extern int world_leaves_count;
extern bsp_dleaf_t *world_leaves; 

extern int world_triangle_group_count;
extern triangle_group_t *world_triangle_groups;


extern int material_count;
extern material_t *materials;
extern char **material_names;


void editor_SaveProject(char *file_name)
{
	FILE *file;
	int i;
	proj_header_t header;
	brush_lump_t brush_lump;
	light_lump_t light_lump;
	
	file = fopen(file_name, "wb");
	
	header.version = PROJ_VERSION;
	header.brush_count = brush_count;
	header.light_count = light_count;
	
	fwrite(&header, sizeof(proj_header_t), 1, file);
	
	for(i = 0; i < brush_count; i++)
	{
		brush_lump.orientation = brushes[i].orientation;
		brush_lump.position = brushes[i].position;
		brush_lump.scale = brushes[i].scale;
		brush_lump.type = brushes[i].type;
		
		fwrite(&brush_lump, sizeof(brush_lump_t), 1, file);
	}
	
	for(i = 0; i < light_count; i++)
	{
		light_lump.orientation = light_positions[i].orientation;
		light_lump.position = light_positions[i].position;
		light_lump.color.r = (float)light_params[i].r / 255.0;
		light_lump.color.g = (float)light_params[i].g / 255.0;
		light_lump.color.b = (float)light_params[i].b / 255.0;
		light_lump.energy = LIGHT_ENERGY(light_params[i].energy);
		light_lump.radius = LIGHT_RADIUS(light_params[i].radius);
		light_lump.type = 0;
		
		fwrite(&light_lump, sizeof(light_lump_t), 1, file);
	}
	fclose(file);
}

void editor_LoadProject(char *file_name)
{
	
}

void editor_ExportBsp(char *file_name)
{
	FILE *file;
	bsp_header_t header;
	light_lump_t light_lump;
	triangle_group_lump_t triangle_group_lump;
	file = fopen(file_name, "wb");
	
	int i;
	int j;
	
	header.version = BSP_FILE_VERSION;
	header.light_count = light_count;
	header.world_vertice_count = world_vertices_count;
	header.world_triangle_group_count = world_triangle_group_count;
	header.world_nodes_count = world_nodes_count;
	header.world_leaves_count = world_leaves_count;
	header.collision_nodes_count = collision_nodes_count;
	
	fwrite(&header, sizeof(bsp_header_t), 1, file);
	
	for(i = 0; i < light_count; i++)
	{
		light_lump.orientation = light_positions[i].orientation;
		light_lump.position = light_positions[i].position;
		light_lump.color.r = (float)light_params[i].r / 255.0;
		light_lump.color.g = (float)light_params[i].g / 255.0;
		light_lump.color.b = (float)light_params[i].b / 255.0;
		light_lump.energy = LIGHT_ENERGY(light_params[i].energy);
		light_lump.radius = LIGHT_RADIUS(light_params[i].radius);
		light_lump.type = 0;
		
		fwrite(&light_lump, sizeof(light_lump_t), 1, file);
	}
	
	fwrite(world_nodes, sizeof(bsp_pnode_t), world_nodes_count, file);
	fwrite(world_leaves, sizeof(bsp_dleaf_t), world_leaves_count, file);
	fwrite(collision_nodes, sizeof(bsp_pnode_t), collision_nodes_count, file);
	
	for(i = 0; i < world_triangle_group_count; i++)
	{
		for(j = 0; j < 32; j++)
		{
			triangle_group_lump.material_name[j] = 0;
		}
		
		triangle_group_lump.vertice_count = world_triangle_groups[i].next;
		strcpy(triangle_group_lump.material_name, material_names[world_triangle_groups[i].material_index]);
		fwrite(&triangle_group_lump, sizeof(triangle_group_lump_t), 1, file);
	}
	
	fclose(file);
}









