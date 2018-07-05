#ifndef ENTITY_INL
#define ENTITY_INL

#include <stdio.h>

#include "ent_common.h"

extern struct component_list_t ent_components[2][COMPONENT_TYPE_LAST];

extern int ent_entity_list_cursor;
extern struct entity_t *ent_entities;
extern int ent_entity_def_list_cursor;
extern struct entity_t *ent_entity_defs;

extern struct entity_transform_t *ent_global_transforms;

__forceinline void *entity_GetComponentPointer(struct component_handle_t component)
{
	struct component_list_t *list;
	if(component.type != COMPONENT_TYPE_NONE)
	{
		list = &ent_components[component.def][component.type];
		return (void *)((char *)list->components + list->component_size * component.index);
	}
	return NULL;
}

__forceinline struct entity_transform_t *entity_GetWorldTransformPointer(struct component_handle_t component)
{
	return ent_global_transforms + component.index;
}


__forceinline struct entity_t *entity_GetEntityPointerIndex(struct entity_handle_t entity)
{
	struct entity_t *entities;
	int cursor;
	
	if(entity.def)
	{
		printf("entity_GetEntityPointerIndex: handle of an entity def\n");
		entities = ent_entity_defs;
		cursor = ent_entity_def_list_cursor;
	}
	else
	{
		entities = ent_entities;
		cursor = ent_entity_list_cursor;
	}
	
	if(entity.entity_index != INVALID_ENTITY_INDEX)
	{
		if(entity.entity_index >= 0 && entity.entity_index < cursor)
		{
			if(!(entities[entity.entity_index].flags & ENTITY_INVALID))
			{
				return &entities[entity.entity_index];
			}
		}
	}
	
	return NULL;
}


#endif
