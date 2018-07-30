#include "phy_projectile.h"
#include "physics.h"

#include <stdio.h>
#include <math.h>

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btCylinderShape.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletCollision/CollisionShapes/btCompoundShape.h"


extern btCollisionDispatcher *narrow_phase;

#ifdef __cplusplus
extern "C"
{
#endif



void physics_UpdateProjectileCollider(struct collider_handle_t collider)
{
	struct collider_t *collider_ptr;
	btRigidBody *rigid_body;
	
	int persistent_manifolds_count;
	btPersistentManifold **persistent_manifolds;
	btPersistentManifold *persistent_manifold;
	
	btVector3 velocity;
	float speed;
	
	if(collider.type != COLLIDER_TYPE_PROJECTILE_COLLIDER)
	{
		return;
	}
	
	collider_ptr = physics_GetColliderPointerHandle(collider);
	
	rigid_body = (btRigidBody *)collider_ptr->rigid_body;
	
	velocity = rigid_body->getLinearVelocity();
	
	speed = sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2]);
	
	if(speed >= collider_ptr->radius)
	{
		
	}
}


#ifdef __cplusplus
}
#endif