#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "GL/glew.h"

#include "bsp.h"

#include "l_main.h"
#include "l_cache.h"
#include "camera.h"
#include "vector.h"
#include "matrix.h"
#include "shader.h"


static int light_list_size;
int light_count;
static int free_position_stack_top;
static int *free_position_stack;
light_position_t *light_positions;
light_params_t *light_params;
char **light_names;

//static int visible_light_list_size;
//int visible_light_count;
//light_position_t *visible_light_positions;
//light_params_t *visible_light_params;



static unsigned int light_indexes_texture;
static unsigned int cluster_texture;

//static int cluster_x_divs;
//static int cluster_y_divs;
//static unsigned short *clusters = NULL;
static unsigned char *light_indexes = NULL;

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

/* from l_cache.c */
extern int light_cache_cursor;
extern int light_cache_stack_top;
extern int light_cache_free_stack[];
extern unsigned int light_cache_uniform_buffer;
extern int *light_cache_index_buffer_base;
extern int *light_cache_index_buffers[];
extern light_cache_slot_t light_cache[];






void light_Init()
{
	int i;
	int c;
	light_list_size = MAX_WORLD_LIGHTS;
	light_count = 0;
	free_position_stack_top = -1;
	
	light_positions = malloc(sizeof(light_position_t) * light_list_size);
	light_params = malloc(sizeof(light_params_t) * light_list_size);
	light_names = malloc(sizeof(char *) * light_list_size);
	free_position_stack = malloc(sizeof(int ) * light_list_size);
	
	
	//visible_light_list_size = MAX_LIGHTS;
	//visible_light_count = 0;
	
	//visible_light_positions = malloc(sizeof(light_position_t) * visible_light_list_size);
	//visible_light_params = malloc(sizeof(light_params_t) * visible_light_list_size);
	
	
	
	//glGenTextures(1, &light_indexes_texture);
	//glGenTextures(1, &cluster_texture);
	
	
	
	/* yeah, init the light cache here... */
	light_cache_index_buffer_base = malloc(sizeof(int) * LIGHT_INDEX_BUFFER_SIZE * LIGHT_CACHE_SIZE);
	c = 0;
	for(i = 0; i < LIGHT_CACHE_SIZE; i++)
	{
		glGenBuffers(1, &light_cache[i].index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_cache[i].index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * LIGHT_INDEX_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		light_cache[i].last_touched = 0;
		light_cache_index_buffers[i] = (int *)((int)light_cache_index_buffer_base + i * LIGHT_INDEX_BUFFER_SIZE);
	}
	
	glGenBuffers(1, &light_cache_uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, light_cache_uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(gpu_lamp_t) * LIGHT_CACHE_SIZE, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	

}

void light_Finish()
{
	int i;
	
	for(i = 0; i < light_count; i++)
	{
		free(light_names[i]);
	}
	
	free(light_names);
	free(light_positions);
	free(light_params);
	free(free_position_stack);
	
	//free(visible_light_positions);
	//free(visible_light_params);
	free(light_cache_index_buffer_base);
	//free(clusters);
	
	glDeleteBuffers(1, &light_cache_uniform_buffer);
	//glDeleteTextures(1, &light_indexes_texture);
}

int light_CreateLight(char *name, mat3_t *orientation, vec3_t position, vec3_t color, float radius, float energy)
{
	int light_index;
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
	
	light_param->radius = 0xffff * (radius / LIGHT_MAX_RADIUS);
	light_param->energy = 0xffff * (energy / LIGHT_MAX_ENERGY);
	light_param->r = 0xff * color.r;
	light_param->g = 0xff * color.g;
	light_param->b = 0xff * color.b;
	light_param->cache = -1;
	light_param->bm_flags = LIGHT_MOVED;
	light_param->leaf = NULL;

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
	bsp_dleaf_t *light_leaf;
	bsp_dleaf_t *leaf;
	bsp_dleaf_t *lleaf;
	bsp_lights_t *lights;
	light_params_t *parms;
	
	
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
			
			int_index = i >> 5;
			bit_index = i % 32;
			
			
			
			
			
			
			
			if(parms->leaf)
			{
				/* don't update anything if this light didn't
				leave the leaf... */
				if((bsp_dleaf_t *)parms->leaf == light_leaf)
					continue;
				
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
			
			
			leaf_index = light_leaf - world_leaves;
			leaf_lights[leaf_index].lights[int_index] |= 1 << bit_index;
			
			for(j = 0; j < world_leaves_count; j++)
			{
				r = 1 << (j % 8);
				
				if(!light_leaf->pvs)
					break;
				
				//assert(light_leaf->pvs);
				
				if(light_leaf->pvs[j >> 3] & r)
				{
					leaf_lights[j].lights[int_index] |= 1 << bit_index;
				}		
			}
			
			parms->leaf = (struct bsp_dleaf_t *)light_leaf;
		}	
	}
}



#if 0

void light_CullLights()
{
	int i;
	camera_t *active_camera = camera_GetActiveCamera();
	vec4_t light_origin;
	
	vec2_t ac;
	vec2_t lb;
	vec2_t rb;
	
	float nzfar = -active_camera->frustum.zfar;
	float nznear = -active_camera->frustum.znear;
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
	
	visible_light_count = 0;
	
	light_position_t *position;
	light_params_t *params;
	
	float qt = nznear / ntop;
	float qr = nznear / nright;
	
	int cluster_x;
	int cluster_y;
	int cluster_z;
	
	//glUseProgram(0);	
	
	
	/* TODO: SSE/SSE2/Assembly the shit ouf of this loop... */
	for(i = 0; i < light_count; i++)
	{
		
		position = &light_positions[i];
		params = &light_params[i];
		
		light_origin.x = position->position.x;
		light_origin.y = position->position.y;
		light_origin.z = position->position.z;
		light_origin.w = 1.0;
		
		mat4_t_vec4_t_mult(&active_camera->world_to_camera_matrix, &light_origin);
		
		light_radius = LIGHT_MAX_RADIUS * ((float)params->radius / 0xffff);
		
		
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
		
		
		if((x_max - x_min) * (y_max - y_min) == 0.0 || positive_z == 4)
		{
			continue;
		}
		else
		{
			visible_light_positions[visible_light_count].position = light_origin.vec3;
			visible_light_params[visible_light_count] = light_params[i];
			
			#if 0
			
			cluster_x = (int)(floor((window_width * (x_min * 0.5 + 0.5)) / CLUSTER_SIZE));
			cluster_y = (int)(floor((window_height * (1.0 - (y_max * 0.5 + 0.5))) / CLUSTER_SIZE));
			
			light_z = light_origin.z + light_radius;
			
			
			
			if(light_z > nznear) cluster_z = 0;
			else cluster_z = (int)((log(-light_z / -nznear)/log(-nzfar / -nznear)) * CLUSTER_Z_DIVS);
			
			visible_light_params[visible_light_count].first_cluster_id = PACK_CLUSTER_ID(cluster_x, cluster_y, cluster_z);
			//printf("%d  ", cluster_z);
			
			cluster_x = (int)(floor((window_width * (x_max * 0.5 + 0.5)) / CLUSTER_SIZE));
			cluster_y = (int)(floor((window_height * (1.0 - (y_min * 0.5 + 0.5))) / CLUSTER_SIZE));
			
			light_z = light_origin.z - light_radius;
			
			if(light_z > nznear) cluster_z = 0;
			else cluster_z = (int)((log(-light_z / -nznear)/log(-nzfar / -nznear)) * CLUSTER_Z_DIVS);
			
			visible_light_params[visible_light_count].first_cluster_id = PACK_CLUSTER_ID(cluster_x, cluster_y, cluster_z);
			
			#endif
			
			visible_light_count++;
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
		
	/*	glBegin(GL_LINES);
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
}
#endif

/*

void light_UpdateLightCache()
{
	int i;
	
	
	gpu_lamp_t *lamp;
	
	glBindBuffer(GL_UNIFORM_BUFFER, light_cache_uniform_buffer);
	lamp = (gpu_lamp_t *) glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	
	for(i = 0; i < visible_light_count; i++)
	{
		lamp[i].position.x = visible_light_positions[i].position.x;
		lamp[i].position.y = visible_light_positions[i].position.y;
		lamp[i].position.z = visible_light_positions[i].position.z;
		lamp[i].position.w = 1.0;
		
		lamp[i].color.r = (float)visible_light_params[i].r / 255.0;
		lamp[i].color.g = (float)visible_light_params[i].g / 255.0;
		lamp[i].color.b = (float)visible_light_params[i].b / 255.0;
		
		lamp[i].radius = LIGHT_MAX_RADIUS * ((float)visible_light_params[i].radius / 0xffff);
		lamp[i].energy = LIGHT_MAX_ENERGY * ((float)visible_light_params[i].energy / 0xffff);
	}
	
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
*/



#define MAX_VISIBLE_LIGHTS 256

int visible_light_count;
int visible_lights[MAX_VISIBLE_LIGHTS];

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
	
	if(!world_nodes)
		return;
	
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
			light_CacheLight(i);
		}
	}
	
	
	
	
	//for(i = 0; i < light_count; i++)
	//{
		/*pos = &light_positions[i];
		parms = &light_params[i];
		cur_leaf = bsp_GetCurrentLeaf(world_nodes, pos->position);
		radius = LIGHT_RADIUS(parms->radius);
		radius *= radius;
		
		for(j = 0; j < k; j++)
		{
			if(cur_leaf == visible_leaves[j])
			{		
				visible_lights[visible_light_count++] = i;
				light_CacheLight(i);				
				break;
			}
		}*/
		
	//}
	
	light_Update();
	//printf("light_VisibleLights\n\n");
}

void light_ClearLightLeaves()
{
	int i;
	
	for(i = 0; i < light_count; i++)
	{
		light_params[i].leaf = NULL;
		light_params[i].bm_flags |= LIGHT_MOVED;
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
		
		light_params[light_index].bm_flags |= LIGHT_MOVED;
	}
	 
}


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
				shader_SetCurrentShaderUniform1i(UNIFORM_LIGHT_INDEX, light_cache[cache_index].gpu_index);
			}
		}
	}
}


















