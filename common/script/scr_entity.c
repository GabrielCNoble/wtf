#include "scr_entity.h"
#include "entity.h"
#include "physics.h"
#include "camera.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* this allows hiding the current entity's reference
at the cost of not allowing concurrent scripts... */
extern struct entity_handle_t ent_current_entity;

extern struct component_fields_t COMPONENT_FIELDS[COMPONENT_TYPE_LAST];

static vec3_t vec3_ret;
static mat3_t mat3_ret;

extern int r_frame;
 
/*
=====================================
=====================================
=====================================
*/

void entity_ScriptMove(vec3_t *direction)
{
	struct script_component_t *script_component;
	struct physics_component_t *physics_component;
	struct entity_t *entity_ptr;
		
	entity_ptr = entity_GetEntityPointerHandle(ent_current_entity);
	
	script_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_SCRIPT]);
	
	if(script_component)
	{
		physics_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);
		
		if(physics_component)
		{
			physics_Move(physics_component->collider.collider_handle, *direction);
		}
	}
}

void entity_ScriptSetEntityVelocity(struct entity_handle_t entity, vec3_t *velocity)
{
	struct physics_component_t *physics_component;
	struct entity_t *entity_ptr;
	
	entity_ptr = entity_GetEntityPointerHandle(entity);
	
	if(entity.def)
	{
		return;
	}
	
	if(entity_ptr)
	{
		physics_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);
		
		if(physics_component)
		{
			physics_SetColliderVelocity(physics_component->collider.collider_handle, *velocity);
		}
	}
}

void entity_ScriptJump(float jump_force)
{
	struct script_component_t *script_component;
	struct physics_component_t *physics_component;
	struct entity_t *entity_ptr;
	
	entity_ptr = entity_GetEntityPointerHandle(ent_current_entity);
	
	script_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_SCRIPT]);
	
	if(script_component)
	{
		physics_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);
		//if(controller->controller.base.type == COMPONENT_TYPE_SCRIPT_CONTROLLER)
		if(physics_component)
		{
			physics_Jump(physics_component->collider.collider_handle, jump_force);
		}
	}
}

/*
=====================================
=====================================
=====================================
*/

void entity_ScriptDie()
{
	entity_MarkForRemoval(ent_current_entity);
}

void entity_ScriptDIEYOUMOTHERFUCKER()
{
	entity_ScriptDie();
}

/*
=====================================
=====================================
=====================================
*/

void *entity_ScriptGetPosition(int local)
{
	return entity_ScriptGetEntityPosition(ent_current_entity, local);
}

void *entity_ScriptGetEntityPosition(struct entity_handle_t entity, int local)
{
	struct entity_t *entity_ptr;
	struct transform_component_t *local_transform;
	struct entity_transform_t *world_transform;
	
	entity_ptr = entity_GetEntityPointerHandle(entity);
	
	if(entity_ptr)
	{
		if(local)
		{
			local_transform = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
			vec3_ret = local_transform->position;
		}
		else
		{
			world_transform = entity_GetWorldTransformPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
			
			vec3_ret.x = world_transform->transform.floats[3][0];
			vec3_ret.y = world_transform->transform.floats[3][1];
			vec3_ret.z = world_transform->transform.floats[3][2];
		}
	}
	else
	{
		printf("entity_ScriptGetEntityPosition: bad entity handle\n");
		vec3_ret.x = 0.0;
		vec3_ret.y = 0.0;
		vec3_ret.z = 0.0;
	}
	
	
	
	return &vec3_ret;
}

void *entity_ScriptGetOrientation(int local)
{
	struct entity_t *entity;
	struct transform_component_t *transform;
	struct entity_transform_t *world_transform;
	
	entity = entity_GetEntityPointerHandle(ent_current_entity);
	
	
	if(local)
	{
		transform = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);
		mat3_ret = transform->orientation;
	}
	else
	{
		world_transform = entity_GetWorldTransformPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);
		
		mat3_ret.floats[0][0] = world_transform->transform.floats[0][0];
		mat3_ret.floats[0][1] = world_transform->transform.floats[0][1];
		mat3_ret.floats[0][2] = world_transform->transform.floats[0][2];
		
		mat3_ret.floats[1][0] = world_transform->transform.floats[1][0];
		mat3_ret.floats[1][1] = world_transform->transform.floats[1][1];
		mat3_ret.floats[1][2] = world_transform->transform.floats[1][2];
		
		mat3_ret.floats[2][0] = world_transform->transform.floats[2][0];
		mat3_ret.floats[2][1] = world_transform->transform.floats[2][1];
		mat3_ret.floats[2][2] = world_transform->transform.floats[2][2];
	}
	
	
	return &mat3_ret;
}

void *entity_ScriptGetForwardVector()
{
	struct entity_t *entity;
	struct transform_component_t *transform;
	
	entity = entity_GetEntityPointerHandle(ent_current_entity);
	transform = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);
	
	return &transform->orientation.r2;
}

void entity_ScriptRotate(vec3_t *axis, float angle, int set)
{
	struct entity_t *entity;
	struct transform_component_t *transform;
	
	entity = entity_GetEntityPointerHandle(ent_current_entity);
	transform = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);
	
	set = set && 1;
	
	mat3_t_rotate(&transform->orientation, *axis, angle, set);
}

int entity_ScriptGetLife()
{
	struct entity_t *entity;
	
	entity = entity_GetEntityPointerHandle(ent_current_entity);
	
	return r_frame - entity->spawn_time;
}

struct entity_handle_t entity_ScriptGetCurrentEntity()
{
	return ent_current_entity;
}

/*
=====================================
=====================================
=====================================
*/

struct component_handle_t entity_ScriptGetComponent(int component_index)
{
	return entity_ScriptGetEntityComponent(ent_current_entity, component_index);
}

struct component_handle_t entity_ScriptGetEntityComponent(struct entity_handle_t entity, int component_index)
{
	struct entity_t *entity_ptr;
	struct component_handle_t component = (struct component_handle_t){COMPONENT_TYPE_NONE, 0, INVALID_COMPONENT_INDEX};
	entity_ptr = entity_GetEntityPointerHandle(entity);
	
	if(entity_ptr)
	{
		component = entity_ptr->components[component_index];
	}
	else
	{
		printf("entity_ScriptGetEntityComponent: bad entity handle\n");
	}
	
	return component;
}

struct entity_handle_t entity_ScriptGetEntity(struct script_string_t *name, int get_def)
{
	char *entity_name;
	
	entity_name = script_string_GetRawString(name);
	
	get_def = get_def && 1;
	
	return entity_GetEntityHandle(entity_name, get_def);
}

struct entity_handle_t entity_ScriptGetChildEntity(struct script_string_t *entity)
{
	return entity_ScriptGetEntityChildEntity(ent_current_entity, entity);	
}

struct entity_handle_t entity_ScriptGetEntityChildEntity(struct entity_handle_t parent_entity, struct script_string_t *entity)
{
	char *entity_name;
	
	entity_name = script_string_GetRawString(entity);
	
	return entity_GetNestledEntityHandle(parent_entity, entity_name);
}

struct entity_handle_t entity_ScriptGetEntityDef(struct script_string_t *def_name)
{
	return entity_ScriptGetEntity(def_name, 1);
}

struct entity_handle_t entity_ScriptSpawnEntity(mat3_t *orientation, vec3_t *position, vec3_t *scale, struct entity_handle_t def, struct script_string_t *name)
{
	char *entity_name = script_string_GetRawString(name);
	return entity_SpawnEntity(orientation, *position, *scale, def, entity_name);
}





void entity_ScriptComponentValue(struct component_handle_t component, char *field_name, void *value, int value_type, int set)
{
	struct camera_component_t *camera_component;
	struct transform_component_t *transform_component;
	struct component_t *component_ptr;
	int i;
	
	int offset;
	int type_size;
	
	component_ptr = entity_GetComponentPointer(component);
	
	if(component_ptr)
	{
		for(i = 0; COMPONENT_FIELDS[component.type].fields[i].field_name; i++)
		{
			if(!strcmp(COMPONENT_FIELDS[component.type].fields[i].field_name, field_name))
			{
				if(COMPONENT_FIELDS[component.type].fields[i].script_type == value_type)
				{
					offset = COMPONENT_FIELDS[component.type].fields[i].offset;
					type_size = script_GetScriptTypeSize(COMPONENT_FIELDS[component.type].fields[i].script_type);
					
					if(set)
					{
						memcpy((char *)component_ptr + offset, value, type_size);
					}
					else
					{
						memcpy(value, (char *)component_ptr + offset, type_size);
					}
					
					return;
				}
			}
		}
	}
	
	
}


void entity_ScriptSetComponentValue33f(struct component_handle_t component, struct script_string_t *field_name, mat3_t *value)
{
	char *field = script_string_GetRawString(field_name);
	entity_ScriptComponentValue(component, field, value, SCRIPT_VAR_TYPE_MAT3T, 1);
}



void entity_ScriptSetComponentValue3f(struct component_handle_t component, struct script_string_t *field_name, vec3_t *value)
{
	char *field = script_string_GetRawString(field_name);
	entity_ScriptComponentValue(component, field, value, SCRIPT_VAR_TYPE_VEC3T, 1);
}

void entity_ScriptGetComponentValue3f(struct component_handle_t component, struct script_string_t *field_name, vec3_t *value)
{
	char *field = script_string_GetRawString(field_name);
	entity_ScriptComponentValue(component, field, value, SCRIPT_VAR_TYPE_VEC3T, 0);
}


/*
=====================================
=====================================
=====================================
*/

void entity_ScriptAddEntityProp(struct entity_handle_t entity, struct script_string_t *name, void *type_info)
{
	char *prop_name = script_string_GetRawString(name);
	int size;
	size = script_GetTypeSize(type_info);
	entity_AddProp(entity, prop_name, size);
}

void entity_ScriptAddEntityProp1i(struct entity_handle_t entity, struct script_string_t *name)
{
	char *prop_name = script_string_GetRawString(name);
	entity_AddProp(entity, prop_name, sizeof(int));
}

void entity_ScriptAddEntityProp1f(struct entity_handle_t entity, struct script_string_t *name)
{
	char *prop_name = script_string_GetRawString(name);
	entity_AddProp(entity, prop_name, sizeof(float));
}

void entity_ScriptAddEntityProp3f(struct entity_handle_t entity, struct script_string_t *name)
{
	char *prop_name = script_string_GetRawString(name);
	entity_AddProp(entity, prop_name, sizeof(vec3_t));
}



void entity_ScriptRemoveEntityProp(struct entity_handle_t entity, struct script_string_t *name)
{
	char *prop_name = script_string_GetRawString(name);
	entity_RemoveProp(entity, prop_name);
	
}

/*

void entity_ScriptRemoveEntityProp3f(struct entity_handle_t entity, struct script_string_t *name)
{
	char *prop_name = script_string_GetRawString(name);
	entity_RemoveProp(entity, prop_name);
}
*/



void entity_ScriptSetEntityPropValue1i(struct entity_handle_t entity, struct script_string_t *name, int value)
{
	char *prop_name = script_string_GetRawString(name);
	entity_SetProp(entity, prop_name, &value);
}

void entity_ScriptSetEntityPropValue1iv(struct entity_handle_t entity, struct script_string_t *name, void *value)
{
	char *prop_name = script_string_GetRawString(name);
	entity_SetProp(entity, prop_name, value);
}

int entity_ScriptGetEntityPropValue1i(struct entity_handle_t entity, struct script_string_t *name)
{
	char *prop_name = script_string_GetRawString(name);
	int value = 0;
	
	entity_GetProp(entity, prop_name, &value);
	
	return value;
}

void entity_ScriptGetEntityPropValue1iv(struct entity_handle_t entity, struct script_string_t *name, void *value)
{
	char *prop_name = script_string_GetRawString(name);
	entity_GetProp(entity, prop_name, value);
}

int entity_ScriptIncEntityPropValue1i(struct entity_handle_t entity, struct script_string_t *name)
{
	char *prop_name = script_string_GetRawString(name);
	struct entity_prop_t *prop = entity_GetPropPointer(entity, prop_name);
	
	if(prop)
	{
		(*(int *)prop->memory)++;
		return *(int *)prop->memory;
	}
	
	return 0;
}

int entity_ScriptDecEntityPropValue1i(struct entity_handle_t entity, struct script_string_t *name)
{
	char *prop_name = script_string_GetRawString(name);
	struct entity_prop_t *prop = entity_GetPropPointer(entity, prop_name);
	
	if(prop)
	{
		(*(int *)prop->memory)--;
		return *(int *)prop->memory;
	}
	
	return 0;
}




void entity_ScriptSetEntityPropValue3f(struct entity_handle_t entity, struct script_string_t *name, vec3_t *value)
{
	char *prop_name = script_string_GetRawString(name);
	entity_SetProp(entity, prop_name, value);
}


void *entity_ScriptGetEntityPropValue3f(struct entity_handle_t entity, struct script_string_t *name)
{
	char *prop_name = script_string_GetRawString(name);
	entity_GetProp(entity, prop_name, &vec3_ret);
	return &vec3_ret;
}

void entity_ScriptGetEntityPropValue3fv(struct entity_handle_t entity, struct script_string_t *name, vec3_t *value)
{
	char *prop_name = script_string_GetRawString(name);
	entity_GetProp(entity, prop_name, value);
}





void entity_ScriptSetEntityPropValue(struct entity_handle_t entity, struct script_string_t *name, void *value)
{
	char *prop_name = script_string_GetRawString(name);
	entity_SetProp(entity, prop_name, value);
}

void entity_ScriptGetEntityPropValue(struct entity_handle_t entity, struct script_string_t *name, void *value)
{
	char *prop_name = script_string_GetRawString(name);
	entity_GetProp(entity, prop_name, value);
}




int entity_ScriptEntityHasProp(struct entity_handle_t entity, struct script_string_t *name)
{
	char *prop_name = script_string_GetRawString(name);
	return entity_GetPropPointer(entity, prop_name) != NULL;
}


/*
=====================================
=====================================
=====================================
*/



void entity_ScriptSetCameraPosition(vec3_t *position)
{
	struct entity_t *entity;
	struct camera_component_t *camera_component;
	struct transform_component_t *transform_component;
	
	entity = entity_GetEntityPointerHandle(ent_current_entity);
	
	camera_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_CAMERA]);
	
	if(camera_component)
	{
		transform_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);
		transform_component->position = *position;
	}	
}


void entity_ScriptSetCamera(struct component_handle_t camera)
{
	struct camera_component_t *camera_component;
	
	if(camera.type == COMPONENT_TYPE_CAMERA)
	{
		camera_component = entity_GetComponentPointer(camera);
		
		if(camera_component)
		{
			camera_SetCamera(camera_component->camera);
		}
	}
}


/*
=====================================
=====================================
=====================================
*/

void entity_ScriptFindPath(vec3_t *to)
{
	entity_FindPath(ent_current_entity, *to);
}

void entity_ScriptGetWaypointDirection(vec3_t *direction)
{
	#if 0
	struct entity_t *entity_ptr;
	struct waypoint_t *waypoint;
	struct script_controller_component_t *controller;
	struct transform_component_t *transform;
	struct entity_transform_t *global_transform;
	vec3_t d = {0.0, 0.0, 0.0};
	
	float l;
	
	entity_ptr = entity_GetEntityPointerHandle(ent_current_entity);
	
	controller = entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_SCRIPT_CONTROLLER]);
	global_transform = entity_GetWorldTransformPointer(entity_ptr->components[COMPONENT_INDEX_TRANSFORM]);
	
	if(controller)
	{
		if(controller->controller.base.type == COMPONENT_TYPE_SCRIPT_CONTROLLER)
		{
			if(controller->route)
			{
				waypoint = controller->route[controller->current_waypoint];
				d.x = waypoint->position.x - global_transform->transform.floats[3][0];
				d.y = waypoint->position.y - global_transform->transform.floats[3][1];
				d.z = waypoint->position.z - global_transform->transform.floats[3][2];
				
				l = length3(d);
				d.x /= l;
				d.y /= l;
				d.z /= l;
				
				if(l < 1.5)
				{
					entity_ScriptAdvanceWaypoint();
				}	
			}
		}
	}
	
	
	//printf("entity: %d --- [%f %f %f]\n", ent_current_entity.entity_index, d.x, d.y, d.z);
	
	*direction = d;
	
	#endif
}

void entity_ScriptAdvanceWaypoint()
{
	#if 0
	struct entity_t *entity;
	struct script_controller_component_t *controller;
	
	entity = entity_GetEntityPointerHandle(ent_current_entity);
	controller = entity_GetComponentPointer(entity->components[COMPONENT_INDEX_SCRIPT_CONTROLLER]);
	
	if(controller->route_length)
	{
		if(controller->current_waypoint < controller->route_length - 1)
		{
			controller->current_waypoint++;
		}
	}
	
	#endif
	
}

void entity_ScriptPrint(struct script_string_t *script_string)
{
	printf(script_string_GetRawString(script_string));
}

#ifdef __cplusplus
}
#endif







