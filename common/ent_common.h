#ifndef ENT_COMMON_H
#define ENT_COMMON_H

#include "phy_common.h"
#include "model.h"
#include "bsp_common.h"
#include "script/scr_common.h"
#include "nav_common.h"
#include "camera_types.h"
#include "containers/list.h"


#define ENTITY_NAME_MAX_LEN 24				/* including trailing null... */
#define ENTITY_DEF_NAME_MAX_LEN 24
#define ENTITY_PROP_NAME_MAX_LEN 24
#define ENTITY_TRIGGER_NAME_MAX_LEN 32

#define MAX_ENTITIES 65536
#define MAX_VISIBLE_ENTITIES 1024
#define MAX_ENTITY_DEFS 1024

#define INVALID_ENTITY_INDEX 0x7fffffff
#define INVALID_COMPONENT_INDEX 0x7ffffff

#define INVALID_COMPONENT_HANDLE (struct component_handle_t){COMPONENT_TYPE_NONE,0,INVALID_COMPONENT_INDEX}
#define INVALID_ENTITY_HANDLE (struct entity_handle_t){1, INVALID_ENTITY_INDEX}


#define ENTITY_SCRIPT_FILE_EXTENSION "eas"
#define ENTITY_FILE_EXTENSION "ent"

#ifdef __cplusplus
extern "C"
{
#endif

enum ENTITY_TYPE
{
	ENTITY_TYPE_INVALID,
	ENTITY_TYPE_STATIC,
	ENTITY_TYPE_MOVABLE,
};

enum ENTITY_DEF_FLAGS
{
	ENTITY_DEF_ANIMATED = 1,
};

enum ENTITY_FLAGS
{
	ENTITY_FLAG_INVALID = 1,
	ENTITY_FLAG_HAS_MOVED = 1 << 1,
	ENTITY_FLAG_INVISIBLE = 1 << 2,
	ENTITY_FLAG_MARKED_INVALID = 1 << 3,
	ENTITY_FLAG_EXECUTED_SPAWN_FUNCTION = 1 << 4,
	ENTITY_FLAG_EXECUTED_DIE_FUNCTION = 1 << 5,

	ENTITY_FLAG_NOT_INITIALIZED = 1 << 6,


	ENTITY_FLAG_STATIC = 1 << 7,

	ENTITY_FLAG_SERIALIZED = 1 << 27,
	ENTITY_FLAG_MODIFIED = 1 << 28,					/* to allow serialization of entities that were modified after being spawned... */
	ENTITY_FLAG_ON_DISK = 1 << 29
};





enum COMPONENT_FLAGS
{
	COMPONENT_FLAG_INVALID = 1,
	COMPONENT_FLAG_DEACTIVATED = 1 << 1,
	COMPONENT_FLAG_SERIALIZED = 1 << 2,
	COMPONENT_FLAG_INITIALIZED = 1 << 3,
	COMPONENT_FLAG_ENTITY_DEF_REF = 1 << 4,
};

enum SCRIPT_CONTROLLER_COMPONENT_FLAGS
{
	SCRIPT_CONTROLLER_FLAG_FIRST_RUN = 1,
};


enum COMPONENT_TYPES
{
	COMPONENT_TYPE_TRANSFORM = 0,
	COMPONENT_TYPE_PHYSICS,
	COMPONENT_TYPE_MODEL,
	COMPONENT_TYPE_LIGHT,
	COMPONENT_TYPE_SCRIPT,
	COMPONENT_TYPE_CAMERA,
	COMPONENT_TYPE_PARTICLE_SYSTEM,
	COMPONENT_TYPE_LIFE,
	COMPONENT_TYPE_NAVIGATION,
	COMPONENT_TYPE_LAST,
	COMPONENT_TYPE_NONE = COMPONENT_TYPE_LAST
};

struct component_handle_t
{
	unsigned type : 4;
	unsigned def : 1;
	unsigned index : 27;
};


struct entity_handle_t
{
	unsigned def : 1;
	unsigned entity_index : 31;
};

struct component_field_t
{
	char *field_name;
	int script_type;
	int offset;
};

struct component_fields_t
{
	struct component_field_t fields[16];
};

/*
==============================================================
==============================================================
==============================================================
*/

struct component_t
{
	struct entity_handle_t entity;
	short type;
	short flags;
};


struct transform_component_t
{
	struct component_t base;

	mat3_t orientation;
	vec3_t scale;
	vec3_t position;

	int top_list_index;
	//int depth_index;

	int flags;

	struct component_handle_t parent;
	int children_count;
	int max_children;
	struct component_handle_t *child_transforms;

	char *instance_name;
};

struct entity_aabb_t
{
	vec3_t current_extents;
	vec3_t original_extents;
};

struct entity_transform_t
{
	mat4_t transform;
};

/*
==============================================================
==============================================================
==============================================================
*/

enum PHYSICS_COMPONENT_FLAGS
{
	PHYSICS_COMPONENT_FLAG_STATIC = 1,
};

struct physics_component_t
{
	struct component_t base;

	union
	{
		struct collider_def_t *collider_def;
		struct collider_handle_t collider_handle;

	}collider;

	int first_entity_contact;
    short entity_contact_count;
    short max_entity_contact_count;

	int flags;
};

/*
==============================================================
==============================================================
==============================================================
*/

struct model_component_t
{
	struct component_t base;
	int model_index;
};

/*
==============================================================
==============================================================
==============================================================
*/

struct light_component_t
{
	struct component_t base;

	struct list_t light_list;
	struct list_t transform_list;

	//int light_index;
	//struct component_handle_t transform;
};

/*
==============================================================
==============================================================
==============================================================
*/

struct script_component_t
{
	struct component_t base;
	struct script_t *script;
	int flags;
};

/*
==============================================================
==============================================================
==============================================================
*/

struct camera_component_t
{
	struct component_t base;
	camera_t *camera;
	//struct component_handle_t transform;
};

/*
==============================================================
==============================================================
==============================================================
*/

struct particle_system_component_t
{
	struct component_t base;
	int particle_system_t;
	struct component_handle_t transform;
};

/*
==============================================================
==============================================================
==============================================================
*/


struct life_component_t
{
	struct component_t base;
	float life;
};


/*
==============================================================
==============================================================
==============================================================
*/

struct navigation_component_t
{
    struct component_t base;
    struct list_t route;
    int current_waypoint;
};

/*
==============================================================
==============================================================
==============================================================
*/

struct entity_script_t
{
	struct script_t script;

	struct entity_handle_t *entity_handle;
	void *on_update_entry_point;
	void *on_first_run_entry_point;
	void *on_spawn_entry_point;
	void *on_die_entry_point;
	void *on_collision_entry_point;

	void *collided_array;
};


enum ENTITY_PROP_TYPE
{
	ENTITY_PROP_TYPE_INT,
	ENTITY_PROP_TYPE_FLOAT,
	ENTITY_PROP_TYPE_VEC2,
	ENTITY_PROP_TYPE_VEC3,
	ENTITY_PROP_TYPE_VEC4,
	ENTITY_PROP_TYPE_MAT2,
	ENTITY_PROP_TYPE_MAT3,
	ENTITY_PROP_TYPE_MAT4,
	ENTITY_PROP_TYPE_LAST,
	ENTITY_PROP_TYPE_NONE,
};


struct entity_prop_t
{
	char *name;
	int size;
	int type;
	void *memory;
};


struct entity_contact_t
{
    struct entity_handle_t entity;
    vec3_t position;
};

struct entity_source_file_t
{
	char file_name[PATH_MAX];
};

/*

Each entity's transform component keep references to children
transform components. Those transform components reference back
the entities they belong to, which also keep a reference to
those transforms. This is how nestled entities work.

For each case of a single entity def getting referenced several times
a new transform component will be allocated, and will be made to point
to the referenced entity def. This component is not directly reachable
from the original def, but the original def is reachable from it.

The handle for this allocated component will be stored in the children
list of the transform component of the parent entity to the reference.

*/
struct entity_t
{
	struct component_handle_t components[COMPONENT_TYPE_LAST];
	struct entity_handle_t def;

    struct list_t props;

	//int max_props;
	//int prop_count;
	//struct entity_prop_t *props;


	int ref_count;									/* how many times this entity (if a def) is being referenced from other entities... */


	struct bsp_dleaf_t *leaf;
	int flags;
	int spawn_time;
	char *name;
};



enum TRIGGER_FLAGS
{
	TRIGGER_FLAG_INVALID = 1,
	TRIGGER_FLAG_TRIGGERED = 1 << 1,
};

enum TRIGGER_FILTER_FLAG
{
	TRIGGER_FILTER_FLAG_TRIGGER_ON_EQUAL = 1,
};

struct trigger_filter_t
{
    //entity_prop_t prop;
    char *prop_name;
    int flag;
};


struct trigger_t
{
	mat3_t orientation;
	vec3_t position;
	vec3_t scale;

	struct list_t trigger_filters;

    struct collider_handle_t collider;

    int flags;

    char *event_name;
    char *name;
};









#ifdef __cplusplus
}
#endif





#endif
