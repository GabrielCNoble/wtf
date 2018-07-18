#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <intrin.h>

#include "GL\glew.h"

#include "physics.h"
#include "bsp.h"
#include "gpu.h"
#include "memory.h"
#include "matrix.h"

#include "stack_list.h"


#include "camera.h"


/* from world.c */
extern int w_world_vertices_count;
extern vertex_t *w_world_vertices;


//extern int player_count;
//extern player_t *players;
//extern bsp_pnode_t *collision_nodes;


int collision_def_count = 0;
collider_def_t *collider_defs = NULL;
collider_def_t *last_collider_def = NULL;


//int collider_list_cursor = 0;
//int max_colliders = 0;
//collider_t *colliders = NULL;
//int colliders_free_positions_stack_top = -1; 
//int *colliders_free_positions_stack = NULL;

#define EXTRA_MARGIN 0.01

extern int world_hull_node_count;
extern bsp_pnode_t *world_hull;


struct stack_list_t phy_colliders[COLLIDER_TYPE_LAST]; 


/* I'm not sure why this works, but it does... */
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
	
	btDefaultCollisionConfiguration *collision_configuration = NULL;
	btCollisionDispatcher *narrow_phase = NULL;
	btBroadphaseInterface *broad_phase = NULL;
	btSequentialImpulseConstraintSolver *solver = NULL;
	btDiscreteDynamicsWorld *physics_world = NULL;
	
	btCollisionObject *world_collision_object = NULL;
	btBvhTriangleMeshShape *world_collision_mesh = NULL;
	btTriangleMesh *world_triangles = NULL;
}


#ifdef __cplusplus
extern "C"
{
#endif



int physics_Init()
{	
	int i;
	collision_configuration = new btDefaultCollisionConfiguration();
	narrow_phase = new btCollisionDispatcher(collision_configuration);
	broad_phase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver;
	physics_world = new btDiscreteDynamicsWorld(narrow_phase, broad_phase, solver, collision_configuration);
	physics_world->setGravity(btVector3(0.0, -GRAVITY * 1000.0, 0.0));
	
	//max_colliders = 128;
	//colliders = (collider_t *) memory_Malloc(sizeof(collider_t ) * max_colliders, "physics_Init");
	//colliders_free_positions_stack = (int *) memory_Malloc(sizeof(int) * max_colliders, "physics_Init");
	
	
	for(i = 0; i < COLLIDER_TYPE_LAST; i++)
	{
		phy_colliders[i] = stack_list_create(sizeof(collider_t), 64, NULL);
	}
	
	
	
	
	return 1;
}

void physics_Finish()
{
	int i;
	btRigidBody *collider_rigid_body;
		
	
	
	while(collider_defs)
	{
		last_collider_def = collider_defs->next;
		if(collider_defs->type == COLLIDER_TYPE_RIGID_BODY_COLLIDER)
		{
			memory_Free(collider_defs->collision_shape);
		}
		
		memory_Free(collider_defs->name);
		memory_Free(collider_defs);
		collider_defs = last_collider_def;
	}
	
	/*for(i = 0; i < collider_list_cursor; i++)
	{
		if(colliders[i].flags & COLLIDER_INVALID)
			continue;
		
		collider_rigid_body = (btRigidBody *)colliders[i].rigid_body;
		physics_DestroyCollisionShape(collider_rigid_body->getCollisionShape());
		physics_world->removeRigidBody(collider_rigid_body);	
		delete collider_rigid_body;
	}*/
	
//	memory_Free(colliders);
//	memory_Free(colliders_free_positions_stack);
	
	for(i = 0; i < COLLIDER_TYPE_LAST; i++)
	{
		stack_list_destroy(&phy_colliders[i]);
	}
	
	delete physics_world;
	delete solver;
	delete broad_phase;
	delete narrow_phase;
	delete collision_configuration;
	
}


#define STEP_DELTA 16.666 

void physics_ProcessCollisions(double delta_time)
{
	
	int i;
	int step;
//	int c = player_count;
/*	int j; */
	
	float l;
	float d;
	float s;
	float c;
	
	int steps;
	float remaining_delta = delta_time;
	
	camera_t *active_camera = camera_GetActiveCamera();
			
	//if(!collision_nodes)
	//	return;	
	
	//printf("%f\n", delta_time);

	physics_UpdateColliders();
	physics_world->stepSimulation(delta_time * 0.001, 8, 1.0 / 120.0);
	//physics_world->stepSimulation(1.0 / 60.0);
	physics_PostUpdateColliders();
	
	//steps = (int)(delta_time / STEP_DELTA);
	
	
	//for(step = 0; step < steps; step++)
	/*while(remaining_delta > 0.0)
	{
		for(i = 0; i < player_count; i++)
		{
			l = players[i].delta.x * players[i].delta.x + players[i].delta.z * players[i].delta.z;
			
			if(l > MAX_HORIZONTAL_DELTA * MAX_HORIZONTAL_DELTA)
			{
				l = sqrt(l);
				l = MAX_HORIZONTAL_DELTA / l;
				
				players[i].delta.x *= l;
				players[i].delta.z *= l;
			} 
			
			players[i].delta.y -= GRAVITY * STEP_DELTA * 0.05;
						
			if(!(players[i].bm_movement & PLAYER_ON_GROUND))
			{
				players[i].delta.x *= 1.0 - (AIR_FRICTION * STEP_DELTA * 0.01);
				players[i].delta.z *= 1.0 - (AIR_FRICTION * STEP_DELTA * 0.01);
			}  
			else 
			{
				if(!(players[i].bm_movement & (MOVE_FORWARD | MOVE_BACKWARD | MOVE_STRAFE_LEFT | MOVE_STRAFE_RIGHT)))
				{
					players[i].delta.x /= 1.0 + (GROUND_FRICTION * STEP_DELTA * 0.01);
					players[i].delta.z /= 1.0 + (GROUND_FRICTION * STEP_DELTA * 0.01);
				}
			}
			
			player_Move(&players[i], STEP_DELTA);
			
			players[i].player_position.x = players[i].collision_box_position.x;
			players[i].player_position.z = players[i].collision_box_position.z;
			
			if(players[i].bm_movement & PLAYER_STEPPING_UP)
			{ 
				
				s = fabs(players[i].player_position.y);
				c = fabs(players[i].collision_box_position.y);
						
				players[i].player_position.y += (players[i].collision_box_position.y - players[i].player_position.y) * 0.1 * STEP_DELTA * 0.075;

						
				if(fabs(s - c) <= 0.01)
					players[i].bm_movement &= ~PLAYER_STEPPING_UP;
				
			}
			else
			{
				players[i].player_position.y += (players[i].collision_box_position.y - players[i].player_position.y) * 0.25 * STEP_DELTA * 0.075;
				players[i].bm_movement &= ~PLAYER_STEPPING_UP;
			}
		}
		
		remaining_delta -= STEP_DELTA;
		
	}
		
	for(i = 0; i < player_count; i++)
	{
				
		players[i].player_camera->world_position.x = players[i].player_position.x;
		players[i].player_camera->world_position.y = players[i].player_position.y + PLAYER_CAMERA_HEIGHT;
		players[i].player_camera->world_position.z = players[i].player_position.z;
		
		camera_PitchYawCamera(players[i].player_camera, players[i].yaw, players[i].pitch);
			
		if(players[i].player_camera == active_camera)
		{
			camera_ComputeWorldToCameraMatrix(players[i].player_camera);
		}	
	}*/
	
}

collider_def_t *physics_CreateColliderDef(char *name)
{
	collider_def_t *def;
	def = (collider_def_t *) memory_Malloc(sizeof(collider_def_t), "physics_CreateColliderDef");
	
	def->name = memory_Strdup(name, "physics_CreateColliderDef");
	def->ref_count = 0;
	def->next = NULL;
	def->prev = NULL;
	
	def->mass = 0.0;
	def->max_collision_shapes = 0;
	def->collision_shape_count = 0;
	def->collision_shape = NULL;
	def->cached_collision_shape = NULL;
	//def->mass = 1.0;		
	
	//def->max_collision_shapes = 16;
	//def->collision_shape_count = 0;
	//def->collision_shape = (collision_shape_t *) memory_Malloc(sizeof(collision_shape_t) * def->max_collision_shapes, "physics_CreateColliderDef");	
	//def->cached_collision_shape = NULL;	
	
	def->flags = 0;
	def->type = COLLIDER_TYPE_NONE;
	
	if(!collider_defs)
	{
		collider_defs = def;
	}
	else
	{
		last_collider_def->next = def;
		def->prev = last_collider_def;
	}
	
	last_collider_def = def;
	
	return def;
}

collider_def_t *physics_CreateRigidBodyColliderDef(char *name)
{
	collider_def_t *def;

	def = physics_CreateColliderDef(name);
	
	def->mass = 1.0;		
	def->max_collision_shapes = 16;
	def->collision_shape_count = 0;
	def->collision_shape = (collision_shape_t *) memory_Malloc(sizeof(collision_shape_t) * def->max_collision_shapes, "physics_CreateColliderDef");		
	def->flags = COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
	def->type = COLLIDER_TYPE_RIGID_BODY_COLLIDER;
		
	return def;
}

collider_def_t *physics_CreateCharacterColliderDef(char *name, float height, float crouch_height, float radius, float step_height, float slope_angle)
{
	collider_def_t *def;	
	def = physics_CreateColliderDef(name);
	
	def->height = height;
	def->crouch_height = crouch_height;
	def->radius = radius;
	def->slope_angle = slope_angle;
	def->step_height = step_height;
	
	def->mass = 1.0;
	
	def->flags = COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
	def->type = COLLIDER_TYPE_CHARACTER_COLLIDER;
		
	return def;
}

collider_def_t *physics_CreateProjectileColliderDef(char *name, float radius, float mass)
{
	collider_def_t *def;
	
	def = physics_CreateColliderDef(name);
	
	
	def->radius = radius;
	def->mass = mass;
	def->flags = COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
	def->type = COLLIDER_TYPE_PROJECTILE_COLLIDER;
	
	return def;
}

void physics_DestroyColliderDef(char *name)
{
	collider_def_t *r;
	
	r = collider_defs;
	
	while(r)
	{
		if(!strcmp(name, r->name))
		{
			physics_DestroyColliderDefPointer(r);
		}
		r = r->next;
	}
}

void physics_DestroyColliderDefPointer(collider_def_t *def)
{
	btCollisionShape *collision_shape;
	btCompoundShape *compound_shape;
	int i;
	int c;
	
	if(def)
	{
		if(def->ref_count)
		{
			//printf("physics_DestroyColliderDefPointer: def %s has %d references!\n", def->name, def->ref_count);
			//return;
			
			//for(i = 0; i < collider_list_cursor; i++)
			//{
			//	if(colliders[i].flags & COLLIDER_INVALID)
			//	{
			//		continue;
			//	}
				
				/* those colliders will  */
			//	if(colliders[i].def == def)
			//	{
			//		colliders[i].def = NULL;
			//	}
			//}
			
		}
		
		if(def == collider_defs)
		{
			collider_defs = collider_defs->next;
			
			if(collider_defs)
			{
				collider_defs->prev = NULL;
			}
		}
		else
		{
			def->prev->next = def->next;
				
			if(def->next)
			{
				def->next->prev = def->prev;
			}
			else
			{
				last_collider_def = last_collider_def->prev;
			}				
		}
				
		memory_Free(def->collision_shape);
		memory_Free(def);
	}
	
}

void physics_DestroyColliderDefs()
{	
	while(collider_defs)
	{
		last_collider_def = collider_defs->next;
		physics_DestroyColliderDefPointer(collider_defs);
		collider_defs = last_collider_def;
	}
}

collider_def_t *physics_GetColliderDefPointer(char *name)
{
	collider_def_t *r;
	
	r = collider_defs;
	
	while(r)
	{
		if(!strcmp(name, r->name))
		{
			return r;
		}
		r = r->next;
	}
	
	return NULL;
}


#define MIN_COLLISION_SHAPE_SCALE 0.01

void physics_AddCollisionShape(collider_def_t *def, vec3_t scale, vec3_t relative_position, mat3_t *relative_orientation, int type)
{
	int collision_shape_index;
	collision_shape_t *collision_shape;
	
	if(def)
	{		
		switch(type)
		{
			case COLLISION_SHAPE_BOX:
			case COLLISION_SHAPE_CYLINDER:
			//case COLLISION_SHAPE_SPHERE:
				collision_shape_index = def->collision_shape_count++;
		
				if(collision_shape_index >= def->max_collision_shapes)
				{
					collision_shape = (collision_shape_t *) memory_Malloc(sizeof(collision_shape_t ) * (def->max_collision_shapes + 16), "physics_AddCollisionShape");
					memcpy(collision_shape, def->collision_shape, sizeof(collision_shape_t ) * def->max_collision_shapes);
					
					memory_Free(def->collision_shape);
					
					def->collision_shape = collision_shape;
					def->max_collision_shapes += 16;
				}
				
				if(scale.x < MIN_COLLISION_SHAPE_SCALE) scale.x = MIN_COLLISION_SHAPE_SCALE;
				if(scale.y < MIN_COLLISION_SHAPE_SCALE) scale.y = MIN_COLLISION_SHAPE_SCALE;
				if(scale.z < MIN_COLLISION_SHAPE_SCALE) scale.z = MIN_COLLISION_SHAPE_SCALE;
				
				if(relative_orientation)
				{
					def->collision_shape[collision_shape_index].orientation = *relative_orientation;
				}
				else
				{
					def->collision_shape[collision_shape_index].orientation = mat3_t_id();
				}
				
				def->collision_shape[collision_shape_index].position = relative_position;
				def->collision_shape[collision_shape_index].scale = scale;
				def->collision_shape[collision_shape_index].type = type;
				
				def->flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
				
				/* updating ALL the colliders that use this
				shape every time something changes may become
				a bottleneck, but changing the collision shape
				at runtime is not intended... */
				physics_UpdateReferencingColliders(def);
			break;
			
			default:
			
			return;
		}
		
			
	}
}



void physics_RemoveCollisionShape(collider_def_t *def, int shape_index)
{
	if(def)
	{
		if(shape_index >= 0 && shape_index < def->collision_shape_count)
		{
			if(shape_index < def->collision_shape_count - 1)
			{
				def->collision_shape[shape_index] = def->collision_shape[def->collision_shape_count - 1];
			}
			
			def->collision_shape_count--;
			def->flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
			
			physics_UpdateReferencingColliders(def);
		}
	}
}

void physics_TranslateCollisionShape(collider_def_t *def, vec3_t translation, int shape_index)
{
	if(def)
	{
		
		if(shape_index >= 0 && shape_index < def->collision_shape_count)
		{
			def->collision_shape[shape_index].position.x += translation.x;
			def->collision_shape[shape_index].position.y += translation.y;
			def->collision_shape[shape_index].position.z += translation.z;
			
			def->flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
			physics_UpdateReferencingColliders(def);
		}
	}
}

void physics_RotateCollisionShape(collider_def_t *def, vec3_t axis, float amount, int shape_index)
{
	if(def)
	{
		if(shape_index >= 0 && shape_index < def->collision_shape_count)
		{
			mat3_t_rotate(&def->collision_shape[shape_index].orientation, axis, amount, 0);	
			def->flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
			physics_UpdateReferencingColliders(def);
		}
	}
}

void physics_ScaleCollisionShape(collider_def_t *def, vec3_t scale, int shape_index)
{
	if(def)
	{
		if(shape_index >= 0 && shape_index < def->collision_shape_count)
		{			
			def->collision_shape[shape_index].scale.x += scale.x;
			def->collision_shape[shape_index].scale.y += scale.y;
			def->collision_shape[shape_index].scale.z += scale.z;
			
			if(def->collision_shape[shape_index].scale.x < MIN_COLLISION_SHAPE_SCALE) def->collision_shape[shape_index].scale.x = MIN_COLLISION_SHAPE_SCALE;
			if(def->collision_shape[shape_index].scale.y < MIN_COLLISION_SHAPE_SCALE) def->collision_shape[shape_index].scale.y = MIN_COLLISION_SHAPE_SCALE;
			if(def->collision_shape[shape_index].scale.z < MIN_COLLISION_SHAPE_SCALE) def->collision_shape[shape_index].scale.z = MIN_COLLISION_SHAPE_SCALE;
			
			def->flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
			physics_UpdateReferencingColliders(def);
		}
	}
}




void *physics_BuildCollisionShape(collider_def_t *def)
{
	int i;
	int c;
	
	btCompoundShape *compound_shape = NULL;
	btCollisionShape *collision_shape;
	collision_shape_t *def_collision_shape;
	btTransform collision_shape_transform;
	btVector3 inertia_tensor;
	
	if(def)
	{	
		/* would like to share collision shapes, but this wouldn't allow
		them to be scaled on a per instance basis... */		
		switch(def->type)
		{
			case COLLIDER_TYPE_RIGID_BODY_COLLIDER:
				if(def->collision_shape_count > 1)
				{
					compound_shape = new btCompoundShape();
					for(i = 0; i < def->collision_shape_count; i++)
					{
						
						def_collision_shape = &def->collision_shape[i];
						collision_shape_transform.setIdentity();
						collision_shape_transform.setOrigin(btVector3(def_collision_shape->position.x, def_collision_shape->position.y, def_collision_shape->position.z));
						collision_shape_transform.setBasis(&def_collision_shape->orientation.floats[0][0]);
						
						switch(def->collision_shape[i].type)
						{
							case COLLISION_SHAPE_BOX:		
								compound_shape->addChildShape(collision_shape_transform, new btBoxShape(btVector3(def_collision_shape->scale.x, def_collision_shape->scale.y, def_collision_shape->scale.z)));
							break;
							
							case COLLISION_SHAPE_CYLINDER:
								compound_shape->addChildShape(collision_shape_transform, new btCylinderShape(btVector3(def_collision_shape->scale.x, def_collision_shape->scale.y, def_collision_shape->scale.z)));
							break;
						}
					}
					collision_shape = compound_shape;
				}
				else if(def->collision_shape_count == 1)
				{
					def_collision_shape = &def->collision_shape[0];
					switch(def_collision_shape->type)
					{
						case COLLISION_SHAPE_BOX:
							collision_shape = new btBoxShape(btVector3(def_collision_shape->scale.x, def_collision_shape->scale.y, def_collision_shape->scale.z));
						break;
						
						case COLLISION_SHAPE_CYLINDER:
							collision_shape = new btCylinderShape(btVector3(def_collision_shape->scale.x, def_collision_shape->scale.y, def_collision_shape->scale.z));
						break;
					}
				}
			break;
			
			case COLLIDER_TYPE_CHARACTER_COLLIDER:
				collision_shape = new btCapsuleShape(def->radius, def->height - 2.0 * def->radius);
			break;
			
			case COLLIDER_TYPE_PROJECTILE_COLLIDER:
				collision_shape = new btSphereShape(def->radius);
			break;
		}		
			
		/* still not sure if it is necessary to recalculate this 
		once the collision shape gets scaled...  */
		if(def->flags & COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR)
		{
			collision_shape->calculateLocalInertia(def->mass, inertia_tensor);
			def->inertia_tensor.x = inertia_tensor[0];
			def->inertia_tensor.y = inertia_tensor[1];
			def->inertia_tensor.z = inertia_tensor[2];
			
			if(def->cached_collision_shape)
			{
				physics_DestroyCollisionShape(def->cached_collision_shape);
			}
			
			def->cached_collision_shape = collision_shape;
			
			def->flags &= ~COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
		}		
	}
	
	return collision_shape;
}

void physics_IncColliderDefRefCount(collider_def_t *def)
{
	if(def)
	{
		def->ref_count++;
	}
}

void physics_DecColliderDefRefCount(collider_def_t *def)
{
	if(def)
	{
		if(def->ref_count > 0)
		{
			def->ref_count--;
		}
	}
}

void physics_UpdateReferencingColliders(collider_def_t *def)
{
	int i;
	
	btRigidBody *rigid_body;
	btCollisionShape *collision_shape;
		
	if(def)
	{
		if(!def->ref_count)
			return;
		
		//for(i = 0; i < collider_list_cursor; i++)
		//{
		//	if(colliders[i].flags & COLLIDER_INVALID)
		//		continue;
			
		//	if(colliders[i].def != def)
		//		continue;	
			
			
		//	if(!(colliders[i].flags & COLLIDER_NO_SCALE_HINT))
		//	{
				/* make a copy of the collision shape, so this collider can be freely scaled... */
		//		collision_shape = (btCollisionShape *) physics_BuildCollisionShape(def);
		//		rigid_body = (btRigidBody *) colliders[i].rigid_body;
				
				/* get rid of the outdated copy this collider has... */
		//		physics_DestroyCollisionShape(rigid_body->getCollisionShape());
				
		//		rigid_body->setCollisionShape(collision_shape);
		//		rigid_body->setMassProps(def->mass, btVector3(def->inertia_tensor.x, def->inertia_tensor.y, def->inertia_tensor.z));
		//	}
			
		//	rigid_body->setAngularVelocity(btVector3(0.0, 0.0, 0.0));
		//	rigid_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
		//	rigid_body->activate(true);
		//	
		//	colliders[i].flags |= COLLIDER_UPDATE_RIGID_BODY;
		//}
	}
}

void physics_DestroyCollisionShape(void *collision_shape)
{
	int i;
	int c;
	
	
	btCollisionShape *coll_shape;
	btCompoundShape *compound_shape;
	
	
	coll_shape = (btCollisionShape *) collision_shape;
	
	if(coll_shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
	{
		compound_shape = (btCompoundShape *) coll_shape;
		c = compound_shape->getNumChildShapes();
		
		for(i = c - 1; i >= 0; i--)
		{
			delete compound_shape->getChildShape(i);
		}
	}
	
	delete coll_shape;
}


/*
=================================================================
=================================================================
=================================================================
*/


struct collider_handle_t physics_CreateEmptyCollider(int type)
{
	int collider_index;
	struct collider_handle_t handle;
	struct collider_t *collider;
	
	
	handle.type = COLLIDER_TYPE_NONE;
	handle.index = INVALID_COLLIDER_INDEX;
	
	switch(type)
	{
		case COLLIDER_TYPE_RIGID_BODY_COLLIDER:
		case COLLIDER_TYPE_CHARACTER_COLLIDER:
		case COLLIDER_TYPE_PROJECTILE_COLLIDER:
			collider_index = stack_list_add(&phy_colliders[type], NULL);
		break;
		
		default:
			return handle;
		break;
	}
	
	collider = (struct collider_t *)stack_list_get(&phy_colliders[type], collider_index);
	
	handle.type = type;
	handle.index = collider_index;
	
	/*if(colliders_free_positions_stack_top > -1)
	{
		collider_index = colliders_free_positions_stack[colliders_free_positions_stack_top];
		colliders_free_positions_stack_top--;
	}
	else
	{
		collider_index = collider_list_cursor++;
		
		if(collider_index >= max_colliders)
		{
			
			memory_Free(colliders_free_positions_stack);
			
			collider = (collider_t *) memory_Malloc(sizeof(collider_t) * (max_colliders + 16), "physics_CreateEmtpyCollider");
			colliders_free_positions_stack = (int *) memory_Malloc(sizeof(int) * (max_colliders + 16), "physics_CreateEmptyCollider");
			
			memcpy(collider, colliders, sizeof(collider_t) * max_colliders);
			memory_Free(colliders);
			
			colliders = collider;
			max_colliders += 16;
		}
		
	}
	
	collider = &colliders[collider_index];*/
	
	
	collider->orientation.floats[0][0] = 1.0;
	collider->orientation.floats[0][1] = 0.0;
	collider->orientation.floats[0][2] = 0.0;
	
	collider->orientation.floats[1][0] = 0.0;
	collider->orientation.floats[1][1] = 1.0;
	collider->orientation.floats[1][2] = 0.0;
	
	collider->orientation.floats[2][0] = 0.0;
	collider->orientation.floats[2][1] = 0.0;
	collider->orientation.floats[2][2] = 1.0;
	
	collider->position.x = 0.0;
	collider->position.y = 0.0;
	collider->position.z = 0.0;
	
	collider->flags = 0;
	collider->character_collider_flags = 0;
	collider->type = type;
	collider->rigid_body = NULL;
	collider->def = NULL;
	//collider->collision_shape = NULL;
	
	return handle;
	
	//return collider_index;
}

struct collider_handle_t physics_CreateCollider(mat3_t *orientation, vec3_t position, vec3_t scale, collider_def_t *def, int flags)
{
	int collider_index;
	struct collider_handle_t handle;
	
	struct collider_t *collider;
	btCollisionShape *collider_collision_shape;
	btRigidBody *collider_rigid_body;
	btTransform collider_transform;
	btDefaultMotionState *collider_motion_state;
	
	handle.type = COLLIDER_TYPE_NONE;
	handle.index = INVALID_COLLIDER_INDEX;

	if(!def)
	{
		printf("physics_CreateCollider: bad def\n");
		return handle;
	}

	
	//collider_index = physics_CreateEmptyCollider();
	//collider = &colliders[collider_index];
	
	handle = physics_CreateEmptyCollider(def->type);
	collider = physics_GetColliderPointerHandle(handle);
	
	collider->position = position;
	collider->scale = scale;
	collider->type = def->type;
	collider->flags = COLLIDER_UPDATE_RIGID_BODY;
	collider->def = def;
	
	collider_transform.setIdentity();
	collider_transform.setOrigin(btVector3(position.x, position.y, position.z));
	 
	if(def->type == COLLIDER_TYPE_RIGID_BODY_COLLIDER && orientation)
	{
		collider->orientation = *orientation;
		collider_transform.setBasis((float *)orientation);	
	}
	else
	{
		collider->height = def->height;
		collider->radius = def->radius;
		collider->max_slope = def->slope_angle;
		collider->step_height = def->step_height;
	}
	
	
	if((flags & COLLIDER_NO_SCALE_HINT) && def->cached_collision_shape)
	{
		collider_collision_shape = (btCollisionShape *)def->cached_collision_shape;
	}
	else
	{
		collider_collision_shape = (btCollisionShape *) physics_BuildCollisionShape(def);
	}
	
	
	collider_motion_state = new btDefaultMotionState(collider_transform);
	collider_rigid_body = new btRigidBody(def->mass, collider_motion_state, collider_collision_shape, btVector3(def->inertia_tensor.x, def->inertia_tensor.y, def->inertia_tensor.z));
	
	physics_world->addRigidBody(collider_rigid_body);
	
	
	collider_rigid_body->setWorldTransform(collider_transform);
	collider_rigid_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
	collider_rigid_body->setAngularVelocity(btVector3(0.0, 0.0, 0.0));
	
	if(def->type == COLLIDER_TYPE_CHARACTER_COLLIDER || def->type == COLLIDER_TYPE_PROJECTILE_COLLIDER)
	{
		/* a character controller is a capsule that can't rotate... */
		collider_rigid_body->setAngularFactor(btVector3(0.0, 0.0, 0.0));
		
		//if(def->type == COLLIDER_TYPE_PROJECTILE_COLLIDER)
		//{
		//	collider_rigid_body->setLinearFactor(btVector3(0.0, 0.0, 0.0));
		//}
	}
	
	
	collider_rigid_body->setSleepingThresholds(0.05, 0.05);
	collider_rigid_body->activate(true);
	
	collider->rigid_body = collider_rigid_body;
	
	return handle;
	//return collider_index;
}

int physics_CopyCollider(int collider_index)
{/*
	int new_collider_index;
	collider_t *src;
	collider_t *new_collider;
	btRigidBody *src_rigid_body;
	btRigidBody *new_collider_rigid_body;
	btTransform new_collider_transform;
	btDefaultMotionState *new_collider_motion_state;
	btBoxShape *new_collider_test_collision_shape;
	btVector3 new_collider_test_local_inertia;
	

	if(collider_index < 0 || collider_index >= collider_list_cursor)
		return -1;
	
	src = &colliders[collider_index];
	
	if(src->flags & COLLIDER_INVALID)
		return -1;
	
	
	src_rigid_body = (btRigidBody *)src->rigid_body;
	
	new_collider_index = physics_CreateEmptyCollider();
	new_collider = &colliders[collider_index];
	
	
	
	new_collider->position = position;
	new_collider->orientation = *orientation;
	
	src_rigid_body->getWorldTransform(&new_collider_transform);
	
	collider_motion_state = new btDefaultMotionState(new_collider_transform);

	*/
	return collider_index;
}

void physics_DestroyColliderHandle(struct collider_handle_t collider)
{
	btRigidBody *collider_rigid_body;
	struct collider_t *collider_ptr;
	btCollisionShape *collider_collision_shape;
	collider_def_t *collider_def;
	struct stack_list_t *list;
	
	if(collider.type != COLLIDER_TYPE_NONE)
	{
		list = &phy_colliders[collider.type];
		
		if(collider.index >= 0 && collider.index < list->element_count)
		{
			collider_ptr = (struct collider_t *)list->elements + collider.index;
			
			if(!(collider_ptr->flags & COLLIDER_INVALID))
			{
				collider_ptr->flags |= COLLIDER_INVALID;
				collider_rigid_body = (btRigidBody *)collider_ptr->rigid_body;
				 
				physics_world->removeRigidBody(collider_rigid_body);
				collider_collision_shape = collider_rigid_body->getCollisionShape();
				delete collider_rigid_body;
				
				physics_DestroyCollisionShape(collider_collision_shape);
				
				physics_DecColliderDefRefCount(collider_ptr->def);
				
				collider_ptr->rigid_body = NULL;
				collider_ptr->def = NULL;
				
				stack_list_remove(list, collider.index);
			}
		}
	}
}

struct collider_t *physics_GetColliderPointerHandle(struct collider_handle_t collider)
{
	stack_list_t *list;
	struct collider_t *collider_ptr;
	
	if(collider.type != COLLIDER_TYPE_NONE)
	{
		list = &phy_colliders[collider.type];
	
		if(collider.index >= 0 && collider.index < list->element_count)
		{
			collider_ptr = (struct collider_t *)list->elements + collider.index;
			if(!(collider_ptr->flags & COLLIDER_INVALID))
			{
				return collider_ptr;
			}
		}
	}
	return NULL;
}

void physics_GetColliderAabb(struct collider_handle_t collider, vec3_t *aabb)
{
	struct collider_t *collider_ptr;
	struct stack_list_t *list;
	btVector3 max;
	btVector3 min;
	
	btRigidBody *rigid_body;
	
	if(collider.type != COLLIDER_TYPE_NONE)
	{
		list = &phy_colliders[collider.type];
	
		if(collider.index >= 0 && collider.index < list->element_count)
		{
			collider_ptr = (struct collider_t *)list->elements + collider.index;
			
			if(!(collider_ptr->flags & COLLIDER_INVALID))
			{
				//collider = colliders + collider_index;
				rigid_body = (btRigidBody *) collider_ptr->rigid_body;
				rigid_body->getAabb(min, max);
				
				aabb->x = (max[0] - min[0]) / 2.0;
				aabb->y = (max[1] - min[1]) / 2.0;
				aabb->z = (max[2] - min[2]) / 2.0;
			}
		}
	}
	
	
	 
}

/*
=================================================================
=================================================================
=================================================================
*/

void physics_SetColliderPosition(struct collider_handle_t collider, vec3_t position)
{
	struct collider_t *collider_ptr;
	struct stack_list_t *list;
	
	if(collider.type != COLLIDER_TYPE_NONE)
	{
		list = &phy_colliders[collider.type];
		
		if(collider.index >= 0 && collider.index < list->element_count)
		{
			collider_ptr = (struct collider_t *)list->elements + collider.index;
			
			if(!(collider_ptr->flags & COLLIDER_INVALID))
			{
				collider_ptr->position = position;
				collider_ptr->flags |= COLLIDER_UPDATE_RIGID_BODY;
			}
		}
	}
	
	
}

void physics_SetColliderOrientation(struct collider_handle_t collider, mat3_t *orientation)
{
	struct collider_t *collider_ptr;
	struct stack_list_t *list;
	
	if(collider.type != COLLIDER_TYPE_NONE)
	{
		list = &phy_colliders[collider.type];
		
		if(collider.index >= 0 && collider.index < list->element_count)
		{
			collider_ptr = (struct collider_t *)list->elements + collider.index;
			
			if(!(collider_ptr->flags & COLLIDER_INVALID))
			{
				collider_ptr->orientation = *orientation;
				collider_ptr->flags |= COLLIDER_UPDATE_RIGID_BODY;
			}
		}
	}
} 

void physics_SetColliderScale(struct collider_handle_t collider, vec3_t scale)
{
	struct collider_t *collider_ptr;
	struct stack_list_t *list;
	
	if(collider.type != COLLIDER_TYPE_NONE)
	{
		list = &phy_colliders[collider.type];
		
		if(collider.index >= 0 && collider.index < list->element_count)
		{
			collider_ptr = (struct collider_t *)list->elements + collider.index;
			
			if(!(collider_ptr->flags & COLLIDER_INVALID))
			{
				if(collider_ptr->flags & COLLIDER_NO_SCALE_HINT)
				{
					return;
				}

				collider_ptr->scale = scale;
				collider_ptr->flags |= COLLIDER_UPDATE_RIGID_BODY;
			}
		}
	}
}

void physics_ApplyCentralForce(struct collider_handle_t collider, vec3_t force)
{
	struct collider_t *collider_ptr;
	btRigidBody *rigid_body;
	
	collider_ptr = physics_GetColliderPointerHandle(collider);
	
	if(collider_ptr)
	{
		rigid_body = (btRigidBody *)collider_ptr->rigid_body;
		rigid_body->activate(true);
		rigid_body->applyCentralForce(btVector3(force.x, force.y, force.z));
	}
}

void physics_ApplyCentralImpulse(struct collider_handle_t collider, vec3_t impulse)
{
	struct collider_t *collider_ptr;
	btRigidBody *rigid_body;
	
	collider_ptr = physics_GetColliderPointerHandle(collider);
	
	if(collider_ptr)
	{
		rigid_body = (btRigidBody *)collider_ptr->rigid_body;
		rigid_body->activate(true);
		rigid_body->applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
	}
}


/*
=================================================================
=================================================================
=================================================================
*/

void physics_UpdateColliders()
{
	int i;
	int collider_count;
	int j;
	
	struct collider_t *collider;
	struct stack_list_t *list;
	btRigidBody *rigid_body;
	btCollisionShape *collision_shape;
	btTransform rigid_body_transform;
	
	
	for(j = 0; j < COLLIDER_TYPE_LAST; j++)
	{
		
		list = &phy_colliders[j];
		collider_count = list->element_count;
		
		for(i = 0; i < collider_count; i++)
		{
			
			collider = (struct collider_t *)list->elements + i;
			
			if(collider->flags & COLLIDER_INVALID)
			{
				continue;
			}
				
			if(!(collider->flags & COLLIDER_UPDATE_RIGID_BODY))
			{
				continue;
			}
					
			//collider = &colliders[i];
					
			rigid_body = (btRigidBody *)collider->rigid_body;
					
			rigid_body_transform.setIdentity();
			rigid_body_transform.setOrigin(btVector3(collider->position.x, collider->position.y, collider->position.z));
			
			if(collider->type == COLLIDER_TYPE_RIGID_BODY_COLLIDER)
			{
				rigid_body_transform.setBasis(&collider->orientation.floats[0][0]);
				
				if(!(collider->flags & COLLIDER_NO_SCALE_HINT))
				{
					collision_shape = rigid_body->getCollisionShape();
					
					collision_shape->setLocalScaling(btVector3(collider->scale.x, collider->scale.y, collider->scale.z));
				}
			}
			
			rigid_body->setWorldTransform(rigid_body_transform);
			rigid_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
			rigid_body->setAngularVelocity(btVector3(0.0, 0.0, 0.0));
			rigid_body->activate(true);
			
			collider->flags &= ~COLLIDER_UPDATE_RIGID_BODY;
		}
	}
	
	
}

void physics_PostUpdateColliders()
{
	int i;
	int j;
	int collider_count;
	
	struct collider_t *collider;
	struct stack_list_t *list;
	struct collider_handle_t handle;
	btRigidBody *collider_rigid_body;
	btTransform collider_rigid_body_transform;
	btVector3 collider_rigid_body_position;
	btMatrix3x3 collider_rigid_body_orientation;
	
	for(j = 0; j < COLLIDER_TYPE_LAST; j++)
	{
		
		list = &phy_colliders[j];
		collider_count = list->element_count;
		
		for(i = 0; i < collider_count; i++)
		{
			collider = (struct collider_t *)list->elements + i;
			
			if(collider->flags & COLLIDER_INVALID)
				continue;
			
			switch(collider->type)
			{
				case COLLIDER_TYPE_RIGID_BODY_COLLIDER:
				
				break;
				
				case COLLIDER_TYPE_CHARACTER_COLLIDER:
					handle.type = COLLIDER_TYPE_CHARACTER_COLLIDER;
					handle.index = i;
					physics_UpdateCharacterCollider(handle);
				break;
				
				case COLLIDER_TYPE_PROJECTILE_COLLIDER:
					handle.type = COLLIDER_TYPE_PROJECTILE_COLLIDER;
					handle.index = i;
					physics_UpdateProjectileCollider(handle);
				break;
			}
			
			collider_rigid_body = (btRigidBody *)collider->rigid_body;	
				
			collider_rigid_body->getMotionState()->getWorldTransform(collider_rigid_body_transform);
			
			collider_rigid_body_position = collider_rigid_body_transform.getOrigin();
			collider_rigid_body_orientation = collider_rigid_body_transform.getBasis();
			
			collider->position.x = collider_rigid_body_position[0];
			collider->position.y = collider_rigid_body_position[1];
			collider->position.z = collider_rigid_body_position[2];

			collider->orientation.floats[0][0] = collider_rigid_body_orientation[0][0];
			collider->orientation.floats[0][1] = collider_rigid_body_orientation[1][0];
			collider->orientation.floats[0][2] = collider_rigid_body_orientation[2][0];
				
			collider->orientation.floats[1][0] = collider_rigid_body_orientation[0][1];
			collider->orientation.floats[1][1] = collider_rigid_body_orientation[1][1];
			collider->orientation.floats[1][2] = collider_rigid_body_orientation[2][1];
				
			collider->orientation.floats[2][0] = collider_rigid_body_orientation[0][2];
			collider->orientation.floats[2][1] = collider_rigid_body_orientation[1][2];
			collider->orientation.floats[2][2] = collider_rigid_body_orientation[2][2];
		}
	}
	
	
}


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

int physics_Raycast(vec3_t from, vec3_t to, vec3_t *hit_position, vec3_t *hit_normal)
{
	 
	btVector3 vfrom = btVector3(from.x, from.y, from.z);
	btVector3 vto = btVector3(to.x, to.y, to.z);
	
	btCollisionWorld::ClosestRayResultCallback callback(vfrom, vto);
	
	float htime;
	
	if(world_collision_object)
	{
		physics_world->rayTest(vfrom, vto, callback);
		
		if(callback.m_closestHitFraction < 1.0)
		{
			if(hit_position)
			{
				hit_position->x = callback.m_hitPointWorld[0];
				hit_position->y = callback.m_hitPointWorld[1];
				hit_position->z = callback.m_hitPointWorld[2];	
			}
			
			if(hit_normal)
			{
				hit_normal->x = callback.m_hitNormalWorld[0];
				hit_normal->y = callback.m_hitNormalWorld[1];
				hit_normal->z = callback.m_hitNormalWorld[2];
			}
			
			return 1;
		}
		
	}
	
	return 0;
}

/*
=================================================================
=================================================================
=================================================================
*/

void physics_ClearWorldCollisionMesh()
{
	if(world_collision_object)
	{
		physics_world->removeCollisionObject(world_collision_object);
		delete world_collision_object;
		delete world_collision_mesh;
		
		world_collision_object = NULL;
		world_collision_mesh = NULL;
	}
}

void physics_BuildWorldCollisionMesh()
{
	int i;
	
	//btTriangleMesh *triangle_mesh;
	
	if(w_world_vertices_count < 3)
		return;
		
	
	physics_ClearWorldCollisionMesh();	
	
	
//	triangle_mesh = new btTriangleMesh();
	world_triangles = new btTriangleMesh();
	for(i = 0; i < w_world_vertices_count; i += 3)
	{
		world_triangles->addTriangle(btVector3(w_world_vertices[i	 ].position.x, w_world_vertices[i	   ].position.y, w_world_vertices[i	 ].position.z),
		  						     btVector3(w_world_vertices[i + 1].position.x, w_world_vertices[i + 1].position.y, w_world_vertices[i + 1].position.z),
								     btVector3(w_world_vertices[i + 2].position.x, w_world_vertices[i + 2].position.y, w_world_vertices[i + 2].position.z));
	}
	
	world_collision_mesh = new btBvhTriangleMeshShape(world_triangles, true, true);	
	
	world_collision_object = new btCollisionObject();
	world_collision_object->setCollisionShape(world_collision_mesh);
	
	physics_world->addCollisionObject(world_collision_object);
}

#ifdef __cplusplus
}

} /* why is g++ asking for this? */
}
#endif




