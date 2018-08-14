#ifndef L_MAIN_H
#define L_MAIN_H

#include "matrix_types.h"
#include "vector_types.h"

#include "l_common.h"
#include "bsp_common.h"
#include "camera_types.h"




enum LIGHT_FLAGS
{
	LIGHT_CACHED = 1,
	LIGHT_MOVED = 1 << 1,
	LIGHT_NEEDS_REUPLOAD = 1 << 2,
	LIGHT_GENERATE_SHADOWS = 1 << 3,
	LIGHT_DROPPED_SHADOW = 1 << 4,
	LIGHT_UPDATE_SHADOW_MAP = 1 << 5,
	LIGHT_INVALID = 1 << 6
};


/* to which view (camera) a
set of clusters belong to... */
typedef struct
{
	camera_t *view;
	unsigned int clusters;
}view_clusters_t;


struct light_cluster_t
{
	unsigned x0 : 6;
	unsigned y0 : 5;
	unsigned z0 : 5;

	unsigned x1 : 6;
	unsigned y1 : 5;
	unsigned z1 : 5;
};

typedef struct
{
	mat4_t world_to_light_matrix;
	mat3_t orientation;
	vec3_t position;
}light_position_t;


typedef struct
{
	bsp_dleaf_t *leaf;					/* in which leaf this light is in (updated every time it moves)... */
	vec3_t box_max;								/* this box is calculated when the visible triangles are determined. As long as
												   the light remains inside this box, no update is needed...*/
	vec3_t box_min;

	unsigned char r;
	unsigned char g;
	unsigned char b;
	char cache;									/* which cache slot holds this light... */

	unsigned short radius;
	unsigned short energy;
	unsigned short visible_triangle_count;		/* this could go somewhere else... */
	//unsigned int shadow_map;

	unsigned char bm_flags;
	unsigned char align0;

	short shadow_map;

	short x;
	short y;
	short w;
	short h;

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

	struct light_cluster_t cluster;

}light_params_t;


typedef struct
{
	light_params_t *params;
	light_position_t *position;
}light_ptr_t;

struct gpu_light_t
{

	vec4_t forward_axis;
	vec4_t position_radius;
	vec4_t color_energy;
	int bm_flags;
	int x_y;
	int align0;
	int align1;
};


typedef struct
{
	unsigned int light_indexes_bm;
	/*unsigned int time_stamp;
	int align1;
	int align2;*/
}cluster_t;

typedef struct
{
	short x;
	short y;
	short light_index;
	short align0;
}shadow_map_t;



/*
===================================================================
===================================================================
===================================================================
*/



int light_Init();

void light_Finish();

int light_CreateLight(char *name, mat3_t *orientation, vec3_t position, vec3_t color, float radius, float energy, int bm_flags);

int light_DestroyLight(char *name);

int light_DestroyLightIndex(int light_index);

void light_DestroyAllLights();

int light_Getlight(char *name);

light_ptr_t light_GetLightPointer(char *name);

light_ptr_t light_GetLightPointerIndex(int light_index);



/* updates which leaves contains which lights... */
//void light_MarkLightsOnLeaves();

/* find out which clusters each visible light affect
(and eliminate lights outside the frustum)... */
//void light_LightBounds();

/* update the cluster texture (very time consuming!)... */
//void light_UpdateClusters();

/* create a list of visible lights from the visible
leaves... */
//void light_VisibleLights();

/* update the index list each light keeps to
render its shadow map... */
//void light_VisibleTriangles(int light_index);


void light_ClearLightLeaves();

void light_EntitiesOnLights();

void light_TranslateLight(int light_index, vec3_t direction, float amount);

//void light_SetLight(int light_index);

void light_AllocShadowMap(int light_index);

void light_FreeShadowMap(int light_index);

void light_AllocShadowMaps();


void light_SerializeLights(void **buffer, int *buffer_size);

void light_DeserializeLights(void **buffer);



#endif








