#ifndef ENTITY_H
#define ENTITY_H

#include "ent_common.h"

#include "macros.h"

#include "vector.h"
#include "matrix.h"
#include "model.h"
#include "bsp.h"
#include "physics.h"

#include <limits.h>


/*
==============================================================
==============================================================
==============================================================
*/


/*
==============================================================
==============================================================
==============================================================
*/

#ifdef __cplusplus
extern "C"
{
#endif


int entity_Init();

void entity_Finish();

/*
==============================================================
==============================================================
==============================================================
*/



static ALWAYS_FORCE_INLINE void *entity_GetComponentPointer(struct component_handle_t component);

static ALWAYS_FORCE_INLINE void *entity_GetComponentPointerIndex(int index, int type, int def);

static ALWAYS_FORCE_INLINE struct entity_transform_t *entity_GetWorldTransformPointer(struct component_handle_t component);

static ALWAYS_FORCE_INLINE struct entity_aabb_t *entity_GetAabbPointer(struct component_handle_t component);


/*
==============================================================
==============================================================
==============================================================
*/

struct component_handle_t entity_AllocComponent(int component_type, int alloc_for_def);

void entity_DeallocComponent(struct component_handle_t component);




void entity_AddTransformToTopList(struct component_handle_t transform);

void entity_RemoveTransformFromTopList(struct component_handle_t transform);



void entity_ParentTransformComponent(struct component_handle_t parent_transform, struct component_handle_t child_transform);

void entity_UnparentTransformComponent(struct component_handle_t parent_transform, struct component_handle_t child_transform);


void entity_ParentEntityToEntityTransform(struct component_handle_t parent_transform, struct entity_handle_t child);

void entity_UnpparentEntityFromEntityTransform(struct component_handle_t parent_transform, struct entity_handle_t child);

void entity_ParentEntity(struct entity_handle_t parent, struct entity_handle_t child);

void entity_UnparentEntity(struct entity_handle_t parent, struct entity_handle_t child);

void entity_ParentEntityToBone(struct component_handle_t skeleton_component, int bone_index, struct entity_handle_t entity);

void entity_UnparentEntityToBone(struct component_handle_t skeleton_component, struct entity_handle_t entity);

/*
==============================================================
==============================================================
==============================================================
*/

struct entity_handle_t entity_CreateEntityDef(char *name);

void entity_DestroyEntityDef(struct entity_handle_t entity_def);

struct component_handle_t entity_AddComponent(struct entity_handle_t entity, int component_type);

void entity_RemoveComponent(struct entity_handle_t entity, int component_type);



int entity_AddProp(struct entity_handle_t entity, char *name, int size);

void entity_AddPropTyped(struct entity_handle_t entity, char *name, int type);

void entity_AddProp1i(struct entity_handle_t entity, char *name);

void entity_AddProp1f(struct entity_handle_t entity, char *name);

void entity_AddProp2f(struct entity_handle_t entity, char *name);

void entity_AddProp3f(struct entity_handle_t entity, char *name);

void entity_AddProp4f(struct entity_handle_t entity, char *name);

void entity_AddProp22f(struct entity_handle_t entity, char *name);

void entity_AddProp33f(struct entity_handle_t entity, char *name);

void entity_AddProp44f(struct entity_handle_t entity, char *name);


void entity_RemoveProp(struct entity_handle_t entity, char *name);

void entity_SetProp(struct entity_handle_t entity, char *name, void *value);

void entity_GetProp(struct entity_handle_t entity, char *name, void *value);

struct entity_prop_t *entity_GetPropPointer(struct entity_handle_t entity, char *name);

struct entity_prop_t *entity_GetPropPointerIndex(struct entity_handle_t entity, int prop_index);



/*
==============================================================
==============================================================
==============================================================
*/

void entity_SetModel(struct entity_handle_t entity, int model_index);

void entity_SetCollider(struct entity_handle_t entity, void *collider);

void entity_SetScript(struct entity_handle_t entity, struct script_t *script);

void entity_SetTransform(struct entity_handle_t entity, mat3_t *orientation, vec3_t position, vec3_t scale, int clear_aabb);

//void entity_SetControllerScript(struct entity_handle_t entity, void *script);

void entity_SetCameraTransform(struct entity_handle_t entity, mat3_t *orientation, vec3_t position);

void entity_SetSkeleton(struct entity_handle_t entity, struct skeleton_handle_t skeleton);

/*
==============================================================
==============================================================
==============================================================
*/

struct entity_handle_t entity_CreateEntity(char *name, int def);

struct entity_handle_t entity_SpawnEntity(mat3_t *orientation, vec3_t position, vec3_t scale, struct entity_handle_t entity_def, char *name);

void entity_MarkForRemoval(struct entity_handle_t entity);

void entity_RemoveEntity(struct entity_handle_t entity);

void entity_RemoveAllEntities();

void entity_RemoveMarkedEntities();

void entity_ResetEntitySpawnTimes();

struct entity_t *entity_GetEntityPointer(char *name, int get_def);

static ALWAYS_FORCE_INLINE struct entity_t *entity_GetEntityPointerHandle(struct entity_handle_t entity);

static ALWAYS_FORCE_INLINE struct entity_t *entity_GetEntityParentPointerHandle(struct entity_handle_t entity);

static ALWAYS_FORCE_INLINE struct entity_t *entity_GetEntityPointerIndex(int entity_index);

static ALWAYS_FORCE_INLINE struct entity_t *entity_GetEntityDefPointerIndex(int entity_def_index);

static ALWAYS_FORCE_INLINE struct entity_handle_t entity_GetEntityHandle(char *name, int get_def);

static ALWAYS_FORCE_INLINE struct entity_handle_t entity_GetNestledEntityHandle(struct entity_handle_t parent_entity, char *entity);

struct entity_source_file_t *entity_GetSourceFile(struct entity_handle_t entity);


/*
==============================================================
==============================================================
==============================================================
*/

void entity_TranslateEntity(struct entity_handle_t entity, vec3_t direction, float amount);

void entity_SetEntityPosition(struct entity_handle_t entity, vec3_t position);


void entity_RotateEntity(struct entity_handle_t entity, vec3_t axis, float amount, int set);

void entity_SetEntityOrientation(struct entity_handle_t entity, mat3_t *orientation);


void entity_ScaleEntity(int entity_index, vec3_t axis, float amount);

void entity_FindPath(struct entity_handle_t entity, vec3_t to);

struct entity_handle_t *entity_GetTouchedEntities(struct entity_handle_t entity, int *count);

void entity_TouchedEntities(struct entity_handle_t entity);

struct entity_contact_t *entity_GetEntityContacts(struct entity_handle_t entity);

/*
==============================================================
==============================================================
==============================================================
*/

void entity_UpdateAabb(struct entity_handle_t entity);

void entity_AabbWorldExtents(struct entity_handle_t entity, vec3_t *extent_max, vec3_t *extent_min);

void entity_UpdateScriptComponents();

void entity_UpdatePhysicsComponents();

void entity_UpdateTransformComponents();

void entity_UpdateCameraComponents();

void entity_UpdateEntities();

int entity_LineOfSightToEntity(struct entity_handle_t from, struct entity_handle_t to);

/*
==============================================================
==============================================================
==============================================================
*/


int entity_CreateTrigger(mat3_t *orientation, vec3_t position, vec3_t scale, char *event_name, char *name);

struct trigger_t *entity_GetTriggerPointerIndex(int trigger_index);

int entity_GetTrigger(char *name);

int entity_IsTriggered(int trigger_index);

void entity_DestroyTriggerIndex(int trigger_index);

void entity_AddTriggerFilter(int trigger_index, char *filter_name);

void entity_RemoveTriggerFilter(int trigger_index, char *filter_name);

void entity_UpdateTriggers();

void entity_SetTriggerPosition(int trigger_index, vec3_t position);

void entity_TranslateTrigger(int trigger_index, vec3_t direction, float amount);

void entity_ScaleTrigger(int trigger_index, vec3_t axis, float amount);

void entity_DestroyAllTriggers();



/*
==============================================================
==============================================================
==============================================================
*/


struct entity_script_t *entity_LoadScript(char *file_name, char *script_name);


/*
==============================================================
==============================================================
==============================================================
*/

void entity_EmitDrawCmdsForEntity(struct entity_handle_t entity, mat4_t *transform);

/*
==============================================================
==============================================================
==============================================================
*/


void entity_SaveEntityDef(char *file_name, struct entity_handle_t entity_def);

struct entity_handle_t entity_LoadEntityDef(char *file_name);



/*
==============================================================
==============================================================
==============================================================
*/


#include "entity.inl"


#ifdef __cplusplus
}
#endif

#endif





