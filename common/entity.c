#include "entity.h"
#include "r_main.h"

#include "stack_list.h"
#include "list.h"

#include "memory.h"
#include "log.h"
#include "physics.h"
#include "script.h"
#include "navigation.h"

#include "engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
 

/* from world.c */
extern int w_world_leaves_count;
extern bsp_pnode_t *w_world_nodes;
extern bsp_dleaf_t *w_world_leaves;
extern bsp_entities_t *w_leaf_entities;
extern int w_visible_leaves_count;
extern bsp_dleaf_t **w_visible_leaves;


/* from model.c */
extern struct model_t *mdl_models;


extern int r_frame;

//struct entity_aabb_t *ent_aabbs = NULL;


//struct entity_def_t *ent_entity_defs = NULL;

int ent_visible_entities_count = 0;
int ent_visible_entities_indexes[MAX_VISIBLE_ENTITIES];



/*
===============================================================
===============================================================
===============================================================
*/

static int COMPONENT_SIZES[COMPONENT_TYPE_LAST];
struct component_fields_t COMPONENT_FIELDS[COMPONENT_TYPE_LAST];
//char *COMPONENT_FIELDS[COMPONENT_TYPE_LAST][16];

#define DECLARE_COMPONENT_SIZE(type, size) COMPONENT_SIZES[type]=size

#define COMPONENT_FIELD(name, script_type, offset) (struct component_field_t){name, script_type, offset}
#define COMPONENT_FIELD_OFFSET(type, field)(int)(&((type *)0)->field)
#define DECLARE_COMPONENT_FIELDS(component_type, ...)COMPONENT_FIELDS[component_type]=(struct component_fields_t)__VA_ARGS__

struct stack_list_t ent_components[2][COMPONENT_TYPE_LAST];
struct stack_list_t ent_entities[2];
struct stack_list_t ent_entity_aabbs;
struct stack_list_t ent_world_transforms;
struct list_t ent_top_transforms;


void (*dispose_component_callback[COMPONENT_TYPE_LAST])() = {NULL};

/*
===============================================================
===============================================================
===============================================================
*/






static mat3_t id_rot;

//int ent_entity_transform_cursor = 0;
//mat4_t *ent_entity_transforms = NULL;
 
#ifdef __cplusplus
extern "C"
{
#endif


void entity_EntityListDisposeCallback(void *data)
{
	struct entity_t *entity;
	
	entity = (struct entity_t *)data;
	
	if(entity->name)
	{
		memory_Free(entity->name);
	}
}

void entity_TransformComponentDisposeCallback(void *data)
{
	struct transform_component_t *transform_component;
	
	transform_component = (struct transform_component_t *)data;
	
	if(transform_component->child_transforms)
	{
		memory_Free(transform_component->child_transforms);
	}
}

 
int entity_Init()
{
	int i;
	int j;
	int def_list;
	int component_type;
	
	struct entity_t *entities;
	struct entity_t *entity;

	for(i = 0; i < 2; i++)
	{
		ent_entities[i] = stack_list_create(sizeof(struct entity_t), 64, entity_EntityListDisposeCallback);		
	}	
	
	ent_entity_aabbs = stack_list_create(sizeof(struct entity_aabb_t), 64, NULL);
	ent_world_transforms = stack_list_create(sizeof(struct entity_transform_t), 64, NULL);
	ent_top_transforms = list_create(sizeof(struct component_handle_t), 64, NULL);	
	
	/*
	===============================================================
	===============================================================
	===============================================================
	*/
	
	 
	  
	 
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_TRANSFORM, sizeof(struct transform_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_PHYSICS, sizeof(struct physics_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_MODEL, sizeof(struct model_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_LIGHT, sizeof(struct light_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_SCRIPT, sizeof(struct script_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_CAMERA, sizeof(struct camera_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_PARTICLE_SYSTEM, sizeof(struct particle_system_component_t));
		
	DECLARE_COMPONENT_FIELDS(COMPONENT_TYPE_TRANSFORM, {
															COMPONENT_FIELD("orientation", SCRIPT_VAR_TYPE_MAT3T, COMPONENT_FIELD_OFFSET(struct transform_component_t, orientation)),
															COMPONENT_FIELD("scale", SCRIPT_VAR_TYPE_VEC3T, COMPONENT_FIELD_OFFSET(struct transform_component_t, scale)), 
															COMPONENT_FIELD("position", SCRIPT_VAR_TYPE_VEC3T, COMPONENT_FIELD_OFFSET(struct transform_component_t, position)),
															COMPONENT_FIELD(NULL, SCRIPT_VAR_TYPE_NONE, 0)
														});
	
	
	DECLARE_COMPONENT_FIELDS(COMPONENT_TYPE_CAMERA, {
															COMPONENT_FIELD("position", SCRIPT_VAR_TYPE_COMPONENT, COMPONENT_FIELD_OFFSET(struct camera_component_t, transform)),
													 		COMPONENT_FIELD("orientation", SCRIPT_VAR_TYPE_COMPONENT, COMPONENT_FIELD_OFFSET(struct camera_component_t, transform))
													});
	
	dispose_component_callback[COMPONENT_TYPE_TRANSFORM] = entity_TransformComponentDisposeCallback;
	
	for(def_list = 0; def_list < 2; def_list++)
	{
		for(component_type = COMPONENT_TYPE_TRANSFORM; component_type < COMPONENT_TYPE_LAST; component_type++)
		{
			ent_components[def_list][component_type] = stack_list_create(COMPONENT_SIZES[component_type], 64, dispose_component_callback[component_type]);
		}
	}	

	id_rot = mat3_t_id();	
	
	
		
	return 1;
}

void entity_Finish()
{
	int i;
	int j;
		
	stack_list_destroy(&ent_entities[0]);
	stack_list_destroy(&ent_entities[1]);
	
	stack_list_destroy(&ent_entity_aabbs);
	stack_list_destroy(&ent_world_transforms);
	list_destroy(&ent_top_transforms);
	
	
	for(j = 0; j < 2; j++)
	{
		for(i = COMPONENT_TYPE_TRANSFORM; i < COMPONENT_TYPE_LAST; i++)
		{
			stack_list_destroy(&ent_components[j][i]);
		}
	}
}

/*
==============================================================
==============================================================
==============================================================
*/

inline struct stack_list_t *entity_ListForType(int type, int def_list)
{
	if(type != COMPONENT_TYPE_NONE)
	{
		def_list = def_list && 1;
		return &ent_components[def_list][type];
	}	
	return NULL;	
}


struct component_handle_t entity_AllocComponent(int component_type, int alloc_for_def)
{
	struct stack_list_t *list;
	struct component_t *component;
	struct transform_component_t *transform;
	struct component_handle_t *transforms;
	struct script_component_t *script;
	struct component_handle_t handle;
	
	handle.def = 0;
	handle.type = COMPONENT_TYPE_NONE;
	handle.index = 0;

	list = entity_ListForType(component_type, alloc_for_def);
	
	alloc_for_def = alloc_for_def && 1;
	
	if(list)
	{
		handle.index = stack_list_add(list, NULL);
		handle.type = component_type;
		handle.def = alloc_for_def;
		
		component = entity_GetComponentPointer(handle);
		
		/* this component could've been allocated to keep data
		from another component, so we clear it here to make sure
		it won't point by accident to an entity... */
		component->type = component_type;
		component->entity.entity_index = INVALID_ENTITY_INDEX;
		component->flags = 0;
		
		//if(!alloc_for_def)
		//{
		switch(component_type)
		{
			case COMPONENT_TYPE_TRANSFORM:
				transform = (struct transform_component_t *)component;
				transform->parent.type = COMPONENT_TYPE_NONE;
				transform->children_count = 0;
				
				/* if this transorm isn't getting alloc'd for
				a entity def, add it to the top list so it
				gets treated as the top of a hierarchy... */
				if(!alloc_for_def)
				{
					entity_AddTransformToTopList(handle);
					stack_list_add(&ent_world_transforms, NULL);
					stack_list_add(&ent_entity_aabbs, NULL);
				}
			break;
			
			case COMPONENT_TYPE_SCRIPT:
				script = (struct script_component_t *)component;
				script->script = NULL;
			break;
		}        
		//}
	}
	else
	{
		printf("entity_AllocComponent: bad component type\n");
	}

	return handle;
}

 
void entity_DeallocComponent(struct component_handle_t component)
{
	struct stack_list_t *list;
	struct component_t *component_ptr;
	struct camera_component_t *camera_component;
	
	list = entity_ListForType(component.type, component.def);
	
	if(list)
	{
		
		component_ptr = entity_GetComponentPointer(component);
		
		if(!component.def)
		{
			switch(component.type)
			{
				case COMPONENT_TYPE_TRANSFORM:
					/* remove this transform from the top list, so
					the engine can stop updating it... */
					entity_RemoveTransformFromTopList(component);
					stack_list_remove(&ent_world_transforms, component.index);
					stack_list_remove(&ent_entity_aabbs, component.index);
				break;
				
				/*case COMPONENT_TYPE_CAMERA:
					camera_component = (struct camera_component_t *)component_ptr;
					
					camera_DestroyCamera(camera_component->camera);
				break;*/
			}
		}
		
		
		component_ptr->flags |= COMPONENT_FLAG_INVALID;
		component_ptr->entity.entity_index = INVALID_ENTITY_INDEX;
		
		stack_list_remove(list, component.index);
		
	}
}



void entity_AddTransformToTopList(struct component_handle_t transform)
{
	struct component_handle_t *transforms;
	struct transform_component_t *transform_ptr;
	
	if(transform.def)
	{
		printf("entity_AddTransformToTopList: can't add def transform component to top list\n");
		return;
	}
	
	transform_ptr = entity_GetComponentPointer(transform);	

	transform_ptr->top_list_index = list_add(&ent_top_transforms, &transform);
	transform_ptr->depth_index = 0;
}

void entity_RemoveTransformFromTopList(struct component_handle_t transform)
{
	struct transform_component_t *transform_ptr;
	struct transform_component_t *other;
	struct component_handle_t *handle;
	if(transform.def)
	{
		printf("entity_RemoveTransformFromTopList: can't remove def transform component from top list\n");
		return;
	}
	
	transform_ptr = entity_GetComponentPointer(transform);
	
	if(transform_ptr->top_list_index < 0)
	{
		printf("entity_RemoveTransformFromTopList: transform component already removed from list\n");
		return;
	}
	
	list_remove(&ent_top_transforms, transform_ptr->top_list_index);
	handle = list_get(&ent_top_transforms, transform_ptr->top_list_index);
	
	if(handle)
	{
		other = entity_GetComponentPointer(*handle);
		other->top_list_index = transform_ptr->top_list_index;
	}
	
	transform_ptr->top_list_index = -1;	
	//transform_ptr->depth_index = 0;
}


void entity_ParentTransformComponent(struct component_handle_t parent_transform, struct component_handle_t child_transform)
{
	struct transform_component_t *parent;
	struct transform_component_t *child;
	struct transform_component_t *other;
	struct component_handle_t *transforms;
	struct component_handle_t h;
	
	if(parent_transform.def != child_transform.def)
	{
		printf("entity_ParentTransformComponent: cannot parent def transform with non def transform\n");
		return;
	}
	
	if(parent_transform.index == child_transform.index)
	{
		printf("entity_ParentTransformCompnent: cannot parent transform with itself\n");
		return;
	}
	
	parent = entity_GetComponentPointer(parent_transform);
	child = entity_GetComponentPointer(child_transform);
	
	h = parent->parent;
	
	if(h.type != COMPONENT_TYPE_NONE)
	{
		while(h.type != COMPONENT_TYPE_NONE && h.index != child_transform.index)
		{
			other = entity_GetComponentPointer(h);
			h = other->parent;
		}
		
		if(h.index == child_transform.index)
		{
			/* if the parent's transform parent handle points
			to the child transform, don't parent them as this
			would create a cycle in the hierarchy... */
			printf("entity_ParentTransformComponent: parenting would cause circular references\n");
			return;
		}
	}
	
	
	
	if(child->parent.type != COMPONENT_TYPE_NONE)
	{
		entity_UnparentTransformComponent(child->parent, child_transform);
	}
	
	if(parent->children_count >= parent->max_children)
	{
		transforms = memory_Malloc(sizeof(struct component_handle_t) * (parent->max_children + 4), "entity_ParentTransformComponent");
		if(parent->child_transforms)
		{
			memcpy(transforms, parent->child_transforms, sizeof(struct component_handle_t) * parent->max_children);
			memory_Free(parent->child_transforms);
		}
	
		parent->child_transforms = transforms;
		parent->max_children += 4;
	}
	
	parent->child_transforms[parent->children_count] = child_transform;
	parent->children_count++;
	
	child->parent = parent_transform;
	
	if(!parent_transform.def)
	{
		/* the child transform isn't the top of a hierarchy
		anymore, so remove it's handle from the list so it
		can be updated only through it's parent transform... */
		entity_RemoveTransformFromTopList(child_transform);
	}
}

void entity_UnparentTransformComponent(struct component_handle_t parent_transform, struct component_handle_t child_transform)
{
	int i;
	struct transform_component_t *transform;
	
	if(parent_transform.def != child_transform.def)
	{
		printf("entity_UnparentTransformComponent: cannot unparent def transform from non def transform\n");
		return;
	}
	
	transform = entity_GetComponentPointer(parent_transform);
	
	for(i = 0; i < transform->children_count; i++)
	{
		if(*(int *)&transform->child_transforms[i] == *(int *)&child_transform)
		{
			if(i < transform->children_count - 1)
			{
				transform->child_transforms[i] = transform->child_transforms[transform->children_count - 1];
			}
			
			transform->children_count--;
			
			/* this newly unparent transform gets to be 
			the top of a hierarchy now... */
			entity_AddTransformToTopList(child_transform);
			
			transform = entity_GetComponentPointer(child_transform);
			transform->parent.type = COMPONENT_TYPE_NONE;
					
			return;
		}
	}
	
}

void entity_ParentEntity(struct entity_handle_t parent, struct entity_handle_t child)
{
	struct entity_t *parent_entity;
	struct entity_t *child_entity;
	
	if(parent.def != child.def)
	{
		printf("entity_ParentEntity: can't parent entity def to non def\n");
		return;
	}
	
	parent_entity = entity_GetEntityPointerHandle(parent);
	child_entity = entity_GetEntityPointerHandle(child);
	
	entity_ParentTransformComponent(parent_entity->components[COMPONENT_TYPE_TRANSFORM], child_entity->components[COMPONENT_TYPE_TRANSFORM]);
}


void entity_UnparentEntity(struct entity_handle_t parent, struct entity_handle_t child)
{
	struct entity_t *parent_entity;
	struct entity_t *child_entity;
	
	if(parent.def != child.def)
	{
		printf("entity_UnparentEntity: can't unparent entity def to non def\n");
		return;
	}
	
	parent_entity = entity_GetEntityPointerHandle(parent);
	child_entity = entity_GetEntityPointerHandle(child);
	
	entity_UnparentTransformComponent(parent_entity->components[COMPONENT_TYPE_TRANSFORM], child_entity->components[COMPONENT_TYPE_TRANSFORM]);
}

/*
==============================================================
==============================================================
==============================================================
*/


struct entity_handle_t entity_CreateEntityDef(char *name)
{
	int entity_def_index;
	struct entity_handle_t handle;
	struct entity_t *entity_def;
	struct transform_component_t *transform_component;
	int i;
	
	handle = entity_CreateEntity(name, 1);	
	entity_AddComponent(handle, COMPONENT_TYPE_TRANSFORM);
	entity_SetTransform(handle, NULL, vec3_t_c(0.0, 0.0, 0.0), vec3_t_c(1.0, 1.0, 1.0), 0);
	
	return handle;
}

void entity_DestroyEntityDef(struct entity_handle_t entity)
{
	//if(entity_def_index)
}

struct component_handle_t entity_AddComponent(struct entity_handle_t entity, int component_type)
{
	struct entity_t *entity_ptr;
	int component_index;
	
	struct component_handle_t component_transform;
	struct component_handle_t component;
	
	struct model_component_t *model_component;
	struct physics_controller_component_t *physics_controller;
	struct script_controller_component_t *script_controller;
	struct camera_component_t *camera_component;
	struct transform_component_t *transform_component;
	struct entity_aabb_t *aabb;
	struct component_t *component_ptr;
	
	component.type = COMPONENT_TYPE_NONE;
		
	entity_ptr = entity_GetEntityPointerHandle(entity);
	
	if(!entity_ptr)
	{
		if(entity.def)
		{
			printf("entity_AddComponent: bad entity def handle!\n");
		}
		else
		{
			printf("entity_AddComponent: bad entity handle!\n");
		}
		
		return component;
	}
	
	
	if(component_type == COMPONENT_TYPE_TRANSFORM)
	{
		if(entity_ptr->components[COMPONENT_TYPE_TRANSFORM].type != COMPONENT_TYPE_NONE)
		{
			printf("entity_AddComponent: cannot overwrite entity's transform component\n");
			return component;
		}
	}
	
	
	component = entity_AllocComponent(component_type, entity.def);
	component_ptr = entity_GetComponentPointer(component);
	component_ptr->entity = entity;
	
	assert(component_ptr);
	
	if(component.type != COMPONENT_TYPE_NONE)
	{
		entity_ptr->components[component.type] = component;
		component_ptr->entity = entity;
		
		switch(component_ptr->type)
		{		
		
			case COMPONENT_TYPE_TRANSFORM:
				//transform_component = (struct transform_component_t *)component_ptr;
				//transform_component->parent.index = INVALID_ENTITY_INDEX;
			break;
			
			case COMPONENT_TYPE_CAMERA:
				/* a camera component needs a transform component so it can be
				updated by other transforms above it in the hierarchy... */
				component_transform = entity_AllocComponent(COMPONENT_TYPE_TRANSFORM, entity.def);
				camera_component = (struct camera_component_t *)component_ptr;
				
				camera_component->transform = component_transform;
				
				/* parent this camera's transform to the entity's transform that owns the camera component... */
				entity_ParentTransformComponent(entity_ptr->components[COMPONENT_TYPE_TRANSFORM], component_transform);
			break;
		}
	}
	
	return component;
	
	//controller->flags = 0;
}

void entity_RemoveComponent(struct entity_handle_t entity, int component_type, int component_index)
{
	
}

void entity_AddProp(struct entity_handle_t entity, char *name, int size)
{
	struct entity_prop_t *prop;
	struct entity_t *entity_ptr;
	
	int prop_index;
	
	entity_ptr = entity_GetEntityPointerHandle(entity);
	
	if(size <= 0)
	{
		return;
	}
	
	if(entity_ptr)
	{
		
		for(prop_index = 0; prop_index < entity_ptr->prop_count; prop_index++)
		{
			if(!strcmp(name, entity_ptr->props[prop_index].name))
			{
				if(size == entity_ptr->props[prop_index].size)
				{
					/* don't add a prop if the requested name and size
					is the same of an existing one... */
					return;
				}
				
				break;
			}
		}
		
		if(prop_index == entity_ptr->prop_count)
		{
			if(entity_ptr->prop_count >= entity_ptr->max_props)
			{
				prop = memory_Malloc(sizeof(struct entity_prop_t) * (entity_ptr->max_props + 4), "entity_AddProp");
				
				if(entity_ptr->props)
				{
					memcpy(prop, entity_ptr->props, sizeof(struct entity_prop_t) * entity_ptr->max_props);
					memory_Free(entity_ptr->props);
				}
				
				entity_ptr->props = prop;
				entity_ptr->max_props += 4;
			}
			
			
			prop = entity_ptr->props + entity_ptr->prop_count;
			entity_ptr->prop_count++;
			
			prop->name = memory_Strdup(name, "entity_AddProp");
			
		}
		else
		{
			prop = entity_ptr->props + prop_index;
			memory_Free(prop->memory);
		}
		
		size = (size + 3) & (~3);
		
		prop->size = size;
		prop->memory = memory_Malloc(prop->size, "entity_AddProp");
	}
}

void entity_RemoveProp(struct entity_handle_t entity, char *name)
{
	
}

void entity_PropValue(struct entity_handle_t entity, char *name, void *value, int set)
{
	struct entity_prop_t *prop;
	struct entity_t *entity_ptr;
	int i;
	
	entity_ptr = entity_GetEntityPointerHandle(entity);
	
	if(entity_ptr)
	{
		for(i = 0; i < entity_ptr->prop_count; i++)
		{
			if(!strcmp(name, entity_ptr->props[i].name))
			{
				if(set)
				{
					memcpy(entity_ptr->props[i].memory, value, entity_ptr->props[i].size);
				}
				else
				{
					memcpy(value, entity_ptr->props[i].memory, entity_ptr->props[i].size);
				}
				
				return;
			}
		}
	}
}

void entity_SetProp(struct entity_handle_t entity, char *name, void *value)
{
	entity_PropValue(entity, name, value, 1);
}

void entity_GetProp(struct entity_handle_t entity, char *name, void *value)
{
	entity_PropValue(entity, name, value, 0);
}

/*
==============================================================
==============================================================
==============================================================
*/

void entity_SetModel(struct entity_handle_t entity, int model_index)
{
	struct entity_t *entity_ptr;
	struct model_component_t *model_component;
	struct entity_aabb_t *aabb;
	struct model_t *model;
	
	int component;
	
	entity_ptr = entity_GetEntityPointerHandle(entity);
	
	if(!entity_ptr)
	{
		if(entity.def)
		{
			printf("entity_SetEntityModel: bad entity def handle!\n");
		}
		else
		{
			printf("entity_SetEntityModel: bad entity handle!\n");
		}
		
		return;
	}
	
	
	model_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_MODEL]);
	
	if(!model_component)
	{
		if(entity.def)
		{
			printf("entity_SetEntityModel: entity def [%s] has no model component\n", entity_ptr->name);
		}
		else
		{
			printf("entity_SetEntityModel: entity [%s] has no model component\n", entity_ptr->name);
		}
		
		return;
	}
	
	model_component->model_index = model_index;
	model = model_GetModelPointerIndex(model_index);
	
	if(!entity.def)
	{
		/* if this entity is not a def it means its transform component has a
		global transform and a aabb linked to it, so use the model's aabb
		to set the transform's aabb original extents... */
		aabb = entity_GetAabbPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
		aabb->original_extents = model->aabb_max;
		aabb->current_extents = aabb->original_extents;
	}
	
}


void entity_SetCollider(struct entity_handle_t entity, void *collider)
{
	//struct controller_component_t *controller;
	//struct physics_controller_component_t *physics_controller;
	struct physics_component_t *physics_component;
	struct entity_t *entity_ptr;
	struct collider_handle_t handle;

	entity_ptr = entity_GetEntityPointerHandle(entity);
	
	if(!entity_ptr)
	{
		if(entity.def)
		{
			printf("entity_SetCollider: bad entity def handle\n");
		}
		else
		{
			printf("entity_SetCollider: bad entity handle\n");
		}
		return;
	}
	
	
	physics_component = (struct physics_component_t *)entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);
		
	if(!physics_component)
	{
		if(entity.def)
		{
			printf("entity_SetCollider: entity def [%s] doesn't have a valid controller component\n", entity_ptr->name);
		}
		else
		{
			printf("entity_SetCollider: entity [%s] doesn't have a valid controller component\n", entity_ptr->name);
		}
			
		return;
	}
		
	if(entity.def)
	{
		physics_component->collider.collider_def = collider;
	}
	else
	{
		physics_component->collider.collider_handle = *(struct collider_handle_t *)collider;
	}
		
	physics_component->flags = 0;
}

void entity_SetScript(struct entity_handle_t entity, struct script_t *script)
{
	struct entity_t *entity_ptr;
	struct script_component_t *script_component;
		
	entity_ptr = entity_GetEntityPointerHandle(entity);
	
	if(!entity_ptr)
	{
		if(entity.def)
		{
			printf("entity_SetScript: bad entity def handle\n");
		}
		else
		{
			printf("entity_SetScript: bad entity handle\n");
		}
		return;
	}
	
	script_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_SCRIPT]);
	
	if(!script_component)
	{
		if(entity.def)
		{
			printf("entity_SetScript: entity def [%s] doesn't have a valid script component\n", entity_ptr->name);
		}
		else
		{
			printf("entity_SetScript: entity [%s] doesn't have a valid script component\n", entity_ptr->name);
		}
		return;
	}
	
	//printf("%x\n", script);
	
	script_component->script = script;
	script_component->flags = SCRIPT_CONTROLLER_FLAG_FIRST_RUN;
}

void entity_SetTransform(struct entity_handle_t entity, mat3_t *orientation, vec3_t position, vec3_t scale, int clear_aabb)
{
	struct transform_component_t *transform_component;
	struct entity_t *entity_ptr;
	struct entity_aabb_t *aabb;
	
	entity_ptr = entity_GetEntityPointerHandle(entity);
	
	if(!entity_ptr)
	{
		if(entity.def)
		{
			printf("entity_SetTransform: bad entity def handle\n");
		}
		else
		{
			printf("entity_SetTransform: bad entity handle\n");
		}
		return;
	}

	transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
	
	if(!transform_component)
	{
		if(entity.def)
		{
			printf("entity_SetTransform: entity def [%s] doesn't have a valid transform component\n", entity_ptr->name);
		}
		else
		{
			printf("entity_SetTransform: entity [%s] doesn't have a valid transform component\n", entity_ptr->name);
		}
		return;
	}
	
	if(transform_component)
	{
		if(orientation)
		{
			transform_component->orientation = *orientation;
		}
		else
		{
			transform_component->orientation = mat3_t_id();
		}
			
		transform_component->position = position;
		transform_component->scale = scale;
			
		
		if(clear_aabb)
		{
			aabb = entity_GetAabbPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
				
			aabb->original_extents.x = 0.0;
			aabb->original_extents.y = 0.0;
			aabb->original_extents.z = 0.0;
				
			aabb->current_extents.x = 0.0;
			aabb->current_extents.y = 0.0;
			aabb->current_extents.z = 0.0;
		}
			
	}	
}


void entity_SetCameraTransform(struct entity_handle_t entity, mat3_t *orientation, vec3_t position)
{
	struct entity_t *entity_ptr;
	struct transform_component_t *transform_component;
	struct camera_component_t *camera_component;
	
	entity_ptr = entity_GetEntityPointerHandle(entity);
	
	camera_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_CAMERA]);
	transform_component = entity_GetComponentPointer(camera_component->transform);
	
	if(!orientation)
	{
		transform_component->orientation = mat3_t_id();
	}
	else
	{
		transform_component->orientation = *orientation;
	}
	
	transform_component->position = position;
	transform_component->scale = vec3_t_c(1.0, 1.0, 1.0);	
}

void entity_SetCamera(struct entity_handle_t entity, camera_t *camera)
{
	struct entity_t *entity_ptr;
	struct camera_component_t *camera_component;
	
	entity_ptr = entity_GetEntityPointerHandle(entity);
	camera_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_CAMERA]);
	camera_component->camera = camera;
}

/*
==============================================================
==============================================================
==============================================================
*/
 
struct entity_handle_t entity_CreateEntity(char *name, int def)
{
	struct entity_t *entity_ptr;
	struct entity_handle_t handle;
	int entity_index;
	int i;

	def = def && 1;
	
	entity_index = stack_list_add(&ent_entities[def], NULL);
	
	handle.def = def;
	handle.entity_index = entity_index;

	entity_ptr = stack_list_get(&ent_entities[def], entity_index);
	entity_ptr->flags = 0;
	entity_ptr->leaf = NULL;
	entity_ptr->prop_count = 0;
	
	entity_ptr->def.def = 1;
	entity_ptr->def.entity_index = INVALID_ENTITY_INDEX;
		
	for(i = 0; i < COMPONENT_TYPE_LAST; i++)
	{
		entity_ptr->components[i].type = COMPONENT_TYPE_NONE;
	}
	
	//entity_AddComponent(handle, COMPONENT_TYPE_TRANSFORM);
	
	if(name)
	{
		if(!entity_ptr->name)
		{
			entity_ptr->name = memory_Malloc(ENTITY_NAME_MAX_LEN, "entity_CreateEntity");
		}
		
		for(i = 0; name[i] && i < ENTITY_NAME_MAX_LEN - 1; i++)
		{
			entity_ptr->name[i] = name[i];
		}
		
		/* TODO: make sure no two entities in the world
		have the same name... */
		entity_ptr->name[i] = '\0';
	}
	
	return handle;
}


struct entity_handle_t entity_SpawnEntity(mat3_t *orientation, vec3_t position, vec3_t scale, struct entity_handle_t entity_def, char *name)
{
	struct entity_t *entity_ptr;
	struct entity_t *entity_def_ptr;
	struct entity_handle_t handle;
	struct entity_handle_t child_handle;
	
	struct model_component_t *model_component;
	struct model_component_t *def_model_component;
	struct transform_component_t *transform_component;
	struct transform_component_t *child_transform;
	
	struct camera_component_t *camera_component;
	struct transform_component_t *camera_transform_component;
	struct camera_component_t *def_camera_component;
	struct transform_component_t *def_camera_transform_component;
	
	//struct script_controller_component_t *script_controller;
	//struct script_controller_component_t *def_script_controller;
	
	struct physics_component_t *physics_component;
	struct physics_component_t *def_physics_component;
	
	struct script_component_t *script_component;
	struct script_component_t *def_script_component;
	
	//struct physics_controller_component_t *physics_controller;
	//struct physics_controller_component_t *def_physics_controller;
	
	//struct controller_component_t *controller;
	//struct controller_component_t *def_controller;
	struct component_t *component_ptr;
	struct component_handle_t component_transform;
	
	struct entity_aabb_t *aabb;
	struct model_t *model;
	int entity_index;
	int collider_index;
	struct collider_handle_t collider_handle;
	
	struct component_handle_t component;
	
	int i;
	int component_type;
	int j;
	
	handle.entity_index = INVALID_ENTITY_INDEX;
	handle.def = 0;
	
	if(!entity_def.def)
	{
		printf("entity_SpawnEntity: cannot spawn an entity from another\n");
		return handle;
	}
	
	if(entity_def.entity_index == INVALID_ENTITY_INDEX)
	{
		printf("entity_SpawnEntity: invalid entity def handle\n");
		return handle;
	}
	
	handle = entity_CreateEntity(name, 0);
	 
	entity_ptr = entity_GetEntityPointerHandle(handle);
	entity_def_ptr = entity_GetEntityPointerHandle(entity_def);
	
	if(entity_ptr->components)
	{
		for(i = 0; i < COMPONENT_TYPE_LAST; i++)
		{
			entity_ptr->components[i].type = COMPONENT_TYPE_NONE;
		}
	}
	
	
			
	for(component_type = 0; component_type < COMPONENT_TYPE_LAST; component_type++)
	{
		if(entity_def_ptr->components[component_type].type != COMPONENT_TYPE_NONE)
		{
			entity_AddComponent(handle, component_type);
			
			switch(component_type)
			{	
				case COMPONENT_TYPE_TRANSFORM:
					entity_SetTransform(handle, orientation, position, scale, 1);
				break;
				
				case COMPONENT_TYPE_MODEL:
					def_model_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_TYPE_MODEL]);
					entity_SetModel(handle, def_model_component->model_index);
				break;	
				
				case COMPONENT_TYPE_PHYSICS:
					def_physics_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_TYPE_PHYSICS]);
					collider_handle = physics_CreateCollider(orientation, position, scale, def_physics_component->collider.collider_def, 0);
					entity_SetCollider(handle, &collider_handle);
				break;
				
				case COMPONENT_TYPE_SCRIPT:
					def_script_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_TYPE_SCRIPT]);
					entity_SetScript(handle, def_script_component->script);
				break;
				
				case COMPONENT_TYPE_CAMERA:
					camera_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_CAMERA]);
					def_camera_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_TYPE_CAMERA]);
					
					camera_transform_component = entity_GetComponentPointer(camera_component->transform);
					def_camera_transform_component = entity_GetComponentPointer(def_camera_component->transform);
					
					camera_transform_component->orientation = def_camera_transform_component->orientation;
					camera_transform_component->position = def_camera_transform_component->position;
					camera_transform_component->scale = def_camera_transform_component->scale;
					
					camera_component->camera = camera_CreateCamera("camera", vec3_t_c(0.0, 0.0, 0.0), NULL, 0.68, 1366.0, 768.0, 0.1, 500.0, 0);
				break;
			}
		}
	}
	
	
	/* entities spanwed through a def will inherit all
	the props and its values... */
	for(i = 0; i < entity_def_ptr->prop_count; i++)
	{
		entity_AddProp(handle, entity_def_ptr->props[i].name, entity_def_ptr->props[i].size);
		entity_SetProp(handle, entity_def_ptr->props[i].name, entity_def_ptr->props[i].memory);
	}
	
	entity_ptr->spawn_time = r_frame;
	entity_ptr->flags = 0;
	entity_ptr->def = entity_def;
	
	transform_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_TYPE_TRANSFORM]);
	
	for(i = 0; i < transform_component->children_count; i++)
	{
		child_transform = entity_GetComponentPointer(transform_component->child_transforms[i]);
		
		if(child_transform->base.entity.entity_index != INVALID_ENTITY_INDEX)
		{
			entity_def_ptr = entity_GetEntityPointerHandle(child_transform->base.entity);
			child_handle = entity_SpawnEntity(&child_transform->orientation, child_transform->position, child_transform->scale, child_transform->base.entity, entity_def_ptr->name);			
			entity_ParentEntity(handle, child_handle);
		}
	
	}
		
	return handle;
}

void entity_RemoveEntity(struct entity_handle_t entity)
{
	struct entity_t *entity_ptr;
	struct component_handle_t component;
	
	struct camera_component_t *camera_component;
	struct physics_component_t *physics_component;
	struct script_component_t *script_component;
	struct transform_component_t *transform_component;
	struct component_t *component_ptr;
	
	int component_type;
	int i;
	
	if(entity.def)
	{
		printf("entity_RemoveEntity: can't remove entity def\n");
		return;
	}
	
	entity_ptr = entity_GetEntityPointerHandle(entity);
	
	if(entity_ptr)
	{
		
		/*entity_ptr->flags |= ENTITY_MARKED_INVALID;
		
		if(entity_ptr->components[COMPONENT_TYPE_SCRIPT].type != COMPONENT_TYPE_NONE)
		{
			script_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_SCRIPT]);
			
			script_ExecuteScriptImediate(script_component->script, script_component);
		}*/
		
		transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
		
		for(i = 0; i < transform_component->children_count; i++)
		{
			component_ptr = entity_GetComponentPointer(transform_component->child_transforms[i]);
			
			if(component_ptr->entity.entity_index != INVALID_ENTITY_INDEX)
			{
				entity_RemoveEntity(component_ptr->entity);
			}
		}
		
		for(component_type = 0; component_type < COMPONENT_TYPE_LAST; component_type++)
		{
			component = entity_ptr->components[component_type];
			if(component.type != COMPONENT_TYPE_NONE)
			{
				switch(component_type)
				{
					case COMPONENT_TYPE_CAMERA:
						camera_component = entity_GetComponentPointer(component);
						camera_DestroyCamera(camera_component->camera);
						entity_DeallocComponent(camera_component->transform);
					break;
					
					case COMPONENT_TYPE_PHYSICS:
						physics_component = entity_GetComponentPointer(component);
						physics_DestroyColliderHandle(physics_component->collider.collider_handle);
					break;
				}
				entity_DeallocComponent(component);
			}			
		}
	
		entity_ptr->flags |= ENTITY_INVALID;
		
		stack_list_remove(&ent_entities[0], entity.entity_index);
	}
	
}

void entity_RemoveAllEntities()
{
	int i;
	int c;
	struct entity_handle_t handle;
	
	c = ent_entities[0].element_count;
	
	for(i = 0; i < c; i++)
	{
		handle.def = 0;
		handle.entity_index = i;
		
		if(entity_GetEntityPointerHandle(handle))
		{
			entity_RemoveEntity(handle);
		}
	}
}

struct entity_t *entity_GetEntityPointer(char *name, int get_def)
{
	/*int i;
	int c;
	struct entity_t *entities;
	
	if(get_def)
	{
		c = ent_entity_def_list_cursor;
		entities = ent_entity_defs;
	}
	else
	{
		c = ent_entity_list_cursor;
		entities = ent_entities;
	}
	
	for(i = 0; i < c; i++)
	{
		if(entities[i].flags & ENTITY_INVALID)
		{
			continue;
		}
			
		if(!strcmp(entities[i].name, name))
		{
			return entities + i;
		}
	}
	
	return NULL;*/
}


void entity_TranslateEntity(int entity_index, vec3_t direction, float amount)
{
	/*struct entity_t *entity;
	struct controller_component_t *controller;
	struct transform_component_t *transform;
	collider_t *collider;
	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
	{
		entity = &ent_entities[entity_index];
		
		transform = entity_GetComponentPointer(entity->components[COMPONENT_INDEX_TRANSFORM]);
		
		transform->position.x += direction.x * amount;
		transform->position.y += direction.y * amount;
		transform->position.z += direction.z * amount;
		
		transform->flags |= ENTITY_HAS_MOVED;
		
		if(entity->components[COMPONENT_INDEX_CONTROLLER].type != COMPONENT_TYPE_NONE)
		{
			controller = entity_GetComponentPointer(entity->components[COMPONENT_INDEX_CONTROLLER]);
			
			if(!(controller->flags & COMPONENT_FLAG_DEACTIVATED))
			{
				if(controller->collider.collider_index >= 0)
				{
					physics_SetColliderPosition(controller->collider.collider_index, transform->position);
				}
			}	
		}
	
	}*/
	
}

void entity_RotateEntity(int entity_index, vec3_t axis, float amount)
{
	/*struct entity_t *entity;
	
	
	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
	{
		entity = &ent_entities[entity_index];
		
		if(!(entity->flags & ENTITY_INVALID))
		{			
			mat3_t_rotate(&entity->orientation, axis, amount, 0);
			entity->flags |= ENTITY_HAS_MOVED;
			
			if(entity->collider_index > -1)
			{
				physics_SetColliderOrientation(entity->collider_index, &entity->orientation);
			}
			
		}
	}*/
}

#define ENTITY_MIN_SCALE 0.01

void entity_ScaleEntity(int entity_index, vec3_t axis, float amount)
{
	/*struct entity_t *entity;
	
	
	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
	{
		entity = &ent_entities[entity_index];
		
		if(!(entity->flags & ENTITY_INVALID))
		{			
			
			entity->scale.x += axis.x * amount;
			entity->scale.y += axis.y * amount;
			entity->scale.z += axis.z * amount;
				
			if(entity->scale.x < ENTITY_MIN_SCALE) entity->scale.x = ENTITY_MIN_SCALE;
			if(entity->scale.y < ENTITY_MIN_SCALE) entity->scale.y = ENTITY_MIN_SCALE;
			if(entity->scale.z < ENTITY_MIN_SCALE) entity->scale.z = ENTITY_MIN_SCALE;
			
			if(entity->collider_index > -1)
			{
				physics_SetColliderScale(entity->collider_index, entity->scale);
			}
						
			entity->flags |= ENTITY_HAS_MOVED;
		}
	}*/
}


void entity_FindPath(struct entity_handle_t entity, vec3_t to)
{
	#if 0
	//struct transform_component_t *transform;
	struct entity_transform_t *transform;
	struct script_controller_component_t *controller;
	struct entity_t *entity_ptr;
	struct waypoint_t **route;
	struct waypoint_t *waypoint;
	int route_length;
	int i;
	
	entity_ptr = entity_GetEntityPointerHandle(entity);
	
	transform = entity_GetWorldTransformPointer(entity_ptr->components[COMPONENT_INDEX_TRANSFORM]);
	controller = entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_SCRIPT_CONTROLLER]);
	
	if(controller->controller.base.type == COMPONENT_TYPE_SCRIPT_CONTROLLER)
	{
		route = navigation_FindPath(&route_length, vec3(transform->transform.floats[3][0], transform->transform.floats[3][1], transform->transform.floats[3][2]), to);
		
		controller->route_length = 0;
			
		if(route)
		{
			controller->route_length = route_length;
			
			if(route_length > controller->max_route_length)
			{
				if(controller->route)
				{
					memory_Free(controller->route);
				}
				route_length = (route_length + 3) & (~3);
				controller->route = memory_Malloc(sizeof(struct waypoint_t *) * route_length, "entity_FindPath");
				controller->max_route_length = route_length;
			}
			
			for(i = 0; i < controller->route_length; i++)
			{
				controller->route[i] = route[i];
			}
			
			controller->current_waypoint = 0;
		}
	}
	
	#endif
}


/*
=========================
entity_UpdateEntityAabbIndex

self explanatory...

=========================
*/
void entity_UpdateEntityAabbIndex(struct entity_handle_t entity)
{
	struct entity_t *entity_ptr;
	struct transform_component_t *transform_component; 
	struct entity_transform_t *world_transform;
	struct entity_aabb_t *aabb;
	vec4_t transformed_aabb[8];
	int i;
	
	vec3_t max;
	vec3_t min;
	vec3_t center;
	
	if(entity.def)
	{
		printf("entity_UpdateEntityAabbIndex: can't update aabb from entity def\n");
		return;
	}
	
	
	//if(entity.entity_index >= 0 && entity.entity_index < ent_entity_list_cursor)
	//{
		//entity_ptr = &ent_entities[entity.entity_index];
		
	entity_ptr = entity_GetEntityPointerHandle(entity);
			
	if(entity_ptr)
	{
			
			//transform_component = (struct transform_component_t *)ent_transform_components.components + entity->components[COMPONENT_INDEX_TRANSFORM];
		transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
		world_transform = entity_GetWorldTransformPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
		aabb = entity_GetAabbPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
		
			/*aabb = &ent_aabbs[entity_index];*/
			
		transformed_aabb[0].x = aabb->original_extents.x;
		transformed_aabb[0].y = aabb->original_extents.y;
		transformed_aabb[0].z = aabb->original_extents.z;
		transformed_aabb[0].w = 0.0;
			
		transformed_aabb[1].x = -aabb->original_extents.x;
		transformed_aabb[1].y = aabb->original_extents.y;
		transformed_aabb[1].z = aabb->original_extents.z;
		transformed_aabb[1].w = 0.0;
			
		transformed_aabb[2].x = -aabb->original_extents.x;
		transformed_aabb[2].y = aabb->original_extents.y;
		transformed_aabb[2].z = -aabb->original_extents.z;
		transformed_aabb[2].w = 0.0;
			
		transformed_aabb[3].x = aabb->original_extents.x;
		transformed_aabb[3].y = aabb->original_extents.y;
		transformed_aabb[3].z = -aabb->original_extents.z;
		transformed_aabb[3].w = 0.0;
					
		max.x = FLT_MIN;
		max.y = FLT_MIN;
		max.z = FLT_MIN;
			
		min.x = FLT_MAX;
		min.y = FLT_MAX; 
		min.z = FLT_MAX;
			
		for(i = 0; i < 4; i++)
		{
				//transformed_aabb[i].x *= transform_component->scale.x;
				//transformed_aabb[i].y *= transform_component->scale.y;
				//transformed_aabb[i].z *= transform_component->scale.z;
				//mat3_t_vec3_t_mult(&transform_component->orientation, &transformed_aabb[i]);
				
			mat4_t_vec4_t_mult(&world_transform->transform, &transformed_aabb[i]);
			
			if(transformed_aabb[i].x > max.x) max.x = transformed_aabb[i].x;
			if(transformed_aabb[i].y > max.y) max.y = transformed_aabb[i].y;
			if(transformed_aabb[i].z > max.z) max.z = transformed_aabb[i].z;
				
			if(-transformed_aabb[i].x > max.x) max.x = -transformed_aabb[i].x;
			if(-transformed_aabb[i].y > max.y) max.y = -transformed_aabb[i].y;
			if(-transformed_aabb[i].z > max.z) max.z = -transformed_aabb[i].z;
			
		}			
		aabb->current_extents = max;
	}
	//}
}

void entity_UpdateScriptComponents()
{
	int i;
	int c;
	
	//static int last_update = 0;
	
	//struct script_controller_component_t *script_controllers;
	//struct script_controller_component_t *script_controller;
	
	struct script_component_t *script_components;
	struct script_component_t *script_component;
	struct entity_t *entity;
	
	struct stack_list_t *list;
	
	list = &ent_components[0][COMPONENT_TYPE_SCRIPT];
	
	//script_components = (struct script_component_t *)ent_components[0][COMPONENT_TYPE_SCRIPT].elements;
	c = ent_components[0][COMPONENT_TYPE_SCRIPT].element_count;
	
	if(engine_GetEngineState() & ENGINE_JUST_RESUMED)
	{
		for(i = 0; i < c; i++)
		{
			//script_component = script_components + i;	
			script_component = (struct script_component_t *)list->elements + i;
			
			if(script_component->base.flags & COMPONENT_FLAG_INVALID)
			{
				continue;
			}
			
			script_component->flags |= SCRIPT_CONTROLLER_FLAG_FIRST_RUN;	
			script_ExecuteScriptImediate((struct script_t *)script_component->script, script_component);
		}
	}
	else
	{
		for(i = 0; i < c; i++)
		{	
			//script_component = script_components + i;	
			//script_component = entity_GetComponentPointerIndex(i, COMPONENT_TYPE_SCRIPT, 0);
			
			script_component = (struct script_component_t *)list->elements + i;
			
			if(script_component->base.flags & COMPONENT_FLAG_INVALID)
			{
				continue;
			}
			script_ExecuteScriptImediate((struct script_t *)script_component->script, script_component);
		}
	}
	
	//last_update = r_frame;
}

void entity_RecursiveUpdateTransform(struct entity_transform_t *parent_transform, struct component_handle_t child_transform)
{
	//struct entity_transform_t current_transform;
	struct entity_transform_t *global_transform;
	struct transform_component_t *transform_component;
	mat4_t transform;
	int i;
	
	transform_component = entity_GetComponentPointer(child_transform);
	global_transform = entity_GetWorldTransformPointer(child_transform);
	
	if(parent_transform)
	{
		mat4_t_compose2(&transform, &transform_component->orientation, transform_component->position, transform_component->scale);
		mat4_t_mult(&global_transform->transform, &transform, &parent_transform->transform);
	}
	else
	{
		mat4_t_compose2(&global_transform->transform, &transform_component->orientation, transform_component->position, transform_component->scale);
	}
	
	for(i = 0; i < transform_component->children_count; i++)
	{
		entity_RecursiveUpdateTransform(global_transform, transform_component->child_transforms[i]);
	}
	
}

void entity_UpdateTransformComponents()
{
	int i;
	int c;
	int j;
	mat4_t *transform;
	struct entity_t *entity;
	//struct entity_aabb_t *aabb;
	struct model_t *model;
	//ent_entity_transform_cursor = 0;
	bsp_dleaf_t *old_leaf;
	bsp_dleaf_t *cur_leaf;
	struct collider_t *collider;
	struct component_handle_t *top_transforms;
	mat3_t rotation;
	
	struct transform_component_t *transform_components;
	struct transform_component_t *local_transform;
	struct entity_transform_t *world_transform;
	struct entity_transform_t *world_transforms;
	//collider_t *collider;
	struct entity_aabb_t *aabb;
	//struct physics_controller_component_t *physics_controller_components;
	//struct physics_controller_component_t *physics_controller;
	
	//struct physics_component_t *physics_components;
	struct physics_component_t *physics_component;
	
	//struct controller_component_t *controller;
	
	
	transform_components = (struct transform_component_t *)ent_components[0][COMPONENT_TYPE_TRANSFORM].elements;
	world_transforms = (struct entity_transform_t *)ent_world_transforms.elements;
	top_transforms = (struct component_handle_t *)ent_top_transforms.elements;

	c = ent_top_transforms.element_count;
	
	//physics_components = (struct physics_component_t *)ent_components[0][COMPONENT_TYPE_PHYSICS].elements;
	
	for(i = 0; i < c; i++)
	{		
		world_transform = world_transforms + top_transforms[i].index;
		local_transform = transform_components + top_transforms[i].index;
		
		if(local_transform->base.flags & COMPONENT_FLAG_INVALID)
		{
			continue;
		}
		
		if(local_transform->base.entity.entity_index != INVALID_ENTITY_INDEX)
		{
			entity = entity_GetEntityPointerHandle(local_transform->base.entity);
			
			if(entity->components[COMPONENT_TYPE_PHYSICS].type != COMPONENT_TYPE_NONE)
			{
				physics_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_PHYSICS]);
				collider = physics_GetColliderPointerHandle(physics_component->collider.collider_handle);
				
				mat3_t_mult(&rotation, &local_transform->orientation, &collider->orientation);
				
				mat4_t_compose2(&world_transform->transform, &rotation, collider->position, local_transform->scale);			
			}
			else
			{
				mat4_t_compose2(&world_transform->transform, &local_transform->orientation, local_transform->position, local_transform->scale);
			}
			
			if(local_transform->child_transforms)
			{
				for(j = 0; j < local_transform->children_count; j++)
				{
					entity_RecursiveUpdateTransform(world_transform, local_transform->child_transforms[j]);
				}
			}
			
			if(entity->components[COMPONENT_TYPE_MODEL].type != COMPONENT_TYPE_NONE)
			{
				entity_UpdateEntityAabbIndex(local_transform->base.entity);
			}
		}
		
	}
}


void entity_UpdateCameraComponents()
{
	int i;
	int c;
	
	struct camera_component_t *camera_components;
	struct camera_component_t *camera_component;
	struct entity_transform_t *world_transform;
	camera_t *camera;
	
	camera_components = (struct camera_component_t *)ent_components[0][COMPONENT_TYPE_CAMERA].elements;
	c = ent_components[0][COMPONENT_TYPE_CAMERA].element_count;
	
	
	for(i = 0; i < c; i++)
	{
		camera_component = camera_components + i;
		world_transform = entity_GetWorldTransformPointer(camera_component->transform);
		
		if(camera_component->base.flags & COMPONENT_FLAG_INVALID)
		{
			continue;
		}
		
		if(camera_component->camera)
		{
			camera = camera_component->camera;
			
			camera->world_position.x = world_transform->transform.floats[3][0];
			camera->world_position.y = world_transform->transform.floats[3][1];
			camera->world_position.z = world_transform->transform.floats[3][2];
			
			
			camera->world_orientation.floats[0][0] = world_transform->transform.floats[0][0];
			camera->world_orientation.floats[0][1] = world_transform->transform.floats[0][1];
			camera->world_orientation.floats[0][2] = world_transform->transform.floats[0][2];
			
			camera->world_orientation.floats[1][0] = world_transform->transform.floats[1][0];
			camera->world_orientation.floats[1][1] = world_transform->transform.floats[1][1];
			camera->world_orientation.floats[1][2] = world_transform->transform.floats[1][2];
			
			camera->world_orientation.floats[2][0] = world_transform->transform.floats[2][0];
			camera->world_orientation.floats[2][1] = world_transform->transform.floats[2][1];
			camera->world_orientation.floats[2][2] = world_transform->transform.floats[2][2];
			
			camera_ComputeWorldToCameraMatrix(camera);
		}
	}
}

void entity_UpdateEntities()
{
	/*int i;
	int c;
	struct entity_t *entity;
	
	c = ent_entities[0].element_count;
	
	for(i = 0; i < c; i++)
	{
		entity = (struct entity_t *)ent_entities[0].elements + i;
		
		if(entity->flags & ENTITY_INVALID)
		{
			continue;
		}
		
		entity->life++;
	}*/
}


/*
==============================================================
==============================================================
==============================================================
*/

struct entity_handle_t ent_current_entity;

void *entity_SetupScriptDataCallback(struct script_t *script, void *script_controller)
{
	//struct controller_script_t *controller_script;
	struct entity_script_t *entity_script;
	struct entity_t *ent;
	void *entry_point;
//	struct script_controller_component_t *controller;
	struct script_component_t *script_component;
	
	entity_script = (struct entity_script_t *)script;
	script_component = (struct script_component_t *)script_controller;
	//ent = entity_GetEntityPointerHandle(script_component->base.entity);
	
	//controller = entity_GetComponentPointer(ent->components[COMPONENT_INDEX_SCRIPT_CONTROLLER]);
	
//	*entity_script->entity_handle = script_component->base.entity;
	ent_current_entity = script_component->base.entity;
	
	//if(ent->flags & ENTITY_MARKED_INVALID)
	//{
	//	entry_point = entity_script->on_die_entry_point;
	//}
	//else
	{
		if(script_component->flags & SCRIPT_CONTROLLER_FLAG_FIRST_RUN)
		{
			entry_point = entity_script->on_first_run_entry_point;
		}
		else
		{
			entry_point = entity_script->script.main_entry_point;
		}
	}
	
	
	
	script_component->flags &= ~SCRIPT_CONTROLLER_FLAG_FIRST_RUN;
	
	return entry_point;
}

int entity_GetScriptDataCallback(struct script_t *script)
{
	struct entity_script_t *entity_script;
	
	entity_script = (struct entity_script_t *)script;
	entity_script->on_first_run_entry_point = script_GetFunctionAddress("OnFirstRun", script);
	entity_script->on_spawn_entry_point = script_GetFunctionAddress("OnSpawn", script);
	entity_script->on_die_entry_point = script_GetFunctionAddress("OnDie", script);
	
/*	if(!entity_script->entity_handle)
	{
		return 0;
	}*/
	
	
	return 1;
}

struct entity_script_t *entity_LoadScript(char *file_name, char *script_name)
{
	return (struct entity_script_t *)script_LoadScript(file_name, script_name, sizeof(struct entity_script_t), entity_GetScriptDataCallback, entity_SetupScriptDataCallback);
}


/*
==============================================================
==============================================================
==============================================================
*/


#ifdef __cplusplus
}
#endif












