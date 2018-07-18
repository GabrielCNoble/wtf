#ifndef SCR_ENTITY_H
#define SCR_ENTITY_H

#include "ent_common.h"
#include "scr_string.h"
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

void entity_ScriptDie();

void entity_ScriptDIEYOUMOTHERFUCKER();

/*
=====================================
=====================================
=====================================
*/


void entity_ScriptGetPosition(vec3_t *position, int local);

void *entity_ScriptGetOrientation();

void *entity_ScriptGetForwardVector();

void entity_ScriptRotate(vec3_t *axis, float angle, int set);

int entity_ScriptGetLife();

/*
=====================================
=====================================
=====================================
*/

struct component_handle_t entity_ScriptGetComponent(int component_index);

struct component_handle_t entity_ScriptGetEntityComponent(struct entity_handle_t entity, int component_index);

struct entity_handle_t entity_ScriptGetEntity(struct script_string_t *name, int get_def);

struct entity_handle_t entity_ScriptGetEntityDef(struct script_string_t *def_name);

struct entity_handle_t entity_ScriptSpawnEntity(mat3_t *orientation, vec3_t *position, vec3_t *scale, struct entity_handle_t def, struct script_string_t *name);




void entity_ScriptSetComponentValue33f(struct component_handle_t component, struct script_string_t *field_name, mat3_t *value);

void entity_ScriptSetComponentValue3f(struct component_handle_t component, struct script_string_t *field_name, vec3_t *value);

/*
=====================================
=====================================
=====================================
*/

void entity_ScriptAddEntityProp(struct entity_handle_t entity, struct script_string_t *name, void *type_info);

void entity_ScriptAddEntityProp3f(struct entity_handle_t entity, struct script_string_t *name);

void entity_ScriptRemoveEntityProp3f(struct entity_handle_t entity, struct script_string_t *name);

void entity_ScriptGetEntityPropValue3f(struct entity_handle_t entity, struct script_string_t *name, vec3_t *value);

void entity_ScriptSetEntityPropValue3f(struct entity_handle_t entity, struct script_string_t *name, vec3_t *value);

/*
=====================================
=====================================
=====================================
*/


void entity_ScriptSetCameraPosition(vec3_t *position);

//void entity_ScriptSetCamera();

void entity_ScriptSetCamera(struct component_handle_t camera);

/*
=====================================
=====================================
=====================================
*/


void entity_ScriptFindPath(vec3_t *to);

void entity_ScriptGetWaypointDirection(vec3_t *direction);

void entity_ScriptAdvanceWaypoint();

void entity_ScriptPrint(struct script_string_t *script_string);

#ifdef __cplusplus
}
#endif





#endif
