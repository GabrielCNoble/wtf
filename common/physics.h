#ifndef PHYSICS_H
#define PHYSICS_H

#include "phy_common.h"
#include "phy_character.h"
#include "phy_projectile.h"



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


struct collider_handle_t physics_CreateColliderDef(char *name);

struct collider_handle_t physics_CreateRigidBodyColliderDef(char *name);

struct collider_handle_t physics_CreateCharacterColliderDef(char *name, float height, float crouch_height, float radius, float step_height, float slope_angle, float max_walk_speed, float mass);

struct collider_handle_t physics_CreateProjectileColliderDef(char *name, float radius, float mass);



void physics_DestroyColliderDefByName(char *name);

void physics_DestroyColliderDef(struct collider_handle_t def);

//void physics_DestroyColliderDefPointer(struct collider_def_t *def);

void physics_DestroyColliderDefs();

struct collider_handle_t physics_GetColliderDefByName(char *name);

struct collider_def_t *physics_GetColliderDefPointer(char *name);

struct collider_def_t *physics_GetColliderDefPointerHandle(struct collider_handle_t def);

struct collider_def_t *physics_GetColliderDefsList(int *def_count);

//truct collider_def_t *physics_GetColliderDefPointerIndex(int collider_def_index);

void physics_AddCollisionShape(struct collider_handle_t def_handle, vec3_t scale, vec3_t relative_position, mat3_t *relative_orientation, int type);

void physics_RemoveCollisionShape(struct collider_handle_t def_handle, int shape_index);

struct collision_shape_t *physics_GetCollisionShapePointer(struct collider_handle_t collider, int shape_index);

void physics_TranslateCollisionShape(struct collider_handle_t def_handle, vec3_t translation, int shape_index);

void physics_SetCollisionShapePosition(struct collider_handle_t def_handle, vec3_t position, int shape_index);

void physics_RotateCollisionShape(struct collider_handle_t def_handle, vec3_t axis, float amount, int shape_index);

void physics_SetCollisionShapeOrientation(struct collider_handle_t def_handle, mat3_t *orientation, int shape_index);

void physics_ScaleCollisionShape(struct collider_handle_t def_handle, vec3_t scale, int shape_index);

void physics_SetCollisionShapeScale(struct collider_handle_t def_handle, vec3_t scale, int shape_index);


void *physics_BuildCollisionShape(struct collider_handle_t def_handle);

void physics_IncColliderDefRefCount(struct collider_handle_t def_handle);

void physics_DecColliderDefRefCount(struct collider_handle_t def_handle);

void physics_UpdateReferencingColliders(struct collider_handle_t def_handle);

void physics_DestroyCollisionShape(void *collision_shape);


/*
=================================================================
=================================================================
=================================================================
*/


struct collider_handle_t physics_CreateEmptyCollider(int type);

//struct collider_handle_t physics_CreateCollider(mat3_t *orientation, vec3_t position, vec3_t scale, struct collider_def_t *def, int flags);

struct collider_handle_t physics_CreateCollider(mat3_t *orientation, vec3_t position, vec3_t scale, struct collider_handle_t def_handle, int flags);

struct collider_handle_t physics_CreateTrigger(mat3_t *orientation, vec3_t position, vec3_t scale);

int physics_CopyCollider(int collider_index);

void physics_DestroyCollider(struct collider_handle_t collider);

struct collider_t *physics_GetColliderPointer(struct collider_handle_t collider);

void physics_GetColliderAabb(struct collider_handle_t collider, vec3_t *aabb);

/*
=================================================================
=================================================================
=================================================================
*/

void physics_SetColliderPosition(struct collider_handle_t collider, vec3_t position);

void physics_SetColliderOrientation(struct collider_handle_t collider, mat3_t *orientation);

void physics_SetColliderScale(struct collider_handle_t collider, vec3_t scale);

void physics_SetColliderVelocity(struct collider_handle_t collider, vec3_t velocity);

void physics_SetColliderMass(struct collider_handle_t collider, float mass);

void physics_SetColliderStatic(struct collider_handle_t collider, int set);

void physics_ApplyCentralForce(struct collider_handle_t collider, vec3_t force);

void physics_ApplyCentralImpulse(struct collider_handle_t collider, vec3_t impulse);





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

int physics_AreColliding(struct collider_handle_t collider_a, struct collider_handle_t collider_b);

int physics_HasNewCollisions(struct collider_handle_t collider);

struct contact_record_t *physics_GetColliderContactRecords(struct collider_handle_t collider);

/*
=================================================================
=================================================================
=================================================================
*/

int physics_Raycast(vec3_t from, vec3_t to, vec3_t *hit_position, vec3_t *hit_normal, int world_only);

//int physics_RaycastFromCollider(vec3)

/*
=================================================================
=================================================================
=================================================================
*/

void physics_ClearWorldCollisionMesh();

void physics_BuildWorldCollisionMesh();



#include "physics.inl"



#ifdef __cplusplus
}
#endif



#endif








