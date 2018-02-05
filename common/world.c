#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GL\glew.h"

#include "gpu.h"
#include "physics.h"
#include "material.h"
#include "shader.h"

#include "camera.h"

#include "world.h"
#include "mesh.h"
#include "texture.h"

#include "l_main.h"
#include "l_cache.h"

#include "bsp_common.h"
#include "bsp.h"
#include "bsp_file.h"

#include "engine.h"

//bsp_node_t *world_bsp = NULL;
//int world_leaves_count = 0;
//bsp_sleaf_t *world_leaves = NULL;
int world_vertices_count = 0;
vertex_t *world_vertices = NULL;

int world_nodes_count = 0;
bsp_pnode_t *world_nodes = NULL;
int collision_nodes_count = 0;
bsp_pnode_t *collision_nodes = NULL;


int world_leaves_count = 0;
bsp_dleaf_t *world_leaves = NULL;
bsp_lights_t *leaf_lights = NULL;
bsp_batch_t *leaf_batches = NULL; 
int *leaf_batches_indexes = NULL;

int world_hull_node_count = 0;
bsp_pnode_t *world_hull = NULL;

int visible_leaves_count = 0;
bsp_dleaf_t **visible_leaves = NULL;



//static int max_world_mesh_vertexes;
//static int world_mesh_vertex_count;

//static vertex_t *world_mesh_vertices;
//static world_mesh_triangle_t *world_mesh;

int world_handle = -1;
int world_start = -1;
int world_count = -1;

//static int global_triangle_group_list_size;
int world_triangle_group_count = 0;
triangle_group_t *world_triangle_groups = NULL;
unsigned int *index_buffer = NULL;
unsigned int world_element_buffer;

extern int forward_pass_shader;




void world_Init()
{
	
	//global_triangle_group_list_size = 0;
	world_triangle_group_count = 0;
	world_triangle_groups = NULL;
	//index_buffer = NULL;
	
	//index_buffer = malloc(sizeof(int) * 200000);
	glGenBuffers(1, &world_element_buffer);
	
	/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 200000, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);*/
	
}

void world_Finish()
{
	//free(world_mesh);
	//free(world_mesh_vertices);

	
	if(world_triangle_groups)
	{
		free(world_triangle_groups);
	}
	if(index_buffer)
	{
		free(index_buffer);
	}
	
	bsp_DeleteBsp();
	
	glDeleteBuffers(1, &world_element_buffer);
}


int world_LoadBsp(char *file_name)
{
	FILE *file;
	bsp_header_t header;
	light_lump_t light_lump;
	file = fopen(file_name, "rb");
	
	if(!file)
	{
		printf("couldn't open file [%s]\n", file_name);
		return 0;
	}
	
	int i;
	int light_count;
	
	fread(&header, sizeof(bsp_header_t), 1, file);
	
	for(i = 0; i < header.light_count; i++)
	{	
		fread(&light_lump, sizeof(light_lump_t), 1, file);
		
		light_CreateLight("light", &light_lump.orientation, light_lump.position, light_lump.color, light_lump.radius, light_lump.energy, light_lump.bm_flags);
	}
	
	world_nodes = malloc(sizeof(bsp_pnode_t) * header.world_nodes_count);
	world_leaves = malloc(sizeof(bsp_dleaf_t) * header.world_leaves_count);
	collision_nodes = malloc(sizeof(bsp_pnode_t) * header.collision_nodes_count);
	
	fread(world_nodes, sizeof(bsp_pnode_t), world_nodes_count, file);
	fread(world_leaves, sizeof(bsp_dleaf_t), world_leaves_count, file);
	fread(collision_nodes, sizeof(bsp_pnode_t), collision_nodes_count, file);
	
	
	
	fclose(file);
	
	
	return 1;
}

void world_BuildBatches()
{
	
	int i;
	int c;
	
	int k;
	bsp_dleaf_t *leaf;
	
	if(!world_leaves)
		return;
		
	int total_batches = 0;	
	
	for(i = 0; i < world_leaves_count; i++)
	{
		for(k = 0; k < world_triangle_group_count; k++)
		{
			world_triangle_groups[k].next = 0;
		}
		
		leaf = &world_leaves[i];
		
		c = leaf->tris_count;
		
		for(k = 0; k < c; k++)
		{
			world_triangle_groups[leaf->tris[k].triangle_group].next++;
		}
		
		
		for(k = 0; k < world_triangle_group_count; k++)
		{
			if(world_triangle_groups[k].next)
			{
				total_batches++;
			}
		}
	}
		
		
}

void world_VisibleLeaves()
{
	camera_t *active_camera = camera_GetActiveCamera();
	visible_leaves = bsp_PotentiallyVisibleLeaves(&visible_leaves_count, active_camera->world_position);
	
}

/* BOTTLENECK: */
void world_VisibleWorld()
{
	
	int i;
	int c;
	int j;
	
	int start;
	int next;
	int leaf_index;
	int positive_z;
	
	camera_t *active_camera = camera_GetActiveCamera();
	
	float nzfar = -active_camera->frustum.zfar;
	float nznear = -active_camera->frustum.znear;
	float ntop = active_camera->frustum.top;
	float nright = active_camera->frustum.right;
	float qt = nznear / ntop;
	float qr = nznear / nright;
	float x_max;
	float x_min;
	float y_max;
	float y_min;
	
	float s;
	float e;
	
	bsp_dleaf_t *leaf;
	triangle_group_t *group;
	bsp_striangle_t *triangle;
	//bsp_dleaf_t **visible;
	//int visible_count;
	unsigned int *indexes;
	
	vec4_t corners[8];
		
	if(!world_nodes)
		return;
	
	
	//s = engine_GetDeltaTime();
	
	//visible_leaves_count = 0;
	//leaf = bsp_GetCurrentLeaf(world_nodes, active_camera->world_position);
	visible_leaves = bsp_PotentiallyVisibleLeaves(&visible_leaves_count, active_camera->world_position);
	
	/*e = engine_GetDeltaTime();
	
	printf("%f\n", e - s);*/
		
	if(visible_leaves)
	{
		
		//printf("camera in leaf %d\n", visible_leaves[0]->leaf_index);
		//light_VisibleLights(visible_leaves, visited_leaves_count);
		
		
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world_element_buffer);
		
		//s = engine_GetDeltaTime();
		
		/* BOTTLENECK: this call is taking between 2 and 6 ms, with brief sparks of 10
		ms... holy shit... */
		//indexes = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
		indexes = index_buffer;
		//e = engine_GetDeltaTime();
		
		
		//printf("%f\n", e - s);
		
		
		/* zero out the next index of every triangle group... */
		for(i = 0; i < world_triangle_group_count; i++)
		{
			world_triangle_groups[i].next = 0;
		}
		
		//printf("current pvs: %d leaves     ", visited_leaves_count);
		
		for(j = 0; j < visible_leaves_count; j++)
		{
			break;
			leaf = visible_leaves[j];
			
			corners[0].x = leaf->center.x - leaf->extents.x;
			corners[0].y = leaf->center.y + leaf->extents.y;
			corners[0].z = leaf->center.z + leaf->extents.z;
			corners[0].w = 1.0;
			
			corners[1].x = leaf->center.x - leaf->extents.x;
			corners[1].y = leaf->center.y - leaf->extents.y;
			corners[1].z = leaf->center.z + leaf->extents.z;
			corners[1].w = 1.0;
			
			corners[2].x = leaf->center.x + leaf->extents.x;
			corners[2].y = leaf->center.y - leaf->extents.y;
			corners[2].z = leaf->center.z + leaf->extents.z;
			corners[2].w = 1.0;
			
			corners[3].x = leaf->center.x + leaf->extents.x;
			corners[3].y = leaf->center.y + leaf->extents.y;
			corners[3].z = leaf->center.z + leaf->extents.z;
			corners[3].w = 1.0;
			
			
			
			corners[4].x = leaf->center.x - leaf->extents.x;
			corners[4].y = leaf->center.y + leaf->extents.y;
			corners[4].z = leaf->center.z - leaf->extents.z;
			corners[4].w = 1.0;
			
			corners[5].x = leaf->center.x - leaf->extents.x;
			corners[5].y = leaf->center.y - leaf->extents.y;
			corners[5].z = leaf->center.z - leaf->extents.z;
			corners[5].w = 1.0;
			
			corners[6].x = leaf->center.x + leaf->extents.x;
			corners[6].y = leaf->center.y - leaf->extents.y;
			corners[6].z = leaf->center.z - leaf->extents.z;
			corners[6].w = 1.0;
			
			corners[7].x = leaf->center.x + leaf->extents.x;
			corners[7].y = leaf->center.y + leaf->extents.y;
			corners[7].z = leaf->center.z - leaf->extents.z;
			corners[7].w = 1.0;
			
			x_max = -9999999999.9;
			x_min = 9999999999.9;
			
			y_max = -9999999999.9;
			y_min = 9999999999.9;
			
			positive_z = 0;
			for(i = 0; i < 8; i++)
			{
				mat4_t_vec4_t_mult(&active_camera->world_to_camera_matrix, &corners[i]);
				if(corners[i].z > nznear)
				{
					corners[i].z = nznear;
					positive_z++;
				}
				
				corners[i].x = (corners[i].x * qr) / corners[i].z;
				corners[i].y = (corners[i].y * qt) / corners[i].z;
				
				if(corners[i].x > x_max) x_max = corners[i].x;
				if(corners[i].x < x_min) x_min = corners[i].x;
				
				if(corners[i].y > y_max) y_max = corners[i].y;
				if(corners[i].y < y_min) y_min = corners[i].y;
						
			}
			
			
			if(x_max > 1.0) x_max = 1.0;
			else if(x_max < -1.0) x_max = -1.0;
			
			if(x_min > 1.0) x_min = 1.0;
			else if(x_min < -1.0) x_min = -1.0;
			
			if(y_max > 1.0) y_max = 1.0;
			else if(y_max < -1.0) y_max = -1.0;
			
			if(y_min > 1.0) y_min = 1.0;
			else if(y_min < -1.0) y_min = -1.0;
			
			
			if((x_max - x_min) * (y_max - y_min) <= 0.0 || positive_z == 8)
			{
				if(j < visible_leaves_count - 1)
				{
					visible_leaves[j] = visible_leaves[visible_leaves_count - 1];	
				}
				j--;
				visible_leaves_count--;
				continue;
			}
			
			
			/*glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glUseProgram(0);
			
			glPointSize(8.0);
			glBegin(GL_LINES);
			glColor3f(0.0, 1.0, 0.0);
			
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
		
		
		//printf("draw leaves: %d\n", visited_leaves_count);
		
		//visited_leaves_count = 0;
		//c = world_leaves_count;
		
		/* add the current leaf to the list of visited leaves... */
		//visited_leaves[visited_leaves_count++] = leaf;
		
		
		/* access this leaf's pvs... */
		/*for(i = 0; i < c; i++)
		{
			if(leaf->pvs[i >> 3] & (1 << (i % 8)))
			{
				visited_leaves[visited_leaves_count++] = &world_leaves[i];
			}
		}*/
		
		
		
		/* for each leaf on the list... */
		for(j = 0; j < visible_leaves_count; j++)
		{
			leaf = visible_leaves[j];
			
			c = leaf->tris_count;
			
			/* add it's triangles for rendering... */
			for(i = 0; i < c; i++)
			{
				triangle = &leaf->tris[i];
				
				group = &world_triangle_groups[triangle->triangle_group];
				
				start = group->start;
				next = group->next;
				
				/* the world's GL_ELEMENT_ARRAY_BUFFER indexes into
				the gpu heap, so this is why world_start is added here... */
				indexes[start + next	] = world_start + triangle->first_vertex;
				indexes[start + next + 1] = world_start + triangle->first_vertex + 1;
				indexes[start + next + 2] = world_start + triangle->first_vertex + 2;
				
				group->next += 3;
			}
		}

		//glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(int) * world_vertices_count, index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		e = engine_GetDeltaTime();
		
		//printf("%f\n", e - s);
		
		
		
	}	
		
}


void world_Update()
{
	int i;
	int j;
	int c;
	int k;
	
	bsp_dleaf_t *leaf;
	
	int total_batches = 0;	
	int cur_group_index;
	int cur_batch_index;
	
	
	if(!world_leaves)
		return;
	
	if(world_handle != -1)
	{
		gpu_Free(world_handle);
		free(index_buffer);
	}
	if(leaf_lights)
	{
		free(leaf_lights);
	}
	
	leaf_lights = malloc(sizeof(bsp_lights_t ) * world_leaves_count);
	index_buffer = malloc(sizeof(unsigned int ) * world_vertices_count * 10);
	
	for(i = 0; i < world_leaves_count; i++)
	{
		for(j = 0; j < MAX_WORLD_LIGHTS >> 5; j++)
		{
			leaf_lights[i].lights[j] = 0;	
		}
	}
		
	world_handle = gpu_Alloc(sizeof(vertex_t) * world_vertices_count);
	world_start = gpu_GetAllocStart(world_handle) / sizeof(vertex_t);
	
	gpu_Write(world_handle, 0, world_vertices, sizeof(vertex_t) * world_vertices_count, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world_element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * world_vertices_count, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	light_ClearLightLeaves();
	//light_UpdateCacheGroups();
	
}





void world_Move(vec3_t *position, vec3_t *velocity)
{
	
}

void world_TryStepUp(vec3_t *position, vec3_t *velocity, trace_t *trace)
{
	
}

void world_TryStepDown(vec3_t *position, vec3_t *velocity, trace_t *trace)
{
	
}













