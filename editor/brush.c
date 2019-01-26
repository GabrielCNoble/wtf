#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GL\glew.h"

#include <x86intrin.h>

#include "brush.h"
#include "r_verts.h"
#include "vector.h"
#include "matrix.h"
#include "bsp_cmp.h"
#include "input.h"
#include "material.h"
#include "camera.h"
#include "c_memory.h"
#include "bsp_cmp.h"

#include "..\..\common\r_debug.h"

/*#define malloc ((void *)0)
#define calloc ((void *)0)
#define free ((void *)0)
#define strdup ((void *)0)*/




vertex_t cube_brush_vertices[] =
{
    /* +Z */
    {{-1.0, 1.0, 1.0}, { 0.0, 0.0, 1.0}, { 1.0, 0.0, 0.0}, {0.0, 1.0}},
    {{-1.0,-1.0, 1.0}, { 0.0, 0.0, 1.0}, { 1.0, 0.0, 0.0}, {0.0, 0.0}},
    {{ 1.0,-1.0, 1.0}, { 0.0, 0.0, 1.0}, { 1.0, 0.0, 0.0}, {1.0, 0.0}},
    {{ 1.0, 1.0, 1.0}, { 0.0, 0.0, 1.0}, { 1.0, 0.0, 0.0}, {1.0, 1.0}},


    /* +X */
    {{ 1.0, 1.0, 1.0}, { 1.0, 0.0, 0.0}, { 0.0, 0.0, 1.0}, {0.0, 1.0}},
	{{ 1.0,-1.0, 1.0}, { 1.0, 0.0, 0.0}, { 0.0, 0.0, 1.0}, {0.0, 0.0}},
	{{ 1.0,-1.0,-1.0}, { 1.0, 0.0, 0.0}, { 0.0, 0.0, 1.0}, {1.0, 0.0}},
	{{ 1.0, 1.0,-1.0}, { 1.0, 0.0, 0.0}, { 0.0, 0.0, 1.0}, {1.0, 1.0}},


    /* -Z */
	{{ 1.0, 1.0,-1.0}, { 0.0, 0.0,-1.0}, { -1.0, 0.0,0.0}, {0.0, 1.0}},
	{{ 1.0,-1.0,-1.0}, { 0.0, 0.0,-1.0}, { -1.0, 0.0,0.0}, {0.0, 0.0}},
	{{-1.0,-1.0,-1.0}, { 0.0, 0.0,-1.0}, { -1.0, 0.0,0.0}, {1.0, 0.0}},
	{{-1.0, 1.0,-1.0}, { 0.0, 0.0,-1.0}, { -1.0, 0.0,0.0}, {1.0, 1.0}},


    /* -X */
	{{-1.0, 1.0,-1.0}, {-1.0, 0.0, 0.0}, {0.0, 0.0, -1.0}, {0.0, 1.0}},
	{{-1.0,-1.0,-1.0}, {-1.0, 0.0, 0.0}, {0.0, 0.0, -1.0}, {0.0, 0.0}},
	{{-1.0,-1.0, 1.0}, {-1.0, 0.0, 0.0}, {0.0, 0.0, -1.0}, {1.0, 0.0}},
	{{-1.0, 1.0, 1.0}, {-1.0, 0.0, 0.0}, {0.0, 0.0, -1.0}, {1.0, 1.0}},


    /* +Y */
	{{-1.0, 1.0, 1.0}, { 0.0, 1.0, 0.0}, { 1.0, 0.0, 0.0}, {0.0, 1.0}},
	{{ 1.0, 1.0, 1.0}, { 0.0, 1.0, 0.0}, { 1.0, 0.0, 0.0}, {0.0, 0.0}},
	{{ 1.0, 1.0,-1.0}, { 0.0, 1.0, 0.0}, { 1.0, 0.0, 0.0}, {1.0, 0.0}},
	{{-1.0, 1.0,-1.0}, { 0.0, 1.0, 0.0}, { 1.0, 0.0, 0.0}, {1.0, 1.0}},


    /* -Y */
	{{-1.0,-1.0,-1.0}, { 0.0,-1.0, 0.0}, { -1.0,0.0, 0.0}, {0.0, 1.0}},
	{{ 1.0,-1.0,-1.0}, { 0.0,-1.0, 0.0}, { -1.0,0.0, 0.0}, {0.0, 0.0}},
	{{ 1.0,-1.0, 1.0}, { 0.0,-1.0, 0.0}, { -1.0,0.0, 0.0}, {1.0, 0.0}},
	{{-1.0,-1.0, 1.0}, { 0.0,-1.0, 0.0}, { -1.0,0.0, 0.0}, {1.0, 1.0}},
};


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

int process_brushes = 2;
SDL_Thread *brush_thread;
SDL_mutex *brush_delete_mutex;

static unsigned int element_buffer;

int brush_count = 0;
brush_t *brushes = NULL;
brush_t *last_brush = NULL;


brush_t *invalid_brushes = NULL;
brush_t *last_invalid_brushes = NULL;


int unique_brush_index = 0;

struct brush_t *last_updated_brush = NULL;
struct brush_t *last_updated_against = NULL;
int check_intersection = 2;


void brush_Init()
{
	//brush_list_size = 512;
	//brush_count = 0;
	//expanded_brush_count = 0;
	//brushes = memory_Malloc(sizeof(brush_t ) * brush_list_size, "brush_Init");
	//free_position_stack = memory_Malloc(sizeof(int) * brush_list_size, "brush_Init");

	glGenBuffers(1, &element_buffer);

	check_intersection = 2;

	//brush_thread = SDL_CreateThread(brush_ProcessBrushesAsync, "brush_ProcessBrushes", NULL);

	//brush_delete_mutex = SDL_CreateMutex();
}

void brush_Finish()
{

	process_brushes = 0;

	SDL_WaitThread(brush_thread, NULL);

	while(brushes)
	{
		last_brush = brushes->next;
		brush_DestroyBrush(brushes);
		brushes = last_brush;
	}



	glDeleteBuffers(1, &element_buffer);
}


#if 0

void brush_ProcessBrushes()
{
	int i;

	brush_UploadBrushes();
	//brush_UpdateTexCoords();
	//brush_ClipBrushes();
}

#endif

void brush_UpdateAllBrushes()
{
	last_updated_brush = NULL;
	last_updated_against = NULL;
	check_intersection = 1;
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

	brush = memory_Malloc(sizeof(brush_t));

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

	brush->bm_flags = BRUSH_MOVED | BRUSH_CLIP_POLYGONS;

	brush->brush_mutex = SDL_CreateMutex();

	brush->update_count = brush_count + 1;

	//brush->triangle_groups = NULL;
	//brush->triangle_group_count = 0;
	//brush->max_triangle_groups = 0;

	brush->batches = NULL;
	brush->max_batches = 0;
	brush->batch_count = 0;

	brush->max_vertexes = 0;
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

	SDL_LockMutex(brush->brush_mutex);

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


	check_intersection = 1;
	last_updated_against = NULL;
	last_updated_brush = NULL;

	brush_count++;

	//return brush_index;
	return brush;
}


brush_t *brush_CreateBrush(vec3_t position, mat3_t *orientation, vec3_t scale, short type, short b_subtractive)
{
	brush_t *brush = NULL;

	vertex_t *polygon_vertices;
	int i = 0;
	int c = 0;

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

			for(i = 0; i < 6; i++)
            {
                polygon_vertices = cube_brush_vertices + i * 4;
                brush_AddPolygonToBrush(brush, polygon_vertices, polygon_vertices->normal, vec2(1.0, 1.0), 4, -1);
            }
		break;
	}

	brush_FinalizeBrush(brush, 1);

	R_DBG_POP_FUNCTION_NAME();

	return brush;
}



brush_t *brush_CopyBrush(brush_t *src)
{
	brush_t *brush = NULL;
	int i = 0;
	bsp_polygon_t *polygon = NULL;


	R_DBG_PUSH_FUNCTION_NAME();


	brush = brush_CreateEmptyBrush();

	brush->type = src->type;
	brush->position = src->position;
	brush->scale = src->scale;
	brush->orientation = src->orientation;
	brush->bm_flags = src->bm_flags | BRUSH_CLIP_POLYGONS;

	brush_AllocBaseVertices(brush, src->base_polygons_vert_count, NULL);
	brush_AllocBasePolygons(brush, src->base_polygons_count);

	for(i = 0; i < src->base_polygons_count; i++)
    {
        polygon = src->base_polygons + i;
        brush_AddPolygonToBrush(brush, polygon->vertices, polygon->normal, polygon->tiling, polygon->vert_count, polygon->material_index);
    }

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
	SDL_mutex *mutex;
	intersection_record_t *intersection_record;
	intersection_record_t *next_intersection_record;
	int brush_index;

	if(brush->type == BRUSH_INVALID)
		return;


	R_DBG_PUSH_FUNCTION_NAME();

	//SDL_LockMutex(brush_delete_mutex);

	mutex = brush->brush_mutex;

	SDL_LockMutex(mutex);

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

		renderer_Free(brush->handle);
//		glDeleteBuffers(1, &brush->element_buffer);
	}

	intersection_record = brush->intersection_records;

	while(intersection_record)
	{
		next_intersection_record = intersection_record->next;
		brush_RemoveIntersectionRecord(intersection_record->intersecting_brush, brush, 1);
		intersection_record = next_intersection_record;
	}

	last_updated_brush = NULL;
	last_updated_against = NULL;
	check_intersection = 1;

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

	SDL_UnlockMutex(mutex);
	SDL_DestroyMutex(mutex);

	R_DBG_POP_FUNCTION_NAME();
}

void brush_DestroyBrushIndex(int brush_index)
{
	//brush_DestroyBrush(&brushes[brush_index]);
}

void brush_DestroyAllBrushes()
{
	int i;

	while(brushes)
	{
		last_brush = brushes->next;
		brush_DestroyBrush(brushes);
		brushes = last_brush;
	}

	brush_count = 0;
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

	int *indexes = NULL;
	int index_count = 0;

	int i;
	int c;

	R_DBG_PUSH_FUNCTION_NAME();

	if(brush)
	{
		bsp_TriangulatePolygonsIndexes(brush->base_polygons, &indexes, &index_count);
		model_CalculateTangentsIndexes(brush->base_polygons_vertices, indexes, index_count);
		memory_Free(indexes);

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

	SDL_UnlockMutex(brush->brush_mutex);

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
		brush->base_polygons_vertices = memory_Malloc(sizeof(vertex_t) * brush->base_polygons_vert_count);

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
		brush->base_polygons = memory_Malloc(sizeof(bsp_polygon_t) * polygon_count);

		for(i = 0; i < polygon_count; i++)
		{
			brush->base_polygons[i].next = brush->base_polygons + i + 1;
		}

		brush->base_polygons[i - 1].next = NULL;
	}
}


void brush_AddPolygonToBrush(brush_t *brush, vertex_t *polygon_vertices, vec3_t normal, vec2_t tiling, int polygon_vertice_count, int material_index)
{
	int i;
	bsp_polygon_t *polygon;
	bsp_polygon_t *prev_polygon;

	if(brush && polygon_vertice_count > 0)
	{
		/* this function won't do any sort of bound checking. It
		will assume a big enough buffer was allocated to contain
		the polygons and it's vertices... */

		polygon = brush->base_polygons + brush->base_polygons_count;


		//polygon->vertices = polygon_vertices;

		polygon->vertices = brush->base_polygons_vertices;

		if(brush->base_polygons_count)
		{
			prev_polygon = brush->base_polygons + brush->base_polygons_count - 1;
			polygon->vertices = prev_polygon->vertices + prev_polygon->vert_count;
		}

		if(polygon_vertices)
		{
			memcpy(polygon->vertices, polygon_vertices, sizeof(vertex_t) * polygon_vertice_count);
		}

		polygon->b_used = 0;
		polygon->indexes = NULL;
		polygon->material_index = material_index;
		polygon->vert_count = polygon_vertice_count;
		polygon->normal = normal;
		polygon->tiling = tiling;

		brush->base_polygons_count++;
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
	*vertices = (float *)memory_Malloc(sizeof(float) * 3 * (*vert_count));
	*normals = (float *)memory_Malloc(sizeof(float) * 3 * (*vert_count));

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
	//struct c_vertex_t *compact_vertices;
	struct compact_vertex_t *compact_vertices;
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


	if(brush->clipped_polygons_vert_count > renderer_GetAllocSize(brush->handle) / sizeof(struct compact_vertex_t))
	{
		if(renderer_IsAllocValid(brush->handle))
		{
			renderer_Free(brush->handle);
		}

		brush->handle = renderer_AllocVerticesAlign(sizeof(struct compact_vertex_t) * brush->clipped_polygons_vert_count, sizeof(struct compact_vertex_t));
		brush->start = renderer_GetAllocStart(brush->handle) / sizeof(struct compact_vertex_t);
	}





	vertices = memory_Malloc(sizeof(vertex_t) * brush->clipped_polygons_vert_count);
	memcpy(vertices, brush->clipped_polygons_vertices, sizeof(vertex_t) * brush->clipped_polygons_vert_count);

	for(i = 0; i < brush->clipped_polygons_vert_count; i++)
	{
        vertices[i].position.x -= brush->position.x;
		vertices[i].position.y -= brush->position.y;
		vertices[i].position.z -= brush->position.z;
	}


	//compact_vertices = model_ConvertVertices2(vertices, brush->clipped_polygons_vert_count);

	compact_vertices = model_ConvertVertices(vertices, brush->clipped_polygons_vert_count);

	//gpu_Write(brush->handle, 0, brush->clipped_polygons_vertices, sizeof(vertex_t) * brush->clipped_polygons_vert_count);
	//gpu_Write(brush->handle, 0, compact_vertices, sizeof(struct c_vertex_t ) * brush->clipped_polygons_vert_count);

	renderer_Write(brush->handle, 0, compact_vertices, sizeof(struct compact_vertex_t ) * brush->clipped_polygons_vert_count);

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



	if(brush->clipped_polygons_index_count > renderer_GetAllocSize(brush->index_handle) / sizeof(int))
	{
		if(renderer_IsAllocValid(brush->index_handle))
		{
			renderer_Free(brush->index_handle);
		}

		brush->index_handle = renderer_AllocIndexesAlign(sizeof(int) * brush->clipped_polygons_index_count, sizeof(int));
		brush->index_start = renderer_GetAllocStart(brush->index_handle) / sizeof(int);
	}




	b = (int *)renderer_MapAlloc(brush->index_handle, GL_WRITE_ONLY) + brush->index_start;

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

	renderer_UnmapAlloc(brush->index_handle);

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
							edge = memory_Malloc(sizeof(bsp_edge_t));
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
		brush->batches = memory_Malloc(sizeof(struct batch_t) * brush->max_batches);
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

	brush_ClipBrushes(0, 1);

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

	//SDL_LockMutex(brush->brush_mutex);

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

	if(brush->update_count < 2)
	{
		brush->update_count++;
	}

	//SDL_UnlockMutex(brush->brush_mutex);

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

	//SDL_LockMutex(brush->brush_mutex);

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

	if(brush->update_count < 2)
	{
		brush->update_count++;
	}

	//SDL_UnlockMutex(brush->brush_mutex);
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


	//SDL_LockMutex(brush->brush_mutex);


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


	if(brush->update_count < 2)
	{
		brush->update_count++;
	}

	//SDL_UnlockMutex(brush->brush_mutex);

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



void brush_UpdateBrushes()
{
    brush_GenTexCoords();
    brush_ClipBrushes(1, 0);
    brush_UploadBrushes();
}


void brush_ClipBrushes(int subtractive_only, int force_reupdate)
{
	int i;
	int j;
	int k;
	int c;

	unsigned long long start;
	unsigned long long end;

	int first_clip;
	bsp_polygon_t *polygon;
	brush_t *brush;
	brush_t *brush2;
	intersection_record_t *record;

	int updated_brush_count = 0;
	int updated_against_count = 0;

	//#define INTESECTION_CHECK_COUNT 6
	//#define CLIP_COUNT 4



	//start = __rdtsc();

	if(force_reupdate)
    {
        brush = brushes;

        while(brush)
        {
            brush->bm_flags |= BRUSH_MOVED | BRUSH_CLIP_POLYGONS;
            brush = brush->next;
        }
    }

    brush = brushes;

	while(brush)
	{
		//SDL_LockMutex(brush->brush_mutex);

		/*if(!process_brushes)
		{
			return;
		}*/

		brush2 = brushes;

		if(brush->bm_flags & BRUSH_MOVED)
		{
			brush->bm_flags |= BRUSH_CLIP_POLYGONS;

			while(brush2)
			{

				/*if(!process_brushes)
				{
					return;
				}*/

				if(brush2 == brush)
				{
					brush2 = brush2->next;
					continue;
				}

				//SDL_LockMutex(brush2->brush_mutex);

				if(brush_CheckBrushIntersection(brush, brush2))
				{
					brush_AddIntersectionRecord(brush, brush2);
					brush2->bm_flags |= BRUSH_CLIP_POLYGONS;
				}
				else
				{
					if(brush_RemoveIntersectionRecord(brush, brush2, 0))
					{
						brush2->bm_flags |= BRUSH_CLIP_POLYGONS;
					}
				}

				//SDL_UnlockMutex(brush2->brush_mutex);

				//brush2->bm_flags |= BRUSH_CLIP_POLYGONS;

				brush2 = brush2->next;
			}
		}
		else
		{
			/* this brush didn't move since last time we checked... */
			while(brush2)
			{

				/*if(!process_brushes)
				{
					return;
				}*/

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

				//SDL_LockMutex(brush2->brush_mutex);

				if(brush_CheckBrushIntersection(brush, brush2))
				{
					brush_AddIntersectionRecord(brush, brush2);
					brush2->bm_flags |= BRUSH_CLIP_POLYGONS;
				}
				else
				{
					if(brush_RemoveIntersectionRecord(brush, brush2, 0))
					{
						brush2->bm_flags |= BRUSH_CLIP_POLYGONS;
					}
				}

				//SDL_UnlockMutex(brush2->brush_mutex);

				//brush2->bm_flags |= BRUSH_CLIP_POLYGONS;

				brush2 = brush2->next;

				brush->bm_flags |= BRUSH_CLIP_POLYGONS;
			}
		}

		//SDL_UnlockMutex(brush->brush_mutex);

		brush = brush->next;
	}

	//end = __rdtsc();

	//printf("check intersection: %llu\n", end - start);

	brush = brushes;


	//start = __rdtsc();

	while(brush)
	{

		//SDL_LockMutex(brush->brush_mutex);

		/*if(!process_brushes)
		{
			SDL_UnlockMutex(brush->brush_mutex);
			return;
		}*/



		if(!(brush->bm_flags & BRUSH_CLIP_POLYGONS))
		{

			//SDL_UnlockMutex(brush->brush_mutex);

			/* this brush didn't touch any other brush this check, so
			skip it... */
			brush = brush->next;
			continue;
		}

		//printf("clip\n");

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

			/*if(!process_brushes)
			{
				SDL_UnlockMutex(brush->brush_mutex);
				return;
			}*/

			if((brush2->bm_flags & BRUSH_SUBTRACTIVE) || (!subtractive_only))
            {
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

                //SDL_UnlockMutex(brush2->brush_mutex);

                /* this brush is inside another brush... */
                if(!brush->clipped_polygons)
                {
                    break;
                }
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
			brush->bm_flags |= BRUSH_UPLOAD_DATA;
				//brush_UploadBrushVertices(brush);
				//brush_UploadBrushIndexes(brush);
				//brush_UpdateBrushElementBuffer(brush);
		}


		/*if(brush->update_count > 0)
		{
			brush->update_count--;
		}*/

		//if(!brush->update_count)
		{
			brush->bm_flags &= ~(BRUSH_MOVED | BRUSH_CLIP_POLYGONS);
		}

		//SDL_UnlockMutex(brush->brush_mutex);

		brush = brush->next;

			//updated_brush_count++;

			//if(updated_brush_count >= CLIP_COUNT)
			//{
				//printf("break!\n");
			//	break;
			//}
	}


	//end = __rdtsc();

	//printf("clip: %llu\n", end - start);

	//	last_updated_brush = brush;


	//	if(!last_updated_brush)
	//	{
	//		check_intersection = 1;
	//	}
	//}
}

void brush_GenTexCoords()
{
    brush_t *brush;

    int i;
    int j;

    bsp_polygon_t *polygons;
    bsp_polygon_t *polygon;
    vertex_t vertice;

    vec3_t vertice_vec;

    vec3_t face_tanh;
    vec3_t face_tanv;
    vec3_t face_normal;
    vec3_t face_center;

    float proj;
    float abs_proj;
    float highest_proj;
    float abs_highest_proj;

    float u;
    float v;

    int highest_proj_index;

    double integer_part;

    vec3_t vertice_plane_vec;

    vec3_t basis[3] = {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};

    brush = brushes;

    while(brush)
	{

		//SDL_LockMutex(brush->brush_mutex);
		if(brush->bm_flags & BRUSH_USE_WORLD_SPACE_TEX_COORDS)
		{
			if(brush->bm_flags & BRUSH_MOVED)
			{
				polygons = brush->base_polygons;

				for(i = 0; i < brush->base_polygons_count; i++)
				{
					polygon = polygons + i;
					face_normal = polygon->normal;

					//highest_proj = -1.0;

					abs_highest_proj = -1.0;

					for(j = 0; j < 3; j++)
					{
						proj = face_normal.floats[j];
						abs_proj = fabs(proj);

						if(abs_proj > abs_highest_proj)
						{
							abs_highest_proj = abs_proj;
							highest_proj = proj;
							highest_proj_index = j;
						}

					}

					//face_tanh = cross(face_normal, basis[(highest_proj_index + 1) % 3]);
					//face_tanv = cross(face_normal, face_tanh);

					if(brush->bm_flags & BRUSH_SUBTRACTIVE)
					{
						highest_proj = -highest_proj;
					}


					switch(highest_proj_index)
					{
						case 0:
							/* face is facing +X or -X... */

							if(highest_proj > 0.0)
							{
								face_normal = vec3_t_c(1.0, 0.0, 0.0);
								face_tanh = vec3_t_c(0.0, 0.0, -1.0);
								face_tanv = vec3_t_c(0.0, 1.0, 0.0);
							}
							else
							{
								face_normal = vec3_t_c(-1.0, 0.0, 0.0);
								face_tanh = vec3_t_c(0.0, 0.0, 1.0);
								face_tanv = vec3_t_c(0.0, 1.0, 0.0);
							}

						break;

						case 1:
							/* face is facing +Y or -Y... */

							if(highest_proj > 0.0)
							{
								face_normal = vec3_t_c(0.0, 1.0, 0.0);
								face_tanv = vec3_t_c(-1.0, 0.0, 0.0);
								face_tanh = vec3_t_c(0.0, 0.0, -1.0);
							}
							else
							{
								face_normal = vec3_t_c(0.0, -1.0, 0.0);
								face_tanv = vec3_t_c(-1.0, 0.0, 0.0);
								face_tanh = vec3_t_c(0.0, 0.0, 1.0);
							}

						break;

						case 2:
							/* face is facing +Z or -Z... */
							face_normal = vec3_t_c(0.0, 0.0, 1.0);
							face_tanh = vec3_t_c(1.0, 0.0, 0.0);
							face_tanv = vec3_t_c(0.0, 1.0, 0.0);
						break;
					}

					/*if(highest_proj < 0.0)
					{
						face_normal.floats[highest_proj_index] = -face_normal.floats[highest_proj_index];
					}*/


					face_center = vec3_t_c(0.0, 0.0, 0.0);

					for(j = 0; j < polygon->vert_count; j++)
					{
						vertice_vec = polygon->vertices[j].position;

						face_center.x += vertice_vec.x;
						face_center.y += vertice_vec.y;
						face_center.z += vertice_vec.z;

						proj = dot3(vertice_vec, face_normal);

						vertice_plane_vec.x = vertice_vec.x - face_normal.x * proj;
						vertice_plane_vec.y = vertice_vec.y - face_normal.y * proj;
						vertice_plane_vec.z = vertice_vec.z - face_normal.z * proj;

						//u = modf(dot3(vertice_plane_vec, face_tanh), &integer_part) * 0.5 + 0.5;
						//v = modf(dot3(vertice_plane_vec, face_tanv), &integer_part) * 0.5 + 0.5;

						u = dot3(vertice_plane_vec, face_tanh) * 0.5 + 0.5;
						v = dot3(vertice_plane_vec, face_tanv) * 0.5 + 0.5;

						polygon->vertices[j].tex_coord.x = u * polygon->tiling.x;
						polygon->vertices[j].tex_coord.y = v * polygon->tiling.y;
					}

					face_center.x /= polygon->vert_count;
					face_center.y /= polygon->vert_count;
					face_center.z /= polygon->vert_count;

					face_center.x += face_normal.x * 0.05;
					face_center.y += face_normal.y * 0.05;
					face_center.z += face_normal.z * 0.05;


				//	renderer_DrawLine(face_center, vec3_t_c(face_center.x + face_normal.x, face_center.y + face_normal.y, face_center.z + face_normal.z), vec3_t_c(0.0, 0.0, 1.0), 1.0, 1);
				//	renderer_DrawLine(face_center, vec3_t_c(face_center.x + face_tanh.x, face_center.y + face_tanh.y, face_center.z + face_tanh.z), vec3_t_c(1.0, 0.0, 0.0), 1.0, 1);
				//	renderer_DrawLine(face_center, vec3_t_c(face_center.x + face_tanv.x, face_center.y + face_tanv.y, face_center.z + face_tanv.z), vec3_t_c(0.0, 1.0, 0.0), 1.0, 1);

				}
			}


		}

		//SDL_UnlockMutex(brush->brush_mutex);

		brush = brush->next;
	}
}


void brush_UploadBrushes()
{
    struct brush_t *brush;

    brush = brushes;

    static int r = 0;

    static int update_count = 0;
    static brush_t *last_uploaded = NULL;

    /*if(last_uploaded)
	{
		brush = last_uploaded;
	}
	else
	{*/
		brush = brushes;
	//}

    while(brush)
	{
		if(brush->bm_flags & BRUSH_UPLOAD_DATA)
		{
			//SDL_LockMutex(brush->brush_mutex);

			if(!SDL_TryLockMutex(brush->brush_mutex))
			{
				brush_UploadBrushVertices(brush);
				brush_UploadBrushIndexes(brush);
				brush->bm_flags &= ~BRUSH_UPLOAD_DATA;

				SDL_UnlockMutex(brush->brush_mutex);

				update_count++;
			}

			/*if(update_count >= 10)
			{
				update_count = 0;
                break;
			}*/
		}

		brush = brush->next;

//		printf("upload brushes: %d\n", r);
//		r++;
	}

	//last_uploaded = brush;
}


#if 0
int brush_ProcessBrushesAsync(void *data)
{
    #if 0
	static int r = 0;
	//SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);
	while(process_brushes)
	{
		//printf("process brushes: %d\n", r);
		//r++;

		if(process_brushes > 1)
		{
			bsp_LockBrushPolygons();
			brush_UpdateTexCoords();
			brush_ClipBrushes();
			bsp_UnlockBrushPolygons();
		}
	}

	#endif
}

#endif

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

int brush_AddIntersectionRecord(brush_t *add_to, brush_t *to_add)
{
	intersection_record_t *record;
	intersection_record_t *records;

	if(add_to && to_add)
	{
		if(brush_GetIntersectionRecord(add_to, to_add))
		{
			return 1;
		}

		add_to->bm_flags |= BRUSH_CLIP_POLYGONS;
		to_add->bm_flags |= BRUSH_CLIP_POLYGONS;

		if(add_to->freed_records)
		{
			record = add_to->freed_records;
			add_to->freed_records = record->next;
		}
		else
		{
			record = memory_Malloc(sizeof(intersection_record_t));
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

				return 0;

			}
			else
			{
				record->prev = add_to->last_intersection_record;
				add_to->last_intersection_record->next = record;
			}


		}

		add_to->last_intersection_record = record;
	}

	return 0;
}

int brush_RemoveIntersectionRecord(brush_t *remove_from, brush_t *to_remove, int free_record)
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
				to_remove->bm_flags |= BRUSH_CLIP_POLYGONS;

				return 1;
			}
			//prev_record = record;
			record = record->next;
		}

	}

	return 0;
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




/*
===================================================================
===================================================================
===================================================================
*/

/* serialization structures */

char brush_section_start_tag[] = "[brush section start]";

struct brush_section_start_t
{
	char tag[(sizeof(brush_section_start_tag) + 3) & (~3)];
	int brush_count;

	int reserved0;
	int reserved1;
	int reserved2;
	int reserved3;
	int reserved4;
	int reserved5;
	int reserved6;
	int reserved7;

};

char brush_section_end_tag[] = "[brush section end]";

struct brush_section_end_t
{
	char tag[(sizeof(brush_section_end_tag) + 3) & (~3)];
};

struct brush_record_t
{
	mat3_t orientation;
	vec3_t position;
	vec3_t scale;
	int vertex_count;
	int triangle_group_count;
	int polygon_count;
	short type;
	short bm_flags;

	int reserved0;
	int reserved1;
	int reserved2;
	int reserved3;
	int reserved4;
	int reserved5;
	int reserved6;
	int reserved7;

};

struct polygon_record_t
{
	vec3_t normal;
	int vert_count;
	int first_index_offset;

	vec2_t tiling;
	//int reserved0;
	//int reserved1;
	int reserved2;
	int reserved3;
	int reserved4;
	int reserved5;
	int reserved6;
	int reserved7;

	char material_name[PATH_MAX];
};



void brush_SerializeBrushes(void **buffer, int *buffer_size)
{
	brush_t *brush;
	struct brush_section_start_t *section_start;
	struct brush_section_end_t *section_end;
	struct brush_record_t *brush_record;
	struct polygon_record_t *polygon_record;
	bsp_polygon_t *polygon;
	vertex_t *vertices;

	material_t *material;
	char *out;
	int size;
	int i;

	size = sizeof(struct brush_section_start_t) + sizeof(struct brush_section_end_t) + sizeof(struct brush_record_t) * brush_count;


	brush = brushes;

	while(brush)
	{
		size += sizeof(struct polygon_record_t) * brush->base_polygons_count;
		polygon = brush->base_polygons;

		while(polygon)
		{
			size += sizeof(vertex_t) * polygon->vert_count;
			polygon = polygon->next;
		}

		brush = brush->next;
	}

	out = memory_Calloc(size, 1);

	*buffer = out;
	*buffer_size = size;

	section_start = (struct brush_section_start_t *)out;
	out += sizeof(struct brush_section_start_t);

	strcpy(section_start->tag, brush_section_start_tag);

	section_start->brush_count = brush_count;
	section_start->reserved0 = 0;
	section_start->reserved1 = 0;
	section_start->reserved2 = 0;
	section_start->reserved3 = 0;
	section_start->reserved4 = 0;
	section_start->reserved5 = 0;
	section_start->reserved6 = 0;
	section_start->reserved7 = 0;

	brush = brushes;

	while(brush)
	{

		brush_record = (struct brush_record_t *)out;
		out += sizeof(struct brush_record_t);

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
			polygon_record = (struct polygon_record_t *)out;
			out += sizeof(struct polygon_record_t);

			material = material_GetMaterialPointerIndex(polygon->material_index);

			strcpy(polygon_record->material_name,  material->name);
			polygon_record->normal = polygon->normal;
			polygon_record->tiling = polygon->tiling;
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

	section_end = (struct brush_section_end_t *)out;
	out += sizeof(struct brush_section_end_t);

	strcpy(section_end->tag, brush_section_end_tag);
}

void brush_DeserializeBrushes(void **buffer)
{
	struct brush_section_start_t *header;
	struct brush_record_t *brush_record;
	struct polygon_record_t *polygon_record;
	vertex_t *vertices;
	brush_t *brush;
	int i;
	int j;
	int k;
	char *in;

	in = *(char **)buffer;

	if(in)
    {
        while(1)
        {
            if(!strcmp(in, brush_section_start_tag))
            {
                header = (struct brush_section_start_t *)in;
                in += sizeof(struct brush_section_start_t );

                for(i = 0; i < header->brush_count; i++)
                {
                    brush_record = (struct brush_record_t *)in;
                    in += sizeof(struct brush_record_t);

                    brush = brush_CreateEmptyBrush();
                    brush_InitializeBrush(brush, &brush_record->orientation, brush_record->position, brush_record->scale, brush_record->type, brush_record->vertex_count, brush_record->polygon_count);

                    //brush->bm_flags = brush_record->bm_flags;

                    if(brush_record->bm_flags & BRUSH_SUBTRACTIVE)
                    {
                        brush->bm_flags |= BRUSH_SUBTRACTIVE;
                    }

                    if(brush_record->bm_flags & BRUSH_USE_WORLD_SPACE_TEX_COORDS)
                    {
                        brush->bm_flags |= BRUSH_USE_WORLD_SPACE_TEX_COORDS;
                    }

                    //brush->bm_flags |= BRUSH_MOVED;

                    for(j = 0; j < brush_record->polygon_count; j++)
                    {
                        polygon_record = (struct polygon_record_t *)in;
                        in += sizeof(struct polygon_record_t);

                        vertices = (vertex_t *)in;
                        in += sizeof(vertex_t) * polygon_record->vert_count;

                        brush_AddPolygonToBrush(brush, vertices, polygon_record->normal, polygon_record->tiling, polygon_record->vert_count, material_MaterialIndex(polygon_record->material_name));
                    }

                    brush_FinalizeBrush(brush, 0);
                }
            }
            else if(!strcmp(in, brush_section_end_tag))
            {
                in += sizeof(struct brush_section_end_t);
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





