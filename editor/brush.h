#ifndef BRUSH_H
#define BRUSH_H

#include <assert.h>

#include "r_common.h"

#include "matrix_types.h"
#include "vector_types.h"
#include "model.h"
#include "world.h"
#include "bsp_common.h"

#include "SDL2/SDL.h"
//#include "bsp_file.h"
//#include "bsp_cmp.h"


#define BRUSH_MAX_VERTICES (8192*3)


enum BRUSH_TYPE
{
	BRUSH_CUBE = 1,
	BRUSH_CONE,
	BRUSH_CYLINDER,
	BRUSH_BOUNDS,
	BRUSH_COLLISION,
	BRUSH_EMPTY,
	BRUSH_INVALID,
	BRUSH_COMPOUND,
};

enum BRUSH_FLAGS
{
	BRUSH_MOVED = 1,
	BRUSH_ON_COMPOUND = 1 << 1,
	BRUSH_SUBTRACTIVE = 1 << 2,
	BRUSH_UPDATE = 1 << 3,
	BRUSH_INVISIBLE = 1 << 4,
	BRUSH_CLIP_POLYGONS = 1 << 5,
	BRUSH_HAS_PREV_CLIPS = 1 << 6,
	BRUSH_USE_WORLD_SPACE_TEX_COORDS = 1 << 7,
	BRUSH_UPLOAD_DATA = 1 << 8,
};


struct brush_t;


typedef struct intersection_record_t
{
	struct intersection_record_t *next;
	struct intersection_record_t *prev;
	struct brush_t *intersecting_brush;
}intersection_record_t;


/* Each brush_t contains its own GL_ELEMENT_ARRAY_BUFFER, in order to
keep update operations down to a managable overhead. The indexes kept
on this buffer are relative to the gpu heap, where vertices are stored
for rendering. The final index value is calculated by taking the vertex
index (which is relative to its brush), and adding the start field
(which is an offset from the beginning of the gpu heap to the first vertex
position from the space allocated). */
typedef struct brush_t
{

	struct brush_t *next;
	struct brush_t *prev;

	mat3_t orientation;
	vec3_t position;
	vec3_t scale;

	//int max_triangle_groups;
	//int triangle_group_count;
	//triangle_group_t *triangle_groups;				// keep a local list of triangle groups,
												       //to avoid doing a potentially expensive
													   //update on a global group list when a brush
													   //is added/removed to/from the world... */



	int max_batches;
	int batch_count;
	struct batch_t *batches;


	struct bsp_striangle_t *triangles;
	struct bsp_edge_t *edges;						/* necessary to manipulate individual faces... */


	int base_polygons_count;
	int base_polygons_vert_count;
	struct bsp_polygon_t *base_polygons;
	vertex_t *base_polygons_vertices;
	struct bsp_node_t *brush_bsp;


	int clipped_polygon_count;
	int clipped_polygons_vert_count;
	struct bsp_polygon_t *clipped_polygons;
	vertex_t *clipped_polygons_vertices;
	int *clipped_polygons_indexes;
	int clipped_polygons_index_count;


	int max_vertexes;								/* max number before a gpu realloc is needed... */
	int start;
	struct gpu_alloc_handle_t handle;

	int index_start;
	struct gpu_alloc_handle_t index_handle;

	int type;
	//unsigned int element_buffer;
	int bm_flags;

	intersection_record_t *intersection_records;
	intersection_record_t *last_intersection_record;
	intersection_record_t *freed_records;


	SDL_mutex *brush_mutex;

	int update_count;


	//intersection_record_t *

	//int max_intersections;
	//int *intersections;


}brush_t;





/*
===================================================================
===================================================================
===================================================================
*/



void brush_Init();

void brush_Finish();

void brush_ProcessBrushes();

void brush_UpdateAllBrushes();



brush_t *brush_CreateBrush(vec3_t position, mat3_t *orientation, vec3_t scale, short type, short b_subtractive);

brush_t *brush_CreateEmptyBrush();


void brush_InitializeBrush(brush_t *brush, mat3_t *orientation, vec3_t position, vec3_t scale, int type, int vertice_count, int polygon_count);

void brush_FinalizeBrush(brush_t *brush, int transform_base_vertices);

void brush_AllocBaseVertices(brush_t *brush, int vert_count, vertex_t *vertices);

void brush_AllocBasePolygons(brush_t *brush, int polygon_count);

void brush_AddPolygonToBrush(brush_t *brush, vertex_t *polygon_vertices, vec3_t normal, vec2_t tiling, int polygon_vertice_count, int material_index);

void brush_LinkPolygonsToVertices(brush_t *brush);




void brush_IncBrushMaterialsRefCount(brush_t *brush);

void brush_DecBrushMaterialsRefCount(brush_t *brush);



brush_t *brush_CopyBrush(brush_t *src);

void brush_DestroyBrush(brush_t *brush);

void brush_DestroyBrushIndex(int brush_index);

void brush_DestroyAllBrushes();

void brush_CreateCylinder(int base_vertexes, int *vert_count, float **vertices, float **normals);



void brush_UpdateBrushElementBuffer(brush_t *brush);

void brush_UploadBrushVertices(brush_t *brush);

void brush_UploadBrushIndexes(brush_t *brush);

void brush_BuildBrushBsp(brush_t *brush);

void brush_BuildEdgeList(brush_t *brush);

void brush_BuildBatches(brush_t *brush);

bsp_polygon_t *brush_BuildPolygonListFromBrushes();




void brush_TranslateBrush(brush_t *brush, vec3_t direction);

void brush_RotateBrush(brush_t *brush, vec3_t axis, float amount);

void brush_ScaleBrush(brush_t *brush, vec3_t axis, float amount);

void brush_SetFaceMaterial(brush_t *brush, int face_index, int material_index);

//void brush_AddBrushToCompoundBrush(brush_t *compound_brush, brush_t *brush);

//void brush_ComputeCompoundBrush(brush_t *brush);

void brush_SetAllVisible();

void brush_SetAllInvisible();



void brush_ClipBrushes();

void brush_UpdateTexCoords();

void brush_UploadBrushes();

int brush_ProcessBrushesAsync(void *data);

int brush_CheckBrushIntersection(brush_t *a, brush_t *b);

int brush_AddIntersectionRecord(brush_t *add_to, brush_t *to_add);

int brush_RemoveIntersectionRecord(brush_t *remove_from, brush_t *to_remove, int free_record);

intersection_record_t *brush_GetIntersectionRecord(brush_t *brush, brush_t *brush2);





void brush_SerializeBrushes(void **buffer, int *buffer_size);

void brush_DeserializeBrushes(void **buffer);



#endif
















