#include "phy_character.h"
#include "physics.h"
#include "r_debug.h"


#ifdef __cplusplus
extern "C++"
{
#endif // __cplusplus

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

#ifdef __cplusplus
}
#endif // __cplusplus

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
	struct character_collider_t *collider;
	btRigidBody *rigid_body;

	if(!world_collision_object)
	{
		/* don't allow any action upon character colliders
		if there isn't a world they can collide with... */
		return;
	}

	if(character_collider.type != COLLIDER_TYPE_CHARACTER_COLLIDER)
	{
		return;
	}

	collider = (struct character_collider_t *)physics_GetColliderPointer(character_collider);


	if(collider)
	{
		if(collider->flags & CHARACTER_COLLIDER_FLAG_ON_GROUND)
		{
			rigid_body = (btRigidBody *)collider->base.collision_object;
			rigid_body->activate(true);
			rigid_body->applyCentralImpulse(btVector3(0.0, jump_force, 0.0));
		}
	}
}

void physics_Move(struct collider_handle_t character_collider, vec3_t direction)
{
	struct character_collider_t *collider;
	btRigidBody *rigid_body;

	if(!world_collision_object)
	{
		/* don't allow any action upon character colliders
		if there isn't a world they can collide with... */
		return;
	}

	if(character_collider.type != COLLIDER_TYPE_CHARACTER_COLLIDER)
	{
        return;
	}

	collider = (struct character_collider_t *)physics_GetColliderPointer(character_collider);

	if(collider)
	{
		if(direction.x != 0.0 && direction.z != 0.0)
		{
			collider->flags |= CHARACTER_COLLIDER_FLAG_WALKING;
			rigid_body = (btRigidBody *)collider->base.collision_object;
			rigid_body->activate(true);
			rigid_body->applyCentralForce(btVector3(direction.x, 0.0, direction.z));
		}
	}
}

void physics_UpdateCharacterCollider(struct collider_handle_t character_collider)
{

	struct character_collider_t *collider;
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
	vec3_t contact_position;
	vec3_t normal;

	int i;
	int j;
	int c;
	int k;

	//int largest_hit_index = -1;
	//int smallest_hit_time_index;
	//float largest_hit_dist;
	//float smallest_hit_time = 0.0;
	//float smallest_hit_angle;

	btTransform rigid_body_transform;
	btVector3 rigid_body_position;

	float dist;
	float hit_time;
	btVector3 linear_velocity;
	btVector3 gravity_on_plane;
	btManifoldPoint contact_point;
//	btVector3 contact_position;
	btVector3 contact_normal;
	//btVector3 smallest_angle_contact_point;
	//btVector3 smallest_hit_normal;
	btVector3 e0;
	btVector3 e1;
	btVector3 *triangle;
	int triangle_index;

	PHY_ScalarType vert_type;
	int vert_count;
	int vert_stride;
	btVector3 *triangles;

	struct contact_record_t *contact_records;
	int contact_record_count;

	vec3_t *v3_triangles;


	PHY_ScalarType index_type;
	int index_count;
	int index_stride;
	int *indexes;

	float horizontal_delta;

	float contact_slope;

	float stop_force_weight;
	float gravity_proj;
	float angle_to_ground;

	int bottom_hit_index = -1;
	float bottom_hit_dist;
	float hit_point_y;


	btVector3 smallest_vertical_contact_normal;
	float smallest_vertical_contact_dist;

	btVector3 smallest_horizontal_contact_normal;
	float smallest_horizontal_contact_time;



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


	collider = (struct character_collider_t *)physics_GetColliderPointer(character_collider);

	rigid_body = (btRigidBody *)collider->base.collision_object;
	//capsule = (btCapsuleShape *)rigid_body->getCollisionShape();

	//to = btVector3(collider->position.x, collider->position.y - collider->step_height * 0.5, collider->position.z);
	//from = btVector3(collider->position.x, collider->position.y, collider->position.z);

	//AllConvexResultCallback result_callback(from, to, rigid_body);
//	ClosestNotMeConvexResultCallback closest_result_callback(from, to, rigid_body);
	//btCollisionWorld::ClosestConvexResultCallback result_callback(from, to);



	//transform_from.setIdentity();
	//transform_from.setOrigin(from);

	//transform_to.setIdentity();
	//transform_to.setOrigin(to);

//	physics_world->convexSweepTest((const btConvexShape *)rigid_body->getCollisionShape(), transform_from, transform_to, result_callback);
//	physics_world->convexSweepTest(capsule, transform_from, transform_to, closest_result_callback);


	//positions = result_callback.m_hitPointWorld;
	//normals = result_callback.m_hitNormalWorld;


	//largest_hit_dist = 0.0;
	//bottom_hit_dist = 0.0;
	//smallest_hit_time = 1.0;
	//smallest_hit_angle = 0.0;

	smallest_vertical_contact_dist = 2.0;
	smallest_horizontal_contact_time = 2.0;


	contact_records = physics_GetColliderContactRecords(character_collider);

//	c = result_callback.m_hitFractions.size();

	bottom_hit_dist = collider->height * 0.5;
	collider->flags &= ~CHARACTER_COLLIDER_FLAG_ON_GROUND;

	//world_collision_mesh->getMeshInterface()->getLockedVertexIndexBase((unsigned char **)&triangles, vert_count, vert_type, vert_stride, (unsigned char **)&indexes, index_stride, index_count, index_type, 0);

	//persistent_manifolds = narrow_phase->getInternalManifoldPointer();

	//if(persistent_manifolds)
	{
	//	persistent_manifolds_count = narrow_phase->getNumManifolds();

		for(i = 0; i < collider->base.contact_record_count; i++)
		{
			/* go over all contacts... */
			//persistent_manifold = persistent_manifolds[i];

			//if(persistent_manifold->getBody0() == rigid_body || persistent_manifold->getBody1() == rigid_body)
			{
				/* if this collider is involved in this contact point... */
				//c = persistent_manifold->getNumContacts();

				//for(j = 0; j < c; j++)
				{

					//point = contact_records[i].position;

					contact_position = contact_records[i].position;

					contact_normal[0] = contact_records[i].normal.x;
					contact_normal[1] = contact_records[i].normal.y;
					contact_normal[2] = contact_records[i].normal.z;

					//contact_point = persistent_manifold->getContactPoint(j);

					//point.x = contact_point.m_positionWorldOnA[0];
					//point.y = contact_point.m_positionWorldOnA[1];
					//point.z = contact_point.m_positionWorldOnA[2];

					//if(persistent_manifold->getBody0() == world_collision_object)
					//{
						//triangles = &world_triangles->m_3componentVertices[0];
					//	v3_triangles = (vec3_t *)&world_triangles->m_3componentVertices[0];
					//	triangle_index = contact_point.m_index0 * 3;
					//	collision_with_world = 1;
					//}
					//else if(persistent_manifold->getBody1() == world_collision_object)
					//{
						//triangles = &world_triangles->m_3componentVertices[0];
					//	v3_triangles = (vec3_t *)&world_triangles->m_3componentVertices[0];
					//	triangle_index = contact_point.m_index1 * 3;
					//	collision_with_world = 1;
					//}
					//else
					//{
					//	collision_with_world = 0;
					//}


					//if(collision_with_world)
					//{
						/* if the other collision object involved is the world,
						get the contact normal directly from the collision mesh... */
						//e0 = triangles[triangle_index + 1] - triangles[triangle_index];
						//e1 = triangles[triangle_index + 2] - triangles[triangle_index + 1];

					//	e0[0] = v3_triangles[triangle_index + 1].x - v3_triangles[triangle_index].x;
					//	e0[1] = v3_triangles[triangle_index + 1].y - v3_triangles[triangle_index].y;
					//	e0[2] = v3_triangles[triangle_index + 1].z - v3_triangles[triangle_index].z;

					//	e1[0] = v3_triangles[triangle_index + 2].x - v3_triangles[triangle_index + 1].x;
					//	e1[1] = v3_triangles[triangle_index + 2].y - v3_triangles[triangle_index + 1].y;
					//	e1[2] = v3_triangles[triangle_index + 2].z - v3_triangles[triangle_index + 1].z;

					//	contact_normal = e0.cross(e1);
					//	contact_normal = contact_normal.normalize();
					//}
					//else
					//{
					//	contact_normal = contact_point.m_normalWorldOnB;
					//}




					hit_point_y = contact_position.y;

					/* distance from the contact point to
					the middle of the collider... */
					dist = collider->base.position.y - hit_point_y;

					/* the smaller the hit time, the closer
					it is to the middle of the box */
					hit_time = (1.0 - (dist / bottom_hit_dist));

					renderer_DrawPoint(contact_position, vec3_t_c(0.0, 1.0, 0.0), 8.0, 1, 0, 1);
					renderer_DrawLine(contact_position, vec3_t_c(contact_position.x + contact_normal[0], contact_position.y + contact_normal[1], contact_position.z + contact_normal[2]), vec3_t_c(1.0, 1.0, 0.0), 1.0, 1);

					contact_slope = 1.0 - fabs(contact_normal[1]);

                    if(hit_time >= 0.0 && hit_time <= 1.0)
					{
						if(contact_slope > collider->max_slope_angle)
						{
							/* vertical-ish surface (step or wall)... */

							if(dist < smallest_vertical_contact_dist)
							{
								smallest_vertical_contact_dist = dist;
								smallest_vertical_contact_normal = contact_normal;
							}
						}
						else
						{
							/* walkable surface... */

							if(hit_time >= smallest_horizontal_contact_time - SIMD_EPSILON && hit_time <= smallest_horizontal_contact_time + SIMD_EPSILON)
							{
								/* if this contact has the same contact time as the smallest one so far... */

								if(contact_normal[1] > smallest_horizontal_contact_normal[1])
								{
									/* if this contact is with a more horizontal surface, keep
									it's normal... */

									smallest_horizontal_contact_normal = contact_normal;
									collider->flags |= CHARACTER_COLLIDER_FLAG_ON_GROUND;
								}
							}

							else if(hit_time < smallest_horizontal_contact_time)
							{
								/* this hit is closer to the bottom of the box... */
								smallest_horizontal_contact_time = hit_time;
								smallest_horizontal_contact_normal = contact_normal;
								collider->flags |= CHARACTER_COLLIDER_FLAG_ON_GROUND;
							}

						}
					}
				}

			}
		}

	}


	if(collider->flags & CHARACTER_COLLIDER_FLAG_ON_GROUND)
	{
		gravity_on_plane = -rigid_body->getGravity();

		gravity_proj = gravity_on_plane.dot(smallest_horizontal_contact_normal);

		gravity_on_plane = gravity_on_plane - smallest_horizontal_contact_normal * gravity_proj;

		rigid_body->applyCentralForce(gravity_on_plane);
	}
	else
	{
        //collider->character_collider_flags &= ~CHARACTER_COLLIDER_FLAG_STEPPING_UP;
	}

	if(collider->flags & CHARACTER_COLLIDER_FLAG_WALKING)
	{
		linear_velocity = rigid_body->getLinearVelocity();

		horizontal_delta = sqrt(linear_velocity[0] * linear_velocity[0] + linear_velocity[2] * linear_velocity[2]);

		if(horizontal_delta > collider->max_walk_speed)
		{
			horizontal_delta = collider->max_walk_speed / horizontal_delta;

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


    #if 0

	if(smallest_vertical_contact_dist < 2.0)
	{
		/* we hit a vertical surface... */

		//if(smallest_vertical_contact_dist <= collider->step_height)

		smallest_vertical_contact_dist = bottom_hit_dist - smallest_vertical_contact_dist;

		if(smallest_vertical_contact_dist <= 0.5 && smallest_vertical_contact_dist > 0.09)
		{
            /* we hit something climbable, so adjust the collider position */

            if(collider->character_collider_flags & CHARACTER_COLLIDER_FLAG_ON_GROUND)
			{
				/* to step up, we need to do it from somewhere. So, if we're touching a climbable
				step but aren't touching the ground, it means we're probably stepping down instead... */
				//rigid_body_position = rigid_body->getLinearVelocity();

				//collider->position.x -= smallest_vertical_contact_normal[0] * collider->radius;
				//collider->position.y += smallest_vertical_contact_dist;
				//collider->position.z -= smallest_vertical_contact_normal[2] * collider->radius;


				//rigid_body_position = rigid_body->getCenterOfMassPosition();

				linear_velocity = rigid_body->getLinearVelocity();



                //rigid_body_transform = rigid_body->getCenterOfMassTransform();
                rigid_body->getMotionState()->getWorldTransform(rigid_body_transform);

				rigid_body_position = rigid_body_transform.getOrigin();

				rigid_body_position[0] -= smallest_vertical_contact_normal[0] * collider->radius;
				rigid_body_position[1] += smallest_vertical_contact_dist;
				rigid_body_position[2] -= smallest_vertical_contact_normal[2] * collider->radius;

                rigid_body_transform.setOrigin(rigid_body_position);
                rigid_body->getMotionState()->setWorldTransform(rigid_body_transform);

				//rigid_body->setCe().setOrigin(rigid_body_position);

				rigid_body->setLinearVelocity(btVector3(collider->linear_velocity.x, 0.0, collider->linear_velocity.z));
				rigid_body->clearForces();

				collider->character_collider_flags |= CHARACTER_COLLIDER_FLAG_STEPPING_UP;

				//collider->flags |= COLLIDER_FLAG_UPDATE_RIGID_BODY;
			}
		}
	}

	#endif



	collider->flags &= ~CHARACTER_COLLIDER_FLAG_WALKING;

	//world_collision_mesh->getMeshInterface()->unLockVertexBase(0);
}







#ifdef __cplusplus
}
#endif
