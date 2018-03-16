#include "entity.h"
#include "r_main.h"

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ent_entity_list_cursor = 0;
int ent_entity_list_size = 0;
int ent_free_stack_top = -1;
int *ent_free_stack = NULL;
entity_t *ent_entities = NULL;


int ent_entity_def_list_cursor = 0;
int ent_entity_def_list_size = 0;
int ent_entity_def_list_free_stack_top = -1;
int *ent_entity_def_list_free_stack = NULL;
entity_def_t *ent_entity_defs = NULL;


static mat3_t id_rot;

int ent_entity_transform_cursor = 0;
mat4_t *ent_entity_transforms = NULL;
 
int entity_Init()
{
	int i;
	ent_entity_list_size = MAX_ENTITIES;
	//ent_entities = malloc(sizeof(entity_t) * ent_entity_list_size);
	//ent_free_stack = malloc(sizeof(int) * ent_entity_list_size);
	
	
	ent_entities = memory_Malloc(sizeof(entity_t) * ent_entity_list_size, "entity_Init");
	ent_free_stack = memory_Malloc(sizeof(int) * ent_entity_list_size, "entity_Init");
	
	for(i = 0; i < ent_entity_list_size; i++)
	{
		//ent_entities[i].name = malloc(ENTITY_NAME_MAX_LEN);
		ent_entities[i].name = memory_Malloc(ENTITY_NAME_MAX_LEN, "entity_Init");
		ent_entities[i].name[0] = '\0';
	}
	
	ent_entity_def_list_size = MAX_ENTITY_DEFS;
	//ent_entity_defs = malloc(sizeof(entity_def_t) * ent_entity_def_list_size);
	//ent_entity_def_list_free_stack = (malloc(sizeof(int) * ent_entity_def_list_size));
	
	ent_entity_defs = memory_Malloc(sizeof(entity_def_t) * ent_entity_def_list_size, "entity_Init");
	ent_entity_def_list_free_stack = memory_Malloc(sizeof(int) * ent_entity_def_list_size, "entity_Init");
	
	for(i = 0; i < ent_entity_list_size; i++)
	{
		//ent_entity_defs[i].name = malloc(ENTITY_NAME_MAX_LEN);
		ent_entity_defs[i].name = memory_Malloc(ENTITY_NAME_MAX_LEN, "entity_Init");
		ent_entity_defs[i].name[0] = '\0';
	}
	
	
	//ent_entity_transforms = malloc(sizeof(mat4_t ) * ent_entity_list_size);
	ent_entity_transforms = memory_Malloc(sizeof(mat4_t ) * ent_entity_list_size, "entity_Init");
	
	id_rot = mat3_t_id();	
	
	return 1;
}

void entity_Finish()
{
	int i;
	
	for(i = 0; i < MAX_ENTITIES; i++)
	{
		//free(ent_entities[i].name);
		memory_Free(ent_entities[i].name);
	}
	
	for(i = 0; i < MAX_ENTITY_DEFS; i++)
	{
		memory_Free(ent_entity_defs[i].name);
		//free(ent_entity_defs[i].name);
	}
	
		
	//free(ent_entities);
	//free(ent_free_stack);
	//free(ent_entity_transforms);
	
	memory_Free(ent_entities);
	memory_Free(ent_free_stack);
	memory_Free(ent_entity_transforms);
	
	
	//free(ent_entity_defs);
	//free(ent_entity_def_list_free_stack);
	
	memory_Free(ent_entity_defs);
	memory_Free(ent_entity_def_list_free_stack);
	
}

int entity_CreateEntityDef(char *name, int type, int model_index)
{
	int entity_def_index;
	int name_len = 0;
	entity_def_t *entity_def;
	
	if(model_index < 0)
	{
		printf("entity_CreateEntityDef: bad model index for entity def [%s]!\n", name);
		return -1;
	}
	
	if(ent_entity_def_list_free_stack_top > -1)
	{
		entity_def_index = ent_entity_def_list_free_stack[ent_entity_def_list_free_stack_top];
		ent_free_stack_top--;
	}
	else
	{
		entity_def_index = ent_entity_def_list_cursor++;
		
		if(entity_def_index >= MAX_ENTITY_DEFS)
		{
			printf("entity_CreateEntityDef: no more entities!\n");
			return -1;
		}
		
	}
	
	entity_def = &ent_entity_defs[entity_def_index];

	entity_def->model_index = model_index;	
	//entity_def->name = malloc(512);
	entity_def->flags = 0;
	entity_def->type = type;
	
	if(name)
	{
		name_len = strlen(name) + 1;
	
		if(name_len >= ENTITY_NAME_MAX_LEN)
		{
			name_len = ENTITY_NAME_MAX_LEN - 1;
		}
		
		memcpy(entity_def->name, name, name_len);
	}
	
	entity_def->name[name_len] = '\0';
	
	//printf("entity def: %s with model %d\n", entity_def->name, entity_def->model_index);
	
	return entity_def_index;
}

int entity_DestroyEntityDef(entity_def_t *entity_def)
{
	int entity_def_index;
	if(entity_def)
	{
		entity_def_index = entity_def - ent_entity_defs;
		
		if(entity_def_index >= 0 && entity_def_index < ent_entity_def_list_cursor)
		{
			if(entity_def->type != ENTITY_TYPE_INVALID)
			{
				entity_def->type = ENTITY_TYPE_INVALID;
				entity_def->model_index = -1;
				
				ent_entity_def_list_free_stack_top++;
				ent_entity_def_list_free_stack[ent_entity_def_list_free_stack_top] = entity_def_index;
				
				return 1;
			}
		}
	}
	
	return 0;
}

int entity_DestroyEntityDefIndex(int entity_def_index)
{
	entity_def_t *entity_def;
	
	if(entity_def_index >= 0 && entity_def_index < ent_entity_def_list_cursor)
	{
		entity_def = &ent_entity_defs[entity_def_index];
		
		if(entity_def->type != ENTITY_TYPE_INVALID)
		{
			entity_def->type = ENTITY_TYPE_INVALID;
			entity_def->model_index = -1;
			
			ent_entity_def_list_free_stack_top++;
			
			ent_entity_def_list_free_stack[ent_entity_def_list_free_stack_top] = entity_def_index;
			
			return 1;
		}
	}
	
	return 0;
}

int entity_GetEntityDef(char *name)
{
	int i;
	
	
	for(i = 0; i < ent_entity_def_list_cursor; i++)
	{
		if(ent_entity_defs[i].type == ENTITY_INVALID)
			continue;
			
			
		if(!strcmp(name, ent_entity_defs[i].name))
		{
			return i;
		}
	}
	
	return -1;
}

entity_def_t *entity_GetEntityDefPointerIndex(int index)
{
	if(index >= 0 && index < ent_entity_def_list_cursor)
	{
		if(ent_entity_defs[index].type != ENTITY_TYPE_INVALID)
		{
			return &ent_entity_defs[index];
		}
	}
	
	return NULL;
}

int entity_CreateEntity(char *name, vec3_t position, vec3_t scale, mat3_t *orientation, int def_index)
{
	int entity_index;
	int name_len = 0;
	entity_t *entity;
	entity_def_t *def;
	
	if(def_index < 0)
	{
		printf("entity_CreateEntity: bad entity def index for entity [%s]!\n", name);
		return -1;
	}
	
	if(ent_free_stack_top > -1)
	{
		entity_index = ent_free_stack[ent_free_stack_top];
		ent_free_stack_top--;
	}
	else
	{
		entity_index = ent_entity_list_cursor++;
		
		if(entity_index >= MAX_ENTITIES)
		{
			printf("entity_CreateEntity: no more entities!\n");
			return -1;
		}
		
	}
	
	entity = &ent_entities[entity_index];
	def = &ent_entity_defs[def_index];
	
	entity->leaf = NULL;
	entity->model_index = def->model_index;
	entity->position = position;
	entity->scale = scale;
	entity->flags = 0;
	entity->skin_gpu_handle = -1;
	
	model_IncModelMaterialsRefs(entity->model_index);
	
	if(!orientation)
	{
		entity->orientation = id_rot;
	}
	else
	{
		entity->orientation = *orientation;
	}
	
	if(name)
	{
		name_len = strlen(name) + 1;
	
		if(name_len >= ENTITY_NAME_MAX_LEN)
		{
			name_len = ENTITY_NAME_MAX_LEN - 1;
		}
		
		memcpy(entity->name, name, name_len);
	}
	
	entity->name[name_len] = '\0';
	
	return entity_index;
}


int entity_DestroyEntity(char *name)
{
	int i;
	
	for(i = 0; i < ent_entity_list_cursor; i++)
	{
		if(ent_entities[i].flags & ENTITY_INVALID)
			continue;
			
		if(!strcmp(name, ent_entities[i].name))
		{
			return entity_DestroyEntityIndex(i);
		}	
	}
	
	return 0;
}

int entity_DestroyEntityIndex(int entity_index)
{
	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
	{
		if(!(ent_entities[entity_index].flags & ENTITY_INVALID))
		{
			ent_entities[entity_index].name[0] = '\0';
			ent_entities[entity_index].flags |= ENTITY_INVALID;
			
			model_DecModelMaterialsRefs(ent_entities[entity_index].model_index);
			
			ent_entities[entity_index].model_index = -1;
			ent_entities[entity_index].leaf = NULL;
			
			return 0;
		}
	}
	
	return 1;
}

int entity_GetEntity(char *name)
{
	
}

entity_t *entity_GetEntityPointer(char *name)
{
	
}

entity_t *entity_GetEntityPointerIndex(int entity_index)
{
	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
	{
		if(!(ent_entities[entity_index].flags & ENTITY_INVALID))
		{
			return &ent_entities[entity_index];
		}
	}
	
	return NULL;
}



int entity_LoadModel(char *file_name, char *model_name, char *entity_def_name, int type)
{
	int model_index = -1;
	int entity_def_index = -1;
	
	model_index = model_LoadModel(file_name, model_name);
	if(model_index < 0)
	{
		printf("entity_LoadModel: bad model index!\n");
		return -1;
	}
	
	return entity_CreateEntityDef(entity_def_name, type, model_index);
}



void entity_TranslateEntity(int entity_index, vec3_t direction, float amount)
{
	entity_t *entity;
	
	
	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
	{
		entity = &ent_entities[entity_index];
		
		if(!(entity->flags & ENTITY_INVALID))
		{
			
			entity->position.x += direction.x * amount;
			entity->position.y += direction.y * amount;
			entity->position.z += direction.z * amount;
			
			entity->flags |= ENTITY_HAS_MOVED;
		}
	}
	
}

void entity_RotateEntity(int entity_index, vec3_t axis, float amount)
{
	entity_t *entity;
	
	
	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
	{
		entity = &ent_entities[entity_index];
		
		if(!(entity->flags & ENTITY_INVALID))
		{			
			mat3_t_rotate(&entity->orientation, axis, amount, 0);
			entity->flags |= ENTITY_HAS_MOVED;
		}
	}
}

#define ENTITY_MIN_SCALE 0.01

void entity_ScaleEntity(int entity_index, vec3_t axis, float amount)
{
	entity_t *entity;
	
	
	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
	{
		entity = &ent_entities[entity_index];
		
		if(!(entity->flags & ENTITY_INVALID))
		{			
			//mat3_t_rotate(&entity->orientation, axis, amount, 0);
			
			entity->scale.x += axis.x * amount;
			entity->scale.y += axis.y * amount;
			entity->scale.z += axis.z * amount;
				
			if(entity->scale.x < ENTITY_MIN_SCALE) entity->scale.x = ENTITY_MIN_SCALE;
			if(entity->scale.y < ENTITY_MIN_SCALE) entity->scale.y = ENTITY_MIN_SCALE;
			if(entity->scale.z < ENTITY_MIN_SCALE) entity->scale.z = ENTITY_MIN_SCALE;
			
			//printf("[%f %f %f]\n", entity->scale.x, entity->scale.y, entity->scale.z);
			
			entity->flags |= ENTITY_HAS_MOVED;
		}
	}
}




extern model_t *models;

void entity_UpdateEntities()
{
	int i;
	int j;
	mat4_t *transform;
	entity_t *entity;
	model_t *model;
	ent_entity_transform_cursor = 0;
	
	for(i = 0; i < ent_entity_list_cursor; i++)
	{
		if(ent_entities[i].flags & ENTITY_INVALID)
			continue;
			
		if(ent_entities[i].flags & ENTITY_INVISIBLE)
			continue;	
		
		entity = &ent_entities[i];
		
		transform = &ent_entity_transforms[ent_entity_transform_cursor];
		mat4_t_compose(transform, &entity->orientation, entity->position);
		/*mat4_t_compose(transform, NULL, vec3(0.0, 0.0, 0.0));
		
		transform.floats[0]*/
		
		transform->floats[0][0] *= entity->scale.x;
		transform->floats[0][1] *= entity->scale.x;
		transform->floats[0][2] *= entity->scale.x;
		
		
		transform->floats[1][0] *= entity->scale.y;
		transform->floats[1][1] *= entity->scale.y;
		transform->floats[1][2] *= entity->scale.y;
		
		
		transform->floats[2][0] *= entity->scale.z;
		transform->floats[2][1] *= entity->scale.z;
		transform->floats[2][2] *= entity->scale.z;
		
		/*mat4_t_scale(transform, vec3(1.0, 0.0, 0.0), entity->scale.x);
		mat4_t_scale(transform, vec3(0.0, 1.0, 0.0), entity->scale.y);
		mat4_t_scale(transform, vec3(0.0, 0.0, 1.0), entity->scale.z);*/
		
		
		ent_entity_transform_cursor++;	
		
		model = models + entity->model_index;
		
		for(j = 0; j < model->triangle_group_count; j++)
		{
			renderer_SubmitDrawCommand(transform, model->draw_mode, model->vert_start + model->triangle_groups[j].start, model->triangle_groups[j].next, model->triangle_groups[j].material_index);
		}
		
		
	}
}

void entity_SetInvisible(int entity_index)
{
	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
	{
		if(!(ent_entities[entity_index].flags & ENTITY_INVALID))
		{
			ent_entities[entity_index].flags |= ENTITY_INVISIBLE;
		}
	}
}

void entity_SetVisible(int entity_index)
{
	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
	{
		if(!(ent_entities[entity_index].flags & ENTITY_INVALID))
		{
			ent_entities[entity_index].flags &= ~ENTITY_INVISIBLE;
		}
	}
}














