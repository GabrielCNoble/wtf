#ifndef ENTITY_H
#define ENTITY_H

#include "ent_common.h"

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



__forceinline void *entity_GetComponentPointer(struct component_handle_t component);

__forceinline void *entity_GetComponentPointerIndex(int index, int type, int def);

__forceinline struct entity_transform_t *entity_GetWorldTransformPointer(struct component_handle_t component);

__forceinline struct entity_aabb_t *entity_GetAabbPointer(struct component_handle_t component);

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



void entity_ParentEntity(struct entity_handle_t parent, struct entity_handle_t child);

void entity_UnparentEntity(struct entity_handle_t parent, struct entity_handle_t child);

/*
==============================================================
==============================================================
==============================================================
*/

struct entity_handle_t entity_CreateEntityDef(char *name);

void entity_DestroyEntityDef(struct entity_handle_t entity_def);

struct component_handle_t entity_AddComponent(struct entity_handle_t entity, int component_type);

void entity_RemoveComponent(struct entity_handle_t entity, int component_type, int component_index);



void entity_AddProp(struct entity_handle_t entity, char *name, int size);

void entity_RemoveProp(struct entity_handle_t entity, char *name);

void entity_SetProp(struct entity_handle_t entity, char *name, void *value);

void entity_GetProp(struct entity_handle_t entity, char *name, void *value);



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

/*
==============================================================
==============================================================
==============================================================
*/

struct entity_handle_t entity_CreateEntity(char *name, int def);

struct entity_handle_t entity_SpawnEntity(mat3_t *orientation, vec3_t position, vec3_t scale, struct entity_handle_t entity_def, char *name);

void entity_RemoveEntity(struct entity_handle_t entity);

void entity_RemoveAllEntities();

struct entity_t *entity_GetEntityPointer(char *name, int get_def);

__forceinline struct entity_t *entity_GetEntityPointerHandle(struct entity_handle_t entity);

__forceinline struct entity_t *entity_GetEntityPointerIndex(int entity_index);

__forceinline struct entity_t *entity_GetEntityDefPointerIndex(int entity_def_index);

__forceinline struct entity_handle_t entity_GetEntityHandle(char *name, int get_def);


/*
==============================================================
==============================================================
==============================================================
*/

void entity_TranslateEntity(int entity_index, vec3_t direction, float amount);

void entity_RotateEntity(int entity_index, vec3_t axis, float amount);

void entity_ScaleEntity(int entity_index, vec3_t axis, float amount);

void entity_FindPath(struct entity_handle_t entity, vec3_t to);

/*
==============================================================
==============================================================
==============================================================
*/

void entity_UpdateEntityAabbIndex(struct entity_handle_t entity);

void entity_UpdateScriptComponents();

void entity_UpdateTransformComponents();

void entity_UpdateCameraComponents();

//void entity_Update

void entity_UpdateEntities();

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





#include "entity.inl"


#ifdef __cplusplus
}
#endif

#endif





