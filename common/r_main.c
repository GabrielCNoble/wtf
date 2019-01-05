#include <stdio.h>
#include <string.h>

#include "r_common.h"
#include "r_shader.h"
#include "r_main.h"
#include "r_debug.h"
#include "r_imediate.h"
#include "r_gl.h"
#include "r_vis.h"
//#include "r_text.h"
#include "r_verts.h"
#include "r_view.h"
//#include "r_draw.h"

#include "containers/stack_list.h"

//#include "..\editor\level editor\ed_level_draw.h"

//#include "camera.h"
//#include "player.h"
#include "log.h"
#include "shader.h"
//#include "physics.h"
#include "model.h"
#include "world.h"
#include "material.h"
#include "entity.h"
#include "texture.h"
#include "l_main.h"
//#include "l_cache.h"
//#include "brush.h"
//#include "editor.h"
#include "gui.h"
#include "bsp.h"
//#include "log.h"
#include "c_memory.h"
#include "portal.h"
#include "particle.h"

#include "engine.h"

#include "SDL2\SDL.h"
#include "GL\glew.h"



SDL_Window *window;
SDL_GLContext context;




//int r_active_shader = -1;

//unsigned int r_gbuffer_id = 0;
//unsigned int r_gbuffer_color = 0;
//unsigned int r_gbuffer_normal = 0;
//unsigned int r_gbuffer_depth = 0;


struct framebuffer_t r_cbuffer;
struct framebuffer_t r_bbuffer;
struct framebuffer_t r_sbuffer;

//unsigned int r_pbuffer_id = 0;
//unsigned int r_pbuffer_color = 0;
//unsigned int r_pbuffer_depth = 0;

unsigned int r_default_vao;


int r_z_pre_pass_shader;
int r_forward_pass_shader;
int r_forward_pass_no_shadow_shader;
int r_particle_forward_pass_shader;
int r_flat_pass_shader;
int r_wireframe_pass_shader;
int r_geometry_pass_shader;
int r_shade_pass_shader;
int r_stencil_lights_pass_shader;
int r_shadow_pass_shader;
int r_skybox_shader;
int r_bloom0_shader;
int r_bloom1_shader;
int r_tonemap_shader;
int r_blit_texture_shader;
int r_portal_shader;
int r_forward_pass_portal_shader;

int r_gui_shader;

extern int r_imediate_color_shader;

vec3_t r_clear_color = {0.0, 0.0, 0.0};


/* from world.c */
//extern unsigned int world_element_buffer;
//extern int w_world_index_handle;
//extern int *w_index_buffer;
extern int w_world_batch_count;
extern struct batch_t *w_world_batches;

extern int w_world_z_batch_count;
extern struct batch_t *w_world_z_batches;
//extern int w_world_start;
extern int w_world_index_start;
extern int w_world_sorted_index_start;
extern int w_world_sorted_index_count;

extern vertex_t *w_world_vertices;
extern int w_world_vertices_count;

unsigned int *r_occlusion_queries;
//extern int w_world_vertices_count;
//extern int w_world_leaves_count;
//extern struct bsp_dleaf_t *w_world_leaves;
//extern int w_visible_lights_count;
//extern unsigned short w_visible_lights[];



/* from particle.c */
//extern int ps_particle_system_count;
//extern particle_system_t *ps_particle_systems;
extern struct stack_list_t ps_particle_systems;
extern int ps_particle_quad_start;



/* from material.c */
extern int mat_material_count;
extern material_t *mat_materials;

/* from texture.c */
//extern int tex_texture_count;
//extern texture_t *tex_textures;

extern struct stack_list_t tex_textures;

/* from shader.c */
//extern struct shader_t *shaders;




/* from gui.c */
//extern widget_t *widgets;
//extern widget_t *last_widget;
//extern widget_t *top_widget;
//extern mat4_t gui_projection_matrix;


//#include "l_globals.h"

//extern int l_light_list_cursor;
//extern light_params_t *l_light_params;
//extern light_position_t *l_light_positions;



/* from player.c */
//extern player_count;
//extern player_t *players;
//extern player_t *active_player;


//extern int ptl_portal_list_cursor;
//extern portal_t *ptl_portals;



//#define QUERY_STAGES


#define MAX_RENDER_FUNCTIONS 16
static int pre_shading_render_function_count = 0;
static int post_shading_render_function_count = 0;
static int window_resize_callback_count = 0;
static int renderer_resolution_change_callback_count = 0;
static void (*renderer_PreShadingRegisteredFunction[MAX_RENDER_FUNCTIONS])(void);
static void (*renderer_PostShadingRegisteredFunction[MAX_RENDER_FUNCTIONS])(void);
static void (*renderer_WindowResizeCallback[MAX_RENDER_FUNCTIONS])(void);
static void (*renderer_RendererResolutionChangeCallback[MAX_RENDER_FUNCTIONS])(void);
//void (*renderer_DrawWorld)() = NULL;
//void (*renderer_DrawOpaque)() = NULL;

unsigned int r_frame = 0;

unsigned int query_object;




int r_max_batch_size = 10;
int r_draw_command_group_count = 0;
//int r_draw_cmds_count = 0;
//int r_max_draw_cmds_count = 0;
draw_command_group_t *r_draw_command_groups = NULL;
//static struct draw_command_t *r_sorted_draw_cmds = NULL;
static struct list_t r_sorted_draw_cmds;
//static struct draw_command_t *r_unsorted_draw_cmds = NULL;

float r_fade_value = 0.1;


/*int r_translucent_draw_group_count = 0;
draw_group_t *r_translucent_draw_groups = NULL;
static draw_command_t *r_translucent_draw_cmds = NULL;*/





//unsigned int r_bloom_fbs[3];
//unsigned int r_bloom_texs[2][3];
//int r_bloom_radius[3] = {2.0, 4.0, 8.0};




//unsigned int r_intensity_id = 0;
//unsigned int r_intensity_color = 0;

//unsigned int r_intensity_half_id = 0;
//unsigned int r_intensity_half_horizontal_color = 0;
//unsigned int r_intensity_half_vertical_color = 0;

//unsigned int r_intensity_quarter_id = 0;
//unsigned int r_intensity_quarter_horizontal_color = 0;
//unsigned int r_intensity_quarter_vertical_color = 0;

//unsigned int r_intensity_eight_id = 0;
//unsigned int r_intensity_eight_horizontal_color = 0;
//unsigned int r_intensity_eight_vertical_color = 0;


int r_width = 0;
int r_height = 0;
int r_window_width = 0;
int r_window_height = 0;
int r_window_flags = 0;

int r_clusters_per_row = 0;
int r_cluster_rows = 0;
int r_cluster_layers = 0;
struct cluster_t *r_clusters = NULL;
unsigned int r_cluster_texture;

int r_msaa_samples = 1;
int r_msaa_supported = 1;

int r_draw_shadow_maps = 1;
int r_z_prepass = 1;
int r_query_stages = 0;
int r_bloom = 1;
int r_tonemap = 1;
int r_draw_gui = 1;
int r_clear_colorbuffer = 1;

//int r_deferred = 0;

int r_flat = 0;
int r_wireframe = 0;
int r_debug = 1;


int r_caps[R_LAST_CAP] = {0};




int r_max_portal_recursion_level = 3;

mat4_t r_projection_matrix;
int r_projection_matrix_changed = 1;

mat4_t r_view_matrix;
int r_view_matrix_changed = 1;

mat4_t r_model_matrix;
mat4_t r_view_projection_matrix;
mat4_t r_model_view_matrix;
mat4_t r_model_view_projection_matrix;

//view_def_t *r_active_view = NULL;
//camera_t *r_main_view = NULL;




struct stack_list_t r_views;
struct view_data_t *r_active_views[R_MAX_ACTIVE_VIEWS];
struct view_handle_t r_main_view = INVALID_VIEW_HANDLE;


unsigned int r_cluster_texture = 0;
unsigned int r_light_uniform_buffer = 0;
unsigned int r_bsp_uniform_buffer = 0;
unsigned int r_world_vertices_uniform_buffer = 0;


struct gpu_light_t *r_light_buffer = NULL;
struct gpu_bsp_node_t *r_bsp_buffer = NULL;
vec4_t *r_world_vertices_buffer = NULL;
int r_bsp_node_count = 0;


//extern camera_t *active_camera;
//extern camera_t *main_view;

unsigned int r_draw_calls = 0;
unsigned int r_material_swaps = 0;
unsigned int r_shader_swaps = 0;
unsigned int r_shader_uniform_updates = 0;
unsigned int r_frame_vert_count = 0;


unsigned int r_shadow_map_framebuffer = 0;
unsigned int r_shadow_map_array = 0;
unsigned int r_shadow_map_resolution = 0;
unsigned int r_force_shadow_map_update = 0;


int r_frame_clamping = 0;


unsigned int r_visible_lights_count = 0;
unsigned int r_visible_lights[W_MAX_WORLD_LIGHTS];

struct list_t r_visible_entities;

//unsigned int r_visible_entities_count = 0;
//struct entity_handle_t r_visible_entities[R_MAX_VISIBLE_ENTITIES];

unsigned int r_visible_leaves_count = 0;
struct bsp_dleaf_t **r_visible_leaves;



struct view_def_t r_default_view;


#ifdef __cplusplus
extern "C"
{
#endif


int renderer_Init(int width, int height, int init_mode)
{
	char *ext_str;
	char *sub_str;

	int i;

	int w;
	int h;

	r_window_flags = SDL_WINDOW_OPENGL;
	SDL_DisplayMode display_mode;

	int r;

	if(init_mode == INIT_FULLSCREEN_DESKTOP || init_mode == INIT_FULLSCREEN)
	{
		SDL_GetDisplayMode(0, 0, &display_mode);
		r_width = display_mode.w;
		r_height = display_mode.h;
		r_window_width = r_width;
		r_window_height = r_height;
		if(init_mode == INIT_FULLSCREEN_DESKTOP)
		{
			r_window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

	}
	else
	{
		r_width = width;
		r_height = height;
		r_window_width = width;
		r_window_height = height;
	}

	//r_active_shader = -1;

	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	//SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG | SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 1);

	window = SDL_CreateWindow("wtf editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, r_width, r_height, r_window_flags);
	context = SDL_GL_CreateContext(window);



	SDL_GL_MakeCurrent(window, context);

	//SDL_GL_SetSwapInterval(0);

	renderer_SetFrameRateClamping(1);

	int error = glewInit();

	if(error != GLEW_NO_ERROR)
	{
		log_LogMessage(LOG_MESSAGE_ERROR, 0, "renderer_Init: glew didn't init!\nError cause:\n %s", glewGetErrorString(error));
		return 0;
	}

	log_LogMessage(LOG_MESSAGE_NONE, 0, "Window resolution: %d x %d", r_width, r_height);


    /*glGenVertexArrays(1, &r_default_vao);
    glBindVertexArray(r_default_vao);*/


    r_views = stack_list_create(sizeof(struct view_def_t), 32, NULL);

    r_occlusion_queries = memory_Calloc(65536, sizeof(unsigned int ));

    glGenQueries(65356, r_occlusion_queries);

    renderer_CheckFunctionPointers();

    renderer_SetMainView(INVALID_VIEW_HANDLE);

    r_default_view.flags = 0;
    r_default_view.fov_y = 0.68;
    r_default_view.world_orientation = mat3_t_id();
    r_default_view.world_position = vec3_t_c(0.0, 0.0, 0.0);
    //r_default_view.zoom = 1.0;
    //r_default_view.x_shift = 0.0;
    //r_default_view.y_shift = 0.0;

    r_default_view.width = r_window_width;
    r_default_view.height = r_window_height;
    r_default_view.name = "default view";
    renderer_CreateViewData(&r_default_view.view_data);

    CreatePerspectiveMatrix(&r_default_view.view_data.projection_matrix, r_default_view.fov_y, (float)r_default_view.width / (float)r_default_view.height, 0.1, 500.0, 0.0, 0.0, &r_default_view.frustum);
    renderer_ComputeMainViewMatrix();



	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearStencil(0);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_MULTISAMPLE);
	//glDepthFunc(GL_LESS);
	glStencilMask(0xff);

	log_LogMessage(LOG_MESSAGE_NONE, 0, "GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	log_LogMessage(LOG_MESSAGE_NONE, 0, "OpenGL version: %s", glGetString(GL_VERSION));
	log_LogMessage(LOG_MESSAGE_NONE, 0, "Vendor: %s", glGetString(GL_VENDOR));
	log_LogMessage(LOG_MESSAGE_NONE, 0, "Renderer: %s", glGetString(GL_RENDERER));



	r_cbuffer.framebuffer_id = 0;
	r_bbuffer.framebuffer_id = 0;

	//renderer_SetRenderer(RENDERER_TYPE_CLUSTERED_FORWARD);

	renderer_SetRendererResolution(r_width, r_height, 1);


    /* uniform buffer accessed by the clustered forward pass... */
	glGenBuffers(1, &r_light_uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, r_light_uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(struct gpu_light_t) * R_MAX_VISIBLE_LIGHTS, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);


    /* uniform buffer accessed during hybrid ray-traced shadows and reflections... */
	glGenBuffers(1, &r_bsp_uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, r_bsp_uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(struct gpu_bsp_node_t) * R_MAX_BSP_NODES, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);




	glGenBuffers(1, &r_world_vertices_uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, r_world_vertices_uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(vec4_t) * 32, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);


	/*glGenBuffers(1, &r_world_index_uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, r_world_index_uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(struct int) * R_MAX_BSP_NODES, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);*/




	r_light_buffer = memory_Malloc(sizeof(struct gpu_light_t) * R_MAX_VISIBLE_LIGHTS);
	r_bsp_buffer = memory_Malloc(sizeof(struct gpu_bsp_node_t) * R_MAX_BSP_NODES);




    r_visible_entities = list_create(sizeof(struct entity_handle_t), 512, NULL);


	//r_draw_groups = malloc(sizeof(draw_group_t) * MAX_MATERIALS);
	//r_draw_cmds = malloc(sizeof(draw_command_t) * MAX_MATERIALS * MAX_ENTITIES);

	r_draw_command_groups = memory_Malloc(sizeof(draw_command_group_t) * (MAX_MATERIALS + 1));

/*	for(i = 0; i <= MAX_MATERIALS; i++)
	{
		r_draw_command_groups[i].max_draw_cmds = 8192;
		r_draw_command_groups[i].draw_cmds_count = 0;
		r_draw_command_groups[i].
	}*/

	//r_max_draw_cmds_count = R_MAX_DRAW_COMMANDS;
	//r_unsorted_draw_cmds = memory_Malloc(sizeof(struct draw_command_t) * r_max_draw_cmds_count);
	r_sorted_draw_cmds = list_create(sizeof(struct draw_command_t), R_MAX_DRAW_COMMANDS, NULL);
	//r_sorted_draw_cmds = memory_Malloc(sizeof(struct draw_command_t) * r_max_draw_cmds_count);



	//assert(r_draw_command_groups);
	//assert(r_draw_cmds);

	//r_translucent_draw_groups = malloc(sizeof(draw_group_t) * MAX_MATERIALS);
	//r_translucent_draw_cmds = malloc(sizeof(draw_command_t) * MAX_MATERIALS * MAX_ENTITIES);

	//r_translucent_draw_groups = memory_Malloc(sizeof(draw_group_t) * MAX_MATERIALS, "renderer_Init");
	//r_translucent_draw_cmds = memory_Malloc(sizeof(draw_command_t) * MAX_MATERIALS * MAX_ENTITIES, "renderer_Init");

	glGenQueries(1, &query_object);

    renderer_InitVerts();
	renderer_InitImediateDrawing();
	renderer_InitDebug();

	renderer_Enable(R_DEBUG);
	renderer_Enable(R_VERBOSE_DEBUG);

	//renderer_VerboseDebugOutput(1);

	//renderer_SetShadowMapResolution(512);

	//renderer_Debug(1, 1);

	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "%s: subsystem initialized properly!", __func__);
	return 1;
}

void renderer_Finish()
{

	/*free(r_draw_cmds);
	free(r_draw_groups);

	free(r_translucent_draw_cmds);
	free(r_translucent_draw_groups);*/

	//memory_Free(r_sorted_draw_cmds);
	//memory_Free(r_unsorted_draw_cmds);
	memory_Free(r_draw_command_groups);
	memory_Free(r_light_buffer);

	stack_list_destroy(&r_views);
	list_destroy(&r_sorted_draw_cmds);

	renderer_FinishDebug();
	renderer_FinishImediateDrawing();
	renderer_FinishVerts();

	glDeleteQueries(65536, r_occlusion_queries);
	//memory_Free(r_translucent_draw_cmds);
	//memory_Free(r_translucent_draw_groups);

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}




void renderer_BindDiffuseTexture(int texture_index)
{
	unsigned int gl_handle;

	if(texture_index < 0)
	{
		gl_handle = 0;
	}
	else
	{
		texture_index++;
		gl_handle = ((struct texture_t *)tex_textures.elements + texture_index)->gl_handle;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gl_handle);

	renderer_SetDefaultUniform1i(UNIFORM_texture_sampler0, 0);
	//renderer_SetUniform1i(UNIFORM_texture_sampler0, 0);
}

void renderer_BindNormalTexture(int texture_index)
{
	unsigned int gl_handle;

	if(texture_index < 0)
	{
		gl_handle = 0;
	}
	else
	{
		texture_index++;
		gl_handle = ((struct texture_t *)tex_textures.elements + texture_index)->gl_handle;
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gl_handle);

	renderer_SetDefaultUniform1i(UNIFORM_texture_sampler1, 1);
}

void renderer_BindShadowTexture()
{
	//glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, l_shared_shadow_map);

	renderer_SetDefaultUniform1i(UNIFORM_texture_cube_array_sampler0, 5);
	renderer_BindTextureTexUnit(GL_TEXTURE5, GL_TEXTURE_CUBE_MAP_ARRAY, r_shadow_map_array);
}

void renderer_BindClusterTexture()
{
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, r_cluster_texture);
	renderer_SetDefaultUniform1i(UNIFORM_cluster_texture, 3);
	renderer_SetDefaultUniform1i(UNIFORM_r_clusters_per_row, r_clusters_per_row);
	renderer_SetDefaultUniform1i(UNIFORM_r_cluster_rows, r_cluster_rows);
	renderer_SetDefaultUniform1i(UNIFORM_r_cluster_layers, r_cluster_layers);
}

/*
======================
renderer_SetTexture

sets a texture by it's ENGINE HANDLE...

======================
*/


void renderer_BindTexture(int texture_unit, int texture_target, int texture_index)
{
	int gl_texture;

	struct texture_t *texture;

	if(texture_index < 0 || texture_index >= tex_textures.element_count)
	{
		texture_index = -1;
	}

	texture_index++;

	texture = (struct texture_t *)tex_textures.elements + texture_index;

	/*if(texture->bm_flags & TEXTURE_INVALID)
	{
		texture_index = -1;
	}*/

	gl_texture = texture->gl_handle;
	glActiveTexture(texture_unit);
	glBindTexture(texture_target, gl_texture);
}



/*
======================
renderer_BindTexture

sets a texture by it's GL HANDLE...

======================
*/
void renderer_BindTextureTexUnit(int texture_unit, int texture_target, int texture)
{
	glActiveTexture(texture_unit);
	glBindTexture(texture_target, texture);
}


void renderer_SetClearColor(float r, float g, float b)
{
	r_clear_color.r = r;
	r_clear_color.g = g;
	r_clear_color.b = b;
}

void renderer_SetUniformBuffers()
{
	R_DBG_PUSH_FUNCTION_NAME();

	struct view_def_t *main_view;
	int i;


    if(w_world_vertices)
    {
        main_view = renderer_GetMainViewPointer();

        for(i = 0; i < w_world_vertices_count; i++)
        {
            r_world_vertices_buffer[i].vec3 = w_world_vertices[i].position;
            r_world_vertices_buffer[i].w = 1.0;

            mat4_t_vec4_t_mult(&main_view->view_data.view_matrix, r_world_vertices_buffer + i);
        }

        glBindBuffer(GL_UNIFORM_BUFFER, r_world_vertices_uniform_buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(vec4_t) * w_world_vertices_count, r_world_vertices_buffer, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }



	//int cpu_timer = renderer_StartCpuTimer("renderer_SetUniformBuffers");
    //int gpu_timer = renderer_StartGpuTimer("renderer_SetUniformBuffers");
	glBindBufferBase(GL_UNIFORM_BUFFER, R_LIGHT_UNIFORM_BUFFER_BINDING, r_light_uniform_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, R_BSP_UNIFORM_BUFFER_BINDING, r_bsp_uniform_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, R_WORLD_VERTICES_UNIFORM_BUFFER_BINDING, r_world_vertices_uniform_buffer);
	//renderer_StopTimer(cpu_timer);
	//renderer_StopTimer(gpu_timer);
	R_DBG_POP_FUNCTION_NAME();
}



/*
======================
renderer_SetProjectionMatrix

sets the global variable r_projection_matrix,
which will be used by every drawing command
that doesn't use any "gl_" built in variables
in the shaders.

This function does not upload anything to
OpenGL...

======================
*/
void renderer_SetProjectionMatrix(mat4_t *matrix)
{
	if(!matrix)
	{
		r_projection_matrix.floats[0][0] = 1.0;
		r_projection_matrix.floats[0][1] = 0.0;
		r_projection_matrix.floats[0][2] = 0.0;
		r_projection_matrix.floats[0][3] = 0.0;

		r_projection_matrix.floats[1][0] = 0.0;
		r_projection_matrix.floats[1][1] = 1.0;
		r_projection_matrix.floats[1][2] = 0.0;
		r_projection_matrix.floats[1][3] = 0.0;

		r_projection_matrix.floats[2][0] = 0.0;
		r_projection_matrix.floats[2][1] = 0.0;
		r_projection_matrix.floats[2][2] = 1.0;
		r_projection_matrix.floats[2][3] = 0.0;

		r_projection_matrix.floats[3][0] = 0.0;
		r_projection_matrix.floats[3][1] = 0.0;
		r_projection_matrix.floats[3][2] = 0.0;
		r_projection_matrix.floats[3][3] = 1.0;
	}
	else
	{
		r_projection_matrix = *matrix;
	}

	r_projection_matrix_changed = 1;

}

/*
======================
renderer_SetViewMatrix

sets the global variable r_view_matrix,
which will be used by every drawing command
that doesn't use any "gl_" built in variables
in the shaders.

This function does not upload anything to
OpenGL...

======================
*/
void renderer_SetViewMatrix(mat4_t *matrix)
{
	if(!matrix)
	{
		r_view_matrix.floats[0][0] = 1.0;
		r_view_matrix.floats[0][1] = 0.0;
		r_view_matrix.floats[0][2] = 0.0;
		r_view_matrix.floats[0][3] = 0.0;

		r_view_matrix.floats[1][0] = 0.0;
		r_view_matrix.floats[1][1] = 1.0;
		r_view_matrix.floats[1][2] = 0.0;
		r_view_matrix.floats[1][3] = 0.0;

		r_view_matrix.floats[2][0] = 0.0;
		r_view_matrix.floats[2][1] = 0.0;
		r_view_matrix.floats[2][2] = 1.0;
		r_view_matrix.floats[2][3] = 0.0;

		r_view_matrix.floats[3][0] = 0.0;
		r_view_matrix.floats[3][1] = 0.0;
		r_view_matrix.floats[3][2] = 0.0;
		r_view_matrix.floats[3][3] = 1.0;
	}
	else
	{
		r_view_matrix = *matrix;
	}

	r_view_matrix_changed = 1;

}

/*
======================
renderer_SetModelMatrix

sets the global variable r_model_matrix,
which will be used by every drawing command
that doesn't use any "gl_" built in variables
in the shaders.

This function does not upload anything to
OpenGL...

======================
*/
void renderer_SetModelMatrix(mat4_t *matrix)
{
	if(!matrix)
	{
		r_model_matrix.floats[0][0] = 1.0;
		r_model_matrix.floats[0][1] = 0.0;
		r_model_matrix.floats[0][2] = 0.0;
		r_model_matrix.floats[0][3] = 0.0;

		r_model_matrix.floats[1][0] = 0.0;
		r_model_matrix.floats[1][1] = 1.0;
		r_model_matrix.floats[1][2] = 0.0;
		r_model_matrix.floats[1][3] = 0.0;

		r_model_matrix.floats[2][0] = 0.0;
		r_model_matrix.floats[2][1] = 0.0;
		r_model_matrix.floats[2][2] = 1.0;
		r_model_matrix.floats[2][3] = 0.0;

		r_model_matrix.floats[3][0] = 0.0;
		r_model_matrix.floats[3][1] = 0.0;
		r_model_matrix.floats[3][2] = 0.0;
		r_model_matrix.floats[3][3] = 1.0;
	}
	else
	{
		r_model_matrix = *matrix;
	}



}


/*
======================
renderer_UpdateMatrices

computes the r_model_view_matrix and
r_model_view_projection_matrix,
and uploads all the matrices to
OpenGL...

======================
*/
void renderer_UpdateMatrices()
{
	mat4_t_mult_fast(&r_model_view_matrix, &r_model_matrix, &r_view_matrix);
	mat4_t_mult_fast(&r_model_view_projection_matrix, &r_model_view_matrix, &r_projection_matrix);

	renderer_SetDefaultUniformMatrix4fv(UNIFORM_model_matrix, &r_model_matrix.floats[0][0]);
	renderer_SetDefaultUniformMatrix4fv(UNIFORM_model_view_matrix, &r_model_view_matrix.floats[0][0]);
	renderer_SetDefaultUniformMatrix4fv(UNIFORM_model_view_projection_matrix, &r_model_view_projection_matrix.floats[0][0]);

	if(r_view_matrix_changed)
	{
		renderer_SetDefaultUniformMatrix4fv(UNIFORM_view_matrix, &r_view_matrix.floats[0][0]);
	}

	if(r_projection_matrix_changed)
	{
		renderer_SetDefaultUniformMatrix4fv(UNIFORM_projection_matrix, &r_projection_matrix.floats[0][0]);
	}

	r_view_matrix_changed = 0;
	r_projection_matrix_changed = 0;
}



/*
===============
renderer_SetMaterial

set the material to be used
with following rendering
commands. If the index passed
is not a valid material, the
default material will be used...

===============
*/
void renderer_SetMaterial(int material_index)
{
	material_t *material;
	struct texture_t *texture;
	float color[4];
	int texture_flags = 0;

	if(material_index <= mat_material_count)
	{
		material = &mat_materials[material_index];

		if(material->flags & MATERIAL_INVALID)
			material = &mat_materials[-1];

		color[0] = (float)material->r / 255.0;
		color[1] = (float)material->g / 255.0;
		color[2] = (float)material->b / 255.0;
		color[3] = (float)material->a / 255.0;

		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
		//glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 1024 * ((float)material->roughness / 255.0));

		if(material->diffuse_texture != -1)
		{
			renderer_BindDiffuseTexture(material->diffuse_texture);
			texture_flags |= MATERIAL_USE_DIFFUSE_TEXTURE;
		}

		if(material->normal_texture != -1)
		{
			renderer_BindNormalTexture(material->normal_texture);
			texture_flags |= MATERIAL_USE_NORMAL_TEXTURE;

			texture = (struct texture_t *)tex_textures.elements + material->normal_texture;

			texture_flags |= (material->flags & (MATERIAL_INVERT_NORMAL_X | MATERIAL_INVERT_NORMAL_Y));

			/*if(material->flags & MATERIAL_INVERT_NORMAL_X)
			{
				texture_flags |= MATERIAL_INVERT_NORMAL_X;
			}

			if(material->flags & MATERIAL_INVERT_NORMAL_Y)
			{
				texture_flags |= MATERIAL_INVERT_NORMAL_Y;
			}*/

		}

		renderer_SetDefaultUniform1ui(UNIFORM_material_flags, texture_flags);

		r_material_swaps++;
	}
}

void renderer_SetMaterialColor(vec4_t color)
{
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color.floats);
}


/*
void renderer_UpdateDrawCommandGroups(view_data_t *view_data)
{
	int i;
	int c;
	int group_index = 0;
	draw_command_group_t *group;
	view_material_ref_record_t *ref;
	draw_command_t *view_draw_commands;
	int count;

	r_draw_command_group_count = 0;

	#define R_DRAW_COMMAND_RESIZE_INCREMENT 256

	if(view_data->view_draw_command_list_cursor >= r_max_draw_cmds_count)
	{
		r_max_draw_cmds_count = (view_data->view_draw_command_list_cursor - R_DRAW_COMMAND_RESIZE_INCREMENT - 1) & (~(R_DRAW_COMMAND_RESIZE_INCREMENT - 1));
		memory_Free(r_sorted_draw_cmds);
		r_sorted_draw_cmds = memory_Malloc(sizeof(draw_command_t) * (r_max_draw_cmds_count + R_DRAW_COMMAND_RESIZE_INCREMENT), "renderer_SubmitDrawCommand");
		r_max_draw_cmds_count += R_DRAW_COMMAND_RESIZE_INCREMENT;
	}


	for(i = -1; i < mat_material_count; i++)
	{

		if(mat_materials[i].flags & MATERIAL_INVALID)
		{
			continue;
		}

		ref = &view_data->view_material_refs[i];

		if(ref->last_touched != r_frame)
		{
			continue;
		}

		r_draw_command_groups[r_draw_command_group_count].material_index = i;
		r_draw_command_groups[r_draw_command_group_count].draw_cmds_count = 0;
		r_draw_command_groups[r_draw_command_group_count].max_draw_cmds = ref->frame_ref_count;
		mat_materials[i].r_draw_group = &r_draw_command_groups[r_draw_command_group_count];
		r_draw_command_group_count++;
	}

	r_draw_command_groups[0].draw_cmds = r_sorted_draw_cmds;

	for(i = 1; i < r_draw_command_group_count; i++)
	{
		r_draw_command_groups[i].draw_cmds = r_draw_command_groups[i - 1].draw_cmds + r_draw_command_groups[i - 1].max_draw_cmds;
	}

	c = view_data->view_draw_command_list_cursor;
	view_draw_commands = view_data->view_draw_commands;
	for(i = 0; i < c; i++)
	{
		group = mat_materials[view_draw_commands[i].material_index].r_draw_group;
		group->draw_cmds[group->draw_cmds_count] = view_draw_commands[i];
		group->draw_cmds_count++;
	}
}*/

void renderer_SubmitDrawCommandToView(struct view_data_t *view_data, mat4_t *transform, unsigned short draw_mode, unsigned int start, unsigned int count, int material_index, int indexed_draw)
{
    draw_command_group_t *draw_group;
	struct draw_command_t *draw_command;
	material_t *material;

	view_material_ref_record_t *ref;

	if(material_index < -1 || material_index >= mat_material_count)
	{
		material_index = -1;
	}

	material = &mat_materials[material_index];

	if(material->flags & MATERIAL_INVALID)
	{
		material = &mat_materials[-1];
		material_index = -1;
	}

	/*if(view_data->draw_commands_frame != r_frame)
    {
        view_data->draw_commands_frame = r_frame;
        view_data->draw_commands.element_count = 0;
    }*/

	//if(view_data->draw_commands.element_count >= view_data->draw_commands.max_elements)
	//{
		/* we reached the limit for this frame, so
		don't accept anything else until next frame,
		where the list will have been resized... */
	//	return;
	//}

	//printf("wow\n");

	draw_command = (struct draw_command_t *)view_data->draw_commands.elements + view_data->draw_commands.element_count;
	view_data->draw_commands.element_count++;
	//draw_command = &view_data->view_draw_commands[view_data->view_draw_command_list_cursor];
	//view_data->view_draw_command_list_cursor++;

	draw_command->draw_mode = draw_mode;
	draw_command->flags = 0;

	if(indexed_draw)
	{
		draw_command->flags |= DRAW_COMMAND_FLAG_INDEXED_DRAW;
	}

	draw_command->material_index = material_index;
	draw_command->transform = transform;
	draw_command->start = start;
	draw_command->count = count;
}

void renderer_SubmitDrawCommand(mat4_t *transform, unsigned short draw_mode, unsigned int start, unsigned int count, int material_index, int indexed_draw)
{

    struct view_def_t *main_view;

    main_view = renderer_GetMainViewPointer();

    renderer_SubmitDrawCommandToView(&main_view->view_data, transform, draw_mode, start, count, material_index, indexed_draw);

    #if 0
	draw_command_group_t *draw_group;
	draw_command_t *draw_command;
	material_t *material;

	view_material_ref_record_t *ref;

	if(material_index < -1 || material_index >= mat_material_count)
	{
		material_index = -1;
	}

	material = &mat_materials[material_index];

	if(material->flags & MATERIAL_INVALID)
	{
		material = &mat_materials[-1];
		material_index = -1;
	}

	if(r_draw_cmds_count >= r_max_draw_cmds_count)
	{
		/* we reached the limit for this frame, so
		don't accept anything else until next frame,
		where the list will have been resized... */
		return;
	}

	draw_command = &r_unsorted_draw_cmds[r_draw_cmds_count];
	r_draw_cmds_count++;
	//draw_command = &view_data->view_draw_commands[view_data->view_draw_command_list_cursor];
	//view_data->view_draw_command_list_cursor++;

	draw_command->draw_mode = draw_mode;
	draw_command->flags = 0;

	if(indexed_draw)
	{
		draw_command->flags |= DRAW_COMMAND_FLAG_INDEXED_DRAW;
	}

	draw_command->material_index = material_index;
	draw_command->transform = transform;
	draw_command->start = start;
	draw_command->count = count;

	#endif
}

void renderer_SortViewDrawCommands(struct view_data_t *view)
{

	int i;
	int c;
	int group_index = 0;
	int material_index;
	draw_command_group_t *group;
	view_material_ref_record_t *ref;
	//draw_command_t *view_draw_commands;
	struct draw_command_t *unsorted_draw_commands;
	struct draw_command_t *sorted_draw_commands;
	int count;

	static char material_draw_groups[MAX_MATERIALS + 1];

	r_draw_command_group_count = 0;

	if(view->draw_commands.element_count >= r_sorted_draw_cmds.max_elements)
	{
	    list_resize(&r_sorted_draw_cmds, r_sorted_draw_cmds.max_elements + R_DRAW_COMMAND_LIST_RESIZE_INCREMENT);
	}

	for(i = 0; i <= MAX_MATERIALS; i++)
	{
		material_draw_groups[i] = -1;
	}

    sorted_draw_commands = (struct draw_command_t *)r_sorted_draw_cmds.elements;
    unsorted_draw_commands = (struct draw_command_t *)view->draw_commands.elements;

	for(i = 0; i < view->draw_commands.element_count; i++)
	{
		//material_index = r_unsorted_draw_cmds[i].material_index + 1;
		material_index = unsorted_draw_commands[i].material_index + 1;

		if(material_draw_groups[material_index] < 0)
		{
			material_draw_groups[material_index] = r_draw_command_group_count;
			r_draw_command_groups[r_draw_command_group_count].material_index = material_index - 1;
			r_draw_command_groups[r_draw_command_group_count].max_draw_cmds = 0;
			r_draw_command_groups[r_draw_command_group_count].draw_cmds_count = 0;
			r_draw_command_group_count++;
		}

		group_index = material_draw_groups[material_index];
		r_draw_command_groups[group_index].max_draw_cmds++;
	}

	r_draw_command_groups[0].draw_cmds = sorted_draw_commands;

	for(i = 1; i < r_draw_command_group_count; i++)
	{
		r_draw_command_groups[i].draw_cmds = r_draw_command_groups[i - 1].draw_cmds + r_draw_command_groups[i - 1].max_draw_cmds;
	}

	for(i = 0; i < view->draw_commands.element_count; i++)
	{
		group_index = material_draw_groups[unsorted_draw_commands[i].material_index + 1];
		group = &r_draw_command_groups[group_index];
		group->draw_cmds[group->draw_cmds_count] = unsorted_draw_commands[i];
		group->draw_cmds_count++;
	}


    view->draw_commands.element_count = 0;
	//r_draw_cmds_count = 0;
}

void renderer_SortDrawCommands()
{
    struct view_def_t *main_view;

    main_view = renderer_GetMainViewPointer();

    renderer_SortViewDrawCommands(&main_view->view_data);
    #if 0
	int i;
	int c;
	int group_index = 0;
	int material_index;
	draw_command_group_t *group;
	view_material_ref_record_t *ref;
	//draw_command_t *view_draw_commands;
	draw_command_t *draw_commands;
	int count;

	static char material_draw_groups[MAX_MATERIALS + 1];

	r_draw_command_group_count = 0;

	/*if(view_data->view_draw_command_list_cursor >= r_max_draw_cmds_count)
	{
		r_max_draw_cmds_count = (view_data->view_draw_command_list_cursor - R_DRAW_COMMAND_LIST_RESIZE_INCREMENT - 1) & (~(R_DRAW_COMMAND_LIST_RESIZE_INCREMENT - 1));
		memory_Free(r_sorted_draw_cmds);
		r_sorted_draw_cmds = memory_Malloc(sizeof(draw_command_t) * (r_max_draw_cmds_count + R_DRAW_COMMAND_LIST_RESIZE_INCREMENT), "renderer_SubmitDrawCommand");
		r_max_draw_cmds_count += R_DRAW_COMMAND_LIST_RESIZE_INCREMENT;
	}*/

	if(r_draw_cmds_count >= r_max_draw_cmds_count)
	{
		memory_Free(r_sorted_draw_cmds);
		r_sorted_draw_cmds = memory_Malloc(sizeof(draw_command_t) * (r_max_draw_cmds_count + R_DRAW_COMMAND_LIST_RESIZE_INCREMENT));

		draw_commands = memory_Malloc(sizeof(draw_command_t) * (r_max_draw_cmds_count + R_DRAW_COMMAND_LIST_RESIZE_INCREMENT));

		memcpy(draw_commands, r_unsorted_draw_cmds, sizeof(draw_command_t) * r_max_draw_cmds_count);

		memory_Free(r_unsorted_draw_cmds);

		r_unsorted_draw_cmds = draw_commands;
		r_max_draw_cmds_count += R_DRAW_COMMAND_LIST_RESIZE_INCREMENT;
	}


	for(i = 0; i <= MAX_MATERIALS; i++)
	{
		material_draw_groups[i] = -1;
	}


	for(i = 0; i < r_draw_cmds_count; i++)
	{
		material_index = r_unsorted_draw_cmds[i].material_index + 1;

		if(material_draw_groups[material_index] < 0)
		{
			material_draw_groups[material_index] = r_draw_command_group_count;
			r_draw_command_groups[r_draw_command_group_count].material_index = material_index - 1;
			r_draw_command_groups[r_draw_command_group_count].max_draw_cmds = 0;
			r_draw_command_groups[r_draw_command_group_count].draw_cmds_count = 0;
			r_draw_command_group_count++;
		}

		group_index = material_draw_groups[material_index];
		r_draw_command_groups[group_index].max_draw_cmds++;
	}

	r_draw_command_groups[0].draw_cmds = r_sorted_draw_cmds;

	for(i = 1; i < r_draw_command_group_count; i++)
	{
		r_draw_command_groups[i].draw_cmds = r_draw_command_groups[i - 1].draw_cmds + r_draw_command_groups[i - 1].max_draw_cmds;
	}

	for(i = 0; i < r_draw_cmds_count; i++)
	{
		group_index = material_draw_groups[r_unsorted_draw_cmds[i].material_index + 1];
		group = &r_draw_command_groups[group_index];
		group->draw_cmds[group->draw_cmds_count] = r_unsorted_draw_cmds[i];
		group->draw_cmds_count++;
	}

	r_draw_cmds_count = 0;

	#endif
}

/*
====================================================================
====================================================================
====================================================================
*/

void renderer_SetWindowSize(int width, int height)
{

	SDL_DisplayMode current_display;
	int i;

	if(width < RENDERER_MIN_WIDTH)
	{
		width = RENDERER_MIN_WIDTH;
	}
	else if(width > RENDERER_MAX_WIDTH)
	{
		width = RENDERER_MAX_WIDTH;
	}


	if(height < RENDERER_MIN_HEIGHT)
	{
		height = RENDERER_MIN_HEIGHT;
	}
	else if(height > RENDERER_MIN_HEIGHT)
	{
		height = RENDERER_MIN_HEIGHT;
	}

	if(r_window_width != width || r_window_height != height)
	{
		r_window_width = width;
		r_window_height = height;

		SDL_SetWindowSize(window, r_window_width, r_window_height);
		SDL_GetDisplayMode(0, 0, &current_display);
		SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

		if(r_window_width < r_width || r_window_height < r_height)
		{
			renderer_SetRendererResolution(r_width, r_height, r_msaa_samples);
		}

		if(r_window_width == width && r_window_height == height)
		{
			renderer_Fullscreen(1);
		}
		else
		{
			renderer_Fullscreen(0);
		}
	}
}


/*
======================
renderer_SetRendererResolution

sets the renderer resolution, which
will probably differ from the window
resolution. This allows going fullscreen
without having to render at the window's
resolution...

======================
*/
void renderer_SetRendererResolution(int width, int height, int samples)
{

	int clusters_per_row;
	int cluster_rows;
	int cluster_layers;

	if(width < RENDERER_MIN_WIDTH)
	{
		width = RENDERER_MIN_WIDTH;
	}
	else if(width > RENDERER_MAX_WIDTH)
	{
		width = RENDERER_MAX_WIDTH;
	}

	if(height < RENDERER_MIN_HEIGHT)
	{
		height = RENDERER_MIN_HEIGHT;
	}
	else if(height > RENDERER_MAX_HEIGHT)
	{
		height = RENDERER_MAX_HEIGHT;
	}

	if(samples < RENDERER_MIN_MSAA_SAMPLES)
	{
		samples = RENDERER_MIN_MSAA_SAMPLES;
	}
	else if(samples > RENDERER_MAX_MSAA_SAMPLES)
	{
		samples = RENDERER_MAX_MSAA_SAMPLES;
	}

	r_width = width;
	r_height = height;
	r_msaa_samples = 1;

	if(!r_cbuffer.framebuffer_id)
	{
		r_cbuffer = renderer_CreateFramebuffer(r_width, r_height);
		renderer_AddAttachment(&r_cbuffer, GL_COLOR_ATTACHMENT0, GL_RGBA16F, 1, GL_LINEAR);
		//renderer_AddAttachment(&r_cbuffer, GL_COLOR_ATTACHMENT1, GL_RGBA8, 1, GL_LINEAR);
		renderer_AddAttachment(&r_cbuffer, GL_DEPTH_STENCIL_ATTACHMENT, 0, 1, GL_NEAREST);
	}
	else
	{
		renderer_ResizeFramebuffer(&r_cbuffer, r_width, r_height);
	}

	if(!r_bbuffer.framebuffer_id)
	{
		r_bbuffer = renderer_CreateFramebuffer(r_width, r_height);
		renderer_AddAttachment(&r_bbuffer, GL_COLOR_ATTACHMENT0, GL_RGBA8, 1, GL_LINEAR);
		renderer_ShareAttachment(&r_cbuffer, &r_bbuffer, GL_DEPTH_ATTACHMENT);
		//renderer_AddAttachment(&r_bbuffer, GL_DEPTH_ATTACHMENT, 0, 1);
	}
	else
	{
		renderer_ResizeFramebuffer(&r_bbuffer, r_width, r_height);
	}

	if(r_width > r_window_width || r_height > r_window_height)
	{
		renderer_SetWindowSize(r_width, r_height);
	}


	clusters_per_row = (int)(ceil((float)r_width / (float)R_CLUSTER_WIDTH));
	cluster_rows = (int)(ceil((float)r_height / (float)R_CLUSTER_HEIGHT));


    if(clusters_per_row != r_clusters_per_row || cluster_rows != r_clusters_per_row)
	{
		r_clusters_per_row = clusters_per_row;
		r_cluster_rows = cluster_rows;
		r_cluster_layers = CLUSTER_LAYERS;


        if(r_clusters)
		{
			memory_Free(r_clusters);
		}

		r_clusters = memory_Calloc(r_clusters_per_row * r_cluster_rows * r_cluster_layers, sizeof(struct cluster_t));

		if(r_cluster_texture)
		{
			glDeleteTextures(1, &r_cluster_texture);
		}

		glGenTextures(1, &r_cluster_texture);
		glBindTexture(GL_TEXTURE_3D, r_cluster_texture);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, r_clusters_per_row, r_cluster_rows, r_cluster_layers, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
		glBindTexture(GL_TEXTURE_3D, 0);
	}
}


int renderer_GetFullscreen()
{
    return (r_window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) && 1;
}


void renderer_Fullscreen(int enable)
{
	int w;
	int h;
	SDL_DisplayMode display_mode;
	SDL_Rect display_bounds;

	if(enable)
	{
		if(r_window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
			return;

		SDL_GetDisplayBounds(0, &display_bounds);
		r_window_width = display_bounds.w;
		r_window_height = display_bounds.h;
		r_window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		//SDL_SetWindowSize(window, r_window_width, r_window_height);
		//SDL_SetWindowFullscreen(window, r_window_flags);
		//renderer_SetRendererResolution(r_window_width, r_window_height, 1);
	}
	else
	{
		if(!(r_window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP))
			return;

		r_window_flags &= ~SDL_WINDOW_FULLSCREEN_DESKTOP;
		r_window_width = r_width;
		r_window_height = r_height;

	}

	SDL_SetWindowSize(window, r_window_width, r_window_height);
	SDL_SetWindowFullscreen(window, r_window_flags);

	//renderer_Backbuffer(r_width, r_height, 1);

	r_view_matrix_changed = 1;
	r_projection_matrix_changed = 1;
}

void renderer_ToggleFullscreen()
{
	if(r_window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
	{
		renderer_Fullscreen(0);
	}
	else
	{
		renderer_Fullscreen(1);
	}
}

void renderer_Multisample(int enable)
{
	if(enable)
	{
		r_msaa_samples = 1;
	}
	else
	{
		r_msaa_samples = 4;
	}

//	renderer_UpdateColorbuffer();
}

void renderer_ToggleMultisample()
{
	if(r_msaa_samples > 1)
	{
		renderer_Multisample(0);
	}
	else
	{
		renderer_Multisample(1);
	}
}
/*
void renderer_DeferredRenderer(int enable)
{
	if(enable)
	{
		r_deferred = 1;
	}
	else
	{
		r_deferred = 0;
	}
}*/

void renderer_Fullbright(int enable)
{
	if(enable)
	{
		r_flat = 1;
	}
	else
	{
		r_flat = 0;
	}
}


void renderer_SetShadowMapResolution(int resolution)
{
int i;

	int shadow_map_index;
/*
	if(resolution != r_shadow_map_resolution)
	{

		r_shadow_map_resolution = resolution;

		if(r_shadow_map_resolution <= 0)
		{
			r_draw_shadow_maps = 0;
		}
		else
		{
			r_draw_shadow_maps = 1;
			r_force_shadow_map_update = 1;

			if(r_shadow_map_array)
			{
				glDeleteFramebuffers(1, &r_shadow_map_framebuffer);
				glDeleteTextures(1, &r_shadow_map_array);
			}

			glGenFramebuffers(1, &r_shadow_map_framebuffer);


			glGenTextures(1, &r_shadow_map_array);
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, r_shadow_map_array);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
			glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT16, r_shadow_map_resolution, r_shadow_map_resolution, MAX_VISIBLE_LIGHTS * 6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);


			//glBindFramebuffer(GL_READ_FRAMEBUFFER, r_shadow_map_framebuffer);
		}
	}*/
}

int renderer_GetShadowMapResolution()
{
	return r_shadow_map_resolution;
}



void renderer_SetFrameRateClamping(int clamping)
{
	if(clamping < 0)
	{
		clamping = 0;
	}
	else if(clamping > 2)
	{
		clamping = 2;
	}

	r_frame_clamping = clamping;

	SDL_GL_SetSwapInterval(r_frame_clamping);
}

int renderer_GetFrameRateClamping()
{
	return r_frame_clamping;
}
/*
void renderer_SetRenderer(int renderer)
{
    switch(renderer)
    {
        case RENDERER_TYPE_CLUSTERED_FORWARD:
            renderer_DrawWorld = renderer_DrawWorldClustererdForward;
            renderer_DrawOpaque = renderer_DrawOpaqueClusteredForward;
        break;
    }
}*/



void renderer_RegisterCallback(void (*r_fn)(void), int type)
{
	switch(type)
	{
		case PRE_SHADING_STAGE_CALLBACK:
			renderer_PreShadingRegisteredFunction[pre_shading_render_function_count++] = r_fn;
		break;

		case POST_SHADING_STAGE_CALLBACK:
			renderer_PostShadingRegisteredFunction[post_shading_render_function_count++] = r_fn;
		break;

		case RENDERER_RESOLUTION_CHANGE_CALLBACK:
			renderer_RendererResolutionChangeCallback[renderer_resolution_change_callback_count++] = r_fn;
		break;

		case WINDOW_RESIZE_CALLBACK:
			renderer_WindowResizeCallback[window_resize_callback_count++] = r_fn;
		break;
	}

}

void renderer_ClearRegisteredCallbacks()
{
	pre_shading_render_function_count = 0;
	post_shading_render_function_count = 0;
	renderer_resolution_change_callback_count = 0;
	window_resize_callback_count = 0;
}

/*
====================================================================
====================================================================
====================================================================
*/

#if 0

void renderer_SetMainView(struct view_handle_t view)
{
    struct view_def_t *view_def;
    /*if(view.view_index == INVALID_VIEW_INDEX)
    {
        r_main_view = &r_default_view;
    }
    else
    {
        r_main_view = view;
    }*/

    r_main_view = view;

    if(view.view_index == INVALID_VIEW_INDEX)
    {

    }

	renderer_SetProjectionMatrix(&r_main_view->view_data.projection_matrix);
	renderer_SetViewMatrix(&r_main_view->view_data.view_matrix);
}

#endif


#if 0
view_def_t *renderer_GetMainView()
{
    return r_main_view;
}

#endif


#if 0
struct view_def_t *renderer_GetView(struct view_handle_t view)
{
    if(view.view_index == INVALID_VIEW_INDEX)
    {
        return &r_default_view;
    }

    if(view.view_index >= 0 && view.view_index < r_views.element_count)
    {

    }
}
#endif


#if 0
void renderer_ComputeViewMatrix(view_def_t *view)
{

	view->view_data.view_matrix.floats[0][0] = view->world_orientation.floats[0][0];
	view->view_data.view_matrix.floats[1][0] = view->world_orientation.floats[0][1];
	view->view_data.view_matrix.floats[2][0] = view->world_orientation.floats[0][2];

	view->view_data.view_matrix.floats[3][0] =  (-view->world_position.floats[0]) * view->view_data.view_matrix.floats[0][0]	+
										  		(-view->world_position.floats[1]) * view->view_data.view_matrix.floats[1][0]	+
										  		(-view->world_position.floats[2]) * view->view_data.view_matrix.floats[2][0];



	view->view_data.view_matrix.floats[0][1] = view->world_orientation.floats[1][0];
	view->view_data.view_matrix.floats[1][1] = view->world_orientation.floats[1][1];
	view->view_data.view_matrix.floats[2][1] = view->world_orientation.floats[1][2];

	view->view_data.view_matrix.floats[3][1] =  (-view->world_position.floats[0]) * view->view_data.view_matrix.floats[0][1]	+
										  		(-view->world_position.floats[1]) * view->view_data.view_matrix.floats[1][1]	+
										  		(-view->world_position.floats[2]) * view->view_data.view_matrix.floats[2][1];




	view->view_data.view_matrix.floats[0][2] = view->world_orientation.floats[2][0];
	view->view_data.view_matrix.floats[1][2] = view->world_orientation.floats[2][1];
	view->view_data.view_matrix.floats[2][2] = view->world_orientation.floats[2][2];

	view->view_data.view_matrix.floats[3][2] =  (-view->world_position.floats[0]) * view->view_data.view_matrix.floats[0][2]	+
										 		(-view->world_position.floats[1]) * view->view_data.view_matrix.floats[1][2]	+
										  		(-view->world_position.floats[2]) * view->view_data.view_matrix.floats[2][2];


	view->view_data.view_matrix.floats[0][3] = 0.0;
	view->view_data.view_matrix.floats[1][3] = 0.0;
	view->view_data.view_matrix.floats[2][3] = 0.0;
	view->view_data.view_matrix.floats[3][3] = 1.0;
}
#endif

#if 0
void renderer_ComputeActiveViewMatrix()
{
    renderer_ComputeViewMatrix(r_main_view);
}
#endif

/*
void renderer_SetMainView(camera_t *view)
{
	r_main_view = view;
	main_view = view;
}*/

#if 0
void renderer_SetupView(camera_t *view)
{
	//renderer_SetActiveView(view);
	//renderer_UpdateDrawCommandGroups(&view->view_data);
	//renderer_UpdateDrawCommandGroupsFromView(view);
	//renderer_SetViewVisibleWorld(view);
	//renderer_SetViewVisibleLights(view);

	//light_BindCache();
}
#endif

void renderer_SetViewData(struct view_data_t *view_data)
{
	//gpu_BindGpuHeap();
	//light_BindCache();

	//renderer_SetProjectionMatrix(&view_data->projection_matrix);
	//renderer_SetViewMatrix(&view_data->view_matrix);
	//renderer_SetViewDrawCommands(view_data);
	//renderer_SetViewLightData(view_data);
}


void renderer_SetViewLightData(struct view_data_t *view_data)
{
	#if 0
	int i;
	int c;
	int light_index;
	int gpu_index = 0;

	float s;
	float e;

	int x;
	int y;
	int z;

	int cluster_x_start;
	int cluster_y_start;
	int cluster_z_start;

	int cluster_x_end;
	int cluster_y_end;
	int cluster_z_end;

	int cluster_index;
	int offset;


	vec4_t light_position;
	light_position_t *pos;
	light_params_t *parms;
	view_light_t *view_lights;

	struct gpu_light_t *lights;

	if(!l_light_cache_uniform_buffer)
		return;


	R_DBG_PUSH_FUNCTION_NAME();


	//glBindBuffer(GL_UNIFORM_BUFFER, l_light_cache_uniform_buffer);
	lights = r_light_buffer;

	view_lights = view_data->view_lights;

	for(i = 0; i < view_data->view_lights_list_cursor && i < MAX_VISIBLE_LIGHTS; i++)
	{
		light_index = view_lights[i].light_index;
		pos = &l_light_positions[light_index];
		parms = &l_light_params[light_index];

		light_position.x = pos->position.x;
		light_position.y = pos->position.y;
		light_position.z = pos->position.z;
		light_position.w = 1.0;

		mat4_t_vec4_t_mult(&view_data->view_matrix, &light_position);

		lights[i].position_radius.x = light_position.x;
		lights[i].position_radius.y = light_position.y;
		lights[i].position_radius.z = light_position.z;

		lights[i].position_radius.w = LIGHT_RADIUS(parms->radius);

		lights[i].color_energy.r = (float)parms->r / 255.0;
		lights[i].color_energy.g = (float)parms->g / 255.0;
		lights[i].color_energy.b = (float)parms->b / 255.0;
		lights[i].color_energy.a = LIGHT_ENERGY(parms->energy);

		//lights[i].x_y = (parms->y << 16) | parms->x;
		lights[i].bm_flags = parms->bm_flags & (~LIGHT_GENERATE_SHADOWS);
	}

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct gpu_light_t) * MAX_VISIBLE_LIGHTS, lights);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);


	/*************************************************************/
	/*************************************************************/
	/*************************************************************/


	for(z = 0; z < CLUSTER_LAYERS; z++)
	{
		for(y = 0; y < CLUSTER_ROWS; y++)
		{
			for(x = 0; x < CLUSTERS_PER_ROW; x++)
			{
				cluster_index = CLUSTER_OFFSET(x, y, z);
				l_clusters[cluster_index].light_indexes_bm = 0;
			}
		}
	}


	view_lights = view_data->view_lights;
	c = view_data->view_lights_list_cursor;

	for(i = 0; i < c && i < MAX_VISIBLE_LIGHTS; i++)
	{
		light_index = view_lights[i].light_index;
		parms = &l_light_params[light_index];

		UNPACK_CLUSTER_INDEXES2(cluster_x_start, cluster_y_start, cluster_z_start, cluster_x_end, cluster_y_end, cluster_z_end, view_lights[i].view_clusters);

		for(z = cluster_z_start; z <= cluster_z_end && z < CLUSTER_LAYERS; z++)
		{
			for(y = cluster_y_start; y <= cluster_y_end && y < CLUSTER_ROWS; y++)
			{
				for(x = cluster_x_start; x <= cluster_x_end && x < CLUSTERS_PER_ROW; x++)
				{
					cluster_index = CLUSTER_OFFSET(x, y, z);
					l_clusters[cluster_index].light_indexes_bm |= 1 << i;
				}
			}
		}

	}


	/* this is a bottleneck... */
	glBindTexture(GL_TEXTURE_3D, l_cluster_texture);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, CLUSTERS_PER_ROW, CLUSTER_ROWS, CLUSTER_LAYERS, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, l_clusters);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, CLUSTERS_PER_ROW, CLUSTER_ROWS, CLUSTER_LAYERS, GL_RED_INTEGER, GL_UNSIGNED_INT, l_clusters);
	glBindTexture(GL_TEXTURE_3D, 0);







	R_DBG_POP_FUNCTION_NAME();

	#endif
}

void renderer_SetViewDrawCommands(struct view_data_t *view_data)
{
	#if 0
	int i;
	int c;
	int group_index = 0;
	draw_command_group_t *group;
	view_material_ref_record_t *ref;
	draw_command_t *view_draw_commands;
	int count;

	static char material_draw_groups[MAX_MATERIALS + 1];

	r_draw_command_group_count = 0;

	if(view_data->view_draw_command_list_cursor >= r_max_draw_cmds_count)
	{
		r_max_draw_cmds_count = (view_data->view_draw_command_list_cursor - R_DRAW_COMMAND_LIST_RESIZE_INCREMENT - 1) & (~(R_DRAW_COMMAND_LIST_RESIZE_INCREMENT - 1));
		memory_Free(r_sorted_draw_cmds);
		r_sorted_draw_cmds = memory_Malloc(sizeof(draw_command_t) * (r_max_draw_cmds_count + R_DRAW_COMMAND_LIST_RESIZE_INCREMENT));
		r_max_draw_cmds_count += R_DRAW_COMMAND_LIST_RESIZE_INCREMENT;
	}

	for(i = 0; i < MAX_MATERIALS + 1; i++)
	{
		r_draw_command_groups[i].draw_cmds_count = 0;
		r_draw_command_groups[i].max_draw_cmds = 0;
	}

	c = view_data->view_draw_command_list_cursor;
	view_draw_commands = view_data->view_draw_commands;

	for(i = 0; i < c; i++)
	{
		if(!r_draw_command_groups[r_draw_command_group_count].draw_cmds_count)
		{
			material_draw_groups[view_draw_commands[i].material_index] = r_draw_command_group_count;
			group_index = r_draw_command_group_count;
			r_draw_command_groups[group_index].material_index = view_draw_commands[i].material_index;
			r_draw_command_group_count++;
		}
		else
		{
			group_index = material_draw_groups[view_draw_commands[i].material_index];
		}

		r_draw_command_groups[group_index].max_draw_cmds++;
	}

	r_draw_command_groups[0].draw_cmds = r_sorted_draw_cmds;

	for(i = 1; i < r_draw_command_group_count; i++)
	{
		r_draw_command_groups[i].draw_cmds = r_draw_command_groups[i - 1].draw_cmds + r_draw_command_groups[i - 1].max_draw_cmds;
	}

	c = view_data->view_draw_command_list_cursor;
	view_draw_commands = view_data->view_draw_commands;
	for(i = 0; i < c; i++)
	{
		group_index = material_draw_groups[view_draw_commands[i].material_index];
		group = &r_draw_command_groups[group_index];
		group->draw_cmds[group->draw_cmds_count] = view_draw_commands[i];
		group->draw_cmds_count++;
	}

	#endif
}


void renderer_SetViewVisibleWorld(struct view_data_t *view_data)
{
	#if 0
	int j;
	int leaf_count;
	int c;
	int i;

	int start;
	int next;

	bsp_striangle_t *triangle;
	batch_t *batch;
	bsp_dleaf_t *leaf;

	if(view_data->view_leaves_list_cursor)
	{

		leaf_count = view_data->view_leaves_list_cursor;

		for(j = 0; j < leaf_count; j++)
		{

			leaf = view_data->view_leaves[j];

			c = leaf->tris_count;

			/* add it's triangles for rendering... */
			for(i = 0; i < c; i++)
			{
				triangle = &leaf->tris[i];
				batch = &world_batches[triangle->triangle_group];
				//group = &world_triangle_groups[triangle->triangle_group];

				start = batch->start;
				next = batch->next;

				/* the world's GL_ELEMENT_ARRAY_BUFFER indexes into
				the gpu heap, so this is why world_start is added here... */

				/*assert(start >= 0);
				assert(next >= 0);*/

				//printf("first: %d\n", world_start);

				//index_buffer[0] = world_start + triangle->first_vertex;

				//assert(start + next >= 0);

				index_buffer[start + next	] = world_start + triangle->first_vertex;
				index_buffer[start + next + 1] = world_start + triangle->first_vertex + 1;
				index_buffer[start + next + 2] = world_start + triangle->first_vertex + 2;

				batch->next += 3;
			}
		}

		//gpu_Write(world_index_handle, 0, index_buffer, sizeof(int) * world_vertices_count);

		/*log_LogMessage(LOG_MESSAGE_NOTIFY, "after writting to index_buffer");
		memory_CheckCorrupted();*/

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world_element_buffer);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * world_start, sizeof(int) * world_vertices_count, index_buffer);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	#endif
}


/*
====================================================================
====================================================================
====================================================================
*/

void renderer_StartFrame()
{
    //int cpu_timer = renderer_StartCpuTimer("renderer_StartFrame");
    renderer_ComputeMainViewMatrix();
    renderer_VisibleWorld();
    renderer_VisibleLights();
    renderer_VisibleEntities();

    int mask = GL_DEPTH_BUFFER_BIT;

    if(r_clear_colorbuffer)
    {
        mask |= GL_COLOR_BUFFER_BIT;
    }

    renderer_BindFramebuffer(GL_DRAW_FRAMEBUFFER, &r_cbuffer);
    glClearColor(r_clear_color.r, r_clear_color.g, r_clear_color.b, 1.0);
	glClear(mask);

	//gpu_BindGpuHeap();

	renderer_EnableVertexReads();

	//renderer_StopTimer(cpu_timer);
}

void renderer_DrawFrame()
{

	int i;

	float s;
	float e;
    int timer;

    renderer_StartFrame();
	renderer_SetUniformBuffers();

	for(i = 0; i < pre_shading_render_function_count; i++)
	{
		renderer_PreShadingRegisteredFunction[i]();
	}

	renderer_SortDrawCommands();


    //if(r_z_prepass)
    //{
    renderer_ZPrePass();
    //}

    renderer_DrawWorld();

    renderer_DrawParticles();

	//if(r_bloom && (!r_flat))
	//{
	//	renderer_DrawBloom();
	//}

	if(r_tonemap && (!r_flat) && (!r_wireframe))
	{
		renderer_Tonemap();
	}
	else
	{
		renderer_BlitColorbuffer();
	}

	for(i = 0; i < post_shading_render_function_count; i++)
	{
		renderer_PostShadingRegisteredFunction[i]();
	}

	if(r_debug)
	{
		renderer_DrawDebug();
	}

	renderer_BlitBackbuffer();

	gui_DrawGUI();
	renderer_EndFrame();
}



void renderer_EndFrame()
{
	//glDisable(GL_MULTISAMPLE);

	//gpu_UnbindGpuHeap();

	renderer_DisableVertexReads();

	SDL_GL_SwapWindow(window);
	r_frame++;


	r_draw_calls = 0;
	r_material_swaps = 0;
	r_shader_swaps = 0;
	r_shader_uniform_updates = 0;
	r_frame_vert_count = 0;
//	r_draw_cmds_count = 0;
}


void renderer_ZPrePass()
{
	int i;
	int j;
	int c;

	int result;

	struct batch_t *batch;
    int draw_cmd_count;
    struct draw_command_t *draw_cmds;

    struct view_def_t *active_view = renderer_GetMainViewPointer();

	renderer_SetShader(r_z_pre_pass_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(struct compact_vertex_t), &((struct compact_vertex_t *)0)->position);

	renderer_SetProjectionMatrix(&active_view->view_data.projection_matrix);
	renderer_SetViewMatrix(&active_view->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);

	renderer_UpdateMatrices();


	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	c = w_world_batch_count;

	/*for(i = 0; i < c; i++)
	{
		batch = &w_world_batches[i];
		renderer_DrawElements(GL_TRIANGLES, batch->next, GL_UNSIGNED_INT, (void *)((batch->start + w_world_index_start) * sizeof(int)));
	}*/

    /*#define BATCH_SIZE 30

    i = 0;

	while(i < w_world_sorted_index_count)
    {
        c = BATCH_SIZE;

        if(i + c > w_world_sorted_index_count)
        {
            c = w_world_sorted_index_count - i;
        }

        renderer_DrawElements(GL_TRIANGLES, c, GL_UNSIGNED_INT, (void *)((w_world_sorted_index_start + i) * sizeof(int)));

        i += BATCH_SIZE;
    }*/


    for(i = 0; i < w_world_z_batch_count; i++)
    {
        glBeginQuery(GL_ANY_SAMPLES_PASSED, r_occlusion_queries[i]);
        batch = w_world_z_batches + i;
        renderer_DrawElements(GL_TRIANGLES, batch->next, GL_UNSIGNED_INT, (void *)((w_world_sorted_index_start + batch->start) * sizeof(int)));
        glEndQuery(GL_ANY_SAMPLES_PASSED);
    }

    glFlush();

    //renderer_DrawElements(GL_TRIANGLES, w_world_sorted_index_count, GL_UNSIGNED_INT, (void *)(w_world_sorted_index_start * sizeof(int)));

	/*for(i = 0; i < w_world_sorted_index_count; i += 3)
    {
        renderer_DrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void *)((w_world_sorted_index_start + i) * sizeof(int)));
    }*/

	/* draw movable geometry... */
    /*for(i = 0; i < r_draw_command_group_count; i++)
    {
        draw_cmd_count = r_draw_command_groups[i].draw_cmds_count;
        draw_cmds = r_draw_command_groups[i].draw_cmds;

        for(j = 0; j < draw_cmd_count; j++)
        {
            renderer_SetModelMatrix(draw_cmds[j].transform);
            renderer_UpdateMatrices();

            if(draw_cmds[j].flags & DRAW_COMMAND_FLAG_INDEXED_DRAW)
            {
                renderer_DrawElements(draw_cmds[j].draw_mode, draw_cmds[j].count, GL_UNSIGNED_INT, (void *)(draw_cmds[j].start * sizeof(int)));
            }
            else
            {
                renderer_DrawArrays(draw_cmds[j].draw_mode, draw_cmds[j].start, draw_cmds[j].count);
            }
        }
    }*/

    c = 0;

    //for(i = 0; i < w_world_z_batch_count; i++)
    {
        /*glGetQueryObjectiv(r_occlusion_queries[i], GL_QUERY_RESULT_AVAILABLE, &result);

        if(!result)
        {
            w_world_z_batches[i].next = -1;
        }*/

        /*glGetQueryObjectiv(r_occlusion_queries[i], GL_QUERY_RESULT, &result);

        if(!result)
        {
            w_world_z_batches[i].start = -1;
        }*/
    }

    //printf("%d leafs visible\n", c);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	//renderer_StopTimer(cpu_timer);
	//renderer_StopTimer(gpu_timer);
}

void renderer_DrawWorld()
{

    struct view_def_t *active_view;
    int i;
    int j;
    int c;

    struct batch_t *batch;
    int draw_cmd_count;
    struct draw_command_t *draw_cmds;

    int draw_count = 0;

    active_view = renderer_GetMainViewPointer();

    if(r_flat)
	{
		renderer_SetShader(r_flat_pass_shader);
	}
	else
	{

	    /*if(r_wireframe)
        {
            renderer_SetShader(r_wireframe_pass_shader);
        }
        else*/
        {
            renderer_SetShader(r_forward_pass_shader);
            renderer_BindClusterTexture();
            renderer_SetDefaultUniform1i(UNIFORM_r_width, r_width);
            renderer_SetDefaultUniform1i(UNIFORM_r_height, r_height);
            renderer_SetDefaultUniform1i(UNIFORM_r_frame, r_frame);
            renderer_SetDefaultUniform1i(UNIFORM_r_world_vertices_count, w_world_vertices_count);
        }
	}

	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(struct compact_vertex_t), &((struct compact_vertex_t *)0)->position);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(struct compact_vertex_t), &((struct compact_vertex_t *)0)->normal);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(struct compact_vertex_t), &((struct compact_vertex_t *)0)->tangent);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, sizeof(struct compact_vertex_t), &((struct compact_vertex_t *)0)->tex_coord);

	renderer_SetProjectionMatrix(&active_view->view_data.projection_matrix);
	renderer_SetViewMatrix(&active_view->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_UpdateMatrices();



    /* draw static geometry... */
	c = w_world_batch_count;

	/*if(r_wireframe)
    {
        glDepthFunc(GL_LEQUAL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        renderer_SetMaterialColor(vec4(0.0, 0.0, 1.0, 1.0));

        for(i = 0; i < c; i++)
        {
            batch = &w_world_batches[i];
            renderer_DrawElements(GL_TRIANGLES, batch->next, GL_UNSIGNED_INT, (void *)((batch->start + w_world_index_start) * sizeof(int)));
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else*/
    {
        //if(r_z_prepass)
        //{
        glDepthFunc(GL_EQUAL);
        glDepthMask(GL_FALSE);
        /*}
        else
        {
            glDepthFunc(GL_LESS);
            glDepthMask(GL_TRUE);
        }*/

        for(i = 0; i < c; i++)
        {
            batch = &w_world_batches[i];
            renderer_SetMaterial(batch->material_index);
            renderer_DrawElements(GL_TRIANGLES, batch->next, GL_UNSIGNED_INT, (void *)((batch->start + w_world_index_start) * sizeof(int)));
        }
    }

	/*if(r_wireframe)
    {
        glDepthFunc(GL_LEQUAL);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        renderer_SetMaterialColor(vec4(0.0, 1.0, 0.0, 1.0));

        for(i = 0; i < r_draw_command_group_count; i++)
        {
            draw_cmd_count = r_draw_command_groups[i].draw_cmds_count;
            draw_cmds = r_draw_command_groups[i].draw_cmds;

            for(j = 0; j < draw_cmd_count; j++)
            {
                renderer_SetModelMatrix(draw_cmds[j].transform);
                renderer_UpdateMatrices();

                if(draw_cmds[j].flags & DRAW_COMMAND_FLAG_INDEXED_DRAW)
                {
                    renderer_DrawElements(draw_cmds[j].draw_mode, draw_cmds[j].count, GL_UNSIGNED_INT, (void *)(draw_cmds[j].start * sizeof(int)));
                }
                else
                {
                    renderer_DrawArrays(draw_cmds[j].draw_mode, draw_cmds[j].start, draw_cmds[j].count);
                }
            }
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else*/
    {
        //if(r_z_prepass)
        //{
        //glDepthFunc(GL_EQUAL);
        //glDepthMask(GL_FALSE);
        /*}
        else
        {*/
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);
        /*}*/

        /* draw movable geometry... */
        for(i = 0; i < r_draw_command_group_count; i++)
        {
            renderer_SetMaterial(r_draw_command_groups[i].material_index);
            draw_cmd_count = r_draw_command_groups[i].draw_cmds_count;
            draw_cmds = r_draw_command_groups[i].draw_cmds;

            for(j = 0; j < draw_cmd_count; j++)
            {
                renderer_SetModelMatrix(draw_cmds[j].transform);
                renderer_UpdateMatrices();

                if(draw_cmds[j].flags & DRAW_COMMAND_FLAG_INDEXED_DRAW)
                {
                    renderer_DrawElements(draw_cmds[j].draw_mode, draw_cmds[j].count, GL_UNSIGNED_INT, (void *)(draw_cmds[j].start * sizeof(int)));
                }
                else
                {
                    renderer_DrawArrays(draw_cmds[j].draw_mode, draw_cmds[j].start, draw_cmds[j].count);
                }
            }
        }
    }

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
}



void renderer_Tonemap()
{

    //int cpu_timer = renderer_StartCpuTimer("renderer_Tonemap");
    //int gpu_timer = renderer_StartGpuTimer("renderer_Tonemap");

	renderer_BindFramebuffer(GL_DRAW_FRAMEBUFFER, &r_bbuffer);
	//renderer_BindFramebuffer(GL_READ_FRAMEBUFFER, &r_cbuffer);

	//glClear(GL_COLOR_BUFFER_BIT);
	//glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_width, r_height, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

	renderer_SetShader(r_tonemap_shader);
	renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_cbuffer.color_attachments[0].handle);
	renderer_SetDefaultUniform1i(UNIFORM_texture_sampler0, 0);

	//glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	renderer_EnableImediateDrawing();
	renderer_Rectf(-1.0, -1.0, 1.0, 1.0);
	renderer_DisableImediateDrawing();

	//glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	//renderer_StopTimer(cpu_timer);
	//renderer_StopTimer(gpu_timer);
}

void renderer_DrawTranslucent()
{

}



void renderer_DrawShadowMaps()
{
    #if 0
	camera_t *active_camera = camera_GetActiveCamera();
	//triangle_group_t *triangle_group;
	light_position_t *pos;
	light_params_t *parms;
	//int c = brush_count;
	mat4_t mvm;

	int i;
	int j;
	int light_index;

	int shadow_map_res;



	//int cache_index;
	int shadow_map_index;

	//if(!w_world_leaves)
	//	return;

	R_DBG_PUSH_FUNCTION_NAME();

	renderer_SetShader(r_shadow_pass_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(struct compact_vertex_t), &((struct compact_vertex_t *)0)->position);

	glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(0.0, 0.0);
	//glCullFace(GL_FRONT);


	renderer_SetProjectionMatrix(&l_shadow_map_projection_matrix);
	renderer_SetModelMatrix(NULL);

	shadow_map_res = renderer_GetShadowMapResolution();

	glDepthMask(GL_TRUE);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_shadow_map_framebuffer);
	glDrawBuffer(GL_NONE);
	glViewport(0, 0, shadow_map_res, shadow_map_res);


	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);


	for(j = 0; j < w_visible_lights_count; j++)
	{
		light_index = w_visible_lights[j];
		pos = &l_light_positions[light_index];
		parms = &l_light_params[light_index];

		//if(!(parms->bm_flags & LIGHT_GENERATE_SHADOWS))
		//continue;

		if((!(parms->bm_flags & LIGHT_UPDATE_SHADOW_MAP)) && (!r_force_shadow_map_update))
		{
			continue;
		}


		//if(parms->bm_flags & LIGHT_DROPPED_SHADOW)
		//	continue;

		parms->bm_flags &= ~LIGHT_UPDATE_SHADOW_MAP;

		shadow_map_index = l_light_params[light_index].shadow_map;

		for(i = 0; i < 6; i++)
		{
			glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, r_shadow_map_array, 0, shadow_map_index * 6 + i);
			glClear(GL_DEPTH_BUFFER_BIT);

			if(w_world_leaves)
			{
				mvm = l_shadow_map_mats[i];

				mvm.floats[3][0] = mvm.floats[0][0] * (-pos->position.x) + mvm.floats[1][0] * (-pos->position.y) + mvm.floats[2][0] * (-pos->position.z);
				mvm.floats[3][1] = mvm.floats[0][1] * (-pos->position.x) + mvm.floats[1][1] * (-pos->position.y) + mvm.floats[2][1] * (-pos->position.z);
				mvm.floats[3][2] = mvm.floats[0][2] * (-pos->position.x) + mvm.floats[1][2] * (-pos->position.y) + mvm.floats[2][2] * (-pos->position.z);
				mvm.floats[3][3] = 1.0;

				renderer_SetViewMatrix(&mvm);
				renderer_UpdateMatrices();

				//glCullFace(GL_FRONT);
				//glPolygonOffset(-1.0, 0.0);
				//renderer_DrawElements(GL_TRIANGLES, parms->visible_triangles.element_count, GL_UNSIGNED_INT, (void *)(sizeof(int) * parms->indices_start));

				glCullFace(GL_BACK);
				glPolygonOffset(2.0, 0.0);
				renderer_DrawElements(GL_TRIANGLES, parms->visible_triangles.element_count, GL_UNSIGNED_INT, (void *)(sizeof(int) * parms->indices_start));
			}
		}
	}

	r_force_shadow_map_update = 0;

	glViewport(0, 0, r_width, r_height);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glCullFace(GL_BACK);

	R_DBG_POP_FUNCTION_NAME();

	#endif
}


void renderer_DrawParticles()
{
	int i;
	int c;
	//camera_t *active_camera;
	struct particle_system_t *ps;
	struct particle_system_t *particle_systems;

//	active_camera = camera_GetActiveCamera();
    struct view_def_t *active_view = renderer_GetMainViewPointer();

	renderer_SetShader(r_particle_forward_pass_shader);
	renderer_BindClusterTexture();
	renderer_SetDefaultUniform1i(UNIFORM_r_width, r_width);
	renderer_SetDefaultUniform1i(UNIFORM_r_height, r_height);

	renderer_SetProjectionMatrix(&active_view->view_data.projection_matrix);
	renderer_SetViewMatrix(&active_view->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_UpdateMatrices();

	glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

    renderer_ClearVertexAttribPointers();
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 4, GL_FLOAT, 0, 0, NULL);

	particle_systems = (struct particle_system_t *)ps_particle_systems.elements;
	c = ps_particle_systems.element_count;

	for(i = 0; i < c; i++)
	{
		ps = particle_systems + i;

		if(ps->flags & PARTICLE_SYSTEM_FLAG_INVALID)
		{
			continue;
		}

		if(!ps->particle_count)
		{
			continue;
		}

		//renderer_SetTexture(GL_TEXTURE0, GL_TEXTURE_2D, ps->texture);
		renderer_BindTexture(GL_TEXTURE0, GL_TEXTURE_2D_ARRAY, ps->texture);
		renderer_SetDefaultUniform4fv(UNIFORM_particle_positions, ps->particle_count, (float *)ps->particle_positions);
		renderer_SetDefaultUniform1iv(UNIFORM_particle_frames, ps->particle_count, ps->particle_frames);
		renderer_SetDefaultUniform1i(UNIFORM_texture_array_sampler0, 0);

		renderer_DrawArraysInstanced(GL_QUADS, ps_particle_quad_start, 4, ps->particle_count);
		//renderer_DrawArrays(GL_QUADS, ps_particle_quad_start, 4);
	}

	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

}



void renderer_DrawGUI()
{

}


void renderer_DrawBloom()
{
    #if 0
	int i;


	renderer_EnableImediateDrawing();

	renderer_SetShader(r_bloom0_shader);
	renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_cbuffer.color_attachments[0].handle);
	renderer_SetDefaultUniform1i(UNIFORM_texture_sampler0, 0);

	renderer_SetProjectionMatrix(NULL);
	renderer_SetViewMatrix(NULL);
	renderer_SetModelMatrix(NULL);


	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_intensity_id);
	glViewport(0, 0, r_width, r_height);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	renderer_Rectf(-1.0, -1.0, 1.0, 1.0);


	renderer_SetShader(r_bloom1_shader);
	renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_intensity_color);
	renderer_SetDefaultUniform1i(UNIFORM_texture_sampler0, 0);

	for(i = 1; i < 3; i++)
	{

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_bloom_fbs[i]);
		glViewport(0, 0, r_width >> (1 + i), r_height >> (1 + i));


		renderer_SetDefaultUniform1i(UNIFORM_r_width, r_width >> (1 + i));
		renderer_SetDefaultUniform1i(UNIFORM_r_height, r_height >> (1 + i));
		renderer_SetDefaultUniform1i(UNIFORM_r_bloom_radius, r_bloom_radius[i]);

		switch(i)
		{
			case 1:
				renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_intensity_color);
			break;

			default:
				renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_bloom_texs[1][i - 1]);
			break;
		}
		/*if(!i)
		{
			renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_intensity_color);
		}
		else
		{
			renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_bloom_texs[1][i - 1]);
		}*/


		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		renderer_Rectf(-1.0, -1.0, 1.0, 1.0);


		renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_bloom_texs[0][i]);
		renderer_SetDefaultUniform1i(UNIFORM_r_width, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		renderer_Rectf(-1.0, -1.0, 1.0, 1.0);
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	renderer_SetShader(r_blit_texture_shader);
	renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_bloom_texs[1][2]);
	//renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_intensity_color);
	renderer_SetDefaultUniform1i(UNIFORM_texture_sampler0, 0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_cbuffer.framebuffer_id);
	glViewport(0, 0, r_width, r_height);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	renderer_Rectf(-1.0, -1.0, 1.0, 1.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	renderer_DisableImediateDrawing();

	#endif
}

void renderer_DrawSkyBox()
{
	#if 0
	mat4_t world_to_camera_rotation;
	camera_t *active_camera = camera_GetActiveCamera();
	texture_t *skb;
	int pos_x;
	int neg_x;
	int pos_y;
	int neg_y;
	int pos_z;
	int neg_z;
	int skybox_tex;
	memcpy(&world_to_camera_rotation, &active_camera->world_to_camera_matrix, sizeof(mat4_t));

	world_to_camera_rotation.floats[3][0] = 0.0;
	world_to_camera_rotation.floats[3][1] = 0.0;
	world_to_camera_rotation.floats[3][2] = 0.0;


	skybox_tex = texture_GetTexture("env");
	//skb = &textures[skybox_tex];


	/*glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();*/

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&world_to_camera_rotation.floats[0][0]);
	shader_UseShader(skybox_shader);
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_CUBE_SAMPLER0, 0);

	/*glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, light_cache_shadow_maps[0]);*/

	texture_BindTexture(skybox_tex, GL_TEXTURE0);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0x0, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glBegin(GL_QUADS);

	//#if 0
	/* +X */
	glVertex3f( 1.0, 1.0,-1.0);
	glVertex3f( 1.0,-1.0,-1.0);
	glVertex3f( 1.0,-1.0, 1.0);
	glVertex3f( 1.0, 1.0, 1.0);
	/* -X */
	glVertex3f(-1.0, 1.0, 1.0);
	glVertex3f(-1.0,-1.0, 1.0);
	glVertex3f(-1.0,-1.0,-1.0);
	glVertex3f(-1.0, 1.0,-1.0);

	/* +Y */
	glVertex3f(-1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0,-1.0);
	glVertex3f( 1.0, 1.0,-1.0);
	glVertex3f( 1.0, 1.0, 1.0);

	/* -Y */
	glVertex3f( 1.0,-1.0, 1.0);
	glVertex3f( 1.0,-1.0,-1.0);
	glVertex3f(-1.0,-1.0,-1.0);
	glVertex3f(-1.0,-1.0, 1.0);

	/* +Z */
	glVertex3f( 1.0, 1.0, 1.0);
	glVertex3f( 1.0,-1.0, 1.0);
	glVertex3f(-1.0,-1.0, 1.0);
	glVertex3f(-1.0, 1.0, 1.0);

	/* -Z */
	glVertex3f(-1.0, 1.0,-1.0);
	glVertex3f(-1.0,-1.0,-1.0);
	glVertex3f( 1.0,-1.0,-1.0);
	glVertex3f( 1.0, 1.0,-1.0);

	//#endif

	glEnd();

	//glEnable(GL_DEPTH_TEST);
	//glDisable(GL_TEXTURE_2D);
	//glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDisable(GL_STENCIL_TEST);

	/*glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);*/

	#endif

}

/*
====================================================================
====================================================================
====================================================================
*/


void renderer_BlitColorbuffer()
{
	//renderer_BindBackbuffer(1, 0);
	//renderer_BindColorbuffer(0, 1);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, r_cbuffer_id);
	//glReadBuffer(GL_COLOR_ATTACHMENT0);

	/*glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_bbuffer.framebuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glViewport(0, 0, r_width, r_height);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, r_cbuffer.framebuffer_id);
	glReadBuffer(GL_COLOR_ATTACHMENT0);*/

	renderer_BindFramebuffer(GL_DRAW_FRAMEBUFFER, &r_bbuffer);
	renderer_BindFramebuffer(GL_READ_FRAMEBUFFER, &r_cbuffer);

	//while(glGetError() != GL_NO_ERROR);
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_width, r_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_width, r_height, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	//printf("renderer_BlitColorbuffer: %x\n", glGetError());
}

void renderer_BlitBackbuffer()
{
	renderer_BindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);
    glClear(GL_DEPTH_BUFFER_BIT);


    //renderer_BindFramebuffer(GL_READ_FRAMEBUFFER, &r_bbuffer);


    //glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_width, r_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);


	//glViewport(0, 0, r_window_width, r_window_height);

	/*renderer_SetProjectionMatrix(NULL);
	renderer_SetViewMatrix(NULL);
	renderer_SetModelMatrix(NULL);*/


	//while(glGetError() != GL_NO_ERROR);

//	glDisable(GL_DEPTH_TEST);

	renderer_EnableImediateDrawing();
	renderer_SetShader(r_blit_texture_shader);
	renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_bbuffer.color_attachments[0].handle);
	renderer_SetDefaultUniform1i(UNIFORM_texture_sampler0, 0);
	renderer_Rectf(-1.0, -1.0, 1.0, 1.0);
	renderer_DisableImediateDrawing();

//	glEnable(GL_DEPTH_TEST);

	//glDisable(GL_BLEND);

	//glRectf(-1.0, -1.0, 1.0, 1.0);

	//renderer_SetProjectionMatrix(NULL);
	//renderer_SetViewMatrix(NULL);
	//renderer_SetModelMatrix(NULL);
	//renderer
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, r_bbuffer_id);
	//glReadBuffer(GL_COLOR_ATTACHMENT0);

	//while(glGetError() != GL_NO_ERROR);
	//glBlitFramebuffer(0, 0, r_width - 1, r_height - 1, 0, 0, r_window_width - 1, r_window_height - 1, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	//printf("renderer_BlitBackbuffer: %x\n", glGetError());

	//renderer_StopTimer(cpu_timer);
	//renderer_StopTimer(gpu_timer);
}


void renderer_DrawPlayers()
{
/*
	int i;
	int c = player_count;

	camera_t *active_camera = camera_GetActiveCamera();

	glUseProgram(0);
	glLoadMatrixf(&active_camera->view_data.view_matrix.floats[0][0]);
	glPointSize(8.0);
	glColor3f(1.0, 1.0, 1.0);

	glBegin(GL_POINTS);

	for(i = 0; i < c; i++)
	{
		if(&players[i] == active_player)
			continue;

		if(!(players[i].bm_flags & PLAYER_IN_WORLD))
			continue;

		if(players[i].bm_flags & PLAYER_INVALID)
			continue;

		glVertex3f(players[i].player_position.x, players[i].player_position.y, players[i].player_position.z);
	}


	glEnd();
	glPointSize(1.0);
	*/





/*	int i;
	int c = player_count;
	camera_t *active_camera = camera_GetActiveCamera();
	mat4_t transform;
	mat4_t temp_transform;
	mat4_t weapon_transform;
	mat4_t model_view_matrix;
	float body_color[] = {1.0, 0.0, 0.0, 1.0};
	float weapon_color[] = {0.0, 1.0, 0.0, 1.0};

	mat3_t weapon_rot = mat3_t_id();
	vec3_t weapon_position = vec3(1.0, -1.0, -2.0);

	glEnable(GL_STENCIL_TEST);
	glDepthMask(GL_FALSE);

	glStencilFunc(GL_NOTEQUAL, 1, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	shader_UseShader(forward_pass_shader);
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_FLAGS, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_LIGHT_COUNT, visible_light_count);

	for(i = 0; i < c; i++)
	{
		if(&players[i] != active_player)
		{

			glLoadMatrixf(&visible_players_body_transforms[i].floats[0][0]);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, body_color);
			glDrawArrays(GL_TRIANGLES, players[i].body_start, players[i].body_count);

			glLoadMatrixf(&visible_players_weapon_transforms[i].floats[0][0]);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, weapon_color);
			glDrawArrays(GL_TRIANGLES, players[i].weapon_start, players[i].weapon_count);
		}
	}

	glDisable(GL_STENCIL_TEST);
	glDepthMask(GL_TRUE);*/


}

void renderer_DrawActivePlayer()
{
	/*int i;
	int c = player_count;
	camera_t *active_camera = camera_GetActiveCamera();
	float color[] = {0.0, 0.0, 1.0, 1.0};

	mat4_t transform;
	mat4_t camera_transform;
	mat4_t weapon_transform;
	mat4_t model_view_matrix;
	mat3_t weapon_rot = mat3_t_id();
	vec3_t weapon_position;


	if(!active_player) return;*/

	/*glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	shader_UseShader(forward_pass_shader);
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_FLAGS, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_LIGHT_COUNT, visible_light_count);


	mat4_t_compose(&camera_transform, &active_camera->world_orientation, active_camera->world_position);


	weapon_position = vec3(1.0, -1.0, -1.0);
	weapon_position.z += active_player->weapon_z_shift;
	weapon_position.x += active_player->weapon_x_shift;
	weapon_position.y -= active_player->weapon_y_shift;

	mat4_t_compose(&transform, &weapon_rot, weapon_position);
	mat4_t_scale(&transform, vec3(0.0, 0.0, 1.0), 0.5);

	glDepthMask(GL_FALSE);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	mat4_t_mult_fast(&weapon_transform, &transform, &camera_transform);
	mat4_t_mult_fast(&model_view_matrix, &weapon_transform, &active_camera->world_to_camera_matrix);
	glLoadMatrixf(&model_view_matrix.floats[0][0]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
	glDrawArrays(GL_TRIANGLES, active_player->weapon_start, active_player->weapon_count);

	glDisable(GL_STENCIL_TEST);
	glDepthMask(GL_TRUE);*/

}


/*
===============
renderer_DrawOpaque

draw anything that isn't
the world, and also isn't
translucent...

===============
*/






/*
==============
renderer_DrawWorld

draws only opaque world geometry...

==============
*/



void renderer_DrawPortals()
{
	#if 0
	int i;
	unsigned short *view_portals;
	portal_t *portal;

	vec3_t right_vector;
	vec3_t up_vector;
	vec3_t center;

	//double eq[] = {0.0, 0.0, -1.0, -10.0};

	mat4_t r = mat4_t_id();

	view_portals = r_main_view->view_data.view_portals;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_pbuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//glClearColor(0.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_STENCIL_TEST);
	//glCullFace(GL_BACK);
	//r = r_main_view->world_to_camera_matrix;

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	//glEnable(GL_CLIP_PLANE0);

	//glClipPlane(GL_CLIP_PLANE0, eq);

	glEnable(GL_CLIP_DISTANCE0);

	for(i = 0; i < r_main_view->view_data.view_portals_list_cursor; i++)
	{
		//renderer_RecursiveDrawPortals(&ptl_portals[r_main_view->view_data.view_portals[i]], -1);
		//renderer_DrawPortal(&ptl_portals[r_main_view->view_data.view_portals[i]], -1);
	}

	glDisable(GL_CLIP_DISTANCE0);

	//renderer_RecursiveDrawPortalsOnView(r_main_view, &r);

	//glDisable(GL_CLIP_PLANE0);
	//glCullFace(GL_FRONT);

	glDisable(GL_STENCIL_TEST);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_cbuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	renderer_SetActiveView(r_main_view);
	renderer_SetShader(r_portal_shader);
	renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_pbuffer_color);
	renderer_EnableImediateDrawing();
	renderer_SetModelMatrix(NULL);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_CLAMP);
	for(i = 0; i < r_main_view->view_data.view_portals_list_cursor; i++)
	{
		portal = &ptl_portals[view_portals[i]];

		/*if(!portal->view)
		{
			continue;
		}

		if(portal->view->bm_flags & CAMERA_INACTIVE)
		{
			continue;
		}*/

		right_vector.x = portal->orientation.floats[0][0] * portal->extents.x;
		right_vector.y = portal->orientation.floats[1][0] * portal->extents.x;
		right_vector.z = portal->orientation.floats[2][0] * portal->extents.x;

		up_vector.x = portal->orientation.floats[0][1] * portal->extents.y;
		up_vector.y = portal->orientation.floats[1][1] * portal->extents.y;
		up_vector.z = portal->orientation.floats[2][1] * portal->extents.y;

		center = portal->position;

		renderer_Begin(GL_QUADS);
		renderer_Vertex3f(center.x - right_vector.x + up_vector.x, center.y - right_vector.y + up_vector.y, center.z - right_vector.z + up_vector.z);
		renderer_Vertex3f(center.x - right_vector.x - up_vector.x, center.y - right_vector.y - up_vector.y, center.z - right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x - up_vector.x, center.y + right_vector.y - up_vector.y, center.z + right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x + up_vector.x, center.y + right_vector.y + up_vector.y, center.z + right_vector.z + up_vector.z);
		renderer_End();
	}
	glDisable(GL_DEPTH_CLAMP);

	renderer_DisableImediateDrawing();

	#endif
}

void renderer_RecursiveDrawPortals(portal_t *portal, int viewing_portal_index)
{
	#if 0
	int i;
	int portal_index;
	portal_view_data_t *view_data = NULL;
	portal_recursive_view_data_t *recursive_view_data = NULL;

	mat4_t current_view_matrix;
	vec3_t right_vector;
	vec3_t up_vector;
	vec3_t center;

	static int r_recursion_level = -1;
	r_recursion_level++;

	if(r_recursion_level <= r_max_portal_recursion_level)
	{
		portal_index = portal - ptl_portals;
		recursive_view_data = &portal->portal_recursive_views[r_recursion_level];

		if(recursive_view_data->frame != r_frame)
		{
			/* this data isn't relative to this frame... */
			r_recursion_level--;
			return;
		}

		/* find the view data that corresponds to the view we're viewing this
		portal from... */
		for(i = 0; i < recursive_view_data->views_count; i++)
		{
			if(recursive_view_data->views[i].ref_portal_index == viewing_portal_index)
			{
				view_data = &recursive_view_data->views[i];
				break;
			}
		}

		if(!view_data)
		{
			r_recursion_level--;
			return;
		}

		right_vector.x = portal->orientation.floats[0][0] * portal->extents.x;
		right_vector.y = portal->orientation.floats[1][0] * portal->extents.x;
		right_vector.z = portal->orientation.floats[2][0] * portal->extents.x;

		up_vector.x = portal->orientation.floats[0][1] * portal->extents.y;
		up_vector.y = portal->orientation.floats[1][1] * portal->extents.y;
		up_vector.z = portal->orientation.floats[2][1] * portal->extents.y;

		center = portal->position;

		/* mark this portal in the stencil buffer... */
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		//glStencilFunc(GL_ALWAYS, r_recursion_level + 1, 0xff);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_CLAMP);
		/* don't write depth, so stuff can be
		drawn inside the portal... */
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		renderer_EnableImediateDrawing();
		renderer_Begin(GL_QUADS);
		renderer_Vertex3f(center.x - right_vector.x + up_vector.x, center.y - right_vector.y + up_vector.y, center.z - right_vector.z + up_vector.z);
		renderer_Vertex3f(center.x - right_vector.x - up_vector.x, center.y - right_vector.y - up_vector.y, center.z - right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x - up_vector.x, center.y + right_vector.y - up_vector.y, center.z + right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x + up_vector.x, center.y + right_vector.y + up_vector.y, center.z + right_vector.z + up_vector.z);
		renderer_End();
		renderer_DisableImediateDrawing();
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);

		//renderer_DrawOpaque();

		/* recurse down if this portal sees a portal... */
		for(i = 0; i < view_data->view_data.view_portals_list_cursor; i++)
		{
			renderer_RecursiveDrawPortals(&ptl_portals[view_data->view_data.view_portals[i]], portal_index);
		}

		/* draw it's stuff after we're done with deeper levels of recursions,
		to save some fill rate... */
		//renderer_SetShader(r_portal)
		glDisable(GL_DEPTH_CLAMP);
		renderer_SetShader(r_forward_pass_portal_shader);
		renderer_SetViewData(&view_data->view_data);
		renderer_SetCurrentVertexFormat(VERTEX_FORMAT_V3F_N4IREV_T4IREV_TC2F);

		renderer_SetDefaultUniform1i(UNIFORM_r_width, r_width);
		renderer_SetDefaultUniform1i(UNIFORM_r_height, r_height);
		renderer_SetNamedUniform4fv("clip_plane", (float *)&view_data->near_plane);
		renderer_SetClusterTexture();

		renderer_ExecuteDrawCommands();



		/* clear this portal from the stencil... */
		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
		//glStencilFunc(GL_GEQUAL, r_recursion_level, 0xff);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_CLAMP);

		/* don't write depth, since doing so
		would overwrite drawn pixels inside
		the portal with the portal polygon's... */
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		renderer_EnableImediateDrawing();
		renderer_Begin(GL_QUADS);
		renderer_Vertex3f(center.x - right_vector.x + up_vector.x, center.y - right_vector.y + up_vector.y, center.z - right_vector.z + up_vector.z);
		renderer_Vertex3f(center.x - right_vector.x - up_vector.x, center.y - right_vector.y - up_vector.y, center.z - right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x - up_vector.x, center.y + right_vector.y - up_vector.y, center.z + right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x + up_vector.x, center.y + right_vector.y + up_vector.y, center.z + right_vector.z + up_vector.z);
		renderer_End();
		renderer_DisableImediateDrawing();
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
	}

	r_recursion_level--;

	#endif
}



void renderer_DrawPortal(portal_t *portal, int viewing_portal_index)
{
	#if 0
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_CLAMP);
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	renderer_EnableImediateDrawing();

	renderer_RecursiveDrawPortalsStencil(portal, viewing_portal_index);

	renderer_DisableImediateDrawing();
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_CLAMP);

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	renderer_SetShader(r_forward_pass_portal_shader);
	renderer_SetCurrentVertexFormat(VERTEX_FORMAT_V3F_N4IREV_T4IREV_TC2F);
	renderer_SetDefaultUniform1i(UNIFORM_r_width, r_width);
	renderer_SetDefaultUniform1i(UNIFORM_r_height, r_height);

	renderer_RecursiveDrawPortalsViews(portal, viewing_portal_index);

	glClear(GL_STENCIL_BUFFER_BIT);

	#endif
}



static int r_portal_view_recursion_level = -1;

void renderer_RecursiveDrawPortalsStencil(portal_t *portal, int viewing_portal_index)
{
	#if 0
	int i;
	int portal_index;
	portal_view_data_t *view_data = NULL;
	portal_recursive_view_data_t *recursive_view_data = NULL;

	mat4_t current_view_matrix;
	vec3_t right_vector;
	vec3_t up_vector;
	vec3_t center;

	static int r_recursion_level = -1;
	r_recursion_level++;

	if(r_recursion_level <= r_max_portal_recursion_level)
	{
		portal_index = portal - ptl_portals;
		recursive_view_data = &portal->portal_recursive_views[r_recursion_level];

		if(recursive_view_data->frame != r_frame)
		{
			/* this data isn't relative to this frame... */
			r_recursion_level--;
			return;
		}

		/* find the view data that corresponds to the view we're viewing this
		portal from... */
		for(i = 0; i < recursive_view_data->views_count; i++)
		{
			if(recursive_view_data->views[i].ref_portal_index == viewing_portal_index)
			{
				view_data = &recursive_view_data->views[i];
				break;
			}
		}

		if(!view_data)
		{
			r_recursion_level--;
			return;
		}

		right_vector.x = portal->orientation.floats[0][0] * portal->extents.x;
		right_vector.y = portal->orientation.floats[1][0] * portal->extents.x;
		right_vector.z = portal->orientation.floats[2][0] * portal->extents.x;

		up_vector.x = portal->orientation.floats[0][1] * portal->extents.y;
		up_vector.y = portal->orientation.floats[1][1] * portal->extents.y;
		up_vector.z = portal->orientation.floats[2][1] * portal->extents.y;

		center = portal->position;
		glStencilFunc(GL_ALWAYS, r_recursion_level + 1, 0xff);
		renderer_Begin(GL_QUADS);
		renderer_Vertex3f(center.x - right_vector.x + up_vector.x, center.y - right_vector.y + up_vector.y, center.z - right_vector.z + up_vector.z);
		renderer_Vertex3f(center.x - right_vector.x - up_vector.x, center.y - right_vector.y - up_vector.y, center.z - right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x - up_vector.x, center.y + right_vector.y - up_vector.y, center.z + right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x + up_vector.x, center.y + right_vector.y + up_vector.y, center.z + right_vector.z + up_vector.z);
		renderer_End();

		/* recurse down if this portal sees a portal... */
		for(i = 0; i < view_data->view_data.view_portals_list_cursor; i++)
		{
			renderer_RecursiveDrawPortalsStencil(&ptl_portals[view_data->view_data.view_portals[i]], portal_index);
		}
	}

	r_recursion_level--;

	#endif
}

void renderer_RecursiveDrawPortalsViews(portal_t *portal, int viewing_portal_index)
{
	#if 0

	int i;
	int portal_index;
	portal_view_data_t *view_data = NULL;
	portal_recursive_view_data_t *recursive_view_data = NULL;

	mat4_t current_view_matrix;
	vec3_t right_vector;
	vec3_t up_vector;
	vec3_t center;

	static int r_recursion_level = -1;
	r_recursion_level++;

	if(r_recursion_level <= r_max_portal_recursion_level)
	{
		portal_index = portal - ptl_portals;
		recursive_view_data = &portal->portal_recursive_views[r_recursion_level];

		if(recursive_view_data->frame != r_frame)
		{
			/* this data isn't relative to this frame... */
			r_recursion_level--;
			return;
		}

		/* find the view data that corresponds to the view we're viewing this
		portal from... */
		for(i = 0; i < recursive_view_data->views_count; i++)
		{
			if(recursive_view_data->views[i].ref_portal_index == viewing_portal_index)
			{
				view_data = &recursive_view_data->views[i];
				break;
			}
		}

		if(!view_data)
		{
			r_recursion_level--;
			return;
		}
		//glDisable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, r_recursion_level + 1, 0xff);
		renderer_SetViewData(&view_data->view_data);
		renderer_SetNamedUniform4fv("clip_plane", (float *)&view_data->near_plane);
		renderer_SetClusterTexture();
		renderer_ExecuteDrawCommands();

	}

	r_recursion_level--;

	#endif
}





/*
==============================================================
==============================================================
==============================================================
*/

void renderer_Enable(int cap)
{
    if(cap > R_FIRST_CAP && cap < R_LAST_CAP)
    {
        r_caps[R_FIRST_CAP] = R_FIRST_CAP;

        switch(cap)
        {
            case R_FULLSCREEN:
                renderer_Fullscreen(1);
            break;

            case R_VERBOSE_DEBUG:
                renderer_VerboseDebugOutput(1);
            break;
        }
    }
}

void renderer_Disable(int cap)
{
    if(cap > R_FIRST_CAP && cap < R_LAST_CAP)
    {
        r_caps[R_FIRST_CAP] = 0;

        switch(cap)
        {
            case R_FULLSCREEN:
                renderer_Fullscreen(0);
            break;

            case R_VERBOSE_DEBUG:
                renderer_VerboseDebugOutput(0);
            break;
        }
    }
}

int renderer_IsEnabled(int cap)
{
    if(cap > R_FIRST_CAP && cap < R_LAST_CAP)
    {
        return r_caps[cap];
    }
}






#ifdef __cplusplus
}
#endif







