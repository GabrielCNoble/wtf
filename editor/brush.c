#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GL\glew.h"

#include "brush.h"
#include "gpu.h"
#include "vector.h"
#include "matrix.h"
#include "bsp_cmp.h"


/* keeping several copies of the vertices is necessary to enable multiple normals per vertex... */
static float cube_bmodel_verts[] = 
{ 
   -1.0, 1.0, 1.0,-1.0,-1.0, 1.0, 1.0,-1.0, 1.0, 1.0,-1.0, 1.0, 1.0, 1.0, 1.0,-1.0, 1.0, 1.0,
   
   
   
	1.0, 1.0, 1.0, 
	1.0,-1.0, 1.0, 
	1.0,-1.0,-1.0, 
	
	1.0,-1.0,-1.0, 
	1.0, 1.0,-1.0, 
	1.0, 1.0, 1.0,
	
	
	
	1.0, 1.0,-1.0, 1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0, 1.0,-1.0, 1.0, 1.0,-1.0,
	
   -1.0, 1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0, 1.0,-1.0,-1.0, 1.0,-1.0, 1.0, 1.0,-1.0, 1.0,-1.0,
   
   -1.0, 1.0,-1.0,-1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,-1.0,-1.0, 1.0,-1.0,
   
   -1.0,-1.0, 1.0,-1.0,-1.0,-1.0, 1.0,-1.0,-1.0, 1.0,-1.0,-1.0, 1.0,-1.0, 1.0,-1.0,-1.0, 1.0,         
};


static float cube_bmodel_normals[] = 
{
   0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
   
   1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
   
   0.0, 0.0,-1.0, 0.0, 0.0,-1.0, 0.0, 0.0,-1.0, 0.0, 0.0,-1.0, 0.0, 0.0,-1.0, 0.0, 0.0,-1.0,
   
  -1.0, 0.0, 0.0,-1.0, 0.0, 0.0,-1.0, 0.0, 0.0,-1.0, 0.0, 0.0,-1.0, 0.0, 0.0,-1.0, 0.0, 0.0,
  
   0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0,
   
   0.0,-1.0, 0.0, 0.0,-1.0, 0.0, 0.0,-1.0, 0.0, 0.0,-1.0, 0.0, 0.0,-1.0, 0.0, 0.0,-1.0, 0.0,
};



vec3_t cube_bmodel_collision_verts[] = 
{
	 /* +Z */
	{-1.0, 1.0, 1.0},
	{-1.0,-1.0, 1.0}, 
	{ 1.0,-1.0, 1.0},
	{ 1.0, 1.0, 1.0},
	 
	 /* +X */
	{ 1.0, 1.0, 1.0},
	{ 1.0,-1.0, 1.0},
	{ 1.0,-1.0,-1.0},
	{ 1.0, 1.0,-1.0},
	 
	 /* -Z */
	{ 1.0, 1.0,-1.0},
	{ 1.0,-1.0,-1.0},
	{-1.0,-1.0,-1.0},
	{-1.0, 1.0,-1.0},
	
	/* -X */
	{-1.0, 1.0,-1.0},
	{-1.0,-1.0,-1.0},
	{-1.0,-1.0, 1.0},
	{-1.0, 1.0, 1.0},
	
	/* +Y */
	{-1.0, 1.0, 1.0},
	{ 1.0, 1.0, 1.0},
	{ 1.0, 1.0,-1.0},
	{-1.0, 1.0,-1.0},
	
	/* -Y */
	{-1.0,-1.0, 1.0},
	{ 1.0,-1.0, 1.0},
	{ 1.0,-1.0,-1.0},
	{-1.0,-1.0,-1.0},   
	 
};


vec3_t cube_bmodel_collision_normals[] = 
{
	{ 0.0, 0.0, 1.0},
    { 1.0, 0.0, 0.0},
    { 0.0, 0.0,-1.0},
    {-1.0, 0.0, 0.0},
	{ 0.0, 1.0, 0.0},
	{ 0.0,-1.0, 0.0},	
};

#define CUBE_BMODEL_SIZE sizeof(float) * 6 * 36
#define CUBE_BMODEL_VERTEX_COUNT 36

#define CUBE_BMODEL_COLLISION_VERTEX_COUNT 24
#define CUBE_BMODEL_COLLISION_NORMAL_COUNT 6



static int brush_list_size;
int free_position_stack_top = -1;
int *free_position_stack = NULL;
int brush_count;
brush_t *brushes;
int expanded_brush_count;
brush_t *expanded_brushes = NULL;

static unsigned int element_buffer;


/* from editor.c */
extern int default_material;
extern int red_default_material;



void brush_Init()
{
	brush_list_size = 256;
	brush_count = 0;
	expanded_brush_count = 0;
	brushes = malloc(sizeof(brush_t ) * brush_list_size);
	free_position_stack = malloc(sizeof(int*) * brush_list_size);
	
	glGenBuffers(1, &element_buffer);
}

void brush_Finish()
{
	int i;
	for(i = 0; i < brush_count; i++)
	{
		if(brushes[i].type != BRUSH_BOUNDS && brushes[i].type != BRUSH_INVALID)
		{
			free(brushes[i].vertices);
			free(brushes[i].triangles);
			free(brushes[i].triangle_groups);
			
			glDeleteBuffers(1, &brushes[i].element_buffer);	
		}
		
	}
	
	free(brushes);
	free(free_position_stack);
	
	if(expanded_brushes)
		free(expanded_brushes);
	
	glDeleteBuffers(1, &element_buffer);
}

int brush_CreateBrush(vec3_t position, mat3_t *orientation, vec3_t scale, short type)
{
	
	//printf("start of brush_CreateBrush\n");
	
	brush_t *brush;
	int brush_index;
	int i;
	int c;
	int default_group;
	float *vertex_positions;
	float *vertex_normals;
	int vertex_count;
	int alloc_handle;
	int material;
	int *b;
	
	vec3_t p;
	vec3_t v;
	
	if(free_position_stack_top > -1)
	{
		brush_index = free_position_stack[free_position_stack_top];
		
		free_position_stack_top--;
	}
	else
	{
		brush_index = brush_count++;
		
		if(brush_index >= brush_list_size)
		{
			brush = malloc(sizeof(brush_t) * (brush_list_size + 64));
			memcpy(brush, brushes, sizeof(brush_t) * brush_list_size);
			free(brushes);
			
			brushes = brush;
			brush_list_size += 64;
		}
		
	}
	
	brush = &brushes[brush_index];
	
	brush->type = type;
	brush->position = position;
	brush->scale = scale;
	brush->orientation = *orientation;
	//brush->vertex_count = vertex_count;
	//brush->vertices = malloc(sizeof(vertex_t) * vertex_count);
	//brush->triangles = malloc(sizeof(bsp_striangle_t) * (vertex_count / 3));
//	brush->polygons = NULL;
		
//	brush->max_triangle_groups = 4;
//	brush->triangle_groups = malloc(sizeof(triangle_group_t) * brush->max_triangle_groups);
		
//	brush->triangle_group_count = 1;
	
	if(type == BRUSH_BOUNDS)
	{
		brush->vertices = NULL;
		brush->triangle_groups = NULL;
		brush->triangles = NULL;
		brush->vertex_count = 0;
		brush->start = -1;
		brush->handle = -1;
		brush->element_buffer = 0;
	}
	else
	{
		switch(type)
		{
			case BRUSH_CUBE:
			case BRUSH_CYLINDER:
				material = default_material;
				vertex_positions = cube_bmodel_verts;
				vertex_normals = cube_bmodel_normals;
				vertex_count = CUBE_BMODEL_VERTEX_COUNT;
			break;
			
			/*case BRUSH_CYLINDER:
				material = red_default_material;
				vertex_positions = cube_bmodel_verts;
				vertex_normals = cube_bmodel_normals;
				vertex_count = CUBE_BMODEL_VERTEX_COUNT;
			break;*/
			
			/*case BRUSH_CYLINDER:
				brush_CreateCylinderBrush(8, &vertex_count, &vertex_positions, &vertex_normals);
				brush->base_vertex_count = 8;
			break;*/
		}
		
		//brush->collision_vertices = malloc(sizeof(vec3_t ) * CUBE_BMODEL_COLLISION_VERTEX_COUNT);
		//brush->collision_normals = malloc(sizeof(vec3_t) * CUBE)
		
		//brush->type = type;
		//brush->position = position;
		//brush->scale = scale;
		//brush->orientation = *orientation;
		brush->vertex_count = vertex_count;
		brush->vertices = malloc(sizeof(vertex_t) * vertex_count);
		brush->triangles = malloc(sizeof(bsp_striangle_t) * (vertex_count / 3));
		brush->polygons = NULL;
		
		brush->max_triangle_groups = 4;
		brush->triangle_groups = malloc(sizeof(triangle_group_t) * brush->max_triangle_groups);
		
		brush->triangle_group_count = 1;
		
		//if(type == BRUSH_BOUNDS) brush->triangle_groups[0].material_index = -1;
		//else brush->triangle_groups[0].material_index = default_material;
		
		brush->triangle_groups[0].material_index = material;
		brush->triangle_groups[0].start = 0;
		brush->triangle_groups[0].next = 0;
		//brush->triangle_groups[0].vertex_count = vertex_count;
		brush->max_vertexes = vertex_count + 128;
		
		brush->handle = gpu_Alloc(sizeof(vertex_t) * brush->max_vertexes);
		brush->start = gpu_GetAllocStart(brush->handle) / sizeof(vertex_t);
		
		glGenBuffers(1, &brush->element_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brush->element_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * brush->max_vertexes, NULL, GL_DYNAMIC_DRAW);
		
		default_group = 0;
		//if(type == BRUSH_BOUNDS) default_group = -1;
		//else default_group = 0;
		
		for(i = 0; i < vertex_count; i++)
		{
			if(!(i % 3))
			{
				brush->triangles[i / 3].first_vertex = i;
				brush->triangles[i / 3].triangle_group = default_group;		/* only triangle group in this brush upon creation... */
			}
			
			p.x = vertex_positions[i * 3	] * scale.x;
			p.y = vertex_positions[i * 3 + 1] * scale.y;
			p.z = vertex_positions[i * 3 + 2] * scale.z;
			
			
			v.x = p.x * orientation->floats[0][0] +
				  p.y * orientation->floats[1][0] +
				  p.z * orientation->floats[2][0] + position.x;
				  
			v.y = p.x * orientation->floats[0][1] +
				  p.y * orientation->floats[1][1] +
				  p.z * orientation->floats[2][1] + position.y;
				  
			v.z = p.x * orientation->floats[0][2] +
				  p.y * orientation->floats[1][2] +
				  p.z * orientation->floats[2][2] + position.z;	
			
				    	  
			brush->vertices[i].position = v;
			
			
			
			p.x = vertex_normals[i * 3	];
			p.y = vertex_normals[i * 3 + 1];
			p.z = vertex_normals[i * 3 + 2];
			
			
			v.x = p.x * orientation->floats[0][0] +
				  p.y * orientation->floats[1][0] +
				  p.z * orientation->floats[2][0];
				  
			v.y = p.x * orientation->floats[0][1] +
				  p.y * orientation->floats[1][1] +
				  p.z * orientation->floats[2][1];
				  
			v.z = p.x * orientation->floats[0][2] +
				  p.y * orientation->floats[1][2] +
				  p.z * orientation->floats[2][2];	
			
			brush->vertices[i].normal = v;
		}
		
		gpu_Write(brush->handle, 0, brush->vertices, sizeof(vertex_t) * vertex_count, 0);
		
		brush_UpdateBrushElementBuffer(brush);
		
		
		//glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
	}
	
	
	
	
	
	
	
	//printf("end of brush_CreateBrush\n");
	return brush_index;
	
}

int brush_CreateEmptyBrush()
{
	brush_t *brush;
	int brush_index;
	int i;
	int c;
	int default_group;
	float *vertex_positions;
	float *vertex_normals;
	int vertex_count;
	int alloc_handle;
	int *b;
	
	vec3_t p;
	vec3_t v;
		
	if(free_position_stack_top > -1)
	{
		brush_index = free_position_stack[free_position_stack_top];
		
		free_position_stack_top--;
	}
	else
	{
		brush_index = brush_count++;
		
		if(brush_index >= brush_list_size)
		{
			brush = malloc(sizeof(brush_t) * (brush_list_size + 64));
			memcpy(brush, brushes, sizeof(brush_t) * brush_list_size);
			free(brushes);
			
			brushes = brush;
			brush_list_size += 64;
		}
		
	}
	
	brush = &brushes[brush_index];
	
	brush->type = BRUSH_EMPTY;
	brush->polygons = NULL;
	
	return brush_index;
	
	
}

void brush_BuildTriangleGroups(brush_t *brush)
{
	int i;
	int c = brush->vertex_count / 3;
	int j;
	int k;
	
	int triangle_group_count = 0;
	triangle_group_t *triangle_groups = malloc(sizeof(triangle_group_t ) * 512);	/* yeah, this should do... */
	triangle_group_t *cur_group = NULL;
	
	for(i = 0; i < c; i++)
	{
		/*for(j = 0; j < triangle_group_count; j++)
		{
			if(brush->triangles[i].)
		}*/
	}
	
	
	free(triangle_groups);
	
	
	
}

int brush_CopyBrush(brush_t *src)
{
	brush_t *brush;
	int brush_index;
	int i;
	int c;
	int default_group;
	float *vertex_positions;
	float *vertex_normals;
	int vertex_count;
	int alloc_handle;
	int *b;
	
	vec3_t p;
	vec3_t v;
		
	if(src->type == BRUSH_INVALID)
		return -1;	
		
	if(free_position_stack_top > -1)
	{
		brush_index = free_position_stack[free_position_stack_top];
		
		free_position_stack_top--;
	}
	else
	{
		brush_index = brush_count++;
		
		if(brush_index >= brush_list_size)
		{
			brush = malloc(sizeof(brush_t) * (brush_list_size + 64));
			memcpy(brush, brushes, sizeof(brush_t) * brush_list_size);
			free(brushes);
			
			brushes = brush;
			brush_list_size += 64;
		}
		
	}
	
	brush = &brushes[brush_index];
	
	
	brush->type = src->type;
	brush->max_vertexes = src->max_vertexes;
	brush->vertex_count = src->vertex_count;
	brush->vertices = malloc(sizeof(vertex_t) * brush->vertex_count);
	
	for(i = 0; i < brush->vertex_count; i++)
	{
		brush->vertices[i] = src->vertices[i];
		/*printf("[%f %f %f]    [%f %f %f]\n", brush->vertices[i].position.x, brush->vertices[i].position.y, brush->vertices[i].position.z,
											 src->vertices[i].position.x, src->vertices[i].position.y, src->vertices[i].position.z);*/
	}
	
	
	brush->position = src->position;
	brush->scale = src->scale;
	brush->orientation = src->orientation;
	brush->polygons = NULL;
	
	
	brush->max_triangle_groups = src->max_triangle_groups;
	brush->triangle_group_count = src->triangle_group_count;
	brush->triangle_groups = malloc(sizeof(triangle_group_t) * brush->triangle_group_count);
	
	for(i = 0; i < brush->triangle_group_count; i++)
	{
		brush->triangle_groups[i] = src->triangle_groups[i];
		brush->triangle_groups[i].next = 0;
		
		/*printf("[%d %d %d]    [%d %d %d]\n", brush->triangle_groups[i].material_index, brush->triangle_groups[i].start, brush->triangle_groups[i].next, 
		                                     src->triangle_groups[i].material_index, src->triangle_groups[i].start, src->triangle_groups[i].next);*/
		
	}
	
	
	//brush->triangles = src->triangles;
	
	brush->triangles = malloc(sizeof(bsp_striangle_t) * (brush->vertex_count / 3));
	
	for(i = 0; i < brush->vertex_count / 3; i++)
	{
		brush->triangles[i] = src->triangles[i];
	}
	
	brush->handle = gpu_Alloc(sizeof(vertex_t) * brush->vertex_count);
	brush->start = gpu_GetAllocStart(brush->handle) / sizeof(vertex_t);
	gpu_Write(brush->handle, 0, brush->vertices, sizeof(vertex_t) * brush->vertex_count, 0);
	
	glGenBuffers(1, &brush->element_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brush->element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * brush->vertex_count, NULL, GL_DYNAMIC_DRAW);
	brush_UpdateBrushElementBuffer(brush);
	
	
	
	
	return brush_index;
	
}

void brush_DestroyBrush(brush_t *brush)
{
	bsp_polygon_t *polygon;
	bsp_polygon_t *next;
	
	int brush_index;
	
	if(brush->type == BRUSH_INVALID)
		return;
	
	if(brush->type != BRUSH_BOUNDS)
	{
		free(brush->vertices);
		free(brush->triangle_groups);
		free(brush->triangles);
		polygon = brush->polygons;
		
		while(polygon)
		{
			next = polygon->next;
			free(polygon->vertices);
			polygon = next;
		}
		gpu_Free(brush->handle);
		glDeleteBuffers(1, &brush->element_buffer);
	}
	
	brush->type = BRUSH_INVALID;
	brush->polygons = NULL;
	brush_index = brush - brushes;
	
	free_position_stack_top++;
	
	free_position_stack[free_position_stack_top] = brush_index;
}

void brush_DestroyBrushIndex(int brush_index)
{
	brush_DestroyBrush(&brushes[brush_index]);
}

void brush_DestroyAllBrushes()
{
	int i;
	
	for(i = 0; i < brush_count; i++)
	{
		brush_DestroyBrush(&brushes[i]);
	}
	
	brush_count = 0;
	free_position_stack_top = -1;
}

void brush_CreateCylinderBrush(int base_vertexes, int *vert_count, float **vertices, float **normals)
{
	float *verts;
	float *norms;
	int i;
	int k;
	float a;
	float b = (2.0 * 3.14159265) / base_vertexes;
	float l;
	
	vec3_t r;
	vec3_t t;
	vec3_t s;
	
	if(base_vertexes < 3)
	{
		*vertices = NULL;
		*normals = NULL;
		*vert_count = 0;
		return;
	}
	
	*vert_count = ((base_vertexes * 2 * 3) + (base_vertexes * 6));
	
	//*vertices = (vertex_t *)malloc(sizeof(vertex_t) * (*vert_count));
	*vertices = (float *)malloc(sizeof(float) * 3 * (*vert_count));
	*normals = (float *)malloc(sizeof(float) * 3 * (*vert_count));
	
	verts = *vertices;
	norms = *normals;
	a = 0.0;
	
	/* top cap... */
	for(i = 0; i < base_vertexes * 3;)
	{
		
		verts[i * 3] = 0.0;
		verts[i * 3 + 1] = 1.0;
		verts[i * 3 + 2] = 0.0;
		
		/*(*vertices)[i].position.x = 0.0;
		(*vertices)[i].position.y = 1.0;
		(*vertices)[i].position.z = 0.0;*/
		i++;
		
		
		
		verts[i * 3] = cos(a);
		verts[i * 3 + 1] = 1.0;
		verts[i * 3 + 2] = sin(a);
		/*(*vertices)[i].position.x = cos(a);
		(*vertices)[i].position.y = 1.0;
		(*vertices)[i].position.z = sin(a);*/
		a -= b;
		i++;
		
		verts[i * 3] = cos(a);
		verts[i * 3 + 1] = 1.0;
		verts[i * 3 + 2] = sin(a);
		/*(*vertices)[i].position.x = cos(a);
		(*vertices)[i].position.y = 1.0;
		(*vertices)[i].position.z = sin(a);*/
		i++;
	}
	
	k = i;
	a = 0.0;
	
	/* bottom cap... */
	for(i = 0; i < base_vertexes * 3;)
	{
		verts[k * 3 + i * 3] = 0.0;
		verts[k * 3 + i * 3 + 1] = -1.0;
		verts[k * 3 + i * 3 + 2] = 0.0;
		/*(*vertices)[k + i].position.x = 0.0;
		(*vertices)[k + i].position.y = -1.0;
		(*vertices)[k + i].position.z = 0.0;*/
		i++;
		
		verts[k * 3 + i * 3] = cos(a);
		verts[k * 3 + i * 3 + 1] = -1.0;
		verts[k * 3 + i * 3 + 2] = sin(a);
		
		/*(*vertices)[k + i].position.x = cos(a);
		(*vertices)[k + i].position.y = -1.0;
		(*vertices)[k + i].position.z = sin(a);*/
		a += b;
		i++;
		
		verts[k * 3 + i * 3] = cos(a);
		verts[k * 3 + i * 3 + 1] = -1.0;
		verts[k * 3 + i * 3 + 2] = sin(a);
		
		/*(*vertices)[k + i].position.x = cos(a);
		(*vertices)[k + i].position.y = -1.0;
		(*vertices)[k + i].position.z = sin(a);*/
		i++;
	}
	
	k += i;
	a = 0.0;
	/* side... */
	for(i = 0; i < base_vertexes * 6;)
	{
		verts[k * 3 + i * 3] = cos(a);
		verts[k * 3 + i * 3 + 1] = 1.0;
		verts[k * 3 + i * 3 + 2] = sin(a);
		
		/*(*vertices)[k + i].position.x = cos(a);
		(*vertices)[k + i].position.y = 1.0;
		(*vertices)[k + i].position.z = sin(a);*/
		i++;
		
		verts[k * 3 + i * 3] = verts[k * 3 + (i - 1) * 3];
		verts[k * 3 + i * 3 + 1] = -1.0;
		verts[k * 3 + i * 3 + 2] = verts[k * 3 + (i - 1) * 3 + 2];
		
		/*(*vertices)[k + i].position.x = (*vertices)[k + i - 1].position.x;
		(*vertices)[k + i].position.y = -1.0;
		(*vertices)[k + i].position.z = (*vertices)[k + i - 1].position.z;*/
		a -= b;
		i++;
		
		
		verts[k * 3 + i * 3] = cos(a);
		verts[k * 3 + i * 3 + 1] = 1.0;
		verts[k * 3 + i * 3 + 2] = sin(a);
		/*(*vertices)[k + i].position.x = cos(a);
		(*vertices)[k + i].position.y = 1.0;
		(*vertices)[k + i].position.z = sin(a);*/
		i++;
		
		verts[k * 3 + i * 3] = verts[k * 3 + (i - 1) * 3];
		verts[k * 3 + i * 3 + 1] = 1.0;
		verts[k * 3 + i * 3 + 2] = verts[k * 3 + (i - 1) * 3 + 2];
		/*(*vertices)[k + i].position.x = (*vertices)[k + i - 1].position.x;
		(*vertices)[k + i].position.y = 1.0;
		(*vertices)[k + i].position.z = (*vertices)[k + i - 1].position.z;*/
		i++;
		
		verts[k * 3 + i * 3] = verts[k * 3 + (i - 3) * 3];
		verts[k * 3 + i * 3 + 1] = -1.0;
		verts[k * 3 + i * 3 + 2] = verts[k * 3 + (i - 3) * 3 + 2];
		/*(*vertices)[k + i].position.x = (*vertices)[k + i - 3].position.x;
		(*vertices)[k + i].position.y = -1.0;
		(*vertices)[k + i].position.z = (*vertices)[k + i - 3].position.z;*/
		i++;
		
		verts[k * 3 + i * 3] = verts[k * 3 + (i - 3) * 3];
		verts[k * 3 + i * 3 + 1] = -1.0;
		verts[k * 3 + i * 3 + 2] = verts[k * 3 + (i - 3) * 3 + 2];
		/*(*vertices)[k + i].position.x = (*vertices)[k + i - 3].position.x;
		(*vertices)[k + i].position.y = -1.0;
		(*vertices)[k + i].position.z = (*vertices)[k + i - 3].position.z;*/
		i++;
	}
	
	k = *vert_count;
	
	for(i = 0; i < k;)
	{
		s.x = verts[(i + 1) * 3] - verts[i * 3];
		s.y = verts[(i + 1) * 3 + 1] - verts[i * 3 + 1];
		s.z = verts[(i + 1) * 3 + 2] - verts[i * 3 + 2];
		
		t.x = verts[(i + 2) * 3] - verts[i * 3];
		t.y = verts[(i + 2) * 3 + 1] - verts[i * 3 + 1];
		t.z = verts[(i + 2) * 3 + 2] - verts[i * 3 + 2];
		
		/*s.x = (*vertices)[i + 1].position.x - (*vertices)[i].position.x;
		s.y = (*vertices)[i + 1].position.y - (*vertices)[i].position.y;
		s.z = (*vertices)[i + 1].position.z - (*vertices)[i].position.z;
		
		
		t.x = (*vertices)[i + 2].position.x - (*vertices)[i].position.x;
		t.y = (*vertices)[i + 2].position.y - (*vertices)[i].position.y;
		t.z = (*vertices)[i + 2].position.z - (*vertices)[i].position.z;*/
		
		
		r = cross(s, t);
		
		
	/*	(*vertices)[i].normal.x = r.x;
		(*vertices)[i].normal.y = r.y;
		(*vertices)[i].normal.z = r.z;
		
		(*vertices)[i + 1].normal.x = r.x;
		(*vertices)[i + 1].normal.y = r.y;
		(*vertices)[i + 1].normal.z = r.z;
		
		(*vertices)[i + 2].normal.x = r.x;
		(*vertices)[i + 2].normal.y = r.y;
		(*vertices)[i + 2].normal.z = r.z;*/
		
		l = sqrt(r.x * r.x + r.y * r.y + r.z * r.z);
		
		norms[i * 3] = r.x / l;
		norms[i * 3 + 1] = r.y / l;
		norms[i * 3 + 2] = r.z / l;
		
		norms[(i + 1) * 3] = r.x / l;
		norms[(i + 1) * 3 + 1] = r.y / l;
		norms[(i + 1) * 3 + 2] = r.z / l;
		
		norms[(i + 2) * 3] = r.x / l;
		norms[(i + 2) * 3 + 1] = r.y / l;
		norms[(i + 2) * 3 + 2] = r.z / l;
		
		i += 3;
	}
}

void brush_UpdateBrushElementBuffer(brush_t *brush)
{
	int i;
	int c;
	int index_start;
	int triangle_group;
	int index_current;
	int *b;
	c = brush->vertex_count / 3;
	//start = brush->start;
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brush->element_buffer);
	b = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	
	for(i = 0; i < c; i++)
	{
		/* the triangle_group_t of this triangle... */
		triangle_group = brush->triangles[i].triangle_group;
		
		/* the offset within the GL_ELEMENT_ARRAY_BUFFER of this brush... */
		index_start = brush->triangle_groups[triangle_group].start;
		
		/* position this entry should go... */
		index_current = brush->triangle_groups[triangle_group].next;
		
		/* brush->start offsets to its first vertex inside the gpu heap... */
		b[index_start + index_current	 ] = brush->triangles[i].first_vertex + 	brush->start;
		b[index_start + index_current + 1] = brush->triangles[i].first_vertex + 1 + brush->start;
		b[index_start + index_current + 2] = brush->triangles[i].first_vertex + 2 + brush->start;
		
		brush->triangle_groups[triangle_group].next += 3;
	}
	
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void brush_UpdateBrush(brush_t *brush)
{
	//int last_vert_count = gpu_GetAllocSize(brush.draw_data->handle) / sizeof(vertex_t);
	//int size = sizeof(vertex_t) * brush.draw_data->vert_count;
	//int size = brush.draw_data->vert_count * 6 * sizeof(float);
	/*if(brush.draw_data->vert_count > last_vert_count)
	{
		gpu_Free(brush.draw_data->handle);
		brush.draw_data->handle = gpu_Alloc(size);
		brush.draw_data->start = gpu_GetAllocStart(brush.draw_data->handle);
	}*/
	
	gpu_Write(brush->handle, 0, brush->vertices, sizeof(vertex_t) * brush->vertex_count, 0);
}

void brush_TranslateBrush(brush_t *brush, vec3_t direction)
{
	int i;
	int c = brush->vertex_count;
	
	for(i = 0; i < c; i++)
	{		
		brush->vertices[i].position.x += direction.x;
		brush->vertices[i].position.y += direction.y;
		brush->vertices[i].position.z += direction.z;
	}
	
	brush->position.x += direction.x;
	brush->position.y += direction.y;
	brush->position.z += direction.z;
	
	brush_UpdateBrush(brush);
}

void brush_RotateBrush(brush_t *brush, vec3_t axis, float amount)
{
	int i;
	int c = brush->vertex_count;
	vec3_t v;
	vec3_t p;
	vec3_t n;
	
	mat3_t rotation;
	mat3_t old_rotation;
	
	
	mat3_t_rotate(&rotation, axis, amount, 1);
	
	memcpy(&old_rotation.floats[0][0], &brush->orientation.floats[0][0], sizeof(mat3_t));
	
	mat3_t_mult(&brush->orientation, &old_rotation, &rotation);
	
	for(i = 0; i < c; i++)
	{
		
		v.x = brush->vertices[i].position.x;
		v.y = brush->vertices[i].position.y;
		v.z = brush->vertices[i].position.z;
		
		n.x = brush->vertices[i].normal.x;
		n.y = brush->vertices[i].normal.y;
		n.z = brush->vertices[i].normal.z;
		
		v.x -= brush->position.x;
		v.y -= brush->position.y;
		v.z -= brush->position.z;
		
		p.x = v.x * rotation.floats[0][0] + 
		      v.y * rotation.floats[1][0] + 
		      v.z * rotation.floats[2][0];
		      
		p.y = v.x * rotation.floats[0][1] + 
		      v.y * rotation.floats[1][1] + 
		      v.z * rotation.floats[2][1];
			  
		p.z = v.x * rotation.floats[0][2] + 
		      v.y * rotation.floats[1][2] + 
		      v.z * rotation.floats[2][2];	        
		
		
		brush->vertices[i].position.x = p.x + brush->position.x;
		brush->vertices[i].position.y = p.y + brush->position.y;
		brush->vertices[i].position.z = p.z + brush->position.z;
		
		
		
		p.x = n.x * rotation.floats[0][0] + 
		      n.y * rotation.floats[1][0] + 
		      n.z * rotation.floats[2][0];
		      
		p.y = n.x * rotation.floats[0][1] + 
		      n.y * rotation.floats[1][1] + 
		      n.z * rotation.floats[2][1];
			  
		p.z = n.x * rotation.floats[0][2] + 
		      n.y * rotation.floats[1][2] + 
		      n.z * rotation.floats[2][2];	        
		
		/*brush.draw_data->verts[3 + i * 6] = p.x;
		brush.draw_data->verts[3 + i * 6 + 1] = p.y;
		brush.draw_data->verts[3 + i * 6 + 2] = p.z;*/
		
		brush->vertices[i].normal.x = p.x;
		brush->vertices[i].normal.y = p.y;
		brush->vertices[i].normal.z = p.z;
	}
	
	brush_UpdateBrush(brush);
}

void brush_ScaleBrush(brush_t *brush, vec3_t axis, float amount)
{
	int i;
	//int c = brush.draw_data->vert_count;
	
	int c = brush->vertex_count;
	
	vec3_t prev_scale;
	vec3_t new_scale;
	vec3_t translation;
	vec3_t v;
	vec3_t p;
	
	float f;
	
	mat3_t inverse_rotation;

	
	prev_scale = brush->scale;
	
	translation.x = brush->position.x;
	translation.y = brush->position.y;
	translation.z = brush->position.z;
	
	new_scale.x = prev_scale.x + axis.x * amount;
	new_scale.y = prev_scale.y + axis.y * amount;
	new_scale.z = prev_scale.z + axis.z * amount;
	
	if(new_scale.x <= 0.025)
	{
		f = 0.025 - new_scale.x;
		new_scale.x += f; 
	}
	
	if(new_scale.y <= 0.025)
	{
		f = 0.025 - new_scale.y;
		new_scale.y += f; 
	}
	
	if(new_scale.z <= 0.025)
	{
		f = 0.025 - new_scale.z;
		new_scale.z += f; 
	}
	
	
		
	
	/*brush->scale.x += axis.x * amount;
	brush->scale.y += axis.y * amount;
	brush->scale.z += axis.z * amount;*/
	
	
	//prev_scale = MultiplyVector3(&brush.position_data->orientation, prev_scale);
	//new_scale = MultiplyVector3(&brush.position_data->orientation, brush.position_data->scale);
	
	memcpy(&inverse_rotation.floats[0][0], &brush->orientation.floats[0][0], sizeof(mat3_t));
	
	mat3_t_transpose(&inverse_rotation);
	
	//new_scale = brush->scale;
	
	
		
	brush->scale = new_scale;	
	
	for(i = 0; i < c; i++)
	{
		
		//v.x = brush.draw_data->verts[i * 6] - translation.x;
		//v.y = brush.draw_data->verts[i * 6 + 1] - translation.y;
		//v.z = brush.draw_data->verts[i * 6 + 2] - translation.z;
		
		v.x = brush->vertices[i].position.x - translation.x;
		v.y = brush->vertices[i].position.y - translation.y;
		v.z = brush->vertices[i].position.z - translation.z;
		
		v = MultiplyVector3(&inverse_rotation, v);
		
		v.x *= (new_scale.x / prev_scale.x);
		v.y *= (new_scale.y / prev_scale.y);
		v.z *= (new_scale.z / prev_scale.z);
		
		v = MultiplyVector3(&brush->orientation, v);
		
		/*brush.draw_data->verts[i * 6] = v.x  + translation.x;
		brush.draw_data->verts[i * 6 + 1] = v.y + translation.y;
		brush.draw_data->verts[i * 6 + 2] = v.z + translation.z;*/
		
		brush->vertices[i].position.x = v.x + translation.x;
		brush->vertices[i].position.y = v.y + translation.y;
		brush->vertices[i].position.z = v.z + translation.z;
		
	}
	
	brush_UpdateBrush(brush);
}




















