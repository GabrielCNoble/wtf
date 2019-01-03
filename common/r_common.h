#ifndef R_COMMON_H
#define R_COMMON_H

#include "gmath/vector.h"
#include "gmath/matrix.h"
#include "containers/list.h"
//#include "camera_types.h"


#define RENDERER_MIN_WIDTH 800
#define RENDERER_MAX_WIDTH 1920

#define RENDERER_MIN_HEIGHT 600
#define RENDERER_MAX_HEIGHT 1080

#define RENDERER_MIN_MSAA_SAMPLES 1
#define RENDERER_MAX_MSAA_SAMPLES 16

#define R_DRAW_COMMAND_LIST_RESIZE_INCREMENT 128

#define R_CLUSTER_WIDTH 64
#define R_CLUSTER_HEIGHT 64

#define R_LIGHT_UNIFORM_BUFFER_BINDING 0
#define R_BSP_UNIFORM_BUFFER_BINDING 2
#define R_WORLD_VERTICES_UNIFORM_BUFFER_BINDING 3

#define R_MAX_VISIBLE_LIGHTS 32
#define R_MAX_BSP_NODES 512
#define R_MAX_VISIBLE_LEAVES 512
#define R_MAX_VISIBLE_ENTITIES 4096

#define R_MAX_ACTIVE_VIEWS 32
#define R_VIEW_NAME_MAX_LEN 24
#define R_MAX_DRAW_COMMANDS 32768


enum INIT_MODE
{
	INIT_FULLSCREEN_DESKTOP = 1,
	INIT_FULLSCREEN,
	INIT_WINDOWED
};

enum WINDOW_FLAGS
{
	WINDOW_FULLSCREEN = 1,
};




enum R_CAPABILITIES
{
    R_FIRST_CAP = 0,
    R_CLEAR_COLOR_BUFFER,
    R_Z_PRE_PASS,
    R_SHADOW_PASS,
    R_BLOOM,
    R_TONEMAP,
    R_FLAT,
    R_WIREFRAME,
    R_FULLSCREEN,
    R_DEBUG,
    R_VERBOSE_DEBUG,
    R_LAST_CAP,
};




enum RENDERER_CALLBACK_TYPE
{
	PRE_SHADING_STAGE_CALLBACK = 1,
	POST_SHADING_STAGE_CALLBACK,
	RENDERER_RESOLUTION_CHANGE_CALLBACK,
	WINDOW_RESIZE_CALLBACK,
};

enum RENDERER_STAGE
{
	RENDERER_DRAW_SHADOW_MAPS_STAGE = 0,
	RENDERER_Z_PREPASS_STAGE,
	RENDERER_DRAW_WORLD_STAGE,
	RENDERER_SWAP_BUFFERS_STAGE,
	RENDERER_BIND_LIGHT_CACHE,
	RENDERER_BIND_GPU_CACHE,
	RENDERER_DRAW_FRAME,
	RENDERER_DRAW_GUI,
	RENDERER_STAGE_COUNT
};

enum RENDERER_TYPE
{
    RENDERER_TYPE_FOWARD,
    RENDERER_TYPE_DEFERRED,
    RENDERER_TYPE_CLUSTERED_FORWARD,
    RENDERER_TYPE_CLUSTERED_DEFERRED,
};

enum RENDERER_TEXTURES
{
	RENDERER_TEXTURE0,
	RENDERER_TEXTURE1,
	RENDERER_TEXTURE2,
	RENDERER_TEXTURE3,
	RENDERER_TEXTURE4,
	RENDERER_TEXTURE5,
};

enum RENDERER_TEXTURE_TARGETS
{
	RENDERER_TEXTURE_1D,
	RENDERER_TEXTURE_1D_ARRAY,
	RENDERER_TEXTURE_2D,
	RENDERER_TEXTURE_2D_ARRAY,
	RENDERER_TEXTURE_3D,
};

enum VERTEX_ATTRIB
{
	VERTEX_ATTRIB_POSITION,
	VERTEX_ATTRIB_NORMAL,
	VERTEX_ATTRIB_TEX_COORDS,
	VERTEX_ATTRIB_TANGENT,
	VERTEX_ATTRIB_COLOR,
};

enum VERTEX_FORMAT
{
	VERTEX_FORMAT_V3F_N3F_T3F_TC2F,
	VERTEX_FORMAT_V3F_N4IREV_T4IREV_TC2F,
	VERTEX_FORMAT_V3F_N4IREV_T4IREV_TC2S,
	VERTEX_FORMAT_V3F_N3F,
	VERTEX_FORMAT_V3F,
	VERTEX_FORMAT_CUSTOM,
};

enum DRAW_COMMAND_FLAGS
{
	DRAW_COMMAND_FLAG_INDEXED_DRAW = 1,
};


struct draw_command_t
{
	mat4_t *transform;
	unsigned int start;
	unsigned int count;
	unsigned char draw_mode;
	unsigned char flags;
	//unsigned short draw_mode;
	short material_index;			/* this could be a char... */
};

typedef struct
{
	int material_index;
	unsigned short max_draw_cmds;
	unsigned short draw_cmds_count;
	struct draw_command_t *draw_cmds;
}draw_command_group_t;


struct batch_t
{
	int start;
	int next;
	int material_index;
};



struct cluster_t
{
	unsigned int light_indexes_bm;
	/*unsigned int time_stamp;
	int align1;
	int align2;*/
};

#define R_MAX_FRAMEBUFFER_COLOR_ATTACHMENTS 3


struct framebuffer_attachment_t
{
	unsigned int handle;
	int format;
	int internal_format;
	int type;
};


struct framebuffer_t
{
	unsigned int framebuffer_id;

	struct
	{
		int handle;
		int format;
		int internal_format;
		int samples;
		int type;

	}color_attachments[R_MAX_FRAMEBUFFER_COLOR_ATTACHMENTS];

	unsigned int depth_attachment;
	unsigned int stencil_attachment;

	unsigned short width;
	unsigned short height;

	//unsigned int format;
	//unsigned int internal_format;
	//unsigned int type;
};

typedef struct
{

}tex_unit_t;










enum R_VIEW_FLAGS
{
    R_VIEW_FLAG_INVALID = 1,
};


#define INVALID_VIEW_INDEX 0xffff
#define DEFAULT_VIEW_INDEX 0xfffe

struct view_handle_t
{
    unsigned short view_index;
};

#define INVALID_VIEW_HANDLE (struct view_handle_t){INVALID_VIEW_INDEX}
#define DEFAULT_VIEW_HANDLE (struct view_handle_t){DEFAULT_VIEW_INDEX}


struct view_data_t
{
	mat4_t projection_matrix;
	mat4_t view_matrix;

    unsigned int draw_commands_frame;
	struct list_t draw_commands;



	//frustum_t frustum;

//	unsigned short view_lights_list_cursor;
//	unsigned short view_lights_list_size;
//	view_light_t *view_lights;

//	unsigned short view_entities_list_cursor;
//	unsigned short view_entities_list_size;
//	unsigned short *view_entities;

//	unsigned int view_portals_frame;
//	unsigned short view_portals_list_cursor;
//	unsigned short view_portals_list_size;
//	unsigned short *view_portals;

//	unsigned short view_triangles_cursor;
//	unsigned short view_triangles_size;
//	bsp_striangle_t *view_triangles;
	//unsigned int view_world_batch_cursor;
	//unsigned int view_world_batch_size;
	//batch_t *view_world_batches;
	//unsigned int *view_visible_world;

	//view_material_ref_record_t *view_material_refs;

//	unsigned int view_draw_command_frame;
//	unsigned int view_draw_command_list_size;
//	unsigned int view_draw_command_list_cursor;
//	draw_command_t *view_draw_commands;

//	int view_leaves_list_cursor;
//	int view_leaves_list_size;
//	bsp_dleaf_t **view_leaves;
};

struct view_def_t
{
	struct view_data_t view_data;

    frustum_t frustum;

	mat3_t world_orientation;
	vec3_t world_position;

	//float zoom;
	float fov_y;
	int width;
	int height;

	//float x_shift;
	//float y_shift;

	int flags;
	char *name;
};








//typedef camera_t view_def_t;




struct gpu_light_t
{
	vec4_t forward_axis;
	vec4_t position_radius;
	vec4_t color_energy;
	int bm_flags;
	float proj_param_a;
	float proj_param_b;
	int shadow_map;
};

struct gpu_bsp_node_t
{
	vec4_t normal_dist;
	unsigned int children[2];
	int node_count;
	int align1;
};




#endif











