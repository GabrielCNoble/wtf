#include "phy_character.h"
#include "physics.h"
#include "r_debug.h"

extern "C++"
{

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btCylinderShape.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletCollision/CollisionShapes/btCompoundShape.h"

extern btDiscreteDynamicsWorld *physics_world;
extern btCollisionObject *world_collision_object;
extern btCollisionDispatcher *narrow_phase;
extern btBvhTriangleMeshShape *world_collision_mesh;
extern btTriangleMesh *world_triangles;

}

#ifdef __cplusplus
extern "C"
{
#endif



struct ClosestNotMeConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
	ClosestNotMeConvexResultCallback(const btVector3 &convexFromWorld, const btVector3 &convexToWorld, const btCollisionObject *caller = NULL)
	: btCollisionWorld::ClosestConvexResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0))
	{	
		m_convexFromWorld = convexFromWorld;
		m_convexToWorld = convexToWorld;	
		m_caller = caller;
		m_hitFraction = 1.0;
	}
		
	const btCollisionObject *m_caller;
	btScalar m_hitFraction;	
		
	virtual	btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult,bool normalInWorldSpace)
	{
		if(m_caller)
		{
			if(m_caller == convexResult.m_hitCollisionObject)
			{
				/* reject results where the collision object 
				collides with itself... */
				return btScalar(1.0);
			}
		}
		
		if(convexResult.m_hitFraction < m_hitFraction)
		{
			m_hitFraction = convexResult.m_hitFraction;
			return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
		}
		
		return btScalar(1.0);
	}
};



struct AllConvexResultCallback : public btCollisionWorld::ConvexResultCallback
	{
		AllConvexResultCallback(const btVector3 &convexFromWorld, const btVector3 &convexToWorld, const btCollisionObject *caller = NULL)
		{
			
			m_convexFromWorld = convexFromWorld;
			m_convexToWorld = convexToWorld;
			m_caller = caller;
			
			m_collisionObjects.resize(0);
			m_hitNormalWorld.resize(0);
			m_hitPointWorld.resize(0);
			m_hitFractions.resize(0);
		}
		
		btAlignedObjectArray<const btCollisionObject*>		m_collisionObjects;

		btVector3	m_convexFromWorld; //used to calculate hitPointWorld from hitFraction
		btVector3	m_convexToWorld;
		
		const btCollisionObject *m_caller;

		btAlignedObjectArray<btVector3>	m_hitNormalWorld;
		btAlignedObjectArray<btVector3>	m_hitPointWorld;
		btAlignedObjectArray<btScalar> m_hitFractions;
		
		
		virtual	btScalar	addSingleResult(btCollisionWorld::LocalConvexResult& convexResult,bool normalInWorldSpace)
		{
			if(m_caller)
			{
				if(m_caller == convexResult.m_hitCollisionObject)
				{
					/* reject results where the collision object 
					collides with itself... */
					return btScalar(1.0);
				}
			}
			
			m_collisionObjects.push_back(convexResult.m_hitCollisionObject);
			m_hitFractions.push_back(convexResult.m_hitFraction);
						
			if (normalInWorldSpace)
			{
				m_hitNormalWorld.push_back(convexResult.m_hitNormalLocal);
			} 
			
			else
			{
				///need to transform normal into worldspace
				m_hitNormalWorld.push_back(convexResult.m_hitCollisionObject->getWorldTransform().getBasis()*convexResult.m_hitNormalLocal);
			}
			m_hitPointWorld.push_back(convexResult.m_hitPointLocal);
			
			return convexResult.m_hitFraction;
			
		}
		
	};



void physics_Jump(struct collider_handle_t character_collider, float jump_force)
{
	struct collider_t *collider;
	btRigidBody *rigid_body;
	
	if(!world_collision_object)
	{
		/* don't allow any action upon character colliders
		if there isn't a world they can collide with... */
		return;
	}
	
	collider = physics_GetColliderPointerHandle(character_collider);
	
	if(collider->type != COLLIDER_TYPE_CHARACTER_COLLIDER)
	{
		return;
	}
	
	if(collider->character_collider_flags & CHARACTER_COLLIDER_FLAG_ON_GROUND)
	{
		rigid_body = (btRigidBody *)collider->rigid_body;
		rigid_body->activate(true);
		rigid_body->applyCentralImpulse(btVector3(0.0, jump_force, 0.0));
		//rigid_body->applyCentralForce(btVector3(0.0, jump_force, 0.0));
	}
}

void physics_Move(struct collider_handle_t character_collider, vec3_t direction)
{
	struct collider_t *collider;
	btRigidBody *rigid_body;
	
	if(!world_collision_object)
	{
		/* don't allow any action upon character colliders
		if there isn't a world they can collide with... */
		return;
	}
	
	collider = physics_GetColliderPointerHandle(character_collider);
	
	if(collider->type != COLLIDER_TYPE_CHARACTER_COLLIDER)
	{
		return;
	}
	
	if(direction.x != 0.0 && direction.z != 0.0)
	{
		collider->character_collider_flags |= CHARACTER_COLLIDER_FLAG_WALKING;
		rigid_body = (btRigidBody *)collider->rigid_body;
		rigid_body->activate(true);
		rigid_body->applyCentralForce(btVector3(direction.x, 0.0, direction.z));
	}
	
		
}

void physics_UpdateCharacterCollider(struct collider_handle_t character_collider)
{
	
	struct collider_t *collider;
	btRigidBody *rigid_body;
	btCapsuleShape *capsule;
	btTransform transform_from;
	btTransform transform_to;
	btVector3 from;
	btVector3 to;
	
	struct collider_handle_t handle;
	
	int persistent_manifolds_count;
	btPersistentManifold **persistent_manifolds;
	btPersistentManifold *persistent_manifold;
	
	//btAlignedObjectArray *normals;
	//btAlignedObjectArray *positions;
	
	vec3_t point;
	vec3_t normal;
	
	int i;
	int j;
	int c;
	int k;

	int largest_hit_index = -1;
	int smallest_hit_time_index;
	float largest_hit_dist;
	float smallest_hit_time = 0.0;
	float smallest_hit_angle;
	
	float dist;
	float hit_time;
	btVector3 linear_velocity;
	btVector3 gravity_on_plane;
	btManifoldPoint contact_point;
	btVector3 contact_position;
	btVector3 contact_normal;
	btVector3 smallest_angle_contact_point;
	btVector3 smallest_hit_normal;
	btVector3 e0;
	btVector3 e1;
	btVector3 *triangle;
	int triangle_index;
	
	PHY_ScalarType vert_type;
	int vert_count;
	int vert_stride;
	btVector3 *triangles;
	
	
	PHY_ScalarType index_type;
	int index_count;
	int index_stride;
	int *indexes;
	
	float horizontal_delta;
	
	float stop_force_weight;
	float gravity_proj;
	float angle_to_ground;
	
	int bottom_hit_index = -1;
	float bottom_hit_dist;
	float hit_point_y;
	
	int collision_with_world;
	
	if(!world_collision_object)
	{
		/* don't allow any action upon character colliders
		if there isn't a world they can collide with... */
		return;
	}
	
	/*handle.index = character_collider_index;
	handle.type = COLLIDER_TYPE_CHARACTER_COLLIDER;
	
	collider = physics_GetColliderPointerHandle(handle);
	
	if(collider->type != COLLIDER_TYPE_CHARACTER_COLLIDER)
	{
		return;
	}*/
	
	if(character_collider.type != COLLIDER_TYPE_CHARACTER_COLLIDER)
	{
		return;
	}
	
	
	collider = physics_GetColliderPointerHandle(character_collider);
	
	rigid_body = (btRigidBody *)collider->rigid_body;
	capsule = (btCapsuleShape *)rigid_body->getCollisionShape();
	
	to = btVector3(collider->position.x, collider->position.y - collider->step_height * 0.5, collider->position.z);
	from = btVector3(collider->position.x, collider->position.y, collider->position.z);
	
	AllConvexResultCallback result_callback(from, to, rigid_body);
//	ClosestNotMeConvexResultCallback closest_result_callback(from, to, rigid_body);
	//btCollisionWorld::ClosestConvexResultCallback result_callback(from, to);
	
	
	
	transform_from.setIdentity();
	transform_from.setOrigin(from);
	
	transform_to.setIdentity();
	transform_to.setOrigin(to);
	
	physics_world->convexSweepTest(capsule, transform_from, transform_to, result_callback);
//	physics_world->convexSweepTest(capsule, transform_from, transform_to, closest_result_callback);
	
	
	//positions = result_callback.m_hitPointWorld;
	//normals = result_callback.m_hitNormalWorld;
	
	
	largest_hit_dist = 0.0;
	bottom_hit_dist = 0.0;
	smallest_hit_time = 1.0;
	smallest_hit_angle = 0.0;

	c = result_callback.m_hitFractions.size();
	
	bottom_hit_dist = collider->height * 0.5;
	collider->character_collider_flags &= ~CHARACTER_COLLIDER_FLAG_ON_GROUND;
	
	world_collision_mesh->getMeshInterface()->getLockedVertexIndexBase((unsigned char **)&triangles, vert_count, vert_type, vert_stride, (unsigned char **)&indexes, index_stride, index_count, index_type, 0);
	
	#if 0
	
	for(i = 0; i < c; i++)
	{
		point.x = result_callback.m_hitPointWorld[i][0];
		point.y = result_callback.m_hitPointWorld[i][1];
		point.z = result_callback.m_hitPointWorld[i][2];
		
		//renderer_DrawPoint(point, vec3(0.0, 1.0, 0.0), 8.0, 1, 0, 1);
		
		//renderer_DrawLine(point, vec3(point.x + result_callback.m_hitNormalWorld[i][0], point.y + result_callback.m_hitNormalWorld[i][1], point.z + result_callback.m_hitNormalWorld[i][2]), vec3(1.0, 1.0, 0.0), 1.0, 1);
		
		hit_point_y = result_callback.m_hitPointWorld[i][1];
						
		dist = collider->position.y - hit_point_y;
		hit_time = (1.0 - (dist / bottom_hit_dist));
		
		if(hit_time <= 1.0 && hit_time >= 0.0)
		{
			dist = bottom_hit_dist * hit_time;	
			//printf("%f %f\n", dist, result_callback.m_hitNormalWorld[i][1]);
			
			if(result_callback.m_hitNormalWorld[i][1] < collider->max_slope && result_callback.m_hitNormalWorld[i][1] > -collider->max_slope)
			{					
				if(dist > largest_hit_dist)
				{
					largest_hit_dist = dist;
				}
			}
			else
			{
				if(hit_time <= smallest_hit_time)
				{
					if(hit_time >= smallest_hit_time - SIMD_EPSILON && hit_time <= smallest_hit_time + SIMD_EPSILON)
					{
						if(result_callback.m_hitNormalWorld[i][1] > smallest_hit_angle)
						{
							smallest_hit_time = hit_time;
							smallest_hit_time_index = i;
							smallest_hit_angle = result_callback.m_hitNormalWorld[i][1];
						}
					}
					else
					{
						smallest_hit_time = hit_time;
						smallest_hit_time_index = i;
						smallest_hit_angle = result_callback.m_hitNormalWorld[i][1];
						collider->character_collider_flags |= CHARACTER_COLLIDER_FLAG_ON_GROUND;
					}
					
					
				}
				
			}
		}
	}
	
	#endif
	
	persistent_manifolds = narrow_phase->getInternalManifoldPointer();
	
	if(persistent_manifolds)
	{
		persistent_manifolds_count = narrow_phase->getNumManifolds();
		
		for(i = 0; i < persistent_manifolds_count; i++)
		{
			persistent_manifold = persistent_manifolds[i];
			
			if(persistent_manifold->getBody0() == rigid_body || persistent_manifold->getBody1() == rigid_body)
			{
				c = persistent_manifold->getNumContacts();
				
				for(j = 0; j < c; j++)
				{
					contact_point = persistent_manifold->getContactPoint(j);
					
					point.x = contact_point.m_positionWorldOnA[0];
					point.y = contact_point.m_positionWorldOnA[1];
					point.z = contact_point.m_positionWorldOnA[2];
					
					
					if(persistent_manifold->getBody0() == world_collision_object)
					{
						triangles = &world_triangles->m_4componentVertices[0];
						triangle_index = contact_point.m_index0 * 3;
						collision_with_world = 1;
					}
					else if(persistent_manifold->getBody1() == world_collision_object)
					{
						triangles = &world_triangles->m_4componentVertices[0];
						triangle_index = contact_point.m_index1 * 3;
						collision_with_world = 1;
					}
					else
					{
						collision_with_world = 0;
					}
					
					
					if(collision_with_world)
					{
						e0 = triangles[triangle_index + 1] - triangles[triangle_index];
						e1 = triangles[triangle_index + 2] - triangles[triangle_index + 1];

						contact_normal = e0.cross(e1);
						contact_normal = contact_normal.normalize();
					}
					else
					{
						contact_normal = contact_point.m_normalWorldOnB;
					}
							
					hit_point_y = point.y;
									
					dist = collider->position.y - hit_point_y;
					hit_time = (1.0 - (dist / bottom_hit_dist));
					
					renderer_DrawPoint(point, vec3_t_c(0.0, 1.0, 0.0), 8.0, 1, 0, 1);
					renderer_DrawLine(point, vec3_t_c(point.x + contact_point.m_normalWorldOnB[0], point.y + contact_point.m_normalWorldOnB[1], point.z + contact_point.m_normalWorldOnB[2]), vec3_t_c(1.0, 1.0, 0.0), 1.0, 1);
					
					
					if(hit_time <= 1.0 && hit_time >= 0.0)
					{
						dist = bottom_hit_dist * hit_time;	
				
						if(contact_normal[1] < collider->max_slope && contact_normal[1] > -collider->max_slope)
						{					
							if(dist > largest_hit_dist)
							{
								largest_hit_dist = dist;
							}
						}
						else
						{
							if(hit_time <= smallest_hit_time)
							{
								if(hit_time >= smallest_hit_time - SIMD_EPSILON && hit_time <= smallest_hit_time + SIMD_EPSILON)
								{
									if(contact_normal[1] > smallest_hit_angle)
									{
										smallest_hit_time = hit_time;
										smallest_hit_time_index = j;
										smallest_hit_angle = contact_normal[1];
										smallest_hit_normal = contact_normal;
										collider->character_collider_flags |= CHARACTER_COLLIDER_FLAG_ON_GROUND;
									}
								}
								else
								{
									smallest_hit_time = hit_time;
									smallest_hit_time_index = i;
									smallest_hit_angle = contact_normal[1];
									smallest_hit_normal = contact_normal;
									collider->character_collider_flags |= CHARACTER_COLLIDER_FLAG_ON_GROUND;
								}
							}
							
						}
					}
					
						
												
				}
				
			}
		}
		
	}
	
	
	if(collider->character_collider_flags & CHARACTER_COLLIDER_FLAG_ON_GROUND)
	{		
		gravity_on_plane = -rigid_body->getGravity();
		
		gravity_proj = gravity_on_plane.dot(smallest_hit_normal);
		
		gravity_on_plane = gravity_on_plane - smallest_hit_normal * gravity_proj;

		rigid_body->applyCentralForce(gravity_on_plane);
	} 
	
	if(collider->character_collider_flags & CHARACTER_COLLIDER_FLAG_WALKING)
	{
		linear_velocity = rigid_body->getLinearVelocity();
		
		horizontal_delta = sqrt(linear_velocity[0] * linear_velocity[0] + linear_velocity[2] * linear_velocity[2]);
		
		if(horizontal_delta > MAX_HORIZONTAL_DELTA)
		{
			horizontal_delta = MAX_HORIZONTAL_DELTA / horizontal_delta;
			
			linear_velocity[0] *= horizontal_delta;
			linear_velocity[2] *= horizontal_delta;
			
			rigid_body->setLinearVelocity(linear_velocity);
		}
		
	}
	else 
	{
		//if(collider->character_collider_flags & CHARACTER_COLLIDER_FLAG_ON_GROUND)
		{
			linear_velocity = rigid_body->getLinearVelocity();
			linear_velocity[0] *= 4.0;
			linear_velocity[1] = 0.0;
			linear_velocity[2] *= 4.0;
			rigid_body->applyCentralForce(-linear_velocity);
			//rigid_body->setLinearVelocity(linear_velocity);
		}
	}
	
	collider->character_collider_flags &= ~CHARACTER_COLLIDER_FLAG_WALKING;
	
	world_collision_mesh->getMeshInterface()->unLockVertexBase(0);	
}







#ifdef __cplusplus
}
}
}
#endif
