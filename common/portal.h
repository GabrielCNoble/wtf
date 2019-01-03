#ifndef PORTAL_H
#define PORTAL_H

#include "vector.h"
#include "matrix.h"
#include "camera_types.h"
#include "bsp.h"
#include "w_common.h"

typedef struct
{
	struct view_data_t view_data;
	mat3_t orientation;
	vec3_t position;

	vec4_t near_plane;

	int ref_portal_index;						/* to which portal this view refers to... */
}portal_view_data_t;									/* a portal gets one of those added to itself whenever it gets seen by another view point... */

typedef struct
{
	int views_count;
	int views_max;
	int frame;
	portal_view_data_t *views;
}portal_recursive_view_data_t;							/* each one of those keeps view specific data for each recursive view level. The first level (0) contains
												   		   data refering only to the main view point. Subsequent levels include data from portals that see portals... */

typedef struct
{
	portal_recursive_view_data_t *portal_recursive_views;

	struct bsp_dleaf_t *leaf;
	mat3_t orientation;
	vec3_t position;
	vec2_t extents;
	short linked_portal;
	short ignore;

	short max_recursion;

}portal_t;


#ifdef __cplusplus
extern "C"
{
#endif

int portal_Init();

void portal_Finish();

int portal_CreatePortal(vec3_t position, vec2_t extents, mat3_t *orientation, int max_recursion);

portal_t *portal_GetPortalPointerIndex(int portal_index);

void portal_DestroyPortal(portal_t *portal);

void portal_DestroyPortalIndex(int portal_index);

void portal_LinkPortals(int portal0, int portal1);

/*
==========================================================================
==========================================================================
==========================================================================
*/

void portal_TranslatePortal(int portal_index, vec3_t direction, float amount);

void portal_RotatePortal(int portal_index, vec3_t axis, float amount);

void portal_ScalePortal(int portal_index, vec2_t scale, float amount);

/*
==========================================================================
==========================================================================
==========================================================================
*/

//int portal_CalculateViewMatrix(camera_t *view, portal_t *portal);

int portal_CalculatePortalView(portal_t *portal, mat3_t *view_orientation, vec3_t view_position, int recursion_level, int viewing_portal_index);

void portal_UpdatePortals();


#ifdef __cplusplus
}
#endif

#endif












