#ifndef W_COMMON_H
#define W_COMMON_H

#include "scr_common.h"


#define MAX_WORLD_LIGHTS 512

#define MAX_VISIBLE_LIGHTS 32

#define W_MAX_PORTAL_RECURSION_LEVEL 9

#define WORLD_SCRIPT_FILE_EXTENSION "was"

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


struct world_script_t
{
	struct script_t script;

	void *on_map_enter;
	void *on_map_exit;
	void *on_map_update;
};


#endif





