#include <stdio.h>

#include "ed_proj.h"
#include "brush.h"
#include "l_main.h"
#include "l_common.h"



extern int brush_count;
extern brush_t *brushes;

extern int light_count;
extern light_position_t *light_positions;
extern light_params_t *light_params;


char current_project_name[512] = {"test_project.wtf"};



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
