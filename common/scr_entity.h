#ifndef SCR_ENTITY_H
#define SCR_ENTITY_H

#include "ent_common.h"
#include "vector.h"

#ifdef __cplusplus
extern "C"
{
#endif



/*
=====================================
=====================================
=====================================
*/

void entity_ScriptMove(vec3_t *direction);

void entity_ScriptJump(float jump_force);

/*
=====================================
=====================================
=====================================
*/


void *entity_ScriptGetPosition();

void *entity_ScriptGetOrientation();

void *entity_ScriptGetForwardVector();

void entity_ScriptRotate(vec3_t *axis, float angle, int set);

/*
=====================================
=====================================
=====================================
*/

struct component_handle_t entity_ScriptGetComponent(int component_index);

struct component_handle_t entity_ScriptGetEntityComponent(struct entity_handle_t entity, int component_index);

void entity_ScriptSetCameraPosition(vec3_t *position);

void entity_ScriptSetCamera();

/*
=====================================
=====================================
=====================================
*/


void entity_ScriptFindPath(vec3_t *to);

void entity_ScriptGetWaypointDirection(vec3_t *direction);

void entity_ScriptAdvanceWaypoint();


#ifdef __cplusplus
}
#endif





#endif
