#ifndef ENTITY_H
#define ENTITY_H

#include "ent_common.h"

#include "vector.h"
#include "matrix.h"
#include "model.h"
#include "bsp.h"
#include "physics.h"

#include <limits.h>


struct component_list_t
{
	int component_size;
	int component_count;
	int max_components;
	void *components;
	int free_stack_top;
	int *free_stack;
};

/*
==============================================================
==============================================================
==============================================================
*/

typedef struct
{
	/* this field has to be the first... */
	char tag[20];
	
	
	unsigned int entity_count;
	unsigned int entity_def_count;
	
	
	unsigned int reserved0;
	unsigned int reserved1;
	unsigned int reserved2;
	unsigned int reserved3;
	unsigned int reserved4;
	unsigned int reserved5;
	unsigned int reserved6;
	unsigned int reserved7;
}entity_section_header_t;


typedef struct
{
	vec3_t scale;
	int type;
	int flags;
	
	unsigned int reserved0;
	unsigned int reserved1;
	unsigned int reserved2;
	unsigned int reserved3;
	unsigned int reserved4;
	unsigned int reserved5;
	unsigned int reserved6;
	unsigned int reserved7;
	
	char def_name[ENTITY_DEF_NAME_MAX_LEN];
	char model_name[MODEL_NAME_MAX_LEN];
	char collider_def_name[COLLIDER_DEF_NAME_MAX_LEN];
	
}entity_def_record_t;



typedef struct
{
	mat3_t orientation;
	vec3_t position;
	vec3_t scale;
	int type;
	int flags;
	
	unsigned int reserved0;
	unsigned int reserved1;
	
	char entity_name[ENTITY_NAME_MAX_LEN];
	char entity_def_name[ENTITY_DEF_NAME_MAX_LEN];
}entity_record_t;


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

struct component_list_t entity_CreateComponentList(int component_size, int max_components);

void entity_DestroyComponentList(struct component_list_t *component_list);

int entity_AddComponentToList(struct component_list_t *component_list, void *component);

void entity_RemoveComponentFromList(struct component_list_t *component_list, int index);

//inline void *entity_GetComponentPointer(int component, int component_type, int get_from_def);

__forceinline void *entity_GetComponentPointer(struct component_handle_t component);

__forceinline struct entity_transform_t *entity_GetWorldTransformPointer(struct component_handle_t component);

/*
==============================================================
==============================================================
==============================================================
*/

struct component_handle_t entity_AllocComponent(int component_type, int alloc_for_def);

void entity_DeallocComponent(struct component_handle_t component);

/*
==============================================================
==============================================================
==============================================================
*/

struct entity_handle_t entity_CreateEntityDef(char *name);

void entity_DestroyEntityDef(struct entity_handle_t entity_def);

struct component_handle_t entity_AddComponent(struct entity_handle_t entity, int component_type);

void entity_RemoveComponent(struct entity_handle_t entity, int component_type, int component_index);


void entity_SetModel(struct entity_handle_t entity, int model_index);

void entity_SetCollider(struct entity_handle_t entity, void *collider);

void entity_SetEntityAIScript(struct entity_handle_t entity, void *script);

void entity_SetControllerScript(struct entity_handle_t entity, void *script);

/*
==============================================================
==============================================================
==============================================================
*/


struct entity_handle_t entity_SpawnEntity(mat3_t *orientation, vec3_t position, vec3_t scale, struct entity_handle_t entity_def, char *name);

void entity_RemoveEntity(int entity_index);

struct entity_t *entity_GetEntityPointer(char *name, int get_def);

//struct entity_t *entity_GetEntityPointerIndex(int entity_index);

__forceinline struct entity_t *entity_GetEntityPointerIndex(struct entity_handle_t entity);


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

void entity_UpdateEntityAabbIndex(int entity_index);

void entity_UpdateScriptControllerComponents();

void entity_UpdateTransformComponents();

/*
==============================================================
==============================================================
==============================================================
*/


struct controller_script_t *entity_LoadScript(char *file_name, char *script_name);


/*
==============================================================
==============================================================
==============================================================
*/

void entity_SerializeEntities(void **buffer, int *buffer_size);

void entity_DeserializeEntities(void **buffer);



#include "entity.inl"


#ifdef __cplusplus
}
#endif

#endif





