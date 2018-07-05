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

int ed_search_paths_set = 0;
char ed_full_project_name[512];
char ed_full_path_to_project_file_folder[512];
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
extern int mat_material_count;
extern material_t *mat_materials;
extern char **mat_material_names;

/* from texture.c */
extern int texture_count;
extern texture_t *textures;
extern texture_info_t *texture_info;

extern int mdl_model_list_cursor;
extern model_t *mdl_models;

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
	#if 0
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
	//entity_def_record_t *entity_def_record;
	//entity_record_t *entity_record;
	model_record_t *model_record;
	
	
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
	unsigned int brush_buffer_size = 0;
	
	//char *project_name;
	
	file = fopen(ed_full_project_name, "rb");
	strcpy(full_project_path, ed_full_path_to_project_file_folder);
	
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
		#else
		
		#endif
	}
	else
	{
		
		path_SetDir(ed_full_path_to_project_file_folder);
		
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
	
	
	/*if(!ed_search_paths_set)
	{
		
		strcpy(full_path, ed_full_path_to_project_file_folder);
		
		path_AddSearchPath(full_path, SEARCH_PATH_TEXTURE);
		path_AddSearchPath(full_path, SEARCH_PATH_SOUND);
		path_AddSearchPath(full_path, SEARCH_PATH_MODEL);
		
		strcat(full_path, "/textures");
		path_AddSearchPath(full_path, SEARCH_PATH_TEXTURE);
		
		strcpy(full_path, ed_full_path_to_project_file_folder);
		strcat(full_path, "/sounds");
		path_AddSearchPath(full_path, SEARCH_PATH_SOUND);
		
		strcpy(full_path, ed_full_path_to_project_file_folder);
		strcat(full_path, "/models");
		path_AddSearchPath(full_path, SEARCH_PATH_MODEL);
		
		ed_search_paths_set = 1;
	}*/
	
	
	//editor_CopyProjectResources(ed_full_path_to_project_file_folder);
	strcpy(full_project_path, ed_full_project_name);
	file = fopen(full_project_path, "wb");
	 
	temp_header.wtf0 = WTF_CONSTANT0;
	temp_header.wtf1 = WTF_CONSTANT1;
	temp_header.version = PROJ_VERSION;
	temp_header.brush_count = brush_count;
	temp_header.light_count = 0;
	temp_header.camera_count = camera_count;
	temp_header.material_count = 0;
	temp_header.spawn_point_count = 0;
	temp_header.texture_count = 0;
	temp_header.model_count = 0;
	temp_header.entity_def_count = 0;
	temp_header.entity_count = 0;
	
	
	/*for(i = 0; i < brush_count; i++)
	{
		if(brushes[i].type == BRUSH_INVALID)
			continue;
		
		if(brushes[i].type == BRUSH_BOUNDS)
			continue;	
			
		temp_header.brush_count++;
		vertex_count += brushes[i].base_polygons_vert_count;
		polygon_count += brushes[i].base_polygons_count;
	}*/
	
	/*brush = brushes;
	
	while(brush)
	{
		vertex_count += brush->base_polygons_vert_count;
		polygon_count += brush->base_polygons_vert_count;
		brush = brush->next;
	}*/
	
//	brush_buffer_size = brush_CalculateSerializationSize();
	
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
	
	for(i = 0; i < mat_material_count; i++)
	{
		if(mat_materials[i].flags & MATERIAL_INVALID)
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
				brush_buffer_size + 
				//(sizeof(polygon_record_size_t)  + strlen(proj_file_tags_str[POLYGON_RECORD]) + 1) +
				(sizeof(model_record_size_t) * strlen(proj_file_tags_str[MODEL_RECORD]) + 1) * temp_header.model_count + 
				(sizeof(entity_def_record_size_t) * strlen(proj_file_tags_str[ENTITY_DEF_RECORD]) + 1) * temp_header.entity_def_count + 
				(sizeof(entity_record_size_t) * strlen(proj_file_tags_str[ENTITY_RECORD]) + 1) * temp_header.entity_count;
	
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
	
	for(i = 0; i < mat_material_count; i++)
	{
		
		if(mat_materials[i].flags & MATERIAL_INVALID)
			continue;
		
		editor_WriteTag(file_buffer, &file_buffer_cursor, MATERIAL_RECORD);
		
		material_record = (material_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(material_record_t );
		
		
		strcpy(material_record->name, mat_material_names[i]);
		file_buffer_cursor += strlen(mat_material_names[i]) + 1;		
		
		material_record->base.r = (float)mat_materials[i].r / 255.0;
		material_record->base.g = (float)mat_materials[i].g / 255.0;
		material_record->base.b = (float)mat_materials[i].b / 255.0;
		material_record->base.a = (float)mat_materials[i].a / 255.0;
		
		material_record->bm_flags = 0;
		
		if(mat_materials[i].diffuse_texture > -1)
		{
			material_record->bm_flags |= MATERIAL_USE_DIFFUSE_TEXTURE;
		}
		
		if(mat_materials[i].normal_texture > -1)
		{
			material_record->bm_flags |= MATERIAL_USE_NORMAL_TEXTURE;
		}
				
		if(material_record->bm_flags & MATERIAL_USE_DIFFUSE_TEXTURE)
		{
			strcpy(file_buffer + file_buffer_cursor, texture_info[mat_materials[i].diffuse_texture].name);
			file_buffer_cursor += strlen(texture_info[mat_materials[i].diffuse_texture].name) + 1;
		}
		
		if(material_record->bm_flags & MATERIAL_USE_NORMAL_TEXTURE)
		{
			strcpy(file_buffer + file_buffer_cursor, texture_info[mat_materials[i].normal_texture].name);
			file_buffer_cursor += strlen(texture_info[mat_materials[i].normal_texture].name) + 1;
		}	
	}
	
	for(i = 0; i < brush_count; i++)
	{
		brush = &brushes[i];	
		
		if(brush->type == BRUSH_INVALID)
			continue;
			
		if(brush->type == BRUSH_BOUNDS)
			continue;	
	
		//editor_WriteTag(file_buffer, &file_buffer_cursor, BRUSH_RECORD);
		
		
		brush_record = (brush_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(brush_record_t);
			
		
		brush_record->orientation = brush->orientation;
		brush_record->position = brush->position;
		brush_record->scale = brush->scale;
		brush_record->type = brush->type;
		brush_record->triangle_group_count = brush->batch_count;
		brush_record->vertex_count = brush->base_polygons_vert_count;
		brush_record->polygon_count = brush->base_polygons_count;
		
		memcpy(file_buffer + file_buffer_cursor, brush->base_polygons_vertices, sizeof(vertex_t) * brush->base_polygons_vert_count);
		
		file_buffer_cursor += sizeof(vertex_t ) * brush->base_polygons_vert_count;
		
		for(j = 0; j < brush->base_polygons_count; j++)
		{
		
			//editor_WriteTag(file_buffer, &file_buffer_cursor, POLYGON_RECORD);
			polygon_record = (polygon_record_t *)(file_buffer + file_buffer_cursor);
			file_buffer_cursor += sizeof(polygon_record_t );
			
			polygon = brush->base_polygons + j;
			polygon_record->vert_count = polygon->vert_count;
			polygon_record->normal = polygon->normal;
			polygon_record->first_index_offset = polygon->vertices - brush->base_polygons_vertices;
			
			strcpy(polygon_record->material_name, mat_material_names[polygon->material_index]);
			file_buffer_cursor += strlen(mat_material_names[polygon->material_index]) + 1;			
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
	
	for(i = 0; i < model_list_cursor; i++)
	{
		if(models[i].flags & MODEL_INVALID)
			continue;
			
		editor_WriteTag(file_buffer, &file_buffer_cursor, MODEL_RECORD);
		
		model_record = (model_record_t *)(file_buffer + file_buffer_cursor);
		model_record->flags = models[i].flags;
		
		file_buffer_cursor += sizeof(model_record_t);
				
		strcpy(file_buffer + file_buffer_cursor, models[i].file_name);
		file_buffer_cursor += strlen(models[i].file_name) + 1;
		
		strcpy(file_buffer + file_buffer_cursor, models[i].name);
		file_buffer_cursor += strlen(models[i].name) + 1;
			
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
		file_buffer_cursor += strlen(model->name) + 1;	
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
	
	#endif
	
}

int editor_OpenProject(char *file_name)
{
	#if 0
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
	entity_def_record_t *entity_def_record;
	entity_record_t *entity_record;
	model_record_t *model_record;
	
	brush_t *brush;
	brush_t *brush2;
	vertex_t *vertices;
	material_t *material;
	short shader_index;
	short material_index;
	int model_index;
	int entity_def_index;
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
	path_SetDir(ed_full_path_to_project_file_folder);
	
	strcpy(full_path, ed_full_path_to_project_file_folder);
	
	path_AddSearchPath(full_path, SEARCH_PATH_TEXTURE);
	path_AddSearchPath(full_path, SEARCH_PATH_SOUND);
	path_AddSearchPath(full_path, SEARCH_PATH_MODEL);
	
	strcat(full_path, "/textures");
	path_AddSearchPath(full_path, SEARCH_PATH_TEXTURE);
	
	strcpy(full_path, ed_full_path_to_project_file_folder);
	strcat(full_path, "/sounds");
	path_AddSearchPath(full_path, SEARCH_PATH_SOUND);
	
	strcpy(full_path, ed_full_path_to_project_file_folder);
	strcat(full_path, "/models");
	path_AddSearchPath(full_path, SEARCH_PATH_MODEL);
	
	ed_search_paths_set = 1;
	
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
				
				file_buffer_cursor += strlen(proj_file_tags_str[BRUSH_RECORD]);
				
				
				brush = brush_CreateEmptyBrush();

				
			
	
				brush_record = (brush_record_t *)(file_buffer + file_buffer_cursor);
				
				file_buffer_cursor += sizeof(brush_record_t);
		
				brush_InitializeBrush(brush, &brush_record->orientation, brush_record->position, brush_record->scale, brush_record->type, brush_record->vertex_count, brush_record->polygon_count);
				
				/*brush->orientation = brush_record->orientation;
				brush->position = brush_record->position;
				brush->scale = brush_record->scale;
				brush->type = brush_record->type;
				brush->bm_flags = BRUSH_MOVED | BRUSH_CLIP_POLYGONS;
				
				brush_AllocBaseVertices(brush, brush_record->vertex_count, (vertex_t *)(file_buffer + file_buffer_cursor));
				brush_AllocBasePolygons(brush, brush_record->polygon_count);*/
				
				//memcpy(brush->base_polygons_vertices, file_buffer + file_buffer_cursor, sizeof(vertex_t) * brush_record->vertex_count);
				file_buffer_cursor += sizeof(vertex_t) * brush_record->vertex_count;
					
				for(k = 0; k < brush_record->polygon_count; k++)
				{
					file_buffer_cursor += strlen(proj_file_tags_str[POLYGON_RECORD]) + 1;
					polygon_record = (polygon_record_t *)(file_buffer + file_buffer_cursor);
					file_buffer_cursor += sizeof(polygon_record_t);
					strcpy(name, polygon_record->material_name);
					
					brush_AddPolygonToBrush(brush, NULL, polygon_record->normal, polygon_record->vert_count, material_MaterialIndex(name));
					/*brush->base_polygons[k].next = brush->base_polygons + k + 1;
					brush->base_polygons[k].b_used = 0;
					brush->base_polygons[k].normal = polygon_record->normal;
					brush->base_polygons[k].vert_count = polygon_record->vert_count;
					brush->base_polygons[k].vertices = brush->base_polygons_vertices + polygon_record->first_index_offset;
					
					
					brush->base_polygons[k].material_index = material_MaterialIndex(name);*/
					file_buffer_cursor += strlen(name) + 1;
				}
				
				brush_LinkPolygonsToVertices(brush);
				brush_FinalizeBrush(brush, 0);
				//brush->base_polygons[k - 1].next = NULL;
				
				
				//brush->handle = gpu_Alloc(sizeof(vertex_t) * brush->max_vertexes);
				/*brush->handle = gpu_AllocAlign(sizeof(vertex_t) * brush->max_vertexes, sizeof(vertex_t));
				brush->start = gpu_GetAllocStart(brush->handle) / sizeof(vertex_t);
					
				glGenBuffers(1, &brush->element_buffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brush->element_buffer);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * brush->max_vertexes, NULL, GL_DYNAMIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				
				brush_BuildBrushBsp(brush);
				brush_BuildEdgeList(brush);*/
				
				
	
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
			else if(!strcmp(file_buffer + file_buffer_cursor, proj_file_tags_str[MODEL_RECORD] + 1))
			{
				file_buffer_cursor += strlen(proj_file_tags_str[MODEL_RECORD]);
				
				model_record = (model_record_t *)(file_buffer + file_buffer_cursor);
				file_buffer_cursor += sizeof(model_record_t);
				
				/* file name */
				strcpy(name, file_buffer + file_buffer_cursor);
				file_buffer_cursor += strlen(name) + 1;
				
				/* model name */
				strcpy(name2, file_buffer + file_buffer_cursor);
				file_buffer_cursor += strlen(name2) + 1;
				
				printf("%s\n", name);
				
				model_LoadModel(name, name2);
				continue;
				
			}
			else if(!strcmp(file_buffer + file_buffer_cursor, proj_file_tags_str[ENTITY_DEF_RECORD] + 1))
			{
				file_buffer_cursor += strlen(proj_file_tags_str[ENTITY_DEF_RECORD]);
				
				entity_def_record = (entity_def_record_t *)(file_buffer + file_buffer_cursor);
				file_buffer_cursor += sizeof(entity_def_record_t);
				
				/* entity def name */
				strcpy(name, file_buffer + file_buffer_cursor);
				file_buffer_cursor += strlen(name) + 1;
				
				/* model name */
				strcpy(name2, file_buffer + file_buffer_cursor);
				file_buffer_cursor += strlen(name2) + 1;
				
				model_index = model_GetModel(name2);
				
				entity_CreateEntityDef(name, ENTITY_TYPE_MOVABLE, model_index, NULL);
				continue;
			}
			else if(!strcmp(file_buffer + file_buffer_cursor, proj_file_tags_str[ENTITY_RECORD] + 1))
			{
				file_buffer_cursor += strlen(proj_file_tags_str[ENTITY_RECORD]);
				
				entity_record = (entity_record_t *)(file_buffer + file_buffer_cursor);
				file_buffer_cursor += sizeof(entity_record_t);
				
				/* entity name */
				strcpy(name, file_buffer + file_buffer_cursor);
				file_buffer_cursor += strlen(name) + 1;
				
				/* entity def name */
				strcpy(name2, file_buffer + file_buffer_cursor);
				file_buffer_cursor += strlen(name2) + 1;
				
				entity_def_index = entity_GetEntityDef(name2);
				
				entity_CreateEntity(name, entity_record->position, entity_record->scale, &entity_record->orientation, entity_def_index);
				
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
	#if 0
	int i;
	FILE *file;
	unsigned long long file_size;
	char *file_buffer;
	char *formated_path;
	
	char full_path[512];
	
	for(i = 0; i < texture_count; i++)
	{
		if(!textures[i].gl_handle)
		{
			continue;
		}
		
		//if(!(textures[i].bm_flags & TEXTURE_COPY))
		//	continue;
			
		file = fopen(texture_info[i].file_name, "rb");
	
		fseek(file, 0, SEEK_END);
		file_size = ftell(file);
		rewind(file);
		
		file_buffer = malloc(file_size);
		fread(file_buffer, file_size, 1, file);
		fclose(file);
		
		
		strcpy(full_path, base_path);
		strcat(full_path, "/textures/");
		strcat(full_path, path_GetFileNameFromPath(texture_info[i].file_name));
		formated_path = path_FormatPath(full_path);
		
		
		file = fopen(formated_path, "wb");
		fwrite(file_buffer, file_size, 1, file);
		fflush(file);
		fclose(file);
		
		strcpy(full_path, "textures/");
		strcat(full_path, path_GetFileNameFromPath(texture_info[i].file_name));
		formated_path = path_FormatPath(full_path);
		strcpy(texture_info[i].file_name, formated_path);
		
		free(file_buffer);
	}
	
	for(i = 0; i < model_list_cursor; i++)
	{
		if(models[i].flags & MODEL_INVALID)
			continue;
		
		file = fopen(models[i].file_name, "rb");
		
		fseek(file, 0, SEEK_END);
		file_size = ftell(file);
		rewind(file);
		
		
		file_buffer = malloc(file_size);
		fread(file_buffer, file_size, 1, file);
		fclose(file);
		
		
		strcpy(full_path, base_path);
		strcat(full_path, "/models/");
		strcat(full_path, path_GetFileNameFromPath(models[i].file_name));
		formated_path = path_FormatPath(full_path);
		
		file = fopen(full_path, "wb");
		fwrite(file_buffer, file_size, 1, file);
		fflush(file);
		fclose(file);
		
		strcpy(full_path, path_GetFileNameFromPath(models[i].file_name));
		formated_path = path_FormatPath(full_path);
		strcpy(models[i].file_name, formated_path);
		
		free(file_buffer);
	}
	
	#endif
	
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
		strcpy(ed_full_project_name, name);
		strcat(ed_full_project_name, "/");
		if(!name_start_index)
		{
			name_start_index = -1;
		}
		strcat(ed_full_project_name, name + name_start_index + 1);
		
		
	}
	else
	{
		/* copy the name until before the extension '.' ... */
		memcpy(ed_full_project_name, name, ext_index);
		ed_full_project_name[ext_index] = '\0';
	}
	
	strcat(ed_full_project_name, ".wtf");
	
	name_start_index = name_len;
	while(ed_full_project_name[name_start_index] != '/' && ed_full_project_name[name_start_index] != '\\' && name_start_index > 0)
	{
		name_start_index--;
	}
	
	
	/* no slashed path, which means this file is being saved in the same
	folder as the executable... */
	if(!name_start_index)
	{
		ed_full_path_to_project_file_folder[0] = '\0';
	}
	else
	{
		memcpy(ed_full_path_to_project_file_folder, ed_full_project_name, name_start_index);
		ed_full_path_to_project_file_folder[name_start_index] = '\0';
	}
		
	ext_index = 0;
	while(ed_full_project_name[ext_index])
	{
		if(ed_full_project_name[ext_index] == '\\')
		{
			ed_full_project_name[ext_index] = '/';
		}
		ext_index++;
	}
	
	ext_index = 0;
	while(ed_full_path_to_project_file_folder[ext_index])
	{
		if(ed_full_path_to_project_file_folder[ext_index] == '\\')
		{
			ed_full_path_to_project_file_folder[ext_index] = '/';
		}
		ext_index++;
	}
		
	SDL_SetWindowTitle(window, ed_full_project_name);
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
	
	/*light_DestroyAllLights();
	brush_DestroyAllBrushes();
	material_DestroyAllMaterials();
	camera_DestroyAllCameras();
	player_DestroyAllSpawnPoints();
	bsp_DeleteBsp();*/
	
	
	editor_RestartEditor();	
	
	
}

void editor_ExportBsp(char *file_name)
{
	#if 0
	FILE *file;
	int i;
	int j;
	int k;
	int l;
	bsp_header_t temp_header;

	
	bsp_header_t *header;
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
	model_record_t *model_record;
	
	
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
	
	/*file = fopen(ed_full_project_name, "rb");
	strcpy(full_project_path, ed_full_path_to_project_file_folder);
	
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
		#else
		
		#endif
	}
	else
	{
		
		path_SetDir(ed_full_path_to_project_file_folder);
		
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
	}*/
	
	
	//strcpy(full_project_path, ed_full_project_name);
	//file = fopen(full_project_path, "wb");
	 
	//temp_header.wtf0 = WTF_CONSTANT0;
//	temp_header.wtf1 = WTF_CONSTANT1;
	temp_header.version = BSP_FILE_VERSION;
	//temp_header.brush_count = 0;
	temp_header.light_count = 0;
//	temp_header.camera_count = 0;
	temp_header.material_count = 0;
	temp_header.spawn_point_count = 0;
	temp_header.texture_count = 0;
	temp_header.model_count = 0;
	temp_header.entity_def_count = 0;
	temp_header.entity_count = 0;
	temp_header.particle_emitter_count = 0;
	temp_header.collision_nodes_count = collision_nodes_count;
	temp_header.world_leaves_count = world_leaves_count;
	temp_header.world_nodes_count = world_nodes_count;
	temp_header.world_triangle_group_count = world_triangle_group_count;
	temp_header.world_vertice_count = world_vertices_count;
	
	
	/*for(i = 0; i < brush_count; i++)
	{
		if(brushes[i].type == BRUSH_INVALID)
			continue;
		
		if(brushes[i].type == BRUSH_BOUNDS)
			continue;	
			
		temp_header.brush_count++;
		vertex_count += brushes[i].base_polygons_vert_count;
		polygon_count += brushes[i].base_polygons_count;
	}*/
	
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
	
	for(i = 0; i < mat_material_count; i++)
	{
		if(mat_materials[i].flags & MATERIAL_INVALID)
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
				/*(sizeof(brush_record_size_t) + strlen(proj_file_tags_str[BRUSH_RECORD]) + 1) * temp_header.brush_count + */
				//(sizeof(polygon_record_size_t)  + strlen(proj_file_tags_str[POLYGON_RECORD]) + 1) +
				(sizeof(model_record_size_t) * strlen(proj_file_tags_str[MODEL_RECORD]) + 1) * temp_header.model_count + 
				(sizeof(entity_def_record_size_t) * strlen(proj_file_tags_str[ENTITY_DEF_RECORD]) + 1) * temp_header.entity_def_count + 
				(sizeof(entity_record_size_t) * strlen(proj_file_tags_str[ENTITY_RECORD]) + 1) * temp_header.entity_count +
				(sizeof(bsp_pnode_t) * temp_header.world_nodes_count) + 
				(sizeof(bsp_dleaf_t) * temp_header.world_leaves_count) + 
				(sizeof(bsp_pnode_t) * temp_header.collision_nodes_count);
				/*(sizeof())*/
				/*(sizeof(vertex_t)) * vertex_count;*/
	
	file_buffer = calloc(file_size, 1);
	
	
	header = (bsp_header_t *)file_buffer;
	file_buffer_cursor += sizeof(bsp_header_t);
	
	//*header = temp_header;
//	//header->wtf0 = WTF_CONSTANT0;
//	header->wtf1 = WTF_CONSTANT1;
	header->version = PROJ_VERSION;
	
	//header->brush_count = temp_header.brush_count;
	header->light_count = temp_header.light_count;
	header->material_count = temp_header.material_count;
	header->spawn_point_count = temp_header.spawn_point_count;
	//header->camera_count = temp_header.camera_count;
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
	
	for(i = 0; i < mat_material_count; i++)
	{
		
		if(mat_materials[i].flags & MATERIAL_INVALID)
			continue;
		
		editor_WriteTag(file_buffer, &file_buffer_cursor, MATERIAL_RECORD);
		
		material_record = (material_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(material_record_t );
		
		
		strcpy(material_record->name, mat_material_names[i]);
		file_buffer_cursor += strlen(mat_material_names[i]) + 1;		
		
		material_record->base.r = (float)mat_materials[i].r / 255.0;
		material_record->base.g = (float)mat_materials[i].g / 255.0;
		material_record->base.b = (float)mat_materials[i].b / 255.0;
		material_record->base.a = (float)mat_materials[i].a / 255.0;
		
		material_record->bm_flags = 0;
		
		if(mat_materials[i].diffuse_texture > -1)
		{
			material_record->bm_flags |= MATERIAL_USE_DIFFUSE_TEXTURE;
		}
		
		if(mat_materials[i].normal_texture > -1)
		{
			material_record->bm_flags |= MATERIAL_USE_NORMAL_TEXTURE;
		}
				
		if(material_record->bm_flags & MATERIAL_USE_DIFFUSE_TEXTURE)
		{
			strcpy(file_buffer + file_buffer_cursor, texture_info[mat_materials[i].diffuse_texture].name);
			file_buffer_cursor += strlen(texture_info[mat_materials[i].diffuse_texture].name) + 1;
		}
		
		if(material_record->bm_flags & MATERIAL_USE_NORMAL_TEXTURE)
		{
			strcpy(file_buffer + file_buffer_cursor, texture_info[mat_materials[i].normal_texture].name);
			file_buffer_cursor += strlen(texture_info[mat_materials[i].normal_texture].name) + 1;
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
		brush_record->triangle_group_count = brush->batch_count;
		brush_record->vertex_count = brush->base_polygons_vert_count;
		brush_record->polygon_count = brush->base_polygons_count;
		
		memcpy(file_buffer + file_buffer_cursor, brush->base_polygons_vertices, sizeof(vertex_t) * brush->base_polygons_vert_count);
		
		file_buffer_cursor += sizeof(vertex_t ) * brush->base_polygons_vert_count;
		
		for(j = 0; j < brush->base_polygons_count; j++)
		{
		
			editor_WriteTag(file_buffer, &file_buffer_cursor, POLYGON_RECORD);
			
			
			polygon_record = (polygon_record_t *)(file_buffer + file_buffer_cursor);
			file_buffer_cursor += sizeof(polygon_record_t );
			
			polygon = brush->base_polygons + j;
			polygon_record->vert_count = polygon->vert_count;
			polygon_record->normal = polygon->normal;
			polygon_record->first_index_offset = polygon->vertices - brush->base_polygons_vertices;
			
			strcpy(polygon_record->material_name, mat_material_names[polygon->material_index]);
			file_buffer_cursor += strlen(mat_material_names[polygon->material_index]) + 1;			
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
	
	for(i = 0; i < model_list_cursor; i++)
	{
		if(models[i].flags & MODEL_INVALID)
			continue;
			
		editor_WriteTag(file_buffer, &file_buffer_cursor, MODEL_RECORD);
		
		model_record = (model_record_t *)(file_buffer + file_buffer_cursor);
		model_record->flags = models[i].flags;
		
		file_buffer_cursor += sizeof(model_record_t);
				
		strcpy(file_buffer + file_buffer_cursor, models[i].file_name);
		file_buffer_cursor += strlen(models[i].file_name) + 1;
		
		strcpy(file_buffer + file_buffer_cursor, models[i].name);
		file_buffer_cursor += strlen(models[i].name) + 1;
			
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
		file_buffer_cursor += strlen(model->name) + 1;	
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

char *editor_GetFullPathToProjectFileFolder()
{
	return ed_full_path_to_project_file_folder;
}






