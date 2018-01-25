#ifndef BRUSH_H
#define BRUSH_H

#include "matrix_types.h"
#include "vector_types.h"
#include "mesh.h"
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
	
	struct bsp_polygon_t *polygons;						/* HACK!! */
	
	int max_vertexes;								/* max number before a gpu realloc is needed... */
	int vertex_count;
	int base_vertex_count;
	int start;
	int handle;
	int type;
	unsigned int element_buffer;
}brush_t;




void brush_Init();

void brush_Finish();

int brush_CreateBrush(vec3_t position, mat3_t *orientation, vec3_t scale, short type);

int brush_CreateEmptyBrush();

void brush_BuildTriangleGroups(brush_t *brush);

int brush_CopyBrush(brush_t *src);

void brush_DestroyBrush(brush_t *brush);

void brush_DestroyBrushIndex(int brush_index);

void brush_DestroyAllBrushes();

void brush_CreateCylinderBrush(int base_vertexes, int *vert_count, float **vertices, float **normals);

void brush_UpdateBrushElementBuffer(brush_t *brush);

void brush_UpdateBrush(brush_t *brush);

void brush_TranslateBrush(brush_t *brush, vec3_t direction);

void brush_RotateBrush(brush_t *brush, vec3_t axis, float amount);

void brush_ScaleBrush(brush_t *brush, vec3_t axis, float amount);

#endif 
















