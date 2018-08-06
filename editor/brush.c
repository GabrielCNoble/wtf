#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GL\glew.h"

#include "brush.h"
#include "gpu.h"
#include "vector.h"
#include "matrix.h"
#include "bsp_cmp.h"
#include "input.h"
#include "material.h"
#include "camera.h"
#include "c_memory.h"

#include "..\..\common\r_debug.h"

#define malloc ((void *)0)
#define calloc ((void *)0)
#define free ((void *)0)
#define strdup ((void *)0)





vec3_t cube_bmodel_verts[] =
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
	{-1.0,-1.0,-1.0},
	{ 1.0,-1.0,-1.0},
	{ 1.0,-1.0, 1.0},
	{-1.0,-1.0, 1.0},
};


vec3_t cube_bmodel_tangents[] =
{
	{ 1.0, 0.0, 0.0},
    { 0.0, 0.0, 1.0},
    { -1.0, 0.0,0.0},
    {0.0, 0.0, -1.0},
	{ 1.0, 0.0, 0.0},
	{ -1.0,0.0, 0.0},
};

vec3_t cube_bmodel_normals[] =
{
	{ 0.0, 0.0, 1.0},
    { 1.0, 0.0, 0.0},
    { 0.0, 0.0,-1.0},
    {-1.0, 0.0, 0.0},
	{ 0.0, 1.0, 0.0},
	{ 0.0,-1.0, 0.0},
};

vec2_t cube_bmodel_tex_coords[] =
{
	{0.0, 1.0},
	{0.0, 0.0},
	{1.0, 0.0},
	{1.0, 1.0},

	{0.0, 1.0},
	{0.0, 0.0},
	{1.0, 0.0},
	{1.0, 1.0},

	{0.0, 1.0},
	{0.0, 0.0},
	{1.0, 0.0},
	{1.0, 1.0},

	{0.0, 1.0},
	{0.0, 0.0},
	{1.0, 0.0},
	{1.0, 1.0},

	{0.0, 1.0},
	{0.0, 0.0},
	{1.0, 0.0},
	{1.0, 1.0},

	{0.0, 1.0},
	{0.0, 0.0},
	{1.0, 0.0},
	{1.0, 1.0},
};



#define CUBE_BMODEL_SIZE sizeof(float) * 6 * 36
#define CUBE_BMODEL_VERTEX_COUNT 36

#define CUBE_BMODEL_COLLISION_VERTEX_COUNT 24
#define CUBE_BMODEL_COLLISION_NORMAL_COUNT 6





//int brush_list_size;
//int free_position_stack_top = -1;
//int *free_position_stack = NULL;
//int brush_count;
//brush_t *brushes;

int max_recently_moved_brush_index_count = 0;
int recently_moved_brush_index_count = 0;
int *recently_moved_brush_indexes = NULL;


int expanded_brush_count;
brush_t *expanded_brushes = NULL;

static unsigned int element_buffer;

int brush_count = 0;
brush_t *brushes = NULL;
brush_t *last_brush = NULL;
int unique_brush_index = 0;


void brush_Init()
{
	//brush_list_size = 512;
	//brush_count = 0;
	//expanded_brush_count = 0;
	//brushes = memory_Malloc(sizeof(brush_t ) * brush_list_size, "brush_Init");
	//free_position_stack = memory_Malloc(sizeof(int) * brush_list_size, "brush_Init");

	glGenBuffers(1, &element_buffer);
}

void brush_Finish()
{
	while(brushes)
	{
		last_brush = brushes->next;
		brush_DestroyBrush(brushes);
		brushes = last_brush;
	}



	glDeleteBuffers(1, &element_buffer);
}

void brush_ProcessBrushes()
{
	int i;
	brush_CheckIntersecting();
}

brush_t *brush_CreateBrush(vec3_t position, mat3_t *orientation, vec3_t scale, short type, short b_subtractive)
{

	//printf("start of brush_CreateBrush\n");

	brush_t *brush = NULL;
	int brush_index = 0;
	int i = 0;
	int c = 0;
	int default_group = 0;
	float *vertex_positions = NULL;
	float *vertex_normals = NULL;
	int vertex_count = 0;
	int triangle_count = 0;
	int index_count = 0;
	int alloc_handle = -1;
	int polygon_count = 0;
	int material = -1;
	int *b = NULL;
	int *indexes = NULL;
	vertex_t *polygon_vertices = NULL;

	bsp_polygon_t *polygons = NULL;
	bsp_polygon_t *polygon = NULL;

	vec3_t p;
	vec3_t v;

	R_DBG_PUSH_FUNCTION_NAME();



	brush = brush_CreateEmptyBrush();

	brush->type = type;
	brush->position = position;
	brush->scale = scale;
	brush->orientation = *orientation;
	brush->bm_flags |= BRUSH_CLIP_POLYGONS;

	if(b_subtractive)
	{
		brush->bm_flags |= BRUSH_SUBTRACTIVE;
	}

	switch(type)
	{
		case BRUSH_CUBE:
		case BRUSH_CYLINDER:

			brush_AllocBaseVertices(brush, 24, NULL);
			brush_AllocBasePolygons(brush, 6);

			for(i = 0; i < 24; i++)
			{
				brush->base_polygons_vertices[i].position = cube_bmodel_verts[i];
				brush->base_polygons_vertices[i].normal = cube_bmodel_normals[i >> 2];
				brush->base_polygons_vertices[i].tex_coord = cube_bmodel_tex_coords[i];
			}

			for(i = 0; i < 6; i++)
			{
				brush_AddPolygonToBrush(brush, NULL, cube_bmodel_normals[i], 4, -1);
			}

			brush_LinkPolygonsToVertices(brush);
			bsp_TriangulatePolygonsIndexes(brush->base_polygons, &indexes, &index_count);
			model_CalculateTangentsIndexes(brush->base_polygons_vertices, indexes, index_count);
			memory_Free(indexes);
		break;


	}

	brush_FinalizeBrush(brush, 1);

	R_DBG_POP_FUNCTION_NAME();

	return brush;
}

brush_t *brush_CreateEmptyBrush()
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

	brush = memory_Malloc(sizeof(brush_t), "brush_CreateEmptyBrush");

	brush->next = NULL;
	brush->prev = NULL;

	brush->type = BRUSH_EMPTY;

	brush->base_polygons = NULL;
	brush->base_polygons_count = 0;
	brush->base_polygons_vertices = NULL;
	brush->base_polygons_vert_count = 0;

	brush->clipped_polygons_index_count = 0;
	brush->clipped_polygons = NULL;
	brush->clipped_polygons_indexes = NULL;
	brush->clipped_polygons_vert_count = 0;
	brush->clipped_polygon_count = 0;

	brush->bm_flags = BRUSH_MOVED;

	//brush->triangle_groups = NULL;
	//brush->triangle_group_count = 0;
	//brush->max_triangle_groups = 0;

	brush->batches = NULL;
	brush->max_batches = 0;
	brush->batch_count = 0;

	brush->max_vertexes = BRUSH_MAX_VERTICES;
	//brush->vertex_count = 0;
	//brush->vertices = NULL;

	brush->brush_bsp = NULL;

	//brush->brush_count = 0;
	//brush->brush_indexes = NULL;
	//brush->max_brush_indexes = 0;

	/*brush->intersections = NULL;
	brush->max_intersections = 0;
	brush->edges = NULL;*/

	brush->intersection_records = NULL;
	brush->last_intersection_record = NULL;
	//brush->first_subtractive = NULL;
	brush->freed_records = NULL;

	brush->handle = INVALID_GPU_ALLOC_HANDLE;
	brush->start = -1;
	brush->index_handle = INVALID_GPU_ALLOC_HANDLE;
	brush->index_start = -1;

	//brush->brush_index = unique_brush_index++;

	if(!brushes)
	{
		brushes = brush;
	}
	else
	{
		last_brush->next = brush;
		brush->prev = last_brush;
	}
	last_brush = brush;

	brush_count++;

	//return brush_index;
	return brush;
}


void brush_InitializeBrush(brush_t *brush, mat3_t *orientation, vec3_t position, vec3_t scale, int type, int vertice_count, int polygon_count)
{
	if(brush && vertice_count > 0 && polygon_count > 0)
	{
		brush->position = position;
		brush->orientation = *orientation;
		brush->scale = scale;
		brush->type = type;
		brush->bm_flags |= BRUSH_MOVED | BRUSH_CLIP_POLYGONS;

		brush_AllocBaseVertices(brush, vertice_count, NULL);
		brush_AllocBasePolygons(brush, polygon_count);
	}
}

void brush_FinalizeBrush(brush_t *brush, int transform_base_vertices)
{
	bsp_polygon_t *polygon;
	vec3_t p;
	vec3_t v;

	int i;
	int c;

	R_DBG_PUSH_FUNCTION_NAME();

	if(brush)
	{

		//brush_BuildBatches(brush);

		//brush->handle = gpu_AllocAlign(sizeof(vertex_t) * brush->max_vertexes, sizeof(vertex_t));
		//brush->handle = gpu_AllocVerticesAlign(sizeof(compact_vertex_t ) * brush->max_vertexes, sizeof(compact_vertex_t));
		//brush->start = gpu_GetAllocStart(brush->handle) / sizeof(compact_vertex_t);

		brush->handle = gpu_AllocVerticesAlign(sizeof(struct c_vertex_t ) * brush->max_vertexes, sizeof(struct c_vertex_t ));
		brush->start = gpu_GetAllocStart(brush->handle) / sizeof(struct c_vertex_t );

		brush->index_handle = gpu_AllocIndexesAlign(sizeof(int) * brush->max_vertexes, sizeof(int));
		brush->index_start = gpu_GetAllocStart(brush->index_handle) / sizeof(int);

		//glGenBuffers(1, &brush->element_buffer);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brush->element_buffer);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * brush->max_vertexes, NULL, GL_DYNAMIC_DRAW);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


		if(transform_base_vertices)
		{
			polygon = brush->base_polygons;

			while(polygon)
			{
				c = polygon->vert_count;

				for(i = 0; i < c; i++)
				{
					p.x = polygon->vertices[i].position.x * brush->scale.x;
					p.y = polygon->vertices[i].position.y * brush->scale.y;
					p.z = polygon->vertices[i].position.z * brush->scale.z;


					v.x = p.x * brush->orientation.floats[0][0] +
					  	  p.y * brush->orientation.floats[1][0] +
					  	  p.z * brush->orientation.floats[2][0] + brush->position.x;

					v.y = p.x * brush->orientation.floats[0][1] +
						  p.y * brush->orientation.floats[1][1] +
						  p.z * brush->orientation.floats[2][1] + brush->position.y;

					v.z = p.x * brush->orientation.floats[0][2] +
						  p.y * brush->orientation.floats[1][2] +
						  p.z * brush->orientation.floats[2][2] + brush->position.z;


					polygon->vertices[i].position = v;

					p.x = polygon->vertices[i].normal.x;
					p.y = polygon->vertices[i].normal.y;
					p.z = polygon->vertices[i].normal.z;


					v.x = p.x * brush->orientation.floats[0][0] +
						  p.y * brush->orientation.floats[1][0] +
						  p.z * brush->orientation.floats[2][0];

					v.y = p.x * brush->orientation.floats[0][1] +
						  p.y * brush->orientation.floats[1][1] +
						  p.z * brush->orientation.floats[2][1];

					v.z = p.x * brush->orientation.floats[0][2] +
						  p.y * brush->orientation.floats[1][2] +
						  p.z * brush->orientation.floats[2][2];

					polygon->vertices[i].normal = v;

					p.x = polygon->vertices[i].tangent.x;
					p.y = polygon->vertices[i].tangent.y;
					p.z = polygon->vertices[i].tangent.z;


					v.x = p.x * brush->orientation.floats[0][0] +
						  p.y * brush->orientation.floats[1][0] +
						  p.z * brush->orientation.floats[2][0];

					v.y = p.x * brush->orientation.floats[0][1] +
						  p.y * brush->orientation.floats[1][1] +
						  p.z * brush->orientation.floats[2][1];

					v.z = p.x * brush->orientation.floats[0][2] +
						  p.y * brush->orientation.floats[1][2] +
						  p.z * brush->orientation.floats[2][2];


					polygon->vertices[i].tangent = v;

				}


				p.x = polygon->normal.x;
				p.y = polygon->normal.y;
				p.z = polygon->normal.z;


				v.x = p.x * brush->orientation.floats[0][0] +
					  p.y * brush->orientation.floats[1][0] +
					  p.z * brush->orientation.floats[2][0];

				v.y = p.x * brush->orientation.floats[0][1] +
					  p.y * brush->orientation.floats[1][1] +
					  p.z * brush->orientation.floats[2][1];

				v.z = p.x * brush->orientation.floats[0][2] +
					  p.y * brush->orientation.floats[1][2] +
					  p.z * brush->orientation.floats[2][2];


				polygon->normal = v;


				polygon = polygon->next;
			}
		}






		brush_BuildBrushBsp(brush);
		brush_BuildEdgeList(brush);
	}

	R_DBG_POP_FUNCTION_NAME();

}

void brush_AllocBaseVertices(brush_t *brush, int vert_count, vertex_t *vertices)
{
	int i;
	if(brush && vert_count > 0)
	{
		if(brush->base_polygons_vertices)
		{
			if(brush->base_polygons_vert_count >= vert_count)
			{
				/* no need to alloc less or equal than this brush already has... */
				return;
			}
		}

		brush->base_polygons_vert_count = vert_count;
		brush->base_polygons_vertices = memory_Malloc(sizeof(vertex_t) * brush->base_polygons_vert_count, "brush_AllocBaseVertices");

		if(vertices)
		{
			for(i = 0; i < vert_count; i++)
			{
				brush->base_polygons_vertices[i] = vertices[i];
			}
		}
	}
}



void brush_AllocBasePolygons(brush_t *brush, int polygon_count)
{
	int i;

	if(brush && polygon_count > 0)
	{
		if(brush->base_polygons)
		{
			if(brush->base_polygons_count >= polygon_count)
			{
				return;
			}
		}

		brush->base_polygons_count = 0;
		brush->base_polygons = memory_Malloc(sizeof(bsp_polygon_t) * polygon_count, "brush_AllocBasePolygons");

		for(i = 0; i < polygon_count; i++)
		{
			brush->base_polygons[i].next = brush->base_polygons + i + 1;
		}

		brush->base_polygons[i - 1].next = NULL;
	}
}


void brush_AddPolygonToBrush(brush_t *brush, vertex_t *polygon_vertices, vec3_t normal, int polygon_vertice_count, int material_index)
{
	int i;
	bsp_polygon_t *polygon;

	if(brush && polygon_vertice_count > 0)
	{
		/* this function won't do any sort of bound checking. It
		will assume a big enough buffer was allocated to contain
		the polygons and it's vertices... */

		polygon = brush->base_polygons + brush->base_polygons_count;
		brush->base_polygons_count++;

		polygon->vertices = polygon_vertices;
		polygon->b_used = 0;
		polygon->indexes = NULL;
		polygon->material_index = material_index;
		polygon->vert_count = polygon_vertice_count;
		polygon->normal = normal;
	}
}

/* necessary when no vertex data was passed while adding polygons to a brush... */
void brush_LinkPolygonsToVertices(brush_t *brush)
{
	int i = 0;
	int first_vertice_offset = 0;
	if(brush)
	{
		for(i = 0; i < brush->base_polygons_count; i++)
		{
			brush->base_polygons[i].vertices = brush->base_polygons_vertices + first_vertice_offset;
			first_vertice_offset += brush->base_polygons[i].vert_count;
		}
	}
}



void brush_IncBrushMaterialsRefCount(brush_t *brush)
{
	int i;
	//triangle_group_t *group;
	struct batch_t *batch;
	if(brush)
	{
		if(brush->type != BRUSH_INVALID)
		{
			//if(brush->triangle_groups)
			if(brush->batches)
			{
				//for(i = 0; i < brush->triangle_group_count; i++)
				for(i = 0; i < brush->batch_count; i++)
				{
					//material_IncRefCount(brush->triangle_groups[i].material_index);
					material_IncRefCount(brush->batches[i].material_index);
				}
			}
		}
	}
}

void brush_DecBrushMaterialsRefCount(brush_t *brush)
{
	int i;
	//triangle_group_t *group;
	struct batch_t *batch;
	if(brush)
	{
		if(brush->type != BRUSH_INVALID)
		{
			//if(brush->triangle_groups)
			if(brush->batches)
			{
				//for(i = 0; i < brush->triangle_group_count; i++)
				for(i = 0; i < brush->batch_count; i++)
				{
					material_DecRefCount(brush->batches[i].material_index);
				}
			}
		}
	}
}



brush_t *brush_CopyBrush(brush_t *src)
{

	//printf("start of brush_CreateBrush\n");

	brush_t *brush = NULL;
	int brush_index = 0;
	int i = 0;
	int c = 0;
	int default_group = 0;
	float *vertex_positions = NULL;
	float *vertex_normals = NULL;
	int vertex_count = 0;
	int triangle_count = 0;
	int index_count = 0;
	int alloc_handle = -1;
	int polygon_count = 0;
	int material = -1;
	int *b = NULL;
	int *indexes = NULL;
	vertex_t *polygon_vertices = NULL;

	bsp_polygon_t *polygons = NULL;
	bsp_polygon_t *polygon = NULL;

	vec3_t p;
	vec3_t v;


	R_DBG_PUSH_FUNCTION_NAME();


	brush = brush_CreateEmptyBrush();

	brush->type = src->type;
	brush->position = src->position;
	brush->scale = src->scale;
	brush->orientation = src->orientation;
	brush->bm_flags = src->bm_flags | BRUSH_CLIP_POLYGONS;

	/*if(src->bm_flags & BRUSH_SUBTRACTIVE)
	{
		brush->bm_flags |= BRUSH_SUBTRACTIVE;
	}*/

	/*switch(type)
	{
		case BRUSH_CUBE:
		case BRUSH_CYLINDER:*/

	brush_AllocBaseVertices(brush, src->base_polygons_vert_count, NULL);
	brush_AllocBasePolygons(brush, src->base_polygons_count);


	for(i = 0; i < src->base_polygons_vert_count; i++)
	{
		brush->base_polygons_vertices[i] = src->base_polygons_vertices[i];
	}

	for(i = 0; i < src->base_polygons_count; i++)
	{
		brush_AddPolygonToBrush(brush, NULL, src->base_polygons[i].normal, src->base_polygons[i].vert_count, src->base_polygons[i].material_index);
	}


			/*for(i = 0; i < 24; i++)
			{
				brush->base_polygons_vertices[i].position = cube_bmodel_verts[i];
				brush->base_polygons_vertices[i].normal = cube_bmodel_normals[i >> 2];
				brush->base_polygons_vertices[i].tex_coord = cube_bmodel_tex_coords[i];
			}

			for(i = 0; i < 6; i++)
			{
				brush_AddPolygonToBrush(brush, NULL, cube_bmodel_normals[i], 4, -1);
			}*/




	brush_LinkPolygonsToVertices(brush);
	bsp_TriangulatePolygonsIndexes(brush->base_polygons, &indexes, &index_count);
	model_CalculateTangentsIndexes(brush->base_polygons_vertices, indexes, index_count);
	memory_Free(indexes);
		//break;


	//}

	brush_FinalizeBrush(brush, 0);

	R_DBG_POP_FUNCTION_NAME();

	return brush;
}

void brush_DestroyBrush(brush_t *brush)
{
	bsp_polygon_t *polygon;
	bsp_polygon_t *next;
	bsp_edge_t *edge;
	bsp_edge_t *next_edge;
	brush_t *other;
	intersection_record_t *intersection_record;
	intersection_record_t *next_intersection_record;
	int brush_index;

	if(brush->type == BRUSH_INVALID)
		return;


	R_DBG_PUSH_FUNCTION_NAME();

	if(brush->type != BRUSH_BOUNDS)
	{

		bsp_DeleteSolidBsp(brush->brush_bsp, 0);

		//if(brush->triangle_groups)
		if(brush->batches)
		{
			brush_DecBrushMaterialsRefCount(brush);
		}

		memory_Free(brush->base_polygons_vertices);
		memory_Free(brush->batches);
		memory_Free(brush->base_polygons);
		memory_Free(brush->clipped_polygons_indexes);

		edge = brush->edges;

		while(edge)
		{
			next_edge = edge->next;
			memory_Free(edge);
			edge = next_edge;
		}

		gpu_Free(brush->handle);
		glDeleteBuffers(1, &brush->element_buffer);
	}

	intersection_record = brush->intersection_records;

	while(intersection_record)
	{
		next_intersection_record = intersection_record->next;
		brush_RemoveIntersectionRecord(intersection_record->intersecting_brush, brush, 1);
		intersection_record = next_intersection_record;
	}


	if(brush == brushes)
	{
		brushes = brushes->next;
		if(brushes)
		{
			brushes->prev = NULL;
		}
	}
	else
	{
		brush->prev->next = brush->next;

		if(brush->next)
		{
			brush->next->prev = brush->prev;
		}
		else
		{
			last_brush = last_brush->prev;
		}
	}

	brush_count--;

	memory_Free(brush);

	R_DBG_POP_FUNCTION_NAME();

	//brush_index = brush - brushes;

	//free_position_stack_top++;

	//free_position_stack[free_position_stack_top] = brush_index;
}

void brush_DestroyBrushIndex(int brush_index)
{
	//brush_DestroyBrush(&brushes[brush_index]);
}

void brush_DestroyAllBrushes()
{
	int i;


	/*for(i = 0; i < brush_count; i++)
	{
		brush_DestroyBrush(&brushes[i]);
	}*/

	brush_count = 0;
	//free_position_stack_top = -1;
}


void brush_CreateCylinder(int base_vertexes, int *vert_count, float **vertices, float **normals)
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
	*vertices = (float *)memory_Malloc(sizeof(float) * 3 * (*vert_count), "brush_CreateCylinder");
	*normals = (float *)memory_Malloc(sizeof(float) * 3 * (*vert_count), "brush_CreateCylinder");

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





#if 0
void brush_UpdateBrushElementBuffer(brush_t *brush)
{
	int i;
	int c;
	int j;
	int k;
	int index_start;
	//int triangle_group;
	int batch;
	int index_current;
	int first_index;
	int *b;
	bsp_polygon_t *polygon;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brush->element_buffer);
	b = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

	first_index = 0;
	c = brush->batch_count;

	for(i = 0; i < c; i++)
	{
		brush->batches[i].next = 0;
	}

	c = brush->clipped_polygon_count;

	for(i = 0; i < c; i++)
	{
		polygon = brush->clipped_polygons + i;

		batch = polygon->triangle_group;
		index_current = brush->batches[batch].next;

		k = (polygon->vert_count - 2) * 3;

		for(j = 0; j < k; j++)
		{
			b[index_start + index_current + j] = brush->clipped_polygons_indexes[first_index + j] + brush->start;
		}

		brush->batches[batch].next += j;
		first_index += j;
	}

	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
#endif

void brush_UploadBrushVertices(brush_t *brush)
{
	struct c_vertex_t *compact_vertices;
	struct vertex_t *vertices;

	int i;

	//int last_vert_count = gpu_GetAllocSize(brush.draw_data->handle) / sizeof(vertex_t);
	//int size = sizeof(vertex_t) * brush.draw_data->vert_count;
	//int size = brush.draw_data->vert_count * 6 * sizeof(float);
	/*if(brush.draw_data->vert_count > last_vert_count)
	{
		gpu_Free(brush.draw_data->handle);
		brush.draw_data->handle = gpu_Alloc(size);
		brush.draw_data->start = gpu_GetAllocStart(brush.draw_data->handle);
	}*/

	R_DBG_PUSH_FUNCTION_NAME();

	vertices = memory_Malloc(sizeof(vertex_t) * brush->clipped_polygons_vert_count, "brush_UploadBrushVertices");
	memcpy(vertices, brush->clipped_polygons_vertices, sizeof(vertex_t) * brush->clipped_polygons_vert_count);

	for(i = 0; i < brush->clipped_polygons_vert_count; i++)
	{
        vertices[i].position.x -= brush->position.x;
		vertices[i].position.y -= brush->position.y;
		vertices[i].position.z -= brush->position.z;
	}


	compact_vertices = model_ConvertVertices2(vertices, brush->clipped_polygons_vert_count);

	//gpu_Write(brush->handle, 0, brush->clipped_polygons_vertices, sizeof(vertex_t) * brush->clipped_polygons_vert_count);
	gpu_Write(brush->handle, 0, compact_vertices, sizeof(struct c_vertex_t ) * brush->clipped_polygons_vert_count);

	memory_Free(compact_vertices);
	memory_Free(vertices);

	R_DBG_POP_FUNCTION_NAME();
	//free(compact_vertices);
	//brush->bm_flags |= BRUSH_CLIP_POLYGONS;
}


void brush_UploadBrushIndexes(brush_t *brush)
{
	int i;
	int c;
	int j;
	int k;
	int index_start;
	//int triangle_group;
	int batch;
	int index_current;
	int first_index;
	int *b;
	bsp_polygon_t *polygon;

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brush->element_buffer);
	//b = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

	R_DBG_PUSH_FUNCTION_NAME();

	b = (int *)gpu_MapAlloc(brush->index_handle, GL_WRITE_ONLY) + brush->index_start;

	first_index = 0;
	c = brush->batch_count;

	for(i = 0; i < c; i++)
	{
		brush->batches[i].next = 0;
	}

	c = brush->clipped_polygon_count;

	for(i = 0; i < c; i++)
	{
		polygon = brush->clipped_polygons + i;

		batch = polygon->triangle_group;
		index_start = brush->batches[batch].start;
		index_current = brush->batches[batch].next;

		k = (polygon->vert_count - 2) * 3;

		for(j = 0; j < k; j++)
		{
			b[index_start + index_current + j] = brush->clipped_polygons_indexes[first_index + j] + brush->start;
		}

		brush->batches[batch].next += k;
		first_index += k;
	}

	gpu_UnmapAlloc(brush->index_handle);

	R_DBG_POP_FUNCTION_NAME();
	//glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void brush_BuildBrushBsp(brush_t *brush)
{
	int i;

	if(brush)
	{
		if(brush->brush_bsp)
			bsp_DeleteSolidBsp(brush->brush_bsp, 0);

		brush->brush_bsp = bsp_SolidBsp(brush->base_polygons);

		for(i = 0; i < brush->base_polygons_count; i++)
		{
			brush->base_polygons[i].next = brush->base_polygons + i + 1;
		}

		brush->base_polygons[i - 1].next = NULL;
	}
}


void brush_BuildEdgeList(brush_t *brush)
{
	int i;
	int c;

	int j;
	int k;

	int edge_count = 0;

	vec3_t p0;
	vec3_t p1;

	vec3_t r0;
	vec3_t r1;

	bsp_polygon_t *polygon_list = brush->base_polygons;
	bsp_polygon_t *p;
	bsp_polygon_t *r;

	bsp_edge_t *edge_list = NULL;
	bsp_edge_t *edge;
	bsp_edge_t *check_edge;

	//polygon_list = bsp_BuildPolygonsFromBrush(brush);


	p = polygon_list;


	while(p)
	{

		c = p->vert_count;

		r = polygon_list;

		while(r)
		{
			if(r == p)
			{
				r = r->next;
				continue;
			}

			for(i = 0; i < c; i++)
			{
				p0 = p->vertices[i].position;
				p1 = p->vertices[(i + 1) % c].position;

				k = r->vert_count;

				for(j = 0; j < k; j++)
				{

					r0 = r->vertices[j].position;
					r1 = r->vertices[(j + 1) % k].position;

					if(((p0.x == r0.x && p0.y == r0.y && p0.z == r0.z) && (p1.x == r1.x && p1.y == r1.y && p1.z == r1.z)) ||
					   ((p0.x == r1.x && p0.y == r1.y && p0.z == r1.z) && (p1.x == r0.x && p1.y == r0.y && p1.z == r0.z)) )
					{

						check_edge = edge_list;

						while(check_edge)
						{

							if((check_edge->polygon0 == p && check_edge->polygon1 == r) ||
							   (check_edge->polygon1 == p && check_edge->polygon0 == r ))
							{
								/* There's already an edge that link those two polygons... */
								break;
							}
							check_edge = check_edge->next;
						}

						/* this isn't a duplicate edge, so
						add it to the list... */
						if(!check_edge)
						{
							edge = memory_Malloc(sizeof(bsp_edge_t), "brush_BuildEdgeList");
							edge->v0 = p0;
							edge->v1 = p1;
							edge->v0_p0 = i;
							edge->v0_p1 = (i + 1) % c;
							edge->v1_p0 = j;
							edge->v1_p1 = (j + 1) % k;

							edge->polygon0 = p;
							edge->polygon1 = r;

							edge->dot = dot3(p->normal, r->normal);

							edge->next = edge_list;
							edge_list = edge;

							edge_count++;
						}

					}
				}
			}

			r = r->next;
		}

		p = p->next;
	}

	brush->edges = edge_list;
}

void brush_BuildBatches(brush_t *brush)
{
	int i;
	//int c = brush->vertex_count / 3;
	int c = brush->clipped_polygon_count;
	bsp_polygon_t *polygon;
	int j;
	int k;

	//int triangle_group_count = 0;
	//triangle_group_t triangle_groups[64];
	//triangle_group_t *cur_group = NULL;

	int batch_count = 0;
	struct batch_t batches[64];
	struct batch_t *cur_batch = NULL;

	for(i = 0; i < c; i++)
	{
		polygon = &brush->clipped_polygons[i];

		/* go over the list of materials found so far... */
		for(j = 0; j < batch_count; j++)
		{
			/* the material this polygon uses was seen before... */
			//if(polygon->material_index == triangle_groups[j].material_index)
			if(polygon->material_index == batches[j].material_index)
			{
				break;
			}
		}

		//if(j < triangle_group_count)
		if(j < batch_count)
		{
			//cur_group = triangle_groups + j;
			//cur_group->next += (polygon->vert_count - 2) * 3;
			cur_batch = batches + j;
			cur_batch->next += (polygon->vert_count - 2) * 3;
		}
		else
		{
			/* first time seen... */
			//cur_group = triangle_groups + triangle_group_count;
			//cur_group->material_index = polygon->material_index;
			//cur_group->start = 0;

			//triangle_group_count++;
			//cur_group->next = (polygon->vert_count - 2) * 3;

			cur_batch = batches + batch_count;
			cur_batch->material_index = polygon->material_index;
			cur_batch->start = 0;
			cur_batch->next = (polygon->vert_count - 2) * 3;

			batch_count++;

		}

		polygon->triangle_group = j;

		//printf("face %d uses material %d and is in triangle group %d\n", i, polygon->material_index, polygon->triangle_group);

	}

	/*for(i = 1; i < triangle_group_count; i++)
	{
		triangle_groups[i].start = triangle_groups[i - 1].start + triangle_groups[i - 1].next;
	}*/

	for(i = 1; i < batch_count; i++)
	{
		batches[i].start = batches[i - 1].start + batches[i - 1].next;
	}

	/*if(brush->triangle_groups)
	{
		brush_DecBrushMaterialsRefCount(brush);
	}*/
	if(brush->batches)
	{
		brush_DecBrushMaterialsRefCount(brush);
	}

	//if(triangle_group_count > brush->max_triangle_groups)
	if(batch_count > brush->max_batches)
	{
		//if(brush->triangle_groups)
		if(brush->batches)
		{
			memory_Free(brush->batches);
		}


		//brush->max_triangle_groups += brush->clipped_polygon_count;
		//brush->triangle_groups = malloc(sizeof(triangle_group_t) * brush->max_triangle_groups);
		brush->max_batches += brush->clipped_polygon_count;
		brush->batches = memory_Malloc(sizeof(struct batch_t) * brush->max_batches, "brush_BuildBatches");
	}

	//memcpy(brush->triangle_groups, triangle_groups, sizeof(triangle_group_t) * triangle_group_count);
	//brush->triangle_group_count = triangle_group_count;
	memcpy(brush->batches, batches, sizeof(struct batch_t) * batch_count);
	brush->batch_count = batch_count;

	brush_IncBrushMaterialsRefCount(brush);
}

bsp_polygon_t *brush_BuildPolygonListFromBrushes()
{
	brush_t *brush;
	bsp_polygon_t *polygons = NULL;
	bsp_polygon_t *polygon = NULL;
	bsp_polygon_t *last_polygon = NULL;

	brush = brushes;

	while(brush)
	{
		if(brush->bm_flags & BRUSH_SUBTRACTIVE)
		{
			brush = brush->next;
			continue;
		}
		polygon = bsp_DeepCopyPolygons(brush->clipped_polygons);

		if(polygon)
		{
			last_polygon = polygon;
			while(last_polygon->next) last_polygon = last_polygon->next;
			last_polygon->next = polygons;
			polygons = polygon;
		}

		brush->bm_flags |= BRUSH_INVISIBLE;
		brush = brush->next;
	}

	return polygon;
}







void brush_TranslateBrush(brush_t *brush, vec3_t direction)
{
	int i;
	int c = brush->base_polygons_vert_count;

	bsp_polygon_t *polygon;

	for(i = 0; i < c; i++)
	{
		brush->base_polygons_vertices[i].position.x += direction.x;
		brush->base_polygons_vertices[i].position.y += direction.y;
		brush->base_polygons_vertices[i].position.z += direction.z;
	}

	brush->position.x += direction.x;
	brush->position.y += direction.y;
	brush->position.z += direction.z;

	brush->bm_flags |= BRUSH_MOVED | BRUSH_CLIP_POLYGONS;

	//brush_UploadBrushVertices(brush);
}

void brush_RotateBrush(brush_t *brush, vec3_t axis, float amount)
{
	int i;
	int c = brush->base_polygons_vert_count;
	vec3_t v;
	vec3_t p;
	vec3_t n;
	vec3_t t;

	mat3_t rotation;
	mat3_t old_rotation;

	bsp_polygon_t *polygon;


	mat3_t_rotate(&rotation, axis, amount, 1);

	memcpy(&old_rotation.floats[0][0], &brush->orientation.floats[0][0], sizeof(mat3_t));

	mat3_t_mult(&brush->orientation, &old_rotation, &rotation);

	for(i = 0; i < c; i++)
	{

		v.x = brush->base_polygons_vertices[i].position.x;
		v.y = brush->base_polygons_vertices[i].position.y;
		v.z = brush->base_polygons_vertices[i].position.z;

		n.x = brush->base_polygons_vertices[i].normal.x;
		n.y = brush->base_polygons_vertices[i].normal.y;
		n.z = brush->base_polygons_vertices[i].normal.z;

		t.x = brush->base_polygons_vertices[i].tangent.x;
		t.y = brush->base_polygons_vertices[i].tangent.y;
		t.z = brush->base_polygons_vertices[i].tangent.z;

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


		brush->base_polygons_vertices[i].position.x = p.x + brush->position.x;
		brush->base_polygons_vertices[i].position.y = p.y + brush->position.y;
		brush->base_polygons_vertices[i].position.z = p.z + brush->position.z;



		p.x = n.x * rotation.floats[0][0] +
		      n.y * rotation.floats[1][0] +
		      n.z * rotation.floats[2][0];

		p.y = n.x * rotation.floats[0][1] +
		      n.y * rotation.floats[1][1] +
		      n.z * rotation.floats[2][1];

		p.z = n.x * rotation.floats[0][2] +
		      n.y * rotation.floats[1][2] +
		      n.z * rotation.floats[2][2];

		brush->base_polygons_vertices[i].normal.x = p.x;
		brush->base_polygons_vertices[i].normal.y = p.y;
		brush->base_polygons_vertices[i].normal.z = p.z;




		p.x = t.x * rotation.floats[0][0] +
		      t.y * rotation.floats[1][0] +
		      t.z * rotation.floats[2][0];

		p.y = t.x * rotation.floats[0][1] +
		      t.y * rotation.floats[1][1] +
		      t.z * rotation.floats[2][1];

		p.z = t.x * rotation.floats[0][2] +
		      t.y * rotation.floats[1][2] +
		      t.z * rotation.floats[2][2];

		brush->base_polygons_vertices[i].tangent.x = p.x;
		brush->base_polygons_vertices[i].tangent.y = p.y;
		brush->base_polygons_vertices[i].tangent.z = p.z;

	}


	polygon = brush->base_polygons;

	while(polygon)
	{

		n = polygon->normal;

		p.x = n.x * rotation.floats[0][0] +
		      n.y * rotation.floats[1][0] +
		      n.z * rotation.floats[2][0];

		p.y = n.x * rotation.floats[0][1] +
		      n.y * rotation.floats[1][1] +
		      n.z * rotation.floats[2][1];

		p.z = n.x * rotation.floats[0][2] +
		      n.y * rotation.floats[1][2] +
		      n.z * rotation.floats[2][2];

		polygon->normal = p;

		polygon = polygon->next;
	}


	brush->bm_flags |= BRUSH_MOVED | BRUSH_CLIP_POLYGONS;
	//brush_UploadBrushVertices(brush);
}

void brush_ScaleBrush(brush_t *brush, vec3_t axis, float amount)
{
	int i;
	//int c = brush.draw_data->vert_count;

	int c = brush->base_polygons_vert_count;

	vec3_t prev_scale;
	vec3_t new_scale;
	vec3_t translation;
	vec3_t v;
	vec3_t p;

	bsp_polygon_t *polygon;

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
		new_scale.x = 0.025;
	}

	if(new_scale.y <= 0.025)
	{
		new_scale.y = 0.025;
	}

	if(new_scale.z <= 0.025)
	{
		new_scale.z = 0.025;
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

		v.x = brush->base_polygons_vertices[i].position.x - translation.x;
		v.y = brush->base_polygons_vertices[i].position.y - translation.y;
		v.z = brush->base_polygons_vertices[i].position.z - translation.z;

		v = MultiplyVector3(&inverse_rotation, v);

		v.x *= (new_scale.x / prev_scale.x);
		v.y *= (new_scale.y / prev_scale.y);
		v.z *= (new_scale.z / prev_scale.z);

		v = MultiplyVector3(&brush->orientation, v);

		brush->base_polygons_vertices[i].position.x = v.x + translation.x;
		brush->base_polygons_vertices[i].position.y = v.y + translation.y;
		brush->base_polygons_vertices[i].position.z = v.z + translation.z;

	}

	brush->bm_flags |= BRUSH_MOVED | BRUSH_CLIP_POLYGONS;

	//brush_UploadBrushVertices(brush);
}


void brush_SetFaceMaterial(brush_t *brush, int face_index, int material_index)
{

	/* material_index better be a valid material here >:( ... */

	if(brush)
	{
		if(brush->type != BRUSH_INVALID)
		{
			if(face_index >= 0 && face_index < brush->base_polygons_count)
			{
				brush->base_polygons[face_index].material_index = material_index;

				//brush_BuildTriangleGroups(brush);
				//brush_UpdateBrushElementBuffer(brush);
				brush->bm_flags |= BRUSH_CLIP_POLYGONS;
			}
		}
	}
}


void brush_SetAllVisible()
{
	int i;
	for(i = 0; i < brush_count; i++)
	{
		if(brushes[i].type == BRUSH_INVALID)
			continue;

		brushes[i].bm_flags &= ~BRUSH_INVISIBLE;
	}
}

void brush_SetAllInvisible()
{
	int i;
	for(i = 0; i < brush_count; i++)
	{
		if(brushes[i].type == BRUSH_INVALID)
			continue;

		brushes[i].bm_flags |= BRUSH_INVISIBLE;
	}
}





void brush_CheckIntersecting()
{
	int i;
	int j;
	int k;
	int c;

	int first_clip;
	bsp_polygon_t *polygon;
	brush_t *brush;
	brush_t *brush2;
	intersection_record_t *record;

	brush = brushes;

	while(brush)
	{

		if(brush->bm_flags & BRUSH_SUBTRACTIVE)
		{
			/* subtractive brushes won't have their polygons clipped.
			They just clip other brushes polygons... */
			brush = brush->next;
			continue;
		}


		if(brush->bm_flags & BRUSH_MOVED)
		{

			/* this brush has moved, so we need to check
			all other brushes in the world, regardless
			if they also have moved or not... */

			brush2 = brushes;
			while(brush2)
			{
				if(brush2 == brush)
				{
					brush2 = brush2->next;
					continue;
				}

				if(brush_CheckBrushIntersection(brush, brush2))
				{
					brush_AddIntersectionRecord(brush, brush2);
				}
				else
				{
					brush_RemoveIntersectionRecord(brush, brush2, 0);
				}

				brush2 = brush2->next;
			}
		}
		else
		{
			/* this brush didn't move since last time we checked... */

			brush2 = brushes;
			while(brush2)
			{

				if(brush2 == brush)
				{
					brush2 = brush2->next;
					continue;
				}

				/* ... so we can avoid a rather costly intersection
				test if the brush we're checking against also didn't
				move... */
				if(!(brush2->bm_flags & BRUSH_MOVED))
				{
					brush2 = brush2->next;
					continue;
				}

				if(brush_CheckBrushIntersection(brush, brush2))
				{
					brush_AddIntersectionRecord(brush, brush2);
				}
				else
				{
					brush_RemoveIntersectionRecord(brush, brush2, 0);
				}

				brush2 = brush2->next;
			}
		}

		brush = brush->next;
	}

	brush = brushes;
	while(brush)
	{

		if(!(brush->bm_flags & BRUSH_CLIP_POLYGONS))
		{
			/* this brush didn't touch any other brush this check, so
			skip it... */
			brush = brush->next;
			continue;
		}


		if(brush->bm_flags & BRUSH_SUBTRACTIVE)
		{
			/* this brush is marked as subtractive,
			so we won't clip its polygons, but always
			alias its base polygons... */
			goto _is_subtractive_brush;
		}


		first_clip = 1;

		record = brush->intersection_records;

		while(record)
		{
			brush2 = record->intersecting_brush;
			if(first_clip)
			{
				if(brush->bm_flags & BRUSH_HAS_PREV_CLIPS)
				{
					/* this brush was touching another brush last time
					we checked, which means its clipped polygons are
					not an alias from its base polygons, and so we
					need to get rid of them here to avoid leaks... */
					bsp_DeletePolygonsContiguous(brush->clipped_polygons);
				}


				/* we're going to clip those polygons, so aliasing them is not enough... */
				brush->clipped_polygons = bsp_DeepCopyPolygonsContiguous(brush->base_polygons);
			}

			if(brush2->bm_flags & BRUSH_SUBTRACTIVE)
			{
				brush->clipped_polygons = bsp_ClipContiguousPolygonsToBsp(CSG_OP_SUBTRACTION, brush2->brush_bsp, brush->brush_bsp, brush->clipped_polygons, brush2->base_polygons, 0);
			}
			else
			{
				brush->clipped_polygons = bsp_ClipContiguousPolygonsToBsp(CSG_OP_UNION, brush2->brush_bsp, NULL, brush->clipped_polygons, NULL, 0);
			}

			first_clip = 0;

			/* this brush is inside another brush... */
			if(!brush->clipped_polygons)
			{
				break;
			}

			record = record->next;
		}


		if(first_clip)
		{
			/* this brush didn't intersect any other brush... */

			if(brush->bm_flags & BRUSH_HAS_PREV_CLIPS)
			{
				/* this brush was touching another brush last time
				we checked, which means it's clipped polygons are
				not an alias from its base polygons, and so we
				need to get rid of them here to avoid leaks... */

				bsp_DeletePolygonsContiguous(brush->clipped_polygons);
			}

			_is_subtractive_brush:

			/* this brush is not touching any other brush, so suffices to
			just alias its base polygons... */
			brush->clipped_polygons = brush->base_polygons;
			brush->bm_flags &= ~BRUSH_HAS_PREV_CLIPS;
		}
		else
		{
			if(brush->clipped_polygons)
			{
				/* this brush is touching someone else,
				so we flag its clipped polygons as not
				an alias from the base polygons, so they
				get properly freed next time we check... */

				brush->bm_flags |= BRUSH_HAS_PREV_CLIPS;
			}
			else
			{
				/* this brush is inside another, so it doesn't
				have any polygons to be freed next time we
				check... */
				brush->bm_flags &= ~BRUSH_HAS_PREV_CLIPS;
			}

		}

		brush->clipped_polygons_vert_count = 0;
		brush->clipped_polygon_count = 0;

		if(brush->clipped_polygons_indexes)
		{
			memory_Free(brush->clipped_polygons_indexes);
			brush->clipped_polygons_indexes = NULL;
			brush->clipped_polygons_index_count = 0;
		}

		if(brush->clipped_polygons)
		{
			brush->clipped_polygons_vertices = brush->clipped_polygons[0].vertices;
			polygon = brush->clipped_polygons;

			while(polygon)
			{
				brush->clipped_polygon_count++;
				brush->clipped_polygons_vert_count += polygon->vert_count;
				polygon = polygon->next;
			}

			bsp_TriangulatePolygonsIndexes(brush->clipped_polygons, &brush->clipped_polygons_indexes, &brush->clipped_polygons_index_count);
			brush_BuildBatches(brush);
			brush_UploadBrushVertices(brush);
			brush_UploadBrushIndexes(brush);
			//brush_UpdateBrushElementBuffer(brush);
		}

		brush->bm_flags &= ~(BRUSH_MOVED | BRUSH_CLIP_POLYGONS);

		brush = brush->next;
	}



}

int brush_CheckBrushIntersection(brush_t *a, brush_t *b)
{
	vec3_t v;
	int i;
	int j;
	int c;
	int k;
	bsp_polygon_t *polygon;

	c = a->base_polygons_count;


	for(i = 0; i < c; i++)
	{
		polygon = &a->base_polygons[i];
		k = polygon->vert_count;

		for(j = 0; j < k; j++)
		{
			if(bsp_IntersectBsp(b->brush_bsp, polygon->vertices[j].position, polygon->vertices[(j + 1) % k].position))
				return 1;
		}

	}

	c = b->base_polygons_count;

	for(i = 0; i < c; i++)
	{
		polygon = &b->base_polygons[i];
		k = polygon->vert_count;

		for(j = 0; j < k; j++)
		{
			if(bsp_IntersectBsp(a->brush_bsp, polygon->vertices[j].position, polygon->vertices[(j + 1) % k].position))
				return 1;
		}

	}

	return 0;
}

void brush_AddIntersectionRecord(brush_t *add_to, brush_t *to_add)
{
	intersection_record_t *record;
	intersection_record_t *records;

	if(add_to && to_add)
	{
		add_to->bm_flags |= BRUSH_CLIP_POLYGONS;

		if(brush_GetIntersectionRecord(add_to, to_add))
		{
			return;
		}

		if(add_to->freed_records)
		{
			record = add_to->freed_records;
			add_to->freed_records = record->next;
		}
		else
		{
			record = memory_Malloc(sizeof(intersection_record_t), "brush_AddIntersectionRecord");
		}

		record->intersecting_brush = to_add;
		record->next = NULL;
		record->prev = NULL;

		if(!add_to->intersection_records)
		{
			add_to->intersection_records = record;
		}
		else
		{
			if(add_to->last_intersection_record->intersecting_brush->bm_flags & BRUSH_SUBTRACTIVE)
			{
				records = add_to->last_intersection_record;

				while(records)
				{
					if(!(records->intersecting_brush->bm_flags & BRUSH_SUBTRACTIVE))
					{
						break;
					}

					records = records->prev;
				}

				if(records)
				{
					record->next = records->next;

					if(record->next)
					{
						record->next->prev = record;
					}

					records->next = record;
					record->prev = records;
				}
				else
				{
					record->next = add_to->intersection_records;
					record->next->prev = record;
					add_to->intersection_records = record;
				}

				return;

			}
			else
			{
				record->prev = add_to->last_intersection_record;
				add_to->last_intersection_record->next = record;
			}


		}

		add_to->last_intersection_record = record;
	}
}

void brush_RemoveIntersectionRecord(brush_t *remove_from, brush_t *to_remove, int free_record)
{
	intersection_record_t *record = NULL;


	if(remove_from && to_remove)
	{
		record = remove_from->intersection_records;

		while(record)
		{

			if(record->intersecting_brush == to_remove)
			{

				if(record == remove_from->intersection_records)
				{
					remove_from->intersection_records = record->next;
					if(remove_from->intersection_records)
					{
						remove_from->intersection_records->prev = NULL;
					}
				}
				else
				{
					record->prev->next = record->next;

					if(record->next)
					{
						record->next->prev = record->prev;
					}
					else
					{
						remove_from->last_intersection_record = record->prev;
					}
				}

				if(free_record)
				{
					memory_Free(record);
				}
				else
				{
					record->next = remove_from->freed_records;
					remove_from->freed_records = record;
				}

				remove_from->bm_flags |= BRUSH_CLIP_POLYGONS;
				break;
			}
			//prev_record = record;
			record = record->next;
		}

	}
}

intersection_record_t *brush_GetIntersectionRecord(brush_t *brush, brush_t *brush2)
{
	intersection_record_t *record = NULL;

	if(brush && brush2)
	{
		record = brush->intersection_records;

		while(record)
		{
			if(record->intersecting_brush == brush2)
			{
				break;
			}
			record = record->next;
		}

	}

	return record;
}



void brush_SerializeBrushes(void **buffer, int *buffer_size)
{
	brush_t *brush;
	brush_section_header_t *header;
	brush_record_t *brush_record;
	polygon_record_t *polygon_record;
	bsp_polygon_t *polygon;
	vertex_t *vertices;
	char *out;
	int size;
	int i;

	size = sizeof(brush_section_header_t) * sizeof(brush_record_t) * brush_count;


	brush = brushes;

	while(brush)
	{
		size += sizeof(polygon_record_t) * brush->base_polygons_count;
		polygon = brush->base_polygons;

		while(polygon)
		{
			size += sizeof(vertex_t) * polygon->vert_count;
			polygon = polygon->next;
		}

		brush = brush->next;
	}

	out = memory_Malloc(size, "brush_SerialzieBrushes");

	*buffer = out;
	*buffer_size = size;

	header = (brush_section_header_t *)out;
	out += sizeof(brush_section_header_t);

	header->brush_count = brush_count;
	header->reserved0 = 0;
	header->reserved1 = 0;
	header->reserved2 = 0;
	header->reserved3 = 0;
	header->reserved4 = 0;
	header->reserved5 = 0;
	header->reserved6 = 0;
	header->reserved7 = 0;

	brush = brushes;

	while(brush)
	{

		brush_record = (brush_record_t *)out;
		out += sizeof(brush_record_t);

		brush_record->bm_flags = brush->bm_flags;

		brush_record->orientation = brush->orientation;
		brush_record->position = brush->position;
		brush_record->scale = brush->scale;

		brush_record->type = brush->type;

		brush_record->polygon_count = brush->base_polygons_count;
		brush_record->vertex_count = brush->base_polygons_vert_count;

		brush_record->reserved0 = 0;
		brush_record->reserved1 = 0;
		brush_record->reserved2 = 0;
		brush_record->reserved3 = 0;
		brush_record->reserved4 = 0;
		brush_record->reserved5 = 0;
		brush_record->reserved6 = 0;
		brush_record->reserved7 = 0;


		polygon = brush->base_polygons;

		while(polygon)
		{
			polygon_record = (polygon_record_t *)out;
			out += sizeof(polygon_record_t);

			strcpy(polygon_record->material_name,  material_GetMaterialName(polygon->material_index));
			polygon_record->normal = polygon->normal;
			polygon_record->vert_count = polygon->vert_count;

			vertices = (vertex_t *)out;
			out += sizeof(vertex_t) * polygon->vert_count;

			for(i = 0; i < polygon->vert_count; i++)
			{
				vertices[i] = polygon->vertices[i];
			}

			polygon = polygon->next;
		}
		brush = brush->next;
	}
}

void brush_DeserializeBrushes(void **buffer)
{
	brush_section_header_t *header;
	brush_record_t *brush_record;
	polygon_record_t *polygon_record;
	vertex_t *vertices;
	brush_t *brush;
	int i;
	int j;
	int k;
	char *in;

	in = (char *)*buffer;


	header = (brush_section_header_t *)in;
	in += sizeof(brush_section_header_t );

	for(i = 0; i < header->brush_count; i++)
	{
		brush_record = (brush_record_t *)in;
		in += sizeof(brush_record_t);

		brush = brush_CreateEmptyBrush();
		brush_InitializeBrush(brush, &brush_record->orientation, brush_record->position, brush_record->scale, brush_record->type, brush_record->vertex_count, brush_record->polygon_count);

		for(j = 0; j < brush_record->polygon_count; j++)
		{
			polygon_record = (polygon_record_t *)in;
			in += sizeof(polygon_record_t);

			vertices = (vertex_t *)in;
			in += sizeof(vertex_t) * polygon_record->vert_count;

			brush_AddPolygonToBrush(brush, NULL, polygon_record->normal, polygon_record->vert_count, material_MaterialIndex(polygon_record->material_name));

			for(k = 0; k < polygon_record->vert_count; k++)
			{
				brush->base_polygons_vertices[k + brush->base_polygons_vert_count] = vertices[k];
			}

			brush->base_polygons_vert_count += polygon_record->vert_count;
		}

		brush_LinkPolygonsToVertices(brush);
	}

	*buffer = in;

}





