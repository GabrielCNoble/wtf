#ifndef BSP_CMP_H
#define BSP_CMP_H

#ifndef REMOVE_ALL_DEPENDENCIES

	#include "bsp_common.h"
	#include "w_common.h"
	#include "brush.h"
	#include "SDL2\SDL.h"

#else


	typedef union
	{
		struct
		{
			float x;
			float y;
		};

		struct
		{
			float floats[2];
		};

		struct
		{
			float a0;
			float a1;
		};
	}vec2_t;


	typedef union
	{
		struct
		{
			float x;
			float y;
			float z;
		};

		struct
		{
			float floats[3];
		};

		struct
		{
			float r;
			float g;
			float b;
		};

		struct
		{
			float a0;
			float a1;
			float a2;
		};

		struct
		{
			vec2_t vec2;
			float e;
		};

	}vec3_t;


	typedef union
	{
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};

		struct
		{
			float floats[4];
		};

		struct
		{
			float r;
			float g;
			float b;
			float a;
		};

		struct
		{
			float a0;
			float a1;
			float a2;
			float a3;
		};
		struct
		{
			vec3_t vec3;
			float pad;
		};

	}vec4_t;



	typedef struct
	{
		vec3_t position;
		vec3_t normal;
		vec3_t tangent;
		vec2_t tex_coord;
	}vertex_t;

#endif

#define FUZZY_ZERO 0.0005





typedef struct bsp_triangle_t
{
	struct bsp_triangle_t *next;

	vertex_t a;
	vertex_t b;
	vertex_t c;

	#ifndef REMOVE_ALL_DEPENDENCIES

	int material_index;
	int brush_index;

	#endif

}bsp_triangle_t;




typedef struct bsp_edge_t
{
	struct bsp_edge_t *next;
	vec3_t v0;
	vec3_t v1;
	short v0_p0;							/* vertex 0 on polygon 0... */
	short v1_p0;							/* vertex 1 on polygon 0... */
	short v0_p1;							/* vertex 0 on polygon 1... */
	short v1_p1;							/* vertex 1 on polygon 1... */
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


	vec3_t center;

	int leaf_index;

	int portal_count;

	#ifndef REMOVE_ALL_DEPENDENCIES

	struct bsp_portal_t *portals[MAX_PORTALS_PER_LEAF];

	int src_leaf_marker_count;
	int *src_leaf_markers;

	unsigned char *pvs;

	#endif

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

enum CSG_SET_OPS
{
	CSG_OP_UNION,
	CSG_OP_SUBTRACTION,
	CSG_OP_INTERSECTION,
};

/*
===============================================================================
===============================================================================
===============================================================================
*/

/* compiler core functions + helpers... */

int bsp_ClassifyPoint(vec3_t point, vec3_t splitter_point, vec3_t splitter_normal);

int bsp_ClassifyPolygon(bsp_polygon_t *polygon, vec3_t point, vec3_t normal);

int bsp_ClipEdge(vec3_t a, vec3_t b, vec3_t point, vec3_t normal, vec3_t *clip_vertex, float *time);

int bsp_SplitPolygon(bsp_polygon_t *polygon, vec3_t point, vec3_t normal, bsp_polygon_t **front, bsp_polygon_t **back);

bsp_polygon_t *bsp_DeepCopyPolygon(bsp_polygon_t *src);

bsp_polygon_t *bsp_DeepCopyPolygons(bsp_polygon_t *src);

void bsp_DeletePolygons(bsp_polygon_t *polygons);

//int bsp_TrimPolygon(bsp_polygon_t *polygon, vec3_t point, vec3_t normal);

//int bsp_ClipCoplanarPolygonToPolygonEdges(bsp_polygon_t *polygon, bsp_polygon_t *splitter, bsp_polygon_t **clips);

bsp_polygon_t *bsp_FindSplitter(bsp_polygon_t **polygons, int ignore_used, int ignore_coplanar);

bsp_polygon_t *bsp_OpPolygonToBsp(int op, bsp_node_t *bsp, bsp_polygon_t *polygon, int copy);

bsp_polygon_t *bsp_ClipPolygonToBsp(bsp_node_t *bsp, bsp_polygon_t *polygon, int copy);

bsp_polygon_t *bsp_ClipPolygonsToBsp(bsp_node_t *bsp, bsp_polygon_t *polygons, int do_copy);

bsp_polygon_t *bsp_ClipBspToBsp(bsp_node_t *bsp, bsp_node_t *input);




void bsp_BuildSolid(bsp_node_t **root, bsp_polygon_t *polygons, int ignore_used, int ignore_coplanar);

void bsp_BuildSolidLeaf(bsp_node_t **root, bsp_polygon_t *polygons);

bsp_node_t *bsp_SolidBsp(bsp_polygon_t *polygons);

void bsp_DeleteSolidBsp(bsp_node_t *bsp, int free_splitters);

bsp_node_t *bsp_SolidLeafBsp(bsp_polygon_t *polygons);

void bsp_DeleteSolidLeafBsp(bsp_node_t *bsp);




void bsp_NegateBsp(bsp_node_t *bsp);

void bsp_CountNodesAndLeaves(bsp_node_t *bsp, int *leaves, int *nodes);

int bsp_IntersectBsp(bsp_node_t *node, vec3_t start, vec3_t end);


/*
===============================================================================
===============================================================================
===============================================================================
*/

#ifndef REMOVE_ALL_DEPENDENCIES

/* engine specific kludge... */

bsp_polygon_t *bsp_DeepCopyPolygonsContiguous(bsp_polygon_t *src);

void bsp_TriangulatePolygon(bsp_polygon_t *polygon, vertex_t **vertices, int *vertex_count);

void bsp_TriangulatePolygonIndexes(bsp_polygon_t *polygon, int **indexes, int *index_count);

void bsp_TriangulatePolygonsIndexes(bsp_polygon_t *polygons, int **indexes, int *index_count);




void bsp_QuickHull(vec2_t *in_verts, int in_vert_count, vec2_t **hull_verts, int *hull_vert_count);

bsp_polygon_t *bsp_ClipContiguousPolygonsToBsp(int op, bsp_node_t *bsp0, bsp_node_t *bsp1, bsp_polygon_t *polygons0, bsp_polygon_t *polygons1, int do_copy);




bsp_polygon_t *bsp_ClipBrushes(brush_t *brush_list, int brush_list_count);

bsp_polygon_t *bsp_ClipBrushes2(brush_t *brush_list, int brush_list_count);




void bsp_TriangulateLeafPolygons(bsp_node_t *node);

void bsp_BuildTriangleGroups(bsp_node_t *root, struct batch_t **groups, int *count);

void bsp_RemoveExterior(bsp_node_t *bsp);

void bsp_AllocPvsForLeaves(bsp_node_t *bsp);

void bsp_LinearizeBsp(bsp_node_t *bsp, vertex_t **vertices, int *vertex_count, bsp_pnode_t **lnodes, int *lnode_count, bsp_dleaf_t **lleaves, int *lleaves_count, struct batch_t *groups, int tri_group_count, int create_leaves);

bsp_edge_t *bsp_BuildBevelEdges(bsp_polygon_t *brush_polygons);

void bsp_ExpandBrushes(vec3_t box_extents);

void bsp_CompileBsp(int remove_outside);

#endif









#endif
