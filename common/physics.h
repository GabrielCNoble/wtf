#ifndef PHYSICS_H
#define PHYSICS_H

#include "phy_common.h"
#include "phy_character.h"



#ifdef __cplusplus
extern "C"
{
#endif

int physics_Init();

void physics_Finish();

void physics_ProcessCollisions(double delta_time);

/*
=================================================================
=================================================================
=================================================================
*/


collider_def_t *physics_CreateColliderDef(char *name);

collider_def_t *physics_CreateCharacterColliderDef(char *name, float height, float crouch_height, float radius, float step_height, float slope_angle);

void physics_DestroyColliderDef(char *name);

void physics_DestroyColliderDefPointer(collider_def_t *def);

void physics_DestroyColliderDefs();

collider_def_t *physics_GetColliderDefPointer(char *name);

collider_def_t *physics_GetColliderDefPointerIndex(int collider_def_index);

void physics_AddCollisionShape(collider_def_t *def, vec3_t scale, vec3_t relative_position, mat3_t *relative_orientation, int type);

void physics_RemoveCollisionShape(collider_def_t *def, int shape_index);

void physics_TranslateCollisionShape(collider_def_t *def, vec3_t translation, int shape_index);

void physics_RotateCollisionShape(collider_def_t *def, vec3_t axis, float amount, int shape_index);

void physics_ScaleCollisionShape(collider_def_t *def, vec3_t scale, int shape_index);

void *physics_BuildCollisionShape(collider_def_t *def);

void physics_IncColliderDefRefCount(collider_def_t *def);

void physics_DecColliderDefRefCount(collider_def_t *def);

void physics_UpdateReferencingColliders(collider_def_t *def);

void physics_DestroyCollisionShape(void *collision_shape);


/*
=================================================================
=================================================================
=================================================================
*/


int physics_CreateEmptyCollider();

int physics_CreateCollider(mat3_t *orientation, vec3_t position, vec3_t scale, collider_def_t *def, int flags);

int physics_CopyCollider(int collider_index);

void physics_DestroyColliderIndex(int collider_index);

collider_t *physics_GetColliderPointerIndex(int collider_index);

void physics_GetColliderAabb(int collider_index, vec3_t *aabb);

/*
=================================================================
=================================================================
=================================================================
*/

void physics_SetColliderPosition(int collider_index, vec3_t position);

void physics_SetColliderOrientation(int collider_index, mat3_t *orientation);

void physics_SetColliderScale(int collider_index, vec3_t scale);

/*
=================================================================
=================================================================
=================================================================
*/

void physics_UpdateColliders();

void physics_PostUpdateColliders();

/*
=================================================================
=================================================================
=================================================================
*/



/*
=================================================================
=================================================================
=================================================================
*/

int physics_Raycast(vec3_t from, vec3_t to, vec3_t *hit_position, vec3_t *hit_normal);

/*
=================================================================
=================================================================
=================================================================
*/

void physics_ClearWorldCollisionMesh();

void physics_BuildWorldCollisionMesh();

#ifdef __cplusplus
}
#endif



#endif








