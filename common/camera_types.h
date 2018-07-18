#ifndef CAMERA_TYPES_H
#define CAMERA_TYPES_H

//#include "scenegraph.h"
#include "matrix_types.h"
#include "vector_types.h"
#include "frustum_types.h"
#include "bsp.h"
#include "r_common.h"
//#include "framebuffer.h"

#define CLUSTERS_PER_ROW 32
#define CLUSTER_ROWS 16
#define CLUSTER_LAYERS 24

#define PACK_LIGHT_VIEW_CLUSTER_INDEXES(light_index, view_cluster_index) ((light_index<<7)|(view_cluster_index))
#define UNPACK_LIGHT_VIEW_CLUSTER_INDEXES(light_index, view_cluster_index, packed_indexes) light_index=((packed_indexes>>7)&0x000003ff);	\
																							view_cluster_index=packed_indexes&0x0000007f;
																							
#define CAMERA_NAME_MAX_LEN 24																							

enum CAMERA_FLAGS
{
	CAMERA_UPDATE_ON_RESIZE = 1,
	CAMERA_INACTIVE = 1 << 1,
};


enum VIEW_TYPES
{
	VIEW_MAIN = 1,
	VIEW_PORTAL,
	VIEW_SHADOW,
};

/*typedef struct
{
	unsigned int light_indexes_bm;
}cluster_t;*/


/* within each light that's
within this view, which view
cluster is assigned to this view... */
typedef struct
{
	unsigned short light_index;
	unsigned short view_cluster_index;
}light_view_cluster_t;

typedef struct
{
	unsigned int view_clusters;
	unsigned short light_index;
	unsigned short align;
	//unsigned short view_cluster_index;
}view_light_t;

typedef struct
{
	unsigned int last_touched;
	unsigned short frame_ref_count;
	unsigned short align;
	
	
}view_material_ref_record_t;


typedef struct
{
	
	mat4_t projection_matrix;
	mat4_t view_matrix;
	
	//frustum_t frustum;
	
	unsigned short view_lights_list_cursor;
	unsigned short view_lights_list_size;
	view_light_t *view_lights;

	unsigned short view_entities_list_cursor;
	unsigned short view_entities_list_size;
	unsigned short *view_entities;
	
	unsigned int view_portals_frame;
	unsigned short view_portals_list_cursor;
	unsigned short view_portals_list_size;
	unsigned short *view_portals;
	
	unsigned short view_triangles_cursor;
	unsigned short view_triangles_size;
	bsp_striangle_t *view_triangles;
	//unsigned int view_world_batch_cursor;
	//unsigned int view_world_batch_size;
	//batch_t *view_world_batches;
	//unsigned int *view_visible_world;
	
	//view_material_ref_record_t *view_material_refs;
	
	unsigned int view_draw_command_frame;
	unsigned int view_draw_command_list_size;
	unsigned int view_draw_command_list_cursor;
	draw_command_t *view_draw_commands;
	
	int view_leaves_list_cursor;
	int view_leaves_list_size;
	bsp_dleaf_t **view_leaves;
}view_data_t;
/*struct view_def
{
	struct view_def *next;
	struct view_def *prev;
	
	mat4_t projection_matrix;
	mat4_t view_matrix;
	mat3_t orientation;
	vec3_t position;
	
	int type;
};*/

typedef struct camera_t
{
	struct camera_t *next;		/* cameras won't be created all the time, so a linked list will be easier to manage... */
	struct camera_t *prev;
	
	view_data_t view_data;
	
	//mat4_t projection_matrix;
	//mat4_t world_to_camera_matrix;
	//mat3_t local_orientation;			/* this will be useful only when there's a scenegraph... */
	mat3_t world_orientation;
	//framebuffer_t geometry_buffer;
	//framebuffer_t output_buffer;
	frustum_t frustum;
	//vec3_t local_position;			/* this will be useful only when there's a scenegraph... */
	vec3_t world_position;
	//node_t *assigned_node;
	float zoom; 
	//float exposure;
	float fov_y;
	int width;
	int height;
	
	float x_shift;
	float y_shift;

	int bm_flags;
	char *name;
}camera_t;

/*typedef struct
{
	framebuffer_t geometry_buffer;
	framebuffer_t composite_buffer;
	framebuffer_t auxiliary_buffer;
	framebuffer_t output_buffer;
}framebuffer_set_t;*/







#endif /* CAMERA_TYPES_H */
