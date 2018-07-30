#ifndef PHYSICS_INL
#define PHYSICS_INL

#include "phy_common.h"
#include "physics.h"
#include "stack_list.h"
#include "list.h"


extern struct list_t phy_collision_records;
extern struct stack_list_t phy_colliders[COLLIDER_TYPE_LAST];


/*struct collider_t *physics_GetColliderPointerHandle(struct collider_handle_t collider)
{
	struct stack_list_t *list;
	struct collider_t *collider_ptr;
	
	if(collider.type != COLLIDER_TYPE_NONE)
	{
		list = &phy_colliders[collider.type];
	
		if(collider.index >= 0 && collider.index < list->element_count)
		{
			collider_ptr = (struct collider_t *)list->elements + collider.index;
			if(!(collider_ptr->flags & COLLIDER_FLAG_INVALID))
			{
				return collider_ptr;
			}
		}
	}
	return NULL;
}*/



/*struct collision_record_t *physics_GetColliderCollisionRecords(struct collider_handle_t collider)
{
	struct collider_t *collider_ptr;
	
	collider_ptr = physics_GetColliderPointerHandle(collider);
	
	if(collider_ptr)
	{
		if(collider_ptr->collision_record_count)
		{
			return (struct collision_record_t *)phy_collision_records.elements + collider_ptr->first_collision_record;
		}
	}
	
	return NULL;
}*/

#endif
