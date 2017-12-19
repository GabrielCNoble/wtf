#include <stdlib.h>
#include <stdio.h>

#include "GL\glew.h"

#include "camera.h"
#include "shader.h"

#include "l_main.h"
#include "l_cache.h"

/* from l_main.c */
extern int light_count;
extern light_position_t *light_positions;
extern light_params_t *light_params;


/* from r_main.c */
extern int r_frame;


int light_cache_cursor = 0;
int light_cache_stack_top = -1;
int light_cache_free_stack[LIGHT_CACHE_SIZE];
unsigned int light_cache_uniform_buffer;
unsigned int light_cache_element_buffer;
int *light_cache_index_buffer_base = NULL;
int *light_cache_index_buffers[LIGHT_CACHE_SIZE];
light_cache_slot_t light_cache[LIGHT_CACHE_SIZE];


/* 300 frames (~5 seconds) before
a cached light gets dropped... */
#define OLD_THRESHOLD 300

/* 120 frames (~2 seconds) between
checks for old cached lights... */
#define MASS_EVICTION_THRESHOLD 120
static int l_last_mass_eviction = 0;


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
			/*if(light_cache_stack_top > -1)
			{
				cache = light_cache_free_stack[light_cache_stack_top--];
			}
			else
			{
				cache = light_cache_cursor++;
			}*/
			
			cache = light_cache_cursor++;
			
			_setup_data:
			
			parms->cache = cache;
			light_cache[cache].last_touched = r_frame;
			light_cache[cache].light_index = light_index;
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
	
	if(light_index >= 0 && light_index < light_cache_cursor)
	{
		cache = light_params[light_index].cache;
		light_params[light_index].cache = -1;
	
		//l_cache[cache].light_index = 0xffff;
		
		if(cache < light_cache_cursor - 1)
		{
			t = light_cache[cache];
			light_cache[cache] = light_cache[light_cache_cursor - 1];
			light_cache[light_cache_cursor - 1] = t;
			
			i = light_cache[cache].light_index;
			light_params[i].cache = cache;
		}
		
		light_cache_cursor--;
		
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
	
	printf("light_EvictOld: before: %d  ", light_cache_cursor);

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
			
			//l_cache[i].light_index = 0xffff;
			
			if(i < light_cache_cursor - 1)
			{
				t = light_cache[i];
				light_cache[i] = light_cache[light_cache_cursor - 1];
				light_cache[light_cache_cursor - 1] = t;
				
				c = light_cache[i].light_index;
				
				light_params[c].cache = i;
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
	
	printf("after: %d\n", light_cache_cursor);
	
	l_last_mass_eviction = 0;
	
}


void light_Update()
{
	int i;
	int c;
	int uniform_index = 0;
	
	vec4_t light_position;
	light_position_t *pos;
	light_params_t *parms;
	camera_t *active_camera = camera_GetActiveCamera();
	gpu_lamp_t *lamp;
	
	
	glBindBuffer(GL_UNIFORM_BUFFER, light_cache_uniform_buffer);
	lamp = (gpu_lamp_t *) glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	
	for(i = 0; i < light_cache_cursor; i++)
	{
		
		c = light_cache[i].light_index;
		pos = &light_positions[c];
		parms = &light_params[c];
		
		
		if(light_cache[i].last_touched != r_frame) 
			continue;
		
		
		light_position.x = pos->position.x;
		light_position.y = pos->position.y;
		light_position.z = pos->position.z;
		light_position.w = 1.0;
				
		mat4_t_vec4_t_mult(&active_camera->world_to_camera_matrix, &light_position);
		
		light_cache[i].gpu_index = uniform_index;
			
		lamp[uniform_index].position.x = light_position.x;
		lamp[uniform_index].position.y = light_position.y;
		lamp[uniform_index].position.z = light_position.z;
		lamp[uniform_index].position.w = 1.0;
			
		lamp[uniform_index].color.r = (float)parms->r / 255.0;
		lamp[uniform_index].color.g = (float)parms->g / 255.0;
		lamp[uniform_index].color.b = (float)parms->b / 255.0;
			
		lamp[uniform_index].radius = LIGHT_MAX_RADIUS * ((float)parms->radius / 0xffff);
		lamp[uniform_index].energy = LIGHT_MAX_ENERGY * ((float)parms->energy / 0xffff);
		
		
		uniform_index++;
		
	}
	
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
}

void light_BindCache()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, LIGHT_PARAMS_UNIFORM_BUFFER_BINDING, light_cache_uniform_buffer);
}

void light_UnbindCache()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, LIGHT_PARAMS_UNIFORM_BUFFER_BINDING, 0);
}
