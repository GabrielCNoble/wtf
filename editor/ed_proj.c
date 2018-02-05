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
#include "r_main.h"
#include "bsp.h"
#include "bsp_cmp.h"
#include "player.h"

#include "SDL2\SDL.h"
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


extern SDL_Window *window;

char current_project_name[512];

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
every time a change happens... */
int b_project_dirty = 0;

/* from bsp_cmp.c */
extern int b_compiling;

/* from pvs.c */
extern int b_calculating_pvs;


/* from player.c */
extern int spawn_point_count;
extern spawn_point_t *spawn_points;


void editor_SaveProject()
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
	polygon_record_t polygon_record;
	spawn_point_record_t spawn_point_record;
	triangle_group_t *triangle_group;
	brush_t *brush;
	bsp_polygon_t *polygon;
	material_t *material;
	char name[64];
	
	file = fopen(current_project_name, "wb");
	
	header.version = PROJ_VERSION;
	header.brush_count = 0;
	header.light_count = 0;
	header.camera_count = camera_count;
	header.material_count = material_count;
	header.spawn_point_count = 0;
	
	
	for(i = 0; i < brush_count; i++)
	{
		if(brushes[i].type == BRUSH_INVALID)
			continue;
		
		if(brushes[i].type == BRUSH_BOUNDS)
			continue;	
			
		header.brush_count++;
	}
	
	for(i = 0; i < light_count; i++)
	{
		if(light_params[i].bm_flags & LIGHT_INVALID)
			continue;
		
		header.light_count++;	
	}
	
	for(i = 0; i < spawn_point_count; i++)
	{
		if(spawn_points[i].bm_flags & SPAWN_POINT_INVALID)
			continue;
			
		header.spawn_point_count++;	
	}
	
	
	fwrite(&header, sizeof(proj_header_t), 1, file);
	
	
	for(i = 0; i < material_count; i++)
	{
		for(j = 0; j < MAX_NAME_LEN; j++)
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
		
		if(brush->type == BRUSH_INVALID)
			continue;
			
		if(brush->type == BRUSH_BOUNDS)
			continue;	
		
		brush_lump.orientation = brush->orientation;
		brush_lump.position = brush->position;
		brush_lump.scale = brush->scale;
		brush_lump.type = brush->type;
		brush_lump.triangle_group_count = brush->triangle_group_count;
		brush_lump.vertex_count = brush->vertex_count;
		brush_lump.polygon_count = brush->polygon_count;
		
		fwrite(&brush_lump, sizeof(brush_lump_t), 1, file);
		fwrite(brush->vertices, sizeof(vertex_t), brush->vertex_count, file);
		
		for(j = 0; j < brush->polygon_count; j++)
		{
			polygon = brush->polygons + j;
			
			polygon_record.vert_count = polygon->vert_count;
			polygon_record.normal = polygon->normal;
			polygon_record.first_index_offset = polygon->vertices - brush->vertices;
			
			for(k = 0; k < MAX_NAME_LEN; k++)
			{
				polygon_record.material_name[k] = '\0';
			}
				
			strcpy(polygon_record.material_name, material_names[polygon->material_index]);
			fwrite(&polygon_record, sizeof(polygon_record_t), 1, file);			
		}
	}
	
	for(i = 0; i < light_count; i++)
	{
		if(light_params[i].bm_flags & LIGHT_INVALID)
			continue;
			
		light_lump.orientation = light_positions[i].orientation;
		light_lump.position = light_positions[i].position;
		light_lump.color.r = (float)light_params[i].r / 255.0;
		light_lump.color.g = (float)light_params[i].g / 255.0;
		light_lump.color.b = (float)light_params[i].b / 255.0;
		light_lump.energy = LIGHT_ENERGY(light_params[i].energy);
		light_lump.radius = LIGHT_RADIUS(light_params[i].radius);
		light_lump.type = 0;
		light_lump.bm_flags = light_params[i].bm_flags;
		
		for(j = 0; j < MAX_NAME_LEN; j++)
		{
			light_lump.name[j] = '\0';
		}
		
		strcpy(light_lump.name, light_names[i]);
		fwrite(&light_lump, sizeof(light_lump_t), 1, file);
	}
	
	for(i = 0; i < spawn_point_count; i++)
	{
		if(spawn_points[i].bm_flags & SPAWN_POINT_INVALID)
			continue;
		
		for(j = 0; j < MAX_NAME_LEN; j++)
		{
			spawn_point_record.name[j] = '\0';
		}
			
		spawn_point_record.position = spawn_points[i].position;
		strcpy(spawn_point_record.name, spawn_points[i].name);
		
		fwrite(&spawn_point_record, sizeof(spawn_point_record_t), 1, file);
	}
	
	
	/*for(i = 0; i < camera_count; i++)
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
	}*/

	fclose(file);
	
	b_project_dirty = 1;
	
}

int editor_OpenProject(char *file_name)
{
	//printf("editor_OpenProject\n");
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
	polygon_record_t polygon_record;
	spawn_point_record_t spawn_point_record;
	brush_t *brush;
	brush_t *brush2;
	vertex_t *vertices;
	material_t *material;
	short shader_index;
	short material_index;
	int count;
	char name[64];
	
	editor_ClearSelection();
	
	editor_CloseProject();
	

	
	if(!(file = fopen(file_name, "rb")))
	{
		printf("couldn't open project [%s]!\n", file_name);
		return 0;
	}
	
	//printf("editor_OpenProject: open file\n");
	
	fread(&header, sizeof(proj_header_t), 1, file);
	//printf("editor_OpenProject: read header\n");
	
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
	
	//printf("editor_OpenProject: read materials\n");
	
	
	for(i = 0; i < header.brush_count; i++)
	{
		j = brush_CreateEmptyBrush();
		brush = &brushes[j];
		
		fread(&brush_lump, sizeof(brush_lump_t), 1, file);
		
		brush->vertices = malloc(sizeof(vertex_t) * brush_lump.vertex_count * 10);
		brush->vertex_count = brush_lump.vertex_count;
		brush->max_vertexes = brush_lump.vertex_count;
		
		brush->polygon_count = brush_lump.polygon_count;
		brush->polygons = malloc(sizeof(bsp_polygon_t) * brush_lump.polygon_count * 10);
		
		brush->orientation = brush_lump.orientation;
		brush->position = brush_lump.position;
		brush->scale = brush_lump.scale;
		brush->type = brush_lump.type;
		
		
		//vertices = malloc(sizeof(vertex_t) * brush_lump.vertex_count);
		
		fread(brush->vertices, sizeof(vertex_t), brush_lump.vertex_count, file);
		//fread(vertices, sizeof(vertex_t), brush_lump.vertex_count, file);
		//free(vertices);
		
		//brush_lump.orientation = mat3_t_id();
		
		//j = brush_CreateBrush(brush_lump.position, &brush_lump.orientation, brush_lump.scale, brush_lump.type);
		//brush2 = &brushes[j];
		
		
		for(k = 0; k < brush_lump.polygon_count; k++)
		{
			fread(&polygon_record, sizeof(polygon_record_t), 1, file);
			
			brush->polygons[k].next = &brush->polygons[k + 1];
			brush->polygons[k].b_used = 0;
			brush->polygons[k].normal = polygon_record.normal;
			brush->polygons[k].vert_count = polygon_record.vert_count;
			brush->polygons[k].vertices = brush->vertices + polygon_record.first_index_offset;
			brush->polygons[k].material_index = material_GetMaterialIndex(polygon_record.material_name);	
			
			
			//printf("[%f %f %f]     [%f %f %f]\n", brush->polygons[k].normal.x, brush->polygons[k].normal.y, brush->polygons[k].normal.z, 
			  //                                    brush2->polygons[k].normal.x, brush2->polygons[k].normal.y, brush2->polygons[k].normal.z);
			
		}
		
		brush->polygons[k - 1].next = NULL;
		
		bsp_TriangulatePolygonsIndexes(brush->polygons, &brush->indexes, &brush->index_count);
			
		brush->handle = gpu_Alloc(sizeof(vertex_t) * brush->vertex_count * 10);
		brush->start = gpu_GetAllocStart(brush->handle) / sizeof(vertex_t);
		gpu_Write(brush->handle, 0, brush->vertices, sizeof(vertex_t) * brush->vertex_count, 0);
	
		glGenBuffers(1, &brush->element_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brush->element_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * brush->index_count, NULL, GL_DYNAMIC_DRAW);
		
		brush_BuildTriangleGroups(brush);
		brush_UpdateBrushElementBuffer(brush);
			
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	
	//printf("editor_OpenProject: read brushes\n");
	
	for(i = 0; i < header.light_count; i++)
	{
		fread(&light_lump, sizeof(light_lump_t), 1, file);
		light_CreateLight(light_lump.name, &light_lump.orientation, light_lump.position, light_lump.color, light_lump.radius, light_lump.energy, light_lump.bm_flags);
	}
	
	//printf("editor_OpenProject: read lights\n");
	
	for(i = 0; i < header.spawn_point_count; i++)
	{
		fread(&spawn_point_record, sizeof(spawn_point_record_t), 1, file);
		player_CreateSpawnPoint(spawn_point_record.position, spawn_point_record.name);
	}
	
	//printf("editor_OpenProject: read spawn points\n");
	
	return 1;
	
	/*for(i = 0; i < header.camera_count; i++)
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
	}*/
}

void editor_SetProjectName(char *name)
{
	
	int i = 0;
	char *s = name;
	int b_found_point;
	
	while(s[i])
	{
		
		if(s[i] == '.')
		{
			break;
		}
		current_project_name[i] = name[i];
		
		i++;
	}
	
	current_project_name[i] = '.';
	i++;
	current_project_name[i] = 'w';
	i++;
	current_project_name[i] = 't';
	i++;
	current_project_name[i] = 'f';
	i++;
	current_project_name[i] = '\0';
	
	SDL_SetWindowTitle(window, current_project_name);

}

void editor_CloseProject()
{
	//printf("edidor_CloseProject\n");
	if(editor_IsProjectDirty())
	{
	
	}
	
	light_DestroyAllLights();
	//printf("light_DestroyAllLights\n");
	brush_DestroyAllBrushes();
	//printf("brush_DestroyAllBrushes\n");
	material_DestroyAllMaterials();
	//printf("material_DestroyAllMaterials\n");
	camera_DestroyAllCameras();
	//printf("camera_DestroyAllCameras\n");
	bsp_DeleteBsp();
	//printf("bsp_DeleteBsp\n");
	
	editor_RestartEditor();
	
	//printf("editor_RestartEditor\n");
	
	//gpu_ClearHeap();
		
}

void editor_ExportBsp(char *file_name)
{
	FILE *file;
	bsp_header_t header;
	light_lump_t light_lump;
	triangle_group_lump_t triangle_group_lump;
	material_lump_t material_lump;
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
			for(j = 0; j < MAX_NAME_LEN; j++)
			{
				attrib_name[j] = 0;
			}
				
			strcpy(attrib_name, texture_names[materials[i].diffuse_texture].file_name);
			
			fwrite(attrib_name, MAX_NAME_LEN, 1, file);
		}
		
		if(material_lump.bm_textures & 2)
		{
			for(j = 0; j < MAX_NAME_LEN; j++)
			{
				attrib_name[j] = 0;
			}
				
			strcpy(attrib_name, texture_names[materials[i].normal_texture].file_name);
			
			fwrite(attrib_name, MAX_NAME_LEN, 1, file);
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
		for(j = 0; j < MAX_NAME_LEN; j++)
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







