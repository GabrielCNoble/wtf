#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <intrin.h>

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


static int light_list_size;
int light_count;
static int free_position_stack_top;
static int *free_position_stack;
light_position_t *light_positions;
light_params_t *light_params;
bsp_striangle_t *light_visible_triangles;

char **light_names;

//static int visible_light_list_size;
//int visible_light_count;
//light_position_t *visible_light_positions;
//light_params_t *visible_light_params;


unsigned int shadow_map_frame_buffer;
static unsigned int light_indexes_texture;
unsigned int cluster_texture;
int stencil_light_mesh_vert_count;
unsigned int stencil_light_mesh_handle;
unsigned int stencil_light_mesh_start;
unsigned int shared_shadow_map;

//static int cluster_x_divs;
//static int cluster_y_divs;
//static unsigned short *clusters = NULL;
static unsigned char *light_indexes = NULL;
static cluster_t *clusters;

/* from r_main.c */
extern int window_width;
extern int window_height;
extern int r_frame;

/* from world.c */
extern bsp_pnode_t *world_nodes;
extern bsp_dleaf_t *world_leaves;
extern int world_leaves_count;
extern bsp_lights_t *leaf_lights;
extern int visible_leaves_count;
extern bsp_dleaf_t **visible_leaves;
extern int world_vertices_count;
extern vertex_t *world_vertices;

/* from l_cache.c */
extern int light_cache_cursor;
extern int light_cache_stack_top;
extern int light_cache_free_stack[];
extern unsigned int light_cache_uniform_buffer;
extern int *light_cache_index_buffer_base;
extern int *light_cache_index_buffers[];
extern light_cache_slot_t light_cache[];



mat4_t shadow_map_mats[6];
mat4_t shadow_map_projection_matrix;

#define MAX_VISIBLE_LIGHTS 32

int visible_light_count;
int visible_lights[MAX_VISIBLE_LIGHTS];

int free_chunk_count;
int free_chunk_size;
ks_chunk_t *free_chunks;

int alloc_chunk_count;
int alloc_chunk_size;
ks_chunk_t *alloc_chunks;

int allocd_shadow_map_count = 0;
static int max_shadow_maps = 0;
shadow_map_t *shadow_maps = NULL;


SDL_Thread *cluster_thread0;
SDL_Thread *cluster_thread1;

SDL_mutex *cluster_thread0_in_lock;
SDL_mutex *cluster_thread1_in_lock;

SDL_mutex *cluster_thread0_out_lock;
SDL_mutex *cluster_thread1_out_lock;

void light_Init()
{
	int i;
	int c;
	int j;
	int k;
	int r;
	cluster_t *cls;
	light_list_size = MAX_WORLD_LIGHTS;
	light_count = 0;
	free_position_stack_top = -1;
	mat3_t m;
	
	
	float *stencil_light_mesh;
	
	light_positions = malloc(sizeof(light_position_t) * light_list_size);
	light_params = malloc(sizeof(light_params_t) * light_list_size);
	light_names = malloc(sizeof(char *) * light_list_size);
	free_position_stack = malloc(sizeof(int ) * light_list_size);
	light_visible_triangles = malloc(sizeof(bsp_striangle_t) * light_list_size * MAX_TRIANGLES_PER_LIGHT);
	
	clusters = malloc(sizeof(cluster_t) * CLUSTERS_PER_ROW * CLUSTER_ROWS * CLUSTER_LAYERS);
	
	
	CreatePerspectiveMatrix(&shadow_map_projection_matrix, (45.17578125 * 3.14159265) / 180.0, 1.0, 0.1, LIGHT_MAX_RADIUS * 100.0, 0.0, 0.0, NULL);
	
	//visible_light_list_size = MAX_LIGHTS;
	//visible_light_count = 0;
	
	//visible_light_positions = malloc(sizeof(light_position_t) * visible_light_list_size);
	//visible_light_params = malloc(sizeof(light_params_t) * visible_light_list_size);
	
	glGenFramebuffers(1, &shadow_map_frame_buffer);
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), -0.5, 1);
	mat3_t_rotate(&m, vec3(1.0, 0.0, 0.0), -1.0, 0);
	//mat3_t_transpose(&m);
	mat4_t_compose(&shadow_map_mats[0], &m, vec3(0.0, 0.0, 0.0));
	
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), 0.5, 1);
	mat3_t_rotate(&m, vec3(1.0, 0.0, 0.0), 1.0, 0);
	mat4_t_compose(&shadow_map_mats[1], &m, vec3(0.0, 0.0, 0.0));
	
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(1.0, 0.0, 0.0), -0.5, 1);
	//mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), 1.0, 0);
	mat4_t_compose(&shadow_map_mats[2], &m, vec3(0.0, 0.0, 0.0));
	
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(1.0, 0.0, 0.0), 0.5, 1);
	//mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), 0.5, 0);
	mat4_t_compose(&shadow_map_mats[3], &m, vec3(0.0, 0.0, 0.0));
	
	
	
	m=mat3_t_id();
	//mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), -1.0, 1);
	mat3_t_rotate(&m, vec3(0.0, 0.0, 1.0), 1.0, 1);
	mat4_t_compose(&shadow_map_mats[5], &m, vec3(0.0, 0.0, 0.0));
	
	
	
	m=mat3_t_id();
	mat3_t_rotate(&m, vec3(0.0, 1.0, 0.0), 1.0, 1);
	mat3_t_rotate(&m, vec3(0.0, 0.0, 1.0), -1.0, 0);
	mat4_t_compose(&shadow_map_mats[4], &m, vec3(0.0, 0.0, 0.0));
	
	
	//while(glGetError() != GL_NO_ERROR);
	glGenTextures(1, &cluster_texture);
	glBindTexture(GL_TEXTURE_3D, cluster_texture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE32UI_EXT, CLUSTERS_PER_ROW, CLUSTER_ROWS, CLUSTER_LAYERS, 0, GL_LUMINANCE_INTEGER_EXT, GL_UNSIGNED_INT, NULL);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, CLUSTERS_PER_ROW, CLUSTER_ROWS, CLUSTER_LAYERS, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	
//	printf("wow:: %x\n", glGetError());
	
	
	glGenTextures(1, &shared_shadow_map);
	glBindTexture(GL_TEXTURE_2D, shared_shadow_map);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	
	while(glGetError() != GL_NO_ERROR);
	/* ~100MB for shadow maps... */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE16F_ARB, SHARED_SHADOW_MAP_WIDTH, SHARED_SHADOW_MAP_HEIGHT, 0, GL_LUMINANCE, GL_FLOAT, NULL);
	if(glGetError() == GL_OUT_OF_MEMORY)
	{
		printf("ERROR: out of graphics memory!\n");
		abort();
	}
	
	/*glGenBuffers(1, &cluster_uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, cluster_uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(cluster_t) * CLUSTERS_PER_ROW * CLUSTER_ROWS * CLUSTER_LAYERS, NULL, GL_DYNAMIC_DRAW);
	cls = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	
	
	for(k = 0; k < CLUSTER_LAYERS; k++)
	{
		for(j = 0; j < CLUSTER_ROWS; j++)
		{
			for(i = 0; i < CLUSTERS_PER_ROW; i++)
			{
				cls[CLUSTER_OFFSET(i, j , k)].light_indexes_bm = 0;
				cls[CLUSTER_OFFSET(i, j , k)].time_stamp = 0xffffffff;
			}
		}
	}
	
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);*/
	
	light_InitCache();
	
	
	mesh_GenerateIcoSphere(1.0, 1, &stencil_light_mesh, &stencil_light_mesh_vert_count);
	
	stencil_light_mesh_vert_count *= 3;
	
	stencil_light_mesh_handle = gpu_Alloc(stencil_light_mesh_vert_count * sizeof(float) * 3);
	stencil_light_mesh_start = gpu_GetAllocStart(stencil_light_mesh_handle) / sizeof(vertex_t);
	gpu_Write(stencil_light_mesh_handle, 0, stencil_light_mesh, stencil_light_mesh_vert_count * sizeof(float) * 3, 0);
	
	/*while(glGetError() != GL_NO_ERROR);
	glGenBuffers(1, &stencil_meshes);
	glBindBuffer(GL_ARRAY_BUFFER, stencil_meshes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * light_stencil_mesh_vert_count, light_stencil_mesh, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	assert(glGetError() != GL_OUT_OF_MEMORY);*/
	
	free(stencil_light_mesh);
	
	
	free_chunk_count = 1;
	free_chunk_size = 1024;
	free_chunks = (ks_chunk_t *)malloc(sizeof(ks_chunk_t) * free_chunk_size);
	free_chunks[0].x = 0;
	free_chunks[0].y = 0;
	free_chunks[0].w = SHARED_SHADOW_MAP_WIDTH;
	free_chunks[0].h = SHARED_SHADOW_MAP_HEIGHT;
	
	alloc_chunk_count = 0;
	alloc_chunk_size = 1024;
	alloc_chunks = (ks_chunk_t *)malloc(sizeof(ks_chunk_t) * alloc_chunk_size);	
	
	c = SHARED_SHADOW_MAP_WIDTH / (SHADOW_MAP_RESOLUTION * 3);
	k = SHARED_SHADOW_MAP_HEIGHT / (SHADOW_MAP_RESOLUTION * 2);
	
	max_shadow_maps = c * k;
	
	shadow_maps = malloc(sizeof(shadow_map_t) * max_shadow_maps);
	
	r = 0;
	for(i = 0; i < c; i++)
	{
		for(j = 0; j < k; j++)
		{
			shadow_maps[r].x = i * SHADOW_MAP_RESOLUTION * 3;
			shadow_maps[r].y = j * SHADOW_MAP_RESOLUTION * 2;
			r++;
		}
	}
	
	
	
	
	
	/*cluster_thread0_in_lock = SDL_CreateMutex();
	SDL_LockMutex(cluster_thread0_in_lock);
	
	cluster_thread1_in_lock = SDL_CreateMutex();
	SDL_LockMutex(cluster_thread1_in_lock);
	
	
	cluster_thread0_out_lock = SDL_CreateMutex();
	cluster_thread0_out_lock = SDL_CreateMutex();*/
	
	//cluster_thread0 = SDL_CreateThread(light_ClusterThread0, "cluster_thread0", NULL);
	//cluster_thread1 = SDL_CreateThread(light_ClusterThread1, "cluster_thread1", NULL);
	
	

}

void light_Finish()
{
	int i;
	
	light_FinishCache();
	
	for(i = 0; i < light_count; i++)
	{
		free(light_names[i]);
	}
	
	free(light_names);
	free(light_positions);
	free(light_params);
	free(free_position_stack);
	free(light_visible_triangles);
	
	free(free_chunks);
	free(alloc_chunks);
	
	free(shadow_maps);
	
	free(clusters);	
	//free(visible_light_positions);
	//free(visible_light_params);
	free(light_cache_index_buffer_base);
	//free(clusters);
	
	glDeleteBuffers(1, &light_cache_uniform_buffer);
	glDeleteTextures(1, &shared_shadow_map);
	//glDeleteTextures(1, &light_indexes_texture);
}

int light_CreateLight(char *name, mat3_t *orientation, vec3_t position, vec3_t color, float radius, float energy)
{
	int light_index;
	int i;
	light_position_t *light_position;
	light_params_t *light_param;
	char **light_name;
	char light_name_str[64];
	int light_name_str_len = 0;
	
	
	if(free_position_stack_top > -1)
	{
		light_index = free_position_stack[free_position_stack_top--];
	}
	else
	{
		light_index = light_count;
		
		/* seems non-sensical, given that there's
		resizing code right bellow this... */
		if(light_index >= MAX_WORLD_LIGHTS)
		{
			return -1;
		}
		
		light_count++;
		
		if(light_index >= light_list_size)
		{
			free(free_position_stack);
			free_position_stack = malloc(sizeof(int) * (light_list_size + 16));
			light_position = malloc(sizeof(light_position_t) * (light_list_size + 16));
			light_param = malloc(sizeof(light_params_t) * (light_list_size + 16));
			light_name = malloc(sizeof(char *) * (light_list_size + 16));
			
			
			memcpy(light_position, light_positions, sizeof(light_position_t) * light_list_size);
			memcpy(light_param, light_params, sizeof(light_params_t) * light_list_size);
			memcpy(light_name, light_names, sizeof(char *) * light_list_size);
			
			free(light_positions);
			free(light_params);
			free(light_names);
			
			light_positions = light_position;
			light_params = light_param;
			light_names = light_name;
			
		}
	}
	
	if(radius < LIGHT_MIN_RADIUS) radius = LIGHT_MIN_RADIUS;
	else if(radius > LIGHT_MAX_RADIUS) radius = LIGHT_MAX_RADIUS;
	
	if(energy < LIGHT_MIN_ENERGY) energy = LIGHT_MIN_ENERGY;
	else if(energy > LIGHT_MAX_ENERGY) energy = LIGHT_MAX_ENERGY;
	
	light_position = &light_positions[light_index];
	light_param = &light_params[light_index];
	light_name = &light_names[light_index];
	
	light_position->position = position;
	light_position->orientation = *orientation;
	light_position->world_to_light_matrix = mat4_t_id();
	
	light_param->radius = 0xffff * (radius / LIGHT_MAX_RADIUS);
	light_param->energy = 0xffff * (energy / LIGHT_MAX_ENERGY);
	light_param->r = 0xff * color.r;
	light_param->g = 0xff * color.g;
	light_param->b = 0xff * color.b;
	light_param->cache = -1;
	light_param->bm_flags = LIGHT_MOVED | LIGHT_GENERATE_SHADOWS;
	light_param->leaf = NULL;
	light_param->shadow_map = -1;
	light_param->box_max.x = -999999999999.9;
	light_param->box_max.y = -999999999999.9;
	light_param->box_max.z = -999999999999.9;
	light_param->box_min.x = 999999999999.9;
	light_param->box_min.y = 999999999999.9;
	light_param->box_min.z = 999999999999.9;
	
	
	
	/*glGenTextures(1, &light_param->shadow_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, light_param->shadow_map);
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	for(i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 512, 512, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);	
	}
	
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);*/
	
	
	

	/* TODO: pad name strings to a multiple of 4
	so fast 4 byte comparisions can be done... */
	*light_name = strdup(name);
	
	
	
	return light_index;
	
}



void light_UpdateLights()
{

	int i;
	int j;
	int r;
	int int_index;
	int bit_index;
	int leaf_index;
	int lleaf_index;
	float energy;
	float radius;
	float dist;
	static int sign[4] = {0x80000000, 0x80000000, 0x80000000, 0x80000000}; 
	bsp_dleaf_t *light_leaf;
	bsp_dleaf_t *leaf;
	bsp_dleaf_t *lleaf;
	bsp_lights_t *lights;
	light_position_t *pos;
	light_params_t *parms;
	unsigned long long s;
	unsigned long long e;
	vec3_t v;
	vec4_t v4;
	vec4_t leaf_extents4;
	vec4_t leaf_pos4;
	vec4_t light_pos4;
	
	
	if(!world_leaves)
		return;
	
	for(i = 0; i < light_count; i++)
	{
		if(!(light_params[i].bm_flags & LIGHT_MOVED))
			continue;
		
		light_params[i].bm_flags &= ~LIGHT_MOVED;
			
		light_leaf = bsp_GetCurrentLeaf(world_nodes, light_positions[i].position);
		
		//printf("a\n");
		
		if(light_leaf)
		{
			parms = &light_params[i];
			pos = &light_positions[i];
			
			int_index = i >> 5;
			bit_index = i % 32;
			
			
			if(parms->leaf)
			{
				/* don't update anything if this light didn't
				leave the leaf... */
				if((bsp_dleaf_t *)parms->leaf != light_leaf)
				{
					leaf = (bsp_dleaf_t *)parms->leaf;
					leaf_index = leaf - world_leaves;
					leaf_lights[leaf_index].lights[int_index] &= ~(1 << bit_index);
					
					for(j = 0; j < world_leaves_count; j++)
					{
						r = 1 << (j % 8);
						
						if(!leaf->pvs)
							break;
					
						if(leaf->pvs[j >> 3] & r)
						{
							//lleaf_index = 1 << (leaf->pvs[j >> 3] & r);			
							leaf_lights[j].lights[int_index] &= ~(1 << bit_index);
						}		
					}
				}	
			}
			
			
			leaf_index = light_leaf - world_leaves;
			leaf_lights[leaf_index].lights[int_index] |= 1 << bit_index;
			
			radius = LIGHT_RADIUS(parms->radius);
			energy = LIGHT_ENERGY(parms->energy);
			radius *= radius;

			
			light_pos4.x = pos->position.x;
			light_pos4.y = pos->position.y;
			light_pos4.z = pos->position.z;
			light_pos4.w = 0.0;
			
			if(!light_leaf->pvs)
				continue;
			
			#define BLODDY_SSE
			
			
			#ifdef BLODDY_SSE
			
			asm
			(
				"movups xmm0, [%[light_pos4]]\n"
				"movups xmm3, [%[sign]]\n"
				:: [light_pos4] "rm" (light_pos4),
				   [sign] "rm" (sign)
			);
			
			#endif
			
			
			for(j = 0; j < world_leaves_count; j++)
			{
				r = 1 << (j % 8);
				
				if(light_leaf->pvs[j >> 3] & r)
				{
					leaf = &world_leaves[j];
					
					//s = _rdtsc();
					
					#ifdef BLODDY_SSE
					asm
					(
						"movups xmm1, [%[leaf_pos4]]\n"
						"movups xmm2, [%[leaf_extents4]]\n"
						
						"movups xmm4, xmm2\n"					/* half extents... */
						"orps xmm4, xmm3\n"						/* negate half extents... */
						
						"movups xmm5, xmm0\n"					/* v = pos->position - leaf->center */
						"subps xmm5, xmm1\n"					/* ================================ */
						
						"minps xmm5, xmm2\n"					/* clamp v to the leaf's positive half-extents... */
						"maxps xmm5, xmm4\n"					/* clamp v to the leaf's negative half-extents...*/
						
						"addps xmm5, xmm1\n"					/* v += leaf->center */
						
						"subps xmm5, xmm0\n"					/* v -= pos->position */
						"movups [%[v4]], xmm5\n"				/* ================== */
						
						
						:[v4] "=m" (v4) : [leaf_pos4] "rm" (leaf->center),
						   				  [leaf_extents4] "rm" (leaf->extents)
			
						
					);
						
					#else
					v4.x = pos->position.x - leaf->center.x;
					v4.y = pos->position.y - leaf->center.y;
					v4.z = pos->position.z - leaf->center.z;
					
					if(v4.x > leaf->extents.x) v4.x = leaf->extents.x;
					else if(v4.x < -leaf->extents.x) v4.x = -leaf->extents.x;
					
					if(v4.y > leaf->extents.y) v4.y = leaf->extents.y;
					else if(v4.y < -leaf->extents.y) v4.y = -leaf->extents.y;
					
					if(v4.z > leaf->extents.x) v4.z = leaf->extents.z;
					else if(v4.z < -leaf->extents.z) v4.z = -leaf->extents.z;
					
					
					v4.x += leaf->center.x;
					v4.y += leaf->center.y;
					v4.z += leaf->center.z;
					
					v4.x = pos->position.x - v4.x;
					v4.y = pos->position.y - v4.y;
					v4.z = pos->position.z - v4.z;
					
					#endif
					
					//e = _rdtsc();
					
					
					//printf("%llu\n", e - s);
					
					dist = dot3(v4.vec3, v4.vec3);
					//dist = length3(v4.vec3);
					
					if(dot3(v4.vec3, v4.vec3) < radius)
					//if(energy * (1.0 / dist) * (1.0 - (dist / radius)) > 0.025)
						leaf_lights[j].lights[int_index] |= 1 << bit_index;
					else
						leaf_lights[j].lights[int_index] &= ~(1 << bit_index);
										
				}		
			}
			
			parms->leaf = (struct bsp_dleaf_t *)light_leaf;
			
			
			
			/*if(pos->position.x > parms->box_max.x || pos->position.x < parms->box_min.x)
				goto _do_box;		
			if(pos->position.y > parms->box_max.y || pos->position.y < parms->box_min.y)
				goto _do_box;
			if(pos->position.z > parms->box_max.z || pos->position.z < parms->box_min.z)
				goto _do_box;
			
			continue;*/	
			
			_do_box:
			light_VisibleTriangles(i);
			
			#undef BLODDY_SSE
			
			
		}	
	}
}



#if 1

void light_LightBounds()
{
	int i;
	
	int x;
	int y;
	int z;
	
	camera_t *active_camera = camera_GetActiveCamera();
	vec4_t light_origin;
	
	vec2_t ac;
	vec2_t lb;
	vec2_t rb;
	
	float nzfar = -active_camera->frustum.zfar;
	float nznear = -active_camera->frustum.znear;
	//float nznear = -4.0;
	float ntop = active_camera->frustum.top;
	float nright = active_camera->frustum.right;
	
	float x_max;
	float x_min;
	float y_max;
	float y_min;
	float d;
	float t;
	float denom;
	float si;
	float co;
	float k;
	float l;
	float light_radius;
	float light_z;
	int positive_z;
	int light_index;
	int offset;
	//visible_light_count = 0;
	
	light_position_t *position;
	light_params_t *params;
	//cluster_t *cluster;
	
	float qt = nznear / ntop;
	float qr = nznear / nright;
	
	short cluster_x_start;
	short cluster_y_start;
	short cluster_z_start;
	short cluster_x_end;
	short cluster_y_end;
	short cluster_z_end; 
	
	#define CLUSTER_NEAR 1.0
	
	denom = log(-nzfar / CLUSTER_NEAR);
	
	//glUseProgram(0);	
	
	/*glBindBuffer(GL_UNIFORM_BUFFER, cluster_uniform_buffer);
	cluster = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);*/
	
	/* TODO: SSE/SSE2/Assembly the shit ouf of this loop... */
	for(i = 0; i < visible_light_count; i++)
	{
		
		light_index = visible_lights[i];
		
		position = &light_positions[light_index];
		params = &light_params[light_index];
		
		light_origin.x = position->position.x;
		light_origin.y = position->position.y;
		light_origin.z = position->position.z;
		light_origin.w = 1.0;
		
		mat4_t_vec4_t_mult(&active_camera->world_to_camera_matrix, &light_origin);
		
		//light_radius = LIGHT_MAX_RADIUS * ((float)params->radius / 0xffff);
		light_radius = LIGHT_RADIUS(params->radius);
		
		d = light_origin.x * light_origin.x + light_origin.y * light_origin.y + light_origin.z * light_origin.z;
		
		if(light_radius * light_radius > d)
		{
			x_max = 1.0;
			x_min = -1.0;
			y_max = 1.0;
			y_min = -1.0;
			positive_z = 0;
			goto _skip_stuff;
		}
		
		
		/* 
			from   '2D Polyhedral Bounds of a Clipped,
			   		Perspective-Projected 3D Sphere
			   		by Michael Mara and Morgan McGuire'
		*/
		
		/*glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();	
		
		glUseProgram(0);	
		
		glColor3f(0.0, 1.0, 0.0);*/
		/*glPointSize(8.0);
		glBegin(GL_POINTS);
		
		glVertex3f((light_origin.x * qr) / light_origin.z, (light_origin.y * qt) / light_origin.z, -0.5);
		
		glEnd();
		glPointSize(1.0);*/
		
		x_max = -10.0;
		x_min = 10.0;
		
		y_max = -10.0;
		y_min = 10.0;
		
		positive_z = 0;
		
		ac.x = light_origin.x;
		ac.y = light_origin.z;
		d = ac.x * ac.x + ac.y * ac.y;
		l = light_radius * light_radius;
		k = nznear - ac.y;
		k = sqrt(light_radius * light_radius - k * k);
				
		if(d > l)
		{
			t = sqrt(d - light_radius * light_radius);
			d = sqrt(d);
					
					
			si = light_radius / d;
			co = t / d;	
					
			rb.x = ac.x * co - ac.y * si;
			rb.y = ac.x * si + ac.y * co;
			lb.x = ac.x * co + ac.y * si;
			lb.y = -ac.x * si + ac.y * co;
			
			if(rb.y > nznear)
			{				
				rb.x = ac.x + k;
				rb.y = nznear;
				positive_z++;
			}
					
			if(lb.y > nznear)
			{				
				lb.x = ac.x - k;
				lb.y = nznear;
				positive_z++;
			}
			
			x_min = (qr * lb.x) / lb.y;
			x_max = (qr * rb.x) / rb.y;
					
			if(x_min < -1.0) x_min = -1.0;
			else if(x_min > 1.0) x_min = 1.0;
			if(x_max > 1.0) x_max = 1.0;
			else if(x_max < -1.0) x_max = -1.0;	
		}
		else
		{
			x_min = -1.0;
			x_max = 1.0;
		}
				
				
		/*glPointSize(8.0);
		glBegin(GL_POINTS);
		
		glVertex3f(x_min, 0.0, -0.5);
		glVertex3f(x_max, 0.0, -0.5);
		
		glEnd();
		glPointSize(1.0);	*/	
				
			/*	draw_debug_DrawPoint(vec3(x_min, 0.0, -0.5), vec3(0.0, 1.0, 0.0), 12.0, 1);
				draw_debug_DrawPoint(vec3(x_max, 0.0, -0.5), vec3(0.0, 1.0, 0.0), 12.0, 1);*/
				
				
				
				
		ac.x = light_origin.y;
		ac.y = light_origin.z;
		d = ac.x * ac.x + ac.y * ac.y;
				//l = light_radius * light_radius;
				
		if(d > l)
		{
			t = sqrt(d - light_radius * light_radius);
			d = sqrt(d);
					//k = nznear - ac.y;
					//k = sqrt(light_radius * light_radius - k * k);
					
			si = light_radius / d;
			co = t / d;	
					
			rb.x = ac.x * co - ac.y * si;
			rb.y = ac.x * si + ac.y * co;
			lb.x = ac.x * co + ac.y * si;
			lb.y = -ac.x * si + ac.y * co;
					
					
			if(rb.y > nznear)
			{				
				rb.x = ac.x + k;
				rb.y = nznear;
				positive_z++;
			}
					
			if(lb.y > nznear)
			{				
				lb.x = ac.x - k;
				lb.y = nznear;
				positive_z++;
			}
					
			y_min = (qt * lb.x) / lb.y;
			y_max = (qt * rb.x) / rb.y;
					
			if(y_min < -1.0) y_min = -1.0;
			else if(y_min > 1.0) y_min = 1.0;
			if(y_max > 1.0) y_max = 1.0;
			else if(y_max < -1.0) y_max = -1.0;
		}
		else
		{
			y_min = -1.0;
			y_max = 1.0;
		}
		
		//printf("%f\n", (x_max - x_min) * (y_max - y_min));
		
		//printf("[%f %f]  [%f %f]\n", x_min, x_max, y_min, y_max);
		
		
		/*
		ivec3 pos;
		pos.x = int(floor(x_coord / (1366.0 / CLUSTER_X_DIVS)));
		pos.y = int(floor(y_coord / (768.0 / CLUSTER_Y_DIVS)));
		
		pos.z = int((log(-view_z / znear) / log(zfar / znear)) * CLUSTER_Z_DIVS);
			
		if(pos.z > CLUSTER_Z_DIVS) pos.z = CLUSTER_Z_DIVS;
		else if(pos.z < 0) pos.z = 0;
		
		return pos;
		*/
		
		_skip_stuff:
		
		
		if((x_max - x_min) * (y_max - y_min) == 0.0 || positive_z == 4)
		{
			_remove_light:
			
			if(i < visible_light_count - 1)
			{
				visible_lights[i] = visible_lights[visible_light_count - 1];
			}
			visible_light_count--;
			i--;
			continue;
		}
		else
		{
			/*visible_light_positions[visible_light_count].position = light_origin.vec3;
			visible_light_params[visible_light_count] = light_params[i];*/
	 		 
			cluster_x_start = (int)((x_min * 0.5 + 0.5) * CLUSTERS_PER_ROW);
			cluster_y_start = (int)((y_min * 0.5 + 0.5) * CLUSTER_ROWS);
			light_z = light_origin.z + light_radius;
			
			if(light_z > -CLUSTER_NEAR) cluster_z_start = 0;
			else cluster_z_start = (int)((log(-light_z / CLUSTER_NEAR) / denom) * (float)(CLUSTER_LAYERS));
			if(cluster_z_start > CLUSTER_LAYERS)
				cluster_z_start = CLUSTER_LAYERS - 1;
				//goto _remove_light;					/* this light is too far away, so get rid of it... */
				
			params->first_cluster = PACK_CLUSTER_INDEXES(cluster_x_start, cluster_y_start, cluster_z_start);	
			
			
			  
			cluster_x_end = (int)((x_max * 0.5 + 0.5) * CLUSTERS_PER_ROW);
			cluster_y_end = (int)((y_max * 0.5 + 0.5) * CLUSTER_ROWS);
			light_z = light_origin.z - light_radius;
			
			if(light_z > -CLUSTER_NEAR) cluster_z_end = 0;
			else cluster_z_end = (int)((log(-light_z / CLUSTER_NEAR) / denom) * (float)(CLUSTER_LAYERS));
			if(cluster_z_end > CLUSTER_LAYERS)
				cluster_z_end = CLUSTER_LAYERS - 1;
			
			params->last_cluster = PACK_CLUSTER_INDEXES(cluster_x_end, cluster_y_end, cluster_z_end);
			
			//UNPACK_CLUSTER_INDEXES(cluster_x_end, cluster_y_end, cluster_z_end, params->last_cluster);
			
			
			
			/*for(z = cluster_z_start; z <= cluster_z_end; z++)
			{
				for(y = cluster_y_start; y <= cluster_y_end; y++)
				{
					for(x = cluster_x_start; x <= cluster_x_end; x++)
					{
						if(cluster[CLUSTER_OFFSET(x, y, z)].time_stamp != r_frame)
						{
							cluster[CLUSTER_OFFSET(x, y, z)].light_indexes_bm = 0;
							cluster[CLUSTER_OFFSET(x, y, z)].time_stamp = r_frame;
						}
							
						cluster[CLUSTER_OFFSET(x, y, z)].light_indexes_bm = 1;
					}
				}
			}*/
			
			
			
		/*	#if 0
			
			cluster_x = (int)(floor((window_width * (x_min * 0.5 + 0.5)) / CLUSTER_SIZE));
			cluster_y = (int)(floor((window_height * (1.0 - (y_max * 0.5 + 0.5))) / CLUSTER_SIZE));
			
			light_z = light_origin.z + light_radius;
			
			
			
			if(light_z > nznear) cluster_z = 0;
			else cluster_z = (int)((log(-light_z / -nznear)/log(-nzfar / -nznear)) * CLUSTER_Z_DIVS);
			
			visible_light_params[visible_light_count].first_cluster_id = PACK_CLUSTER_ID(cluster_x, cluster_y, cluster_z);
			
			cluster_x = (int)(floor((window_width * (x_max * 0.5 + 0.5)) / CLUSTER_SIZE));
			cluster_y = (int)(floor((window_height * (1.0 - (y_min * 0.5 + 0.5))) / CLUSTER_SIZE));
			
			light_z = light_origin.z - light_radius;
			
			if(light_z > nznear) cluster_z = 0;
			else cluster_z = (int)((log(-light_z / -nznear)/log(-nzfar / -nznear)) * CLUSTER_Z_DIVS);
			
			visible_light_params[visible_light_count].first_cluster_id = PACK_CLUSTER_ID(cluster_x, cluster_y, cluster_z);
			
			#endif*/
			
			//visible_light_count++;
		}
				
		
		
		/*glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();		
		
		glColor3f(0.0, 1.0, 0.0);*/
		
		/*glPointSize(8.0);
		glBegin(GL_POINTS);
		
		glVertex3f(x_min, y_max, -0.5);
		glVertex3f(x_min, y_min, -0.5);
		glVertex3f(x_max, y_min, -0.5);
		glVertex3f(x_max, y_max, -0.5);
		
		glEnd();
		glPointSize(1.0);*/
		
		/*glBegin(GL_LINES);
		glVertex3f(x_min, y_max, -0.5);
		glVertex3f(x_min, y_min, -0.5);
		
		glVertex3f(x_min, y_min, -0.5);
		glVertex3f(x_max, y_min, -0.5);
		
		glVertex3f(x_max, y_min, -0.5);
		glVertex3f(x_max, y_max, -0.5);
		
		glVertex3f(x_max, y_max, -0.5);
		glVertex3f(x_min, y_max, -0.5);
		glEnd();
		
		
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();*/
		
		
	}
	
	/*glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);*/
}
#endif


void light_UpdateClusters()
{
	int i;
	int c = visible_light_count;
	int x;
	int y;
	int z;
	
	int cluster_x_start;
	int cluster_y_start;
	int cluster_z_start;
	
	int cluster_x_end;
	int cluster_y_end;
	int cluster_z_end;
	
	int light_index;
	int cluster_index;
	int offset;
	
	//cluster_t *cluster;
	light_params_t *parms;
	
	//glBindBuffer(GL_UNIFORM_BUFFER, cluster_uniform_buffer);
	//cluster = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	
	//SDL_LockMutex(cluster_thread0_out_lock);
	//SDL_LockMutex(cluster_thread1_out_lock);
	
	//SDL_UnlockMutex(cluster_thread0_in_lock);
	//SDL_UnlockMutex(cluster_thread1_in_lock);
	
	for(z = 0; z < CLUSTER_LAYERS; z++)
	{
		for(y = 0; y < CLUSTER_ROWS; y++)
		{
			for(x = 0; x < CLUSTERS_PER_ROW; x++)
			{
				cluster_index = CLUSTER_OFFSET(x, y, z);							
				clusters[cluster_index].light_indexes_bm = 0;
			}
		}
	}
	
	
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadow_map_frame_buffer);
	//glFramebufferTexture3D()
	
	
	//if(c == 4)
	/*if(input_GetKeyStatus(SDL_SCANCODE_K) & KEY_JUST_PRESSED)
	{
		printf("breakpoint!\n");
		
		printf("breakpoint!\n");
	}*/
	
	for(i = 0; i < c; i++)
	{
		light_index = visible_lights[i];
		
		parms = &light_params[light_index];
		offset = light_cache[parms->cache].offset;
		
		UNPACK_CLUSTER_INDEXES(cluster_x_start, cluster_y_start, cluster_z_start, parms->first_cluster);
		UNPACK_CLUSTER_INDEXES(cluster_x_end, cluster_y_end, cluster_z_end, parms->last_cluster); 
		
		for(z = cluster_z_start; z <= cluster_z_end && z < CLUSTER_LAYERS; z++)
		{
			for(y = cluster_y_start; y <= cluster_y_end && y < CLUSTER_ROWS; y++)
			{
				for(x = cluster_x_start; x <= cluster_x_end && x < CLUSTERS_PER_ROW; x++)
				{
					
					cluster_index = CLUSTER_OFFSET(x, y, z);	
					
				/*	if(clusters[cluster_index].time_stamp != r_frame)
					{
						clusters[cluster_index].time_stamp = r_frame;
						clusters[cluster_index].light_indexes_bm = 0;
					}*/
											
					clusters[cluster_index].light_indexes_bm |= 1 << offset;
				}
			}
		}
				
	}
	
	/*if(SDL_LockMutex(cluster_thread0_out_lock))
	{
		SDL_UnlockMutex(cluster_thread0_out_lock);
	}*/
	
	/*if(SDL_LockMutex(cluster_thread1_out_lock))
	{
		SDL_UnlockMutex(cluster_thread1_out_lock);
	}*/
	
	/*SDL_LockMutex(cluster_thread0_in_lock);*/
	/*SDL_LockMutex(cluster_thread1_in_lock);*/
	
	//while(glGetError() != GL_NO_ERROR);
	glBindTexture(GL_TEXTURE_3D, cluster_texture);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, CLUSTERS_PER_ROW, CLUSTER_ROWS, CLUSTER_LAYERS, GL_RED_INTEGER, GL_UNSIGNED_INT, clusters);
	glBindTexture(GL_TEXTURE_3D, 0);
	//printf("%x\n", glGetError());
	
	//glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(cluster_t) * CLUSTERS_PER_ROW * CLUSTER_ROWS * CLUSTER_LAYERS, clusters);
	
	//glUnmapBuffer(GL_UNIFORM_BUFFER);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
}



void light_VisibleLights()
{
	//printf("light_VisibleLights\n");
	int i;
	int c = visible_leaves_count;
	
	int j;
	int k;
	int leaf_index;
	bsp_dleaf_t *leaf;
	bsp_dleaf_t *cur_leaf;
	camera_t *active_camera = camera_GetActiveCamera();
	vec4_t light_origin;
	vec3_t v;
	light_position_t *pos;
	light_params_t *parms;
	bsp_lights_t lights;
	
	visible_light_count = 0;
	
	if(!world_leaves)
	{
		for(i = 0; i < light_count && i < MAX_VISIBLE_LIGHTS; i++)
		{
			if(!(light_params[i].bm_flags & LIGHT_INVALID))
			{
				visible_lights[visible_light_count++] = i;
			}
		}
		
		goto _clusterize;
	}
	
	float d;
	float radius;
	light_EvictOld();
	
	
	for(i = 0; i < MAX_WORLD_LIGHTS >> 5; i++)
	{
		lights.lights[i] = 0;
	}
	
	for(i = 0; i < c; i++)
	{
		leaf = visible_leaves[i];
		
		leaf_index = leaf - world_leaves;
		
		for(j = 0; j < MAX_WORLD_LIGHTS >> 5; j++)
		{
			lights.lights[j] |= leaf_lights[leaf_index].lights[j];
		}
	}
	
	for(i = 0; i < MAX_WORLD_LIGHTS; i++)
	{
		if(lights.lights[i >> 5] & (1 << (i % 32)))
		{
			visible_lights[visible_light_count++] = i;
		}
	}
	
	_clusterize:
	
	light_LightBounds();
	
	//light_AllocShadowMaps();
	
	for(i = 0; i < visible_light_count; i++)
	{
		light_CacheLight(visible_lights[i]);
	}
	
	light_UpdateClusters();
	
	light_UploadCache();
}

void light_VisibleTriangles(int light_index)
{
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
	
	if(light_index >= 0 && light_index <= light_count)
	{
		if(!(light_params[light_index].bm_flags & LIGHT_INVALID))
		{
			parms = &light_params[light_index];
			pos = &light_positions[light_index];
			leaf = (bsp_dleaf_t *)parms->leaf;
		
			if(leaf)
			{	
				radius = LIGHT_RADIUS(parms->radius);
				sqrd_radius = radius * radius;
				
				//printf("update visible triangles\n");
				
				parms->visible_triangle_count = 0;
				parms->bm_flags |= LIGHT_NEEDS_REUPLOAD;
				slot_index = light_index * MAX_TRIANGLES_PER_LIGHT;
				visible_triangles = &light_visible_triangles[slot_index];
				
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
	
	
	
}

void light_ClearLightLeaves()
{
	int i;
	bsp_dleaf_t *leaf;
	int leaf_index;
	for(i = 0; i < light_count; i++)
	{		
		light_params[i].leaf = NULL;
		light_params[i].bm_flags |= LIGHT_MOVED | LIGHT_UPDATE_SHADOW_MAP;
	}
}


void light_TranslateLight(int light_index, vec3_t direction, float amount)
{
	light_position_t *light_position;
	
	
	if(light_index >= 0 && light_index < light_count)
	{
		light_position = &light_positions[light_index];
		light_position->position.x += direction.x * amount;
		light_position->position.y += direction.y * amount;
		light_position->position.z += direction.z * amount;
		
		light_params[light_index].bm_flags |= LIGHT_MOVED | LIGHT_UPDATE_SHADOW_MAP;
	}
	 
}

int cur_light_index = -1;

void light_SetLight(int light_index)
{
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
}



void light_AllocShadowMap(int light_index)
{
	light_params_t *parms;
	
	if(allocd_shadow_map_count >= max_shadow_maps)
		return;
			
	if(light_index >= 0 && light_index < light_count)
	{
		parms = &light_params[light_index];
		
		if(!(parms->bm_flags & LIGHT_INVALID))
		{
			/* only lights that can generate shadows may have
			shadow maps allocated to them... */
			if(parms->bm_flags & LIGHT_GENERATE_SHADOWS)
			{
				
				if(parms->shadow_map > -1)
				{
				//	printf("light already has a shadow map!\n");
					return;
				}
					
				
				parms->shadow_map = allocd_shadow_map_count;
				
				parms->bm_flags |= LIGHT_UPDATE_SHADOW_MAP;
				
				shadow_maps[allocd_shadow_map_count].light_index = light_index;
				
				parms->x = shadow_maps[allocd_shadow_map_count].x;
				parms->y = shadow_maps[allocd_shadow_map_count].y;
				
				allocd_shadow_map_count++;
				
			//	printf("shadow map %d allocd!\n", parms->shadow_map); 
			}
			
		}
		
	}
}


void light_FreeShadowMap(int light_index)
{
	light_params_t *parms;
	int shadow_map_index;			
	shadow_map_t shadow_map;
	if(light_index >= 0 && light_index < light_count)
	{
		parms = &light_params[light_index];
		
		if(!(parms->bm_flags & LIGHT_INVALID))
		{
			
			/* don't check if the light cast shadows here, as
			we can make a light stop generating shadows while it's 
			visible. Doing so allow the engine to free it's shadow
			map once the light gets dropped from the cache. Of course,
			it means we trust the shadow map index is valid. */
			
			shadow_map_index = parms->shadow_map;
					
			//if(shadow_maps[shadow_map_index].light_index > -1)
			{
				parms->shadow_map = -1;
				shadow_maps[shadow_map_index].light_index = -1;
				
				if(shadow_map_index < allocd_shadow_map_count - 1)
				{
					parms = &light_params[shadow_maps[allocd_shadow_map_count - 1].light_index];
					
					parms->shadow_map = shadow_map_index;
					
					shadow_map = shadow_maps[shadow_map_index];
					shadow_maps[shadow_map_index] = shadow_maps[allocd_shadow_map_count - 1];
					shadow_maps[allocd_shadow_map_count - 1] = shadow_map;
					
					
					
				}
				
				allocd_shadow_map_count--;
								
			//	printf("shadow map %d freed!\n", shadow_map_index);
				
			}
			
			
		}
		
	}
}


#if 0
void light_AllocShadowMaps()
{
	int i;
	int c;
	int j;
	int k;
	int m;

	short ah;
	short bw;
	
	short a0;
	short a1;
	short b0;
	short b1;
	
	short w;
	short h;
	short temp0;
	short temp1;
	short old_w;
	short old_h;
	short old_x;
	short old_y;
	float r[2];
	short aw[2];
	short bh[2];
	int prev;
	int next;
	int light_index;
	light_position_t *pos;
	light_params_t *parms;
	//light_data2 *light;
	//light_data0 *light1;
	//int last = free_chunk_count;
	
	//if(w < 1 || h < 1) return -1;
	

	k = visible_light_count;
	w = 0;
	h = 0;	
	
	free_chunk_count = 1;
	free_chunks[0].x = 0;
	free_chunks[0].y = 0;
	free_chunks[0].w = SHARED_SHADOW_MAP_WIDTH;
	free_chunks[0].h = SHARED_SHADOW_MAP_HEIGHT;
	
	alloc_chunk_count = 0;
	
	
	for(m = 0; m < k; m++)
	{
		//light1 = &active_light_a.position_data[m];
		//light = &active_light_a.shadow_data[m];
		
		//light_index = light_cache[m].light_index;
		light_index = visible_lights[m];
		
		parms = &light_params[light_index];
		
		if(!(parms->bm_flags & LIGHT_GENERATE_SHADOWS)) 
			continue;
		
		parms->bm_flags &= ~LIGHT_DROPPED_SHADOW;
		//w = light->cur_res;
		//h = light->cur_res;
		
		w = SHADOW_MAP_RESOLUTION * 3;
		h = SHADOW_MAP_RESOLUTION * 2;
		
		/*if(light1->bm_flags & LIGHT_POINT)
		{
			w *= 3;
			h *= 2;
		}*/
		c = free_chunk_count;
		
		for(i = 0; i < c; i++)
		{	
			if(w <= free_chunks[i].w)
			{
				if(h <= free_chunks[i].h)
				{
					alloc_chunks[alloc_chunk_count].x = free_chunks[i].x;
					alloc_chunks[alloc_chunk_count].y = free_chunks[i].y;
					alloc_chunks[alloc_chunk_count].w = w;
					alloc_chunks[alloc_chunk_count].h = h;
					
					parms->x = alloc_chunks[alloc_chunk_count].x;
					parms->y = alloc_chunks[alloc_chunk_count].y;
					parms->w = alloc_chunks[alloc_chunk_count].w;
					parms->h = alloc_chunks[alloc_chunk_count].h;

										
					alloc_chunk_count++;				
					
					/* the allocation fits exactly within this chunk. */
					if(w == free_chunks[i].w && h == free_chunks[i].h)
					{
	
						if(free_chunk_count > 1)
						{
							free_chunks[i] = free_chunks[free_chunk_count - 1];
						}
						free_chunk_count--;
						//return;
						break;
						
					}
					
					/* this allocation have one of its dimensions equal 
					to the free chunk, so just shrink it */
					else if(w == free_chunks[i].w)
					{
						free_chunks[i].y += h;
						free_chunks[i].h -= h;
						break;
						//continue;
						//return;
					}
					else if(h == free_chunks[i].h)
					{
						free_chunks[i].x += w;
						free_chunks[i].w -= w;
						break;
						//return;
					}
					
					old_w = free_chunks[i].w;
					old_h = free_chunks[i].h;
					old_x = free_chunks[i].x;
					old_y = free_chunks[i].y;
					
					/* The requested block has both its dimensions smaller than
					the free block. This means that a new block will be added,
					and the old one will be modified. */
					
					/* the two combinations for each chunk after the cut */
					aw[0] = w;
					aw[1] = old_w;
					ah = old_h - h;
					
					bh[0] = old_h;
					bh[1] = h;
					bw = old_w - w;
					
					/* here, a0 ,a1, b0 and b1
					are swapped to make sure that
					a1 > a0 and b0 > b1, so
					the relation a1b0 > a0b1
					holds. */
					for(j = 0; j < 2; j++)
					{
						a0 = ah;
						a1 = aw[j];
						/* this swap could be made faster if the
						rotate instruction was used, since those are 
						16 bits figures...  */
						if(a1 > a0)
						{
							temp0 = a1;
							a1 = a0;
							a0 = temp0;
						}
						
						b0 = bw;
						b1 = bh[j];
						if(b0 > b1)
						{
							temp0 = b0;
							b0 = b1;
							b1 = temp0;
						}
						/* calculate the ratio for both configurations... */
						r[j] = (float)(a0*b1)/(float)(a1*b0);
					}
					
					free_chunks[i].h = ah;
					free_chunks[i].y = old_y + h;
					free_chunks[i].x = old_x;
					free_chunks[free_chunk_count].w = bw;
					free_chunks[free_chunk_count].y = old_y;
					free_chunks[free_chunk_count].x = old_x + w;
					
					/* and use the one that results in the 
					smallest ratio between the ratio of the 
					chunks' sides. */
					if(r[0] < r[1])
					{
						free_chunks[i].w = aw[0];
						free_chunks[free_chunk_count].h = bh[0];
					}
					else
					{
						free_chunks[i].w = aw[1];
						free_chunks[free_chunk_count].h = bh[1];
					}
					free_chunk_count++;
					break;
					//return;
					
				}
				else
				{
					parms->bm_flags |= LIGHT_DROPPED_SHADOW;
				}
			}
			else
			{
				parms->bm_flags |= LIGHT_DROPPED_SHADOW;
			}
		}
	}
		
}

#endif






int light_ClusterThread0(void *data)
{
	int i;
	int c = visible_light_count;
	int x;
	int y;
	int z;
	
	int cluster_x_start;
	int cluster_y_start;
	int cluster_z_start;
	
	int cluster_x_end;
	int cluster_y_end;
	int cluster_z_end;
	
	int light_index;
	int cluster_index;
	int offset;
	
	//cluster_t *cluster;
	light_params_t *parms;
	
	while(1)
	{
		SDL_LockMutex(cluster_thread0_in_lock);
		for(i = 0; i < c; i++)
		{
			light_index = visible_lights[i];
			
			parms = &light_params[light_index];
			offset = light_cache[parms->cache].offset;
			
			UNPACK_CLUSTER_INDEXES(cluster_x_start, cluster_y_start, cluster_z_start, parms->first_cluster);
			UNPACK_CLUSTER_INDEXES(cluster_x_end, cluster_y_end, cluster_z_end, parms->last_cluster); 
			
			for(z = cluster_z_start; z <= cluster_z_end; z++)
			{
				for(y = cluster_y_start; y <= cluster_y_end; y++)
				{
					
					for(x = cluster_x_start; x <= cluster_x_end; x++)
					{
						cluster_index = CLUSTER_OFFSET(x, y, z);
						/*if(clusters[cluster_index].time_stamp != r_frame)
						{
							clusters[cluster_index].light_indexes_bm = 0;
							clusters[cluster_index].time_stamp = r_frame;
						}*/
								
						clusters[cluster_index].light_indexes_bm |= 1 << offset;
					}
				}
			}
		}
		/*printf("job0\n");*/
		SDL_Delay(16);
		SDL_UnlockMutex(cluster_thread0_out_lock);
	}
}

int light_ClusterThread1(void *data)
{
	while(1)
	{
		
		SDL_LockMutex(cluster_thread1_in_lock);
		/*printf("job1\n");
		SDL_Delay(16);*/
		SDL_UnlockMutex(cluster_thread1_out_lock);
	}
	
}

/*void light_EnableShadowMapWrite(int light_index, int face_index)
{
	if(face_index >= 0 && face_index <= 5)
	{
		
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, )
}*/















