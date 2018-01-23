#include <stdio.h>
#include <string.h>

#include "ed_proj.h"
#include "brush.h"
#include "l_main.h"
#include "l_common.h"
#include "bsp_file.h"
#include "material.h"
#include "camera.h"
#include "texture.h"
#include "shader.h"


#include "GL\glew.h"


/* from brush.c */
extern int brush_count;
extern brush_t *brushes;

/* from l_main.c */
extern int light_count;
extern light_position_t *light_positions;
extern light_params_t *light_params;
extern char **light_names;

/* from camera.c */
extern int camera_count;
extern camera_t *camera_list;


char current_project_name[512] = {"test_project.wtf"};

/* from world.c */
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

/* from material.c */
extern int material_count;
extern material_t *materials;
extern char **material_names;

/* from texture.c */
extern texture_reg_t *texture_names;

/* from shader.c */
extern shader_t *shaders;

/* cleared every time the project is saved, set
every time a change happen... */
int b_project_dirty = 0;


void editor_SaveProject(char *file_name)
{
	FILE *file;
	int i;
	int j;
	int k;
	int l;
	proj_header_t header;
	brush_lump_t brush_lump;
	light_lump_t light_lump;
	camera_lump_t camera_lump;
	material_lump_t material_lump;
	triangle_group_lump_t triangle_group_lump;
	triangle_group_t *triangle_group;
	brush_t *brush;
	material_t *material;
	char name[64];
	
	file = fopen(file_name, "wb");
	
	header.version = PROJ_VERSION;
	header.brush_count = brush_count;
	header.light_count = light_count;
	header.camera_count = camera_count;
	header.material_count = material_count;
	
	fwrite(&header, sizeof(proj_header_t), 1, file);
	
	
	for(i = 0; i < material_count; i++)
	{
		for(j = 0; j < 32; j++)
		{
			material_lump.name[j] = '\0';
			material_lump.shader_name[j] = '\0';
		}
		
		strcpy(material_lump.name, material_names[i]);
		strcpy(material_lump.shader_name, shaders[materials[i].shader_index].name);
		
		
		material_lump.base.r = (float)materials[i].r / 255.0;
		material_lump.base.g = (float)materials[i].g / 255.0;
		material_lump.base.b = (float)materials[i].b / 255.0;
		material_lump.base.a = (float)materials[i].a / 255.0;
		
		material_lump.bm_textures = 0;
		
		/*if(materials[i].diffuse_texture > -1)
		{
			material_lump.bm_textures |= 1;
		}
		
		if(materials[i].normal_texture > -1)
		{
			material_lump.bm_textures |= 2;
		}*/
		
		fwrite(&material_lump, sizeof(material_lump_t), 1, file);
	}
	
	
	for(i = 0; i < brush_count; i++)
	{
		brush = &brushes[i];	
		brush_lump.orientation = brush->orientation;
		brush_lump.position = brush->position;
		brush_lump.scale = brush->scale;
		brush_lump.type = brush->type;
		brush_lump.triangle_group_count = brush->triangle_group_count;
		brush_lump.vertex_count = brush->vertex_count;
		
		fwrite(&brush_lump, sizeof(brush_lump_t), 1, file);
		
		for(j = 0; j < brushes[i].triangle_group_count; j++)
		{
			for(k = 0; k < 64; k++)
			{
				name[k] = '\0';
			}
			triangle_group = &brushes[i].triangle_groups[j];
			strcpy(triangle_group_lump.material_name, material_names[triangle_group->material_index]);
			triangle_group_lump.vertice_count = triangle_group->next;
			
			fwrite(&triangle_group_lump, sizeof(triangle_group_lump_t), 1, file);
			
			//fwrite(name, 64, 1, file);
			//fwrite(&triangle_group->next, sizeof(int), 1, file);
			fwrite(&brush->vertices[triangle_group->start], sizeof(vertex_t), triangle_group->next, file);
		}
		
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
		light_lump.bm_flags = light_params[i].bm_flags;
		
		for(j = 0; j < 32; j++)
		{
			light_lump.name[j] = '\0';
		}
		
		strcpy(light_lump.name, light_names[i]);
		fwrite(&light_lump, sizeof(light_lump_t), 1, file);
	}
	
	for(i = 0; i < camera_count; i++)
	{
		camera_lump.fovy = camera_list[i].fov_y;
		camera_lump.width = camera_list[i].width;
		camera_lump.height = camera_list[i].height;
		camera_lump.orientation = camera_list[i].world_orientation;
		camera_lump.position = camera_list[i].world_position;
		
		for(j = 0; j < 32; j++)
		{
			camera_lump.name[j] = '\0';
		}
		
		strcpy(camera_lump.name, camera_list[i].name);
		fwrite(&camera_lump, sizeof(camera_lump_t), 1, file);
	}

	fclose(file);
	
	b_project_dirty = 1;
	
}

void editor_OpenProject(char *file_name)
{
	FILE *file;
	int i;
	int j;
	int k;
	int l;
	proj_header_t header;
	brush_lump_t brush_lump;
	light_lump_t light_lump;
	camera_lump_t camera_lump;
	material_lump_t material_lump;
	triangle_group_lump_t triangle_group_lump;
	triangle_group_t *triangle_group;
	brush_t *brush;
	material_t *material;
	short shader_index;
	short material_index;
	int count;
	char name[64];
	
	
	editor_CloseProject();
	
	file = fopen(file_name, "rb");
	
	fread(&header, sizeof(proj_header_t), 1, file);
	
	for(i = 0; i < header.material_count; i++)
	{
		fread(&material_lump, sizeof(material_lump_t), 1, file);
		
		shader_index = shader_GetShaderIndex(material_lump.shader_name);
		
		if(shader_index < 0)
		{
			printf("editor_OpenProject: no shader [%s] for material [%s] was found!\n", material_lump.shader_name, material_lump.name);
			continue;
		}
		
		material_CreateMaterial(material_lump.name, material_lump.base, 1.0, 1.0, shader_index, -1, -1);
	}
	
	
	for(i = 0; i < header.brush_count; i++)
	{
		j = brush_CreateEmptyBrush();
		brush = &brushes[j];
		
		fread(&brush_lump, sizeof(brush_lump_t), 1, file);
		
		brush->vertices = malloc(sizeof(vertex_t) * brush_lump.vertex_count);
		brush->vertex_count = brush_lump.vertex_count;
		brush->max_vertexes = brush_lump.vertex_count;
		brush->orientation = brush_lump.orientation;
		brush->position = brush_lump.position;
		brush->scale = brush_lump.scale;
		brush->type = brush_lump.type;
		brush->triangles = malloc(sizeof(bsp_striangle_t) * brush_lump.vertex_count / 3);
		brush->triangle_groups = malloc(sizeof(triangle_group_t ) * brush_lump.triangle_group_count);
		brush->triangle_group_count = brush_lump.triangle_group_count;
		//brush->triangle_group_count = brush_lump.triangle_group_count;
		
		brush->triangle_groups[0].start = 0;
		
		for(k = 0; k < brush_lump.triangle_group_count; k++)
		{
			fread(&triangle_group_lump, sizeof(triangle_group_lump_t), 1, file);
			
			brush->triangle_groups[k].material_index = material_GetMaterialIndex(triangle_group_lump.material_name);
			
			/* brush_UpdateBrushElementBuffer needs this to be zero... */
			brush->triangle_groups[k].next = 0;
			
			fread(brush->vertices + brush->triangle_groups[k].start, sizeof(vertex_t), triangle_group_lump.vertice_count, file);
			
			for(l = 0; l < triangle_group_lump.vertice_count; l++)
			{
				if(!(l % 3))
				{
					brush->triangles[(l + brush->triangle_groups[k].start) / 3].first_vertex = l + brush->triangle_groups[k].start;
					brush->triangles[(l + brush->triangle_groups[k].start) / 3].triangle_group = brush->triangle_groups[k].material_index;
				}
			}
					
			if(k < brush_lump.triangle_group_count - 1)
			{
				brush->triangle_groups[k + 1].start = brush->triangle_groups[k].start + triangle_group_lump.vertice_count;
			}		
		}
		
		brush->polygons = NULL;
		
		
		brush->handle = gpu_Alloc(sizeof(vertex_t) * brush->max_vertexes);
		brush->start = gpu_GetAllocStart(brush->handle) / sizeof(vertex_t);
		gpu_Write(brush->handle, 0, brush->vertices, sizeof(vertex_t) * brush->vertex_count, 0);
		
		
		glGenBuffers(1, &brush->element_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brush->element_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * brush->max_vertexes, NULL, GL_DYNAMIC_DRAW);
		
		brush_UpdateBrushElementBuffer(brush);
			
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);	
	}
	
	for(i = 0; i < header.light_count; i++)
	{
		fread(&light_lump, sizeof(light_lump_t), 1, file);
		
		light_CreateLight(light_lump.name, &light_lump.orientation, light_lump.position, light_lump.color, light_lump.radius, light_lump.energy, light_lump.bm_flags);
		
		/*light_lump.orientation = light_positions[i].orientation;
		light_lump.position = light_positions[i].position;
		light_lump.color.r = (float)light_params[i].r / 255.0;
		light_lump.color.g = (float)light_params[i].g / 255.0;
		light_lump.color.b = (float)light_params[i].b / 255.0;
		light_lump.energy = LIGHT_ENERGY(light_params[i].energy);
		light_lump.radius = LIGHT_RADIUS(light_params[i].radius);
		light_lump.type = 0;
		
		fwrite(&light_lump, sizeof(light_lump_t), 1, file);*/
	}
	
	for(i = 0; i < header.camera_count; i++)
	{
		/*camera_lump.fovy = camera_list[i].fov_y;
		camera_lump.width = camera_list[i].width;
		camera_lump.height = camera_list[i].height;
		camera_lump.orientation = camera_list[i].world_orientation;
		camera_lump.position = camera_list[i].world_position;
		
		for(j = 0; j < 32; j++)
		{
			camera_lump.name[j] = '\0';
		}
		
		strcpy(camera_lump.name, camera_list[i].name);
		
		fwrite(&camera_lump, sizeof(camera_lump_t), 1, file);*/
	}
}

void editor_CloseProject()
{
	if(b_project_dirty)
	{
		
	}
	
	light_DestroyAllLights();
	brush_DestroyAllBrushes();
	material_DestroyAllMaterials();
	
}

void editor_ExportBsp(char *file_name)
{
	FILE *file;
	bsp_header_t header;
	light_lump_t light_lump;
	triangle_group_lump_t triangle_group_lump;
	material_lump_t material_lump;
	
	char attrib_name[32];
	
	file = fopen(file_name, "wb");
	
	int i;
	int j;
	
	int start;
	int count;
	
	header.version = BSP_FILE_VERSION;
	header.light_count = light_count;
	header.world_vertice_count = world_vertices_count;
	header.world_triangle_group_count = world_triangle_group_count;
	header.world_nodes_count = world_nodes_count;
	header.world_leaves_count = world_leaves_count;
	header.collision_nodes_count = collision_nodes_count;
	header.material_count = material_count;
	
	fwrite(&header, sizeof(bsp_header_t), 1, file);
	
	for(i = 0; i < material_count; i++)
	{
		material_lump.base.r = (float)materials[i].r / 255.0;
		material_lump.base.g = (float)materials[i].g / 255.0;
		material_lump.base.b = (float)materials[i].b / 255.0;
		material_lump.base.a = (float)materials[i].a / 255.0;
		
		material_lump.bm_textures = 0;
		
		if(materials[i].diffuse_texture != -1)
		{
			material_lump.bm_textures |= 1;
		}
		
		if(materials[i].normal_texture != -1)
		{
			material_lump.bm_textures |= 2;
		}
		
		fwrite(&material_lump, sizeof(material_lump_t), 1, file);
		
		if(material_lump.bm_textures & 1)
		{
			for(j = 0; j < 32; j++)
			{
				attrib_name[j] = 0;
			}
				
			strcpy(attrib_name, texture_names[materials[i].diffuse_texture].file_name);
			
			fwrite(attrib_name, 32, 1, file);
		}
		
		if(material_lump.bm_textures & 2)
		{
			for(j = 0; j < 32; j++)
			{
				attrib_name[j] = 0;
			}
				
			strcpy(attrib_name, texture_names[materials[i].normal_texture].file_name);
			
			fwrite(attrib_name, 32, 1, file);
		}
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
	
	fwrite(world_nodes, sizeof(bsp_pnode_t), world_nodes_count, file);
	fwrite(world_leaves, sizeof(bsp_dleaf_t), world_leaves_count, file);
	fwrite(collision_nodes, sizeof(bsp_pnode_t), collision_nodes_count, file);
	
	for(i = 0; i < world_triangle_group_count; i++)
	{
		for(j = 0; j < 32; j++)
		{
			triangle_group_lump.material_name[j] = 0;
		}
		
		start = world_triangle_groups[i].start;
		count = world_triangle_groups[i].next;
		
		triangle_group_lump.vertice_count = count;
		
		
		strcpy(triangle_group_lump.material_name, material_names[world_triangle_groups[i].material_index]);
		fwrite(&triangle_group_lump, sizeof(triangle_group_lump_t), 1, file);
		
		for(j = 0; j < count;)
		{
			fwrite(&world_vertices[start + j], sizeof(vertex_t), 3, file);
			
			j += 3;
		}
		
	}
	
	fclose(file);
}









