#ifndef BSP_CMP_H
#define BSP_CMP_H

#include "bsp_common.h"
#include "w_common.h"
#include "brush.h"

#define FUZZY_ZERO 0.0001


typedef struct bsp_triangle_t 
{
	struct bsp_triangle_t *next;
		
	vertex_t a;
	vertex_t b;
	vertex_t c;
	
	int material_index;
	int brush_index;
}bsp_triangle_t;


typedef struct bsp_polygon_t
{
	struct bsp_polygon_t *next;
	int vert_count;
	int brush_index;								/* the brush from which this polygon came from... */
	int b_used;
	//bsp_triangle_t *triangles;						/* the triangles that form this face... */
	vec3_t *vertices;
	vec3_t normal;
}bsp_polygon_t;


typedef struct bsp_edge_t
{
	struct bsp_edge_t *next;
	vec3_t v0;
	vec3_t v1;
	float dot;
	bsp_polygon_t *polygon0;
	bsp_polygon_t *polygon1;
}bsp_edge_t; 

#define MAX_PORTALS_PER_LEAF 512

typedef struct bsp_leaf_t
{
	short type;
	short bm_flags;
	int polygon_count;
	bsp_polygon_t *polygons;
	int triangle_count;
	bsp_triangle_t *triangles;
	
	int leaf_index;
	
	int portal_count;
	struct bsp_portal_t *portals[MAX_PORTALS_PER_LEAF];
	
	//int pvs_size;
	unsigned char *pvs;
	
	
}bsp_leaf_t;

typedef struct bsp_node_t
{
	
	short type;
	short bm_flags;
	struct bsp_node_t *front;
	struct bsp_node_t *back;

	vec3_t point;
	vec3_t normal;
	vec3_t tangent;				/* used to derive the third vector necessary to create the portal polygon... */

	bsp_polygon_t *splitter;
		
}bsp_node_t;



int bsp_ClassifyPoint(vec3_t point, vec3_t splitter_point, vec3_t splitter_normal);

int bsp_ClassifyTriangle(bsp_triangle_t *triangle, vec3_t point, vec3_t normal);

int bsp_ClassifyPolygon(bsp_polygon_t *polygon, vec3_t point, vec3_t normal);

int bsp_ClipEdge(vec3_t a, vec3_t b, vec3_t point, vec3_t normal, vec3_t *clip_vertex, float *time);

int bsp_SplitTriangle(bsp_triangle_t *triangle, vec3_t point, vec3_t normal, bsp_triangle_t **front, bsp_triangle_t **back);

int bsp_SplitPolygon(bsp_polygon_t *polygon, vec3_t point, vec3_t normal, bsp_polygon_t **front, bsp_polygon_t **back);

int bsp_TrimPolygon(bsp_polygon_t *polygon, vec3_t point, vec3_t normal);

bsp_polygon_t *bsp_DeepCopyPolygon(bsp_polygon_t *src);

bsp_polygon_t *bsp_DeepCopyPolygons(bsp_polygon_t *src);


bsp_triangle_t *bsp_FindSplittingTriangle(bsp_triangle_t *triangles, int ignore_used);

bsp_polygon_t *bsp_FindSplitter(bsp_polygon_t *polygons, int ignore_used);

bsp_polygon_t *bsp_BuildPolygonsFromBrush(brush_t *brush);

bsp_polygon_t *bsp_BuildPolygonsFromTriangles(bsp_triangle_t *triangles);

bsp_triangle_t *bsp_BuildTrianglesFromBrushes();

bsp_polygon_t *bsp_ClipPolygonToBsp(bsp_node_t *bsp, bsp_polygon_t *polygon, int copy);

bsp_polygon_t *bsp_ClipBspToBsp(bsp_node_t *bsp, bsp_node_t *input);

bsp_polygon_t *bsp_ClipBrushes(brush_t *brushes, int brush_count);

void bsp_ClipTriangleToSolidLeaves(bsp_node_t *root, bsp_triangle_t *triangle);

void bsp_ClipTrianglesToSolidLeaves(bsp_node_t *root, bsp_triangle_t *triangles);

void bsp_CountNodesAndLeaves(bsp_node_t *bsp, int *leaves, int *nodes);

void bsp_BuildTriangleGroups(bsp_node_t *root, triangle_group_t **groups, int *count);

void bsp_RemoveExterior(bsp_node_t *bsp);

void bsp_LinearizeBsp(bsp_node_t *bsp, vertex_t **vertices, int *vertex_count, bsp_pnode_t **lnodes, int *lnode_count, bsp_dleaf_t **lleaves, int *lleaves_count, triangle_group_t *groups, int tri_group_count, int create_leaves);

bsp_edge_t *bsp_BuildBevelEdges(bsp_polygon_t *brush_polygons);

bsp_edge_t *bsp_BuildEdgesFromBrushes();

void bsp_ExpandBrushes(vec3_t box_extents);

void bsp_BuildCollisionBsp();


void bsp_MergeLeafTriangles(bsp_node_t *root);

void bsp_BuildSolid(bsp_node_t **root, bsp_polygon_t *polygons);

void bsp_BuildSolidLeaf(bsp_node_t **root, bsp_polygon_t *polygons);

bsp_node_t *bsp_SolidBsp(bsp_polygon_t *polygons);

bsp_node_t *bsp_SolidLeafBsp(bsp_polygon_t *polygons);

void bsp_DeleteSolid(bsp_node_t *root);

void bsp_DeleteSolidLeaf(bsp_node_t *root);

void bsp_CompileBsp(int remove_outside);

int bsp_CompileBspAsync(void *param);

void bsp_DrawExpandedBrushes();








#endif
