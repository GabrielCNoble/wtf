#include <stdio.h>
#include <string.h>

#include "ed_proj.h"
#include "editor.h"
#include "brush.h"
#include "l_main.h"
#include "l_common.h"
#include "bsp_file.h"
#include "material.h"
#include "camera.h"
#include "texture.h"
#include "shader.h"
#include "r_main.h"
#include "bsp.h"
#include "bsp_cmp.h"
#include "player.h"
#include "model.h"
#include "entity.h"

#include "SDL2\SDL.h"
#include "GL\glew.h"
#include "path.h"
 

/* from brush.c */
extern int brush_count;
extern brush_t *brushes;

/* from l_main.c */
extern int light_count;
extern light_position_t *light_positions;
extern light_params_t *light_params;
extern char **light_names;

/* from entity.c */
extern int ent_entity_def_list_cursor;
extern entity_def_t *ent_entity_defs;
extern int ent_entity_list_cursor;
extern entity_t *ent_entities;

/* from camera.c */
extern int camera_count;
extern camera_t *camera_list;


extern SDL_Window *window;

char current_project_name[512];
char absolute_path_to_project[512];
char project_name[512];

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
extern int texture_count;
extern texture_t *textures;
extern texture_info_t *texture_info;

/* from shader.c */
extern shader_t *shaders;

/* cleared every time the project is saved, set
every time a change happens... */
int b_project_dirty = 0;

/* from bsp_cmp.c */
extern int b_compiling;

/* from pvs.c */
extern int b_calculating_pvs;


/* from player.c */
extern int spawn_point_count;
extern spawn_point_t *spawn_points;


void editor_NewProject()
{
	editor_CloseProject();
}


void editor_WriteTag(char *file_buffer, unsigned int *file_buffer_cursor, int tag)
{
	unsigned int cursor = *file_buffer_cursor;
	
	strcpy(file_buffer + cursor, proj_file_tags_str[tag]);
	cursor += strlen(proj_file_tags_str[tag]) + 1;
	*file_buffer_cursor = cursor;
	
	return;
}


void editor_SaveProject()
{
	FILE *file;
	int i;
	int j;
	int k;
	int l;
	proj_header_t temp_header;

	
	proj_header_t *header;
	brush_record_t *brush_record;
	light_record_t *light_record;
	camera_record_t *camera_record;
	material_record_t *material_record;
	triangle_group_record_t *triangle_group_record;
	polygon_record_t *polygon_record;
	spawn_point_record_t *spawn_point_record;
	texture_record_t *texture_record;
	triangle_group_t *triangle_group;
	entity_def_record_t *entity_def_record;
	entity_record_t *entity_record;
	
	
	brush_t *brush;
	bsp_polygon_t *polygon;
	material_t *material;
	model_t *model;
	
	char name[BSP_MAX_NAME_LEN];
	char full_project_path[512];
	char cmd_string[512];
	
	char *file_buffer;
	unsigned int file_size = 0;
	unsigned int file_buffer_cursor = 0;
	unsigned int vertex_count = 0;
	unsigned int polygon_count = 0;
	
	//char *project_name;
	
	file = fopen(current_project_name, "rb");
	strcpy(full_project_path, absolute_path_to_project);
	
	if(!file)
	{		
		#if defined (__WIN32__)
		strcpy(cmd_string, "mkdir ");
		strcat(cmd_string, full_project_path);
		system(cmd_string);
		
		strcpy(cmd_string, "mkdir ");
		strcat(cmd_string, full_project_path);
		strcat(cmd_string, "\\");
		strcat(cmd_string, "textures");
		system(cmd_string);
		
		strcpy(cmd_string, "mkdir ");
		strcat(cmd_string, full_project_path);
		strcat(cmd_string, "\\");
		strcat(cmd_string, "sounds");
		system(cmd_string);
		
		strcpy(cmd_string, "mkdir ");
		strcat(cmd_string, full_project_path);
		strcat(cmd_string, "\\");
		strcat(cmd_string, "models");
		system(cmd_string);
		#endif
	}
	else
	{
		
		path_SetDir(absolute_path_to_project);
		
		if(!path_CheckSubDir("textures"))
		{
			#if defined (__WIN32__)
			strcpy(cmd_string, "mkdir ");
			strcat(cmd_string, full_project_path);
			strcat(cmd_string, "\\");
			strcat(cmd_string, "textures");
			system(cmd_string);
			#else
			
			
			#endif
		}
		
		if(!path_CheckSubDir("sounds"))
		{
			#if defined (__WIN32__)
			strcpy(cmd_string, "mkdir ");
			strcat(cmd_string, full_project_path);
			strcat(cmd_string, "\\");
			strcat(cmd_string, "sounds");
			system(cmd_string);
			#else
			
			
			#endif
		}
		
		if(!path_CheckSubDir("models"))
		{
			#if defined (__WIN32__)
			strcpy(cmd_string, "mkdir ");
			strcat(cmd_string, full_project_path);
			strcat(cmd_string, "\\");
			strcat(cmd_string, "models");
			system(cmd_string);
			#else
			
			
			#endif
		}
		
		
		fclose(file);
	}
	
	
	editor_CopyProjectResources(full_project_path);
	strcpy(full_project_path, current_project_name);
	file = fopen(full_project_path, "wb");
	 
	temp_header.wtf0 = WTF_CONSTANT0;
	temp_header.wtf1 = WTF_CONSTANT1;
	temp_header.version = PROJ_VERSION;
	temp_header.brush_count = 0;
	temp_header.light_count = 0;
	temp_header.camera_count = camera_count;
	temp_header.material_count = 0;
	temp_header.spawn_point_count = 0;
	temp_header.texture_count = 0;
	temp_header.model_count = 0;
	temp_header.entity_def_count = 0;
	temp_header.entity_count = 0;
	
	
	for(i = 0; i < brush_count; i++)
	{
		if(brushes[i].type == BRUSH_INVALID)
			continue;
		
		if(brushes[i].type == BRUSH_BOUNDS)
			continue;	
			
		temp_header.brush_count++;
		vertex_count += brushes[i].vertex_count;
		polygon_count += brushes[i].polygon_count;
	}
	
	for(i = 0; i < light_count; i++)
	{
		if(light_params[i].bm_flags & LIGHT_INVALID)
			continue;
		
		temp_header.light_count++;	
	}
	
	for(i = 0; i < spawn_point_count; i++)
	{
		if(spawn_points[i].bm_flags & SPAWN_POINT_INVALID)
			continue;
			
		temp_header.spawn_point_count++;	
	}
	
	for(i = 0; i < texture_count; i++)
	{
		if(!textures[i].gl_handle)	
			continue;
		
		temp_header.texture_count++;
	}
	
	for(i = 0; i < material_count; i++)
	{
		if(materials[i].flags & MATERIAL_INVALID)
			continue;
			
		temp_header.material_count++;
	}
	
	for(i = 0; i < ent_entity_def_list_cursor; i++)
	{
		if(ent_entity_defs[i].type == ENTITY_TYPE_INVALID)
			continue;
			
		temp_header.entity_def_count++;	
	}
	
	for(i = 0; i < ent_entity_list_cursor; i++)
	{
		if(ent_entities[i].flags & ENTITY_INVALID)
			continue;
			
		temp_header.entity_count++;	
	}
	 
	
	file_size = (sizeof(proj_header_t)) + 
				(sizeof(material_record_size_t) + strlen(proj_file_tags_str[MATERIAL_RECORD]) + 1) * temp_header.material_count + 	
				(sizeof(light_record_size_t) + strlen(proj_file_tags_str[LIGHT_RECORD]) + 1) * 		temp_header.light_count + 
				(sizeof(texture_record_size_t) + strlen(proj_file_tags_str[TEXTURE_RECORD]) + 1) * temp_header.texture_count +
				(sizeof(spawn_point_record_size_t) + strlen(proj_file_tags_str[SPAWN_POINT_RECORD]) + 1) * temp_header.spawn_point_count +
				(sizeof(brush_record_size_t) + strlen(proj_file_tags_str[BRUSH_RECORD]) + 1) * temp_header.brush_count + 
				(sizeof(polygon_record_size_t)  + strlen(proj_file_tags_str[POLYGON_RECORD]) + 1) +
				(sizeof(model_record_size_t) * strlen(proj_file_tags_str[MODEL_RECORD]) + 1) * temp_header.model_count + 
				(sizeof(entity_def_record_size_t) * strlen(proj_file_tags_str[ENTITY_DEF_RECORD]) + 1) * temp_header.entity_def_count + 
				(sizeof(entity_record_size_t) * strlen(proj_file_tags_str[ENTITY_RECORD]) + 1) * temp_header.entity_count +
				(sizeof(vertex_t)) * vertex_count;
	
	file_buffer = calloc(file_size, 1);
	
	
	header = (proj_header_t *)file_buffer;
	file_buffer_cursor += sizeof(proj_header_t);
	
	//*header = temp_header;
	header->wtf0 = WTF_CONSTANT0;
	header->wtf1 = WTF_CONSTANT1;
	header->version = PROJ_VERSION;
	
	header->brush_count = temp_header.brush_count;
	header->light_count = temp_header.light_count;
	header->material_count = temp_header.material_count;
	header->spawn_point_count = temp_header.spawn_point_count;
	header->camera_count = temp_header.camera_count;
	header->texture_count = temp_header.texture_count;
	header->model_count = temp_header.model_count;
	header->entity_def_count = temp_header.entity_def_count;
	header->entity_count = temp_header.entity_count;
	
	header->reserved0 = 0;
	header->reserved1 = 0;
	header->reserved2 = 0;
	header->reserved3 = 0;
	header->reserved4 = 0;
	header->reserved5 = 0;
	header->reserved6 = 0;
	header->reserved7 = 0;
	header->reserved8 = 0;
	header->reserved9 = 0;
	header->reserved10 = 0;
	header->reserved11 = 0;
	header->reserved12 = 0;
	header->reserved13 = 0;
	header->reserved14 = 0;
	header->reserved15 = 0;
	
	
	
	//fwrite(&header, sizeof(proj_header_t), 1, file);
	
	for(i = 0; i < texture_count; i++)
	{
		if(!textures[i].gl_handle)
			continue;
		
		editor_WriteTag(file_buffer, &file_buffer_cursor, TEXTURE_RECORD);
			
		texture_record = (texture_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(texture_record_t);	
					
		texture_record->bm_texture_flags = textures[i].bm_flags & (~TEXTURE_COPY);
		
	
		strcpy(file_buffer + file_buffer_cursor, texture_info[i].name);
		file_buffer_cursor += strlen(texture_info[i].name) + 1;
		
		strcpy(file_buffer + file_buffer_cursor, texture_info[i].file_name);
		file_buffer_cursor += strlen(texture_info[i].file_name) + 1;
	}
	
	for(i = 0; i < material_count; i++)
	{
		
		if(materials[i].flags & MATERIAL_INVALID)
			continue;
		
		editor_WriteTag(file_buffer, &file_buffer_cursor, MATERIAL_RECORD);
		
		material_record = (material_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(material_record_t );
		
		
		strcpy(material_record->name, material_names[i]);
		file_buffer_cursor += strlen(material_names[i]) + 1;		
		
		material_record->base.r = (float)materials[i].r / 255.0;
		material_record->base.g = (float)materials[i].g / 255.0;
		material_record->base.b = (float)materials[i].b / 255.0;
		material_record->base.a = (float)materials[i].a / 255.0;
		
		material_record->bm_flags = 0;
		
		if(materials[i].diffuse_texture > -1)
		{
			material_record->bm_flags |= MATERIAL_USE_DIFFUSE_TEXTURE;
		}
		
		if(materials[i].normal_texture > -1)
		{
			material_record->bm_flags |= MATERIAL_USE_NORMAL_TEXTURE;
		}
				
		if(material_record->bm_flags & MATERIAL_USE_DIFFUSE_TEXTURE)
		{
			strcpy(file_buffer + file_buffer_cursor, texture_info[materials[i].diffuse_texture].name);
			file_buffer_cursor += strlen(texture_info[materials[i].diffuse_texture].name) + 1;
		}
		
		if(material_record->bm_flags & MATERIAL_USE_NORMAL_TEXTURE)
		{
			strcpy(file_buffer + file_buffer_cursor, texture_info[materials[i].normal_texture].name);
			file_buffer_cursor += strlen(texture_info[materials[i].normal_texture].name) + 1;
		}
		
		
	}
	
	for(i = 0; i < brush_count; i++)
	{
		brush = &brushes[i];	
		
		if(brush->type == BRUSH_INVALID)
			continue;
			
		if(brush->type == BRUSH_BOUNDS)
			continue;	
	
		editor_WriteTag(file_buffer, &file_buffer_cursor, BRUSH_RECORD);
		
		
		brush_record = (brush_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(brush_record_t);
			
		
		brush_record->orientation = brush->orientation;
		brush_record->position = brush->position;
		brush_record->scale = brush->scale;
		brush_record->type = brush->type;
		brush_record->triangle_group_count = brush->triangle_group_count;
		brush_record->vertex_count = brush->vertex_count;
		brush_record->polygon_count = brush->polygon_count;
		
		memcpy(file_buffer + file_buffer_cursor, brush->vertices, sizeof(vertex_t) * brush->vertex_count);
		
		file_buffer_cursor += sizeof(vertex_t ) * brush->vertex_count;
		
		for(j = 0; j < brush->polygon_count; j++)
		{
		
			editor_WriteTag(file_buffer, &file_buffer_cursor, POLYGON_RECORD);
			
			
			polygon_record = (polygon_record_t *)(file_buffer + file_buffer_cursor);
			file_buffer_cursor += sizeof(polygon_record_t );
			
			polygon = brush->polygons + j;
			polygon_record->vert_count = polygon->vert_count;
			polygon_record->normal = polygon->normal;
			polygon_record->first_index_offset = polygon->vertices - brush->vertices;
			
			strcpy(polygon_record->material_name, material_names[polygon->material_index]);
			file_buffer_cursor += strlen(material_names[polygon->material_index]) + 1;			
		}
	}
	
	for(i = 0; i < light_count; i++)
	{
		if(light_params[i].bm_flags & LIGHT_INVALID)
			continue;
		
		editor_WriteTag(file_buffer, &file_buffer_cursor, LIGHT_RECORD);
		
		
		light_record = (light_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(light_record_t);
			
			
		light_record->orientation = light_positions[i].orientation;
		light_record->position = light_positions[i].position;
		light_record->color.r = (float)light_params[i].r / 255.0;
		light_record->color.g = (float)light_params[i].g / 255.0;
		light_record->color.b = (float)light_params[i].b / 255.0;
		light_record->energy = LIGHT_ENERGY(light_params[i].energy);
		light_record->radius = LIGHT_RADIUS(light_params[i].radius);
		light_record->type = 0;
		light_record->bm_flags = light_params[i].bm_flags;
		
		strcpy(light_record->name, light_names[i]);
		file_buffer_cursor += strlen(light_names[i]) + 1;
	}
	
	
	for(i = 0; i < spawn_point_count; i++)
	{
		if(spawn_points[i].bm_flags & SPAWN_POINT_INVALID)
			continue;
		
		editor_WriteTag(file_buffer, &file_buffer_cursor, SPAWN_POINT_RECORD);
		
		
		spawn_point_record = (spawn_point_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(spawn_point_record_t );
		
		
		spawn_point_record->position = spawn_points[i].position;
		strcpy(spawn_point_record->name, spawn_points[i].name);
		
		file_buffer_cursor += strlen(spawn_points[i].name) + 1;
	}
	
	for(i = 0; i < ent_entity_def_list_cursor; i++)
	{
		if(ent_entity_defs[i].type == ENTITY_TYPE_INVALID)
			continue;
			
		editor_WriteTag(file_buffer, &file_buffer_cursor, ENTITY_DEF_RECORD);
		
		entity_def_record = (entity_def_record_t *)(file_buffer + file_buffer_cursor);
		entity_def_record->type = ent_entity_defs[i].type;
		entity_def_record->flags = ent_entity_defs[i].flags;
		
		file_buffer_cursor += sizeof(entity_def_record_t);
		
		strcpy(file_buffer + file_buffer_cursor, ent_entity_defs[i].name);
		file_buffer_cursor += strlen(ent_entity_defs[i].name) + 1;
		
		model = model_GetModelPointerIndex(ent_entity_defs[i].model_index);

		strcpy(file_buffer + file_buffer_cursor, model->name);
		file_buffer_cursor += strlen(model->name);	
	}
	
	for(i = 0; i < ent_entity_list_cursor; i++)
	{
		if(ent_entities[i].flags & ENTITY_INVALID)
			continue;
		
		editor_WriteTag(file_buffer, &file_buffer_cursor, ENTITY_RECORD);
		
		
		entity_record = (entity_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(entity_record_t);
	
		entity_record->orientation = ent_entities[i].orientation;
		entity_record->position = ent_entities[i].position;
		entity_record->scale = ent_entities[i].scale;
		entity_record->flags = ent_entities[i].flags;
		
		
		strcpy(file_buffer + file_buffer_cursor, ent_entities[i].name);
		file_buffer_cursor += strlen(ent_entities[i].name) + 1;
		
		model = model_GetModelPointerIndex(ent_entities[i].model_index);
		
		strcpy(file_buffer + file_buffer_cursor, model->name);
		file_buffer_cursor += strlen(model->name) + 1;
	}
	
	
	
	fwrite(file_buffer, file_buffer_cursor, 1, file);
	fflush(file);
	fclose(file);

	//fclose(file);
	
	b_project_dirty = 1;
	
}

int editor_OpenProject(char *file_name)
{
	#if 1
	//printf("editor_OpenProject\n");
	FILE *file;
	int i;
	int j;
	int k;
	int l;
	proj_header_t *header;
	brush_record_t *brush_record;
	light_record_t *light_record;
	camera_record_t camera_record;
	material_record_t *material_record;
	triangle_group_record_t triangle_group_record;
	triangle_group_t *triangle_group;
	polygon_record_t *polygon_record;
	spawn_point_record_t *spawn_point_record;
	texture_record_t *texture_record;
	brush_t *brush;
	brush_t *brush2;
	vertex_t *vertices;
	material_t *material;
	short shader_index;
	short material_index;
	short diffuse_texture_index;
	short normal_texture_index;
	int count;
	char name[BSP_MAX_NAME_LEN];
	char name2[BSP_MAX_NAME_LEN];
	char full_path[512];
	char *file_buffer;
	unsigned long long file_size = 0;
	unsigned int file_buffer_cursor = 0;
	
	
	if(!file_name[0])
		return;
		
	l = strlen(file_name);
	
	
	for(i = l; i > 0; i--)
	{
		if(file_name[i] == '.')
		{
			break;
		}
	}
	
	/* we probably just received the name of the project,
	without extension, which means we need to look for a 
	folder... */
	if(!i)
	{
		strcpy(full_path, file_name);
		strcat(full_path, "/");
		strcat(full_path, file_name);
		strcat(full_path, ".wtf");
	}
	else
	{
		strcpy(full_path, file_name);
	}

	file = fopen(full_path, "rb");
		
	if(!file)
	{
		printf("couldn't open project [%s]!\n", file_name);
		return 0;
	}
	
	
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);
	
	file_buffer = calloc(file_size, 1);
	fread(file_buffer, file_size, 1, file);
	fclose(file);
	
	
	//fread(&header, sizeof(proj_header_t), 1, file);
	
	header = (proj_header_t *)file_buffer;
	
	file_buffer_cursor += sizeof(proj_header_t );
	
	if(header->wtf0 != WTF_CONSTANT0 || header->wtf1 != WTF_CONSTANT1)
	{
		printf("editor_OpenProject: invalid project file!\n");
		fclose(file);
		return 0;
	}
	
	
	editor_ClearSelection();
	editor_CloseProject();	
	
	editor_SetProjectName(file_name);
	path_SetDir(absolute_path_to_project);
	
	strcpy(full_path, absolute_path_to_project);
	strcat(full_path, "/textures");
	path_AddSearchPath(full_path, SEARCH_PATH_TEXTURE);
	
	strcpy(full_path, absolute_path_to_project);
	strcat(full_path, "/sounds");
	path_AddSearchPath(full_path, SEARCH_PATH_SOUND);
	
	strcpy(full_path, absolute_path_to_project);
	strcat(full_path, "/models");
	path_AddSearchPath(full_path, SEARCH_PATH_MODEL);
	
	while(file_buffer_cursor < file_size)
	{
		if(file_buffer[file_buffer_cursor] == '[')
		{
			file_buffer_cursor++;
			
			if(!strcmp(file_buffer + file_buffer_cursor, proj_file_tags_str[MATERIAL_RECORD] + 1))
			{
				file_buffer_cursor += strlen(proj_file_tags_str[MATERIAL_RECORD]);
				
				material_record = (material_record_t *)(file_buffer + file_buffer_cursor);
				file_buffer_cursor += sizeof(material_record_t) + strlen(material_record->name) + 1;	

				if(material_record->bm_flags & MATERIAL_USE_CUSTOM_SHADER)
				{
					strcpy(name, file_buffer + file_buffer_cursor);
					shader_index = shader_GetShaderIndex(name);
					file_buffer_cursor += strlen(name) + 1;
				}
				else
				{
					shader_index = shader_GetShaderIndex("forward_pass");
				}
	
				diffuse_texture_index = -1;
				if(material_record->bm_flags & MATERIAL_USE_DIFFUSE_TEXTURE)
				{
					//fread(name, 64, 1, file);
					strcpy(name, file_buffer + file_buffer_cursor);
					diffuse_texture_index = texture_GetTexture(name);
					
					file_buffer_cursor += strlen(name) + 1;
				}
				
				normal_texture_index = -1;
				if(material_record->bm_flags & MATERIAL_USE_NORMAL_TEXTURE)
				{
					//fread(name, 64, 1, file);
					strcpy(name, file_buffer + file_buffer_cursor);
					normal_texture_index = texture_GetTexture(name);
					
					file_buffer_cursor += strlen(name) + 1;
				}
				
				material_CreateMaterial(material_record->name, material_record->base, 1.0, 1.0, shader_index, diffuse_texture_index, normal_texture_index);
				continue;
			}
			 
			else if(!strcmp(file_buffer + file_buffer_cursor, proj_file_tags_str[TEXTURE_RECORD] + 1))   
			{
				//file_buffer_cursor += 16;
				file_buffer_cursor += strlen(proj_file_tags_str[TEXTURE_RECORD]);
				
				texture_record = (texture_record_t *)(file_buffer + file_buffer_cursor);
				file_buffer_cursor += sizeof(texture_record_t );
				
				strcpy(name, file_buffer + file_buffer_cursor);
				file_buffer_cursor += strlen(name) + 1;
				
				strcpy(name2, file_buffer + file_buffer_cursor);
				file_buffer_cursor += strlen(name2) + 1;
				
				texture_LoadTexture(name2, name, texture_record->bm_texture_flags);
				 
				continue;
			}		
			else if(!strcmp(file_buffer + file_buffer_cursor, proj_file_tags_str[BRUSH_RECORD] + 1))
			{
				//file_buffer_cursor += 14;
				file_buffer_cursor += strlen(proj_file_tags_str[BRUSH_RECORD]);
				
				
				j = brush_CreateEmptyBrush();
				brush = &brushes[j];
				
				//fread(&brush_record, sizeof(brush_record_t), 1, file);
				
				brush_record = (brush_record_t *)(file_buffer + file_buffer_cursor);
				
				file_buffer_cursor += sizeof(brush_record_t);
				
				brush->vertices = malloc(sizeof(vertex_t) * brush_record->vertex_count);
				brush->vertex_count = brush_record->vertex_count;
				brush->max_vertexes = brush_record->vertex_count + 512;
				
				brush->polygon_count = brush_record->polygon_count;
				brush->polygons = malloc(sizeof(bsp_polygon_t) * brush_record->polygon_count);
				
				brush->orientation = brush_record->orientation;
				brush->position = brush_record->position;
				brush->scale = brush_record->scale;
				brush->type = brush_record->type;
				brush->max_intersections = 4096;
				brush->intersections = calloc(sizeof(int) * brush->max_intersections, 1);
				brush->bm_flags = BRUSH_MOVED | BRUSH_CLIP_POLYGONS;
				
				
				memcpy(brush->vertices, file_buffer + file_buffer_cursor, sizeof(vertex_t) * brush_record->vertex_count);
				file_buffer_cursor += sizeof(vertex_t ) * brush_record->vertex_count;
					
				for(k = 0; k < brush_record->polygon_count; k++)
				{
					file_buffer_cursor += strlen(proj_file_tags_str[POLYGON_RECORD]) + 1;
					
					polygon_record = (polygon_record_t *)(file_buffer + file_buffer_cursor);
					file_buffer_cursor += sizeof(polygon_record_t);
					
					brush->polygons[k].next = brush->polygons + k + 1;
					brush->polygons[k].b_used = 0;
					brush->polygons[k].normal = polygon_record->normal;
					brush->polygons[k].vert_count = polygon_record->vert_count;
					brush->polygons[k].vertices = brush->vertices + polygon_record->first_index_offset;
					
					strcpy(name, polygon_record->material_name);
					
					//printf("before\n");
					brush->polygons[k].material_index = material_MaterialIndex(name);
					//printf("get polygon material index\n");	
					
					file_buffer_cursor += strlen(name) + 1;
				}
				
				brush->polygons[k - 1].next = NULL;
				
				bsp_TriangulatePolygonsIndexes(brush->polygons, &brush->indexes, &brush->index_count);
					
				brush->handle = gpu_Alloc(sizeof(vertex_t) * brush->max_vertexes);
				brush->start = gpu_GetAllocStart(brush->handle) / sizeof(vertex_t);
				//gpu_Write(brush->handle, 0, brush->vertices, sizeof(vertex_t) * brush->vertex_count, 0);
			
				brush_UploadBrushVertices(brush);
			
				/* those initializations should go somewhere else (brush_gl.h)... */
				glGenBuffers(1, &brush->element_buffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brush->element_buffer);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * brush->index_count, NULL, GL_DYNAMIC_DRAW);
				//printf("%d\n", i);
				
				
				brush_BuildTriangleGroups(brush);
				brush_UpdateBrushElementBuffer(brush);
				brush_BuildBrushBsp(brush);
					
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				continue;
			}
			else if(!strcmp(file_buffer + file_buffer_cursor, proj_file_tags_str[LIGHT_RECORD] + 1))
			{
				file_buffer_cursor += strlen(proj_file_tags_str[LIGHT_RECORD]);
				
				light_record = (light_record_t *)(file_buffer + file_buffer_cursor);
				file_buffer_cursor += sizeof(light_record_t) + strlen(light_record->name) + 1;		
				light_CreateLight(light_record->name, &light_record->orientation, light_record->position, light_record->color, light_record->radius, light_record->energy, light_record->bm_flags);
				
				continue;
			}
			else if(!strcmp(file_buffer + file_buffer_cursor, proj_file_tags_str[SPAWN_POINT_RECORD] + 1))
			{
				file_buffer_cursor += strlen(proj_file_tags_str[SPAWN_POINT_RECORD]);
				
				spawn_point_record = (spawn_point_record_t *)(file_buffer + file_buffer_cursor);
				file_buffer_cursor += sizeof(spawn_point_record_t) + strlen(spawn_point_record->name) + 1;
				player_CreateSpawnPoint(spawn_point_record->position, spawn_point_record->name);
				
				continue;
			}
			
		}
		
		file_buffer_cursor++;
	}
	
	

	
	free(file_buffer);
	
	return 1;
	
	#endif
}


void editor_CopyProjectResources(char *base_path)
{
	int i;
	FILE *file;
	unsigned long long file_size;
	char *file_buffer;
	
	char full_path[512];
	
	for(i = 0; i < texture_count; i++)
	{
		if(!textures[i].gl_handle)
		{
			continue;
		}
		
		//if(!(textures[i].bm_flags & TEXTURE_COPY))
		//	continue;
			
		file = fopen(texture_info[i].full_path, "rb");
		
		fseek(file, 0, SEEK_END);
		file_size = ftell(file);
		rewind(file);
		
		file_buffer = malloc(file_size);
		fread(file_buffer, file_size, 1, file);
		fclose(file);
		
		strcpy(full_path, base_path);
		strcat(full_path, "/textures/");
		strcat(full_path, texture_info[i].file_name);
		
		
		file = fopen(full_path, "wb");
		fwrite(file_buffer, file_size, 1, file);
		fflush(file);
		fclose(file);
		
		strcpy(texture_info[i].full_path, "textures/");
		
		free(file_buffer);
	}
	
}


void editor_SetProjectName(char *name)
{
	
	int ext_index = 0;
	int name_start_index = 0;
	char *s = name;
	int b_found_point;
	int name_len;
	
	if(!name[0])
		return;
	
	if(name[0] == '.')
		return;
		
		
	
	name_len = strlen(name);
	
	
	
	ext_index = name_len;
	while(s[ext_index] != '.' && ext_index > 0)
	{
		ext_index--;
	}
	
	name_start_index = name_len;
	while(s[name_start_index] != '/' && s[name_start_index] != '\\' && name_start_index > 0)
	{
		name_start_index--;
	}
	
	
	
	
		
	/* no extension, which means we received a folder name... */
	if(!ext_index)
	{
		/* append some stuff... */
		strcpy(current_project_name, name);
		strcat(current_project_name, "/");
		if(!name_start_index)
		{
			name_start_index = -1;
		}
		strcat(current_project_name, name + name_start_index + 1);
		
		
	}
	else
	{
		/* copy the name until before the extension '.' ... */
		memcpy(current_project_name, name, ext_index);
		current_project_name[ext_index] = '\0';
	}
	
	strcat(current_project_name, ".wtf");
	
	name_start_index = name_len;
	while(current_project_name[name_start_index] != '/' && current_project_name[name_start_index] != '\\' && name_start_index > 0)
	{
		name_start_index--;
	}
	
	
	/* no slashed path, which means this file is being saved in the same
	folder as the executable... */
	if(!name_start_index)
	{
		absolute_path_to_project[0] = '\0';
	}
	else
	{
		memcpy(absolute_path_to_project, current_project_name, name_start_index);
		absolute_path_to_project[name_start_index] = '\0';
	}
		
	
		
	
	
		
	//#if defined (__WIN32__)
	
	ext_index = 0;
	while(current_project_name[ext_index])
	{
		if(current_project_name[ext_index] == '/')
		{
			current_project_name[ext_index] = '\\';
		}
		ext_index++;
	}
	
	ext_index = 0;
	while(absolute_path_to_project[ext_index])
	{
		if(absolute_path_to_project[ext_index] == '/')
		{
			absolute_path_to_project[ext_index] = '\\';
		}
		ext_index++;
	}
	
	//#endif
	
	
	SDL_SetWindowTitle(window, current_project_name);
	
	//printf("%s %s\n", current_project_name, absolute_path_to_project);
}

//char project_name[512];

char *editor_GetProjectName()
{
	/*int i = 0;
	char *s = current_project_name;
	int name_len;
	
	name_len = strlen(current_project_name);
	
	
	i = name_len;
	while(s[i] != '.' && i > 0)
	{
		i--;
	}
	
	memcpy(project_name, current_project_name, name_len);
	
	if(!i)
	{
		i = name_len;
	}
	
	project_name[i] = '\0';
	printf("%s\n", project_name);
	
	return project_name;*/
}

void editor_CloseProject()
{
	if(editor_IsProjectDirty())
	{
	
	}
	
	light_DestroyAllLights();
	brush_DestroyAllBrushes();
	material_DestroyAllMaterials();
	camera_DestroyAllCameras();
	player_DestroyAllSpawnPoints();
	bsp_DeleteBsp();
	
	
	editor_RestartEditor();	
	
	
}

void editor_ExportBsp(char *file_name)
{
	#if 0
	FILE *file;
	bsp_header_t header;
	light_record_t light_record;
	triangle_group_record_t triangle_group_record;
	material_record_t material_record;
	int i;
	int j;
	
	int start;
	int count;
	
	
	char name[BSP_FILE_MAX_NAME_LEN];
	
	char attrib_name[MAX_NAME_LEN];
	
	if(b_compiling || b_calculating_pvs)
	{
		printf("editor_ExportBsp: map not compiled!\n");
		return;
	}
	
	i = 0;
	
	
	if(strlen(file_name) + 1 >= BSP_FILE_MAX_NAME_LEN)
	{
		/* truncate 5 chars away from the max, so there's space for
		the extension (.bsp) + a trailing null... */
		file_name[BSP_FILE_MAX_NAME_LEN - 5] = '\0';
	}
	
	strcpy(name, file_name);
	
	while(name[i] != '.' && name[i] != '\0') i++;
	
	/* remove whatever bizarre extension that might come... */
	if(name[i] == '.')
	{
		name[i] = '\0';
	}
	
	strcat(name, ".bsp");
	file = fopen(name, "wb");
	
	
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
		material_record.base.r = (float)materials[i].r / 255.0;
		material_record.base.g = (float)materials[i].g / 255.0;
		material_record.base.b = (float)materials[i].b / 255.0;
		material_record.base.a = (float)materials[i].a / 255.0;
		
		material_record.bm_textures = 0;
		
		if(materials[i].diffuse_texture != -1)
		{
			material_record.bm_textures |= 1;
		}
		
		if(materials[i].normal_texture != -1)
		{
			material_record.bm_textures |= 2;
		}
		
		fwrite(&material_record, sizeof(material_record_t), 1, file);
		
		if(material_record.bm_textures & 1)
		{
			for(j = 0; j < MAX_NAME_LEN; j++)
			{
				attrib_name[j] = 0;
			}
				
			strcpy(attrib_name, texture_info[materials[i].diffuse_texture].file_name);
			
			fwrite(attrib_name, MAX_NAME_LEN, 1, file);
		}
		
		if(material_record.bm_textures & 2)
		{
			for(j = 0; j < MAX_NAME_LEN; j++)
			{
				attrib_name[j] = 0;
			}
				
			strcpy(attrib_name, texture_info[materials[i].normal_texture].file_name);
			
			fwrite(attrib_name, MAX_NAME_LEN, 1, file);
		}
	}
	
	for(i = 0; i < light_count; i++)
	{
		light_record.orientation = light_positions[i].orientation;
		light_record.position = light_positions[i].position;
		light_record.color.r = (float)light_params[i].r / 255.0;
		light_record.color.g = (float)light_params[i].g / 255.0;
		light_record.color.b = (float)light_params[i].b / 255.0;
		light_record.energy = LIGHT_ENERGY(light_params[i].energy);
		light_record.radius = LIGHT_RADIUS(light_params[i].radius);
		light_record.type = 0;
		
		fwrite(&light_record, sizeof(light_record_t), 1, file);
	}
	
	fwrite(world_nodes, sizeof(bsp_pnode_t), world_nodes_count, file);
	fwrite(world_leaves, sizeof(bsp_dleaf_t), world_leaves_count, file);
	fwrite(collision_nodes, sizeof(bsp_pnode_t), collision_nodes_count, file);
	
	for(i = 0; i < world_triangle_group_count; i++)
	{
		for(j = 0; j < MAX_NAME_LEN; j++)
		{
			triangle_group_record.material_name[j] = 0;
		}
		
		start = world_triangle_groups[i].start;
		count = world_triangle_groups[i].next;
		
		triangle_group_record.vertice_count = count;
		
		
		strcpy(triangle_group_record.material_name, material_names[world_triangle_groups[i].material_index]);
		fwrite(&triangle_group_record, sizeof(triangle_group_record_t), 1, file);
		
		fwrite(&world_vertices[start], sizeof(vertex_t), count, file);		
	}
	
	fclose(file);
	
	#endif
}

int editor_IsProjectDirty()
{
	return b_project_dirty;
}

void editor_DirtyProject()
{
	b_project_dirty = 1;
}

void editor_CleanProject()
{
	b_project_dirty = 0;
}

char *editor_GetAbsolutePathToProject()
{
	return absolute_path_to_project;
}






