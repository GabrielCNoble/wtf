#ifndef W_COMMON_H
#define W_COMMON_H

#include "scr_common.h"


#define MAX_WORLD_LIGHTS 512

#define MAX_VISIBLE_LIGHTS 32

#define W_MAX_PORTAL_RECURSION_LEVEL 9
#define W_MAX_BSP_NODES 1024

#define WORLD_SCRIPT_FILE_EXTENSION "was"

#define WORLD_EVENT_NAME_MAX_LEN 64

/* A triangle_group_t groups triangles which share the same material. It keeps
an entry in a GL_ELEMENT_ARRAY_BUFFER, which contain the indexes (laid contiguously on said buffer)
of those triangles. The triangle_group_t keeps how many vertices exist under that group,
and also the material that is to be used when rendering that triangles. */
typedef struct
{
	//unsigned int vertex_count;
	int material_index;
	unsigned int start;								/* offset from the beginning of the GL_ELEMENT_ARRAY_BUFFER */
	unsigned int next;								/* offset to put the next vertex... */
}triangle_group_t;


struct world_var_t
{
	char *name;
	int element_size;
	int element_count;
	int max_elements;
	void *value;
};




struct world_event_t
{
	char *event_name;
	void *event_function;
	int executing;
};




struct world_script_t
{
	struct script_t script;

	void *on_map_enter;
	void *on_map_exit;
	void *on_map_update;

	int event_count;
	int current_event;
	struct world_event_t *events;
};


enum WORLD_CLEAR_FLAGS
{
	WORLD_CLEAR_FLAG_LIGHTS = 1,
	WORLD_CLEAR_FLAG_LIGHT_LEAVES = 1 << 1,
	WORLD_CLEAR_FLAG_ENTITIES = 1 << 2,
	WORLD_CLEAR_FLAG_ENTITY_DEFS = 1 << 3,
	WORLD_CLEAR_FLAG_TRIGGERS = 1 << 4,
    WORLD_CLEAR_FLAG_PHYSICS_MESH = 1 << 5,
    WORLD_CLEAR_FLAG_BSP = 1 << 6,

    WORLD_CLEAR_FLAG_ALL = WORLD_CLEAR_FLAG_LIGHTS |
						   WORLD_CLEAR_FLAG_LIGHT_LEAVES |
						   WORLD_CLEAR_FLAG_ENTITIES |
						   WORLD_CLEAR_FLAG_ENTITY_DEFS |
						   WORLD_CLEAR_FLAG_TRIGGERS |
						   WORLD_CLEAR_FLAG_PHYSICS_MESH |
						   WORLD_CLEAR_FLAG_BSP,
};


#endif





