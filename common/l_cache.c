#include <stdlib.h>
#include <stdio.h>

#include "GL\glew.h"

#include "camera.h"
#include "shader.h"

#include "l_main.h"
#include "l_cache.h"

#include "engine.h"

/* from l_main.c */
extern int light_count;
extern light_position_t *light_positions;
extern light_params_t *light_params;
extern bsp_striangle_t *light_visible_triangles;
extern unsigned int shadow_map_frame_buffer;
extern unsigned int cluster_uniform_buffer;


/* from r_main.c */
extern int r_frame;


/* from world.c */
extern int world_triangle_group_count;
extern int world_start;
extern vertex_t *world_vertices;
extern bsp_dleaf_t *world_leaves;



static int light_cache_cursor = 0;
static int light_cache_triangle_group_count;
//int light_cache_stack_top = -1;
//int light_cache_free_stack[LIGHT_CACHE_SIZE];
unsigned int light_cache_uniform_buffer = 0;
unsigned int light_cache_element_buffer = 0;
unsigned int light_cache_shadow_element_buffer = 0;
unsigned int light_cache_shadow_maps[LIGHT_CACHE_SIZE];
int *light_cache_index_buffer_base = NULL;
int *light_cache_groups_next;
int *light_cache_frustum_counts;
int *light_cache_index_buffers[LIGHT_CACHE_SIZE];
light_cache_slot_t light_cache[LIGHT_CACHE_SIZE];

gpu_lamp_t *lamp_buffer;

unsigned int test_uniform_buffer;


/* 360 frames (~6 seconds) before
a cached light gets dropped... */
#define OLD_THRESHOLD 360

/* 120 frames (~2 seconds) between
checks for old cached lights... */
#define MASS_EVICTION_THRESHOLD 120
static int l_last_mass_eviction = 0;


void light_InitCache()
{
	int i;
	int j;
	int *p;
	
	
	//while(glGetError() != GL_NO_ERROR);
	glGenBuffers(1, &light_cache_uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, light_cache_uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(gpu_lamp_t) * LIGHT_CACHE_SIZE, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	//printf("%d\n", glGetError());
	
	lamp_buffer = malloc(sizeof(gpu_lamp_t) * LIGHT_CACHE_SIZE);
	
	glGenBuffers(1, &test_uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, test_uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(test_t) * 8, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
		
	glGenBuffers(1, &light_cache_shadow_element_buffer);	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_cache_shadow_element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * MAX_INDEXES_PER_FRUSTUM * 6 * LIGHT_CACHE_SIZE, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	light_cache_frustum_counts = malloc(sizeof(int) * 6 * LIGHT_CACHE_SIZE);
	
	#if 0
	
	
	
	#endif
		
	/*for(i = 0; i < LIGHT_CACHE_SIZE; i++)
	{
		glGenTextures(1, &light_cache_shadow_maps[i]);
		glBindTexture(GL_TEXTURE_CUBE_MAP, light_cache_shadow_maps[i]);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
			
		for(j = 0; j < 6; j++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_LUMINANCE16F_ARB, 512, 512, 0, GL_LUMINANCE, GL_FLOAT, NULL);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, light_cache_shadow_maps[i], 0);
		}
			
	}*/
		
	for(i = 0; i < LIGHT_CACHE_SIZE; i++)
	{
		light_cache[i].offset = i;
	}
	
	
}

void light_FinishCache()
{
	free(light_cache_frustum_counts);
	free(lamp_buffer);
	glDeleteBuffers(1, &light_cache_uniform_buffer);
	glDeleteBuffers(1, &light_cache_shadow_element_buffer);
}


void light_CacheLight(int light_index)
{
	light_params_t *parms;
	int cache = -1;
	int i;
	int last_touched = 0;
	int highest_last_touched = 0;
	
	if(light_index >= 0 && light_index < light_count)
	{
		parms = &light_params[light_index];
		
		if(parms->cache > -1)
		{
			light_cache[parms->cache].last_touched = r_frame;
			return;
		} 
			
		if(light_cache_cursor < LIGHT_CACHE_SIZE)
		{
			
			cache = light_cache_cursor++;
			
			_setup_data:
			
			parms->cache = cache;
			light_cache[cache].last_touched = r_frame;
			light_cache[cache].light_index = light_index;
			parms->bm_flags |= LIGHT_NEEDS_REUPLOAD;
			
			light_AllocShadowMap(light_index);		
			//light_UploadIndexes(light_index);
		}
		else
		{
			/* evict the last recently touched slot...  */
			for(i = 0; i < LIGHT_CACHE_SIZE; i++)
			{
				last_touched = r_frame - light_cache[i].last_touched;
				if(last_touched > highest_last_touched)
				{
					highest_last_touched = last_touched;
					cache = i;
				}
			}
			
			/* the whole cache was filled in
			this frame, so there's nothing to
			evict... */
			if(cache == -1)
				return;
			
		
			parms = &light_params[light_cache[cache].light_index];
			parms->cache = -1;
			
			goto _setup_data;
			
		}
		
	}
}


void light_DropLight(int light_index)
{
	int cache;
	int i;
	light_cache_slot_t t;
	
	if(light_index >= 0 && light_index < light_count)
	{
		if(light_params[light_index].bm_flags & LIGHT_INVALID)
			return;
		
		cache = light_params[light_index].cache;
		light_params[light_index].cache = -1;
	
		//l_cache[cache].light_index = 0xffff;
		
		if(cache > -1)
		{
			light_cache[cache].light_index = -1;
			
			if(cache < light_cache_cursor - 1)
			{	
				/* swap cache slots, as it keeps 
				offsets for shadow map elements buffers... */
				t = light_cache[cache];
				light_cache[cache] = light_cache[light_cache_cursor - 1];
				light_cache[light_cache_cursor - 1] = t;
				
				i = light_cache[cache].light_index;
				light_params[i].cache = cache;
			}
					
			light_FreeShadowMap(light_index);
			light_cache_cursor--;
		}
		
		
		
		/* try not to put indexes in the
		free stack whenever possible... */
		/*if(cache == light_cache_cursor - 1)
		{
			light_cache_cursor--;
		}
		else
		{
			light_cache_free_stack[++light_cache_stack_top] = cache;
		}*/
		
	}
}


void light_EvictOld()
{
	int i;
	int c;
	light_params_t *parms;
	light_cache_slot_t t;
	if(l_last_mass_eviction < MASS_EVICTION_THRESHOLD)
	{
		l_last_mass_eviction++;
		return;
	}
	
	//printf("light_EvictOld: before: %d  ", light_cache_cursor);

	for(i = 0; i < light_cache_cursor; i++)
	{
		/*if(l_cache[i].light_index == 0xffff)
			continue;*/
		
		/* could've called light_DropLight here, but this
		eviction process has to be as fast as possible, so
		the code got copied instead... */	
		if(r_frame - light_cache[i].last_touched > OLD_THRESHOLD)
		{
			parms = &light_params[light_cache[i].light_index];
			parms->cache = -1;
			
			light_FreeShadowMap(light_cache[i].light_index);
			
			//l_cache[i].light_index = 0xffff;
			
			light_cache[i].light_index = -1;
			
			if(i < light_cache_cursor - 1)
			{
				
				t = light_cache[i];
				light_cache[i] = light_cache[light_cache_cursor - 1];
				light_cache[light_cache_cursor - 1] = t;
				
				c = light_cache[i].light_index;
				
				light_params[c].cache = i;
				i--;
			}
			
			
			light_cache_cursor--;
			
			/*if(i == light_cache_cursor)
			{
				light_cache_cursor--;
			}
			else
			{
				light_cache_free_stack[++light_cache_stack_top] = i;	
			}*/
		}	
	}
	
	//printf("after: %d\n", light_cache_cursor);
	
	l_last_mass_eviction = 0;
	
}


void light_UploadCache()
{
	int i;
	int light_index;
	int uniform_index = 0;
	
	float s;
	float e;
	
	test_t *p;
	
	vec4_t light_position;
	light_position_t *pos;
	light_params_t *parms;
	camera_t *active_camera = camera_GetActiveCamera();
	gpu_lamp_t *lamp;
	
	if(!light_cache_uniform_buffer)
		return;
	
	//while(glGetError() != GL_NO_ERROR);	
	
	glBindBuffer(GL_UNIFORM_BUFFER, light_cache_uniform_buffer);
	
	//s = engine_GetDeltaTime();
	
	/* BOTTLENECK: this call is taking between 3 and 6 ms... */
	//lamp = (gpu_lamp_t *) glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	lamp = lamp_buffer;
	//e = engine_GetDeltaTime();
		
	
	/*lamp[0].color_energy.r = 1.0;
	lamp[0].color_energy.g = 0.0;
	lamp[0].color_energy.b = 0.0;
	lamp[0].color_energy.a = 0.0;
	
	
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	
	glBindBuffer(GL_UNIFORM_BUFFER, test_uniform_buffer);
	p = (test_t *)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	
	p[0].color.r = 1.0;
	p[0].color.g = 0.0;
	p[0].color.b = 1.0;
	p[0].color.a = 0.0;*/
	
	/*p[0].a0 = 0xffffffff;
	p[0].a1 = 0xffffffff;
	p[0].a2 = 0;
	p[0].a3 = 0xffffffff;*/
	
	//glUnmapBuffer(GL_UNIFORM_BUFFER);	
	//printf("%x\n", glGetError());
	
		
	
	
	
	
//	printf("%x\n", glGetError());
	
	for(i = 0; i < light_cache_cursor; i++)
	{
		
		light_index = light_cache[i].light_index;
		pos = &light_positions[light_index];
		parms = &light_params[light_index];
		
		
		if(light_cache[i].last_touched != r_frame) 
			continue;
		
		//printf("update\n");
			
		if(parms->bm_flags & LIGHT_NEEDS_REUPLOAD)
			light_UploadIndexes(light_index);
			
		
		pos->world_to_light_matrix.floats[3][0] = -pos->position.x;
		pos->world_to_light_matrix.floats[3][1] = -pos->position.y;
		pos->world_to_light_matrix.floats[3][2] = -pos->position.z;
		
		uniform_index = light_cache[i].offset;
			
		
		lamp[uniform_index].position_radius.x = pos->position.x;
		lamp[uniform_index].position_radius.y = pos->position.y;
		lamp[uniform_index].position_radius.z = pos->position.z;
		lamp[uniform_index].position_radius.w = LIGHT_RADIUS(parms->radius);
			
		lamp[uniform_index].color_energy.r = (float)parms->r / 255.0;
		lamp[uniform_index].color_energy.g = (float)parms->g / 255.0;
		lamp[uniform_index].color_energy.b = (float)parms->b / 255.0;
		lamp[uniform_index].color_energy.a = LIGHT_ENERGY(parms->energy);

		lamp[uniform_index].bm_flags = parms->bm_flags;
		lamp[uniform_index].x_y = (parms->y << 16) | parms->x;
		lamp[uniform_index].bm_flags = parms->bm_flags & LIGHT_GENERATE_SHADOWS;
		/*if(parms->bm_flags & LIGHT_GENERATE_SHADOWS)
			lamp[uniform_index].bm_flags = 1;
		else
			lamp[uniform_index].bm_flags = 0;*/
		
		
		
		//uniform_index++;
		
	}
	
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(gpu_lamp_t) * LIGHT_CACHE_SIZE, lamp_buffer);
	
	//glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	//e = engine_GetDeltaTime();
	
	//printf("%f\n", e - s);
	
}

void light_BindCache()
{
	//glBindBuffer(GL_UNIFORM_BUFFER, test_uniform_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, LIGHT_PARAMS_UNIFORM_BUFFER_BINDING, light_cache_uniform_buffer);
	//glBindBufferBase(GL_UNIFORM_BUFFER, TEST_UNIFORM_BUFFER_BINDING, test_uniform_buffer);
	
	//glBindBufferBase(GL_UNIFORM_BUFFER, LIGHT_CLUSTER_UNIFORM_BUFFER_BINDING, cluster_uniform_buffer);
}

void light_UnbindCache()
{
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	/*=========================================================================*/
	/*=========================================================================*/
	/*=========================================================================*/
	//glBindBufferBase(GL_UNIFORM_BUFFER, LIGHT_PARAMS_UNIFORM_BUFFER_BINDING, 0);
	/*=========================================================================*/
	/*=========================================================================*/
	/*=========================================================================*/
	
	
	
	//glBindBufferBase(GL_UNIFORM_BUFFER, TEST_UNIFORM_BUFFER_BINDING, 0);
	//glBindBufferBase(GL_UNIFORM_BUFFER, LIGHT_CLUSTER_UNIFORM_BUFFER_BINDING, 0);
}

#if 0
void light_UpdateCacheGroups()
{
	
	int i;
	int j;
	int *p;
	if(!world_triangle_group_count)
		return;
	
	if(!light_cache_element_buffer)
	{
		glGenBuffers(1, &light_cache_element_buffer);
		glGenBuffers(1, &light_cache_shadow_element_buffer);
		glGenBuffers(1, &light_cache_uniform_buffer);
		glBindBuffer(GL_UNIFORM_BUFFER, light_cache_uniform_buffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(gpu_lamp_t) * LIGHT_CACHE_SIZE, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_cache_shadow_element_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * MAX_INDEXES_PER_FRUSTUM * 6 * LIGHT_CACHE_SIZE, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		light_cache_frustum_counts = malloc(sizeof(int) * 6 * LIGHT_CACHE_SIZE);
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadow_map_frame_buffer);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glClearColor(1000.0, 1000.0, 1000.0, 1000.0);
		
		for(i = 0; i < LIGHT_CACHE_SIZE; i++)
		{
			glGenTextures(1, &light_cache_shadow_maps[i]);
			glBindTexture(GL_TEXTURE_CUBE_MAP, light_cache_shadow_maps[i]);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
			
			//while(glGetError());
			for(j = 0; j < 6; j++)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_LUMINANCE16F_ARB, 512, 512, 0, GL_LUMINANCE, GL_FLOAT, NULL);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, light_cache_shadow_maps[i], 0);
				glClear(GL_COLOR_BUFFER_BIT);
				glBindTexture(GL_TEXTURE_CUBE_MAP, light_cache_shadow_maps[i]);
			}
			
			//printf("%x\n", glGetError());
			
			
		}
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glDrawBuffer(GL_BACK);
		
	}
	else
	{
		//free(light_cache_index_buffer_base);
		free(light_cache_groups_next);
	}
	
	//light_cache_index_buffer_base = malloc(sizeof(int) * 3 * MAX_INDEXES_PER_GROUP * world_triangle_group_count * LIGHT_CACHE_SIZE);
	//light_cache_groups_next = malloc(sizeof(int) * world_triangle_group_count * LIGHT_CACHE_SIZE);
	//while(glGetError() != GL_NO_ERROR);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_cache_element_buffer);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * MAX_INDEXES_PER_GROUP * world_triangle_group_count * LIGHT_CACHE_SIZE, NULL, GL_DYNAMIC_DRAW);
	//printf("%x\n", glGetError());
	
	
	//p = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
	
	/*if(!p)
	{
		printf("WARNING: glMapBuffer returned NULL on light_cache_element_buffer!\n");
	}
	
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER); */
	
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	

	
	for(i = 0; i < LIGHT_CACHE_SIZE; i++)
	{
		light_cache[i].offset = i;
	}
	
}
#endif

void light_UploadIndexes(int light_index)
{
	light_params_t *parms;
	light_position_t *pos;
	bsp_striangle_t *visible_triangles;
	vertex_t *first_vertex;
	vec3_t fv;
	vec3_t v;
	int *indexes;
	int *group_next;
	int *frustum_count;
	int frustum_index;
	int i;
	int c;
	int j;
	int cache;
	int block_index;
	int start_index;
	int group_index;
	float radius;
	
	if(!world_leaves)
		return;
	
	
	if(light_index >= 0 && light_index < light_count)
	{
		parms = &light_params[light_index];
		pos = &light_positions[light_index];
		
		if(!(parms->bm_flags & LIGHT_INVALID))
		{
			cache = parms->cache;
			
			if(cache > -1)
			{
									
				group_next = light_cache_groups_next + (light_cache[cache].offset * world_triangle_group_count);
		
				visible_triangles = &light_visible_triangles[light_index * MAX_TRIANGLES_PER_LIGHT];
				c = parms->visible_triangle_count;
				
				//assert(c < MAX_)
			
				#if 0
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_cache_element_buffer);
				
				indexes = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
				
				for(i = 0; i < world_triangle_group_count; i++)
				{
					group_next[i] = 0;
				}
				
				/* which block of groups we want... */
				block_index = MAX_INDEXES_PER_GROUP * world_triangle_group_count * light_cache[cache].index_buffer;
				
				for(i = 0; i < c; i++)
				{
					group_index = visible_triangles[i].triangle_group;
										
					start_index = group_index * MAX_INDEXES_PER_GROUP + block_index;
					
				 	indexes[start_index + group_next[group_index]	 ] = world_start + visible_triangles[i].first_vertex;
				 	indexes[start_index + group_next[group_index] + 1] = world_start + visible_triangles[i].first_vertex + 1;
				 	indexes[start_index + group_next[group_index] + 2] = world_start + visible_triangles[i].first_vertex + 2;
	 	
				 	group_next[group_index] += 3;
				}

				
				glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
				
				#endif
				
				if(parms->bm_flags & LIGHT_GENERATE_SHADOWS)
				{
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_cache_shadow_element_buffer);
					/* this is likelly to cause a big slowdown... */
					indexes = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE);
					
					/* light_cache_frustum_counts is an array of ints, and each group of 
					six ints represent the counts of the six frustums of a light... */
					frustum_count = light_cache_frustum_counts + light_cache[cache].offset * 6;
					
					/* block_index is the offset from the beginning of light_cache_frustum_counts of 
					the group of six ints that represents the frustum of this light... */
					block_index = MAX_INDEXES_PER_FRUSTUM * 6 * light_cache[cache].offset;				
					
					
					
					radius = LIGHT_RADIUS(parms->radius);
					radius *= radius;
					
					for(i = 0; i < 6; i++)
					{
						frustum_count[i] = 0;
					}			
									
					for(i = 0; i < c; i++)
					{
						first_vertex = &world_vertices[visible_triangles[i].first_vertex];
						
						/* TODO: FIX THIS! */
						for(j = 0; j < 6; j++)
						{
							/* start_index is the offset from the beginning of the light_cache_frustum_counts
							that represent this frustum of this light... */
							start_index = block_index + j * MAX_INDEXES_PER_FRUSTUM;
							
							if(frustum_count[j] >= MAX_INDEXES_PER_FRUSTUM)
								continue;
							
							
							//assert(frustum_count[j] < MAX_INDEXES_PER_FRUSTUM);
								
							//if(indexes[start_index + frustum_count[frustum_index]] != visible_triangles[i].first_vertex)
							{
								indexes[start_index + frustum_count[j]	  ] = world_start + visible_triangles[i].first_vertex;
								indexes[start_index + frustum_count[j] + 1] = world_start + visible_triangles[i].first_vertex + 1;
								indexes[start_index + frustum_count[j] + 2] = world_start + visible_triangles[i].first_vertex + 2;
										
								frustum_count[j] += 3;
							} 
						}
						
						
						#if 0
						
						for(j = 0; j < 3; j++)
						{
							v.x = first_vertex->position.x - pos->position.x;
							v.y = first_vertex->position.y - pos->position.y;
							v.z = first_vertex->position.z - pos->position.z;
							
							/* this vertice is inside the light's volume... */
							if(dot3(v, v) < radius)
							{
								fv.x = fabs(v.x);
								fv.y = fabs(v.y);
								fv.z = fabs(v.z);
								
								if(fv.x > fv.y)
								{
									if(fv.x > fv.z)
									{
										if(v.x > 0.0) frustum_index = LIGHT_FRUSTUM_X_POS;
										else frustum_index = LIGHT_FRUSTUM_X_NEG;
									}
									else
									{
										if(v.z > 0.0) frustum_index = LIGHT_FRUSTUM_Z_POS;
										else frustum_index = LIGHT_FRUSTUM_Z_NEG;
									}
								}
								else
								{
									if(fv.y > fv.z)
									{
										if(v.y > 0.0) frustum_index = LIGHT_FRUSTUM_Y_POS;
										else frustum_index = LIGHT_FRUSTUM_Y_NEG;
									}
									else
									{
										if(v.z > 0.0) frustum_index = LIGHT_FRUSTUM_Z_POS;
										else frustum_index = LIGHT_FRUSTUM_Z_NEG;
									}
								}
								
								start_index = block_index + frustum_index * MAX_INDEXES_PER_FRUSTUM;
								
								if(indexes[start_index + frustum_count[frustum_index]] != visible_triangles[i].first_vertex)
								{
									indexes[start_index + frustum_count[frustum_index]	  ] = world_start + visible_triangles[i].first_vertex;
									indexes[start_index + frustum_count[frustum_index] + 1] = world_start + visible_triangles[i].first_vertex + 1;
									indexes[start_index + frustum_count[frustum_index] + 2] = world_start + visible_triangles[i].first_vertex + 2;
									
									frustum_count[frustum_index] += 3;
								} 
									
							}
							
						}
						
						#endif
						
					}
					
					glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
				}
				
				
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				
				
				
				
				parms->bm_flags &= ~LIGHT_NEEDS_REUPLOAD;
								
			}
		}
	}
}















