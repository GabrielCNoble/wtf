#ifndef ENTITY_H
#define ENTITY_H

#include "vector.h"
#include "matrix.h"
#include "model.h"
#include "bsp.h"

#define ENTITY_NAME_MAX_LEN 512				/* including trailing null... */
#define MAX_ENTITIES 1024
#define MAX_ENTITY_DEFS 1024


enum ENTITY_TYPE
{
	ENTITY_TYPE_INVALID,
	ENTITY_TYPE_STATIC, 
	ENTITY_TYPE_MOVABLE,
};

enum ENTITY_DEF_FLAGS
{
	ENTITY_DEF_ANIMATED = 1,
};

enum ENTITY_FLAGS
{
	ENTITY_INVALID = 1,
	ENTITY_HAS_MOVED = 1 << 1,
	ENTITY_INVISIBLE = 1 << 2,
	ENTITY_ANIMATED = 1 << 3,
};

typedef struct
{
	int model_index;
	int type;
	int flags;
	char *name;
}entity_def_t;

typedef struct
{
	mat3_t orientation;
	vec3_t position;
	vec3_t scale;
	int model_index;
	unsigned int skin_gpu_handle;
	int flags;
	char *name;
	bsp_dleaf_t *leaf;						/* the leaf that contains the entity's origin... */
}entity_t;

int entity_Init();

void entity_Finish();

int entity_CreateEntityDef(char *name, int type, int model_index);

int entity_DestroyEntityDef(entity_def_t *entity_def);

int entity_DestroyEntityDefIndex(int entity_def_index);

int entity_GetEntityDef(char *name);

entity_def_t *entity_GetEntityDefPointerIndex(int index);



int entity_CreateEntity(char *name, vec3_t position, vec3_t scale, mat3_t *orientation, int def_index);

int entity_DestroyEntity(char *name);

int entity_DestroyEntityIndex(int entity_index);

int entity_GetEntity(char *name);

entity_t *entity_GetEntityPointer(char *name);

entity_t *entity_GetEntityPointerIndex(int entity_index);



int entity_LoadModel(char *file_name, char *model_name, char *entity_def_name, int type);



void entity_TranslateEntity(int entity_index, vec3_t direction, float amount);

void entity_RotateEntity(int entity_index, vec3_t axis, float amount);

void entity_ScaleEntity(int entity_index, vec3_t axis, float amount);

/* mark transform entities, and
mark which leaves they overlap... */
void entity_UpdateEntities();

void entity_SetInvisible(int entity_index);

void entity_SetVisible(int entity_index);



#endif





