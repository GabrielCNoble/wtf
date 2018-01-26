#ifndef L_COMMON_H
#define L_COMMON_H

#define LIGHT_MAX_RADIUS 250.0
#define LIGHT_MIN_RADIUS 1.0

#define LIGHT_MAX_ENERGY 500.0
#define LIGHT_MIN_ENERGY 1.0

#define MAX_WORLD_LIGHTS 512

#define LIGHT_CACHE_SIZE 32				/* !!! if this gets changed, it has to be updated on forward_pass.frag !!! */
#define MAX_INDEXES_PER_FRUSTUM (3*1024)		/* 512 triangles... */
#define MAX_INDEXES_PER_GROUP 1536			/* 512 triangles */
#define MAX_TRIANGLES_PER_LIGHT (MAX_INDEXES_PER_FRUSTUM*2)


#define SHADOW_MAP_RESOLUTION 512		/* fixed size shadow maps for all lights... */
#define SHARED_SHADOW_MAP_HEIGHT 8192
#define SHARED_SHADOW_MAP_WIDTH 8192


#define CLUSTERS_PER_ROW 32
#define CLUSTER_ROWS 16
#define CLUSTER_LAYERS 24

#define LIGHT_CLUSTER_UNIFORM_BUFFER_BINDING 3
#define CLUSTER_OFFSET(x, y, z) (x+y*CLUSTERS_PER_ROW+z*CLUSTERS_PER_ROW*CLUSTER_ROWS)


#define LIGHT_RADIUS(radius) (LIGHT_MIN_RADIUS+(LIGHT_MAX_RADIUS-LIGHT_MIN_RADIUS)*((float)((unsigned short)radius)/0xffff))
#define LIGHT_ENERGY(energy) (LIGHT_MIN_ENERGY+(LIGHT_MAX_ENERGY-LIGHT_MIN_ENERGY)*((float)((unsigned short)energy)/0xffff))

#define PACK_CLUSTER_INDEXES(x, y, z) ((z<<24)|(x<<16)|(y))
#define UNPACK_CLUSTER_INDEXES(x, y, z, index) y=((index)&0x000000ff);x=((index>>16)&0x000000ff);z=((index>>24)&0x000000ff)


enum LIGHT_FRUSTUM
{
	LIGHT_FRUSTUM_X_POS = 0,
	LIGHT_FRUSTUM_X_NEG,
	LIGHT_FRUSTUM_Y_POS,
	LIGHT_FRUSTUM_Y_NEG,
	LIGHT_FRUSTUM_Z_POS,
	LIGHT_FRUSTUM_Z_NEG,
};


#endif
