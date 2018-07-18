#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include "GL\glew.h"

#include "model.h"
#include "gpu.h"
#include "path.h"
#include "mpk_file.h"
#include "material.h"
#include "texture.h"
#include "shader.h"
#include "memory.h"
#include "r_debug.h"

#include "mpk_read.h"

#include "stack_list.h"

//#include "physics.h"


/*static int mesh_data_list_size;
static int mesh_data_count;

int mesh_free_stack_top;
int *mesh_free_stack;
static mesh_t *mesh_data;
static int *alloc_handles;*/
  
int mdl_mesh_count;
mesh_t *mdl_meshes;
//mesh_t *mdl_last_mesh;


//int mdl_model_free_stack_top;
//int *mdl_model_free_stack;
//int mdl_max_models;
//int mdl_model_list_cursor;
//struct model_t *mdl_models = NULL;
//struct model_t *mdl_last_model = NULL;


struct stack_list_t mdl_models;




#ifdef __cplusplus
extern "C"
{
#endif

void model_DisposeModelCallback(void *element)
{
	struct model_t *model;
	
	model = (struct model_t *)element;
	
	if(model->name)
	{
		memory_Free(model->name);
	}
	
	if(model->batches)
	{
		memory_Free(model->batches);
	}
	
	if(model->file_name)
	{
		memory_Free(model->file_name);
	}
}


int model_Init()
{
	/*mesh_data_list_size = 64;
	mesh_data_count = 0;
	mesh_data = malloc(sizeof(mesh_t) * mesh_data_list_size);
	alloc_handles = malloc(sizeof(int) * mesh_data_list_size);
	
	mesh_free_stack_top = -1;
	mesh_free_stack = malloc(sizeof(int) * mesh_data_list_size);*/
	mdl_mesh_count = 0;
	mdl_meshes = NULL;
	//mdl_last_mesh = NULL;
	
	//mdl_max_models = 512;
	//mdl_model_list_cursor = 0;
	//mdl_model_free_stack_top = -1;
	
	
	//mdl_models = memory_Malloc(sizeof(struct model_t) * mdl_max_models, "model_Init");
	//mdl_model_free_stack = memory_Malloc(sizeof(int) * mdl_max_models, "model_Init");
	
	mdl_models = stack_list_create(sizeof(struct model_t), 32, model_DisposeModelCallback);
	
	return 1;
}

void model_Finish()
{
	int i;
	mesh_t *next;
	while(mdl_meshes)
	{
		//free(meshes->name);
		//free(meshes->vertices);
		next = mdl_meshes->next;
		memory_Free(mdl_meshes->name);
		memory_Free(mdl_meshes->vertices);
		mdl_meshes = next;
	}

	//memory_Free(mdl_model_free_stack);
	
	stack_list_destroy(&mdl_models);
	
	//for(i = 0; i < mdl_model_list_cursor; i++)
	//{
	//	if(!mdl_models[i].vert_count)
	//		continue;
			
	//	memory_Free(mdl_models[i].name);
	//	memory_Free(mdl_models[i].file_name);
	//	memory_Free(mdl_models[i].batches);
	//}
	//memory_Free(mdl_models);
	
	
	
}


mesh_t *model_CreateMesh(char *name, vertex_t *vertices, unsigned int vert_count, unsigned short draw_mode)
{
	int i;
	
	mesh_t *mesh;
	
	vec3_t max_extents;
	vec3_t min_extents;
	vec3_t center;
	compact_vertex_t *compact;
	
	renderer_PushFunctionName("model_CreateMesh");
	
	//mesh = malloc(sizeof(mesh_t ));
	mesh = memory_Malloc(sizeof(mesh_t), "model_CreateMesh");
	//mesh->name = strdup(name);
	mesh->name = memory_Strdup(name, "model_CreateMesh");
	mesh->vertices = vertices;
	mesh->vert_count = vert_count;
	mesh->draw_mode = draw_mode;
	
	
	max_extents.x = FLT_MIN;
	max_extents.y = FLT_MIN;
	max_extents.z = FLT_MIN;
	
	min_extents.x = FLT_MAX;
	min_extents.y = FLT_MAX;
	min_extents.z = FLT_MAX;
	
	for(i = 0; i < mesh->vert_count; i++)
	{
		if(mesh->vertices[i].position.x > max_extents.x) max_extents.x = mesh->vertices[i].position.x;
		if(mesh->vertices[i].position.y > max_extents.y) max_extents.y = mesh->vertices[i].position.y;
		if(mesh->vertices[i].position.z > max_extents.z) max_extents.z = mesh->vertices[i].position.z;
		
		
		if(mesh->vertices[i].position.x < min_extents.x) min_extents.x = mesh->vertices[i].position.x;
		if(mesh->vertices[i].position.y < min_extents.y) min_extents.y = mesh->vertices[i].position.y;
		if(mesh->vertices[i].position.z < min_extents.z) min_extents.z = mesh->vertices[i].position.z;
	}
	
	center.x = (max_extents.x + min_extents.x) / 2.0;
	center.y = (max_extents.y + min_extents.y) / 2.0;
	center.z = (max_extents.z + min_extents.z) / 2.0;
	
	
	for(i = 0; i < mesh->vert_count; i++)
	{
		mesh->vertices[i].position.x -= center.x;
		mesh->vertices[i].position.y -= center.y;
		mesh->vertices[i].position.z -= center.z;
	}
	
	
	compact = model_ConvertVertices(mesh->vertices, mesh->vert_count);
	
	//mesh->gpu_handle = gpu_Alloc(sizeof(vertex_t) * mesh->vert_count);
	//mesh->vert_start = gpu_GetAllocStart(mesh->gpu_handle) / sizeof(vertex_t);
	
	//mesh->gpu_handle = gpu_Alloc(sizeof(compact_vertex_t) * mesh->vert_count);
	mesh->gpu_handle = gpu_AllocAlign(sizeof(compact_vertex_t) * mesh->vert_count, sizeof(compact_vertex_t), 0);
	mesh->vert_start = gpu_GetAllocStart(mesh->gpu_handle) / sizeof(compact_vertex_t);
	
	//gpu_Write(mesh->gpu_handle, 0, mesh->vertices, sizeof(vertex_t) * mesh->vert_count, 0);
	gpu_Write(mesh->gpu_handle, 0, compact, sizeof(compact_vertex_t) * mesh->vert_count);
	
	max_extents.x -= center.x;
	max_extents.y -= center.y;
	max_extents.z -= center.z;
		
	mesh->aabb_max = max_extents;
	
	mesh->next = mdl_meshes;
	mdl_meshes = mesh;
	
	mdl_mesh_count++;
	
	//free(compact);
	memory_Free(compact);
	
	renderer_PopFunctionName();
	
	return mesh;
}

int model_DestroyMesh(char *name)
{
	mesh_t *mesh;
	mesh_t *prev = NULL;
	
	mesh = mdl_meshes;
	
	renderer_PushFunctionName("model_DestroyMesh");
	
	while(mesh)
	{
		/* not necessarily a fast function, but
		mesh creation / deletion should be done
		only while loading levels... */
		if(!strcmp(name, mesh->name))
		{
			//free(mesh->name);
			//free(mesh->vertices);
			memory_Free(mesh->name);
			memory_Free(mesh->vertices);
			gpu_Free(mesh->gpu_handle);
			
			if(prev)
			{
				prev->next = mesh->next;
			}
			else
			{
				mdl_meshes = mesh->next;
			}
			
			//free(mesh);
			memory_Free(mesh);
			
			mdl_mesh_count--;
			
			renderer_PopFunctionName();
			return 1;
		}
		
		prev = mesh;
		mesh = mesh->next;
	}
	
	renderer_PopFunctionName();
	return 0;
}
 
int model_CreateModel(char *file_name, char *name, mesh_t *mesh, batch_t *batches, int batch_count)
{
	int model_index;
	struct model_t *model;
		
	if(!mesh)
	{
		printf("model_CreateModel: null mesh pointer for model [%s]!\n", name);
		return -1;
	}
	
	if(!batches)
	{
		printf("model_CreateModel: null batch pointer for model [%s]!\n", name);
		return -1;
	}
	
	/*if(mdl_model_free_stack_top > -1)
	{
		model_index = mdl_model_free_stack[mdl_model_free_stack_top];
		mdl_model_free_stack_top--;
	}
	else
	{
		model_index = mdl_model_list_cursor++;
		
		if(model_index >= mdl_max_models)
		{
			model = memory_Malloc(sizeof(struct model_t) * (mdl_max_models + 32), "model_CreateModel");
			memory_Free(mdl_model_free_stack);
			mdl_model_free_stack = memory_Malloc(sizeof(int) * (mdl_max_models + 32), "model_CreateModel");
			
			memcpy(model, mdl_models, sizeof(struct model_t) * mdl_max_models);

			memory_Free(mdl_models);
			mdl_models = model;
			mdl_max_models += 32;
		}
	}*/
	
	model_index = stack_list_add(&mdl_models, NULL);
	model = (struct model_t *)stack_list_get(&mdl_models, model_index);
	
	//model = &mdl_models[model_index];
	
	//model = memory_Malloc(sizeof(struct model_t), "model_CreateModel");
	
	//model->next = NULL;
	//model->prev = NULL;
	
	//model->name = strdup(name);
	model->name = memory_Strdup(name, "model_CreateModel");
	//model->file_name = strdup(file_name);
	model->file_name = memory_Strdup(file_name, "model_CreateModel");
	model->mesh = mesh;
	model->batches = batches;
	model->batch_count = batch_count;
	model->draw_mode = mesh->draw_mode;
	model->vert_count = mesh->vert_count;
	model->vert_start = mesh->vert_start;
	model->aabb_max = mesh->aabb_max;
	model->flags = 0;
	
	
	/*if(!mdl_models)
	{
		mdl_models = model;
	}
	else
	{
		mdl_last_model->next = model;
		model->prev = mdl_last_model;
	}
	
	mdl_last_model = model;*/
	
	//printf("created model [%s] with mesh [%s]\n", name, mesh->name);
	
	return model_index;	
	
	//return model;
}

int model_DestroyModel(char *name)
{
	int i;
	
	/*for(i = 0; i < mdl_model_list_cursor; i++)
	{
		if(!strcmp(name, mdl_models[i].name))
		{
			return model_DestroyModelIndex(i);
		}
	}*/
	
	return 0;
}

int model_DestroyModelIndex(int model_index)
{
	//if(model_index >= 0 && model_index < mdl_model_list_cursor)
	{
		/*if(!(mdl_models[model_index].flags & MODEL_INVALID))
		{
			mdl_models[model_index].vert_count = 0;
			mdl_models[model_index].vert_start = -1;
			memory_Free(mdl_models[model_index].batches);
			mdl_models[model_index].batches = NULL;
			
			mdl_models[model_index].batch_count = 0;
			memory_Free(mdl_models[model_index].name);
			mdl_models[model_index].name = NULL;
			
			mdl_models[model_index].mesh = NULL;
			
			mdl_models[model_index].flags |= MODEL_INVALID;
			
			mdl_model_free_stack_top++;
			mdl_model_free_stack[mdl_model_free_stack_top] = model_index;
			
			return 1;
		}*/
	}
	
	return 0;
}

int model_LoadModel(char *file_name, char *model_name)
{
	mpk_vertex_record_t *vertex_records;
	batch_t *batches;
	int vertex_record_count;
	
	char mesh_name[512];
	
	mesh_t *mesh;
	struct model_t *model;
	int model_index;
	int i;
	
	
	renderer_PushFunctionName("model_LoadModel");
	
	vertex_t *vertices;
	int vertice_count;
	
	if(mpk_read(file_name, &vertex_records, &vertex_record_count, &vertices, &vertice_count))
	{
		batches = memory_Malloc(sizeof(batch_t) * vertex_record_count, "model_LoadModel");
		
		for(i = 0; i < vertex_record_count; i++)
		{
			batches[i].material_index = material_MaterialIndex(vertex_records[i].material_name);
			batches[i].next = vertex_records[i].vertice_count;
			batches[i].start = vertex_records[i].offset;
		}
		
		strcpy(mesh_name, model_name);
		strcat(mesh_name, ".mesh");
		
		mesh = model_CreateMesh(mesh_name, vertices, vertice_count, GL_TRIANGLES);
		model_index = model_CreateModel(file_name, model_name, mesh, batches, vertex_record_count);
		
		memory_Free(vertex_records);
		
		renderer_PopFunctionName();
		
		return model_index;	
	}
	
	printf("model_LoadModel: something went wrong when loading model %s!\n", model_name);
	
	renderer_PopFunctionName();
	
	return -1;
}




int model_GetModel(char *model_name)
{
	int i;
	int c;
	
	struct model_t *models;
	struct model_t *model;
	
	models = (struct model_t *)mdl_models.elements;
	c = mdl_models.element_count;
	
	for(i = 0; i < c; i++)
	{
		model = models + i;
		
		if(model->flags & MODEL_INVALID)
		{
			continue;
		}
		
		if(!strcmp(model->name, model_name))
		{
			return i;
		}
	}

	return -1;
}

struct model_t *model_GetModelPointer(char *model_name)
{
	
}

struct model_t *model_GetModelPointerIndex(int model_index)
{
	struct model_t *model;
	if(model_index >= 0 && model_index < mdl_models.element_count)
	{
		model = (struct model_t *)mdl_models.elements + model_index;
		
		if(!(model->flags & MODEL_INVALID))
		{
			return model;
		}
	}
	
	return NULL;
}

mesh_t *model_GetModelMesh(char *model_name)
{
	/*int i;
	for(i = 0; i < mdl_model_list_cursor; i++)
	{
		if(!strcmp(model_name, mdl_models[i].name))
		{
			if(!(mdl_models[i].flags & MODEL_INVALID))
			{
				return mdl_models[i].mesh;
			}
		}
		
	}
	
	return NULL;*/
}

mesh_t *model_GetModelMeshIndex(int model_index)
{
	/*if(model_index >= 0 && model_index < mdl_model_list_cursor)
	{
		if(!(mdl_models[model_index].flags & MODEL_INVALID))
		{
			return mdl_models[model_index].mesh;
		}
	}*/
}
 
int model_IncModelMaterialsRefs(int model_index)
{
	int i;
	struct model_t *model;
	
	if(model_index >= 0 && model_index < mdl_models.element_count)
	{
		model = (struct model_t *)mdl_models.elements + model_index;
		if(!(model->flags & MODEL_INVALID))
		{
			for(i = 0; i < model->batch_count; i++)
			{
				if(!material_IncRefCount(model->batches[i].material_index))
				{
					/* this material got destroyed, so assing the default 
					material to this model... */
					model->batches[i].material_index = -1;
				}
			}
		}
	}

}

int model_DecModelMaterialsRefs(int model_index)
{
	int i;
	struct model_t *model;
	
	if(model_index >= 0 && model_index < mdl_models.element_count)
	{
		model = (struct model_t *)mdl_models.elements + model_index;
		
		if(!(model->flags & MODEL_INVALID))
		{
			for(i = 0; i < model->batch_count; i++)
			{
				if(!material_DecRefCount(model->batches[i].material_index))
				{
					model->batches[i].material_index = -1;
				}
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
	//float *a = (float *)malloc(sizeof(float) * 3 * 3 * 20);
	float *a = memory_Malloc(sizeof(float) * 3 * 3 * 20, "model_GenerateIcoSphere");
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
				
				//b = (float *)malloc(sizeof(float) * 3 * 3 * f_count * 4);
				b = memory_Malloc(sizeof(float) * 3 * 3 * f_count * 4, "model_GenerateIcoSphere");
				
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

				//free(a);
				memory_Free(a);
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

void model_GenerateIndexes(struct model_t *model)
{
	int vertice_count;
	
}

compact_vertex_t *model_ConvertVertices(vertex_t *in_vertices, int vert_count)
{
	int i;
	compact_vertex_t *out_vertices;
	
	//out_vertices = malloc(sizeof(compact_vertex_t) * vert_count * 10);
	out_vertices = memory_Malloc(sizeof(compact_vertex_t) * vert_count, "model_ConvertVertices");
	
	
	for(i = 0; i < vert_count; i++)
	{
		out_vertices[i].position = in_vertices[i].position;
		
		out_vertices[i].normal = 0;
		out_vertices[i].normal |= (0x03ff) & ((short)(0x01ff * in_vertices[i].normal.z));
		out_vertices[i].normal <<= 10;
		out_vertices[i].normal |= (0x03ff) & ((short)(0x01ff * in_vertices[i].normal.y));
		out_vertices[i].normal <<= 10;
		out_vertices[i].normal |= (0x03ff) & ((short)(0x01ff * in_vertices[i].normal.x));
		
		
		out_vertices[i].tangent = 0;
		out_vertices[i].tangent |= (0x03ff) & ((short)(0x01ff * in_vertices[i].tangent.z));
		out_vertices[i].tangent <<= 10;
		out_vertices[i].tangent |= (0x03ff) & ((short)(0x01ff * in_vertices[i].tangent.y));
		out_vertices[i].tangent <<= 10;
		out_vertices[i].tangent |= (0x03ff) & ((short)(0x01ff * in_vertices[i].tangent.x));
		
		out_vertices[i].tex_coord = in_vertices[i].tex_coord;
	}
	
	
	return out_vertices;
	
	
}


/*
=======================================================================
=======================================================================
=======================================================================
*/


void model_SerializeModels(void **buffer, int *buffer_size)
{
	#if 0
	int out_size = 0;
	int model_count = 0;
	char *out;
	char *name;
	int i;
	int j;
	int name_cursor;
	
	model_section_header_t *header;
	model_record_t *record;
	
	
	out_size += sizeof(model_section_header_t);
	
	for(i = 0; i < mdl_model_list_cursor; i++)
	{
		if(mdl_models[i].flags & MODEL_INVALID)
		{
			continue;
		}
		model_count++;
		out_size += sizeof(model_record_t);
	}
	
	
	out = memory_Malloc(out_size, "model_SerializeModels");
	header = out;
	*buffer = out;
	//out_size = 0;
	//*buffer_size = out_size;
	
	header->model_count = model_count;
	out += sizeof(model_section_header_t);
	
	header->tag[0]  = '[';
	header->tag[1]  = 'm';
	header->tag[2]  = 'o';
	header->tag[3]  = 'd';
	header->tag[4]  = 'e';
	header->tag[5]  = 'l';
	header->tag[6]  = '_';
	header->tag[7]  = 's';
	header->tag[8]  = 'e';
	header->tag[9]  = 'c';
	header->tag[10] = 't';
	header->tag[11] = 'i';
	header->tag[12] = 'o';
	header->tag[13] = 'n';
	header->tag[14] = ']';
	header->tag[15] = '\0';
	
	header->reserved0 = 0;
	header->reserved1 = 0;
	header->reserved2 = 0;
	header->reserved3 = 0;
	header->reserved4 = 0;
	header->reserved5 = 0;
	header->reserved6 = 0;
	header->reserved7 = 0;
	
	for(i = 0; i < mdl_model_list_cursor; i++)
	{
		if(mdl_models[i].flags & MODEL_INVALID)
		{
			continue;
		}
		name_cursor = 0;
		
		record = (model_record_t *)out;
		
		name = mdl_models[i].name;
		for(j = 0; name[j]; j++)
		{
			record->names[j] = name[j];
		}
		
		record->names[j] = '\0';
		
		name_cursor = j + 1;
		
		name = mdl_models[i].file_name;
		for(j = 0; mdl_models[i].file_name[j]; j++)
		{
			record->names[name_cursor + j] = name[j];
		}
		
		name_cursor += j;
		
		/* round the offset to a multiple of four... */
		name_cursor = (name_cursor - 3) & (~3);
		
		out += name_cursor;
	}
	
	*buffer_size = out - (char *)header;
	
	#endif
	
}

void model_DeserializeModels(void **buffer)
{
	#if 0
	model_section_header_t *header;
	model_record_t *record;
	char *in;
	
	int i;
	int j;
	
	char *name;
	char model_name[PATH_MAX];
	char file_name[PATH_MAX];
	
	in = *buffer;
	
	header = (model_section_header_t *)in;
	in += sizeof(model_section_header_t);
	
	for(i = 0; i < header->model_count; i++)
	{
		record = (model_record_t *)in;
		
		/* although the 'names' field is equal to
		the address of the model_record_t, doing
		this will allow modifications to the struct 
		without breaking the code... */
		in += (int)&(((model_record_t *)0)->names[0]);
		
		for(j = 0; in[j]; j++)
		{
			model_name[j] = in[j];
		}
		
		in += j + 1;
		
		for(j = 0; in[j]; j++)
		{
			file_name[j] = in[j];
		}
		
		in += j + 1;
		
		/* records are always 4 byte aligned... */
		in = (char *)(((int)in + 3) & (~3));
				
		model_LoadModel(file_name, model_name);
	}
	
	*buffer = in;
	
	#endif
}

/*
=======================================================================
=======================================================================
=======================================================================
*/




#ifdef __cplusplus
}
#endif








