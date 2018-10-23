#ifndef ENTITY_INL
#define ENTITY_INL

#include <stdio.h>
#include <string.h>

#include "macros.h"
#include "ent_common.h"
#include "stack_list.h"

extern struct stack_list_t ent_components[2][COMPONENT_TYPE_LAST];

extern int ent_entity_list_cursor;
extern struct stack_list_t ent_entities[2];
extern int ent_entity_def_list_cursor;
extern struct entity_t *ent_entity_defs;

extern struct stack_list_t ent_world_transforms;
extern struct stack_list_t ent_entity_aabbs;

static ALWAYS_FORCE_INLINE struct entity_t *entity_GetEntityPointerHandle(struct entity_handle_t entity)
{
	struct entity_t *entity_ptr = NULL;
	int cursor;

	if(entity.entity_index != INVALID_ENTITY_INDEX)
	{
		if(entity.entity_index >= 0 && entity.entity_index < ent_entities[entity.def].element_count)
		{
			entity_ptr = (struct entity_t *)ent_entities[entity.def].elements + entity.entity_index;

			if(entity_ptr->flags & ENTITY_FLAG_INVALID)
			{
				entity_ptr = NULL;
			}
		}
	}

	return entity_ptr;
}

static ALWAYS_FORCE_INLINE void *entity_GetComponentPointer(struct component_handle_t component)
{
	struct stack_list_t *list;
	if(component.type != COMPONENT_TYPE_NONE)
	{
		list = &ent_components[component.def][component.type];
		return (void *)((char *)list->elements + list->element_size * component.index);
	}
	return NULL;
}

static ALWAYS_FORCE_INLINE void *entity_GetComponentPointerIndex(int index, int type, int def)
{
	struct component_handle_t handle;

	def = def && 1;

	handle.def = def;
	handle.type = type;
	handle.index = index;

	return entity_GetComponentPointer(handle);
}

static ALWAYS_FORCE_INLINE struct entity_transform_t *entity_GetWorldTransformPointer(struct component_handle_t component)
{
	return (struct entity_transform_t *)((char *)ent_world_transforms.elements + ent_world_transforms.element_size * component.index);
}

static ALWAYS_FORCE_INLINE struct entity_aabb_t *entity_GetAabbPointer(struct component_handle_t component)
{
	return (struct entity_aabb_t *)((char *)ent_entity_aabbs.elements + ent_entity_aabbs.element_size * component.index);
}

/*__forceinline struct entity_t *entity_GetEntityPointerHandle(struct entity_handle_t entity)
{
	struct entity_t *entity_ptr = NULL;
	int cursor;

	if(entity.entity_index != INVALID_ENTITY_INDEX)
	{
		if(entity.entity_index >= 0 && entity.entity_index < ent_entities[entity.def].element_count)
		{
			entity_ptr = (struct entity_t *)ent_entities[entity.def].elements + entity.entity_index;

			if(entity_ptr->flags & ENTITY_FLAG_INVALID)
			{
				entity_ptr = NULL;
			}
		}
	}

	return entity_ptr;
}*/

static ALWAYS_FORCE_INLINE struct entity_t *entity_GetEntityParentPointerHandle(struct entity_handle_t entity)
{
	struct entity_t *entity_ptr = NULL;
	struct entity_t *parent_entity_ptr = NULL;
	struct transform_component_t *transform = NULL;

	if(entity.entity_index != INVALID_ENTITY_INDEX)
	{
		if(entity.entity_index >= 0 && entity.entity_index < ent_entities[entity.def].element_count)
		{
			entity_ptr = (struct entity_t *)ent_entities[entity.def].elements + entity.entity_index;

			if(!(entity_ptr->flags & ENTITY_FLAG_INVALID))
			{
				transform = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
				transform = entity_GetComponentPointer(transform->parent);

				if(transform)
				{
					parent_entity_ptr = (struct entity_t *)ent_entities[transform->base.entity.def].elements + transform->base.entity.entity_index;
				}
			}
		}
	}

	return parent_entity_ptr;
}

static ALWAYS_FORCE_INLINE struct entity_t *entity_GetEntityPointerIndex(int entity_index)
{
	struct entity_t *entity_ptr;
	int cursor;

	if(entity_index != INVALID_ENTITY_INDEX)
	{
		if(entity_index >= 0 && entity_index < ent_entities[0].element_count)
		{
			entity_ptr = (struct entity_t *)ent_entities[0].elements + entity_index;

			if(!(entity_ptr->flags & ENTITY_FLAG_INVALID))
			{
				return entity_ptr;
			}
		}
	}

	return NULL;
}

static ALWAYS_FORCE_INLINE struct entity_t *entity_GetEntityDefPointerIndex(int entity_def_index)
{
	struct entity_t *entity_ptr;
	int cursor;

	if(entity_def_index != INVALID_ENTITY_INDEX)
	{
		if(entity_def_index >= 0 && entity_def_index < ent_entities[1].element_count)
		{
			entity_ptr = (struct entity_t *)ent_entities[1].elements + entity_def_index;

			if(!(entity_ptr->flags & ENTITY_FLAG_INVALID))
			{
				return entity_ptr;
			}
		}
	}

	return NULL;
}

static ALWAYS_FORCE_INLINE struct entity_handle_t entity_GetEntityHandle(char *name, int get_def)
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

		if(entity->flags & ENTITY_FLAG_INVALID)
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

static struct entity_handle_t entity_GetNestledEntityHandle_Stack[1024];

static ALWAYS_FORCE_INLINE struct entity_handle_t entity_GetNestledEntityHandle(struct entity_handle_t parent_entity, char *entity)
{
	struct entity_t *entity_ptr;
	struct transform_component_t *transform;
	struct transform_component_t *child_transform;

	int i;
	int stack_cursor = 0;
	int cur_top = -1;
	int next_top = -1;

	entity_GetNestledEntityHandle_Stack[0] = parent_entity;
	next_top++;

	entity_ptr = entity_GetEntityPointerHandle(parent_entity);

	if(entity_ptr)
	{
		do
		{
			cur_top = next_top;

			for(;stack_cursor <= cur_top; stack_cursor++)
			{
				entity_ptr = entity_GetEntityPointerHandle(entity_GetNestledEntityHandle_Stack[stack_cursor]);

				if(entity_ptr)
				{
					if(!strcmp(entity_ptr->name, entity))
					{
						return entity_GetNestledEntityHandle_Stack[stack_cursor];
					}

					transform = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);

					for(i = 0; i < transform->children_count; i++)
					{
						child_transform = entity_GetComponentPointer(transform->child_transforms[i]);

						if(child_transform->base.entity.entity_index != INVALID_ENTITY_INDEX)
						{
							next_top++;
							entity_GetNestledEntityHandle_Stack[next_top] = child_transform->base.entity;
						}
					}
				}
			}

		}while(cur_top != next_top);
	}
	else
	{
		printf("entity_GetNestledEntityHandle: bad parent entity handle\n");
	}


	return (struct entity_handle_t){parent_entity.def, INVALID_ENTITY_INDEX};
}
#endif











