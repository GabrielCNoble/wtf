#ifndef NAVIGATION_H
#define NAVIGATION_H


#include "nav_common.h"


#ifdef __cplusplus
extern "C"
{
#endif

int navigation_Init();

void navigation_Finish();

/*
=================================================================
=================================================================
=================================================================
*/


int navigation_CreateWaypoint(vec3_t position);

void navigation_DestroyWaypoint(int waypoint);

struct waypoint_t *navigation_GetWaypointPointer(int waypoint_index);

void navigation_LinkWaypoints(int waypoint_a, int waypoint_b);

void navigation_UnlinkWaypoints(int waypoint_a, int waypoint_b);

void navigation_BuildLinks();

void navigation_UpdateWaypoint(int waypoint_index);

void navigation_MoveWaypoint(int waypoint_index, vec3_t direction, float amount);


/*
=================================================================
=================================================================
=================================================================
*/


struct waypoint_t *navigation_GetClosestWaypoint(vec3_t position);

struct waypoint_t **navigation_FindPath(int *waypoint_count, vec3_t from, vec3_t to);

#ifdef __cplusplus
}
#endif




#endif
