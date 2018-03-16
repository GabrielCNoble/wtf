#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "GL\glew.h"

#include "model.h"
#include "gpu.h"
#include "path.h"
#include "mpk_file.h"
#include "material.h"
#include "texture.h"
#include "shader.h"
//#include "physics.h"


/*static int mesh_data_list_size;
static int mesh_data_count;

int mesh_free_stack_top;
int *mesh_free_stack;
static mesh_t *mesh_data;
static int *alloc_handles;*/
  
int mesh_count;
mesh_t *meshes;


int model_free_stack_top;
int *model_free_stack;
int max_models;
int model_list_cursor;
model_t *models;





int model_Init()
{
	/*mesh_data_list_size = 64;
	mesh_data_count = 0;
	mesh_data = malloc(sizeof(mesh_t) * mesh_data_list_size);
	alloc_handles = malloc(sizeof(int) * mesh_data_list_size);
	
	mesh_free_stack_top = -1;
	mesh_free_stack = malloc(sizeof(int) * mesh_data_list_size);*/
	mesh_count = 0;
	meshes = NULL;
	
	
	max_models = 512;
	model_list_cursor = 0;
	model_free_stack_top = -1;
	
	models = malloc(sizeof(model_t) * max_models);
	model_free_stack = malloc(sizeof(int) * max_models);
	
	return 1;
}

void model_Finish()
{
	int i;
	
	while(meshes)
	{
		free(meshes->name);
		free(meshes->vertices);
		meshes = meshes->next;
	}
	
	
	free(model_free_stack);
	
	for(i = 0; i < model_list_cursor; i++)
	{
		if(!models[i].vert_count)
			continue;
			
		free(models[i].name);	
	}
	
	free(models);
}


mesh_t *model_CreateMesh(char *name, vertex_t *vertices, unsigned int vert_count, unsigned short draw_mode)
{
	mesh_t *mesh;
	
	mesh = malloc(sizeof(mesh_t ));
	mesh->name = strdup(name);
	mesh->vertices = vertices;
	mesh->vert_count = vert_count;
	mesh->draw_mode = draw_mode;
	
	mesh->gpu_handle = gpu_Alloc(sizeof(vertex_t) * mesh->vert_count);
	mesh->vert_start = gpu_GetAllocStart(mesh->gpu_handle) / sizeof(vertex_t);
	
	gpu_Write(mesh->gpu_handle, 0, mesh->vertices, sizeof(vertex_t) * mesh->vert_count, 0);
	
	mesh->next = meshes;
	meshes = mesh;
	
	mesh_count++;
	
	//printf("created mesh [%s] with %d vertices...\n", name, vert_count);
	
	return mesh;
}

int model_DestroyMesh(char *name)
{
	mesh_t *mesh;
	mesh_t *prev = NULL;
	
	mesh = meshes;
	
	while(mesh)
	{
		/* not necessarily a fast function, but
		mesh creation / deletion should be done
		only while loading levels... */
		if(!strcmp(name, mesh->name))
		{
			free(mesh->name);
			free(mesh->vertices);
			gpu_Free(mesh->gpu_handle);
			
			if(prev)
			{
				prev->next = mesh->next;
			}
			else
			{
				meshes = mesh->next;
			}
			
			free(mesh);
			
			mesh_count--;
			return 1;
		}
		
		prev = mesh;
		mesh = mesh->next;
	}
	
	return 0;
}

int model_CreateModel(char *name, mesh_t *mesh, triangle_group_t *triangle_groups, int triangle_group_count)
{
	int model_index;
	model_t *model;
	
	if(!mesh)
	{
		printf("model_CreateModel: null mesh pointer for model [%s]!\n", name);
		return -1;
	}
	
	if(!triangle_groups)
	{
		printf("model_CreateModel: null triangle groups pointer for model [%s]!\n", name);
		return -1;
	}
	
	if(model_free_stack_top > -1)
	{
		model_index = model_free_stack[model_free_stack_top];
		model_free_stack_top--;
	}
	else
	{
		model_index = model_list_cursor++;
		
		if(model_index >= max_models)
		{
			model = malloc(sizeof(model_t) * (max_models + 32));
			free(model_free_stack);
			model_free_stack = malloc(sizeof(int) * (max_models + 32));
			
			memcpy(model, models, sizeof(model_t) * max_models);
			
			free(models);
			models = model;
			max_models += 32;
		}
	}
	
	model = &models[model_index];
	
	
	model->name = strdup(name);
	model->mesh = mesh;
	model->triangle_groups = triangle_groups;
	model->triangle_group_count = triangle_group_count;
	model->draw_mode = mesh->draw_mode;
	model->vert_count = mesh->vert_count;
	model->vert_start = mesh->vert_start;
	model->flags = 0;
	
	
	//printf("created model [%s] with mesh [%s]\n", name, mesh->name);
	
	return model_index;	
	
}

int model_DestroyModel(char *name)
{
	int i;
	
	for(i = 0; i < model_list_cursor; i++)
	{
		if(!strcmp(name, models[i].name))
		{
			return model_DestroyModelIndex(i);
		}
	}
	
	return 0;
}

int model_DestroyModelIndex(int model_index)
{
	if(model_index >= 0 && model_index < model_list_cursor)
	{
		if(!(models[model_index].flags & MODEL_INVALID))
		{
			models[model_index].vert_count = 0;
			models[model_index].vert_start = -1;
			free(models[model_index].triangle_groups);
			models[model_index].triangle_groups = NULL;
			
			models[model_index].triangle_group_count = 0;
			free(models[model_index].name);
			models[model_index].name = NULL;
			
			models[model_index].mesh = NULL;
			
			models[model_index].flags |= MODEL_INVALID;
			
			model_free_stack_top++;
			model_free_stack[model_free_stack_top] = model_index;
			
			return 1;
		}
	}
	
	return 0;
}


int model_LoadModel(char *file_name, char *model_name)
{
	FILE *file;
	char *full_path;
	int mesh_index;
	char name[BSP_MAX_NAME_LEN * 2];
	char name2[BSP_MAX_NAME_LEN * 2];
	
	int i;
	int j;
	
	char *file_buffer = 0;
	unsigned long long file_size = 0;
	unsigned int file_buffer_cursor = 0;
	
	int diffuse_texture_index;
	int normal_texture_index;
	int material_index;
	
	mpk_header_t *header;
	mpk_vertex_record_t *vertex_record;
	material_record_t *material_record;
	texture_record_t *texture_record;
	int triangle_group_count = 0;
	triangle_group_t *triangle_groups = NULL;
	
	mesh_t *mesh;
	int model_index;
	
	vertex_t *vertices;
	int vertice_count;
	
	int *h;
	
	file = fopen(file_name, "rb");
	
	if(!file)
	{
		printf("couldn't open %s!\n", file_name);
		return -1;
	}
	
	
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);
	
	
	file_buffer = calloc(file_size, 1);
	fread(file_buffer, file_size, 1, file);
	fclose(file);
	
	
	header = (mpk_header_t *)file_buffer;
	file_buffer_cursor += sizeof(mpk_header_t);
		
	for(i = 0; i < header->texture_count; i++)
	{
		texture_record = (texture_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(texture_record_t);

		j = 0;
		while(*(file_buffer + file_buffer_cursor))
		{
			name[j] = *(file_buffer + file_buffer_cursor);
			j++;
			file_buffer_cursor++;
		}
		
		name[j] = '\0';
		file_buffer_cursor++;
		
		//strcpy(name, file_buffer + file_buffer_cursor);
		//file_buffer_cursor += strlen(name) + 1;
		
		//full_path = path_FormatPath(name);
		//strcpy(name, full_path);
		
		//strcpy(name2, file_buffer + file_buffer_cursor);
		//file_buffer_cursor += strlen(name2) + 1;
		
		j = 0;
		while(*(file_buffer + file_buffer_cursor))
		{
			name2[j] = *(file_buffer + file_buffer_cursor);
			j++;
			file_buffer_cursor++;
		}
		
		name2[j] = '\0';
		file_buffer_cursor++;
		
		if(texture_LoadTexture(path_FormatPath(name), path_GetFileNameFromPath(name2), texture_record->bm_texture_flags) < 0)
		{
			printf("couldn't find texture %s [%s]!\n", name, name2);
		}
		/*else
		{
			printf("texture %s [%s]...\n", name, name2);
		}	*/
	}
	
	
	
	vertices = malloc(sizeof(vertex_t) * header->vertice_count);
	vertice_count = 0;
	
	triangle_groups = malloc(sizeof(triangle_group_t) * header->vertex_record_count);
	
	
	for(i = 0; i < header->material_count; i++)
	{
		material_record = (material_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(material_record_t) + strlen(material_record->name) + 1;
		
		diffuse_texture_index = -1;
		normal_texture_index = -1;
		
		if(material_record->bm_flags & MATERIAL_USE_DIFFUSE_TEXTURE)
		{
			strcpy(name, file_buffer + file_buffer_cursor);
			file_buffer_cursor += strlen(name) + 1;
			diffuse_texture_index = texture_GetTexture(path_GetFileNameFromPath(name));
		}
		
		if(material_record->bm_flags & MATERIAL_USE_NORMAL_TEXTURE)
		{
			strcpy(name, file_buffer + file_buffer_cursor);
			file_buffer_cursor += strlen(name) + 1;
			normal_texture_index = texture_GetTexture(path_GetFileNameFromPath(name));
		}
		
		material_index = material_CreateMaterial(material_record->name, material_record->base, 1.0, 1.0, shader_GetShaderIndex("forward_pass"), diffuse_texture_index, normal_texture_index);
		
	//	printf("material %s\n", material_record->name);
		
		vertex_record = (mpk_vertex_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(mpk_vertex_record_t );
		
		if(vertex_record->vertice_count)
		{
			memcpy(vertices + vertice_count, file_buffer + file_buffer_cursor, sizeof(vertex_t ) * vertex_record->vertice_count);
			vertice_count += vertex_record->vertice_count;
			
			triangle_groups[triangle_group_count].material_index = material_index;
			triangle_groups[triangle_group_count].next = vertex_record->vertice_count;
			
			file_buffer_cursor += sizeof(vertex_t) * vertex_record->vertice_count;
			
			
			if(triangle_group_count)
			{
				triangle_groups[triangle_group_count].start = triangle_groups[triangle_group_count - 1].start + triangle_groups[triangle_group_count - 1].next;
			}
			else
			{
				triangle_groups[triangle_group_count].start = 0;
			}
			triangle_group_count++;
			
		//	printf("%d vertices...\n", vertex_record->vertice_count);
		}
		
	}
	
	
	/* indigent vertices... */
	if(header->material_count < header->vertex_record_count)
	{
		vertex_record = (mpk_vertex_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(mpk_vertex_record_t );
		
		memcpy(vertices + vertice_count, file_buffer + file_buffer_cursor, sizeof(vertex_t ) * vertex_record->vertice_count);
		file_buffer_cursor += sizeof(vertex_t ) * vertex_record->vertice_count;
		
		 
		vertice_count += vertex_record->vertice_count;
		
		triangle_groups[triangle_group_count].material_index = -1;
		triangle_groups[triangle_group_count].next = vertex_record->vertice_count;
		
		
		if(triangle_group_count)
		{
			triangle_groups[triangle_group_count].start = triangle_groups[triangle_group_count - 1].start + triangle_groups[triangle_group_count - 1].next;
		}
		else
		{
			triangle_groups[triangle_group_count].start = 0;
		}
		triangle_group_count++;
		
	//	printf("%d indigent vertices...\n", vertex_record);
	}
	
	
	strcpy(name, model_name);
	strcat(name, ".mesh");
		
	mesh = model_CreateMesh(name, vertices, vertice_count, GL_TRIANGLES);
	model_index = model_CreateModel(model_name, mesh, triangle_groups, triangle_group_count);
	
	free(file_buffer);
	
	return model_index;
}


int model_GetModel(char *model_name)
{
	int i;
	
	for(i = 0; i < model_list_cursor; i++)
	{
		if(!strcmp(models[i].name, model_name))
		{
			return i;
		}
	}
	
	return -1;
}

model_t *model_GetModelPointer(char *model_name)
{
	
}

model_t *model_GetModelPointerIndex(int model_index)
{
	if(model_index >= 0 && model_index < model_list_cursor)
	{
		if(!(models[model_index].flags & MODEL_INVALID))
		{
			return &models[model_index];
		}
	}
	
	return NULL;
}
 
int model_IncModelMaterialsRefs(int model_index)
{
	int i;
	model_t *model;
	
	if(model_index >= 0 && model_index < model_list_cursor)
	{
		if(!(models[model_index].flags & MODEL_INVALID))
		{
			model = &models[model_index];
			
			for(i = 0; i < model->triangle_group_count; i++)
			{
				material_IncRefCount(model->triangle_groups[i].material_index);
			}
		}
	}
}

int model_DecModelMaterialsRefs(int model_index)
{
	int i;
	model_t *model;
	
	if(model_index >= 0 && model_index < model_list_cursor)
	{
		if(!(models[model_index].flags & MODEL_INVALID))
		{
			model = &models[model_index];
			
			for(i = 0; i < model->triangle_group_count; i++)
			{
				material_DecRefCount(model->triangle_groups[i].material_index);
			}
		}
	}
}

/*model_t *model_GetModel(char *model_name)
{
	int i;
	
	for(i = 0; i < model_list_cursor; i++)
	{
		if(!strcmp(models[i].name, model_name))
		{
			return &models[i];
		}
	}
	
	return NULL;
}

model_t *model_GetModelIndex(int model_index)
{
	if(model_index >= 0 && model_index < model_list_cursor)
	{
		if(models[model_index].vert_count)
		{
			return &models[model_index];
		}
	}
	
	return NULL;
}*/


void model_GenerateIcoSphere(float radius, int sub_divs, float **verts, int *face_count)
{
	int i;
	int j;
	int k;
	mesh_t m;
	float *a = (float *)malloc(sizeof(float) * 3 * 3 * 20);
	float *b;
	float v_offset = cos((60.0 * 3.14159265) / 180.0) * radius;
	float c;
	int f_count = 20;
	int angle_increments = 4;
	float len;
	int src;
	int dst;
	
	/* generate initial icosahedron... */
	
	/* top cap... */
	for(i = 0; i < 5 * 3;)
	{
		a[i * 3] = 0.0;
		a[i * 3 + 1] = radius;
		a[i * 3 + 2] = 0.0;
		
		i++;
		
		a[i * 3] = sin(3.14159265 * (2.0 / 5.0) * (angle_increments)) * radius;
		a[i * 3 + 1] = v_offset;
		a[i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * (angle_increments)) * radius;
		
		i++;
		angle_increments--;
		
		a[i * 3] = sin(3.14159265 * (2.0 / 5.0) * angle_increments) * radius;
		a[i * 3 + 1] = v_offset;
		a[i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * angle_increments) * radius;
			
		i++;
		//angle_increments++;
	}
	
	
	j = i;
	angle_increments = 0;
	/* center strip... */
	for(i = 0; i < 10 * 3;)
	{
		
		angle_increments++;
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * ((angle_increments - 1) * 0.5 + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = -v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * ((angle_increments - 1) * 0.5 + 0.5)) * radius;
		
		i++;
		
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * (angle_increments * 0.5 + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * (angle_increments * 0.5 + 0.5)) * radius;
		
		i++;
		
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * ((angle_increments + 1) * 0.5 + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = -v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * ((angle_increments + 1) * 0.5 + 0.5)) * radius;
		
		i++;
		
		angle_increments++;
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * ((angle_increments + 1) * 0.5 + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * ((angle_increments + 1) * 0.5 + 0.5)) * radius;
		
		i++;

		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * (angle_increments * 0.5 + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = -v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * (angle_increments * 0.5 + 0.5)) * radius;
		
		i++;
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * ((angle_increments - 1) * 0.5 + 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * ((angle_increments - 1) * 0.5 + 0.5)) * radius;
		
		i++;
			
	}
	
	j += i;
	angle_increments = 0;
	/* bottom cap... */
	for(i = 0; i < 5 * 3;)
	{
		a[j * 3 + i * 3] = 0.0;
		a[j * 3 + i * 3 + 1] = -radius;
		a[j * 3 + i * 3 + 2] = 0.0;
		
		i++;
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * ((float)angle_increments - 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = -v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * ((float)angle_increments - 0.5)) * radius;
		
		i++;
		angle_increments++;
		
		a[j * 3 + i * 3] = sin(3.14159265 * (2.0 / 5.0) * ((float)angle_increments - 0.5)) * radius;
		a[j * 3 + i * 3 + 1] = -v_offset;
		a[j * 3 + i * 3 + 2] = cos(3.14159265 * (2.0 / 5.0) * ((float)angle_increments - 0.5)) * radius;
			
		i++;
	}
	
	
	if(sub_divs > 0)
	{
		for(k = 0; k < sub_divs; k++)
		{

				
			
				dst = 0;
				//src = 0;
				
				b = (float *)malloc(sizeof(float) * 3 * 3 * f_count * 4);
				
				/* for each face, add a vertex in the middle
				of each edge, and then triangulate. Each triangle 
				gets subdivided into four equilateral triangles... */
				for(i = 0; i < f_count * 3;)
				{
					
					/* first triangle... */
					/* v0 */
					b[dst * 3] = a[i * 3];
					b[dst * 3 + 1] = a[i * 3 + 1];
					b[dst * 3 + 2] = a[i * 3 + 2];		
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					
					/* adjust the vertex so it's distance to the center
					of the sphere equals the radius... */
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					/* v1 */
					b[dst * 3] = (a[i * 3] + a[(i + 1) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[i * 3 + 1] + a[(i + 1) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[i * 3 + 2] + a[(i + 1) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					/* v2 */
					b[dst * 3] = (a[i * 3] + a[(i + 2) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[i * 3 + 1] + a[(i + 2) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[i * 3 + 2] + a[(i + 2) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					
					/* second triangle */
					/* v3 */
					b[dst * 3] = (a[i * 3] + a[(i + 1) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[i * 3 + 1] + a[(i + 1) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[i * 3 + 2] + a[(i + 1) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					/* v4 */
					b[dst * 3] = a[(i + 1)  * 3];
					b[dst * 3 + 1] = a[(i + 1) * 3 + 1];
					b[dst * 3 + 2] = a[(i + 1) * 3 + 2];
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					/* v5 */
					b[dst * 3] = (a[(i + 1) * 3] + a[(i + 2) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[(i + 1) * 3 + 1] + a[(i + 2) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[(i + 1) * 3 + 2] + a[(i + 2) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					
					/* third triangle... */
					/* v6 */
					b[dst * 3] = (a[(i + 1) * 3] + a[(i + 2) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[(i + 1) * 3 + 1] + a[(i + 2) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[(i + 1) * 3 + 2] + a[(i + 2) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					/* v7 */
					b[dst * 3] = a[(i + 2) * 3];
					b[dst * 3 + 1] = a[(i + 2) * 3 + 1];
					b[dst * 3 + 2] = a[(i + 2) * 3 + 2];
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					/* v8 */
					b[dst * 3] = (a[i * 3] + a[(i + 2) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[i * 3 + 1] + a[(i + 2) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[i * 3 + 2] + a[(i + 2) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					
					
					/* fourth triangle... */
					/* v9 */
					b[dst * 3] = (a[(i + 1) * 3] + a[(i + 2) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[(i + 1) * 3 + 1] + a[(i + 2) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[(i + 1) * 3 + 2] + a[(i + 2) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					/* v10 */
					b[dst * 3] = (a[i * 3] + a[(i + 2) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[i * 3 + 1] + a[(i + 2) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[i * 3 + 2] + a[(i + 2) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					/* v11 */
					b[dst * 3] = (a[i * 3] + a[(i + 1) * 3]) / 2.0;
					b[dst * 3 + 1] = (a[i * 3 + 1] + a[(i + 1) * 3 + 1]) / 2.0;
					b[dst * 3 + 2] = (a[i * 3 + 2] + a[(i + 1) * 3 + 2]) / 2.0;
					len = sqrt(b[dst * 3] * b[dst * 3] + b[dst * 3 + 1] * b[dst * 3 + 1] + b[dst * 3 + 2] * b[dst * 3 + 2]);
					b[dst * 3] = (b[dst * 3] / len) * radius;
					b[dst * 3 + 1] = (b[dst * 3 + 1] / len) * radius;
					b[dst * 3 + 2] = (b[dst * 3 + 2] / len) * radius;
					
					dst++;
					
					i+=3;
				}
				
				f_count *= 4;

				free(a);
				a = b;		

		}
	}
	
	*verts = a;
	*face_count = f_count;
	
	return;
}


void model_CalculateTangents(vertex_t *vertices, int vertice_count)
{
	int i;
	int count = vertice_count;
	//float *tangent_data = NULL;
	
	vec3_t a;
	vec3_t b;
	vec3_t c;
	vec3_t ab;
	vec3_t ac;
	vec3_t t;
	vec3_t bt;
	vec3_t t1;
	vec3_t bt1;
	
	vec2_t duv1;
	vec2_t duv2;
	
	
	
	float x;
	float y;
	float z;
	float w;
	
	float q;
	
	if(vertice_count < 3)
	{
		return;
	}
	
	//*tangent_data = (float *)calloc(vert_count, sizeof(float) * 3);
	//*bitangent_data = (float *)calloc(vert_count, sizeof(float) * 3);
	
	for(i = 0; i < count;)
	{
		duv1.x = vertices[i + 1].tex_coord.x - vertices[i].tex_coord.x;
		duv1.y = vertices[i + 1].tex_coord.y - vertices[i].tex_coord.y;
		
		duv2.x = vertices[i + 2].tex_coord.x - vertices[i].tex_coord.x;
		duv2.y = vertices[i + 2].tex_coord.y - vertices[i].tex_coord.y;
		
		q = 1.0 / (duv1.x * duv2.y - duv1.y * duv2.x);
		
		a.x = vertices[i].position.x;
		a.y = vertices[i].position.y;
		a.z = vertices[i].position.z;
		
		b.x = vertices[i + 1].position.x;
		b.y = vertices[i + 1].position.y;
		b.z = vertices[i + 1].position.z;
		
		c.x = vertices[i + 2].position.x;
		c.y = vertices[i + 2].position.y;
		c.z = vertices[i + 2].position.z;
		
		ab = sub3(b, a);
		ac = sub3(c, a);
		
		t1.floats[0] = (ab.floats[0] * duv2.floats[1] - ac.floats[0] * duv1.floats[1])*q;
		t1.floats[1] = (ab.floats[1] * duv2.floats[1] - ac.floats[1] * duv1.floats[1])*q;
		
		t = gs_orthg(vertices[i].normal, t1);
		vertices[i].tangent.x = t.x;
		vertices[i].tangent.y = t.y;
		vertices[i].tangent.z = t.z;

		i++;
		
		vertices[i].tangent.x = t.x;
		vertices[i].tangent.y = t.y;
		vertices[i].tangent.z = t.z;
		
		i++;
		
		//t = gs_orthg(vertices[i].normal, t1);
		vertices[i].tangent.x = t.x;
		vertices[i].tangent.y = t.y;
		vertices[i].tangent.z = t.z;
		
		i++;

		
	}
	return;
}


void model_CalculateTangentsIndexes(vertex_t *vertices, int *indexes, int index_count)
{
	int i;
	int count = index_count;
	//float *tangent_data = NULL;
	
	vec3_t a;
	vec3_t b;
	vec3_t c;
	vec3_t ab;
	vec3_t ac;
	vec3_t t;
	vec3_t bt;
	vec3_t t1;
	vec3_t bt1;
	
	vec2_t duv1;
	vec2_t duv2;
	
	
	
	float x;
	float y;
	float z;
	float w;
	
	float q;
	
	if(index_count < 3)
	{
		return;
	}
	
	//*tangent_data = (float *)calloc(vert_count, sizeof(float) * 3);
	//*bitangent_data = (float *)calloc(vert_count, sizeof(float) * 3);
	
	for(i = 0; i < count;)
	{
		
		//duv1.floats[0] = uv_data[(i+1) * 2] - uv_data[i * 2];
		//duv1.floats[1] = uv_data[(i+1) * 2 + 1] - uv_data[i * 2 + 1];
		
		//duv2.floats[0] = uv_data[(i+2) * 2] - uv_data[i * 2];
		//duv2.floats[1] = uv_data[(i+2) * 2 + 1] - uv_data[i * 2 + 1];
		
		
		duv1.x = vertices[indexes[i + 1]].tex_coord.x - vertices[indexes[i]].tex_coord.x;
		duv1.y = vertices[indexes[i + 1]].tex_coord.y - vertices[indexes[i]].tex_coord.y;
		
		duv2.x = vertices[indexes[i + 2]].tex_coord.x - vertices[indexes[i]].tex_coord.x;
		duv2.y = vertices[indexes[i + 2]].tex_coord.y - vertices[indexes[i]].tex_coord.y;
		
		q = 1.0 / (duv1.x * duv2.y - duv1.y * duv2.x);
		
		/*x = duv2.floats[1] / q;
		y = -duv1.floats[1] / q;
		z = -duv2.floats[0] / q;
		w = duv1.floats[0] / q;*/
		
		
		a.x = vertices[indexes[i]].position.x;
		a.y = vertices[indexes[i]].position.y;
		a.z = vertices[indexes[i]].position.z;
		
		b.x = vertices[indexes[i + 1]].position.x;
		b.y = vertices[indexes[i + 1]].position.y;
		b.z = vertices[indexes[i + 1]].position.z;
		
		c.x = vertices[indexes[i + 2]].position.x;
		c.y = vertices[indexes[i + 2]].position.y;
		c.z = vertices[indexes[i + 2]].position.z;
		
		//a.floats[0] = vertex_data[i*3];
		//a.floats[1] = vertex_data[i*3+1];
		//a.floats[2] = vertex_data[i*3+2];
	
		
		//b.floats[0] = vertex_data[(i+1)*3]; 
		//b.floats[1] = vertex_data[(i+1)*3+1];
		//b.floats[2] = vertex_data[(i+1)*3+2];
		
		
		//c.floats[0] = vertex_data[(i+2)*3]; 
		//c.floats[1] = vertex_data[(i+2)*3+1];
		//c.floats[2] = vertex_data[(i+2)*3+2];
		
		ab = sub3(b, a);
		ac = sub3(c, a);
		
		t1.floats[0] = (ab.floats[0] * duv2.floats[1] - ac.floats[0] * duv1.floats[1])*q;
		t1.floats[1] = (ab.floats[1] * duv2.floats[1] - ac.floats[1] * duv1.floats[1])*q;
		t1.floats[2] = (ab.floats[2] * duv2.floats[1] - ac.floats[2] * duv1.floats[1])*q;
		
		
		/*bt1.floats[0] = (ac.floats[0] * duv1.floats[0] - ab.floats[0] * duv2.floats[0])*q;
		bt1.floats[1] = (ac.floats[1] * duv1.floats[0] - ab.floats[1] * duv2.floats[0])*q;
		bt1.floats[2] = (ac.floats[2] * duv1.floats[0] - ab.floats[2] * duv2.floats[0])*q;*/
		
		//t = gs_orthg(vec3(normal_data[i*3], normal_data[i*3+1], normal_data[i*3+2]), t1);
		t = gs_orthg(vertices[indexes[i]].normal, t1);
		vertices[indexes[i]].tangent.x = t.x;
		vertices[indexes[i]].tangent.y = t.y;
		vertices[indexes[i]].tangent.z = t.z;
		
		
		//(*tangent_data)[i*3] = t.floats[0];
		//(*tangent_data)[i*3+1] = t.floats[1];
		//(*tangent_data)[i*3+2] = t.floats[2];
		
		/*(*bitangent_data)[i*3] = 1.0;
		(*bitangent_data)[i*3+1] = 0.0;
		(*bitangent_data)[i*3+2] = 0.0;*/
		
		i++;
		
		//t = gs_orthg(vec3(normal_data[i*3], normal_data[i*3+1], normal_data[i*3+2]), t1);
		t = gs_orthg(vertices[indexes[i]].normal, t1);
		vertices[indexes[i]].tangent.x = t.x;
		vertices[indexes[i]].tangent.y = t.y;
		vertices[indexes[i]].tangent.z = t.z;
		//(*tangent_data)[i*3] = t.floats[0];
		//(*tangent_data)[i*3+1] = t.floats[1];
		//(*tangent_data)[i*3+2] = t.floats[2];
		
		/*(*bitangent_data)[i*3] = 1.0;
		(*bitangent_data)[i*3+1] = 0.0;
		(*bitangent_data)[i*3+2] = 0.0;*/
		
		i++;
		
		t = gs_orthg(vertices[indexes[i]].normal, t1);
		vertices[indexes[i]].tangent.x = t.x;
		vertices[indexes[i]].tangent.y = t.y;
		vertices[indexes[i]].tangent.z = t.z;
		
		/*t = gs_orthg(vec3(normal_data[i*3], normal_data[i*3+1], normal_data[i*3+2]), t1);
		(*tangent_data)[i*3] = t.floats[0];
		(*tangent_data)[i*3+1] = t.floats[1];
		(*tangent_data)[i*3+2] = t.floats[2];*/
		
		/*(*bitangent_data)[i*3] = 1.0;
		(*bitangent_data)[i*3+1] = 0.0;
		(*bitangent_data)[i*3+2] = 0.0;*/
		
		i++;

		
	}
	return;
}













