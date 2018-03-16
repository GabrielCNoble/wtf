#ifndef BRUSH_H
#define BRUSH_H

#include <assert.h>

#include "matrix_types.h"
#include "vector_types.h"
#include "model.h"
#include "world.h"
#include "bsp_common.h"
//#include "bsp_cmp.h"

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
};





/* A brush_triangle_t groups three vertices into a triangle, and links them to a 
triangle_group_t. */
/*typedef struct
{
	int first_vertex;
	int triangle_group_index;			
}brush_triangle_t;*/


/* Each brush_t contains its own GL_ELEMENT_ARRAY_BUFFER, in order to
keep update operations down to a managable overhead. The indexes kept
on this buffer are relative to the gpu heap, where vertices are stored
for rendering. The final index value is calculated by taking the vertex
index (which is relative to its brush), and adding the start field 
(which is an offset from the beginning of the gpu heap to the first vertex 
position from the space allocated). */
typedef struct
{
	mat3_t orientation;
	vec3_t position;
	vec3_t scale;
	
	int max_triangle_groups;
	int triangle_group_count;
	triangle_group_t *triangle_groups;				/* keep a local list of triangle groups,
												       to avoid doing a potentially expensive 
													   update on a global group list when a brush 
													   is added/removed to/from the world... */
	bsp_striangle_t *triangles;
	vertex_t *vertices;
	struct bsp_edge_t *edges;						/* necessary to manipulate individual faces... */
	int *indexes;
	int index_count;
	struct bsp_polygon_t *polygons;			
	int polygon_count;
	
	
	struct bsp_polygon_t *clipped_polygons;
	struct bsp_node_t *brush_bsp;
	
	//vec3_t obb[3];
	
	
	int max_vertexes;								/* max number before a gpu realloc is needed... */
	int vertex_count;
	//int base_vertex_count;
	int start;
	int handle;
	int type;
	unsigned int element_buffer;
	int bm_flags;
	
	int max_intersections;
	int *intersections;
	
	
}brush_t;




void brush_Init();

void brush_Finish();

void brush_ProcessBrushes();

int brush_CreateBrush(vec3_t position, mat3_t *orientation, vec3_t scale, short type, short b_subtractive);

int brush_CreateEmptyBrush();

void brush_BuildTriangleGroups(brush_t *brush);

void brush_BuildEdgeList(brush_t *brush);

int brush_CopyBrush(brush_t *src);

void brush_DestroyBrush(brush_t *brush);

void brush_DestroyBrushIndex(int brush_index);

void brush_DestroyAllBrushes();

void brush_CreateCylinder(int base_vertexes, int *vert_count, float **vertices, float **normals);

void brush_UpdateBrushElementBuffer(brush_t *brush);

void brush_UploadBrushVertices(brush_t *brush);

void brush_TranslateBrush(brush_t *brush, vec3_t direction);

void brush_RotateBrush(brush_t *brush, vec3_t axis, float amount);

void brush_ScaleBrush(brush_t *brush, vec3_t axis, float amount);

void brush_SetFaceMaterial(brush_t *brush, int face_index, int material_index);

//void brush_AddBrushToCompoundBrush(brush_t *compound_brush, brush_t *brush);

//void brush_ComputeCompoundBrush(brush_t *brush);

void brush_SetAllVisible();

void brush_SetAllInvisible();

void brush_BuildBrushBsp(brush_t *brush);

void brush_CheckIntersecting();

int brush_CheckBrushIntersection(brush_t *a, brush_t *b);

#endif 
















