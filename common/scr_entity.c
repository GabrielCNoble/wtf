#include "scr_entity.h"
#include "entity.h"
#include "physics.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* this allows hiding the current entity's reference
at the cost of not allowing concurrent scripts... */
extern struct entity_handle_t ent_current_entity;

static vec3_t vec3_ret;
static mat3_t mat3_ret;

/*
=====================================
=====================================
=====================================
*/

void entity_ScriptMove(vec3_t *direction)
{
	struct script_controller_component_t *controller;
	struct entity_t *entity_ptr;
		
	entity_ptr = entity_GetEntityPointerIndex(ent_current_entity);
	
	controller = entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_SCRIPT_CONTROLLER]);
	
	if(controller)
	{
		if(controller->controller.base.type == COMPONENT_TYPE_SCRIPT_CONTROLLER)
		{
			physics_Move(controller->controller.collider.collider_index, *direction);
		}
	}
}

void entity_ScriptJump(float jump_force)
{
	struct script_controller_component_t *controller;
	struct entity_t *entity_ptr;
	
	entity_ptr = entity_GetEntityPointerIndex(ent_current_entity);
	
	controller = entity_GetComponentPointer(entity_ptr->components[COMPONENT_INDEX_SCRIPT_CONTROLLER]);
	
	if(controller)
	{
		if(controller->controller.base.type == COMPONENT_TYPE_SCRIPT_CONTROLLER)
		{
			physics_Jump(controller->controller.collider.collider_index, jump_force);
		}
	}
}

/*
=====================================
=====================================
=====================================
*/

void *entity_ScriptGetPosition(struct entity_handle_t entity)
{
	
}

void *entity_ScriptGetOrientation()
{
	struct entity_t *entity;
	struct transform_component_t *transform;
	
	entity = entity_GetEntityPointerIndex(ent_current_entity);
	transform = entity_GetComponentPointer(entity->components[COMPONENT_INDEX_TRANSFORM]);
	
	mat3_ret = transform->orientation;
	
	return &mat3_ret;
}

void *entity_ScriptGetForwardVector()
{
	struct entity_t *entity;
	struct transform_component_t *transform;
	
	entity = entity_GetEntityPointerIndex(ent_current_entity);
	transform = entity_GetComponentPointer(entity->components[COMPONENT_INDEX_TRANSFORM]);
	
	return &transform->orientation.r2;
}

struct component_handle_t entity_ScriptGetCurrentComponent(int component_index)
{
	return entity_ScriptGetComponent(ent_current_entity, component_index);
}

struct component_handle_t entity_ScriptGetComponent(struct entity_handle_t entity, int component_index)
{
	struct entity_t *entity_ptr;
	entity_ptr = entity_GetEntityPointerIndex(entity);
	return entity_ptr->components[component_index];
}

void entity_ScriptRotate(vec3_t *axis, float angle, int set)
{
	struct entity_t *entity;
	struct transform_component_t *transform;
	
	entity = entity_GetEntityPointerIndex(ent_current_entity);
	transform = entity_GetComponentPointer(entity->components[COMPONENT_INDEX_TRANSFORM]);
	
	set = set && 1;
	
	mat3_t_rotate(&transform->orientation, *axis, angle, set);
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
	struct entity_t *entity_ptr;
	struct waypoint_t *waypoint;
	struct script_controller_component_t *controller;
	struct transform_component_t *transform;
	struct entity_transform_t *global_transform;
	vec3_t d = {0.0, 0.0, 0.0};
	
	float l;
	
	entity_ptr = entity_GetEntityPointerIndex(ent_current_entity);
	
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
}

void entity_ScriptAdvanceWaypoint()
{
	struct entity_t *entity;
	struct script_controller_component_t *controller;
	
	entity = entity_GetEntityPointerIndex(ent_current_entity);
	controller = entity_GetComponentPointer(entity->components[COMPONENT_INDEX_SCRIPT_CONTROLLER]);
	
	if(controller->route_length)
	{
		if(controller->current_waypoint < controller->route_length - 1)
		{
			controller->current_waypoint++;
		}
	}
	
}

#ifdef __cplusplus
}
#endif







