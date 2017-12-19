#ifndef BSP_COMMON_H
#define BSP_COMMON_H

#include "vector.h"
#include "mesh.h"
#include "l_common.h"


#define TRIS_FIRST_INDEX(tri) (tri&0x00ffffff)
#define TRIS_GROUP(tri) ((tri&0xff000000)>>24)
#define TRIS(group, first_index, tri) (tri=group;tri<<=24;tri|=first_index&0x00ffffff)

typedef struct
{
	int first_vertex;
	int triangle_group;
}bsp_striangle_t;			/* could cut this struct in half... 8 bits for triangle group + 24 bits for the first vertex, which
gives 256 different triangle groups and 5592405 triangles... */


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
	int leaf_index;
	bsp_striangle_t *tris;				
	unsigned int tris_count;			/* could get rid of this... */
	unsigned char *pvs;/* this makes this struct not 32 byte aligned. Fuck! */
	
}bsp_dleaf_t;


typedef struct
{
	unsigned int lights[MAX_WORLD_LIGHTS >> 5];
}bsp_lights_t;

/* if this node has both child == 0, it means
it points to a empty leaf, child == 0xffff points
to a solid leaf. In case of empty leaf, the leaf
index will be contained in dist. To obtain the index, 
just do *(int *)&dist... */
typedef struct
{
	vec3_t normal;
	//vec3_t point;
	float dist;									
	unsigned short child[2];		/* relative displacement... */
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
	BSP_SHORT_LEAF,
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
