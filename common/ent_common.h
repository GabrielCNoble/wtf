#ifndef ENT_COMMON_H
#define ENT_COMMON_H

#include "phy_common.h"
#include "model.h"
#include "bsp_common.h"
#include "scr_common.h"
#include "nav_common.h"
#include "camera_types.h"
#include "list.h"


#define ENTITY_NAME_MAX_LEN 24				/* including trailing null... */
#define ENTITY_DEF_NAME_MAX_LEN 24

#define MAX_ENTITIES 65536
#define MAX_VISIBLE_ENTITIES 1024
#define MAX_ENTITY_DEFS 1024

#define INVALID_ENTITY_INDEX 0x7fffffff

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
	ENTITY_INVALID = 1,
	ENTITY_HAS_MOVED = 1 << 1,
	ENTITY_INVISIBLE = 1 << 2,
	ENTITY_MARKED_INVALID = 1 << 3,
	ENTITY_ALREADY_SERIALIZED = 1 << 27,
	//ENTITY_ANIMATED = 1 << 3,
	//ENTITY_GHOST = 1 << 4,					/* don't collide... */
};





enum COMPONENT_FLAGS
{
	COMPONENT_FLAG_INVALID = 1,
	COMPONENT_FLAG_DEACTIVATED = 1 << 1,
	COMPONENT_FLAG_NESTLED = 1 << 27,
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
	int depth_index;
	
	int flags;
	
	struct component_handle_t parent;
	int children_count;
	int max_children;
	struct component_handle_t *child_transforms;
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

struct physics_component_t
{
	struct component_t base;
	
	union
	{
		collider_def_t *collider_def;
		struct collider_handle_t collider_handle;
		//int collider_index;
	}collider;
	
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
	struct component_handle_t transform;
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

struct entity_script_t
{
	struct script_t script;
	
	struct entity_handle_t *entity_handle;
	void *on_first_run_entry_point;
	void *on_spawn_entry_point;
	void *on_die_entry_point;
	
};

struct entity_prop_t
{
	char *name;
	int size;
	void *memory;
};

struct entity_t
{
	struct component_handle_t components[COMPONENT_TYPE_LAST];
	
	struct entity_handle_t def;
	
	int max_props;
	int prop_count;
	struct entity_prop_t *props;
	
	bsp_dleaf_t *leaf;
	int flags;
	int spawn_time;
	char *name;
};










#ifdef __cplusplus
}
#endif





#endif
