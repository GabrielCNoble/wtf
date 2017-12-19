#include <stdio.h>
#include <stdlib.h>


#include "brush.h"
#include "collision.h"
#include "vector_types.h"
#include "gpu.h"

/* from brush.c */
extern int brush_count;
extern brush_t *brushes;


int collision_geometry_vertice_count;
vec3_t *collision_geometry_positions;
vec3_t *collision_geometry_normals;
int *collision_geometry_quantized_normals;

int collision_geometry_start;
int collision_geometry_count;
int collision_geometry_handle;

void collision_Init()
{
	//collision_geometry_vertice_list_size = 0;
	collision_geometry_vertice_count = 0;
	collision_geometry_positions = NULL;
	collision_geometry_normals = NULL;
	//collision_geometry_positions = malloc(sizeof(vec3_t) * collision_geometry_vertice_list_size);
	//collision_geometry_normals = malloc(sizeof(vec3_t) * collision_geometry_vertice_list_size);
}

void collision_Finish()
{
	if(collision_geometry_positions)
	{
		free(collision_geometry_positions);
		free(collision_geometry_normals);
		free(collision_geometry_quantized_normals);
	}
}

void collision_GenerateCollisionGeometry()
{
	
	int i;
	int c = brush_count;
	int j;
	int k;
	int x;
	brush_t *brush;
	int vertice_count = 0;
	
	int q_normal;
	
	vec3_t normal;
	float l;
	
	for(i = 0; i < c; i++)
	{
		vertice_count += brushes[i].vertex_count;
	}
	
	
	if(collision_geometry_positions)
	{
		free(collision_geometry_positions);
		free(collision_geometry_normals);
		free(collision_geometry_quantized_normals);
		//gpu_Free(collision_geometry_handle);
	}
	
	collision_geometry_vertice_count = vertice_count;
	
	collision_geometry_positions = malloc(sizeof(vec3_t) * vertice_count);
	collision_geometry_normals = malloc(sizeof(vec3_t) * vertice_count);
	collision_geometry_quantized_normals = malloc(sizeof(int) * vertice_count);
	
	//collision_geometry_handle = gpu_Alloc(sizeof(vec3_t) * vertice_count);
	
	
	
	x = 0;
	
	
	for(i = 0; i < c; i++)
	{
		brush = &brushes[i];
		k = brush->vertex_count;
		
		for(j = 0; j < k; j++)
		{
			
			normal = brush->vertices[j].normal;
			
			l = fabs(dot3(normal, vec3(0.0, 1.0, 0.0)));
			
			
			collision_geometry_positions[x] = brush->vertices[j].position;
			
			/*collision_geometry_positions[x].x += normal.x * l;
			collision_geometry_positions[x].y += normal.y * l;
			collision_geometry_positions[x].z += normal.z * l;*/
			
			
			collision_geometry_normals[x] = normal;
			
			q_normal |= (int)(0x3ff * (normal.z * 0.5 + 0.5));
			q_normal <<= 10;
			q_normal |= (int)(0x3ff * (normal.y * 0.5 + 0.5));
			q_normal <<= 10;
			q_normal |= (int)(0x3ff * (normal.x * 0.5 + 0.5));
			
			collision_geometry_quantized_normals[x] = q_normal;
			
			x++;
		}
	}
	
	//gpu_Write(collision_geometry_handle, 0, collision_geometry_positions, vertice_count * sizeof(vec3_t), 0);
	
	//collision_geometry_start = gpu_GetAllocStart(collision_geometry_handle) / sizeof(vertex_t);
	//collision_geometry_count = vertice_count;
	
}

void collision_BuildCollisionBVH()
{
	
}




