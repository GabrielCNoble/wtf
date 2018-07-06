#include "entity.h"
#include "r_main.h"

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


#define DECLARE_COMPONENT_SIZE(type, size) COMPONENT_SIZES[type]=size





struct component_list_t ent_components[2][COMPONENT_TYPE_LAST];

int ent_entity_def_list_cursor = 0;
int ent_entity_def_list_size = 0;
int ent_entity_def_list_free_stack_top = -1;
int *ent_entity_def_list_free_stack = NULL;
struct entity_t *ent_entity_defs = NULL;


struct entity_aabb_t *ent_entity_aabbs = NULL;
struct entity_transform_t *ent_global_transforms = NULL;
int *ent_top_transform_components = NULL;

int ent_entity_list_cursor = 0;
int ent_entity_list_size = 0;
int ent_free_stack_top = -1;
int *ent_free_stack = NULL;
struct entity_t *ent_entities = NULL;


int ent_max_top_transforms = 0;
int ent_top_transform_count = 0;
struct component_handle_t *ent_top_transforms = NULL;


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
 
int entity_Init()
{
	
	/* this is less than ideal, but the memory footprint is small (less than 15 MB),
	and aliviates the need of having resizing code (or using stl containers for
	growable buffers)... */
	
	int i;
	int def_list;
	int component_type;
	ent_entity_list_size = MAX_ENTITIES;
	ent_entities = memory_Malloc(sizeof(struct entity_t) * ent_entity_list_size, "entity_Init");
	ent_free_stack = memory_Malloc(sizeof(int) * ent_entity_list_size, "entity_Init");
	
	for(i = 0; i < ent_entity_list_size; i++)
	{
		ent_entities[i].name = memory_Malloc(ENTITY_NAME_MAX_LEN, "entity_Init");
		ent_entities[i].name[0] = '\0';
	}
	
	ent_entity_aabbs = memory_Malloc(sizeof(struct entity_aabb_t) * ent_entity_list_size, "entity_Init");
	ent_global_transforms = memory_Malloc(sizeof(struct entity_transform_t) * ent_entity_list_size, "entity_Init");
	
	ent_entity_def_list_size = MAX_ENTITY_DEFS;
	ent_entity_defs = memory_Malloc(sizeof(struct entity_t) * ent_entity_def_list_size, "entity_Init");
	ent_entity_def_list_free_stack = memory_Malloc(sizeof(int) * ent_entity_def_list_size, "entity_Init");
	
	for(i = 0; i < ent_entity_def_list_size; i++)
	{
		ent_entity_defs[i].name = memory_Malloc(ENTITY_NAME_MAX_LEN, "entity_Init");
		ent_entity_defs[i].name[0] = '\0';
	}
	
	
	/*
	===============================================================
	===============================================================
	===============================================================
	*/
	
	
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_TRANSFORM, sizeof(struct transform_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_PHYSICS_CONTROLLER, sizeof(struct physics_controller_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_SCRIPT_CONTROLLER, sizeof(struct script_controller_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_MODEL, sizeof(struct model_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_LIGHT, sizeof(struct light_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_SCRIPT, sizeof(struct script_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_CAMERA, sizeof(struct camera_component_t));
	
	for(def_list = 0; def_list < 2; def_list++)
	{
		for(component_type = COMPONENT_TYPE_TRANSFORM; component_type < COMPONENT_TYPE_LAST; component_type++)
		{
			ent_components[def_list][component_type] = entity_CreateComponentList(COMPONENT_SIZES[component_type], 32);
		}
	}

	id_rot = mat3_t_id();	
		
	return 1;
}

void entity_Finish()
{
	int i;
	int j;
	
	for(i = 0; i < ent_entity_list_size; i++)
	{
		memory_Free(ent_entities[i].name);
	}
	
	for(i = 0; i < ent_entity_def_list_size; i++)
	{
		memory_Free(ent_entity_defs[i].name);
	}
	
	memory_Free(ent_entities);
	memory_Free(ent_free_stack);
	
	memory_Free(ent_entity_defs);
	memory_Free(ent_entity_def_list_free_stack);
	
	memory_Free(ent_entity_aabbs);
	memory_Free(ent_global_transforms);
	
	if(ent_top_transforms)
	{
		memory_Free(ent_top_transforms);
	}
	
	
	for(j = 0; j < 2; j++)
	{
		for(i = COMPONENT_TYPE_TRANSFORM; i < COMPONENT_TYPE_LAST; i++)
		{
			entity_DestroyComponentList(&ent_components[j][i]);
		}
	}
}


struct component_list_t entity_CreateComponentList(int component_size, int max_components)
{
	struct component_list_t list;
	
	list.component_size = component_size;
	list.component_count = 0;
	list.max_components = max_components;
	list.free_stack_top = -1;
	list.free_stack = memory_Malloc(sizeof(int) * max_components, "entity_CreateComponentList");
	list.components = memory_Calloc(component_size, max_components, "entity_CreateComponentList");
	
	return list;
}

void entity_DestroyComponentList(struct component_list_t *component_list)
{
	if(component_list->components)
	{
		memory_Free(component_list->free_stack);
		memory_Free(component_list->components);
	}
	
}

int entity_AddComponentToList(struct component_list_t *component_list, void *component)
{
	int index;
	
	void *components;
	
	if(component_list->free_stack_top >= 0)
	{
		index = component_list->free_stack[component_list->free_stack_top];
		component_list->free_stack_top--;
	}
	else
	{
		index = component_list->component_count;
		component_list->component_count++;
		
		if(index >= component_list->max_components)
		{
			memory_Free(component_list->free_stack);
			component_list->free_stack = memory_Malloc(sizeof(int) * (component_list->max_components + 32), "entity_AddComponentToList");
			
			components = memory_Malloc(component_list->component_size * (component_list->max_components + 32), "entity_AddComponentToList");
			memcpy(components, component_list->components, component_list->component_size * component_list->max_components);
			memory_Free(component_list->components);
			component_list->components = components;
			component_list->max_components += 32;
		}
	}
	
	if(component)
	{
		memcpy((char *)component_list->components + component_list->component_size * index, component, component_list->component_size);
	}
	
	return index;
}

void entity_RemoveComponentFromList(struct component_list_t *component_list, int index)
{
	if(index >= 0 && index < component_list->max_components)
	{
		component_list->free_stack_top++;
		component_list->free_stack[component_list->free_stack_top] = index;
	}
}


/*
==============================================================
==============================================================
==============================================================
*/

inline struct component_list_t *entity_ListForType(int type, int def_list)
{
	//if(type > COMPONENT_TYPE_NONE && type < COMPONENT_TYPE_LAST)
	if(type != COMPONENT_TYPE_NONE)
	{
		def_list = def_list && 1;
		return &ent_components[def_list][type];
	}	
	return NULL;	
}



inline int entity_IndexForType(int component_type)
{
	switch(component_type)
	{
	 	case COMPONENT_TYPE_TRANSFORM:
	 		return COMPONENT_INDEX_TRANSFORM;																												
																				
		case COMPONENT_TYPE_PHYSICS_CONTROLLER:		
		case COMPONENT_TYPE_SCRIPT_CONTROLLER:				
			return COMPONENT_INDEX_CONTROLLER;																											
																				
		case COMPONENT_TYPE_MODEL:									
			return COMPONENT_INDEX_MODEL;												
		
		case COMPONENT_TYPE_LIGHT:
			return COMPONENT_INDEX_LIGHT;
		
		case COMPONENT_TYPE_SCRIPT:
			return COMPONENT_INDEX_SCRIPT;
		
		case COMPONENT_TYPE_CAMERA:
			return COMPONENT_INDEX_CAMERA;
	}	
	
	return -1;
}




struct component_handle_t entity_AllocComponent(int component_type, int alloc_for_def)
{
	struct component_list_t *list;
	struct component_t *component;
	struct transform_component_t *transform;
	struct component_handle_t *transforms;
	struct component_handle_t handle;
	
	handle.def = 0;
	handle.type = COMPONENT_TYPE_NONE;
	handle.index = 0;

	list = entity_ListForType(component_type, alloc_for_def);
	
	alloc_for_def = alloc_for_def && 1;
	
	if(list)
	{
		handle.index = entity_AddComponentToList((struct component_list_t *)list, NULL);
		handle.type = component_type;
		handle.def = alloc_for_def;
		
		component = entity_GetComponentPointer(handle);
		component->type = component_type;
		
		if(!alloc_for_def)
		{
			switch(component_type)
			{
				case COMPONENT_TYPE_TRANSFORM:
					entity_AddTransformToTopList(handle);
				break;
			}
		}
		
		
	}
	else
	{
		printf("entity_AllocComponent: bad component type\n");
	}

	return handle;
}

 
void entity_DeallocComponent(struct component_handle_t component)
{
	struct component_list_t *list;
	
	list = entity_ListForType(component.type, component.def);
	
	if(list)
	{
		if(!component.def)
		{
			if(component.type == COMPONENT_TYPE_TRANSFORM)
			{
				entity_RemoveTransformFromTopList(component);
			}
		}
		
		
		entity_RemoveComponentFromList(list, component.index);
	}
}

struct component_handle_t entity_AllocTransform(int alloc_for_def)
{
	struct transform_component_t *transform;
	struct component_handle_t handle = entity_AllocComponent(COMPONENT_TYPE_TRANSFORM, alloc_for_def);
	
	transform = entity_GetComponentPointer(handle);
	
	transform->orientation = mat3_t_id();
	transform->position = vec3(0.0, 0.0, 0.0);
	transform->scale = vec3(1.0, 1.0, 1.0);
	
	return handle;
}

void entity_DeallocTransform(struct component_handle_t transform)
{
	entity_DeallocComponent(transform);
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
	
//	if(transform_ptr->top_list_index >= 0 && transform_ptr->top_list_index < ent_top_transform_count)
//	{
//		printf("entity_AddTransformToTopList: transform component already in top list\n");
//		return;
//	}
	
	if(ent_top_transform_count >= ent_max_top_transforms)
	{
		transforms = memory_Malloc(sizeof(struct component_handle_t) * (ent_max_top_transforms + 64), "entity_AllocComponent");
				
		if(ent_top_transforms)
		{
			memcpy(transforms, ent_top_transforms, sizeof(struct component_handle_t) * ent_max_top_transforms);
			memory_Free(ent_top_transforms);
		}
						
		ent_top_transforms = transforms;
		ent_max_top_transforms += 64;
	}
					
	ent_top_transforms[ent_top_transform_count] = transform;
	transform_ptr->top_list_index = ent_top_transform_count;
	ent_top_transform_count++;
}

void entity_RemoveTransformFromTopList(struct component_handle_t transform)
{
	struct transform_component_t *transform_ptr;
	struct transform_component_t *other;
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
	
	if(transform_ptr->top_list_index < ent_top_transform_count - 1)
	{
		ent_top_transforms[transform_ptr->top_list_index] = ent_top_transforms[ent_top_transform_count - 1];
		other = entity_GetComponentPointer(ent_top_transforms[transform_ptr->top_list_index]);
		other->top_list_index = transform_ptr->top_list_index;
	}
	
	ent_top_transform_count--;
	transform_ptr->top_list_index = -1;
}

void entity_ParentTransformComponent(struct component_handle_t parent_transform, struct component_handle_t child_transform)
{
	struct transform_component_t *parent;
	struct transform_component_t *child;
	struct transform_component_t *other;
	struct component_handle_t *transforms;
	
	if(parent_transform.def != child_transform.def)
	{
		printf("entity_ParentTransformComponent: cannot parent def transform with non def transform\n");
		return;
	}
	
	parent = entity_GetComponentPointer(parent_transform);
	child = entity_GetComponentPointer(child_transform);
	
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
		entity_RemoveTransformFromTopList(child_transform);
	}
}

void entiyt_UnparentTransformComponent(struct component_handle_t parent_transform, struct component_handle_t child_transform)
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
			
			entity_AddTransformToTopList(child_transform);
		}
	}
	
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
	
	handle.entity_index = INVALID_ENTITY_INDEX;	
		
	if(ent_entity_def_list_free_stack_top >= 0)
	{
		entity_def_index = ent_entity_def_list_free_stack[ent_entity_def_list_free_stack_top];
		ent_entity_def_list_free_stack_top--;
	}
	else
	{
		if(ent_entity_def_list_cursor < MAX_ENTITY_DEFS)
		{
			entity_def_index = ent_entity_def_list_cursor;
			ent_entity_def_list_cursor++;
		}
		else
		{
			printf("entity_CreateEntity: couldn't create entity def [%s]. Too many entity defs!\n", name);
			return handle;
		}
	}
	
	entity_def = ent_entity_defs + entity_def_index;
	
	for(i = 0; i < COMPONENT_INDEX_LAST; i++)
	{
		entity_def->components[i].type = COMPONENT_TYPE_NONE;
	}
	
	handle.def = 1;
	handle.entity_index = entity_def_index;
	
	entity_def->components[COMPONENT_INDEX_TRANSFORM] = entity_AllocComponent(COMPONENT_TYPE_TRANSFORM, 1);
	transform_component = entity_GetComponentPointer(entity_def->components[COMPONENT_INDEX_TRANSFORM]);
	
	transform_component->children_count = 0;
	transform_component->max_children = 4;
	transform_component->child_transforms = memory_Malloc(sizeof(struct component_handle_t) * transform_component->max_children, "entity_CreateEntityDef");
	
	for(i = 0; i < transform_component->max_children; i++)
	{
		transform_component->child_transforms[i].type = COMPONENT_TYPE_NONE;
	}
	
	
		
	entity_def->leaf = NULL;
	entity_def->flags = 0;
	
	for(i = 0; name[i] && i < ENTITY_NAME_MAX_LEN - 1; i++)
	{
		entity_def->name[i] = name[i];
	}
	
	entity_def->name[i] = '\0';
	
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
	struct entity_aabb_t *aabb;
	struct component_t *component_ptr;
	
	component.type = COMPONENT_TYPE_NONE;
		
	entity_ptr = entity_GetEntityPointerIndex(entity);
	
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
		if(entity_ptr->components[COMPONENT_INDEX_TRANSFORM].type != COMPONENT_TYPE_NONE)
		{
			printf("entity_AddComponent: cannot overwrite entity's transform component\n");
			return component;
		}
	}
	
	
	component = entity_AllocComponent(component_type, entity.def);
	component_ptr = entity_GetComponentPointer(component);
	
	if(component.type != COMPONENT_TYPE_NONE)
	{
		component_index = entity_IndexForType(component_type);
		entity_ptr->components[component_index] = component;
		component_ptr->entity = entity;
		
		switch(component_ptr->type)
		{
			case COMPONENT_TYPE_SCRIPT_CONTROLLER:
				script_controller = (struct script_controller_component_t *)component_ptr;
				script_controller->flags = SCRIPT_CONTROLLER_FLAG_FIRST_RUN;
			break;
			
			case COMPONENT_TYPE_CAMERA:
				component_transform = entity_AllocComponent(COMPONENT_TYPE_TRANSFORM, entity.def);
				camera_component = (struct camera_component_t *)component_ptr;
				
				camera_component->transform = component_transform;
				
				entity_ParentTransformComponent(entity_ptr->components[COMPONENT_INDEX_TRANSFORM], component_transform);
			break;
		}
	}
	
	//controller->flags = 0;
}

void entity_SetModel(struct entity_handle_t entity, int model_index)
{
	struct entity_t *entity_ptr;
	struct model_component_t *model_component;
	struct entity_aabb_t *aabb;
	struct model_t *model;
	
	int component;
	
	entity_ptr = entity_GetEntityPointerIndex(entity);
	
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
	
	
	model_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_MODEL]);
	
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
		aabb = ent_entity_aabbs + entity_ptr->components[COMPONENT_INDEX_TRANSFORM].index;
		
		aabb->original_extents = model->aabb_max;
		aabb->current_extents = aabb->original_extents;
	}
	
}


void entity_SetCollider(struct entity_handle_t entity, void *collider)
{
	struct controller_component_t *controller;
	struct physics_controller_component_t *physics_controller;
	struct entity_t *entity_ptr;

	entity_ptr = entity_GetEntityPointerIndex(entity);
	
	if(!entity_ptr)
	{
		if(entity.def)
		{
			printf("entity_SetEntityCollider: bad entity def handle\n");
		}
		else
		{
			printf("entity_SetEntityCollider: bad entity handle\n");
		}
		return;
	}
	
	
	controller = (struct controller_component_t *)entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_CONTROLLER]);
		
	if(!controller)
	{
		if(entity.def)
		{
			printf("entity_SetEntityCollider: entity def [%s] doesn't have a valid controller component\n", entity_ptr->name);
		}
		else
		{
			printf("entity_SetEntityCollider: entity [%s] doesn't have a valid controller component\n", entity_ptr->name);
		}
			
		return;
	}
		
	if(entity.def)
	{
		controller->collider.collider_def = collider;
	}
	else
	{
		controller->collider.collider_index = (int)collider;
	}
	
	//controller->type = entity_ptr->components[COMPONENT_INDEX_CONTROLLER].type;	
	controller->flags = 0;
}


void entity_SetControllerScript(struct entity_handle_t entity, void *script)
{
	struct entity_t *entity_ptr;
	struct script_controller_component_t *controller;
		
	entity_ptr = entity_GetEntityPointerIndex(entity);
	
	if(!entity_ptr)
	{
		if(entity.def)
		{
			printf("entity_SetControllerScript: bad entity def handle\n");
		}
		else
		{
			printf("entity_SetControllerScript: bad entity handle\n");
		}
		return;
	}
	
	controller = entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_SCRIPT_CONTROLLER]);
	
	if(!controller)
	{
		if(entity.def)
		{
			printf("entity_SetControllerScript: entity def [%s] doesn't have a valid script controller component\n", entity_ptr->name);
		}
		else
		{
			printf("entity_SetControllerScript: entity [%s] doesn't have a valid script controller component\n", entity_ptr->name);
		}
		return;
	}
	
	if(controller->controller.base.type != COMPONENT_TYPE_SCRIPT_CONTROLLER)
	{
		if(entity.def)
		{
			printf("entity_SetControllerScript: entity def [%s] has incorrect controller component\n", entity_ptr->name);
		}
		else
		{
			printf("entity_SetControllerScript: entity [%s] has incorrect  controller component\n", entity_ptr->name);
		}
		return;
	}
	
	controller->script = script;
	controller->flags = SCRIPT_CONTROLLER_FLAG_FIRST_RUN;
}

void entity_SetCameraTransform(struct entity_handle_t entity, mat3_t *orientation, vec3_t position)
{
	struct entity_t *entity_ptr;
	struct transform_component_t *transform_component;
	struct camera_component_t *camera_component;
	
	entity_ptr = entity_GetEntityPointerIndex(entity);
	
	camera_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_CAMERA]);
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
	transform_component->scale = vec3(1.0, 1.0, 1.0);	
}

void entity_SetCamera(struct entity_handle_t entity, camera_t *camera)
{
	struct entity_t *entity_ptr;
	struct camera_component_t *camera_component;
	
	entity_ptr = entity_GetEntityPointerIndex(entity);
	camera_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_CAMERA]);
	camera_component->camera = camera;
}

/*
==============================================================
==============================================================
==============================================================
*/


struct entity_handle_t entity_SpawnEntity(mat3_t *orientation, vec3_t position, vec3_t scale, struct entity_handle_t entity_def, char *name)
{
	struct entity_t *entity_ptr;
	struct entity_t *entity_def_ptr;
	struct entity_handle_t handle;
	struct model_component_t *model_component;
	struct model_component_t *def_model_component;
	struct transform_component_t *transform_component;
	
	struct camera_component_t *camera_component;
	struct transform_component_t *camera_transform_component;
	struct camera_component_t *def_camera_component;
	struct transform_component_t *def_camera_transform_component;
	
	struct script_controller_component_t *script_controller;
	struct script_controller_component_t *def_script_controller;
	
	//struct physics_controller_component_t *physics_controller;
	//struct physics_controller_component_t *def_physics_controller;
	
	struct controller_component_t *controller;
	struct controller_component_t *def_controller;
	struct component_t *component_ptr;
	
	struct component_handle_t component_transform;
	
	struct entity_aabb_t *aabb;
	struct model_t *model;
	int entity_index;
	
	struct component_handle_t component;
	
	int i;
	int j;
	
	if(ent_free_stack_top >= 0)
	{
		entity_index = ent_free_stack[ent_free_stack_top];
		ent_free_stack_top--;
	}
	else
	{
		if(ent_entity_list_cursor < MAX_ENTITIES)
		{
			entity_index = ent_entity_list_cursor;
			ent_entity_list_cursor++;
		}
	} 
	
	entity_ptr = ent_entities + entity_index;
	entity_def_ptr = entity_GetEntityPointerIndex(entity_def);
	
	handle.def = 0;
	handle.entity_index = entity_index;
	
	//entity_def = ent_entity_defs + entity_def_index;
	
	for(i = 0; i < COMPONENT_INDEX_LAST; i++)
	{
		entity_ptr->components[i].type = COMPONENT_TYPE_NONE;
	}
	
	entity_ptr->components[COMPONENT_INDEX_TRANSFORM] = entity_AllocComponent(COMPONENT_TYPE_TRANSFORM, 0);
	transform_component = (struct transform_component_t *)entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_TRANSFORM]);
	aabb = ent_entity_aabbs + entity_ptr->components[COMPONENT_INDEX_TRANSFORM].index;
	
	aabb->current_extents.x = 0.0;
	aabb->current_extents.y = 0.0;
	aabb->current_extents.z = 0.0;
	
	aabb->original_extents.x = 0.0;
	aabb->original_extents.y = 0.0;
	aabb->original_extents.z = 0.0;
	
	for(i = COMPONENT_INDEX_TRANSFORM + 1; i < COMPONENT_INDEX_LAST; i++)
	{
		//switch(entity_def_ptr->components[i].type)
		if(entity_def_ptr->components[i].type != COMPONENT_TYPE_NONE)
		{
			component = entity_def_ptr->components[i];
			entity_ptr->components[i] = entity_AllocComponent(component.type, 0);
			
			switch(i)
			{
				case COMPONENT_INDEX_MODEL:
					model_component = (struct model_component_t *)entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_MODEL]);
					def_model_component = (struct model_component_t *)entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_INDEX_MODEL]);
					
					model_component->model_index = def_model_component->model_index;
					
					model = model_GetModelPointerIndex(model_component->model_index);
					
					aabb->original_extents = model->aabb_max;
					aabb->current_extents = aabb->original_extents;
				break;	
				
				case COMPONENT_INDEX_CONTROLLER:
					controller = entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_CONTROLLER]);
					def_controller = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_INDEX_CONTROLLER]);
						
					controller->collider.collider_index = physics_CreateCollider(orientation, position, scale, def_controller->collider.collider_def, 0);
					
					if(controller->base.type == COMPONENT_TYPE_SCRIPT_CONTROLLER)
					{
						def_script_controller = (struct script_controller_component_t *)def_controller;
						script_controller = (struct script_controller_component_t *)controller;
						script_controller->script = def_script_controller->script;
						script_controller->flags = def_script_controller->flags;
						
						script_controller->route = NULL;
						script_controller->route_length = 0;
						script_controller->max_route_length = 0;
					}
				break;
				
				case COMPONENT_INDEX_CAMERA:
					camera_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_CAMERA]);
					def_camera_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_INDEX_CAMERA]);
					component_transform = entity_AllocComponent(COMPONENT_TYPE_TRANSFORM, 0);
					camera_component->transform = component_transform;
					entity_ParentTransformComponent(entity_ptr->components[COMPONENT_INDEX_TRANSFORM], component_transform);
					
					
					camera_transform_component = entity_GetComponentPointer(camera_component->transform);
					def_camera_transform_component = entity_GetComponentPointer(def_camera_component->transform);
					
					camera_transform_component->orientation = def_camera_transform_component->orientation;
					camera_transform_component->position = def_camera_transform_component->position;
					camera_transform_component->scale = def_camera_transform_component->scale;
					
					camera_component->camera = camera_CreateCamera("camera", vec3(0.0, 0.0, 0.0), NULL, 0.68, 1366.0, 768.0, 0.1, 500.0, 0);
				break;
			}
		}
		
	}

	
	//transform_component->children_count = 0;
	//transform_component->flags = 0;
	
	transform_component->orientation = *orientation;
	transform_component->position = position;
	transform_component->scale = scale;
	
	transform_component->parent.type = COMPONENT_TYPE_NONE;
	
	for(i = 0; i < COMPONENT_INDEX_LAST; i++)
	{
		if(entity_ptr->components[i].type != COMPONENT_TYPE_NONE)
		{
			component_ptr = (struct component_t *)entity_GetComponentPointer(entity_ptr->components[i]);
			component_ptr->entity = handle;
		}
	}
	
	for(i = 0; name[i] && i < ENTITY_NAME_MAX_LEN - 1; i++)
	{
		entity_ptr->name[i] = name[i];
	}
	
	/* TODO: make sure no two entities in the world
	have the same name... */
	entity_ptr->name[i] = '\0';
	
	return handle;
}

void entity_RemoveEntity(int entity_index)
{
	
}

struct entity_t *entity_GetEntityPointer(char *name, int get_def)
{
	int i;
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
	
	return NULL;
}

//int entity_CopyEntity(struct entity_t *entity)
//{
//	if(entity)
//	{
	/*	if(!(entity->flags & ENTITY_INVALID))
		{
			return entity_CreateEntity(entity->name, entity->position, entity->scale, &entity->orientation, entity->def_index);
		}*/
//	}
	
//	return -1;
//}

//int entity_CopyEntityIndex(int entity_index)
//{
//	struct entity_t *entity;
	
//	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
//	{
//		entity = &ent_entities[entity_index];
//		return entity_CopyEntity(entity);
//	}
	
//	return -1;
//}

//int entity_DestroyEntity(char *name)
//{
//	int i;
	
//	for(i = 0; i < ent_entity_list_cursor; i++)
//	{
		/*if(ent_entities[i].flags & ENTITY_INVALID)
			continue;
			
		if(!strcmp(name, ent_entities[i].name))
		{
			return entity_DestroyEntityIndex(i);
		}	*/
//	}
	
//	return 0;
//}

//int entity_DestroyEntityIndex(int entity_index)
//{
//	bsp_dleaf_t *leaf;
//	int leaf_index;
//	int i;
	
//	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
//	{
		/*if(!(ent_entities[entity_index].flags & ENTITY_INVALID))
		{
			ent_entities[entity_index].name[0] = '\0';
			ent_entities[entity_index].flags |= ENTITY_INVALID;
			
			model_DecModelMaterialsRefs(ent_entities[entity_index].model_index);
			
			leaf = ent_entities[entity_index].leaf;
			
			if(leaf)
			{
				leaf_index = leaf - w_world_leaves;
			
				w_leaf_entities[leaf_index].entities[entity_index >> 5] &= ~(1 << (entity_index % 32));
				
				for(i = 0; i < w_world_leaves_count; i++)
				{
					if(leaf->pvs[i >> 3] & (1 << (i % 8)))
					{
						w_leaf_entities[i].entities[entity_index >> 5] &= ~(1 << (entity_index % 32));
					}
				}
			}
			
			
			
			ent_entities[entity_index].model_index = -1;
			ent_entities[entity_index].leaf = NULL;
			
			if(ent_entities[entity_index].collider_index > -1)
			{
				physics_DestroyColliderIndex(ent_entities[entity_index].collider_index);
				ent_entities[entity_index].collider_index = -1;
			}
			
			return 0;
		}*/
//	}
	
//	return 1;
//}

//int entity_GetEntity(char *name)
//{
//	
//}

//struct entity_t *entity_GetEntityPointer(char *name)
//{
//	
//}

//struct entity_t *entity_GetEntityPointerIndex(int entity_index)




void entity_TranslateEntity(int entity_index, vec3_t direction, float amount)
{
	struct entity_t *entity;
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
	
	}
	
}

void entity_RotateEntity(int entity_index, vec3_t axis, float amount)
{
	struct entity_t *entity;
	
	
	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
	{
		entity = &ent_entities[entity_index];
		
	/*	if(!(entity->flags & ENTITY_INVALID))
		{			
			mat3_t_rotate(&entity->orientation, axis, amount, 0);
			entity->flags |= ENTITY_HAS_MOVED;
			
			if(entity->collider_index > -1)
			{
				physics_SetColliderOrientation(entity->collider_index, &entity->orientation);
			}
			
		}*/
	}
}

#define ENTITY_MIN_SCALE 0.01

void entity_ScaleEntity(int entity_index, vec3_t axis, float amount)
{
	struct entity_t *entity;
	
	
	if(entity_index >= 0 && entity_index < ent_entity_list_cursor)
	{
		entity = &ent_entities[entity_index];
		
		/*if(!(entity->flags & ENTITY_INVALID))
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
		}*/
	}
}


void entity_FindPath(struct entity_handle_t entity, vec3_t to)
{
	//struct transform_component_t *transform;
	struct entity_transform_t *transform;
	struct script_controller_component_t *controller;
	struct entity_t *entity_ptr;
	struct waypoint_t **route;
	struct waypoint_t *waypoint;
	int route_length;
	int i;
	
	entity_ptr = entity_GetEntityPointerIndex(entity);
	
	//transform = entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_TRANSFORM]);
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
	struct entity_transform_t *global_transform;
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
	
	
	if(entity.entity_index >= 0 && entity.entity_index < ent_entity_list_cursor)
	{
		entity_ptr = &ent_entities[entity.entity_index];
			
		if(!(entity_ptr->flags & ENTITY_INVALID))
		{
			
			//transform_component = (struct transform_component_t *)ent_transform_components.components + entity->components[COMPONENT_INDEX_TRANSFORM];
			transform_component = (struct transform_component_t *)entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_TRANSFORM]);
			global_transform = ent_global_transforms + entity_ptr->components[COMPONENT_INDEX_TRANSFORM].index;
			aabb = ent_entity_aabbs + entity_ptr->components[COMPONENT_INDEX_TRANSFORM].index;
			
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
				
				mat4_t_vec4_t_mult(&global_transform->transform, &transformed_aabb[i]);
				
				if(transformed_aabb[i].x > max.x) max.x = transformed_aabb[i].x;
				if(transformed_aabb[i].y > max.y) max.y = transformed_aabb[i].y;
				if(transformed_aabb[i].z > max.z) max.z = transformed_aabb[i].z;
				
				if(-transformed_aabb[i].x > max.x) max.x = -transformed_aabb[i].x;
				if(-transformed_aabb[i].y > max.y) max.y = -transformed_aabb[i].y;
				if(-transformed_aabb[i].z > max.z) max.z = -transformed_aabb[i].z;
			
			}			
			aabb->current_extents = max;
		}
	}
}

void entity_UpdateScriptControllerComponents()
{
	int i;
	int c;
	
	//static int last_update = 0;
	
	struct script_controller_component_t *script_controllers;
	struct script_controller_component_t *script_controller;
	struct entity_t *entity;
	
	script_controllers = (struct script_controller_component_t *)ent_components[0][COMPONENT_TYPE_SCRIPT_CONTROLLER].components;
	c = ent_components[0][COMPONENT_TYPE_SCRIPT_CONTROLLER].component_count;
	
	if(engine_GetEngineState() & ENGINE_JUST_RESUMED)
	{
		for(i = 0; i < c; i++)
		{
			script_controller = script_controllers + i;	
			script_controller->flags |= SCRIPT_CONTROLLER_FLAG_FIRST_RUN;	
			script_ExecuteScriptImediate((struct script_t *)script_controller->script, script_controller);
		}
	}
	else
	{
		for(i = 0; i < c; i++)
		{
			script_controller = script_controllers + i;		
			script_ExecuteScriptImediate((struct script_t *)script_controller->script, script_controller);
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
	collider_t *collider;
	mat3_t rotation;
	
	struct transform_component_t *transform_components;
	struct transform_component_t *local_transform;
	struct entity_transform_t *global_transform;
	//collider_t *collider;
	struct entity_aabb_t *aabb;
	struct physics_controller_component_t *physics_controller_components;
	struct physics_controller_component_t *physics_controller;
	struct controller_component_t *controller;
	
	
	transform_components = (struct transform_component_t *)ent_components[0][COMPONENT_TYPE_TRANSFORM].components;
	//c = ent_components[0][COMPONENT_TYPE_TRANSFORM].component_count;
	c = ent_top_transform_count;
	
	physics_controller_components = (struct physics_controller_component_t *)ent_components[0][COMPONENT_TYPE_PHYSICS_CONTROLLER].components;
	
	for(i = 0; i < c; i++)
	{		
		global_transform = ent_global_transforms + ent_top_transforms[i].index;
		local_transform = transform_components + ent_top_transforms[i].index;
		
		if(local_transform->base.entity.entity_index != INVALID_ENTITY_INDEX)
		{
			entity = ent_entities + local_transform->base.entity.entity_index;
			
			if(entity->components[COMPONENT_INDEX_CONTROLLER].type != COMPONENT_TYPE_NONE)
			{
				controller = entity_GetComponentPointer(entity->components[COMPONENT_INDEX_CONTROLLER]);
				collider = physics_GetColliderPointerIndex(controller->collider.collider_index);
				
				mat3_t_mult(&rotation, &local_transform->orientation, &collider->orientation);
				
				mat4_t_compose2(&global_transform->transform, &rotation, collider->position, local_transform->scale);			
			}
			else
			{
				mat4_t_compose2(&global_transform->transform, &local_transform->orientation, local_transform->position, local_transform->scale);
			}
			
			for(j = 0; j < local_transform->children_count; j++)
			{
				entity_RecursiveUpdateTransform(global_transform, local_transform->child_transforms[j]);
			}
			
			if(entity->components[COMPONENT_INDEX_MODEL].type != COMPONENT_TYPE_NONE)
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
	
	camera_components = (struct camera_component_t *)ent_components[0][COMPONENT_TYPE_CAMERA].components;
	c = ent_components[0][COMPONENT_TYPE_CAMERA].component_count;
	
	
	for(i = 0; i < c; i++)
	{
		camera_component = camera_components + i;
		world_transform = entity_GetWorldTransformPointer(camera_component->transform);
		
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


/*
==============================================================
==============================================================
==============================================================
*/

struct entity_handle_t ent_current_entity;

void *entity_SetupScriptDataCallback(struct script_t *script, void *script_controller)
{
	struct controller_script_t *controller_script;
	struct entity_t *ent;
	void *entry_point;
	struct script_controller_component_t *controller;
	
	controller_script = (struct controller_script_t *)script;
	controller = (struct script_controller_component_t *)script_controller;
	ent = entity_GetEntityPointerIndex(controller->controller.base.entity);
	
	controller = entity_GetComponentPointer(ent->components[COMPONENT_INDEX_SCRIPT_CONTROLLER]);
	
	*controller_script->entity_handle = controller->controller.base.entity;
	ent_current_entity = controller->controller.base.entity;
	
	if(controller->flags & SCRIPT_CONTROLLER_FLAG_FIRST_RUN)
	{
		entry_point = controller_script->on_first_run_entry_point;
	}
	else
	{
		entry_point = controller_script->script.main_entry_point;
	}
	
	controller->flags &= ~SCRIPT_CONTROLLER_FLAG_FIRST_RUN;
	
	return entry_point;
}

int entity_GetScriptDataCallback(struct script_t *script)
{
	struct controller_script_t *controller_script;
	
	controller_script = (struct controller_script_t *)script;
	controller_script->entity_handle = (struct entity_handle_t *)script_GetGlobalVarAddress("entity", script);
	controller_script->on_first_run_entry_point = script_GetFunctionAddress("OnFirstRun", script);
	
	if(!controller_script->entity_handle)
	{
		return 0;
	}
	
	
	return 1;
}

struct controller_script_t *entity_LoadScript(char *file_name, char *script_name)
{
	return (struct controller_script_t *)script_LoadScript(file_name, script_name, sizeof(struct controller_script_t), entity_GetScriptDataCallback, entity_SetupScriptDataCallback);
}


/*
==============================================================
==============================================================
==============================================================
*/

void entity_SerializeEntities(void **buffer, int *buffer_size)
{
	#if 0
	entity_section_header_t *header;
	entity_def_record_t *def_record;
	entity_record_t *ent_record;
	collider_def_t *collider_def;
	model_t *model;
	char *name;
	int name_offset;
	
	int entity_def_count = 0;
	int entity_count = 0;
	
	int i;
	int j;
	
	char *out;
	int out_size = 0;
	
	out_size = sizeof(entity_section_header_t);
	
	
	for(i = 0; i < ent_entity_def_list_cursor; i++)
	{
		if(ent_entity_defs[i].type == ENTITY_TYPE_INVALID)
		{
			continue;
		}
		
		out_size += sizeof(entity_def_record_t);
		entity_def_count++;
	}
	
	for(i = 0; i < ent_entity_list_cursor; i++)
	{
		if(ent_entities[i].flags & ENTITY_INVALID)
		{
			continue;
		}
		
		out_size += sizeof(entity_record_t);
		entity_count++;
	}
	
	
	out = memory_Malloc(out_size, "entity_SerializeEntities");
	
	*buffer = out;
	*buffer_size = out_size;
	header = (entity_section_header_t *)out;
	out += sizeof(entity_section_header_t);
	
	
	header->tag[0]  = '[';
	header->tag[1]  = 'e';
	header->tag[2]  = 'n';
	header->tag[3]  = 't';
	header->tag[4]  = 'i';
	header->tag[5]  = 't';
	header->tag[6]  = 'y';
	header->tag[7]  = '_';
	header->tag[8]  = 's';
	header->tag[9]  = 'e';
	header->tag[10] = 'c';
	header->tag[11] = 't';
	header->tag[12] = 'i';
	header->tag[13] = 'o';
	header->tag[14] = 'n';
	header->tag[15] = ']';
	header->tag[16] = '\0';
	header->tag[17] = '\0';
	header->tag[18] = '\0';
	header->tag[19] = '\0';
	
	header->entity_count = entity_count;
	header->entity_def_count = entity_def_count;
	
	header->reserved0 = 0;
	header->reserved1 = 0;
	header->reserved2 = 0;
	header->reserved3 = 0;
	header->reserved4 = 0;
	header->reserved5 = 0;
	header->reserved6 = 0;
	header->reserved7 = 0;
	
	for(i = 0; i < ent_entity_def_list_cursor; i++)
	{
		if(ent_entity_defs[i].type == ENTITY_TYPE_INVALID)
		{
			continue;
		}
		
		def_record = (entity_def_record_t *)out;
		out += sizeof(entity_def_record_t);
		
		name_offset = 0;
		names = out;
		
		def_record->type = ent_entity_defs[i].type;
		def_record->scale = ent_entity_defs[i].original_scale;
		def_record->flags = ent_entity_defs[i].flags;
		
		def_record->reserved0 = 0;
		def_record->reserved1 = 0;
		def_record->reserved2 = 0;
		def_record->reserved3 = 0;
		def_record->reserved4 = 0;
		def_record->reserved5 = 0;
		def_record->reserved6 = 0;
		def_record->reserved7 = 0;
		
		name = ent_entity_defs[i].name
		
		for(j = 0; name[j]; j++)
		{
			def_record->def_name[j] = name[j];
		}
		
		for(; j < ENTITY_DEF_NAME_MAX_LEN; j++)
		{
			def_record->def_name[j] = '\0';
		}
		
		model = model_GetModelPointerIndex(ent_entity_defs[i].model_index);
		
		name = model->name;
		
		for(j = 0; name[j]; j++)
		{
			def_record->model_name[j] = name[j];
		}
		
		for(; j < MODEL_NAME_MAX_LEN; j++)
		{
			def_record->model_name[j] = '\0';
		}
		
		if(!(def_record->flags & ENTITY_GHOST))
		{
			name = ent_entity_defs[i].collider_def->name;
			
			for(j = 0; name[j]; j++)
			{
				def_record->collider_def_name[j] = name[j];
			}
			
			for(; j < COLLIDER_DEF_NAME_MAX_LEN; j++)
			{
				def_record->collider_def_name[j] = '\0';
			}
		}
	}
	
	#endif
}

void entity_DeserializeEntities(void **buffer)
{
	#if 0
	int i;
	int j;
	
	entity_section_header_t *header;
	entity_def_record_t *def_record;
	entity_record_t *ent_record;
	
	collider_def_t *collider_def;
	int model_index;
	int def_index;
	
	char *in;
	
	in = *buffer;
	
	header = (entity_section_header_t *)in;
	in += sizeof(entity_section_header_t );
	
	for(i = 0; i < header->entity_def_count; i++)
	{
		def_record = (entity_def_record_t *)in;
		in += sizeof(entity_def_record_t);
		
		if(!(def_record->flags & ENTITY_GHOST))
		{
			collider_def = physics_GetColliderDefPointer(def_record->collider_def_name);
		}
		else
		{
			collider_def = NULL;
		}
		
		model_index = model_GetModel(def_record->model_name);
		
		if(model_index < 0)
		{
			printf("entity_DeserializeEntities: couldn't find model [%s] for entity def [%s]\n", def_record->model_name, def_record->def_name);
			continue;
		}
		
		entity_CreateEntityDef(def_record->def_name, def_record->type, model_index, collider_def);
	}
	
	
	
	for(i = 0; i < header->entity_count; i++)
	{
		ent_record = (entity_record_t *)in;
		in += sizeof(entity_record_t);
		
		def_index = entity_GetEntityDef(ent_record->entity_def_name);
		
		if(def_index < 0)
		{
			printf("entity_DeserializeEntities: couldn't find entity def [%s] for entity [%s]\n", ent_record->entity_def_name, ent_record->entity_name);
			continue;
		}
		
		entity_CreateEntity(ent_record->entity_name, ent_record->position, ent_record->scale, &ent_record->orientation, def_index);
	}
	
	#endif
	
}

#ifdef __cplusplus
}
#endif












