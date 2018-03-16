#ifndef L_MAIN_H
#define L_MAIN_H

#include "matrix_types.h"
#include "vector_types.h"

#include "l_common.h"
#include "bsp_common.h"




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


typedef struct
{
	mat4_t world_to_light_matrix;
	mat3_t orientation;
	vec3_t position;
}light_position_t;


typedef struct
{
	struct bsp_dleaf_t *leaf;					/* in which leaf this light is in (updated every time it moves)... */
	vec3_t box_max;						/* this box is calculated when the visible triangles are determined. As long as
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
	
	
	
	unsigned int first_cluster;
	unsigned int last_cluster;
}light_params_t;

typedef struct
{
	
	vec4_t forward_axis;
	vec4_t position_radius;
	vec4_t color_energy;
	int bm_flags;
	int x_y;
	int align0;
	int align1;
}gpu_lamp_t;

/* !!! if this gets changed, it has to be updated on shade_pass.frag !!! */
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


int light_Init();

void light_Finish();

int light_CreateLight(char *name, mat3_t *orientation, vec3_t position, vec3_t color, float radius, float energy, int bm_flags);

int light_DestroyLight(char *name);

int light_DestroyLightIndex(int light_index);

void light_DestroyAllLights();

/* updates which leaves contains which lights... */
void light_UpdateLights();

/* find out which clusters each visible light affect 
(and eliminate lights outside the frustum)... */
void light_LightBounds();

/* update the cluster texture (very time consuming!)... */
void light_UpdateClusters();

/* create a list of visible lights from the visible
leaves... */
void light_VisibleLights();

/* update the index list each light keeps to
render its shadow map... */
void light_VisibleTriangles(int light_index);


void light_ClearLightLeaves();

void light_TranslateLight(int light_index, vec3_t direction, float amount);

void light_SetLight(int light_index);

void light_AllocShadowMap(int light_index);

void light_FreeShadowMap(int light_index);

void light_AllocShadowMaps();



int light_ClusterThread0(void *data);

int light_ClusterThread1(void *data);



#endif








