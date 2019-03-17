#ifndef L_COMMON_H
#define L_COMMON_H


#include "containers/list.h"
#include "camera_types.h"
#include "bsp_common.h"

#define LIGHT_MAX_RADIUS 250.0
#define LIGHT_MIN_RADIUS 1.0

#define LIGHT_MAX_ENERGY 5000.0
#define LIGHT_MIN_ENERGY 0.0

#define MAX_WORLD_LIGHTS 512

#define LIGHT_UNIFORM_BUFFER_SIZE 32				/* !!! if this gets changed, it has to be updated on forward_pass.frag !!! */
#define MAX_INDEXES_PER_FRUSTUM (3*1024)		/* 512 triangles... */
#define MAX_INDEXES_PER_GROUP 1536				/* 512 triangles */
#define MAX_TRIANGLES_PER_LIGHT (MAX_INDEXES_PER_FRUSTUM*2)


#define SHADOW_MAP_RESOLUTION 512		/* fixed size shadow maps for all lights... */
#define SHARED_SHADOW_MAP_HEIGHT 8192
#define SHARED_SHADOW_MAP_WIDTH 8192


#define CLUSTERS_PER_ROW 32
#define CLUSTER_ROWS 16
#define CLUSTER_LAYERS 24

#define LIGHT_CLUSTER_UNIFORM_BUFFER_BINDING 3
#define CLUSTER_OFFSET(x, y, z) (x+y*CLUSTERS_PER_ROW+z*CLUSTERS_PER_ROW*CLUSTER_ROWS)


//#define LIGHT_RADIUS(radius) (LIGHT_MIN_RADIUS+(LIGHT_MAX_RADIUS-LIGHT_MIN_RADIUS)*((float)((unsigned short)radius)/(float)0xffff))
//#define LIGHT_ENERGY(energy) (LIGHT_MIN_ENERGY+(LIGHT_MAX_ENERGY-LIGHT_MIN_ENERGY)*((float)((unsigned short)energy)/(float)0xffff))

//#define SET_LIGHT_RADIUS(radius) ((unsigned short)(0xffff * (radius / (LIGHT_MAX_RADIUS + LIGHT_MIN_RADIUS))))
//#define SET_LIGHT_ENERGY(energy) ((unsigned short)(0xffff * (energy / LIGHT_MAX_ENERGY)))


#define UNPACK_LIGHT_RADIUS(radius) (((LIGHT_MAX_RADIUS-LIGHT_MIN_RADIUS)*((float)((unsigned short)radius)/(float)0xffff)))
#define UNPACK_LIGHT_ENERGY(energy) (((LIGHT_MAX_ENERGY-LIGHT_MIN_ENERGY)*((float)((unsigned short)energy)/(float)0xffff)))

#define PACK_LIGHT_RADIUS(radius) ((unsigned short)(0xffff * (radius / (LIGHT_MAX_RADIUS - LIGHT_MIN_RADIUS))))
#define PACK_LIGHT_ENERGY(energy) ((unsigned short)(0xffff * (energy / (LIGHT_MAX_ENERGY - LIGHT_MIN_ENERGY))))


//#define PACK_CLUSTER_INDEXES(x, y, z) ((z<<24)|(x<<16)|(y))
//#define UNPACK_CLUSTER_INDEXES(x, y, z, index) y=((index)&0x000000ff);x=((index>>16)&0x000000ff);z=((index>>24)&0x000000ff)

//#define PACK_CLUSTER_INDEXES2(x0, y0, z0, x1, y1, z1) ((z1<<27)|(y1<<22)|(x1<<16)|(z0<<11)|(y0<<6)|x0)

//#define UNPACK_CLUSTER_INDEXES2(x0, y0, z0, x1, y1, z1, packed_clusters) z1=((packed_clusters>>27)&0x0000001f); \
																		 y1=((packed_clusters>>22)&0x0000001f); \
																		 x1=((packed_clusters>>16)&0x0000003f); \
																		 z0=((packed_clusters>>11)&0x0000001f); \
																		 y0=((packed_clusters>>6)&0x0000001f);	\
																		 x0=(packed_clusters&0x0000003f);


#define LIGHT_MAX_NAME_LEN 16	/* including trailing null... */

#define L_INVALID_SHADOW_MAP_INDEX 0xff


enum LIGHT_FRUSTUM
{
	LIGHT_FRUSTUM_X_POS = 0,
	LIGHT_FRUSTUM_X_NEG,
	LIGHT_FRUSTUM_Y_POS,
	LIGHT_FRUSTUM_Y_NEG,
	LIGHT_FRUSTUM_Z_POS,
	LIGHT_FRUSTUM_Z_NEG,
};





enum LIGHT_FLAGS
{
	LIGHT_CACHED = 1,
	LIGHT_MOVED = 1 << 1,
	LIGHT_UPLOAD_INDICES = 1 << 2,
	LIGHT_GENERATE_SHADOWS = 1 << 3,
	LIGHT_DROPPED_SHADOW = 1 << 4,
	LIGHT_UPDATE_SHADOW_MAP = 1 << 5,
	LIGHT_INVALID = 1 << 6
};


/* to which view (camera) a
set of clusters belong to... */
//typedef struct
//{
//	camera_t *view;
//	unsigned int clusters;
//}view_clusters_t;


/*struct light_cluster_t
{
	unsigned x0 : 6;
	unsigned y0 : 5;
	unsigned z0 : 5;

	unsigned x1 : 6;
	unsigned y1 : 5;
	unsigned z1 : 5;
};*/

struct light_cluster_t
{
	unsigned x : 10;
	unsigned y : 10;
	unsigned z : 10;
};

struct light_position_data_t
{
	//mat4_t world_to_light_matrix;
	mat3_t orientation;
	vec3_t position;
	//vec4_t screen_min_max;

    struct bsp_dleaf_t *leaf;

	unsigned short radius;
	unsigned short flags;
	//unsigned short energy;


    unsigned short first_cluster_data;
    unsigned short cluster_data_count;


	//unsigned int align0;
};          /* 64 bytes... */

struct light_cluster_data_t
{
    struct light_cluster_t first_cluster;
	struct light_cluster_t last_cluster;
};          /* 8 bytes... */

struct light_params_data_t
{
	//struct bsp_dleaf_t *leaf;						/* in which leaf this light is in (updated every time it moves)... */
	//vec3_t box_max;								/* this box is calculated when the visible triangles are determined. As long as
													//the light remains inside this box, no update is needed...*/
	//vec3_t box_min;

	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned int first_triangle;
    unsigned int triangle_count;
	//char cache;

	//unsigned short radius;
	//unsigned short energy;
	//unsigned short visible_triangle_count;		/* this could go somewhere else... */

	//unsigned int shadow_map;

	//unsigned char flags;
	//unsigned char align0;
	unsigned short energy;
	unsigned char align1;
	unsigned char align2;

	char *name;


	//struct list_t triangle_indices[6];

	//struct list_t visible_triangles;
	//struct gpu_alloc_handle_t indices_handle;
	//unsigned int indices_start;



	//short shadow_map;

	//short x;
	//short y;
	//short w;
	//short h;

	//unsigned char align1;
	//unsigned char align2;

	//int view_cluster_count;
	//unsigned short view_cluster_list_cursor;
	//unsigned short view_cluster_list_size;
	//view_clusters_t *view_clusters;
	//unsigned int *view_clusters;
	//view_cluster_t *view_clusters;

//	unsigned int first_cluster;
//	unsigned int last_cluster;

	//struct light_cluster_t first_cluster;
	//struct light_cluster_t last_cluster;

	//vec4_t screen_min_max;

};          /* 8 bytes... */


struct light_pointer_t
{
    struct light_position_data_t *position;
    struct light_params_data_t *params;
    struct light_cluster_data_t *cluster;
};

/*typedef struct
{
	light_params_t *params;
	light_position_t *position;
}light_ptr_t;*/


struct shadow_map_t
{
	//short x;
	//short y;
	//short light_index;
	//short align0;

	short light_index;
	short align;

	int last_touched_frame;				/* this is used to determine which shadow map should be freed when there's no
										free shadow maps to service an allocation request... */
};



#endif
