#ifndef ENTITY_INL
#define ENTITY_INL

#include <stdio.h>

#include "ent_common.h"
#include "stack_list.h"

extern struct stack_list_t ent_components[2][COMPONENT_TYPE_LAST];

extern int ent_entity_list_cursor;
extern struct stack_list_t ent_entities[2];
extern int ent_entity_def_list_cursor;
extern struct entity_t *ent_entity_defs;

extern struct stack_list_t ent_world_transforms;
extern struct stack_list_t ent_entity_aabbs;

__forceinline void *entity_GetComponentPointer(struct component_handle_t component)
{
	struct stack_list_t *list;
	if(component.type != COMPONENT_TYPE_NONE)
	{
		list = &ent_components[component.def][component.type];
		return (void *)((char *)list->elements + list->element_size * component.index);
	}
	return NULL;
}

__forceinline void *entity_GetComponentPointerIndex(int index, int type, int def)
{
	struct component_handle_t handle;
	
	def = def && 1;
	
	handle.def = def;
	handle.type = type;
	handle.index = index;
	
	return entity_GetComponentPointer(handle);
}

__forceinline struct entity_transform_t *entity_GetWorldTransformPointer(struct component_handle_t component)
{
	return (struct entity_transform_t *)((char *)ent_world_transforms.elements + ent_world_transforms.element_size * component.index);
}

__forceinline struct entity_aabb_t *entity_GetAabbPointer(struct component_handle_t component)
{
	return (struct entity_aabb_t *)((char *)ent_entity_aabbs.elements + ent_entity_aabbs.element_size * component.index);
}

__forceinline struct entity_t *entity_GetEntityPointerHandle(struct entity_handle_t entity)
{
	struct entity_t *entity_ptr;
	int cursor;

	if(entity.entity_index != INVALID_ENTITY_INDEX)
	{
		if(entity.entity_index >= 0 && entity.entity_index < ent_entities[entity.def].element_count)
		{
			entity_ptr = (struct entity_t *)ent_entities[entity.def].elements + entity.entity_index;
			
			if(!(entity_ptr->flags & ENTITY_INVALID))
			{
				return entity_ptr;
			}
		}
	}
	
	return NULL;
}

__forceinline struct entity_t *entity_GetEntityPointerIndex(int entity_index)
{
	struct entity_t *entity_ptr;
	int cursor;

	if(entity_index != INVALID_ENTITY_INDEX)
	{
		if(entity_index >= 0 && entity_index < ent_entities[0].element_count)
		{
			entity_ptr = (struct entity_t *)ent_entities[0].elements + entity_index;
			
			if(!(entity_ptr->flags & ENTITY_INVALID))
			{
				return entity_ptr;
			}
		}
	}
	
	return NULL;
}

__forceinline struct entity_t *entity_GetEntityDefPointerIndex(int entity_def_index)
{
	struct entity_t *entity_ptr;
	int cursor;

	if(entity_def_index != INVALID_ENTITY_INDEX)
	{
		if(entity_def_index >= 0 && entity_def_index < ent_entities[1].element_count)
		{
			entity_ptr = (struct entity_t *)ent_entities[1].elements + entity_def_index;
			
			if(!(entity_ptr->flags & ENTITY_INVALID))
			{
				return entity_ptr;
			}
		}
	}
	
	return NULL;
}

__forceinline struct entity_handle_t entity_GetEntityHandle(char *name, int get_def)
{
	int i;
	int c;
	struct entity_t *entity;
	struct entity_handle_t handle;
	
	get_def = get_def && 1;
	
	handle.def = 0;
	handle.entity_index = INVALID_ENTITY_INDEX;
	
	c = ent_entities[get_def].element_count;
	
	for(i = 0; i < c; i++)
	{
		entity = (struct entity_t *)ent_entities[get_def].elements + i;
		
		if(entity->flags & ENTITY_INVALID)
		{
			continue;
		}
		
		if(!strcmp(entity->name, name))
		{
			handle.def = get_def;
			handle.entity_index = i;
			break;
		}
		
	}
	
	return handle;
}

#endif






