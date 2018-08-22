#ifndef BSP_COMMON_H
#define BSP_COMMON_H

#include "vector.h"
#include "model.h"
//#include "l_common.h"


//#define TRIS_FIRST_VERTEX(tri) (tri&0x00ffffff)
//#define TRIS_GROUP(tri) ((tri&0xff000000)>>24)
//#define PACK_TRIS(group, first_index) ((group<<24)|(first_index&0x00ffffff))



/* indexes into an array of vertex_t. first_vertex is the index of the first
vertex of this triangle, and triangle_group is the index of the triangle_group_t
this triangle belongs to. bsp_triangle_t's may be unsorted on memory. */
typedef struct
{
	int first_vertex;
	int batch;				/* a.k.a. material */
}bsp_striangle_t;			/* could cut this struct in half... 8 bits for triangle group + 24 bits for the first vertex, which
gives 256 different triangle batches and 5592405 triangles... */


typedef struct
{
	int byte_count;
	char *leaves;
}pvs_t;


#define BSP_EMPTY_LEAF 0x0000
#define BSP_SOLID_LEAF 0xffff

/* dleaf stands for draw leaf... */
typedef struct
{
	vec3_t center;
	vec3_t extents;
	unsigned int tris_count;			/* could get rid of this... */
	bsp_striangle_t *tris;
	unsigned char *pvs;					/* this makes this struct not 32 byte aligned. Fuck! */
}bsp_dleaf_t;

//#define PACK_NEXT_PREV_CURSOR(next, prev, cursor) ((next&0x00003fff)|((prev&0x00003fff)<<14)|((cursor&0x0000000f)<<28))

//#define UNPACK_NEXT_PREV_CURSOR(next_prev_cursor, next, prev, cursor) next=next_prev_cursor&0x00003fff;				\
																	  prev=(next_prev_cursor>>14)&0x00003fff;		\
																	  cursor=(next_prev_cursor>>28)&0x0000000f;



typedef struct
{
	unsigned short indexes[6];
	unsigned next : 14;
	unsigned prev : 14;
	unsigned cursor : 4;
	//unsigned int next_prev_cursor;
	//unsigned short cursor;
	//unsigned short next;				/* relative displacement... */
}bsp_indexes_t;					/* 16 bytes... */

typedef struct
{
	unsigned int lights[512 >> 5];
}bsp_lights_t;

typedef struct
{
	unsigned int entities[1024 >> 5];
}bsp_entities_t;

typedef struct
{
	int triangle_count;
	int first_vertex;
	int material_index;
}bsp_batch_t;


typedef struct
{
	vec3_t normal;
	vec3_t point;
	vec3_t edge0;
	vec3_t edge1;
}bsp_clipplane_t;



typedef struct bsp_polygon_t
{
	struct bsp_polygon_t *next;
	int vert_count;
	int brush_index;								/* the brush from which this polygon came from... */
	int material_index;
	int triangle_group;
	int b_used;
	vertex_t *vertices;
	int *indexes;
	//bsp_triangle_t *triangles;						/* the triangles that form this face... */
	//vec3_t *vertices;
	vec3_t normal;
}bsp_polygon_t;



/* if this node has both child == 0, it means
it points to a empty leaf, child == 0xffff points
to a solid leaf. In case of empty leaf, the leaf
index will be contained in dist. To obtain the index,
just do *(int *)&dist... */
typedef struct
{
	vec3_t normal;					/* could use packed normal here, but bsp's are really finicky with precision... */
	float dist;
	unsigned short child[2];		/* relative displacement... */	 /* given that the front child will be always one position away, it's
																		only necessary to keep the stride to the back child. */
}bsp_pnode_t;	/* would like to shrink this from 20 to 16 bytes... */

typedef struct
{
	vec3_t normal;
	vec3_t position;
	float frac;
	float dist;
	int bm_flags;
}trace_t;

enum TRACE_FLAGS
{
	TRACE_ALL_SOLID = 1,
	TRACE_START_SOLID = 1 << 1,
	TRACE_IN_OPEN = 1 << 2,
	TRACE_MID_SOLID = 1 << 3,
};

/*enum BSP_NODE_FLAGS
{
	BSP_NODE_SOLID = 1,
	BSP_NODE_LEAF = 1 << 1,
};
*/

enum BSP_LEAF_FLAGS
{
	BSP_SOLID = 1,
};

enum BSP_NODE_TYPE
{
	BSP_LEAF = 1,
	BSP_NODE,

};

enum POINT_PLANE
{
	POINT_FRONT = 1,
	POINT_BACK = 1 << 1,
	POINT_CONTAINED = 1 << 2,
};

enum TRIANGLE_PLANE
{
	TRIANGLE_FRONT = 1,
	TRIANGLE_BACK = 1 << 1,
	TRIANGLE_CONTAINED = 1 << 2,
	TRIANGLE_CONTAINED_FRONT = 1 << 4,
	TRIANGLE_CONTAINED_BACK = 1 << 5,
	TRIANGLE_STRADDLING = 1 << 3,
};

enum POLYGON_SPLITTER
{
	POLYGON_FRONT = 1,
	POLYGON_BACK = 1 << 1,
	POLYGON_CONTAINED = 1 << 2,
	POLYGON_STRADDLING = 1 << 3,
	POLYGON_CONTAINED_FRONT = 1 << 4,
	POLYGON_CONTAINED_BACK = 1 << 5,
};






#endif
