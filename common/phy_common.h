#ifndef PHY_COMMON_H
#define PHY_COMMON_H

#include "gmath/vector.h"
#include "gmath/matrix.h"
#include "containers/list.h"
#include "model.h"


//#include "ent_common.h"

#include <limits.h>
#include <stdint.h>
//#include "player.h"
//#include "projectile.h"

#define GRAVITY 0.0098

//#define GRAVITY 0.015

#define GROUND_FRICTION 0.65
#define AIR_FRICTION 0.01

#define GROUND_DELTA_INCREMENT 0.45
#define AIR_DELTA_INCREMENT 0.35

#define MAX_HORIZONTAL_DELTA 8.25

#define JUMP_DELTA 0.85

#define PLAYER_CAPSULE_HEIGHT 4.0
#define PLAYER_CAPSULE_RADIUS 1.5

#define COLLIDER_DEF_NAME_MAX_LEN 24

/*typedef struct
{
	vec3_t position;
	vec3_t half_extents;
	vertex_t *verts;
	int start;
}block_t;

/*typedef struct
{
	vec3_t a;
	vec3_t b;
	float radius;
}capsule_t;

/*typedef struct
{
	vec3_t a;
	vec3_t b;
	vec3_t c;
	unsigned int packed_normal;

}collision_triangle_t;

typedef struct bvh_node_t
{
	vec3_t position;						// if this node is not a leaf, it represents a volume within the hierarchy...
	vec3_t half_extents;

	int start;								// this is an offset within the array of vertices that compose the static world geometry...

	struct bvh_node_t *left;
	struct bvh_node_t *right;

	union
	{
		struct bvh_node_t *parent;
		struct bvh_node_t *next;
	};

}bvh_node_t;

typedef struct
{
	vec3_t hit_point;
	vec3_t hit_normal;
}ray_cast_result_t;

*/

enum COLLIDER_DEF_FLAGS
{
	COLLIDER_DEF_RECALCULATE_INERTIA_TENSOR = 1,
	COLLIDER_DEF_UPDATE_CACHED_COLLISION_SHAPE = 1 << 1,
	COLLIDER_DEF_DRAW_DEBUG = 1 << 2,
};

enum COLLIDER_TYPE
{
	COLLIDER_TYPE_CHARACTER_COLLIDER = 0,
	COLLIDER_TYPE_RIGID_BODY_COLLIDER,
	COLLIDER_TYPE_PROJECTILE_COLLIDER,
	COLLIDER_TYPE_TRIGGER_COLLIDER,
	COLLIDER_TYPE_COLLIDER_DEF,
	COLLIDER_TYPE_LAST,
	COLLIDER_TYPE_NONE = COLLIDER_TYPE_LAST,
};

enum COLLIDER_FLAGS
{
	COLLIDER_FLAG_INVALID = 1,
	COLLIDER_FLAG_UPDATE_COLLISION_OBJECT = 1 << 1,
	COLLIDER_FLAG_NO_SCALE_HINT = 1 << 2,
	COLLIDER_FLAG_NEW_COLLISIONS = 1 << 3,
	COLLIDER_FLAG_DONT_TRACK_COLLISIONS = 1 << 4,
	COLLIDER_FLAG_STATIC = 1 << 5,
};

enum CHARACTER_COLLIDER_FLAGS
{
	CHARACTER_COLLIDER_FLAG_CROUCHED = 1,
	CHARACTER_COLLIDER_FLAG_ON_GROUND = 1 << 1,
	CHARACTER_COLLIDER_FLAG_WALKING = 1 << 2,
	CHARACTER_COLLIDER_FLAG_STEPPING_UP = 1 << 3,
	CHARACTER_COLLIDER_FLAG_JUST_JUMPED = 1 << 4,
};

enum COLLISION_SHAPES
{
	//COLLISION_SHAPE_EMPTY,
	COLLISION_SHAPE_BOX = 0,
	COLLISION_SHAPE_CYLINDER,
	COLLISION_SHAPE_SPHERE,
	COLLISION_SHAPE_CAPSULE,
	//COLLISION_SHAPE_COMPOUND,
	COLLISION_SHAPE_LAST,
	COLLISION_SHAPE_NONE = COLLISION_SHAPE_LAST
};




#if 0

struct collider_def_t
{
	struct collider_def_t *next;
	struct collider_def_t *prev;

	//union
	//{
	//	struct
	//	{
	int max_collision_shapes;
	int collision_shape_count;
	struct collision_shape_t *collision_shape;
	//	}generic_collider_data;

	//	struct
	//	{
	//void *collision_shape;
	float height;
	float crouch_height;
	float radius;
	float max_step_height;
	float max_slope_angle;
	float max_walk_speed;
	//	}character_collider_data;
	//}collider_data;


	void *cached_collision_shape;
	vec3_t inertia_tensor;

	int ref_count;
	int flags;
	int type;
	float mass;
	char *name;
};

#endif


#define INVALID_COLLIDER_INDEX 0x1fffffff

struct collider_handle_t
{
	unsigned type : 3;
	unsigned index : 29;
};


#define INVALID_COLLIDER_HANDLE (struct collider_handle_t){COLLIDER_TYPE_NONE, INVALID_COLLIDER_INDEX}


struct collider_t
{
	mat3_t orientation;
	vec3_t position;
	vec3_t scale;

	void *collision_object;						/* opaque reference to a btRigidBody... */
	struct collider_handle_t def;
	unsigned short flags;
	unsigned short type;

	int entity_index;

	short contact_record_count;
	unsigned short max_contact_records;
	unsigned int first_contact_record;

};

struct collision_shape_t
{
	mat3_t orientation;
	vec3_t position;
	vec3_t scale;
	int type;
};

enum PHY_CONSTRAINT_TYPE
{
    PHY_CONSTRAINT_TYPE_FIRST = -1,
    PHY_CONSTRAINT_TYPE_SLIDE,
    PHY_CONSTRAINT_TYPE_HINGE,
    PHY_CONSTRAINT_TYPE_LAST,
};

struct constraint_t
{
    int type;
    void *constraint;
    struct collider_handle_t collider_a;
    struct collider_handle_t collider_b;
};

struct collider_def_t
{
    struct collider_t base;

    //int max_collision_shapes;
	//int collision_shape_count;
	//struct collision_shape_t *collision_shape;

	struct list_t collision_shape;

	void *cached_collision_shape;
	vec3_t inertia_tensor;

	//struct constraint_t *

	float height;
	float crouch_height;
	float radius;
	float max_step_height;
	float max_slope_angle;
	float max_walk_speed;

	int ref_count;
//	int flags;
	//int type;
	int def_flags;
	int collider_type;
	float mass;
	char *name;
};


struct rigid_body_collider_t
{
    struct collider_t base;
    vec3_t linear_velocity;
};


struct character_collider_t
{
	struct collider_t base;
	float radius;
	float height;
	float max_step_height;
	float max_slope_angle;
	float max_walk_speed;
	int flags;
};


struct trigger_collider_t
{
	struct collider_t base;
};

struct projectile_collider_t
{
	struct collider_t base;
};

#if 0

struct collider_t
{
	//union
	//{
	//	struct
	//	{

	//	}generic_collider_data;

	//	struct
	//	{

	//	}character_collider_data;
	//}collider_data;
	mat3_t orientation;
	vec3_t position;
	vec3_t scale;
	vec3_t linear_velocity;

	float radius;
	float height;
	float step_height;
	float max_slope;
	float max_walk_speed;
	int character_collider_flags;

	void *rigid_body;						/* opaque reference to a btRigidBody... */
	struct collider_def_t *def;
	unsigned short flags;
	unsigned short type;

	int entity_index;

	short contact_record_count;
	unsigned short max_contact_records;
	unsigned int first_contact_record;

};

#endif

struct contact_record_t
{
	struct collider_handle_t collider;
	int life;
	short material_index;
	unsigned short align;

	vec3_t position;
	vec3_t normal;
	float applied_impulse;

	//short collided_with_world;
	//int material_index;
	//vec3_t world_position;
};



/*
==============================================================================
==============================================================================
==============================================================================
*/


typedef struct
{
	uint32_t collider_count;

	uint32_t reserved0;
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t reserved3;
	uint32_t reserved4;
	uint32_t reserved5;
	uint32_t reserved6;
	uint32_t reserved7;

}collider_section_header_t;


typedef struct
{
	char collider_name[PATH_MAX];
	uint16_t type;

	float mass;
	float height;

}collder_record_t;

/*
==============================================================================
==============================================================================
==============================================================================
*/



#endif






