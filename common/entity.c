#include "entity.h"
#include "ent_serialization.h"
#include "ent_script.h"
#include "r_main.h"
#include "r_view.h"

#include "stack_list.h"
#include "list.h"
#include "id_cache.h"

#include "c_memory.h"
#include "log.h"
#include "physics.h"
#include "script.h"
#include "navigation.h"

#include "engine.h"

#include "world.h"

#include "script.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "matrix.h"


/* from world.c */
extern int w_world_leaves_count;
extern struct bsp_pnode_t *w_world_nodes;
extern struct bsp_dleaf_t *w_world_leaves;
extern bsp_entities_t *w_leaf_entities;
extern int w_visible_leaves_count;
extern struct bsp_dleaf_t **w_visible_leaves;


struct id_cache_t ent_id_cache;

/* from model.c */
extern struct model_t *mdl_models;


extern int r_frame;

unsigned int ent_frame;

//struct entity_aabb_t *ent_aabbs = NULL;


//struct entity_def_t *ent_entity_defs = NULL;

//int ent_visible_entities_count = 0;
//int ent_visible_entities_indexes[MAX_VISIBLE_ENTITIES];



/*
===============================================================
===============================================================
===============================================================
*/

static int COMPONENT_SIZES[COMPONENT_TYPE_LAST] = {0};
struct component_fields_t COMPONENT_FIELDS[COMPONENT_TYPE_LAST] = {(struct component_fields_t){NULL, COMPONENT_TYPE_NONE, -1}} ;
//char *COMPONENT_FIELDS[COMPONENT_TYPE_LAST][16];

#define DECLARE_COMPONENT_SIZE(type, size) COMPONENT_SIZES[type]=size

#define COMPONENT_FIELD(name, script_type, offset) (struct component_field_t){name, script_type, offset}
#define COMPONENT_FIELD_OFFSET(type, field)(int)(&((type *)0)->field)
#define DECLARE_COMPONENT_FIELDS(component_type, ...)COMPONENT_FIELDS[component_type]=(struct component_fields_t)__VA_ARGS__

struct stack_list_t ent_components[2][COMPONENT_TYPE_LAST];
struct stack_list_t ent_entities[2];
struct stack_list_t ent_entity_def_source_files;
struct stack_list_t ent_entity_aabbs;
struct stack_list_t ent_world_transforms;
struct list_t ent_top_transforms;

struct list_t ent_entity_contacts;
struct list_t ent_invalid_entities[2];
struct list_t ent_valid_entity_defs;

struct stack_list_t ent_triggers;


void (*dispose_component_callback[COMPONENT_TYPE_LAST])() = {NULL};



static int ENTITY_PROP_TYPES_SIZES[ENTITY_PROP_TYPE_LAST] = {0};
#define DECLARE_PROP_TYPE_SIZE(type, size) ENTITY_PROP_TYPES_SIZES[type]=size


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
		ent_entities[i] = stack_list_create(sizeof(struct entity_t), 512, entity_EntityListDisposeCallback);
	}

	ent_entity_def_source_files = stack_list_create(sizeof(struct entity_source_file_t), 512, NULL);

	ent_entity_aabbs = stack_list_create(sizeof(struct entity_aabb_t), 512, NULL);
	ent_world_transforms = stack_list_create(sizeof(struct entity_transform_t), 512, NULL);
	ent_top_transforms = list_create(sizeof(struct component_handle_t), 512, NULL);

	ent_frame = 0;

	/*
	===============================================================
	===============================================================
	===============================================================
	*/

	ent_invalid_entities[0] = list_create(sizeof(struct entity_handle_t ), 512, NULL);
	ent_invalid_entities[1] = list_create(sizeof(struct entity_handle_t ), 512, NULL);
	ent_valid_entity_defs = list_create(sizeof(struct entity_handle_t), 512, NULL);
    ent_entity_contacts = list_create(sizeof(struct entity_contact_t), 65536, NULL);

    ent_id_cache = idcache_Create();



	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_TRANSFORM, sizeof(struct transform_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_PHYSICS, sizeof(struct physics_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_MODEL, sizeof(struct model_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_LIGHT, sizeof(struct light_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_SCRIPT, sizeof(struct script_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_CAMERA, sizeof(struct camera_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_PARTICLE_SYSTEM, sizeof(struct particle_system_component_t));
	DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_LIFE, sizeof(struct life_component_t));
    DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_NAVIGATION, sizeof(struct navigation_component_t));
    DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_SKELETON, sizeof(struct skeleton_component_t));
    DECLARE_COMPONENT_SIZE(COMPONENT_TYPE_BONE, sizeof(struct bone_component_t));

	DECLARE_COMPONENT_FIELDS(COMPONENT_TYPE_TRANSFORM, {
															COMPONENT_FIELD("orientation", SCRIPT_VAR_TYPE_MAT3T, COMPONENT_FIELD_OFFSET(struct transform_component_t, orientation)),
															COMPONENT_FIELD("scale", SCRIPT_VAR_TYPE_VEC3T, COMPONENT_FIELD_OFFSET(struct transform_component_t, scale)),
															COMPONENT_FIELD("position", SCRIPT_VAR_TYPE_VEC3T, COMPONENT_FIELD_OFFSET(struct transform_component_t, position)),
															COMPONENT_FIELD(NULL, SCRIPT_VAR_TYPE_NONE, 0)
														});




	DECLARE_PROP_TYPE_SIZE(ENTITY_PROP_TYPE_INT, sizeof(int));
	DECLARE_PROP_TYPE_SIZE(ENTITY_PROP_TYPE_FLOAT, sizeof(float));
	DECLARE_PROP_TYPE_SIZE(ENTITY_PROP_TYPE_VEC2, sizeof(vec2_t));
	DECLARE_PROP_TYPE_SIZE(ENTITY_PROP_TYPE_VEC3, sizeof(vec3_t));
	DECLARE_PROP_TYPE_SIZE(ENTITY_PROP_TYPE_VEC4, sizeof(vec4_t));
	DECLARE_PROP_TYPE_SIZE(ENTITY_PROP_TYPE_MAT2, sizeof(mat2_t));
	DECLARE_PROP_TYPE_SIZE(ENTITY_PROP_TYPE_MAT3, sizeof(mat3_t));
	DECLARE_PROP_TYPE_SIZE(ENTITY_PROP_TYPE_MAT4, sizeof(mat4_t));




	dispose_component_callback[COMPONENT_TYPE_TRANSFORM] = entity_TransformComponentDisposeCallback;

	for(def_list = 0; def_list < 2; def_list++)
	{
		for(component_type = COMPONENT_TYPE_TRANSFORM; component_type < COMPONENT_TYPE_LAST; component_type++)
		{
			ent_components[def_list][component_type] = stack_list_create(COMPONENT_SIZES[component_type], 512, dispose_component_callback[component_type]);
		}
	}

	id_rot = mat3_t_id();

	ent_triggers = stack_list_create(sizeof(struct trigger_t), 128, NULL);


    script_RegisterEnum("COMPONENT_TYPES");
    script_RegisterEnumValue("COMPONENT_TYPES", "COMPONENT_TYPE_TRANSFORM", COMPONENT_TYPE_TRANSFORM);
    script_RegisterEnumValue("COMPONENT_TYPES", "COMPONENT_TYPE_PHYSICS", COMPONENT_TYPE_PHYSICS);
    script_RegisterEnumValue("COMPONENT_TYPES", "COMPONENT_TYPE_MODEL", COMPONENT_TYPE_MODEL);
    script_RegisterEnumValue("COMPONENT_TYPES", "COMPONENT_TYPE_LIGHT", COMPONENT_TYPE_LIGHT);
    script_RegisterEnumValue("COMPONENT_TYPES", "COMPONENT_TYPE_CAMERA", COMPONENT_TYPE_CAMERA);
    script_RegisterEnumValue("COMPONENT_TYPES", "COMPONENT_TYPE_LIFE", COMPONENT_TYPE_LIFE);
    script_RegisterEnumValue("COMPONENT_TYPES", "COMPONENT_TYPE_NAVIGATION", COMPONENT_TYPE_NAVIGATION);
    script_RegisterEnumValue("COMPONENT_TYPES", "COMPONENT_TYPE_SKELETON", COMPONENT_TYPE_SKELETON);
    script_RegisterEnumValue("COMPONENT_TYPES", "COMPONENT_TYPE_BONE", COMPONENT_TYPE_BONE);

    script_RegisterObjectType("entity_handle_t", sizeof(struct entity_handle_t), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
    script_RegisterObjectType("component_handle_t", sizeof(struct component_handle_t), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
    script_RegisterObjectType("collider_handle_t", sizeof(struct collider_handle_t), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
    script_RegisterObjectType("entity_contact_t", sizeof(struct entity_contact_t), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
    script_RegisterObjectType("skeleton_handle_t", sizeof(struct skeleton_handle_t), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);

    script_RegisterObjectProperty("entity_contact_t", "entity_handle_t entity", (int)(&((struct entity_contact_t *)0)->entity));
    script_RegisterObjectProperty("entity_contact_t", "vec3_t position", (int)(&((struct entity_contact_t *)0)->position));


    script_RegisterGlobalFunction("void entity_Jump(float jump_force)", entity_ScriptJump);
    script_RegisterGlobalFunction("void entity_Move(vec3_t &in direction)", entity_ScriptMove);
    script_RegisterGlobalFunction("void entity_FindPath(vec3_t &in to)", entity_ScriptFindPath);
    script_RegisterGlobalFunction("int entity_GetWaypointDirection(vec3_t &out direction)", entity_ScriptGetWaypointDirection);


    script_RegisterGlobalFunction("vec3_t &entity_GetPosition(int local)", entity_ScriptGetPosition);
    script_RegisterGlobalFunction("vec3_t &entity_GetEntityPosition(entity_handle_t entity, int local)", entity_ScriptGetEntityPosition);


    script_RegisterGlobalFunction("mat3_t &entity_GetOrientation(int local)", entity_ScriptGetOrientation);
    script_RegisterGlobalFunction("mat3_t &entity_GetEntityOrientation(entity_handle_t entity, int local)", entity_ScriptGetEntityOrientation);


    script_RegisterGlobalFunction("void entity_Translate(vec3_t &in direction)", entity_ScriptTranslate);
    script_RegisterGlobalFunction("void entity_TranslateEntity(entity_handle_t entity, vec3_t &in direction)", entity_ScriptTranslateEntity);


    script_RegisterGlobalFunction("void entity_Rotate(vec3_t &in axis, float angle, int set)", entity_ScriptRotate);
    script_RegisterGlobalFunction("void entity_RotateEntity(entity_handle_t entity, vec3_t &in ais, float angle, int set)", entity_ScriptRotateEntity);


    script_RegisterGlobalFunction("vec3_t &entity_GetEntityForwardVector(entity_handle_t entity, int local)", entity_ScriptGetEntityForwardVector);
    script_RegisterGlobalFunction("vec3_t &entity_GetEntityRightVector(entity_handle_t entity, int local)", entity_ScriptGetEntityRightVector);

    script_RegisterGlobalFunction("void entity_SetEntityVelocity(entity_handle_t entity, vec3_t &in velocity)", entity_ScriptSetEntityVelocity);


    script_RegisterGlobalFunction("int entity_GetLife()", entity_ScriptGetLife);


    script_RegisterGlobalFunction("entity_handle_t entity_GetCurrentEntity()", entity_ScriptGetCurrentEntity);


    script_RegisterGlobalFunction("component_handle_t entity_GetComponent(int component_index)", entity_ScriptGetComponent);
    script_RegisterGlobalFunction("component_handle_t entity_GetEntityComponent(entity_handle_t entity, int component_index)", entity_ScriptGetEntityComponent);
    script_RegisterGlobalFunction("entity_handle_t entity_GetEntity(string &in name, int get_def)", entity_ScriptGetEntity);
    script_RegisterGlobalFunction("entity_handle_t entity_GetEntityDef(string &in name)", entity_ScriptGetEntityDef);


    script_RegisterGlobalFunction("entity_handle_t entity_GetChildEntity(string &in entity)", entity_ScriptGetChildEntity);
    script_RegisterGlobalFunction("entity_handle_t entity_GetEntityChildEntity(entity_handle_t entity, string &in name)", entity_ScriptGetEntityChildEntity);


    script_RegisterGlobalFunction("entity_handle_t entity_SpawnEntity(mat3_t &in orientation, vec3_t &in position, vec3_t &in scale, entity_handle_t def, string &in name)", entity_ScriptSpawnEntity);


    script_RegisterGlobalFunction("void entity_Die()", entity_ScriptDie);
    script_RegisterGlobalFunction("void entity_DIEYOUMOTHERFUCKER()", entity_ScriptDie);


    script_RegisterGlobalFunction("void entity_SetCameraAsActive(component_handle_t camera)", entity_ScriptSetCameraAsActive);


    script_RegisterGlobalFunction("void entity_SetComponentValue(component_handle_t component, string &in field_name, ? &in value)", entity_ScriptSetComponentValue);
    script_RegisterGlobalFunction("void entity_GetComponentValue(component_handle_t component, string &in field_name, ? &out value)", entity_ScriptGetComponentValue);
    script_RegisterGlobalFunction("void entity_SetComponentValue3f(component_handle_t component, string &in field_name, vec3_t &in value)", entity_ScriptSetComponentValue3f);
    script_RegisterGlobalFunction("void entity_GetComponentValue3f(component_handle_t component, string &in field_name, vec3_t &out value)", entity_ScriptGetComponentValue3f);
    script_RegisterGlobalFunction("void entity_SetComponentValue33f(component_handle_t component, string &in field_name, mat3_t &in value)", entity_ScriptSetComponentValue33f);


    script_RegisterGlobalFunction("void entity_AddEntityProp(entity_handle_t entity, string &in name, ? &in type)", entity_ScriptAddEntityProp);
    script_RegisterGlobalFunction("void entity_AddEntityProp1i(entity_handle_t entity, string &in name)", entity_ScriptAddEntityProp1i);
    script_RegisterGlobalFunction("void entity_AddEntityProp1f(entity_handle_t entity, string &in name)", entity_ScriptAddEntityProp1f);
    script_RegisterGlobalFunction("void entity_AddEntityProp3f(entity_handle_t entity, string &in name)", entity_ScriptAddEntityProp3f);
    script_RegisterGlobalFunction("void entity_RemoveEntityProp(entity_handle_t entity, string &in name)", entity_ScriptRemoveEntityProp);


    script_RegisterGlobalFunction("void entity_SetEntityProp1i(entity_handle_t entity, string &in name, int value)", entity_ScriptSetEntityPropValue1i);
    script_RegisterGlobalFunction("int entity_GetEntityProp1i(entity_handle_t entity, string &in name)", entity_ScriptGetEntityPropValue1i);
    script_RegisterGlobalFunction("int entity_IncEntityProp1i(entity_handle_t entity, string &in name)", entity_ScriptIncEntityPropValue1i);
    script_RegisterGlobalFunction("int entity_DecEntityProp1i(entity_handle_t entity, string &in name)", entity_ScriptDecEntityPropValue1i);
    script_RegisterGlobalFunction("void entity_SetEntityProp3f(entity_handle_t entity, string &in name, vec3_t &in value)", entity_ScriptSetEntityPropValue3f);
    script_RegisterGlobalFunction("vec3_t &entity_GetEntityProp3f(entity_handle_t entity, string &in name)", entity_ScriptGetEntityPropValue3f);


    script_RegisterGlobalFunction("void entity_SetEntityProp(entity_handle_t entity, string &in name, ? &in value)", entity_ScriptSetEntityPropValue);
    script_RegisterGlobalFunction("void entity_GetEntityProp(entity_handle_t entity, string &in name, ? &out value)", entity_ScriptGetEntityPropValue);


    script_RegisterGlobalFunction("int entity_EntityHasProp(entity_handle_t entity, string &in name)", entity_ScriptEntityHasProp);


    script_RegisterGlobalFunction("int entity_IsEntityValid(entity_handle_t entity)", entity_ScriptIsEntityValid);
    script_RegisterGlobalFunction("int entity_LineOfSightToEntity(entity_handle_t entity)", entity_ScriptLineOfSightToEntity);


    script_RegisterGlobalFunction("int entity_GetTrigger(string &in trigger)", entity_ScriptGetTrigger);
    script_RegisterGlobalFunction("void entity_SetTriggerPosition(int trigger_index, vec3_t &in position)", entity_ScriptSetTriggerPosition);
    script_RegisterGlobalFunction("int entity_IsTriggered(int trigger_index)", entity_ScriptIsTriggered);


    script_RegisterGlobalFunction("array<entity_handle_t> @entity_GetEntities()", entity_ScriptGetEntities);




	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "%s: subsystem initialized properly!", __func__);

	return 1;
}

void entity_Finish()
{
	int i;
	int j;

	stack_list_destroy(&ent_entities[0]);
	stack_list_destroy(&ent_entities[1]);
	stack_list_destroy(&ent_entity_def_source_files);
	stack_list_destroy(&ent_triggers);

	stack_list_destroy(&ent_entity_aabbs);
	stack_list_destroy(&ent_world_transforms);
	list_destroy(&ent_top_transforms);
	list_destroy(&ent_entity_contacts);


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
    struct physics_component_t *physics_component;
    struct skeleton_component_t *skeleton_component;
    struct navigation_component_t *navigation_component;

	handle.def = 0;
	handle.type = COMPONENT_TYPE_NONE;
	handle.index = 0;

	//list = entity_ListForType(component_type, alloc_for_def);
	alloc_for_def = alloc_for_def && 1;

	list = &ent_components[alloc_for_def][component_type];

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
				transform->bone.type = COMPONENT_TYPE_NONE;
				transform->children_count = 0;

				/* if this transform isn't getting alloc'd for
				an entity def, add it to the top list so it
				gets treated as the top of a hierarchy... */
				if(!alloc_for_def)
				{
					entity_AddTransformToTopList(handle);   /* should this be here? */
					stack_list_add(&ent_world_transforms, NULL);
					stack_list_add(&ent_entity_aabbs, NULL);
				}
			break;

			case COMPONENT_TYPE_SCRIPT:
				script = (struct script_component_t *)component;
				script->script = NULL;
			break;

			case COMPONENT_TYPE_PHYSICS:
                physics_component = (struct physics_component_t *)component;
                physics_component->max_entity_contact_count = 32;
                physics_component->entity_contact_count = 0;
            break;

            case COMPONENT_TYPE_SKELETON:
                skeleton_component = (struct skeleton_component_t *)component;
                skeleton_component->bone_transforms = list_create(sizeof(struct bone_transform_t), 1, NULL);
            break;

            /*case COMPONENT_TYPE_NAVIGATION:
                navigation_component = (struct navigation_component_t *)component;

                if(!navigation_component->route.elements)
                {
                    navigation_component->route = list_create(sizeof(struct waypoint_t *), 128, NULL);
                }

            break;*/
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
	struct skeleton_component_t *skeleton_component;
	struct bone_transform_t *bone_transform;
	int i;
	int c;

	//list = entity_ListForType(component.type, component.def);
	list = &ent_components[component.def][component.type];

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

				case COMPONENT_TYPE_SKELETON:
                    skeleton_component = (struct skeleton_component_t *)component_ptr;

                    c = skeleton_component->bone_transforms.element_count;
                    bone_transform = (struct bone_transform_t *)skeleton_component->bone_transforms.elements;

                    for(i = 0; i < c; i++)
                    {
                        entity_DeallocComponent(bone_transform[i].transform);
                    }

                    list_destroy(&skeleton_component->bone_transforms);
                break;
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
	//transform_ptr->depth_index = 0;
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
		printf("entity_ParentTransformComponent: cannot parent transform with itself\n");
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
		transforms = memory_Malloc(sizeof(struct component_handle_t) * (parent->max_children + 4));
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
	else
	{
        child->flags |= COMPONENT_FLAG_ENTITY_DEF_REF;
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

void entity_ParentEntityToEntityTransform(struct component_handle_t parent_transform, struct entity_handle_t child)
{
	struct entity_t *child_entity;
	struct entity_t *parent_entity;

	struct component_handle_t child_transform;
	struct transform_component_t *transform_component;
	struct transform_component_t *child_transform_component;

	if(parent_transform.def != child.def)
	{
		printf("entity_ParentEntityToTransform: can't parent entity def to non def\n");
		return;
	}

	child_entity = entity_GetEntityPointerHandle(child);

	if(parent_transform.def)
	{

	    /* if we're parenting two transform components  */
		/* if this is a ref, we alloc a new transform component and make it point
		to this def, thus configuring a entity def reference... */
		child_transform = entity_AllocComponent(COMPONENT_TYPE_TRANSFORM, parent_transform.def);
		transform_component = entity_GetComponentPointer(child_transform);
		transform_component->base.entity = child;
		child_entity->ref_count++;


		if(!transform_component->instance_name)
		{
			transform_component->instance_name = memory_Malloc(ENTITY_NAME_MAX_LEN);
		}

		strcpy(transform_component->instance_name, child_entity->name);

		child_transform_component = entity_GetComponentPointer(child_entity->components[COMPONENT_TYPE_TRANSFORM]);

		transform_component->orientation = child_transform_component->orientation;
		transform_component->position = child_transform_component->position;
		transform_component->scale = child_transform_component->scale;
		//transform_component->flags |= COMPONENT_FLAG_ENTITY_DEF_REF;
	}
	else
	{
		child_transform = child_entity->components[COMPONENT_TYPE_TRANSFORM];
	}

	entity_ParentTransformComponent(parent_transform, child_transform);
}

void entity_UnpparentEntityFromEntityTransform(struct component_handle_t parent_transform, struct entity_handle_t child)
{
	struct entity_t *parent_entity;
	struct entity_t *child_entity;

	int i;

	struct transform_component_t *parent_transform_ptr;
	struct transform_component_t *child_transform_ptr;
	struct transform_component_t *child_child_transform_ptr;
	struct transform_component_t *other_transform;

	struct component_handle_t child_transform;
	struct component_handle_t child_child_transform;

	if(parent_transform.def != child.def)
	{
		printf("entity_UnpparentEntity: can't unapparent entity def to non def\n");
		return;
	}

	child_entity = entity_GetEntityPointerHandle(child);

	if(parent_transform.def)
	{
		parent_transform_ptr = entity_GetComponentPointer(parent_transform);

		for(i = 0; i < parent_transform_ptr->children_count; i++)
		{
		    /* search the parent transform child list for the child transform we want
		    unpparented... */
			other_transform = entity_GetComponentPointer(parent_transform_ptr->child_transforms[i]);

			if(other_transform->base.entity.entity_index == child.entity_index)
			{
				child_transform = parent_transform_ptr->child_transforms[i];
				child_entity->ref_count--;
				break;
			}
		}

		if(i == parent_transform_ptr->children_count)
		{
		    /* didn't find it... */
			return;
		}

        child_transform_ptr = entity_GetComponentPointer(child_transform);
        /* this transform component can have stuff nestled to it... */
        for(i = 0; i < child_transform_ptr->children_count; i++)
        {
            child_child_transform = child_transform_ptr->child_transforms[i];
            child_child_transform_ptr = entity_GetComponentPointer(child_child_transform);

            if(child_child_transform_ptr->base.entity.entity_index != INVALID_ENTITY_INDEX)
            {
                entity_UnpparentEntityFromEntityTransform(child_transform, child_child_transform_ptr->base.entity);

                if(child_child_transform_ptr->flags & COMPONENT_FLAG_ENTITY_DEF_REF)
                {
                    entity_DeallocComponent(child_child_transform);
                }
            }
        }
	}
	else
	{
		child_transform = child_entity->components[COMPONENT_TYPE_TRANSFORM];
	}

	entity_UnparentTransformComponent(parent_transform, child_transform);
}


void entity_ParentEntity(struct entity_handle_t parent, struct entity_handle_t child)
{
	struct entity_t *parent_entity;
	struct entity_t *child_entity;

	struct component_handle_t child_transform;
	struct component_handle_t parent_transform;
	struct transform_component_t *transform_component;
	struct transform_component_t *child_transform_component;

	if(parent.def != child.def)
	{
		printf("entity_ParentEntity: can't parent entity def to non def\n");
		return;
	}

	parent_entity = entity_GetEntityPointerHandle(parent);
	child_entity = entity_GetEntityPointerHandle(child);

	entity_ParentEntityToEntityTransform(parent_entity->components[COMPONENT_TYPE_TRANSFORM], child);
}


void entity_UnparentEntity(struct entity_handle_t parent, struct entity_handle_t child)
{
	struct entity_t *parent_entity;
	struct entity_t *child_entity;

	int i;

	struct transform_component_t *parent_transform;
	struct transform_component_t *other_transform;

	struct component_handle_t child_transform;

	if(parent.def != child.def)
	{
		printf("entity_UnparentEntity: can't unparent entity def to non def\n");
		return;
	}

	parent_entity = entity_GetEntityPointerHandle(parent);
	child_entity = entity_GetEntityPointerHandle(child);

	entity_UnpparentEntityFromEntityTransform(parent_entity->components[COMPONENT_TYPE_TRANSFORM], child);
}

void entity_ParentEntityToBone(struct component_handle_t skeleton, int bone_index, struct entity_handle_t entity)
{
    struct skeleton_component_t *skeleton_ptr;
    struct entity_t *entity_ptr;
    struct bone_transform_t *bone_transform;
    struct transform_component_t *transform_component;
    int i;
    int c;


    if(bone_index >= 0)
    {
        skeleton_ptr = entity_GetComponentPointer(skeleton);
        entity_ptr = entity_GetEntityPointerHandle(entity);

        if(skeleton_ptr && entity_ptr)
        {

            bone_transform = (struct bone_transform_t *)skeleton_ptr->bone_transforms.elements;

            c = skeleton_ptr->bone_transforms.element_count;

            for(i = 0; i < c; i++)
            {
                if(bone_transform[i].bone_index == bone_index)
                {
                    bone_transform = bone_transform + i;
                    break;
                }
            }

            if(i == c)
            {
                i = list_add(&skeleton_ptr->bone_transforms, NULL);
                bone_transform = (struct bone_transform_t *)list_get(&skeleton_ptr->bone_transforms, i);
                bone_transform->bone_index = bone_index;

                if(!entity.def)
                {
                    bone_transform->transform = entity_AllocComponent(COMPONENT_TYPE_TRANSFORM, entity.def);

                    /* when a transform gets alloc'd, it gets set as the top
                    of a hierarchy. To avoid the engine uselessly updating this
                    transform, it gets removed here... */
                    entity_RemoveTransformFromTopList(bone_transform->transform);
                }
            }

            transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
            transform_component->bone = bone_transform->transform;
        }
    }

}

void entity_UnparentEntityToBone(struct component_handle_t skeleton_component, struct entity_handle_t entity)
{
    struct transform_component_t *transform_component;
    struct entity_t *entity_ptr;


    entity_ptr = entity_GetEntityPointerHandle(entity);

    if(entity_ptr)
    {
        transform_component = (struct transform_component_t *)entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);

        //transform_component->
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

	handle = entity_CreateEntity(name, 1);
	entity_AddComponent(handle, COMPONENT_TYPE_TRANSFORM);
	entity_SetTransform(handle, NULL, vec3_t_c(0.0, 0.0, 0.0), vec3_t_c(1.0, 1.0, 1.0), 0);

	return handle;
}

struct component_handle_t entity_AddComponent(struct entity_handle_t entity, int component_type)
{
	struct entity_t *entity_ptr;
	struct entity_t *entity_def_ptr;
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
	struct navigation_component_t *navigation_component;

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
				if(entity.def)
				{
					transform_component = (struct transform_component_t *)component_ptr;

					if(!transform_component->instance_name)
					{
						transform_component->instance_name = memory_Malloc(ENTITY_NAME_MAX_LEN);
					}

					strcpy(transform_component->instance_name, entity_ptr->name);
					entity_ptr->ref_count++;
				}
			break;

			case COMPONENT_TYPE_CAMERA:

			break;

			case COMPONENT_TYPE_NAVIGATION:
                navigation_component = (struct navigation_component_t *)component_ptr;

                if(!navigation_component->route.elements)
                {
                    navigation_component->route = list_create(sizeof(struct waypoint_t *), 512, NULL);
                }

                navigation_component->route.element_count = 0;
                navigation_component->current_waypoint = 0;

            break;
		}
	}


	/* propagate the flag upwards, so this now modified
	entity gets serialized properly... */
	if(!entity.def)
	{
		if(!(entity_ptr->flags & ENTITY_FLAG_NOT_INITIALIZED))
		{
			while(entity_ptr)
			{
				entity_ptr->flags |= ENTITY_FLAG_MODIFIED;
				entity_ptr = entity_GetEntityParentPointerHandle(entity);
			}
		}

	}

	return component;
}

void entity_RemoveComponent(struct entity_handle_t entity, int component_type)
{
	struct entity_t *entity_ptr;
	struct component_t *component;
	struct camera_component_t *camera_component;

	entity_ptr = entity_GetEntityPointerHandle(entity);

	if(entity_ptr)
	{
		if(component_type >= COMPONENT_TYPE_TRANSFORM && component_type < COMPONENT_TYPE_LAST)
		{
			if(entity_ptr->components[component_type].type != COMPONENT_TYPE_NONE)
			{
				component = entity_GetComponentPointer(entity_ptr->components[component_type]);

				switch(component_type)
				{
					case COMPONENT_TYPE_TRANSFORM:
						printf("entity_RemoveComponent: cannot remove component!\n");
					break;

					case COMPONENT_TYPE_CAMERA:
						camera_component = (struct camera_component_t *)component;

						if(!entity.def)
						{
							//camera_DestroyCamera(camera_component->camera);
							renderer_DestroyViewDef(camera_component->view);
						}
					break;
				}

				entity_DeallocComponent(entity_ptr->components[component_type]);
				entity_ptr->components[component_type].type = COMPONENT_TYPE_NONE;


				if(!entity.def)
				{
					entity_ptr->flags |= ENTITY_FLAG_MODIFIED;
				}

			}
		}
	}
}

int entity_AddProp(struct entity_handle_t entity, char *name, int size)
{
	struct entity_prop_t *props;
	struct entity_prop_t *prop;
	struct entity_t *entity_ptr;

	//int size;

	int prop_index = -1;

	int i;

	entity_ptr = entity_GetEntityPointerHandle(entity);

	if(size <= 0)
	{
		return -1;
	}

	if(entity_ptr)
	{

		if(!entity.def)
		{
			entity_ptr->flags |= ENTITY_FLAG_MODIFIED;
		}

		if(!entity_ptr->props.elements)
		{
			entity_ptr->props = list_create(sizeof(struct entity_prop_t), 16, NULL);
		}


		props = (struct entity_prop_t *)entity_ptr->props.elements;

		for(prop_index = 0; prop_index < entity_ptr->props.element_count; prop_index++)
		{
			if(!strcmp(name, props[prop_index].name))
			{
				if(size == props[prop_index].size)
				{
					/* don't add a prop if the requested name and size
					is the same of an existing one... */
					return prop_index;
				}

				break;
			}
		}

		if(prop_index == entity_ptr->props.element_count)
		{
			list_add(&entity_ptr->props, NULL);

			prop = list_get(&entity_ptr->props, prop_index);

			prop->name = memory_Calloc(ENTITY_PROP_NAME_MAX_LEN, 1);

			for(i = 0; i < ENTITY_PROP_NAME_MAX_LEN - 1 && name[i]; i++)
			{
				prop->name[i] = name[i];
			}
		}
		else
		{
			prop = list_get(&entity_ptr->props, prop_index);
			memory_Free(prop->memory);
		}

		//prop->type = type;
		prop->size = size;
		prop->memory = memory_Malloc((prop->size + 3) & (~3));
	}

	return prop_index;
}

void entity_AddPropTyped(struct entity_handle_t entity, char *name, int type)
{
    struct entity_prop_t *prop;
    int prop_index;

    if(type >= ENTITY_PROP_TYPE_INT && type < ENTITY_PROP_TYPE_LAST)
    {
        prop_index = entity_AddProp(entity, name, ENTITY_PROP_TYPES_SIZES[type]);

        if(prop_index > -1)
        {
            prop = entity_GetPropPointerIndex(entity, prop_index);
            prop->type = type;
        }
    }
}

void entity_AddProp1i(struct entity_handle_t entity, char *name)
{
    entity_AddPropTyped(entity, name, ENTITY_PROP_TYPE_INT);
}

void entity_AddProp1f(struct entity_handle_t entity, char *name)
{
    entity_AddPropTyped(entity, name, ENTITY_PROP_TYPE_FLOAT);
}

void entity_AddProp2f(struct entity_handle_t entity, char *name)
{
	entity_AddPropTyped(entity, name, ENTITY_PROP_TYPE_VEC2);
}

void entity_AddProp3f(struct entity_handle_t entity, char *name)
{
	entity_AddPropTyped(entity, name, ENTITY_PROP_TYPE_VEC3);
}

void entity_AddProp4f(struct entity_handle_t entity, char *name)
{
	entity_AddPropTyped(entity, name, ENTITY_PROP_TYPE_VEC4);
}

void entity_AddProp22f(struct entity_handle_t entity, char *name)
{
	entity_AddPropTyped(entity, name, ENTITY_PROP_TYPE_MAT2);
}

void entity_AddProp33f(struct entity_handle_t entity, char *name)
{
	entity_AddPropTyped(entity, name, ENTITY_PROP_TYPE_MAT3);
}

void entity_AddProp44f(struct entity_handle_t entity, char *name)
{
	entity_AddPropTyped(entity, name, ENTITY_PROP_TYPE_MAT4);
}



void entity_RemoveProp(struct entity_handle_t entity, char *name)
{
	int i;
	struct entity_t *entity_ptr;
	struct entity_prop_t *props;


    entity_ptr = entity_GetEntityPointerHandle(entity);

    if(entity_ptr)
	{
		props = (struct entity_prop_t *)entity_ptr->props.elements;

		for(i = 0; i < entity_ptr->props.element_count; i++)
		{
			if(!strcmp(name,props[i].name))
			{
				memory_Free(props[i].memory);
				memory_Free(props[i].name);

                list_remove(&entity_ptr->props, i);

				return;
			}
		}
	}
}

void entity_PropValue(struct entity_handle_t entity, char *name, void *value, int set)
{
	struct entity_prop_t *prop;
	struct entity_t *entity_ptr;
	int i;

	entity_ptr = entity_GetEntityPointerHandle(entity);

	if(entity_ptr)
	{
		prop = entity_GetPropPointer(entity, name);

		if(prop)
		{
			if(set)
			{
				memcpy(prop->memory, value, prop->size);
			}
			else
			{
				memcpy(value, prop->memory, prop->size);
			}

			return;
		}
		else
		{
			printf("entity_PropValue: prop [%s] does not exist\n", name);
		}
	}
	else
	{
		printf("entity_PropValue: bad entity handle\n");
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

struct entity_prop_t *entity_GetPropPointer(struct entity_handle_t entity, char *name)
{
	int i;
	struct entity_t *entity_ptr;
	struct entity_prop_t *props;

	entity_ptr = entity_GetEntityPointerHandle(entity);

	if(entity_ptr)
	{
		props = (struct entity_prop_t *)entity_ptr->props.elements;

		for(i = 0; i < entity_ptr->props.element_count; i++)
		{
			if(!strcmp(name, props[i].name))

			{
				return props + i;
			}
		}
	}

	return NULL;
}

struct entity_prop_t *entity_GetPropPointerIndex(struct entity_handle_t entity, int prop_index)
{
    struct entity_t *entity_ptr;

    entity_ptr = entity_GetEntityPointerHandle(entity);

    if(entity_ptr)
    {
        if(prop_index >= 0 && prop_index < entity_ptr->props.element_count)
        {
            return (struct entity_prop_t *)entity_ptr->props.elements + prop_index;
        }
    }

    return NULL;
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

	if((!entity.def) && model)
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
	struct collider_t *collider_ptr;

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

	physics_component->collider = *(struct collider_handle_t *)collider;

	if(!entity.def)
    {
		collider_ptr = physics_GetColliderPointer(physics_component->collider);

		if(collider_ptr)
		{
			collider_ptr->entity_index = entity.entity_index;
		}
    }

	/*if(entity.def)
	{
		physics_component->collider.collider_def = collider;
	}
	else
	{
		physics_component->collider.collider_handle = *(struct collider_handle_t *)collider;
		collider_ptr = physics_GetColliderPointerHandle(physics_component->collider.collider_handle);

		if(collider_ptr)
		{
			collider_ptr->entity_index = entity.entity_index;
		}
	}*/

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
	struct physics_component_t *physics_component;
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

		physics_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);

		if(physics_component)
		{
			if(!entity.def)
			{
				physics_SetColliderOrientation(physics_component->collider, &transform_component->orientation);
				physics_SetColliderPosition(physics_component->collider, transform_component->position);
				physics_SetColliderScale(physics_component->collider, transform_component->scale);
				physics_SetColliderVelocity(physics_component->collider, vec3_t_c(0.0, 0.0, 0.0));
			}

		}

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

	//camera_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_CAMERA]);

	if(entity_ptr->components[COMPONENT_TYPE_CAMERA].type != COMPONENT_TYPE_NONE)
	{
		transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);

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
}

void entity_SetSkeleton(struct entity_handle_t entity, struct skeleton_handle_t skeleton)
{
    struct entity_t *entity_ptr;
    struct skeleton_component_t *skeleton_component;

    entity_ptr = entity_GetEntityPointerHandle(entity);

    if(entity_ptr)
    {
        skeleton_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_SKELETON]);

        if(!skeleton_component)
        {
            if(entity.def)
            {
                printf("entity_SetSkeleton: entity def [%s] doesn't have a valid skeleton component\n", entity_ptr->name);
            }
            else
            {
                printf("entity_SetSkeleton: entity [%s] doesn't have a valid skeleton component\n", entity_ptr->name);
            }

            return;
        }

        skeleton_component->skeleton = skeleton;
    }
}

/*void entity_SetCamera(struct entity_handle_t entity, camera_t *camera)
{
	struct entity_t *entity_ptr;
	struct camera_component_t *camera_component;

	entity_ptr = entity_GetEntityPointerHandle(entity);
	camera_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_CAMERA]);
	camera_component->camera = camera;
}*/

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
	entity_ptr->flags = ENTITY_FLAG_NOT_INITIALIZED;
	entity_ptr->leaf = NULL;
	//entity_ptr->prop_count = 0;
	entity_ptr->props.element_count = 0;
	entity_ptr->ref_count = 0;

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
			entity_ptr->name = memory_Malloc(ENTITY_NAME_MAX_LEN);
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


struct entity_handle_t entity_RecursiveSpawnEntity(mat3_t *orientation, vec3_t position, vec3_t scale, struct entity_handle_t entity_def, struct component_handle_t referencing_transform, char *name)
{
	struct entity_t *entity_ptr;
	struct entity_t *entity_def_ptr;
	struct entity_t *rec_entity_def_ptr;
	struct entity_handle_t handle;
	struct entity_handle_t child_handle;

	struct model_component_t *model_component;
	struct model_component_t *def_model_component;
	struct transform_component_t *transform_component;
	struct transform_component_t *entity_transform_component;
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

	struct skeleton_component_t *def_skeleton_component;
	struct skeleton_component_t *skeleton_component;
	struct bone_transform_t *bone_transform;
	struct bone_transform_t *def_bone_transform;

	//struct physics_controller_component_t *physics_controller;
	//struct physics_controller_component_t *def_physics_controller;

	//struct controller_component_t *controller;
	//struct controller_component_t *def_controller;
	struct component_t *component_ptr;
	struct component_handle_t component_transform;
	struct component_handle_t component_handle;

	struct entity_aabb_t *aabb;
	struct model_t *model;
	int entity_index;
	int collider_index;
	struct collider_handle_t collider_handle;

	struct entity_prop_t *props;

	struct component_handle_t component;

	int i;
	int component_type;
	int j;

	static int level = -1;

	level++;

	entity_def_ptr = entity_GetEntityPointerHandle(entity_def);

	handle = entity_CreateEntity(idcache_AllocUniqueName(&ent_id_cache, entity_def_ptr->name), 0);

	entity_ptr = entity_GetEntityPointerHandle(handle);


	if(entity_ptr->components)
	{
		for(i = 0; i < COMPONENT_TYPE_LAST; i++)
		{
			entity_ptr->components[i].type = COMPONENT_TYPE_NONE;
		}
	}

	transform_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_TYPE_TRANSFORM]);

	if(!level)
    {
        scale.x *= transform_component->scale.x;
        scale.y *= transform_component->scale.y;
        scale.z *= transform_component->scale.z;
    }



	for(component_type = 0; component_type < COMPONENT_TYPE_LAST; component_type++)
	{
		if(entity_def_ptr->components[component_type].type != COMPONENT_TYPE_NONE)
		{
			component_handle = entity_AddComponent(handle, component_type);

			switch(component_type)
			{
				case COMPONENT_TYPE_TRANSFORM:
					entity_SetTransform(handle, orientation, position, scale, 1);

                    if(transform_component->bone.type != COMPONENT_TYPE_NONE)
                    {
                        entity_transform_component = entity_GetComponentPointer(component_handle);
                    }

				break;

				case COMPONENT_TYPE_MODEL:
					def_model_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_TYPE_MODEL]);
					entity_SetModel(handle, def_model_component->model_index);
				break;

				case COMPONENT_TYPE_PHYSICS:
					def_physics_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_TYPE_PHYSICS]);

					if(def_physics_component->collider.index != INVALID_COLLIDER_INDEX)
					{
						collider_handle = physics_CreateCollider(orientation, position, scale, def_physics_component->collider, 0);

						if(def_physics_component->flags & PHYSICS_COMPONENT_FLAG_STATIC)
						{
                            physics_SetColliderStatic(collider_handle, 1);
						}

						entity_SetCollider(handle, &collider_handle);
					}
				break;

				case COMPONENT_TYPE_SCRIPT:
					def_script_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_TYPE_SCRIPT]);
					entity_SetScript(handle, def_script_component->script);
				break;

				case COMPONENT_TYPE_CAMERA:
					camera_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_CAMERA]);
					/*def_camera_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_TYPE_CAMERA]);

					camera_transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
					def_camera_transform_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_TYPE_TRANSFORM]);

					camera_transform_component->orientation = def_camera_transform_component->orientation;
					camera_transform_component->position = def_camera_transform_component->position;
					camera_transform_component->scale = def_camera_transform_component->scale;*/
                    camera_component->view = renderer_CreateViewDef("view", vec3_t_c(0.0, 0.0, 0.0), NULL, 0.68, 1366.0, 768.0, 0.1, 500.0, 0);
					//camera_component->camera = camera_CreateCamera("camera", vec3_t_c(0.0, 0.0, 0.0), NULL, 0.68, 1366.0, 768.0, 0.1, 500.0, 0);
				break;

				case COMPONENT_TYPE_SKELETON:
                    def_skeleton_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_TYPE_SKELETON]);
                    skeleton_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_SKELETON]);

                    skeleton_component->skeleton = animation_SpawnSkeleton(def_skeleton_component->skeleton);

                    skeleton_component->bone_transforms = list_create(sizeof(struct bone_transform_t), def_skeleton_component->bone_transforms.element_count, NULL);

                    bone_transform = (struct bone_transform_t *)skeleton_component->bone_transforms.elements;
                    def_bone_transform = (struct bone_transform_t *)def_skeleton_component->bone_transforms.elements;
                    j = def_skeleton_component->bone_transforms.element_count;

                    for(i = 0; i < j; i++)
                    {
                        bone_transform[i].bone_index = def_bone_transform[i].bone_index;
                        bone_transform[i].transform = entity_AllocComponent(COMPONENT_TYPE_TRANSFORM, 0);
                        entity_RemoveTransformFromTopList(bone_transform[i].transform);
                    }



                break;
			}
		}
	}


	props = (struct entity_prop_t *)entity_def_ptr->props.elements;

	/* entities spanwed through a def will inherit all
	the props and its values... */
	for(i = 0; i < entity_def_ptr->props.element_count; i++)
	{
		entity_AddProp(handle, props[i].name, props[i].size);
		entity_SetProp(handle, props[i].name, props[i].memory);
	}

	entity_ptr->spawn_time = ent_frame;
	entity_ptr->flags = 0;
	entity_ptr->def = entity_def;

	transform_component = entity_GetComponentPointer(entity_def_ptr->components[COMPONENT_TYPE_TRANSFORM]);

	for(i = 0; i < transform_component->children_count; i++)
	{
	    /* go over the list of nestled transforms and spawn everything... */
		child_transform = entity_GetComponentPointer(transform_component->child_transforms[i]);

		if(child_transform->base.entity.entity_index != INVALID_ENTITY_INDEX)
		{
			rec_entity_def_ptr = entity_GetEntityPointerHandle(child_transform->base.entity);
			child_handle = entity_RecursiveSpawnEntity(&child_transform->orientation, child_transform->position, child_transform->scale, child_transform->base.entity, transform_component->child_transforms[i], NULL);
			entity_ParentEntity(handle, child_handle);
		}
	}




	transform_component = entity_GetComponentPointer(referencing_transform);

	if(transform_component)
	{
	    if(referencing_transform.index != entity_def_ptr->components[COMPONENT_TYPE_TRANSFORM].index)
        {
            /* if the referencing transform is not the same that this entity def has, it means
            this is just a reference to the entity def, and so we may have nestled entities
            in this reference. If that's the case, go over its list of nestled transforms and
            spawn anything that may need spawning... */
            for(i = 0; i < transform_component->children_count; i++)
            {
                child_transform = entity_GetComponentPointer(transform_component->child_transforms[i]);

                if(child_transform->base.entity.entity_index != INVALID_ENTITY_INDEX)
                {
                    rec_entity_def_ptr = entity_GetEntityPointerHandle(child_transform->base.entity);
                    child_handle = entity_RecursiveSpawnEntity(&child_transform->orientation, child_transform->position, child_transform->scale, child_transform->base.entity, transform_component->child_transforms[i], NULL);
                    entity_ParentEntity(handle, child_handle);
                }
            }
	    }
	}

	//entity_def_ptr = entity_GetEntityPointerHandle(entity_def);

	level--;

	return handle;
}

struct entity_handle_t entity_SpawnEntity(mat3_t *orientation, vec3_t position, vec3_t scale, struct entity_handle_t entity_def, char *name)
{
	if(entity_def.entity_index != INVALID_ENTITY_INDEX)
	{
		return entity_RecursiveSpawnEntity(orientation, position, scale, entity_def, (struct component_handle_t){COMPONENT_TYPE_NONE, 1, 0}, name);
	}

	return INVALID_ENTITY_HANDLE;
}

void entity_MarkForRemoval(struct entity_handle_t entity)
{
	struct entity_t *entity_ptr;
	struct component_handle_t component;
	struct entity_script_t *script;
	struct script_component_t *script_component;

	entity_ptr = entity_GetEntityPointerHandle(entity);

	if(entity_ptr)
	{
		entity_ptr->flags |= ENTITY_FLAG_MARKED_INVALID;
		list_add(&ent_invalid_entities[entity.def], &entity);
	}
}


void entity_RecursiveRemoveEntity(struct component_handle_t referencing_transform)
{
    struct entity_t *entity_ptr;
	struct entity_t *child_entity_ptr;
	struct component_handle_t component;

	struct camera_component_t *camera_component;
	struct physics_component_t *physics_component;
	struct script_component_t *script_component;
	struct transform_component_t *transform_component;
	struct transform_component_t *child_transform_component;
	struct component_t *component_ptr;

	int component_type;
	int i;

	transform_component = entity_GetComponentPointer(referencing_transform);

	entity_ptr = entity_GetEntityPointerHandle(transform_component->base.entity);

	if(entity_ptr)
	{
        if(!transform_component->base.entity.def)
        {
            /* if this is an entity instance... */
            if(entity_ptr->components[COMPONENT_TYPE_SCRIPT].type != COMPONENT_TYPE_NONE)
            {
                /* if it has a script component, execute its die function... */
                script_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_SCRIPT]);

                script_ExecuteScriptImediate(script_component->script, script_component);
            }
        }

        /* free the unique name allocated for this entity... */
        idcache_FreeUniqueName(&ent_id_cache, entity_ptr->name);

		for(i = 0; i < transform_component->children_count; i++)
		{
		    /* if this is a def, this will remove everything it has nestled to
		    its transform component. If this is a def ref, the same will happen,
		    but the original def remains untouched...  */
			child_transform_component = (struct transform_component_t *)entity_GetComponentPointer(transform_component->child_transforms[i]);

			if(child_transform_component->base.entity.entity_index != INVALID_ENTITY_INDEX)
			{
			    //if(!(child_transform_component->flags & COMPONENT_FLAG_ENTITY_DEF_REF))
                //{
                /* for entity defs only, make sure this isn't a reference to another
                def... */
                entity_RecursiveRemoveEntity(transform_component->child_transforms[i]);
                //}
			}
		}

        if((!transform_component->base.entity.def) ||
           (!(transform_component->flags & COMPONENT_FLAG_ENTITY_DEF_REF)))
        {

            /* if this is an instance, or if this is a entity def, not a reference to an
            entity def... */




            /* an entity def shouldn't be marked as invalid
            through a reference... */
            entity_ptr->flags = ENTITY_FLAG_INVALID;

            stack_list_remove(&ent_entities[transform_component->base.entity.def], transform_component->base.entity.entity_index);


            for(component_type = 0; component_type < COMPONENT_TYPE_LAST; component_type++)
            {
                component = entity_ptr->components[component_type];
                if(component.type != COMPONENT_TYPE_NONE)
                {
                    switch(component_type)
                    {
                        case COMPONENT_TYPE_CAMERA:
                            camera_component = entity_GetComponentPointer(component);
                            renderer_DestroyViewDef(camera_component->view);
                        break;

                        case COMPONENT_TYPE_PHYSICS:
                            physics_component = entity_GetComponentPointer(component);
                            physics_DestroyCollider(physics_component->collider);
                        break;
                    }
                    entity_DeallocComponent(component);
                }
            }
        }
	}
}

void entity_RemoveEntity(struct entity_handle_t entity)
{
	struct entity_t *entity_ptr;

    entity_ptr = entity_GetEntityPointerHandle(entity);

    if(entity_ptr)
    {
        entity_RecursiveRemoveEntity(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
    }


//	if(entity.def)
//	{
//		printf("entity_RemoveEntity: can't remove entity def\n");
//		return;
//	}

//	entity_ptr = entity_GetEntityPointerHandle(entity);
//
//	if(entity_ptr)
//	{
//
//        if(!entity.def)
//        {
//            if(entity_ptr->components[COMPONENT_TYPE_SCRIPT].type != COMPONENT_TYPE_NONE)
//            {
//                script_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_SCRIPT]);
//
//                script_ExecuteScriptImediate(script_component->script, script_component);
//            }
//        }
//
//        idcache_FreeUniqueName(&ent_id_cache, entity_ptr->name);
//
//		transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
//
//		for(i = 0; i < transform_component->children_count; i++)
//		{
//			child_transform_component = (struct transform_component_t *)entity_GetComponentPointer(transform_component->child_transforms[i]);
//
//			if(child_transform_component->base.entity.entity_index != INVALID_ENTITY_INDEX)
//			{
//			    if(!(child_transform_component->flags & COMPONENT_FLAG_ENTITY_DEF_REF))
//                {
//                    /* for entity defs only, make sure this isn't a reference to another
//                    def... */
//                    entity_RemoveEntity(child_transform_component->base.entity);
//                }
//			}
//		}
//
//		for(component_type = 0; component_type < COMPONENT_TYPE_LAST; component_type++)
//		{
//			component = entity_ptr->components[component_type];
//			if(component.type != COMPONENT_TYPE_NONE)
//			{
//				switch(component_type)
//				{
//					case COMPONENT_TYPE_CAMERA:
//						camera_component = entity_GetComponentPointer(component);
////						camera_DestroyCamera(camera_component->camera);
//                        renderer_DestroyViewDef(camera_component->view);
//			//			entity_DeallocComponent(camera_component->transform);
//					break;
//
//					case COMPONENT_TYPE_PHYSICS:
//						physics_component = entity_GetComponentPointer(component);
//						physics_DestroyCollider(physics_component->collider);
//					break;
//				}
//				entity_DeallocComponent(component);
//			}
//		}
//
//		entity_ptr->flags = ENTITY_FLAG_INVALID;
//
//		stack_list_remove(&ent_entities[entity.def], entity.entity_index);
//	}

}

void entity_RemoveAllEntities(int remove_defs)
{
	int i;
	int c;
	struct entity_handle_t handle;

	remove_defs = remove_defs && 1;

	c = ent_entities[remove_defs].element_count;

	for(i = 0; i < c; i++)
	{
        handle = ENTITY_HANDLE(i, remove_defs);

		if(entity_GetEntityPointerHandle(handle))
		{
		    if(remove_defs)
            {
                entity_MarkForRemoval(handle);
            }
            else
            {
                entity_RemoveEntity(handle);
            }
		}
	}
}


void entity_RemoveMarkedEntities(int remove_defs)
{
	struct entity_t *entity;
	struct entity_t *entity_defs;
	struct entity_handle_t handle;
	struct entity_handle_t *invalid_entities;

	int i;
	int c;

	remove_defs = remove_defs && 1;

	if(remove_defs)
    {
        return;
    }

	invalid_entities = (struct entity_handle_t *)ent_invalid_entities[remove_defs].elements;
	c = ent_invalid_entities[remove_defs].element_count;

	for(i = 0; i < c; i++)
    {
        handle = invalid_entities[i];
        entity = entity_GetEntityPointerHandle(handle);

        if(entity)
        {
            if(entity->flags & ENTITY_FLAG_MARKED_INVALID)
            {
                entity_RemoveEntity(handle);
            }
        }
    }

    ent_invalid_entities[remove_defs].element_count = 0;
}

struct entity_t *entity_GetEntityPointer(char *name, int get_def)
{
	int i;
	int c;
	int entity_index;
	struct entity_t *entities;
	struct entity_t *entity;
	//struct transform_component_t *transform;
//	struct component_handle_t *transform;

	static char ent_scoped_name[1024];

	get_def = get_def && 1;

	entities = (struct entity_t *)ent_entities[get_def].elements;
	c = ent_entities[get_def].element_count;

	for(i = 0; i < c; i++)
	{
		entity = entities + i;

        if(!strcmp(entity->name, name))
		{
			return entity;
		}
	}


	/*if(get_def)
	{
		c = ent_entities[get_def].element_count;

		for(i = 0; i < c; i++)
		{

		}
	}
	else
	{
		c = ent_top_transforms.element_count;
		transform = (struct component_handle_t *)ent_top_transforms.elements;

		for(i = 0; i < c; i++)
		{
			transform = entity_GetComponentPointer(transforms[i]);

			entity_index
		}
	}*/

	return NULL;
}

#if 0
__attribute__((always_inline)) inline struct entity_t *entity_GetEntityPointerHandle(struct entity_handle_t entity)
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

#endif


#if 0
__attribute__((always_inline)) inline void *entity_GetComponentPointer(struct component_handle_t component)
{
	struct stack_list_t *list;
	if(component.type != COMPONENT_TYPE_NONE)
	{
		list = &ent_components[component.def][component.type];
		return (void *)((char *)list->elements + list->element_size * component.index);
	}
	return NULL;
}

#endif

#if 0

__attribute__((always_inline)) inline void *entity_GetComponentPointerIndex(int index, int type, int def)
{
	struct component_handle_t handle;

	def = def && 1;

	handle.def = def;
	handle.type = type;
	handle.index = index;

	return entity_GetComponentPointer(handle);
}

#endif


#if 0
__attribute__((always_inline)) inline struct entity_transform_t *entity_GetWorldTransformPointer(struct component_handle_t component)
{
	return (struct entity_transform_t *)((char *)ent_world_transforms.elements + ent_world_transforms.element_size * component.index);
}
#endif


#if 0

__attribute__((always_inline)) inline struct entity_aabb_t *entity_GetAabbPointer(struct component_handle_t component)
{
	return (struct entity_aabb_t *)((char *)ent_entity_aabbs.elements + ent_entity_aabbs.element_size * component.index);
}

#endif

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

#if 0
__attribute__((always_inline)) inline struct entity_t *entity_GetEntityParentPointerHandle(struct entity_handle_t entity)
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

#endif

#if 0

__attribute__((always_inline)) inline struct entity_t *entity_GetEntityPointerIndex(int entity_index)
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

#endif

#if 0

__attribute__((always_inline)) inline struct entity_t *entity_GetEntityDefPointerIndex(int entity_def_index)
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

#endif

#if 0
__attribute__((always_inline)) inline struct entity_handle_t entity_GetEntityHandle(char *name, int get_def)
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

#endif

#if 0

struct entity_handle_t entity_GetNestledEntityHandle_Stack[1024];

__attribute__((always_inline)) inline struct entity_handle_t entity_GetNestledEntityHandle(struct entity_handle_t parent_entity, char *entity)
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

struct entity_source_file_t *entity_GetSourceFile(struct entity_handle_t entity)
{
	struct entity_source_file_t *source_file = NULL;
	struct entity_t *entity_ptr;

    if(!entity.def)
	{
		return NULL;
	}

	entity_ptr = entity_GetEntityPointerHandle(entity);

    if(entity_ptr)
	{
        if(entity_ptr->flags & ENTITY_FLAG_ON_DISK)
		{
            source_file = (struct entity_source_file_t *)ent_entity_def_source_files.elements + entity_ptr->spawn_time;
		}
	}

	return source_file;
}

struct entity_handle_t *entity_GetEntityDefs(int *count)
{
    struct entity_t *entity_defs;
    struct entity_t *entity_def;
    int i;
    int c;


    entity_defs = (struct entity_t *) ent_entities[1].elements;
    c = ent_entities[1].element_count;


    ent_valid_entity_defs.element_count = 0;

    for(i = 0; i < c; i++)
    {
        entity_def = entity_GetEntityPointerHandle(ENTITY_HANDLE(i, 1));

        if(entity_def)
        {
            list_add(&ent_valid_entity_defs, &ENTITY_HANDLE(i, 1));
        }
    }

    *count = ent_valid_entity_defs.element_count;

    return (struct entity_handle_t *)ent_valid_entity_defs.elements;
}






void entity_TranslateEntity(struct entity_handle_t entity, vec3_t direction, float amount)
{
	struct entity_t *entity_ptr;

	//struct controller_component_t *controller;
	struct physics_component_t *physics_component;
	struct transform_component_t *transform;
//	struct collider_t *collider;

	if(entity.def)
	{
		return;
	}

	entity_ptr = entity_GetEntityPointerHandle(entity);

	if(entity_ptr)
	{
		transform = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);

		transform->position.x += direction.x * amount;
		transform->position.y += direction.y * amount;
		transform->position.z += direction.z * amount;

		transform->flags |= ENTITY_FLAG_HAS_MOVED;

		physics_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);

		if(physics_component)
		{
			physics_SetColliderPosition(physics_component->collider, transform->position);
		}
	}
}

void entity_SetEntityPosition(struct entity_handle_t entity, vec3_t position)
{
    struct entity_t *entity_ptr;
    struct transform_component_t *transform;
    struct physics_component_t *collider;

    entity_ptr = entity_GetEntityPointerHandle(entity);

    if(entity_ptr)
    {
        transform = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);

        transform->position.x = position.x;
        transform->position.y = position.y;
        transform->position.z = position.z;

        transform->flags |= ENTITY_FLAG_HAS_MOVED;

        collider = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);

        if(collider)
        {
            physics_SetColliderPosition(collider->collider, transform->position);
        }
    }
}




void entity_RotateEntity(struct entity_handle_t entity, vec3_t axis, float amount, int set)
{
	struct entity_t *entity_ptr;
	struct transform_component_t *transform;
	struct physics_component_t *physics;

	entity_ptr = entity_GetEntityPointerHandle(entity);

	if(entity_ptr)
    {
        transform = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
        mat3_t_rotate(&transform->orientation, axis, amount, set);

        transform->flags |= ENTITY_FLAG_HAS_MOVED;

        physics = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);

        if(physics)
        {
            physics_SetColliderOrientation(physics->collider, &transform->orientation);
        }
    }
}

void entity_SetEntityOrientation(struct entity_handle_t entity, mat3_t *orientation)
{
    struct entity_t *entity_ptr;
	struct transform_component_t *transform;
	struct physics_component_t *physics;

	entity_ptr = entity_GetEntityPointerHandle(entity);

	if(entity_ptr)
    {
        transform = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
        transform->orientation = *orientation;

        transform->flags |= ENTITY_FLAG_HAS_MOVED;

        physics = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);

        if(physics)
        {
            physics_SetColliderOrientation(physics->collider, &transform->orientation);
        }
    }
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

	//struct transform_component_t *transform;
	struct entity_transform_t *transform;
	struct navigation_component_t *navigation_component;
	struct entity_t *entity_ptr;
	struct waypoint_t *waypoint;
	struct waypoint_t **route;
	struct waypoint_t **component_route;

	vec3_t from;
	//struct waypoint_t *waypoint;
	int route_length;
	int i;

	entity_ptr = entity_GetEntityPointerHandle(entity);

	transform = entity_GetWorldTransformPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
	navigation_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_NAVIGATION]);

	if(navigation_component)
	{

		from.x = transform->transform.floats[3][0];
		from.y = transform->transform.floats[3][1];
		from.z = transform->transform.floats[3][2];

		route = navigation_FindPath(&route_length, from, to);

        navigation_component->route.element_count = 0;

		if(route)
		{
		    if(route_length > navigation_component->route.max_elements)
            {
                list_resize(&navigation_component->route, (route_length + 3) & (~3));
		    }

            navigation_component->route.element_count = route_length;

            component_route = (struct waypoint_t **)navigation_component->route.elements;

			//printf("route starts at: [%f %f %f]\n", from.x, from.y, from.z);

            for(i = 0; i < route_length; i++)
            {
                waypoint = route[i];
                component_route[i] = waypoint;
              //  printf(":[%f %f %f]\n", waypoint->position.x, waypoint->position.y, waypoint->position.z);
            }

           // printf("route ends at: [%f %f %f]\n", to.x, to.y, to.z);

            //memcpy(navigation_component->route.elements, route, sizeof(struct waypoint_t *) * route_length);

            navigation_component->current_waypoint = 0;
		}
	}

}

#define MAX_TOUCHED_ENTITIES 1024
struct entity_handle_t entity_touched_entities[MAX_TOUCHED_ENTITIES];

struct entity_handle_t *entity_GetTouchedEntities(struct entity_handle_t entity, int *count)
{
	int i;
	int start;
	int touched = 0;
	struct physics_component_t *physics_component;
	struct contact_record_t *contact_records;
	struct collider_t *collider;
	struct collider_t *other;
	struct entity_t *entity_ptr;

	if(entity.def)
	{
		return NULL;
	}

	entity_ptr = entity_GetEntityPointerHandle(entity);

	if(entity_ptr)
	{
		physics_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);
		collider = physics_GetColliderPointer(physics_component->collider);

		contact_records = physics_GetColliderContactRecords(physics_component->collider);

		for(i = 0; i < collider->contact_record_count && i < MAX_TOUCHED_ENTITIES; i++)
		{
			other = physics_GetColliderPointer(contact_records[i].collider);

			if(other)
			{
				entity_touched_entities[touched].entity_index = other->entity_index;
				entity_touched_entities[touched].def = 0;

				touched++;
			}
		}
	}

	//assert(touched < MAX_TOUCHED_ENTITIES);

	*count = touched;
	return entity_touched_entities;
}


void entity_TouchedEntities(struct entity_handle_t entity)
{
    struct entity_t *entity_ptr;
    struct physics_component_t *physics_component;
}


/*
=========================
entity_UpdateEntityAabbIndex

self explanatory...

=========================
*/
void entity_UpdateAabb(struct entity_handle_t entity)
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


void entity_AabbWorldExtents(struct entity_handle_t entity, vec3_t *extent_max, vec3_t *extent_min)
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

	entity_ptr = entity_GetEntityPointerHandle(entity);

	if(entity_ptr)
	{
		//transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);

		world_transform = entity_GetWorldTransformPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
		aabb = entity_GetAabbPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);

		if(entity_ptr->components[COMPONENT_TYPE_MODEL].type == COMPONENT_TYPE_NONE)
		{
			/* this entity doesn't have a model component, so no aabb extent can be calculated... */

			extent_max->x = world_transform->transform.floats[3][0];
			extent_max->y = world_transform->transform.floats[3][1];
			extent_max->z = world_transform->transform.floats[3][2];

			extent_min->x = world_transform->transform.floats[3][0];
			extent_min->y = world_transform->transform.floats[3][1];
			extent_min->z = world_transform->transform.floats[3][2];

			return;
		}



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
			mat4_t_vec4_t_mult(&world_transform->transform, &transformed_aabb[i]);

			if(transformed_aabb[i].x > max.x) max.x = transformed_aabb[i].x;
			if(transformed_aabb[i].y > max.y) max.y = transformed_aabb[i].y;
			if(transformed_aabb[i].z > max.z) max.z = transformed_aabb[i].z;

			if(-transformed_aabb[i].x > max.x) max.x = -transformed_aabb[i].x;
			if(-transformed_aabb[i].y > max.y) max.y = -transformed_aabb[i].y;
			if(-transformed_aabb[i].z > max.z) max.z = -transformed_aabb[i].z;
		}

		//*extent_max = max;

		extent_max->x = world_transform->transform.floats[3][0] + max.x;
		extent_max->y = world_transform->transform.floats[3][1] + max.y;
		extent_max->z = world_transform->transform.floats[3][2] + max.z;

		extent_min->x = world_transform->transform.floats[3][0] - max.x;
		extent_min->y = world_transform->transform.floats[3][1] - max.y;
		extent_min->z = world_transform->transform.floats[3][2] - max.z;
	}
}

void entity_UpdateScriptComponents()
{
	int i;
	int c;

	int engine_state;

	//static int last_update = 0;

	//struct script_controller_component_t *script_controllers;
	//struct script_controller_component_t *script_controller;

	struct script_component_t *script_components;
	struct script_component_t *script_component;
	struct entity_t *entity;

	struct stack_list_t *list;

	list = &ent_components[0][COMPONENT_TYPE_SCRIPT];

	c = ent_components[0][COMPONENT_TYPE_SCRIPT].element_count;

	engine_state = engine_GetEngineState();

	if(engine_state & ENGINE_PLAYING)
	{
	    //printf("executing\n");

		for(i = 0; i < c; i++)
		{
			script_component = (struct script_component_t *)list->elements + i;

			if(script_component->base.flags & COMPONENT_FLAG_INVALID)
			{
				continue;
			}

			//printf("execute\n");
			script_ExecuteScriptImediate((struct script_t *)script_component->script, script_component);
		}
	}

	entity_RemoveMarkedEntities(0);

	ent_frame++;
}

void entity_UpdatePhysicsComponents()
{
    int i;
    int j;
    int count;
    struct physics_component_t *physics_components;
    struct physics_component_t *physics_component;
    struct collider_t *collider;
    struct collider_t *other;
    struct contact_record_t *contact_records;
    struct contact_record_t *contact_record;

    struct entity_contact_t *entity_contacts;
    struct entity_contact_t *first_entity_contact;
    struct entity_contact_t *entity_contact;


    int prev_start = 0;
    int increment;

    ent_entity_contacts.element_count = 0;
    entity_contacts = (struct entity_contact_t *)ent_entity_contacts.elements;

    physics_components = (struct physics_component_t *)ent_components[0][COMPONENT_TYPE_PHYSICS].elements;
    count = ent_components[0][COMPONENT_TYPE_PHYSICS].element_count;

    for(i = 0; i < count; i++)
    {
        physics_component = physics_components + i;

        collider = physics_GetColliderPointer(physics_component->collider);

        if(collider)
        {
            if(physics_component->entity_contact_count > physics_component->max_entity_contact_count)
            {
                physics_component->max_entity_contact_count = (physics_component->entity_contact_count + 3) & (~3);
            }

            physics_component->first_entity_contact = prev_start;
            physics_component->entity_contact_count = 0;

            prev_start += physics_component->max_entity_contact_count;

            contact_records = physics_GetColliderContactRecords(physics_component->collider);

            increment = physics_component->max_entity_contact_count + ent_entity_contacts.element_count;

            if(increment > ent_entity_contacts.max_elements)
            {
                increment = 128 + (increment + 3) & (~3);
                list_resize(&ent_entity_contacts, increment);
                entity_contacts = (struct entity_contact_t *)ent_entity_contacts.elements;
            }

            first_entity_contact = entity_contacts + physics_component->first_entity_contact;

            for(j = 0; j < collider->contact_record_count; j++)
            {
                if(physics_component->entity_contact_count < physics_component->max_entity_contact_count)
                {
                    entity_contact = first_entity_contact + physics_component->entity_contact_count;

                    contact_record = contact_records + j;

                    other = physics_GetColliderPointer(contact_record->collider);

                    if(other)
                    {
                        entity_contact->entity = (struct entity_handle_t){0, other->entity_index};
                    }
                    else
                    {
                        entity_contact->entity = INVALID_ENTITY_HANDLE;
                    }
                }

                physics_component->entity_contact_count++;
            }

            ent_entity_contacts.element_count = prev_start;
        }
    }
}


void entity_RecursiveUpdateTransform(struct entity_transform_t *parent_transform, struct component_handle_t child_transform, vec3_t *aabb_max, vec3_t *aabb_min)
{
	//struct entity_transform_t current_transform;
	struct entity_t *entity;
	struct entity_transform_t *world_transform;
	struct transform_component_t *transform_component;
	struct bone_component_t *bone_component;
	struct transform_component_t *bone_transform_component;
	struct entity_aabb_t *aabb;
	mat4_t transform;
	mat4_t bone_transform;
	int i;

	vec3_t aabb_extent_max;
	vec3_t aabb_extent_min;

	vec3_t max;
	vec3_t min;

	vec3_t aabb_center;

	transform_component = entity_GetComponentPointer(child_transform);
	world_transform = entity_GetWorldTransformPointer(child_transform);

	if(parent_transform)
	{
		mat4_t_compose2(&transform, &transform_component->orientation, transform_component->position, transform_component->scale);
		mat4_t_mult(&world_transform->transform, &transform, &parent_transform->transform);
	}
//	else
//	{
//		mat4_t_compose2(&world_transform->transform, &transform_component->orientation, transform_component->position, transform_component->scale);
//	}

	entity = entity_GetEntityPointerHandle(transform_component->base.entity);

	if(entity)
    {
        if(entity->components[COMPONENT_TYPE_BONE].type != COMPONENT_TYPE_NONE)
        {
            bone_component = (struct bone_component_t *)entity_GetComponentPointer(entity->components[COMPONENT_TYPE_BONE]);
            bone_transform_component = entity_GetComponentPointer(bone_component->bone_transform);

            mat4_t_compose2(&bone_transform, &bone_transform_component->orientation,
                                              bone_transform_component->position,
                                              bone_transform_component->scale);

            mat4_t_mult_fast(&transform, &bone_transform, &world_transform->transform);
            world_transform->transform = transform;
        }
    }

	/*aabb_extent_max.x = world_transform->transform.floats[3][0];
	aabb_extent_max.y = world_transform->transform.floats[3][1];
	aabb_extent_max.z = world_transform->transform.floats[3][2];

	aabb_extent_min.x = world_transform->transform.floats[3][0];
	aabb_extent_min.y = world_transform->transform.floats[3][1];
	aabb_extent_min.z = world_transform->transform.floats[3][2];*/

	//entity_AabbWorldExtents(transform_component->base.entity, &aabb_extent_max, &aabb_extent_min);

	for(i = 0; i < transform_component->children_count; i++)
	{
		entity_RecursiveUpdateTransform(world_transform, transform_component->child_transforms[i], &aabb_extent_max, &aabb_extent_min);
	}



	if(entity)
	{
		entity_UpdateAabb(transform_component->base.entity);
		//aabb = entity_GetAabbPointer(child_transform);

	/*	max.x = aabb->current_extents.x + world_transform->transform.floats[3][0];
		max.y = aabb->current_extents.y + world_transform->transform.floats[3][1];
		max.z = aabb->current_extents.z + world_transform->transform.floats[3][2];

		min.x = world_transform->transform.floats[3][0] - aabb->current_extents.x;
		min.y = world_transform->transform.floats[3][1] - aabb->current_extents.y;
		min.z = world_transform->transform.floats[3][2] - aabb->current_extents.z;*/


	//	if(aabb_extent_max.x > max.x) = aabb->current_extents.x += aabb_extent_max


	/*	entity_AabbWorldExtents(transform_component->base.entity, &max, &min);

		if(max.x > aabb_extent_max.x) aabb_extent_max.x = max.x;
		if(max.y > aabb_extent_max.y) aabb_extent_max.y = max.y;
		if(max.z > aabb_extent_max.z) aabb_extent_max.z = max.z;

		if(min.x < aabb_extent_min.x) aabb_extent_min.x = min.x;
		if(min.y < aabb_extent_min.y) aabb_extent_min.y = min.y;
		if(min.z < aabb_extent_min.z) aabb_extent_min.z = min.z;*/

	/*	aabb = entity_GetAabbPointer(child_transform);

		aabb_center.x = (aabb_extent_max.x + aabb_extent_min.x) / 2.0;
		aabb_center.y = (aabb_extent_max.y + aabb_extent_min.y) / 2.0;
		aabb_center.z = (aabb_extent_max.z + aabb_extent_min.z) / 2.0;

		aabb->current_extents.x = aabb_extent_max.x - aabb_center.x;
		aabb->current_extents.y = aabb_extent_max.y - aabb_center.y;
		aabb->current_extents.z = aabb_extent_max.z - aabb_center.z;

		if(aabb_extent_max.x > aabb_max->x) aabb_max->x = aabb_extent_max.x;
		if(aabb_extent_max.y > aabb_max->y) aabb_max->y = aabb_extent_max.y;
		if(aabb_extent_max.z > aabb_max->z) aabb_max->z = aabb_extent_max.z;

		if(aabb_extent_min.x < aabb_min->x) aabb_min->x = aabb_extent_min.x;
		if(aabb_extent_min.y < aabb_min->y) aabb_min->y = aabb_extent_min.y;
		if(aabb_extent_min.z < aabb_min->z) aabb_min->z = aabb_extent_min.z;	*/
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
	struct bsp_dleaf_t *old_leaf;
	struct bsp_dleaf_t *cur_leaf;
	struct collider_t *collider;
	struct component_handle_t *top_transforms;
	mat3_t rotation;

	vec3_t aabb_max;
	vec3_t aabb_min;

	vec3_t root_aabb_max;
	vec3_t root_aabb_min;

	vec3_t aabb_center;

	struct entity_aabb_t *root_aabb;

	struct transform_component_t *transform_components;
	struct transform_component_t *local_transform;
	struct entity_transform_t *world_transform;
	struct entity_transform_t *world_transforms;

	struct entity_aabb_t *aabbs;
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
	aabbs = (struct entity_aabb_t *)ent_entity_aabbs.elements;

	c = ent_top_transforms.element_count;

	//physics_components = (struct physics_component_t *)ent_components[0][COMPONENT_TYPE_PHYSICS].elements;

	for(i = 0; i < c; i++)
	{
		/*world_transform = world_transforms + top_transforms[i].index;
		local_transform = transform_components + top_transforms[i].index;*/

		local_transform = entity_GetComponentPointer(top_transforms[i]);

		if(!local_transform)
		{
			continue;
		}

		world_transform = entity_GetWorldTransformPointer(top_transforms[i]);

		if(local_transform->base.entity.entity_index != INVALID_ENTITY_INDEX)
		{
			entity = entity_GetEntityPointerHandle(local_transform->base.entity);

			if(entity->components[COMPONENT_TYPE_PHYSICS].type != COMPONENT_TYPE_NONE)
			{
				physics_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_PHYSICS]);
				collider = physics_GetColliderPointer(physics_component->collider);

				if(collider)
				{
					mat3_t_mult(&rotation, &local_transform->orientation, &collider->orientation);
					mat4_t_compose2(&world_transform->transform, &rotation, collider->position, local_transform->scale);
				}
				else
				{
					mat4_t_compose2(&world_transform->transform, &local_transform->orientation, local_transform->position, local_transform->scale);
				}
			}
            /*else
            {

            }*/
			/*else if(local_transform->bone.type != COMPONENT_TYPE_NONE)
            {

            }*/
			else
			{
				mat4_t_compose2(&world_transform->transform, &local_transform->orientation, local_transform->position, local_transform->scale);
			}

			entity_RecursiveUpdateTransform(NULL, top_transforms[i], &aabb_max, &aabb_min);

			/*if(local_transform->child_transforms)
			{
				for(j = 0; j < local_transform->children_count; j++)
				{
					entity_RecursiveUpdateTransform(world_transform, local_transform->child_transforms[j], &aabb_max, &aabb_min);
				}
			}*/
			//else
			//{
			//entity_UpdateAabb(local_transform->base.entity);
			//}

		/*	if(entity->components[COMPONENT_TYPE_MODEL].type != COMPONENT_TYPE_NONE)
			{
				entity_UpdateEntityAabbIndex(local_transform->base.entity);
			}*/
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

	struct entity_t *entity;
	struct view_def_t *view_def;
//	camera_t *camera;

	vec3_t forward_vec;
	vec3_t right_vec;
	vec3_t up_vec;

	camera_components = (struct camera_component_t *)ent_components[0][COMPONENT_TYPE_CAMERA].elements;
	c = ent_components[0][COMPONENT_TYPE_CAMERA].element_count;


	for(i = 0; i < c; i++)
	{
		camera_component = camera_components + i;

		if(!(camera_component->base.flags & COMPONENT_FLAG_INVALID))
		{
			entity = entity_GetEntityPointerHandle(camera_component->base.entity);
			world_transform = entity_GetWorldTransformPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);

			//world_transform = entity_GetWorldTransformPointer(camera_component->transform);

			if(camera_component->base.flags & COMPONENT_FLAG_INVALID)
			{
				continue;
			}

			view_def = renderer_GetViewPointer(camera_component->view);

			if(view_def)
			{
				/*camera = camera_component->camera;

				camera->world_position.x = world_transform->transform.floats[3][0];
				camera->world_position.y = world_transform->transform.floats[3][1];
				camera->world_position.z = world_transform->transform.floats[3][2];*/

				view_def->world_position.x = world_transform->transform.floats[3][0];
				view_def->world_position.y = world_transform->transform.floats[3][1];
				view_def->world_position.z = world_transform->transform.floats[3][2];

				right_vec.x = world_transform->transform.floats[0][0];
				right_vec.y = world_transform->transform.floats[0][1];
				right_vec.z = world_transform->transform.floats[0][2];

				up_vec.x = world_transform->transform.floats[1][0];
				up_vec.y = world_transform->transform.floats[1][1];
				up_vec.z = world_transform->transform.floats[1][2];

				forward_vec.x = world_transform->transform.floats[2][0];
				forward_vec.y = world_transform->transform.floats[2][1];
				forward_vec.z = world_transform->transform.floats[2][2];


				right_vec = normalize3(right_vec);
				up_vec = normalize3(up_vec);
				forward_vec = normalize3(forward_vec);



				view_def->world_orientation.floats[0][0] = right_vec.x;
				view_def->world_orientation.floats[0][1] = right_vec.y;
				view_def->world_orientation.floats[0][2] = right_vec.z;

				view_def->world_orientation.floats[1][0] = up_vec.x;
				view_def->world_orientation.floats[1][1] = up_vec.y;
				view_def->world_orientation.floats[1][2] = up_vec.z;

				view_def->world_orientation.floats[2][0] = forward_vec.x;
				view_def->world_orientation.floats[2][1] = forward_vec.y;
				view_def->world_orientation.floats[2][2] = forward_vec.z;




				/*camera->world_orientation.floats[0][0] = world_transform->transform.floats[0][0];
				camera->world_orientation.floats[0][1] = world_transform->transform.floats[0][1];
				camera->world_orientation.floats[0][2] = world_transform->transform.floats[0][2];

				camera->world_orientation.floats[1][0] = world_transform->transform.floats[1][0];
				camera->world_orientation.floats[1][1] = world_transform->transform.floats[1][1];
				camera->world_orientation.floats[1][2] = world_transform->transform.floats[1][2];

				camera->world_orientation.floats[2][0] = world_transform->transform.floats[2][0];
				camera->world_orientation.floats[2][1] = world_transform->transform.floats[2][1];
				camera->world_orientation.floats[2][2] = world_transform->transform.floats[2][2];*/

				renderer_ComputeViewMatrix(camera_component->view);
			}
		}


	}
}


void entity_UpdateEntities()
{
	//int i;
	//int c;
	//struct entity_t *entity;

	//c = ent_entities[0].element_count;

	//for(i = 0; i < c; i++)
	//{
	//	entity = (struct entity_t *)ent_entities[0].elements + i;

	//	if(entity->flags & ENTITY_INVALID)
	//	{
	//		continue;
	//	}

		//entity->life++;
	//}
}

int entity_LineOfSightToEntity(struct entity_handle_t from, struct entity_handle_t to)
{
    struct entity_t *entity_from;
    struct entity_t *entity_to;

    struct entity_transform_t *from_transform;
    struct entity_transform_t *to_transform;

    entity_from = entity_GetEntityPointerHandle(from);
    entity_to = entity_GetEntityPointerHandle(to);

    if(entity_from && entity_to)
	{
		from_transform = entity_GetWorldTransformPointer(entity_from->components[COMPONENT_TYPE_TRANSFORM]);
		to_transform = entity_GetWorldTransformPointer(entity_to->components[COMPONENT_TYPE_TRANSFORM]);


        return !physics_Raycast(vec3_t_c(from_transform->transform.floats[3][0], from_transform->transform.floats[3][1], from_transform->transform.floats[3][2]),
							   vec3_t_c(to_transform->transform.floats[3][0], to_transform->transform.floats[3][1], to_transform->transform.floats[3][2]), NULL, NULL, 1);

	}

	return 0;
}

/*
==============================================================
==============================================================
==============================================================
*/


int entity_CreateTrigger(mat3_t *orientation, vec3_t position, vec3_t scale, char *event_name, char *name)
{
	struct trigger_t *trigger;
    int trigger_index;

    trigger_index = stack_list_add(&ent_triggers, NULL);
    trigger = (struct trigger_t *)stack_list_get(&ent_triggers, trigger_index);

	if(orientation)
	{
		memcpy(&trigger->orientation, orientation, sizeof(mat3_t));
	}
	else
	{
		trigger->orientation = mat3_t_id();
	}


	trigger->position = position;
	trigger->scale = scale;
	trigger->flags = 0;

	/*if(event_name)
	{
		trigger->event_name = memory_Strdup(event_name);
	}
	else
	{
		trigger->event_name = NULL;
	}*/

	trigger->event_name = memory_Calloc(WORLD_EVENT_NAME_MAX_LEN, 1);

	if(event_name)
	{
		strcpy(trigger->event_name, event_name);
	}

	trigger->name = memory_Calloc(ENTITY_TRIGGER_NAME_MAX_LEN, 1);
	strcpy(trigger->name, name);

	//trigger->name = memory_Strdup(name);

	trigger->collider = physics_CreateTrigger(orientation, position, scale);

	trigger->trigger_filters = list_create(sizeof(struct trigger_filter_t), 8, NULL);

	return trigger_index;
}

struct trigger_t *entity_GetTriggerPointerIndex(int trigger_index)
{
    struct trigger_t *trigger;

    if(trigger_index >= 0 && trigger_index < ent_triggers.element_count)
	{
        trigger = (struct trigger_t *)ent_triggers.elements + trigger_index;

		if(!(trigger->flags & TRIGGER_FLAG_INVALID))
		{
			return trigger;
		}
	}

	return NULL;
}

int entity_GetTrigger(char *name)
{
	struct trigger_t *triggers;
	int i;

	triggers = (struct trigger_t *)ent_triggers.elements;

	for(i = 0; i < ent_triggers.element_count; i++)
	{
        if(triggers[i].flags & TRIGGER_FLAG_INVALID)
		{
			continue;
		}

		if(!strcmp(name, triggers[i].name))
		{
			return i;
		}
	}

	return -1;
}

int entity_IsTriggered(int trigger_index)
{
    struct trigger_t *trigger;

    trigger = entity_GetTriggerPointerIndex(trigger_index);

    if(trigger)
	{
		return (trigger->flags & TRIGGER_FLAG_TRIGGERED) && 1;
	}

	return 0;
}

void entity_DestroyTriggerIndex(int trigger_index)
{
	struct trigger_t *trigger;

    trigger = entity_GetTriggerPointerIndex(trigger_index);

    if(trigger)
	{
        trigger->flags |= TRIGGER_FLAG_INVALID;
		physics_DestroyCollider(trigger->collider);

		memory_Free(trigger->event_name);
		memory_Free(trigger->name);

		stack_list_remove(&ent_triggers, trigger_index);
	}
}

void entity_AddTriggerFilter(int trigger_index, char *filter_name)
{
    struct trigger_t *trigger;
    struct trigger_filter_t *filters;
    struct trigger_filter_t *filter;
    int filter_index;

    int i;

    trigger = entity_GetTriggerPointerIndex(trigger_index);

    if(trigger)
	{
		filters = (struct trigger_filter_t *)trigger->trigger_filters.elements;

		for(i = 0; i < trigger->trigger_filters.element_count; i++)
		{
            if(!strcmp(filter_name, filters[i].prop_name))
			{
				return;
			}
		}

        filter_index = list_add(&trigger->trigger_filters, NULL);
        filter = list_get(&trigger->trigger_filters, filter_index);

        //filter->prop_name = memory_Strdup(filter_name);

        filter->prop_name = memory_Calloc(ENTITY_PROP_NAME_MAX_LEN, 1);

        strcpy(filter->prop_name, filter_name);
	}
}

void entity_RemoveTriggerFilter(int trigger_index, char *filter_name)
{
	struct trigger_t *trigger;
    struct trigger_filter_t *filters;
    struct trigger_filter_t *filter;
    int filter_index;

    int i;

    trigger = entity_GetTriggerPointerIndex(trigger_index);

    if(trigger)
	{
		filters = (struct trigger_filter_t *)trigger->trigger_filters.elements;

		for(i = 0; i < trigger->trigger_filters.element_count; i++)
		{
            if(!strcmp(filter_name, filters[i].prop_name))
			{
				memory_Free(filters[i].prop_name);
				list_remove(&trigger->trigger_filters, i);

				return;
			}
		}


	}
}

void entity_UpdateTriggers()
{
    struct trigger_t *triggers;
    struct trigger_t *trigger;
    struct trigger_collider_t *trigger_collider;
    struct trigger_filter_t *filters;
    struct collider_t *other_collider;

    struct entity_prop_t *prop;

    struct entity_t *entity;
    struct entity_handle_t entity_handle;
	int trigger_count;
	int i;
	int j;
	int k;

	struct contact_record_t *contact_records;
	int contact_record_count;

    triggers = (struct trigger_t *)ent_triggers.elements;
    trigger_count = ent_triggers.element_count;

    entity_handle.def = 0;

    for(i = 0; i < trigger_count; i++)
	{
        trigger = triggers + i;

        if(trigger->flags & TRIGGER_FLAG_INVALID)
		{
			continue;
		}

		trigger->flags &= ~TRIGGER_FLAG_TRIGGERED;

		filters = (struct trigger_filter_t *)trigger->trigger_filters.elements;

		trigger_collider = (struct trigger_collider_t *)physics_GetColliderPointer(trigger->collider);
		contact_records = physics_GetColliderContactRecords(trigger->collider);

		for(j = 0; j < trigger_collider->base.contact_record_count; j++)
		{
			other_collider = physics_GetColliderPointer(contact_records[j].collider);

			if(other_collider)
			{
				entity_handle.entity_index = other_collider->entity_index;

				entity = entity_GetEntityPointerHandle(entity_handle);

				if(entity)
				{
					for(k = 0; k < trigger->trigger_filters.element_count; k++)
					{
						prop = entity_GetPropPointer(entity_handle, filters[k].prop_name);

						if(prop)
						{
							/* T-T-T-TRIGGEREEEED!!!!! */

							if(trigger->event_name)
							{
								world_CallEvent(trigger->event_name);
							}

							trigger->flags |= TRIGGER_FLAG_TRIGGERED;

							goto _next_trigger;
						}
					}
				}
			}
		}

		_next_trigger:

		continue;
	}
}

void entity_SetTriggerPosition(int trigger_index, vec3_t position)
{
	struct trigger_t *trigger;

	trigger = entity_GetTriggerPointerIndex(trigger_index);

	if(trigger)
	{
        trigger->position.x = position.x;
		trigger->position.y = position.y;
		trigger->position.z = position.z;

		physics_SetColliderPosition(trigger->collider, trigger->position);
	}
}

void entity_TranslateTrigger(int trigger_index, vec3_t direction, float amount)
{
	struct trigger_t *trigger;

	trigger = entity_GetTriggerPointerIndex(trigger_index);

	if(trigger)
	{
        trigger->position.x += direction.x * amount;
		trigger->position.y += direction.y * amount;
		trigger->position.z += direction.z * amount;

		physics_SetColliderPosition(trigger->collider, trigger->position);
	}
}


void entity_ScaleTrigger(int trigger_index, vec3_t axis, float amount)
{
	struct trigger_t *trigger;

	trigger = entity_GetTriggerPointerIndex(trigger_index);

	if(trigger)
	{
		trigger->scale.x += axis.x * amount;
		trigger->scale.y += axis.y * amount;
		trigger->scale.z += axis.z * amount;

		if(trigger->scale.x <= 0.0) trigger->scale.x = 0.001;
		if(trigger->scale.y <= 0.0) trigger->scale.y = 0.001;
		if(trigger->scale.z <= 0.0) trigger->scale.z = 0.001;

		physics_SetColliderScale(trigger->collider, trigger->scale);
	}
}

void entity_DestroyAllTriggers()
{
	int i;

    for(i = 0; i < ent_triggers.element_count; i++)
	{
		entity_DestroyTriggerIndex(i);
	}
}

/*
==============================================================
==============================================================
==============================================================
*/

//struct entity_handle_t ent_current_entity;

struct entity_handle_t entity_handles[1024];
struct script_array_t script_array;

void *entity_SetupScriptDataCallback(struct script_t *script, void *script_controller)
{
	struct entity_script_t *entity_script;
	struct entity_t *ent;
	void *entry_point;

	struct script_array_t *collided_entities;

	struct script_component_t *script_component;
	struct physics_component_t *physics_component;


	struct entity_handle_t *touched;
	int touched_count;

	assert(script_controller);

	entity_script = (struct entity_script_t *)script;
	script_component = (struct script_component_t *)script_controller;


	/*if(script_GetContextStackTop() < MAX_SCRIPT_CONTEXTS - 2)
	{
		printf("entity_SetupScriptDataCallback: reentrant scripts not supported yet!\n");
	}*/

	//collided_entities = entity_script->collided_array;

	//ent_current_entity = script_component->base.entity;

	script_SetCurrentContextData(&script_component->base.entity, sizeof(struct entity_handle_t));

	ent = entity_GetEntityPointerHandle(script_component->base.entity);

	physics_component = entity_GetComponentPointer(ent->components[COMPONENT_TYPE_PHYSICS]);


	if(engine_GetEngineState() & ENGINE_PLAYING)
	{
		if(ent->flags & ENTITY_FLAG_MARKED_INVALID)
		{
			if(!(ent->flags & ENTITY_FLAG_EXECUTED_DIE_FUNCTION))
			{
				script_QueueEntryPoint(entity_script->on_die_entry_point);
			}

			ent->flags |= ENTITY_FLAG_EXECUTED_DIE_FUNCTION;
		}
		else
		{
			//if(r_frame - ent->spawn_time <= 1)
			//{
			if(!(ent->flags & ENTITY_FLAG_EXECUTED_SPAWN_FUNCTION))
			{
				script_QueueEntryPoint(entity_script->on_spawn_entry_point);
			}

			ent->flags |= ENTITY_FLAG_EXECUTED_SPAWN_FUNCTION;
				//script_QueueEntryPoint(entity_script->on_spawn_entry_point);
			//}

			if(physics_component)
			{
				if(entity_script->on_collision_entry_point)
				{
					if(physics_component->entity_contact_count)
					{
						script_array.buffer = (struct entity_contact_t *)ent_entity_contacts.elements + physics_component->first_entity_contact;
						script_array.element_count = physics_component->entity_contact_count;
						script_array.element_size = sizeof(struct entity_contact_t);

						script_QueueEntryPoint(entity_script->on_collision_entry_point);
						script_PushArg(&script_array, SCRIPT_ARG_TYPE_ADDRESS);
					}


					#if 0
					if(physics_HasNewCollisions(physics_component->collider.collider_handle))
					{
						touched = entity_GetTouchedEntities(ent_current_entity, &touched_count);

						if(touched_count)
						{
							memcpy(entity_handles, touched, sizeof(struct entity_handle_t) * touched_count);

							script_array.buffer = entity_handles;
							script_array.element_size = sizeof(struct entity_handle_t);
							script_array.element_count = touched_count;

						/*	if(collided_entities)
							{
								collided_entities->buffer = entity_handles;
								collided_entities->element_size = sizeof(struct entity_handle_t);
								collided_entities->element_count = touched_count;
							}
							else
							{

							}*/



							script_QueueEntryPoint(entity_script->on_collision_entry_point);
							script_PushArg(&script_array, SCRIPT_ARG_TYPE_ADDRESS);
						}
					}

					#endif
				}

			}

			script_QueueEntryPoint(entity_script->on_update_entry_point);
		}
	}




	//script_component->flags &= ~SCRIPT_CONTROLLER_FLAG_FIRST_RUN;
	return entry_point;
}

int entity_GetScriptDataCallback(struct script_t *script)
{
	struct entity_script_t *entity_script;

	entity_script = (struct entity_script_t *)script;
	entity_script->on_first_run_entry_point = script_GetFunctionAddress("OnFirstRun", script);
	entity_script->on_spawn_entry_point = script_GetFunctionAddress("OnSpawn", script);
	entity_script->on_die_entry_point = script_GetFunctionAddress("OnDie", script);
	entity_script->on_collision_entry_point = script_GetFunctionAddress("OnCollision", script);
	entity_script->on_update_entry_point = script_GetFunctionAddress("OnUpdate", script);

	entity_script->collided_array = script_GetGlobalVarAddress("collided_entities", script);

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

void entity_EmitDrawCmdsForEntity(struct entity_handle_t entity, mat4_t *transform)
{
	struct entity_t *entity_ptr;
	struct model_t *model;

	if(entity.def)
	{
		return;
	}

	entity_ptr = entity_GetEntityPointerHandle(entity);




}


/*
==============================================================
==============================================================
==============================================================
*/

void entity_SaveEntityDef(char *path, struct entity_handle_t entity_def)
{
	FILE *file;
	int i;

	char *fname;
	void *buffer;
	struct entity_source_file_t *source_file;
	int buffer_size;

	char full_path[PATH_MAX];

	struct entity_t *entity = entity_GetEntityPointerHandle(entity_def);
	entity_SerializeEntityDef(&buffer, &buffer_size, entity_def);

    //entity->flags |= ENTITY_FLAG_ON_DISK;

	fname = path_AddExtToName(entity->name, ".ent");

	if(path)
    {
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, fname);

        fname = full_path;
    }

	file = fopen(fname, "wb");
	fwrite(buffer, buffer_size, 1, file);
	fclose(file);


//    source_file = stack_list_get(&ent_entity_def_source_files, entity->spawn_time);
//
//    if((!source_file) || strcmp(source_file->file_name, fname))
//	{
//		entity->spawn_time = stack_list_add(&ent_entity_def_source_files, NULL);
//		source_file = stack_list_get(&ent_entity_def_source_files, entity->spawn_time);
//		strcpy(source_file->file_name, fname);
//	}
}

struct entity_handle_t entity_LoadEntityDef(char *file_name)
{
	FILE *file;
	struct entity_t *entity;
	struct entity_source_file_t *source_file;
	unsigned long int file_size = 0;
	void *buffer;
	void *read_buffer;
	struct entity_handle_t entity_def;

	char *entity_name = path_GetNameNoExt(path_GetFileNameFromPath(file_name));



	//char *file_name = path_AddExtToName(entity_name, ".ent");

	entity_def = entity_GetEntityHandle(entity_name, 1);
	entity = entity_GetEntityPointerHandle(entity_def);

	if(!entity)
    {
        file = path_TryOpenFile(file_name);

        if(!file)
        {
            printf("entity_LoadEntityDef: couldn't load entity def [%s]\n", file_name);
            return INVALID_ENTITY_HANDLE;
        }

        file_size = path_GetFileSize(file);

        buffer = memory_Calloc(file_size, 1);
        read_buffer = buffer;

        fread(buffer, file_size, 1, file);
        fclose(file);

        entity_def = entity_ReadEntity(&read_buffer, (struct entity_handle_t){1, INVALID_ENTITY_INDEX});

        memory_Free(buffer);
    }
    else
    {
        entity->flags &= ~ENTITY_FLAG_MARKED_INVALID;
    }

	return entity_def;
}

/*
==============================================================
==============================================================
==============================================================
*/


//#include "entity.inl"



#ifdef __cplusplus
}
#endif












