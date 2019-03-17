#define LIGHT_MAIN_FILE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <intrin.h>
#include <math.h>
#include <float.h>
#include <limits.h>

#include "GL/glew.h"
#include "SDL2/SDL_thread.h"
#include "SDL2/SDL_mutex.h"

#include "bsp.h"

#include "l_main.h"
#include "l_cache.h"
#include "camera.h"
#include "vector.h"
#include "matrix.h"
#include "shader.h"
#include "gpu.h"
#include "c_memory.h"
#include "r_debug.h"

#include "entity.h"
#include "ent_common.h"

#include "containers/stack_list.h"

#include "engine.h"


#include "l_globals.h"

int l_light_count = 0;
struct stack_list_t l_light_positions;
struct stack_list_t l_light_params;
struct stack_list_t l_light_clusters;

//int l_light_list_size = 0;
//int l_light_list_cursor = 0;
//int l_light_count = 0;
//int l_free_position_stack_top = -1;
//int *l_free_position_stack = NULL;
//light_position_t *l_light_positions = NULL;
//light_params_t *l_light_params = NULL;
//bsp_striangle_t *l_light_visible_triangles = NULL;
//char **l_light_names = NULL;

//static int visible_light_list_size;
//int visible_light_count;
//light_position_t *visible_light_positions;
//light_params_t *visible_light_params;


//unsigned int l_shadow_map_frame_buffer;
static unsigned int l_light_indexes_texture;
//unsigned int cluster_texture;
int stencil_light_mesh_vert_count;
unsigned int stencil_light_mesh_handle;
unsigned int stencil_light_mesh_start;
//unsigned int l_shared_shadow_map;
//unsigned int l_indirection_texture;
//unsigned int l_shadow_maps_array;

//static int cluster_x_divs;
//static int cluster_y_divs;
//static unsigned short *clusters = NULL;
unsigned char *l_light_indexes = NULL;
//cluster_t *l_clusters = NULL;

/* from r_main.c */
extern int r_window_width;
extern int r_window_height;
extern int r_width;
extern int r_height;
extern int r_frame;

/* from world.c */
extern struct bsp_pnode_t *w_world_nodes;
extern struct bsp_dleaf_t *w_world_leaves;
extern int w_world_leaves_count;
extern bsp_lights_t *w_leaf_lights;
extern int w_visible_leaves_count;
extern struct bsp_dleaf_t **w_visible_leaves;
extern int w_world_vertices_count;
extern vertex_t *w_world_vertices;


struct gpu_light_t *l_gpu_light_buffer;



extern struct stack_list_t ent_entities[2];

/* from l_cache.c */
//extern int light_cache_cursor;
//extern int light_cache_stack_top;
//extern int light_cache_free_stack[];
//extern unsigned int light_cache_uniform_buffer;
//extern int *light_cache_index_buffer_base;
//extern int *light_cache_index_buffers[];
//extern light_cache_slot_t light_cache[];



//mat4_t l_shadow_map_mats[6];
//mat4_t l_shadow_map_projection_matrix;

#define MAX_VISIBLE_LIGHTS 32
//#define MAX_SHADOW_MAP_RES 1024

//int visible_light_count;
//int visible_lights[MAX_WORLD_LIGHTS];

//int free_chunk_count;
//int free_chunk_size;
//ks_chunk_t *free_chunks;

//int alloc_chunk_count;
//int alloc_chunk_size;
//ks_chunk_t *alloc_chunks;

//int l_allocd_shadow_map_count = 0;
//static int max_shadow_maps = 0;
unsigned int l_used_shadow_maps = 0;
struct shadow_map_t *l_shadow_maps = NULL;


int l_shadow_maps_resolution = 0;


//SDL_Thread *cluster_thread0;
//SDL_Thread *cluster_thread1;

//SDL_mutex *cluster_thread0_in_lock;
//SDL_mutex *cluster_thread1_in_lock;

//SDL_mutex *cluster_thread0_out_lock;
//SDL_mutex *cluster_thread1_out_lock;

//int free_shadow_map_x;
//int free_shadow_map_y;

int light_Init()
{
	int i;
	int c;
	int j;
	int k;
	int r;
//	cluster_t *cls;
//	l_light_list_size = MAX_WORLD_LIGHTS;
//	l_light_count = 0;
//	l_free_position_stack_top = -1;
	mat3_t m;
	vec2_t indirection;


	float *stencil_light_mesh;

	R_DBG_PUSH_FUNCTION_NAME();

	//l_light_positions = memory_Malloc(sizeof(light_position_t) * l_light_list_size);
	//l_light_params = memory_Malloc(sizeof(light_params_t) * l_light_list_size);
	//l_light_names = memory_Malloc(sizeof(char *) * l_light_list_size);
	//l_free_position_stack = memory_Malloc(sizeof(int ) * l_light_list_size);
	//l_light_visible_triangles = memory_Malloc(sizeof(bsp_striangle_t) * l_light_list_size * MAX_TRIANGLES_PER_LIGHT);


    l_light_positions = stack_list_create(sizeof(struct light_position_data_t), W_MAX_WORLD_LIGHTS, NULL);
    l_light_params = stack_list_create(sizeof(struct light_params_data_t), W_MAX_WORLD_LIGHTS, NULL);
    l_light_clusters = stack_list_create(sizeof(struct light_cluster_data_t), W_MAX_WORLD_LIGHTS, NULL);

	l_shadow_maps = memory_Calloc(MAX_VISIBLE_LIGHTS, sizeof(struct shadow_map_t));
//	l_clusters = memory_Malloc(sizeof(cluster_t) * CLUSTERS_PER_ROW * CLUSTER_ROWS * CLUSTER_LAYERS);

	//for(i = 0; i < l_light_list_size; i++)
	//{
	//	l_light_names[i] = memory_Malloc(LIGHT_MAX_NAME_LEN);

	//	l_light_params[i].visible_triangles = list_create(sizeof(int), 8192, NULL);

		//l_light_params[i].triangle_indices[0] = list_create(sizeof(int), 128, NULL);
		//l_light_params[i].triangle_indices[1] = list_create(sizeof(int), 128, NULL);
		//l_light_params[i].triangle_indices[2] = list_create(sizeof(int), 128, NULL);
		//l_light_params[i].triangle_indices[3] = list_create(sizeof(int), 128, NULL);
		//l_light_params[i].triangle_indices[4] = list_create(sizeof(int), 128, NULL);
		//l_light_params[i].triangle_indices[5] = list_create(sizeof(int), 128, NULL);
		/*l_light_params[i].view_cluster_list_size = 16;
		l_light_params[i].view_cluster_list_cursor = 0;
		l_light_params[i].view_clusters = memory_Malloc(sizeof(unsigned int) * l_light_params[i].view_cluster_list_size, "light_Init");*/
	//}



	CreatePerspectiveMatrix(&l_shadow_map_projection_matrix, (45.17578125 * 3.14159265) / 180.0, 1.0, 0.5, LIGHT_MAX_RADIUS * 10.0, 0.0, 0.0, NULL);

	//visible_light_list_size = MAX_LIGHTS;
	//visible_light_count = 0;

	//visible_light_positions = malloc(sizeof(light_position_t) * visible_light_list_size);
	//visible_light_params = malloc(sizeof(light_params_t) * visible_light_list_size);

	//glGenFramebuffers(1, &l_shadow_map_frame_buffer);


	m=mat3_t_id();
	mat3_t_rotate(&m, vec3_t_c(0.0, 1.0, 0.0), -0.5, 1);
	mat3_t_rotate(&m, vec3_t_c(1.0, 0.0, 0.0), -1.0, 0);
	//mat3_t_transpose(&m);
	mat4_t_compose(&l_shadow_map_mats[0], &m, vec3_t_c(0.0, 0.0, 0.0));



	m=mat3_t_id();
	mat3_t_rotate(&m, vec3_t_c(0.0, 1.0, 0.0), 0.5, 1);
	mat3_t_rotate(&m, vec3_t_c(1.0, 0.0, 0.0), 1.0, 0);
	mat4_t_compose(&l_shadow_map_mats[1], &m, vec3_t_c(0.0, 0.0, 0.0));



	m=mat3_t_id();
	mat3_t_rotate(&m, vec3_t_c(1.0, 0.0, 0.0), -0.5, 1);
	//mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), 1.0, 0);
	mat4_t_compose(&l_shadow_map_mats[2], &m, vec3_t_c(0.0, 0.0, 0.0));



	m=mat3_t_id();
	mat3_t_rotate(&m, vec3_t_c(1.0, 0.0, 0.0), 0.5, 1);
	//mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), 0.5, 0);
	mat4_t_compose(&l_shadow_map_mats[3], &m, vec3_t_c(0.0, 0.0, 0.0));



	m=mat3_t_id();
	//mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), -1.0, 1);
	mat3_t_rotate(&m, vec3_t_c(0.0, 0.0, 1.0), 1.0, 1);
	mat4_t_compose(&l_shadow_map_mats[5], &m, vec3_t_c(0.0, 0.0, 0.0));



	m=mat3_t_id();
	mat3_t_rotate(&m, vec3_t_c(0.0, 1.0, 0.0), 1.0, 1);
	mat3_t_rotate(&m, vec3_t_c(0.0, 0.0, 1.0), -1.0, 0);
	mat4_t_compose(&l_shadow_map_mats[4], &m, vec3_t_c(0.0, 0.0, 0.0));



	//printf("cluster texture: %x\n", glGetError());

	light_SetShadowMapsResolution(SHADOW_MAP_RESOLUTION);


	/*while(glGetError() != GL_NO_ERROR);
	glGenTextures(1, &l_shadow_maps_array);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, l_shadow_maps_array);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT32F, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, MAX_VISIBLE_LIGHTS * 6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

	if(glGetError() == GL_OUT_OF_MEMORY)
	{
		log_LogMessage(LOG_MESSAGE_ERROR, "light_Init: out of graphics memory!");
		return 0;
	}*/


	//glGenBuffers(1, &l_light_uniform_buffer);
	//glBindBuffer(GL_UNIFORM_BUFFER, l_light_uniform_buffer);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(struct gpu_light_t) * LIGHT_UNIFORM_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//l_gpu_light_buffer = memory_Malloc(sizeof(struct gpu_light_t) * LIGHT_UNIFORM_BUFFER_SIZE);


	//printf("wow:: %x\n", glGetError());

	/*glGenTextures(1, &shared_shadow_map);
	glBindTexture(GL_TEXTURE_2D, shared_shadow_map);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	while(glGetError() != GL_NO_ERROR);
	// ~100MB for shadow maps...
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, SHARED_SHADOW_MAP_WIDTH, SHARED_SHADOW_MAP_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	if(glGetError() == GL_OUT_OF_MEMORY)
	{
		log_LogMessage(LOG_MESSAGE_ERROR, "light_Init: out of graphics memory!");
		return 0;
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadow_map_frame_buffer);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shared_shadow_map, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glViewport(0, 0, SHARED_SHADOW_MAP_WIDTH, SHARED_SHADOW_MAP_HEIGHT);

	glClearColor(LIGHT_MAX_RADIUS, LIGHT_MAX_RADIUS, LIGHT_MAX_RADIUS, LIGHT_MAX_RADIUS);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);

	glViewport(0, 0, r_width, r_height);


	glGenTextures(1, &indirection_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, indirection_texture);

	indirection.x = 0.0;
	indirection.y = 0.0;
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RG16F, 1, 1, 0, GL_RG, GL_FLOAT, &indirection.floats[0]);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);*/

	//light_InitCache();


	//model_GenerateIcoSphere(1.0, 1, &stencil_light_mesh, &stencil_light_mesh_vert_count);
	//stencil_light_mesh_vert_count *= 3;
	//stencil_light_mesh_handle = gpu_Alloc(stencil_light_mesh_vert_count * sizeof(float) * 3);
	//stencil_light_mesh_handle = gpu_AllocAlign(stencil_light_mesh_vert_count * sizeof(float) * 3, sizeof(float) * 3, 1);
	//stencil_light_mesh_start = gpu_GetAllocStart(stencil_light_mesh_handle) / sizeof(vertex_t);
	//gpu_Write(stencil_light_mesh_handle, 0, stencil_light_mesh, stencil_light_mesh_vert_count * sizeof(float) * 3);

	/*while(glGetError() != GL_NO_ERROR);
	glGenBuffers(1, &stencil_meshes);
	glBindBuffer(GL_ARRAY_BUFFER, stencil_meshes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * light_stencil_mesh_vert_count, light_stencil_mesh, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	assert(glGetError() != GL_OUT_OF_MEMORY);*/

	//free(stencil_light_mesh);
	//memory_Free(stencil_light_mesh);


	//free_chunk_count = 1;
	//free_chunk_size = 1024;
	//free_chunks = memory_Malloc(sizeof(ks_chunk_t) * free_chunk_size, "light_Init");
	//free_chunks[0].x = 0;
	//free_chunks[0].y = 0;
	//free_chunks[0].w = SHARED_SHADOW_MAP_WIDTH;
	//free_chunks[0].h = SHARED_SHADOW_MAP_HEIGHT;

	//alloc_chunk_count = 0;
	//alloc_chunk_size = 1024;
	//alloc_chunks = memory_Malloc(sizeof(ks_chunk_t) * alloc_chunk_size, "light_Init");

//	c = SHARED_SHADOW_MAP_WIDTH / (SHADOW_MAP_RESOLUTION * 3);
//	k = SHARED_SHADOW_MAP_HEIGHT / (SHADOW_MAP_RESOLUTION * 2);

//	max_shadow_maps = c * k;

//	l_shadow_maps = memory_Malloc(sizeof(shadow_map_t) * max_shadow_maps, "light_Init");

//	r = 0;
//	for(i = 0; i < c; i++)
//	{
//		for(j = 0; j < k; j++)
//		{
//			l_shadow_maps[r].x = i * SHADOW_MAP_RESOLUTION * 3;
//			l_shadow_maps[r].y = j * SHADOW_MAP_RESOLUTION * 2;
//			r++;
//		}
//	}

//	free_shadow_map_x = l_shadow_maps[r - 1].x;
//	free_shadow_map_y = l_shadow_maps[r - 1].y;

	/*free_shadow_map_x = 0;
	free_shadow_map_y = 0;*/






	/*cluster_thread0_in_lock = SDL_CreateMutex();
	SDL_LockMutex(cluster_thread0_in_lock);

	cluster_thread1_in_lock = SDL_CreateMutex();
	SDL_LockMutex(cluster_thread1_in_lock);


	cluster_thread0_out_lock = SDL_CreateMutex();
	cluster_thread0_out_lock = SDL_CreateMutex();*/

	//cluster_thread0 = SDL_CreateThread(light_ClusterThread0, "cluster_thread0", NULL);
	//cluster_thread1 = SDL_CreateThread(light_ClusterThread1, "cluster_thread1", NULL);


	R_DBG_POP_FUNCTION_NAME();

	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "%s: subsystem initialized properly!", __func__);

	return 1;

}

void light_Finish()
{
	int i;
	int c;

	//light_FinishCache();

	//for(i = 0; i < l_light_list_size; i++)
	//{
		//memory_Free(l_light_names[i]);

	//	list_destroy(&l_light_params[i].visible_triangles);
		//list_destroy(&l_light_params[i].triangle_indices[0]);
		//list_destroy(&l_light_params[i].triangle_indices[1]);
		//list_destroy(&l_light_params[i].triangle_indices[2]);
		//list_destroy(&l_light_params[i].triangle_indices[3]);
		//list_destroy(&l_light_params[i].triangle_indices[4]);
		//list_destroy(&l_light_params[i].triangle_indices[5]);
		//memory_Free(l_light_params[i].view_clusters);
	//}

    stack_list_destroy(&l_light_positions);
    stack_list_destroy(&l_light_params);
    stack_list_destroy(&l_light_clusters);


	//memory_Free(l_light_names);
	//memory_Free(l_light_positions);
	//memory_Free(l_light_params);
	//memory_Free(l_free_position_stack);
	//memory_Free(l_light_visible_triangles);

	//free(free_chunks);
	//free(alloc_chunks);

//	memory_Free(l_shadow_maps);
//	memory_Free(l_clusters);
	//free(visible_light_positions);
	//free(visible_light_params);
	//memory_Free(light_cache_index_buffer_base);
	//free(clusters);

	glDeleteBuffers(1, &l_light_uniform_buffer);
	glDeleteTextures(1, &l_shared_shadow_map);
	//glDeleteTextures(1, &light_indexes_texture);
}

void light_SetShadowMapsResolution(int resolution)
{
	/*int i;

	int shadow_map_index;

	if(resolution != l_shadow_maps_resolution)
	{
		if(l_shadow_maps_array)
		{
			glDeleteTextures(1, &l_shadow_maps_array);
		}

		l_shadow_maps_resolution = resolution;

		glGenTextures(1, &l_shadow_maps_array);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, l_shadow_maps_array);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT16, l_shadow_maps_resolution, l_shadow_maps_resolution, MAX_VISIBLE_LIGHTS * 6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

		shadow_map_index = 0;

		for(i = 0; i < MAX_VISIBLE_LIGHTS; i++)
		{
            if(l_used_shadow_maps & (1 << i))
			{
                l_light_params[l_shadow_maps[i].light_index].bm_flags |= LIGHT_UPDATE_SHADOW_MAP;
			}
		}


	}*/
}

int light_GetShadowMapsResolution()
{
	return l_shadow_maps_resolution;
}

int light_CreateLight(char *name, mat3_t *orientation, vec3_t position, vec3_t color, float radius, float energy, int bm_flags)
{
	int light_index;
	int cluster_data_index;
	int i;

	struct light_position_data_t *light_position;
	struct light_params_data_t *light_params;
	struct light_cluster_data_t *light_cluster;

	//light_position_t *light_position;
	//light_params_t *light_param;
	char *light_name;
	//char light_name_str[64];
	int name_len = 0;
/*

	if(l_free_position_stack_top > -1)
	{
		light_index = l_free_position_stack[l_free_position_stack_top--];
	}
	else
	{
		if(light_index >= MAX_WORLD_LIGHTS)
		{
			return -1;
		}

		l_light_list_cursor++;

	}
*/

    light_index = stack_list_add(&l_light_positions, NULL);
    stack_list_add(&l_light_params, NULL);
    stack_list_add(&l_light_clusters, NULL);



	if(radius < LIGHT_MIN_RADIUS) radius = LIGHT_MIN_RADIUS;
	else if(radius > LIGHT_MAX_RADIUS) radius = LIGHT_MAX_RADIUS;

	if(energy < LIGHT_MIN_ENERGY) energy = LIGHT_MIN_ENERGY;
	else if(energy > LIGHT_MAX_ENERGY) energy = LIGHT_MAX_ENERGY;

	//light_position = &l_light_positions[light_index];
	//light_param = &l_light_params[light_index];
	//light_name = l_light_names[light_index];

    light_position = (struct light_position_data_t *)l_light_positions.elements + light_index;
    light_params = (struct light_params_data_t *)l_light_params.elements + light_index;
    light_cluster = (struct light_cluster_data_t *)l_light_clusters.elements + light_index;

	light_position->position = position;
	light_position->orientation = *orientation;
	//light_position->world_to_light_matrix = mat4_t_id();

	//light_position->radius = 0xffff * (radius / LIGHT_MAX_RADIUS);
	light_position->radius = PACK_LIGHT_RADIUS(radius);
	light_position->flags = (LIGHT_MOVED | bm_flags) & (~LIGHT_INVALID);
	//light_position->energy = 0xffff * (energy / LIGHT_MAX_ENERGY);
	light_position->leaf = NULL;
	//light_position->first_cluster_data = cluster_data_index;


	light_params->r = 0xff * color.r;
	light_params->g = 0xff * color.g;
	light_params->b = 0xff * color.b;
	//light_param->cache = -1;
	//light_params->shadow_map = L_INVALID_SHADOW_MAP_INDEX;
	light_params->first_triangle = 0;
	light_params->triangle_count = 0;
	light_params->energy = PACK_LIGHT_ENERGY(energy);

	if(!light_params->name)
    {
        light_params->name = memory_Calloc(LIGHT_MAX_NAME_LEN, 1);
    }

    for(i = 0; i < LIGHT_MAX_NAME_LEN - 1 && name[i]; i++)
    {
        light_params->name[i] = name[i];
    }
	//light_param->bm_flags = (LIGHT_MOVED | bm_flags) & (~LIGHT_INVALID);

	//light_params->flags = LIGHT_MOVED;

	//light_param->indices_handle = INVALID_GPU_ALLOC_HANDLE;

	//if(!(bm_flags & LIGHT_GENERATE_SHADOWS))
	//{
	//	light_param->x = free_shadow_map_x;
	//	light_param->y = free_shadow_map_y;
	//}

	//light_param->leaf = NULL;
	//light_param->shadow_map = -1;
//	light_param->box_max.x = -999999999999.9;
//	light_param->box_max.y = -999999999999.9;
//	light_param->box_max.z = -999999999999.9;
//	light_param->box_min.x = 999999999999.9;
//	light_param->box_min.y = 999999999999.9;
//	light_param->box_min.z = 999999999999.9;

	//light_param->view_cluster_list_cursor = 0;

	//name_len = strlen(name);
	//if(name_len + 1 >= LIGHT_MAX_NAME_LEN)
	//{
		/* names longer than LIGHT_MAX_NAME_LEN get truncated... */
		//name[LIGHT_MAX_NAME_LEN - 1] = '\0';
	//	name_len = LIGHT_MAX_NAME_LEN - 1;
	//}

	//strcpy(*light_name, name);

	//l_light_count++;

	//memcpy(light_name, name, name_len);
	//light_name[name_len] = '\0';

	l_light_count++;

	return light_index;
}

int light_CopyLightIndex(int light_index)
{
    struct light_pointer_t light_ptr;
    int new_light_index = -1;
    vec3_t light_color;
    float energy;
    float radius;

    light_ptr = light_GetLightPointerIndex(light_index);

    if(light_ptr.position)
    {
        light_color.r = (float)light_ptr.params->r / 255.0;
        light_color.g = (float)light_ptr.params->g / 255.0;
        light_color.b = (float)light_ptr.params->b / 255.0;

        energy = UNPACK_LIGHT_ENERGY(light_ptr.params->energy);
        radius = UNPACK_LIGHT_RADIUS(light_ptr.position->radius);

        new_light_index = light_CreateLight("copy", &light_ptr.position->orientation, light_ptr.position->position, light_color, radius, energy, 0);
    }

    return new_light_index;
}


int light_DestroyLight(char *name)
{
	/*int light_index;
	int i;


	for(i = 0; i < l_light_list_cursor; i++)
	{
		if(l_light_params[i].bm_flags & LIGHT_INVALID)
			continue;

		if(!strcmp(name, l_light_names[i]))
		{
			break;
		}
	}

	return light_DestroyLightIndex(light_index);*/

}

int light_DestroyLightIndex(int light_index)
{
	struct bsp_dleaf_t *leaf;
	int i;
	int leaf_index;
	int int_index;
	int bit_index;

	struct light_pointer_t light_ptr;

	light_ptr = light_GetLightPointerIndex(light_index);

	if(light_ptr.position)
    {
        light_ptr.position->flags |= LIGHT_INVALID;

        stack_list_remove(&l_light_positions, light_index);
        stack_list_remove(&l_light_params, light_index);
        stack_list_remove(&l_light_clusters, light_index);

        if(light_ptr.position->leaf)
        {
            leaf = (struct bsp_dleaf_t *)light_ptr.position->leaf;

			leaf_index = leaf - w_world_leaves;

			int_index = light_index >> 5;
			bit_index = light_index % 32;

			w_leaf_lights[leaf_index].lights[int_index] &= ~(1 << bit_index);


			if(leaf->pvs)
			{
				for(i = 0; i < w_world_leaves_count; i++)
				{
					w_leaf_lights[i].lights[int_index] &= ~(1 << bit_index);
				}
			}
        }

        l_light_count--;
    }

	return 0;
}

void light_DestroyAllLights()
{
	int i;

	for(i = 0; i < l_light_positions.element_count; i++)
	{
		light_DestroyLightIndex(i);
	}
}



struct light_pointer_t light_GetLightPointer(char *name)
{
    return (struct light_pointer_t){NULL, NULL, NULL};
}

struct light_pointer_t light_GetLightPointerIndex(int light_index)
{
    struct light_position_data_t *position;
    struct light_pointer_t light_pointer = {NULL, NULL, NULL};

    if(light_index >= 0 && light_index < l_light_positions.element_count)
    {
        position = (struct light_position_data_t *)l_light_positions.elements + light_index;

        if(!(position->flags & LIGHT_INVALID))
        {
            light_pointer.position = position;
            light_pointer.params = (struct light_params_data_t *)l_light_params.elements + light_index;
            light_pointer.cluster = (struct light_cluster_data_t *)l_light_clusters.elements + light_index;
        }
    }

    return light_pointer;
}


struct light_pointer_t l_valid_lights[MAX_WORLD_LIGHTS];

struct light_pointer_t *light_GetValidLights(int *light_count)
{
    int i;
    int count = 0;

    struct light_position_data_t *light_position;

    for(i = 0; i < l_light_positions.element_count; i++)
    {
        light_position = (struct light_position_data_t *)l_light_positions.elements;

        if(light_position->flags & LIGHT_INVALID)
        {
            continue;
        }

        l_valid_lights[count].position = light_position;
        l_valid_lights[count].params = (struct light_params_data_t *)l_light_params.elements + i;
        l_valid_lights[count].cluster = (struct light_cluster_data_t *)l_light_clusters.elements + i;

        count++;
    }

    *light_count = count;

    return l_valid_lights;
}


void light_VisibleTriangles(int light_index)
{

	#if 0
	int slot_index;
	int i;
	int c;
	int j;
	int r;
	int first_vertex_index;
	float radius;
	float sqrd_radius;
	float dist;
	light_params_t *parms;
	light_position_t *pos;
	bsp_striangle_t *visible_triangles;
	int *visible_tris;
	vertex_t *first_vertex;
	vec3_t v;
	vec3_t p;

	vec3_t box_max = {-9999999999.9, -9999999999.9, -9999999999.9};
	vec3_t box_min = {9999999999.9, 9999999999.9, 9999999999.9};

	vec3_t close_max = {-9999999999.9, -9999999999.9, -9999999999.9};
	vec3_t close_min = {9999999999.9, 9999999999.9, 9999999999.9};


	bsp_dleaf_t *leaf;

	if(!world_leaves)
		return;

	if(light_index >= 0 && light_index <= l_light_list_cursor)
	{
		if(!(l_light_params[light_index].bm_flags & LIGHT_INVALID))
		{
			parms = &l_light_params[light_index];
			pos = &l_light_positions[light_index];
			leaf = (bsp_dleaf_t *)parms->leaf;

			if(leaf)
			{
				radius = LIGHT_RADIUS(parms->radius);
				sqrd_radius = radius * radius;

				//printf("update visible triangles\n");

				parms->visible_triangle_count = 0;
				parms->bm_flags |= LIGHT_NEEDS_REUPLOAD;
				slot_index = light_index * MAX_TRIANGLES_PER_LIGHT;
				visible_triangles = &l_light_visible_triangles[slot_index];

				for(r = 0; r < world_leaves_count; r++)
				{
					if(leaf_lights[r].lights[light_index >> 5] & (1 << (light_index % 32)))
					{
						leaf = &world_leaves[r];

						c = leaf->tris_count;

						for(i = 0; i < c && i < MAX_TRIANGLES_PER_LIGHT; i++)
						{
							first_vertex = &world_vertices[leaf->tris[i].first_vertex];
							//first_vertex = &world_vertices[TRIS_FIRST_VERTEX(leaf->tris[i])];

							for(j = 0; j < 3; j++)
							{
								v.x = first_vertex[j].position.x - pos->position.x;
								v.y = first_vertex[j].position.y - pos->position.y;
								v.z = first_vertex[j].position.z - pos->position.z;

								dist = dot3(v, v);

								if(dist < sqrd_radius)
								{
									*visible_triangles = leaf->tris[i];
									visible_triangles++;
									parms->visible_triangle_count++;
									break;
								}

							}
						}
					}
				}

			}
		}
	}

	#endif


}

void light_ClearLightLeaves()
{
	int i;
	int j;

	#if 0


	struct bsp_dleaf_t *leaf;
	int leaf_index;
	for(i = 0; i < l_light_list_cursor; i++)
	{
		l_light_params[i].leaf = NULL;
		l_light_params[i].bm_flags |= LIGHT_MOVED;
	}

	/*for(i = 0; i < w_world_leaves_count; i++)
	{
        for(j = 0; j < MAX_WORLD_LIGHTS >> 5; j++)
		{
			w_leaf_lights[i].lights[j] = 0;
		}
	}*/

	#endif
}

void light_EntitiesOnLights()
{
    int i;
    int entity_count;
    struct entity_t *entities;
    struct entity_t *entity;



/*
    entities = (struct entity_t *)ent_entities.elements;
    entity_count = ent_entities.element_count;


    for(i = 0; i < entity_count; i++)
	{
        entity = entities + i;

		if(entity->flags & ENTITY_FLAG_INVALID)
		{
            continue;
		}



	}*/


}

void light_TranslateLight(int light_index, vec3_t direction, float amount)
{
	//light_position_t *light_position;
	struct light_position_data_t *light_position;

	if(light_index >= 0 && light_index < l_light_positions.element_count)
	{
		light_position = (struct light_position_data_t *)l_light_positions.elements + light_index;

		if(!(light_position->flags & LIGHT_INVALID))
        {
            light_position->position.x += direction.x * amount;
            light_position->position.y += direction.y * amount;
            light_position->position.z += direction.z * amount;
            light_position->flags |= LIGHT_MOVED | LIGHT_UPDATE_SHADOW_MAP;
        }
	}
}

vec3_t light_GetLightPosition(int light_index)
{
    struct light_pointer_t light;

    light = light_GetLightPointerIndex(light_index);

    if(light.position)
    {
        return light.position->position;
    }

    return vec3_t_c(0.0, 0.0, 0.0);
}

int cur_light_index = -1;

/*void light_SetLight(int light_index)
{
	#if 0
	int cache_index;
	if(light_index >= 0 && light_index < light_count)
	{
		cache_index = light_params[light_index].cache;

		if(cache_index >= 0)
		{
			if(light_cache[cache_index].last_touched == r_frame)
			{
				shader_SetCurrentShaderUniform1i(UNIFORM_LIGHT_INDEX, light_cache[cache_index].offset);
				cur_light_index = light_index;
			}
		}
	}
	#endif
}*/



void light_AllocShadowMap(int light_index)
{
    #if 0
	light_params_t *parms;
	light_params_t *other;

	struct shadow_map_t *shadow_map;

	int shadow_map_index;

	unsigned int shadow_maps;

	int oldest_index;
	int oldest_time = 0;
	int cur_time;

	if(l_allocd_shadow_map_count >= MAX_VISIBLE_LIGHTS)
		return;

	if(light_index >= 0 && light_index < l_light_list_cursor)
	{
		parms = &l_light_params[light_index];

		if(!(parms->bm_flags & LIGHT_INVALID))
		{
			/* only lights that can generate shadows may have
			shadow maps allocated to them... */
			//if(parms->bm_flags & LIGHT_GENERATE_SHADOWS)
			//{

			if(parms->shadow_map > -1)
			{
				shadow_map = l_shadow_maps + parms->shadow_map;
				shadow_map->last_touched_frame = r_frame;
				return;
			}

			shadow_maps = l_used_shadow_maps;

			shadow_map_index = 0;
            while(shadow_map_index < MAX_VISIBLE_LIGHTS)
			{
				if(!(shadow_maps & 1))
				{
					break;
				}

				cur_time = r_frame - l_shadow_maps[shadow_map_index].last_touched_frame;

				if(cur_time > oldest_time)
				{
					oldest_time = cur_time;
					oldest_index = shadow_map_index;
				}

				shadow_maps >>= 1;

				shadow_map_index++;
			}

			if(shadow_map_index >= MAX_VISIBLE_LIGHTS)
			{
				if(oldest_time > 1)
				{
					shadow_map = l_shadow_maps + oldest_index;
					other = l_light_params + shadow_map->light_index;
					other->shadow_map = -1;
					shadow_map_index = oldest_index;
				}
			}



			parms->shadow_map = shadow_map_index;
			parms->bm_flags |= LIGHT_UPDATE_SHADOW_MAP;

			shadow_map = l_shadow_maps + shadow_map_index;
			shadow_map->light_index = light_index;
			shadow_map->last_touched_frame = r_frame;

			l_used_shadow_maps |= 1 << shadow_map_index;


			//l_shadow_maps[l_allocd_shadow_map_count].light_index = light_index;

				//parms->x = l_shadow_maps[l_allocd_shadow_map_count].x;
				//parms->y = l_shadow_maps[l_allocd_shadow_map_count].y;

			//l_allocd_shadow_map_count++;

			//	printf("shadow map %d allocd!\n", parms->shadow_map);
			//}

		}

	}

	#endif
}


void light_FreeShadowMap(int light_index)
{

    #if 0
	light_params_t *parms;
	int shadow_map_index;
	struct shadow_map_t shadow_map;
	if(light_index >= 0 && light_index < l_light_list_cursor)
	{
		parms = &l_light_params[light_index];

		if(!(parms->bm_flags & LIGHT_INVALID))
		{
			if(parms->shadow_map > -1)
			{
				shadow_map_index = parms->shadow_map;

				parms->shadow_map = -1;

				l_used_shadow_maps &= ~(1 << shadow_map_index);
			}
		}
	}

	#endif
}


/*
===================================================================
===================================================================
===================================================================
*/

char light_section_start_tag[] = "[light section start]";

struct light_section_start_t
{
	char tag[(sizeof(light_section_start_tag) + 3) & (~3)];
	int light_count;

	int reserved0;
	int reserved1;
	int reserved2;
	int reserved3;
	int reserved4;
	int reserved5;
	int reserved6;
	int reserved7;
};

char light_section_end_tag[] = "[light section end]";

struct light_section_end_t
{
	char tag[(sizeof(light_section_end_tag) + 3) & (~3)];
};


struct light_record_t
{
	mat3_t orientation;
	vec3_t position;
	vec3_t color;
	float radius;
	float energy;
	int flags;

	int reserved0;
	int reserved1;
	int reserved2;
	int reserved3;
	int reserved4;
	int reserved5;
	int reserved6;
	int reserved7;

	char name[LIGHT_MAX_NAME_LEN];

};



void light_SerializeLights(void **buffer, int *buffer_size)
{
	struct light_section_start_t *section_start;
	struct light_section_end_t *section_end;
	struct light_record_t *record;
	char *out;
	int size;
	int i;

	struct light_position_data_t *position;
	struct light_params_data_t *params;


	size = sizeof(struct light_section_start_t ) + sizeof(struct light_section_end_t) + sizeof(struct light_record_t) * l_light_count;
	out = memory_Calloc(size, 1);

	*buffer = out;
	*buffer_size = size;

	section_start = (struct light_section_start_t *)out;
	out += sizeof(struct light_section_start_t);

	strcpy(section_start->tag, light_section_start_tag);
	section_start->light_count = l_light_count;

	for(i = 0; i < l_light_positions.element_count; i++)
	{

	    position = (struct light_position_data_t *)l_light_positions.elements + i;

		//if(l_light_params[i].bm_flags & LIGHT_INVALID)
		//	continue;

		if(position->flags & LIGHT_INVALID)
        {
            continue;
        }

        params = (struct light_params_data_t *)l_light_params.elements + i;

		record = (struct light_record_t *)out;
		out += sizeof(struct light_record_t );

		record->orientation = position->orientation;
		record->position = position->position;

		record->color.r = (float)params->r / 255.0;
		record->color.g = (float)params->g / 255.0;
		record->color.b = (float)params->b / 255.0;

		record->energy = UNPACK_LIGHT_ENERGY(params->energy);
		record->radius = UNPACK_LIGHT_RADIUS(position->radius);

		strcpy(record->name, params->name);
	}

	section_end = (struct light_section_end_t *)out;
	out += sizeof(struct light_section_end_t);

    strcpy(section_end->tag, light_section_end_tag);
}

void light_DeserializeLights(void **buffer)
{
	struct light_section_start_t *section_start;
	struct light_section_end_t *section_end;
	struct light_record_t *record;
	int i;
	char *in;

	in = *buffer;

	if(in)
    {
        while(1)
        {
            if(!strcmp(in, light_section_start_tag))
            {
                section_start = (struct light_section_start_t *)in;
                in += sizeof(struct light_section_start_t);

                for(i = 0; i < section_start->light_count; i++)
                {
                    record = (struct light_record_t *)in;
                    in += sizeof(struct light_record_t);
                    light_CreateLight(record->name, &record->orientation, record->position, record->color, record->radius, record->energy, record->flags);
                }
            }
            else if(!strcmp(in, light_section_end_tag))
            {
                in += sizeof(struct light_section_end_t);
                break;
            }
            else
            {
                in++;
            }
        }

        *buffer = in;
    }
}




/*void light_EnableShadowMapWrite(int light_index, int face_index)
{
	if(face_index >= 0 && face_index <= 5)
	{

	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, )
}*/















