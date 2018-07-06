#ifndef ENT_COMMON_H
#define ENT_COMMON_H

#include "phy_common.h"
#include "model.h"
#include "bsp_common.h"
#include "scr_common.h"
#include "nav_common.h"
#include "camera_types.h"


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
	ENTITY_ANIMATED = 1 << 3,
	ENTITY_GHOST = 1 << 4,					/* don't collide... */
};





enum COMPONENT_FLAGS
{
	COMPONENT_FLAG_INVALID = 1,
	COMPONENT_FLAG_DEACTIVATED = 1 << 1,
};

enum SCRIPT_CONTROLLER_COMPONENT_FLAGS
{
	SCRIPT_CONTROLLER_FLAG_FIRST_RUN = 1,
};


enum COMPONENT_TYPES
{
	COMPONENT_TYPE_TRANSFORM = 0,
	
	COMPONENT_TYPE_PHYSICS_CONTROLLER,
	COMPONENT_TYPE_SCRIPT_CONTROLLER,
	
	COMPONENT_TYPE_MODEL,	
	COMPONENT_TYPE_LIGHT,
	COMPONENT_TYPE_SCRIPT,
	COMPONENT_TYPE_CAMERA,
	COMPONENT_TYPE_LAST,
	COMPONENT_TYPE_NONE = COMPONENT_TYPE_LAST
};

enum COMPONENT_INDEXES
{
	COMPONENT_INDEX_TRANSFORM = 0, 
	
	COMPONENT_INDEX_CONTROLLER,
	COMPONENT_INDEX_PHYSICS_CONTROLLER = COMPONENT_INDEX_CONTROLLER,
	COMPONENT_INDEX_SCRIPT_CONTROLLER = COMPONENT_INDEX_CONTROLLER,
	
	COMPONENT_INDEX_MODEL,
	COMPONENT_INDEX_LIGHT,
	COMPONENT_INDEX_SCRIPT,
	COMPONENT_INDEX_CAMERA,
	COMPONENT_INDEX_LAST,	
	COMPONENT_INDEX_NONE = COMPONENT_INDEX_LAST,
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

/*
==============================================================
==============================================================
==============================================================
*/

struct component_t
{
	struct entity_handle_t entity;
	int type;
};


struct transform_component_t
{
	struct component_t base;
	
	mat3_t orientation;
	vec3_t scale;
	vec3_t position;
		
	int top_list_index;
	
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

struct controller_component_t
{
	struct component_t base;
	
	union
	{
		collider_def_t *collider_def;
		int collider_index;
	}collider;
	
	short flags;
};



struct physics_controller_component_t
{
	struct controller_component_t controller;
	short flags;
	short align;
};

struct script_controller_component_t
{
	struct controller_component_t controller;
	struct script_t *script;
		
	int flags;
	
	int max_route_length;
	int route_length;
	struct waypoint_t **route;
	int current_waypoint;
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
	int light_index;
	struct component_handle_t transform;
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

struct controller_script_t
{
	struct script_t script;
	struct entity_handle_t *entity_handle;
	void *on_first_run_entry_point;
};


struct entity_t
{
	struct component_handle_t components[COMPONENT_INDEX_LAST];
	bsp_dleaf_t *leaf;
	int flags;
	char *name;
};










#ifdef __cplusplus
}
#endif





#endif
