#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <intrin.h>

#include "GL\glew.h"

#include "physics.h"
#include "bsp.h"
#include "gpu.h"
#include "c_memory.h"
#include "matrix.h"
#include "log.h"
#include "r_debug.h"

#include "stack_list.h"
#include "list.h"


#include "camera.h"


/* from world.c */
extern int w_world_vertices_count;
extern vertex_t *w_world_vertices;


//extern int player_count;
//extern player_t *players;
//extern bsp_pnode_t *collision_nodes;


int collision_def_count = 0;
struct collider_def_t *phy_collider_defs = NULL;
struct collider_def_t *phy_last_collider_def = NULL;



int collider_type_size[COLLIDER_TYPE_LAST];



//int collider_list_cursor = 0;
//int max_colliders = 0;
//collider_t *colliders = NULL;
//int colliders_free_positions_stack_top = -1;
//int *colliders_free_positions_stack = NULL;

#define EXTRA_MARGIN 0.01

extern int world_hull_node_count;
extern bsp_pnode_t *world_hull;


struct stack_list_t phy_colliders[COLLIDER_TYPE_LAST];
struct list_t phy_contact_records;


/* I'm not sure why this works, but it does... */
#ifdef __cplusplus
extern "C++"
{
#endif // __cplusplus

	#include "bullet/include/btBulletCollisionCommon.h"
	#include "bullet/include/btBulletDynamicsCommon.h"
	#include "bullet/include/BulletCollision/CollisionShapes/btCollisionShape.h"
	#include "bullet/include/BulletCollision/CollisionShapes/btBoxShape.h"
	#include "bullet/include/BulletCollision/CollisionShapes/btCylinderShape.h"
	#include "bullet/include/BulletCollision/CollisionShapes/btSphereShape.h"
	#include "bullet/include/BulletCollision/CollisionShapes/btCapsuleShape.h"
	#include "bullet/include/BulletCollision/CollisionShapes/btCompoundShape.h"
	#include "bullet/include/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h"

	#include "bullet/include/BulletCollision/CollisionDispatch/btGhostObject.h"

	#include "bullet/include/BulletDynamics/ConstraintSolver/btHingeConstraint.h"

	btDefaultCollisionConfiguration *collision_configuration = NULL;
	btCollisionDispatcher *narrow_phase = NULL;
	btBroadphaseInterface *broad_phase = NULL;
	btSequentialImpulseConstraintSolver *solver = NULL;
	btDiscreteDynamicsWorld *physics_world = NULL;

	btCollisionObject *world_collision_object = NULL;
	btBvhTriangleMeshShape *world_collision_mesh = NULL;
	btTriangleMesh *world_triangles = NULL;
#ifdef __cplusplus
}
#endif // __cplusplus


#ifdef __cplusplus
extern "C"
{
#endif


struct ClosestNotMeRaycastResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
    btCollisionObject *m_me;

    ClosestNotMeRaycastResultCallback(const btVector3 &from, const btVector3 &to, btCollisionObject *me) :
    btCollisionWorld::ClosestRayResultCallback(from, to)
    {
        m_me = me;
    }

    btScalar addSingleResult(btCollisionWorld::LocalRayResult &ray_result, bool world_normal)
    {
        if(ray_result.m_collisionObject == m_me)
        {
            return 1.0;
        }

        if(ray_result.m_hitNormalLocal.dot(m_rayToWorld - m_rayFromWorld) < 0.0)
        {
            /* ignore hit if it hit a backface (or if the origin is inside a collider)... */
            return 1.0;
        }

        return ClosestRayResultCallback::addSingleResult(ray_result, world_normal);
    }
};


struct ClosestWorldOnlyRaycaseResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
	btCollisionObject *m_world;

	ClosestWorldOnlyRaycaseResultCallback(const btVector3 &from, const btVector3 &to, btCollisionObject *world)	:
	btCollisionWorld::ClosestRayResultCallback(from, to)
	{
        m_world = world;
	}

	btScalar addSingleResult(btCollisionWorld::LocalRayResult &ray_result, bool world_normal)
	{
		if(m_world)
		{
			if(ray_result.m_collisionObject != m_world)
			{
				return 1.0;
			}
		}

		return ClosestRayResultCallback::addSingleResult(ray_result, world_normal);
	}
};


int physics_Init()
{
	int i;
	collision_configuration = new btDefaultCollisionConfiguration();
	narrow_phase = new btCollisionDispatcher(collision_configuration);
	broad_phase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver;
	physics_world = new btDiscreteDynamicsWorld(narrow_phase, broad_phase, solver, collision_configuration);
	physics_world->setGravity(btVector3(0.0, -GRAVITY * 1000.0, 0.0));



	collider_type_size[COLLIDER_TYPE_RIGID_BODY_COLLIDER] = sizeof(struct rigid_body_collider_t);
	collider_type_size[COLLIDER_TYPE_CHARACTER_COLLIDER] = sizeof(struct character_collider_t);
	collider_type_size[COLLIDER_TYPE_TRIGGER_COLLIDER] = sizeof(struct trigger_collider_t);
	collider_type_size[COLLIDER_TYPE_PROJECTILE_COLLIDER] = sizeof(struct projectile_collider_t);
	collider_type_size[COLLIDER_TYPE_COLLIDER_DEF] = sizeof(struct collider_def_t);



	//max_colliders = 128;
	//colliders = (collider_t *) memory_Malloc(sizeof(collider_t ) * max_colliders, "physics_Init");
	//colliders_free_positions_stack = (int *) memory_Malloc(sizeof(int) * max_colliders, "physics_Init");


	for(i = 0; i < COLLIDER_TYPE_LAST; i++)
	{
		phy_colliders[i] = stack_list_create(collider_type_size[i], 64, NULL);
	}

	phy_contact_records = list_create(sizeof(struct contact_record_t), 32000, NULL);

	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "%s: subsystem initialized properly!", __func__);


	return 1;
}

void physics_Finish()
{
	int i;
	btRigidBody *collider_rigid_body;

	/*while(phy_collider_defs)
	{
		phy_last_collider_def = phy_collider_defs->next;

		if(phy_collider_defs->type == COLLIDER_TYPE_RIGID_BODY_COLLIDER)
		{
			memory_Free(phy_collider_defs->collision_shape);
		}

		memory_Free(phy_collider_defs->name);
		memory_Free(phy_collider_defs);
		phy_collider_defs = phy_last_collider_def;
	}*/

	for(i = 0; i < COLLIDER_TYPE_LAST; i++)
	{
		stack_list_destroy(&phy_colliders[i]);
	}

	list_destroy(&phy_contact_records);

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

	int timer;

//	camera_t *active_camera = camera_GetActiveCamera();

	//if(!collision_nodes)
	//	return;

	//printf("%f\n", delta_time);

    //timer = renderer_StartCpuTimer("physics_UpdateColliders");
	physics_UpdateColliders();
	//renderer_StopTimer(timer);

	//timer = renderer_StartCpuTimer("physics_world->stepSimulation");
	physics_world->stepSimulation(delta_time * 0.001, 5, 1.0 / 60.0);
	//renderer_StopTimer(timer);

	//timer = renderer_StartCpuTimer("physics_PostUpdateColliders");
	physics_PostUpdateColliders();
	//renderer_StopTimer(timer);
}

struct collider_handle_t physics_CreateColliderDef(char *name)
{

    struct collider_handle_t def = INVALID_COLLIDER_HANDLE;
    struct collider_def_t *def_ptr;
    int i;

    def = physics_CreateEmptyCollider(COLLIDER_TYPE_COLLIDER_DEF);

    def_ptr = (struct collider_def_t *)physics_GetColliderPointer(def);

    if(def_ptr)
    {
        def_ptr->mass = 0.0;
        def_ptr->collider_type = COLLIDER_TYPE_NONE;

        //def_ptr->collision_shape_count = 0;
        def_ptr->collision_shape.elements = NULL;
        def_ptr->collision_shape.element_count = 0;
        def_ptr->collision_shape.element_size = 0;
        def_ptr->collision_shape.max_elements = 0;

        //def_ptr->collision_shape = NULL;
        def_ptr->cached_collision_shape = NULL;
        def_ptr->name = (char *)memory_Calloc(COLLIDER_DEF_NAME_MAX_LEN, 1);

        for(i = 0; i < COLLIDER_DEF_NAME_MAX_LEN - 1 && name[i]; i++)
        {
            def_ptr->name[i] = name[i];
        }

        //def_ptr->name = memory_Strdup(name);
    }

    return def;
}

struct collider_handle_t physics_CreateRigidBodyColliderDef(char *name)
{
    struct collider_handle_t def = INVALID_COLLIDER_HANDLE;
    struct collider_def_t *def_ptr;
	/*collider_def_t *def;

	def = physics_CreateColliderDef(name);

	def->mass = 1.0;
	def->max_collision_shapes = 16;
	def->collision_shape_count = 0;
	def->collision_shape = (collision_shape_t *) memory_Malloc(sizeof(collision_shape_t) * def->max_collision_shapes);
	def->flags = COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
	def->type = COLLIDER_TYPE_RIGID_BODY_COLLIDER;*/


    def = physics_CreateColliderDef(name);
    def_ptr = physics_GetColliderDefPointerHandle(def);

    if(def_ptr)
    {
        def_ptr->mass = 1.0;
        def_ptr->collision_shape = list_create(sizeof(struct collision_shape_t), 16, NULL);
        //def_ptr->max_collision_shapes = 16;
        //def_ptr->collision_shape = (collision_shape_t *) memory_Malloc(sizeof(struct collision_shape_t) * def_ptr->max_collision_shapes);
        def_ptr->def_flags = COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
        def_ptr->collider_type = COLLIDER_TYPE_RIGID_BODY_COLLIDER;
    }


	return def;
}

struct collider_handle_t physics_CreateCharacterColliderDef(char *name, float height, float crouch_height, float radius, float step_height, float slope_angle, float max_walk_speed, float mass)
{
    struct collider_handle_t def = INVALID_COLLIDER_HANDLE;
    struct collider_def_t *def_ptr;

    def = physics_CreateColliderDef(name);

    def_ptr = physics_GetColliderDefPointerHandle(def);

    if(def_ptr)
    {
        def_ptr->height = height;
        def_ptr->crouch_height = crouch_height;
        def_ptr->radius = radius;
        def_ptr->max_slope_angle = slope_angle;
        def_ptr->max_step_height = step_height;
        def_ptr->max_walk_speed = max_walk_speed;
        def_ptr->mass = mass;

        def_ptr->def_flags = COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
        def_ptr->collider_type = COLLIDER_TYPE_CHARACTER_COLLIDER;
    }
	/*collider_def_t *def;
	def = physics_CreateColliderDef(name);

	def->height = height;
	def->crouch_height = crouch_height;
	def->radius = radius;
	def->max_slope_angle = slope_angle;
	def->max_step_height = step_height;
	def->max_walk_speed = max_walk_speed;

	def->mass = mass;

	def->flags = COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
	def->type = COLLIDER_TYPE_CHARACTER_COLLIDER;*/

	return def;
}

struct collider_handle_t physics_CreateProjectileColliderDef(char *name, float radius, float mass)
{
	/*collider_def_t *def;

	def = physics_CreateColliderDef(name);


	def->radius = radius;
	def->mass = mass;
	def->flags = COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
	def->type = COLLIDER_TYPE_PROJECTILE_COLLIDER;

	return def;*/

	return INVALID_COLLIDER_HANDLE;
}

void physics_DestroyColliderDefByName(char *name)
{
	struct collider_handle_t def = physics_GetColliderDefByName(name);
    physics_DestroyColliderDef(def);
}

void physics_DestroyColliderDef(struct collider_handle_t def)
{
    physics_DestroyCollider(def);
}

void physics_DestroyColliderDefPointer(collider_def_t *def)
{
    #if 0
	btCollisionShape *collision_shape;
	btCompoundShape *compound_shape;
	int i;
	int c;

	if(def)
	{
		if(def->ref_count)
		{
			printf("physics_DestroyColliderDefPointer: def %s has %d references!\n", def->name, def->ref_count);
			return;

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

		if(def == phy_collider_defs)
		{
			phy_collider_defs = phy_collider_defs->next;

			if(phy_collider_defs)
			{
				phy_collider_defs->prev = NULL;
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
				phy_last_collider_def = phy_last_collider_def->prev;
			}
		}

		if(def->collision_shape)
		{
			memory_Free(def->collision_shape);
		}

		memory_Free(def);
	}

	#endif

}

void physics_DestroyColliderDefs()
{
    #if 0
	while(phy_collider_defs)
	{
		phy_last_collider_def = phy_collider_defs->next;
		physics_DestroyColliderDefPointer(phy_collider_defs);
		phy_collider_defs = phy_last_collider_def;
	}

	#endif
}

struct collider_handle_t physics_GetColliderDefByName(char *name)
{
    struct collider_def_t *collider_defs;
    struct collider_handle_t def = INVALID_COLLIDER_HANDLE;
    int i;
    int c;


    collider_defs = (struct collider_def_t *)phy_colliders[COLLIDER_TYPE_COLLIDER_DEF].elements;
    c = phy_colliders[COLLIDER_TYPE_COLLIDER_DEF].element_count;

    for(i = 0; i < c; i++)
    {
        if(collider_defs[i].base.flags & COLLIDER_FLAG_INVALID)
        {
            continue;
        }

        if(!strcmp(name, collider_defs[i].name))
        {
            //return collider_defs + i;
            def.type = COLLIDER_TYPE_COLLIDER_DEF;
            def.index = i;
            break;
        }
    }

    return def;
}

struct collider_def_t *physics_GetColliderDefPointer(char *name)
{
    struct collider_handle_t def;

    def = physics_GetColliderDefByName(name);

    return (struct collider_def_t *)physics_GetColliderPointer(def);
}

struct collider_def_t *physics_GetColliderDefPointerHandle(struct collider_handle_t def)
{
    return (struct collider_def_t *)physics_GetColliderPointer(def);
}

struct collider_def_t *physics_GetColliderDefsList(int *def_count)
{
    *def_count = phy_colliders[COLLIDER_TYPE_COLLIDER_DEF].element_count;
    return (struct collider_def_t *)phy_colliders[COLLIDER_TYPE_COLLIDER_DEF].elements;
}



#define MIN_COLLISION_SHAPE_SCALE 0.01

void physics_AddCollisionShape(struct collider_handle_t def_handle, vec3_t scale, vec3_t relative_position, mat3_t *relative_orientation, int type)
{
	int collision_shape_index;
	struct collision_shape_t *collision_shape;
	struct collision_shape_t *collision_shapes;

	struct collider_def_t *def = physics_GetColliderDefPointerHandle(def_handle);

	if(def)
	{
	    if(def->collider_type != COLLIDER_TYPE_RIGID_BODY_COLLIDER)
        {
            return;
        }

		switch(type)
		{
			case COLLISION_SHAPE_BOX:
			case COLLISION_SHAPE_CYLINDER:
			case COLLISION_SHAPE_SPHERE:
				/*collision_shape_index = def->collision_shape_count++;

				if(collision_shape_index >= def->max_collision_shapes)
				{
					collision_shape = (collision_shape_t *) memory_Malloc(sizeof(collision_shape_t ) * (def->max_collision_shapes + 16));
					memcpy(collision_shape, def->collision_shape, sizeof(collision_shape_t ) * def->max_collision_shapes);

					memory_Free(def->collision_shape);

					def->collision_shape = collision_shape;
					def->max_collision_shapes += 16;
				}*/

                collision_shape_index = list_add(&def->collision_shape, NULL);
                collision_shape = (struct collision_shape_t *) list_get(&def->collision_shape, collision_shape_index);

				if(scale.x < MIN_COLLISION_SHAPE_SCALE) scale.x = MIN_COLLISION_SHAPE_SCALE;
				if(scale.y < MIN_COLLISION_SHAPE_SCALE) scale.y = MIN_COLLISION_SHAPE_SCALE;
				if(scale.z < MIN_COLLISION_SHAPE_SCALE) scale.z = MIN_COLLISION_SHAPE_SCALE;

				if(relative_orientation)
				{
					collision_shape->orientation = *relative_orientation;
				}
				else
				{
					collision_shape->orientation = mat3_t_id();
				}

				collision_shape->position = relative_position;
				collision_shape->scale = scale;
				collision_shape->type = type;

				def->def_flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;

				/* updating ALL the colliders that use this
				shape every time something changes may become
				a bottleneck, but changing the collision shape
				at runtime is not intended... */
				physics_UpdateReferencingColliders(def_handle);
			break;

			default:

			return;
		}


	}
}



void physics_RemoveCollisionShape(struct collider_handle_t def_handle, int shape_index)
{
    struct collider_def_t *def = physics_GetColliderDefPointerHandle(def_handle);

	if(def)
	{
	    if(def->collider_type != COLLIDER_TYPE_RIGID_BODY_COLLIDER)
        {
            return;
        }

		if(shape_index >= 0 && shape_index < def->collision_shape.element_count)
		{
			/*if(shape_index < def->collision_shape_count - 1)
			{
				def->collision_shape[shape_index] = def->collision_shape[def->collision_shape_count - 1];
			}

			def->collision_shape_count--;*/

			list_remove(&def->collision_shape, shape_index);

			def->def_flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;

			physics_UpdateReferencingColliders(def_handle);
		}
	}
}


struct collision_shape_t *physics_GetCollisionShapePointer(struct collider_handle_t collider, int shape_index)
{
    struct collider_def_t *def;
    struct collision_shape_t *collision_shape = NULL;

    if(collider.type == COLLIDER_TYPE_COLLIDER_DEF)
    {
        def = physics_GetColliderDefPointerHandle(collider);

        if(def)
        {
            collision_shape = (struct collision_shape_t *)list_get(&def->collision_shape, shape_index);
        }
    }

    return collision_shape;
}


void physics_TranslateCollisionShape(struct collider_handle_t def_handle, vec3_t translation, int shape_index)
{
    struct collider_def_t *def = physics_GetColliderDefPointerHandle(def_handle);
    struct collision_shape_t *collision_shape;

	if(def)
	{

	    collision_shape = physics_GetCollisionShapePointer(def_handle, shape_index);

	    if(collision_shape)
        {
            collision_shape->position.x += translation.x;
			collision_shape->position.y += translation.y;
			collision_shape->position.z += translation.z;

			def->def_flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
			physics_UpdateReferencingColliders(def_handle);
        }

		/*if(shape_index >= 0 && shape_index < def->collision_shape_count)
		{
			def->collision_shape[shape_index].position.x += translation.x;
			def->collision_shape[shape_index].position.y += translation.y;
			def->collision_shape[shape_index].position.z += translation.z;

			def->def_flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
			physics_UpdateReferencingColliders(def_handle);
		}*/
	}
}

void physics_SetCollisionShapePosition(struct collider_handle_t def_handle, vec3_t position, int shape_index)
{
    struct collider_def_t *def = physics_GetColliderDefPointerHandle(def_handle);
    struct collision_shape_t *collision_shape;

	if(def)
	{
	    collision_shape = physics_GetCollisionShapePointer(def_handle, shape_index);

	    if(collision_shape)
        {
            collision_shape->position.x = position.x;
			collision_shape->position.y = position.y;
			collision_shape->position.z = position.z;

			def->def_flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
			physics_UpdateReferencingColliders(def_handle);
	    }

		/*if(shape_index >= 0 && shape_index < def->collision_shape_count)
		{
			def->collision_shape[shape_index].position.x = position.x;
			def->collision_shape[shape_index].position.y = position.y;
			def->collision_shape[shape_index].position.z = position.z;

			def->def_flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
			physics_UpdateReferencingColliders(def_handle);
		}*/
	}
}

void physics_RotateCollisionShape(struct collider_handle_t def_handle, vec3_t axis, float amount, int shape_index)
{
    struct collider_def_t *def = physics_GetColliderDefPointerHandle(def_handle);
    struct collision_shape_t *collision_shape;

	if(def)
	{
	    collision_shape = physics_GetCollisionShapePointer(def_handle, shape_index);

	    if(collision_shape)
        {
            mat3_t_rotate(&collision_shape->orientation, axis, amount, 0);
			def->def_flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
			physics_UpdateReferencingColliders(def_handle);
        }
		/*if(shape_index >= 0 && shape_index < def->collision_shape_count)
		{
			mat3_t_rotate(&def->collision_shape[shape_index].orientation, axis, amount, 0);
			def->def_flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
			physics_UpdateReferencingColliders(def_handle);
		}*/
	}
}

void physics_ScaleCollisionShape(struct collider_handle_t def_handle, vec3_t scale, int shape_index)
{
    struct collider_def_t *def = physics_GetColliderDefPointerHandle(def_handle);
    struct collision_shape_t *collision_shape;

	if(def)
	{
	    collision_shape = physics_GetCollisionShapePointer(def_handle, shape_index);

	    if(collision_shape)
        {
            collision_shape->scale.x += scale.x;
			collision_shape->scale.y += scale.y;
			collision_shape->scale.z += scale.z;

			if(collision_shape->scale.x < MIN_COLLISION_SHAPE_SCALE) collision_shape->scale.x = MIN_COLLISION_SHAPE_SCALE;
			if(collision_shape->scale.y < MIN_COLLISION_SHAPE_SCALE) collision_shape->scale.y = MIN_COLLISION_SHAPE_SCALE;
			if(collision_shape->scale.z < MIN_COLLISION_SHAPE_SCALE) collision_shape->scale.z = MIN_COLLISION_SHAPE_SCALE;

			def->def_flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
			physics_UpdateReferencingColliders(def_handle);
        }
		/*if(shape_index >= 0 && shape_index < def->collision_shape_count)
		{
			def->collision_shape[shape_index].scale.x += scale.x;
			def->collision_shape[shape_index].scale.y += scale.y;
			def->collision_shape[shape_index].scale.z += scale.z;

			if(def->collision_shape[shape_index].scale.x < MIN_COLLISION_SHAPE_SCALE) def->collision_shape[shape_index].scale.x = MIN_COLLISION_SHAPE_SCALE;
			if(def->collision_shape[shape_index].scale.y < MIN_COLLISION_SHAPE_SCALE) def->collision_shape[shape_index].scale.y = MIN_COLLISION_SHAPE_SCALE;
			if(def->collision_shape[shape_index].scale.z < MIN_COLLISION_SHAPE_SCALE) def->collision_shape[shape_index].scale.z = MIN_COLLISION_SHAPE_SCALE;

			def->def_flags |= COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
			physics_UpdateReferencingColliders(def_handle);
		}*/
	}
}




void *physics_BuildCollisionShape(struct collider_handle_t def_handle)
{
	int i;
	int c;

	btCompoundShape *compound_shape = NULL;
	btCollisionShape *collision_shape;
	struct collision_shape_t *def_collision_shape;
	btTransform collision_shape_transform;
	btVector3 inertia_tensor;
	//btMatrix3x3 basis;
	mat3_t *rot;

	struct collider_def_t *def = physics_GetColliderDefPointerHandle(def_handle);

	if(def)
	{
		/* would like to share collision shapes, but this wouldn't allow
		them to be scaled on a per instance basis... */
		switch(def->collider_type)
		{
			case COLLIDER_TYPE_RIGID_BODY_COLLIDER:
				if(def->collision_shape.element_count > 1)
				{
					compound_shape = new btCompoundShape();
					for(i = 0; i < def->collision_shape.element_count; i++)
					{

						//def_collision_shape = &def->collision_shape[i];

						def_collision_shape = physics_GetCollisionShapePointer(def_handle, i);

						collision_shape_transform.setIdentity();
						collision_shape_transform.setOrigin(btVector3(def_collision_shape->position.x, def_collision_shape->position.y, def_collision_shape->position.z));
						collision_shape_transform.setBasis(&def_collision_shape->orientation.floats[0][0]);

						/*rot = &def_collision_shape->orientation;

						collision_shape_transform.setBasis(btMatrix3x3(rot->floats[0][0], rot->floats[1][0], rot->floats[2][0],
                                                                       rot->floats[0][1], rot->floats[1][1], rot->floats[2][1],
                                                                       rot->floats[0][2], rot->floats[1][2], rot->floats[2][2]));*/

						switch(def_collision_shape->type)
						{
							case COLLISION_SHAPE_BOX:
								compound_shape->addChildShape(collision_shape_transform, new btBoxShape(btVector3(def_collision_shape->scale.x, def_collision_shape->scale.y, def_collision_shape->scale.z)));
							break;

							case COLLISION_SHAPE_CYLINDER:
								compound_shape->addChildShape(collision_shape_transform, new btCylinderShape(btVector3(def_collision_shape->scale.x, def_collision_shape->scale.y, def_collision_shape->scale.z)));
							break;

							case COLLISION_SHAPE_SPHERE:
								compound_shape->addChildShape(collision_shape_transform, new btSphereShape(def_collision_shape->scale.x));
							break;
						}
					}
					collision_shape = compound_shape;
				}
				else if(def->collision_shape.element_count == 1)
				{
					//def_collision_shape = &def->collision_shape[0];

					def_collision_shape = physics_GetCollisionShapePointer(def_handle, 0);
					switch(def_collision_shape->type)
					{
						case COLLISION_SHAPE_BOX:
							collision_shape = new btBoxShape(btVector3(def_collision_shape->scale.x, def_collision_shape->scale.y, def_collision_shape->scale.z));
						break;

						case COLLISION_SHAPE_CYLINDER:
							collision_shape = new btCylinderShape(btVector3(def_collision_shape->scale.x, def_collision_shape->scale.y, def_collision_shape->scale.z));
						break;

						case COLLISION_SHAPE_SPHERE:
							collision_shape = new btSphereShape(def_collision_shape->scale.x);
						break;
					}
				}
				else
				{
					return NULL;
				}
			break;

			case COLLIDER_TYPE_CHARACTER_COLLIDER:
				collision_shape = new btCapsuleShape(def->radius, def->height - 2.0 * def->radius);
				//collision_shape->setMargin(0.8);
				//collision_shape = new btBoxShape(btVector3(def->radius, def->height * 0.5, def->radius));
			break;

			case COLLIDER_TYPE_PROJECTILE_COLLIDER:
				collision_shape = new btSphereShape(def->radius);
			break;
		}

		/* still not sure if it is necessary to recalculate this
		once the collision shape gets scaled...  */
		if(def->def_flags & COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR)
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

			def->def_flags &= ~COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR;
		}
	}

	return collision_shape;
}

void physics_IncColliderDefRefCount(struct collider_handle_t def_handle)
{
    struct collider_def_t *def;

    def = physics_GetColliderDefPointerHandle(def_handle);

	if(def)
	{
		def->ref_count++;
	}
}

void physics_DecColliderDefRefCount(struct collider_handle_t def_handle)
{
    struct collider_def_t *def;

    def = physics_GetColliderDefPointerHandle(def_handle);

	if(def)
	{
		if(def->ref_count > 0)
		{
			def->ref_count--;
		}
	}
}

void physics_UpdateReferencingColliders(struct collider_handle_t def_handle)
{
	int i;
	int type;

	int count;

	btRigidBody *rigid_body;
	btCollisionShape *collision_shape;

	struct collider_t *colliders;
	struct collider_t *collider;
	struct collider_def_t *def;

    def = physics_GetColliderDefPointerHandle(def_handle);


	if(def)
	{
		if(!def->ref_count)
			return;

	//	for(type = 0; type < COLLIDER_TYPE_LAST; type++)
	//	{
		colliders = (struct collider_t *)phy_colliders[def->collider_type].elements;
		count = phy_colliders[def->collider_type].element_count;

		for(i = 0; i < count; i++)
		{
			collider = colliders + i;

			if(collider->flags & COLLIDER_FLAG_INVALID)
			{
				continue;
			}

			/*if(collider->def != def)
			{
				continue;
			}*/


			if(!(collider->flags & COLLIDER_FLAG_NO_SCALE_HINT))
			{
				/* make a copy of the collision shape, so this collider can be freely scaled... */
				collision_shape = (btCollisionShape *) physics_BuildCollisionShape(def_handle);
				rigid_body = (btRigidBody *) collider->collision_object;

				/* get rid of the outdated copy this collider has... */
				physics_DestroyCollisionShape(rigid_body->getCollisionShape());

				rigid_body->setCollisionShape(collision_shape);
				rigid_body->setMassProps(def->mass, btVector3(def->inertia_tensor.x, def->inertia_tensor.y, def->inertia_tensor.z));
			}

			rigid_body->setAngularVelocity(btVector3(0.0, 0.0, 0.0));
			rigid_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
			rigid_body->activate(true);

			collider->flags |= COLLIDER_FLAG_UPDATE_COLLISION_OBJECT;
		}

	//	}

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

	if(coll_shape)
	{
		if(coll_shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
		{
			compound_shape = (btCompoundShape *) coll_shape;
			c = compound_shape->getNumChildShapes();

		/*	for(i = 0; i < c; i++)
			{
				delete compound_shape->getChildShape(0);
			}*/

		//	for(i = c - 1; i >= 0; i--)
		//	{
		//		delete compound_shape->getChildShape(i);
		//	}
		}

//		delete coll_shape;
	}
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
		case COLLIDER_TYPE_TRIGGER_COLLIDER:
        case COLLIDER_TYPE_COLLIDER_DEF:
			collider_index = stack_list_add(&phy_colliders[type], NULL);
		break;

		default:
			return handle;
		break;
	}

	collider = (struct collider_t *)stack_list_get(&phy_colliders[type], collider_index);

	memset(collider, 0, collider_type_size[type]);

	handle.type = type;
	handle.index = collider_index;

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
	collider->type = type;
	collider->collision_object = NULL;
	collider->def = INVALID_COLLIDER_HANDLE;
	collider->max_contact_records = 32;
	collider->contact_record_count = 0;
	collider->first_contact_record = 0;
	//collider->collision_shape = NULL;

	return handle;

	//return collider_index;
}

struct collider_handle_t physics_CreateCollider(mat3_t *orientation, vec3_t position, vec3_t scale, struct collider_handle_t def_handle, int flags)
{
	int collider_index;
	struct collider_handle_t handle;

	struct collider_t *collider;
	struct character_collider_t *character_collider;
	struct trigger_collider_t *trigger_collider;
	struct collider_def_t *def;


	btCollisionShape *collider_collision_shape;
	btRigidBody *collider_rigid_body;
	btCollisionObject *collider_collision_object;
	btTransform collider_transform;
	btDefaultMotionState *collider_motion_state;

	handle.type = COLLIDER_TYPE_NONE;
	handle.index = INVALID_COLLIDER_INDEX;

	def = physics_GetColliderDefPointerHandle(def_handle);

	if(!def)
	{
		printf("physics_CreateCollider: bad def\n");
		return handle;
	}


	//collider_index = physics_CreateEmptyCollider();
	//collider = &colliders[collider_index];


	if((flags & COLLIDER_FLAG_NO_SCALE_HINT) && def->cached_collision_shape)
	{
		collider_collision_shape = (btCollisionShape *)def->cached_collision_shape;
	}
	else
	{
		collider_collision_shape = (btCollisionShape *) physics_BuildCollisionShape(def_handle);
	}

	if(!collider_collision_shape)
	{
		printf("physics_CreateCollider: null collision shape\n");
		return handle;
	}


	handle = physics_CreateEmptyCollider(def->collider_type);
	collider = physics_GetColliderPointer(handle);

	collider_transform.setIdentity();
	collider_transform.setOrigin(btVector3(position.x, position.y, position.z));

	collider->position = position;
	collider->scale = scale;

	if(orientation && def->collider_type != COLLIDER_TYPE_CHARACTER_COLLIDER)
	{
		collider->orientation = *orientation;
		/*collider_transform.setBasis(btMatrix3x3(orientation->floats[0][0], orientation->floats[1][0], orientation->floats[2][0],
                                                orientation->floats[0][1], orientation->floats[1][1], orientation->floats[2][1],
                                                orientation->floats[0][2], orientation->floats[1][2], orientation->floats[2][2]));*/
		collider_transform.setBasis((float *)orientation);
	}

	collider->type = def->collider_type;
	collider->flags = COLLIDER_FLAG_UPDATE_COLLISION_OBJECT;
	collider->def = def_handle;

	collider_motion_state = new btDefaultMotionState(collider_transform);

	collider_rigid_body = new btRigidBody(def->mass, collider_motion_state, collider_collision_shape, btVector3(def->inertia_tensor.x, def->inertia_tensor.y, def->inertia_tensor.z));

   /* if(def->mass == 0.0)
    {
        collider_rigid_body->setFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
    }*/

    switch(def->collider_type)
	{
		case COLLIDER_TYPE_CHARACTER_COLLIDER:
			character_collider = (struct character_collider_t *)collider;
			character_collider->height = def->height;
			character_collider->radius = def->radius;
			character_collider->max_slope_angle = def->max_slope_angle;
			character_collider->max_walk_speed = def->max_walk_speed;
            character_collider->max_step_height = def->max_step_height;
            collider_rigid_body->setActivationState(DISABLE_DEACTIVATION);
        case COLLIDER_TYPE_RIGID_BODY_COLLIDER:

		break;
	}




	physics_world->addRigidBody(collider_rigid_body);

	collider_rigid_body->setWorldTransform(collider_transform);
	collider_rigid_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
	collider_rigid_body->setAngularVelocity(btVector3(0.0, 0.0, 0.0));
	collider_rigid_body->setUserIndex(*(int *)&handle);


	if(def->collider_type == COLLIDER_TYPE_CHARACTER_COLLIDER || def->collider_type == COLLIDER_TYPE_PROJECTILE_COLLIDER)
	{
		/* a character controller is a capsule that can't rotate... */
		collider_rigid_body->setAngularFactor(btVector3(0.0, 0.0, 0.0));
	}


	collider_rigid_body->setSleepingThresholds(0.05, 0.05);
	collider_rigid_body->activate(true);

	collider->collision_object = collider_rigid_body;

	return handle;
}

struct collider_handle_t physics_CreateTrigger(mat3_t *orientation, vec3_t position, vec3_t scale)
{
	int trigger_index;
	struct collider_handle_t handle = INVALID_COLLIDER_HANDLE;
	struct trigger_collider_t *trigger;

	//btGhostObject *ghost_object;
	btCollisionObject *collision_object;
	btRigidBody *rigid_body;
	btCollisionShape *collision_shape;

	btDefaultMotionState *motion_state;
	btTransform transform;

    handle = physics_CreateEmptyCollider(COLLIDER_TYPE_TRIGGER_COLLIDER);

    trigger = (struct trigger_collider_t *)physics_GetColliderPointer(handle);

    if(orientation)
	{
		memcpy(&trigger->base.orientation, orientation, sizeof(mat3_t));
	}
	else
	{
		trigger->base.orientation = mat3_t_id();
	}

	trigger->base.position = position;
	trigger->base.scale = scale;
	trigger->base.type = COLLIDER_TYPE_TRIGGER_COLLIDER;

	collision_shape = new btBoxShape(btVector3(1.0, 1.0, 1.0));
	collision_object = new btCollisionObject();
	collision_object->setCollisionShape(collision_shape);
	collision_object->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
	collision_object->setActivationState(DISABLE_DEACTIVATION);
	collision_object->setUserIndex(*(int *)&handle);

	//transform.setIdentity();

	//motion_state = new btDefaultMotionState();

	//motion_state->setWorldTransform(transform);

	//rigid_body = new btRigidBody(0.0, motion_state, collision_shape);



	//ghost_object = new btGhostObject();
	//ghost_object->setCollisionShape(collision_shape);

	trigger->base.collision_object = collision_object;


	physics_world->addCollisionObject(collision_object, 0xffff, 0xffff);
	//physics_world->addRigidBody(collision_object->setCollisionShape(collision_shape););

	trigger->base.flags |= COLLIDER_FLAG_UPDATE_COLLISION_OBJECT;

	return handle;
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

void physics_DestroyCollider(struct collider_handle_t collider)
{
	btRigidBody *collider_rigid_body;
	btCollisionObject *collider_ghost_object;
	struct collider_t *collider_ptr;
	btCollisionShape *collider_collision_shape;
	struct collider_def_t *collider_def;
	struct stack_list_t *list;


	collider_ptr = physics_GetColliderPointer(collider);

	if(collider_ptr)
	{
		collider_ptr->flags |= COLLIDER_FLAG_INVALID;

		switch(collider_ptr->type)
		{
			case COLLIDER_TYPE_RIGID_BODY_COLLIDER:
			case COLLIDER_TYPE_CHARACTER_COLLIDER:
				collider_rigid_body = (btRigidBody *)collider_ptr->collision_object;
				collider_collision_shape = collider_rigid_body->getCollisionShape();
				physics_world->removeRigidBody(collider_rigid_body);
				delete collider_rigid_body;
			break;

			case COLLIDER_TYPE_TRIGGER_COLLIDER:
				collider_ghost_object = (btCollisionObject *)collider_ptr->collision_object;
				collider_collision_shape = collider_ghost_object->getCollisionShape();
				physics_world->removeCollisionObject(collider_ghost_object);
				delete collider_ghost_object;
			break;

			case COLLIDER_TYPE_COLLIDER_DEF:
			    collider_def = (struct collider_def_t *)collider_ptr;
			    list_destroy(&collider_def->collision_shape);
                //memory_Free(collider_def->collision_shape);
            break;
		}

		if(collider_ptr->type != COLLIDER_TYPE_COLLIDER_DEF)
        {
            physics_DestroyCollisionShape(collider_collision_shape);
            physics_DecColliderDefRefCount(collider_ptr->def);
        }

		collider_ptr->collision_object = NULL;
		collider_ptr->def = INVALID_COLLIDER_HANDLE;

		stack_list_remove(&phy_colliders[collider.type], collider.index);
	}

}

struct collider_t *physics_GetColliderPointer(struct collider_handle_t collider)
{
	stack_list_t *list;
	struct collider_t *collider_ptr;

	if(collider.type != COLLIDER_TYPE_NONE)
	{
		list = &phy_colliders[collider.type];

		if(collider.index >= 0 && collider.index < list->element_count)
		{
			collider_ptr = (struct collider_t *)((char *)list->elements + collider.index * list->element_size);

			if(!(collider_ptr->flags & COLLIDER_FLAG_INVALID))
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
        if(collider.type != COLLIDER_TYPE_TRIGGER_COLLIDER)
		{
            collider_ptr = physics_GetColliderPointer(collider);

			if(collider_ptr)
			{
				rigid_body = (btRigidBody *) collider_ptr->collision_object;
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

	collider_ptr = physics_GetColliderPointer(collider);

	if(collider_ptr)
	{
        collider_ptr->position = position;
		collider_ptr->flags |= COLLIDER_FLAG_UPDATE_COLLISION_OBJECT;

		//printf("collider position: [%f %f %f]\n", position.x, position.y, position.z);
	}
}

void physics_SetColliderOrientation(struct collider_handle_t collider, mat3_t *orientation)
{
	struct collider_t *collider_ptr;
	struct stack_list_t *list;

    collider_ptr = physics_GetColliderPointer(collider);

    if(collider_ptr)
	{
        collider_ptr->orientation = *orientation;
		collider_ptr->flags |= COLLIDER_FLAG_UPDATE_COLLISION_OBJECT;
	}
}

void physics_SetColliderScale(struct collider_handle_t collider, vec3_t scale)
{
	struct collider_t *collider_ptr;
	struct stack_list_t *list;

	collider_ptr = physics_GetColliderPointer(collider);

	if(collider_ptr)
	{
        if(collider_ptr->flags & COLLIDER_FLAG_NO_SCALE_HINT)
		{
			return;
		}

		collider_ptr->scale = scale;
		collider_ptr->flags |= COLLIDER_FLAG_UPDATE_COLLISION_OBJECT;
	}

}

void physics_SetColliderVelocity(struct collider_handle_t collider, vec3_t velocity)
{
	struct rigid_body_collider_t *rigid_body_collider;

	if(collider.type == COLLIDER_TYPE_RIGID_BODY_COLLIDER)
	{
        rigid_body_collider = (struct rigid_body_collider_t *)physics_GetColliderPointer(collider);

		if(rigid_body_collider)
		{
            rigid_body_collider->linear_velocity = velocity;
            rigid_body_collider->base.flags |= COLLIDER_FLAG_UPDATE_COLLISION_OBJECT;
		}
	}
}

void physics_SetColliderMass(struct collider_handle_t collider, float mass)
{
    struct rigid_body_collider_t *collider_ptr;
    btRigidBody *rigid_body;
    btCollisionShape *collision_shape;

    btVector3 inertia_tensor;


	if(collider.type == COLLIDER_TYPE_RIGID_BODY_COLLIDER)
	{
		collider_ptr = (struct rigid_body_collider_t *)physics_GetColliderPointer(collider);

		if(collider_ptr)
		{
			collider_ptr->base.flags &= ~COLLIDER_FLAG_STATIC;

			if(mass <= 0.0)
			{
				mass = 0.0;
				collider_ptr->base.flags |= COLLIDER_FLAG_STATIC;
			}

			rigid_body = (btRigidBody *)collider_ptr->base.collision_object;

			collision_shape = rigid_body->getCollisionShape();

			collision_shape->calculateLocalInertia(mass, inertia_tensor);

			rigid_body->setMassProps(mass, inertia_tensor);
		}
	}
}

void physics_SetColliderStatic(struct collider_handle_t collider, int set)
{
    /*struct collider_t *collider_ptr;

    if(collider.type != COLLIDER_TYPE_RIGID_BODY_COLLIDER)
	{
		return;
	}

    collider_ptr = physics_GetColliderPointerHandle(collider);

    if(collider_ptr)
	{
        if(set)
		{
			physics_SetColliderMass(collider, 0.0);
		}
		else
		{
			physics_SetColliderMass(collider, collider_ptr->def->mass);
		}
	}*/
}

void physics_ApplyCentralForce(struct collider_handle_t collider, vec3_t force)
{
	struct collider_t *collider_ptr;
	btRigidBody *rigid_body;

	collider_ptr = physics_GetColliderPointer(collider);

	if(collider_ptr)
	{
		rigid_body = (btRigidBody *)collider_ptr->collision_object;
		rigid_body->activate(true);
		rigid_body->applyCentralForce(btVector3(force.x, force.y, force.z));
	}
}

void physics_ApplyCentralImpulse(struct collider_handle_t collider, vec3_t impulse)
{
	struct collider_t *collider_ptr;
	btRigidBody *rigid_body;


	if(collider.type == COLLIDER_TYPE_RIGID_BODY_COLLIDER)
	{
		collider_ptr = physics_GetColliderPointer(collider);

		if(collider_ptr)
		{
			rigid_body = (btRigidBody *)collider_ptr->collision_object;
			rigid_body->activate(true);
			rigid_body->applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
		}
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
	struct collider_def_t *def;
	struct rigid_body_collider_t *rigid_body_collider;
	struct trigger_collider_t *trigger_collider;
	struct stack_list_t *list;
	struct collider_handle_t collider_handle;
	btRigidBody *rigid_body;
	btGhostObject *ghost_object;
	btCollisionObject *collision_object;
	btCollisionShape *collision_shape;
	btTransform collision_object_transform;
	mat3_t *rot;

	for(j = 0; j < COLLIDER_TYPE_LAST; j++)
	{

		list = &phy_colliders[j];
		collider_count = list->element_count;

		collider_handle.type = j;

		for(i = 0; i < collider_count; i++)
		{
			collider_handle.index = i;
			collider = physics_GetColliderPointer(collider_handle);

			if(!collider)
			{
				continue;
			}

			if(!(collider->flags & COLLIDER_FLAG_UPDATE_COLLISION_OBJECT))
			{
				continue;
			}

			collision_object_transform.setIdentity();
			collision_object_transform.setOrigin(btVector3(collider->position.x, collider->position.y, collider->position.z));

            switch(collider->type)
			{
				case COLLIDER_TYPE_RIGID_BODY_COLLIDER:
				case COLLIDER_TYPE_CHARACTER_COLLIDER:
					rigid_body = (btRigidBody *)collider->collision_object;


					if(collider->type == COLLIDER_TYPE_RIGID_BODY_COLLIDER)
					{
						collision_object_transform.setBasis(&collider->orientation.floats[0][0]);

						if(!(collider->flags & COLLIDER_FLAG_NO_SCALE_HINT))
						{
							collision_shape = rigid_body->getCollisionShape();

							collision_shape->setLocalScaling(btVector3(collider->scale.x, collider->scale.y, collider->scale.z));
						}
					}

					rigid_body->setWorldTransform(collision_object_transform);

					if(collider->type == COLLIDER_TYPE_RIGID_BODY_COLLIDER)
					{
						rigid_body_collider = (struct rigid_body_collider_t *)collider;

						def = physics_GetColliderDefPointerHandle(rigid_body_collider->base.def);

						if(def->mass == 0.0)
                        {
                            rigid_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
                        }
                        else
                        {
                            rigid_body->setLinearVelocity(btVector3(rigid_body_collider->linear_velocity.x, rigid_body_collider->linear_velocity.y, rigid_body_collider->linear_velocity.z));
                        }
						//rigid_body->setLinearVelocity(btVector3(rigid_body_collider->linear_velocity.x, rigid_body_collider->linear_velocity.y, rigid_body_collider->linear_velocity.z));
						//rigid_body->setAngularVelocity(btVector3(0.0, 0.0, 0.0));
					}

					rigid_body->activate(true);

				break;

				case COLLIDER_TYPE_TRIGGER_COLLIDER:
					trigger_collider = (struct trigger_collider_t *)collider;
					//ghost_object = (btGhostObject *)collider->collision_object;

					collision_object = (btCollisionObject *) collider->collision_object;
                    rot = &collider->orientation;

					collision_object_transform.setBasis(&collider->orientation.floats[0][0]);
					/*collision_object_transform.setBasis(btMatrix3x3(rot->floats[0][0], rot->floats[1][0], rot->floats[2][0],
                                                                    rot->floats[0][1], rot->floats[1][1], rot->floats[2][1],
                                                                    rot->floats[0][2], rot->floats[1][2], rot->floats[2][2]));*/

					collision_shape = collision_object->getCollisionShape();
					//collision_shape->setLocalScaling()
					collision_shape->setLocalScaling(btVector3(collider->scale.x, collider->scale.y, collider->scale.z));

					collision_object->setWorldTransform(collision_object_transform);

					collision_object->activate(true);

				break;
			}



			collider->flags &= ~COLLIDER_FLAG_UPDATE_COLLISION_OBJECT;
		}
	}


}

void physics_PostUpdateColliders()
{
	int i;
	int j;
	int k;
	int collider_type;
	int contact_point_index;
	int contact_point_count;
	int collider_count;

	int prev_start = 0;
	int contact_record_index;
	int total_contact_record_count;

	struct collider_t *collider;
	struct rigid_body_collider_t *rigid_body_collider;
	struct stack_list_t *list;
	struct collider_handle_t handle;
	btRigidBody *collider_rigid_body;
	btGhostObject *collider_ghost_object;
	btCollisionObject *collider_collision_object;
	btTransform collider_rigid_body_transform;
	btVector3 collider_rigid_body_position;
	btVector3 collider_rigid_body_linear_velocity;
	btMatrix3x3 collider_rigid_body_orientation;
	btPersistentManifold **persistent_manifolds;
	btPersistentManifold *persistent_manifold;

	int triangle_index;


	vec3_t *triangles;

	PHY_ScalarType vert_type;
	PHY_ScalarType index_type;

	int vert_count;
	int vert_stride;

	int *indexes;
	int index_count;
	int index_stride;


	vec3_t a;
	vec3_t b;
	vec3_t c;

	vec3_t e0;
	vec3_t e1;
	vec3_t contact_normal;
	vec3_t contact_position;

	//btManifoldPoint &contact_point = NULL;

	struct contact_record_t *contact_records;

	btCollisionObject *body0;
	btCollisionObject *body1;

	struct collider_t *collider0;
	struct collider_t *collider1;

	struct collider_t *colliders[2];

	int handle0;
	int handle1;

	struct collider_handle_t collider_handle;

	float step_delta;


	void (*physics_UpdateColliderFunction)(struct collider_handle_t collider);


	/* "realloc" contacts... */
//	for(collider_type = 0; collider_type < COLLIDER_TYPE_LAST; collider_type++)
//	{
//        list = &phy_colliders[collider_type];
//		collider_count = list->element_count;
//
//		for(i = 0; i < collider_count; i++)
//		{
//			collider = (struct collider_t *)((char *)list->elements + i * list->element_size);
//
//			if(collider->flags & COLLIDER_FLAG_INVALID)
//			{
//				continue;
//			}
//
//			if(collider->flags & COLLIDER_FLAG_DONT_TRACK_COLLISIONS)
//			{
//				continue;
//			}
//
//			if(collider->contact_record_count > collider->max_contact_records)
//			{
//				j = collider->contact_record_count;
//				collider->max_contact_records = (j + 3) & (~3);
//			}
//
//			collider->contact_record_count = 0;
//			collider->first_contact_record = prev_start;
//
//			prev_start += collider->max_contact_records;
//		}
//	}
//
//	if(prev_start > phy_contact_records.max_elements)
//	{
//		list_resize(&phy_contact_records, prev_start);
//	}

    /*
    ===================================================================
    ===================================================================
    ===================================================================
    */

    /* find out how many contact points there are in total, and
    keep in each collider how many contact points involve them... */
	j = narrow_phase->getNumManifolds();

	persistent_manifolds = narrow_phase->getInternalManifoldPointer();

    total_contact_record_count = 0;

	if(persistent_manifolds)
    {
        for(i = 0; i < j; i++)
        {
            persistent_manifold = persistent_manifolds[i];

            body0 = (btCollisionObject *)persistent_manifold->getBody0();
			body1 = (btCollisionObject *)persistent_manifold->getBody1();

			handle0 = body0->getUserIndex();
			handle1 = body1->getUserIndex();

			collider0 = physics_GetColliderPointer(*(struct collider_handle_t *)&handle0);
			collider1 = physics_GetColliderPointer(*(struct collider_handle_t *)&handle1);

			contact_point_count = persistent_manifold->getNumContacts();

            /* both collider0 and collider1 add to the total
            contact count... */

			if(collider0)
            {
                if(!(collider0->flags & COLLIDER_FLAG_DONT_TRACK_COLLISIONS))
                {
                    if(collider0->first_contact_record != 0xffffffff)
                    {
                        collider0->first_contact_record = 0xffffffff;
                        collider0->contact_record_count = 0;
                    }

                    collider0->contact_record_count += contact_point_count;

                    total_contact_record_count += contact_point_count;
                }
            }


            if(collider1)
            {
                if(!(collider1->flags & COLLIDER_FLAG_DONT_TRACK_COLLISIONS))
                {
                    if(collider1->first_contact_record != 0xffffffff)
                    {
                        collider1->first_contact_record = 0xffffffff;
                        collider1->contact_record_count = 0;
                    }

                    collider1->contact_record_count += contact_point_count;

                    total_contact_record_count += contact_point_count;
                }
            }
        }
    }

    if(total_contact_record_count > phy_contact_records.max_elements)
    {
        /* make sure we have enough space to store all contact points... */
        list_resize(&phy_contact_records, total_contact_record_count);
    }

    contact_records = (struct contact_record_t *)phy_contact_records.elements;

    /*
    ===================================================================
    ===================================================================
    ===================================================================
    */

    /* "alloc" contact records for each collider... */
    contact_point_count = 0;

    for(collider_type = 0; collider_type < COLLIDER_TYPE_LAST; collider_type++)
	{
        list = &phy_colliders[collider_type];
		collider_count = list->element_count;

		for(i = 0; i < collider_count; i++)
		{
			collider = (struct collider_t *)((char *)list->elements + i * list->element_size);

			if(collider->flags & COLLIDER_FLAG_INVALID)
			{
				continue;
			}

			if(collider->flags & COLLIDER_FLAG_DONT_TRACK_COLLISIONS)
			{
				continue;
			}

			collider->first_contact_record = contact_point_count;
			contact_point_count += collider->contact_record_count;
			collider->contact_record_count = 0;
		}
	}



	if(persistent_manifolds && world_triangles)
	{
	    /* generate the actual contact records from contact
	    points from bullet... */
		triangles = (vec3_t *)&world_triangles->m_3componentVertices[0];

		for(i = 0; i < j; i++)
		{
			persistent_manifold = persistent_manifolds[i];

			body0 = (btCollisionObject *)persistent_manifold->getBody0();
			body1 = (btCollisionObject *)persistent_manifold->getBody1();

			handle0 = body0->getUserIndex();
			handle1 = body1->getUserIndex();

			collider0 = physics_GetColliderPointer(*(struct collider_handle_t *)&handle0);
			collider1 = physics_GetColliderPointer(*(struct collider_handle_t *)&handle1);

			contact_point_count = persistent_manifold->getNumContacts();

			for(contact_point_index = 0; contact_point_index < contact_point_count; contact_point_index++)
			{
				btManifoldPoint &contact_point = persistent_manifold->getContactPoint(contact_point_index);

				contact_position.x = contact_point.m_positionWorldOnA[0];
				contact_position.y = contact_point.m_positionWorldOnA[1];
				contact_position.z = contact_point.m_positionWorldOnA[2];

				if(!(collider0 && collider1))
				{
				    /* if one of the two colliders is null, it means
				    we have a collision against the world... */
					if(!collider0)
					{
						triangle_index = contact_point.m_index0 * 3;
					}
					else
					{
						triangle_index = contact_point.m_index1 * 3;
					}


					e0.x = triangles[triangle_index + 1].x - triangles[triangle_index].x;
                    e0.y = triangles[triangle_index + 1].y - triangles[triangle_index].y;
                    e0.z = triangles[triangle_index + 1].z - triangles[triangle_index].z;

                    e1.x = triangles[triangle_index + 2].x - triangles[triangle_index + 1].x;
                    e1.y = triangles[triangle_index + 2].y - triangles[triangle_index + 1].y;
                    e1.z = triangles[triangle_index + 2].z - triangles[triangle_index + 1].z;

                    contact_normal = cross(e0, e1);
                    contact_normal = normalize3(contact_normal);
				}
				else
				{
					contact_normal.x = contact_point.m_normalWorldOnB[0];
					contact_normal.y = contact_point.m_normalWorldOnB[1];
					contact_normal.z = contact_point.m_normalWorldOnB[2];
				}



				if(collider0)
				{
					if(!(collider0->flags & COLLIDER_FLAG_DONT_TRACK_COLLISIONS))
					{
						if(collider0->contact_record_count < collider0->max_contact_records)
						{
							contact_record_index = collider0->first_contact_record + collider0->contact_record_count;

							if(body1)
							{
								contact_records[contact_record_index].collider = *(struct collider_handle_t *)&handle1;
							}
							else
							{
								contact_records[contact_record_index].collider = INVALID_COLLIDER_HANDLE;
							}

							contact_records[contact_record_index].normal = contact_normal;
							contact_records[contact_record_index].position = contact_position;
							contact_records[contact_record_index].life = contact_point.m_lifeTime;

						}
						collider0->contact_record_count++;
					}
				}


				if(collider1)
				{
					if(!(collider1->flags & COLLIDER_FLAG_DONT_TRACK_COLLISIONS))
					{
						if(collider1->contact_record_count < collider1->max_contact_records)
						{
							contact_record_index = collider1->first_contact_record + collider1->contact_record_count;

							if(body0)
							{
								contact_records[contact_record_index].collider = *(struct collider_handle_t *)&handle0;
							}
							else
							{
								contact_records[contact_record_index].collider = INVALID_COLLIDER_HANDLE;
							}

							contact_records[contact_record_index].normal = contact_normal;
							contact_records[contact_record_index].position = contact_position;
							contact_records[contact_record_index].life = contact_point.m_lifeTime;
						}

						collider1->contact_record_count++;
					}
				}
			}
		}
	}

	for(collider_type = 0; collider_type < COLLIDER_TYPE_LAST; collider_type++)
	{
		list = &phy_colliders[collider_type];
		collider_count = list->element_count;

		switch(collider_type)
		{
			case COLLIDER_TYPE_CHARACTER_COLLIDER:
				physics_UpdateColliderFunction = physics_UpdateCharacterCollider;
			break;

			case COLLIDER_TYPE_PROJECTILE_COLLIDER:
				physics_UpdateColliderFunction = physics_UpdateProjectileCollider;
			break;

			case COLLIDER_TYPE_RIGID_BODY_COLLIDER:
				continue;
			break;

			case COLLIDER_TYPE_TRIGGER_COLLIDER:
				continue;
			break;
		}

		handle.type = collider_type;

		for(i = 0; i < collider_count; i++)
		{
			collider = (struct collider_t *)((char *)list->elements + i * list->element_size);

			if(collider->flags & COLLIDER_FLAG_INVALID)
			{
				continue;
			}

			handle.index = i;
			physics_UpdateColliderFunction(handle);
		}
	}


	for(collider_type = 0; collider_type < COLLIDER_TYPE_LAST; collider_type++)
	{
		list = &phy_colliders[collider_type];
		collider_count = list->element_count;

		switch(collider_type)
		{
			case COLLIDER_TYPE_CHARACTER_COLLIDER:
			case COLLIDER_TYPE_PROJECTILE_COLLIDER:
			case COLLIDER_TYPE_RIGID_BODY_COLLIDER:
				for(i = 0; i < collider_count; i++)
				{
					collider = (struct collider_t *)((char *)list->elements + i * list->element_size);

					if(collider->flags & COLLIDER_FLAG_INVALID)
					{
						continue;
					}

					collider_rigid_body = (btRigidBody *)collider->collision_object;

					collider_rigid_body_transform = collider_rigid_body->getWorldTransform();
					collider_rigid_body_position = collider_rigid_body_transform.getOrigin();
					collider_rigid_body_orientation = collider_rigid_body_transform.getBasis();
					collider_rigid_body_linear_velocity = collider_rigid_body->getLinearVelocity();

					collider->position.x = collider_rigid_body_position[0];
					collider->position.y = collider_rigid_body_position[1];
					collider->position.z = collider_rigid_body_position[2];

					if(collider_type == COLLIDER_TYPE_RIGID_BODY_COLLIDER)
					{

						rigid_body_collider = (struct rigid_body_collider_t *)collider;

						rigid_body_collider->linear_velocity.x = collider_rigid_body_linear_velocity[0];
						rigid_body_collider->linear_velocity.y = collider_rigid_body_linear_velocity[1];
						rigid_body_collider->linear_velocity.z = collider_rigid_body_linear_velocity[2];

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
			break;

			case COLLIDER_TYPE_TRIGGER_COLLIDER:

				//collider = (struct collider_t *)((char *)list->elements);

				//printf("%d\n", collider->contact_record_count);
				continue;
			break;

		}
	}



}


/*
=================================================================
=================================================================
=================================================================
*/

int physics_AreColliding(struct collider_handle_t collider_a, struct collider_handle_t collider_b)
{
	struct collider_t *collider;
	struct contact_record_t *contact_records;
	int i;
	int first_contact_record;

	collider = physics_GetColliderPointer(collider_a);
	contact_records = (struct contact_record_t *)phy_contact_records.elements;

	if(collider)
	{
		first_contact_record = collider->first_contact_record;

		for(i = 0; i < collider->contact_record_count; i++)
		{
			if(collider_b.type == contact_records[first_contact_record + i].collider.type)
			{
				if(collider_b.index == contact_records[first_contact_record + i].collider.index)
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

int physics_HasNewCollisions(struct collider_handle_t collider)
{
	struct collider_t *collider_ptr;
	struct contact_record_t *contact_records;
	int i;

	collider_ptr = physics_GetColliderPointer(collider);

	if(collider_ptr)
	{
		contact_records = (struct contact_record_t *)phy_contact_records.elements;

		for(i = 0; i < collider_ptr->contact_record_count; i++)
		{
			if(contact_records[collider_ptr->first_contact_record + i].life <= 1)
			{
				return 1;
			}
		}
	}

	return 0;
}

struct contact_record_t *physics_GetColliderContactRecords(struct collider_handle_t collider)
{
	struct collider_t *collider_ptr;

	collider_ptr = physics_GetColliderPointer(collider);

	if(collider_ptr)
	{
		if(collider_ptr->contact_record_count)
		{
			return (struct contact_record_t *)phy_contact_records.elements + collider_ptr->first_contact_record;
		}
	}

	return NULL;
}

/*
=================================================================
=================================================================
=================================================================
*/

struct collider_handle_t physics_Raycast(vec3_t from, vec3_t to, vec3_t *hit_position, vec3_t *hit_normal, int world_only)
{

	btVector3 v_from = btVector3(from.x, from.y, from.z);
	btVector3 v_to = btVector3(to.x, to.y, to.z);

	struct collider_t *collider;
	struct collider_handle_t collider_handle = INVALID_COLLIDER_HANDLE;
	const btCollisionObject *collision_object;
	int user_index;

	//btCollisionWorld::ClosestRayResultCallback hit(v_from, v_to);

	btCollisionObject *world = NULL;

	if(world_only)
	{
		world = world_collision_object;
	}

	ClosestWorldOnlyRaycaseResultCallback hit(v_from, v_to, world);

	float htime;

	if(world_collision_object)
	{
		physics_world->rayTest(v_from, v_to, hit);

		if(hit.m_closestHitFraction < 1.0)
		{
			if(hit_position)
			{
				hit_position->x = hit.m_hitPointWorld[0];
				hit_position->y = hit.m_hitPointWorld[1];
				hit_position->z = hit.m_hitPointWorld[2];
			}

			if(hit_normal)
			{
				hit_normal->x = hit.m_hitNormalWorld[0];
				hit_normal->y = hit.m_hitNormalWorld[1];
				hit_normal->z = hit.m_hitNormalWorld[2];
			}

			collision_object = hit.m_collisionObject;
			user_index = collision_object->getUserIndex();
			collider_handle = *(struct collider_handle_t *)&user_index;
		}

	}

	return collider_handle;
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
		delete world_triangles;

		world_collision_object = NULL;
		world_collision_mesh = NULL;
	}
}

void physics_BuildWorldCollisionMesh()
{
	int i;
	struct collider_handle_t handle = {COLLIDER_TYPE_NONE, INVALID_COLLIDER_INDEX};

	//btTriangleMesh *triangle_mesh;

	if(w_world_vertices_count < 3)
		return;


	physics_ClearWorldCollisionMesh();


//	triangle_mesh = new btTriangleMesh();
	world_triangles = new btTriangleMesh(true, false);

	for(i = 0; i < w_world_vertices_count; i += 3)
	{
		world_triangles->addTriangle(btVector3(w_world_vertices[i	 ].position.x, w_world_vertices[i	 ].position.y, w_world_vertices[i	 ].position.z),
		  						     btVector3(w_world_vertices[i + 1].position.x, w_world_vertices[i + 1].position.y, w_world_vertices[i + 1].position.z),
								     btVector3(w_world_vertices[i + 2].position.x, w_world_vertices[i + 2].position.y, w_world_vertices[i + 2].position.z));
	}

	world_collision_mesh = new btBvhTriangleMeshShape(world_triangles, false, true);

	world_collision_object = new btCollisionObject();
	world_collision_object->setCollisionShape(world_collision_mesh);
	world_collision_object->setUserIndex(*(int *)&handle);

	physics_world->addCollisionObject(world_collision_object);
}

#ifdef __cplusplus
}

} /* why is g++ asking for this? */
}
#endif




