#include <stdio.h>
#include <string.h>

#include "r_common.h"
#include "r_shader.h"
#include "r_main.h"
#include "r_debug.h"
#include "r_imediate.h"
#include "r_gl.h"
#include "r_text.h"

#include "stack_list.h"

//#include "..\editor\level editor\ed_level_draw.h"

#include "camera.h"
#include "player.h"
#include "shader.h"
//#include "physics.h"
#include "model.h"
#include "world.h"
#include "material.h"
#include "entity.h"
#include "texture.h"
#include "l_main.h"
#include "l_cache.h"
//#include "brush.h"
//#include "editor.h"
#include "gui.h"
#include "bsp.h"
#include "log.h"
#include "memory.h"
#include "portal.h"
#include "particle.h"

#include "engine.h"

#include "SDL2\SDL.h"
#include "GL\glew.h"



SDL_Window *window;
SDL_GLContext context;




//int r_active_shader = -1;

unsigned int r_gbuffer_id = 0;
unsigned int r_gbuffer_color = 0;
unsigned int r_gbuffer_normal = 0;
unsigned int r_gbuffer_depth = 0;


struct framebuffer_t r_cbuffer;
struct framebuffer_t r_bbuffer;

unsigned int r_pbuffer_id = 0;
unsigned int r_pbuffer_color = 0;
unsigned int r_pbuffer_depth = 0;



int r_z_pre_pass_shader;
int r_forward_pass_shader;
int r_particle_forward_pass_shader;
int r_flat_pass_shader;
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
extern int w_world_index_handle;
extern int *w_index_buffer;
extern int w_world_batch_count;
extern batch_t *w_world_batches;
extern int w_world_start;
extern int w_world_index_start;
extern int w_world_vertices_count;
extern int w_world_leaves_count;
extern bsp_dleaf_t *w_world_leaves;
extern int w_visible_lights_count;
extern unsigned short w_visible_lights[];



/* from particle.c */
//extern int ps_particle_system_count;
//extern particle_system_t *ps_particle_systems;
extern struct stack_list_t ps_particle_systems;
extern int ps_particle_quad_start;



/* from material.c */
extern int mat_material_count;
extern material_t *mat_materials;

/* from texture.c */
extern int tex_texture_count;
extern texture_t *tex_textures;

/* from shader.c */
extern shader_t *shaders;




/* from gui.c */
extern widget_t *widgets;
extern widget_t *last_widget;
extern widget_t *top_widget;
extern mat4_t gui_projection_matrix;


#include "l_globals.h"




/* from player.c */
extern player_count;
extern player_t *players;
extern player_t *active_player;


extern int ptl_portal_list_cursor;
extern portal_t *ptl_portals;



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

unsigned int r_frame = 0;

unsigned int query_object;





int r_draw_command_group_count = 0;
int r_draw_cmds_count = 0;
int r_max_draw_cmds_count = 0;
draw_command_group_t *r_draw_command_groups = NULL;
static draw_command_t *r_sorted_draw_cmds = NULL;
static draw_command_t *r_unsorted_draw_cmds = NULL;


/*int r_translucent_draw_group_count = 0;
draw_group_t *r_translucent_draw_groups = NULL;
static draw_command_t *r_translucent_draw_cmds = NULL;*/





unsigned int r_bloom_fbs[3];
unsigned int r_bloom_texs[2][3];
int r_bloom_radius[3] = {2.0, 4.0, 8.0};




unsigned int r_intensity_id = 0;
unsigned int r_intensity_color = 0;

unsigned int r_intensity_half_id = 0;
unsigned int r_intensity_half_horizontal_color = 0;
unsigned int r_intensity_half_vertical_color = 0;

unsigned int r_intensity_quarter_id = 0;
unsigned int r_intensity_quarter_horizontal_color = 0;
unsigned int r_intensity_quarter_vertical_color = 0;

unsigned int r_intensity_eight_id = 0;
unsigned int r_intensity_eight_horizontal_color = 0;
unsigned int r_intensity_eight_vertical_color = 0;


int r_width = 0;
int r_height = 0;
int r_window_width = 0;
int r_window_height = 0;
int r_window_flags = 0;

int r_msaa_samples = 1;
int r_msaa_supported = 1;

int r_draw_shadow_maps = 1;
int r_z_prepass = 1;
int r_query_stages = 0;
int r_bloom = 0;
int r_tonemap = 1;
int r_draw_gui = 1;

int r_deferred = 0;

int r_flat = 0;

int r_debug = 1;
int r_debug_verbose = 0;
int r_debug_draw_portal_outlines = 1;
int r_debug_draw_views = 1;
int r_debug_draw_waypoints = 1;
int r_debug_draw_colliders = 1;
int r_debug_draw_entities = 1;

int r_max_portal_recursion_level = 3;

mat4_t r_projection_matrix;
int r_projection_matrix_changed = 1;

mat4_t r_view_matrix;
int r_view_matrix_changed = 1;

mat4_t r_model_matrix;
mat4_t r_view_projection_matrix;
mat4_t r_model_view_matrix;
mat4_t r_model_view_projection_matrix;

camera_t *r_active_view = NULL;
camera_t *r_main_view = NULL;

struct gpu_light_t *r_light_buffer = NULL;

extern camera_t *active_camera;
extern camera_t *main_view;

int r_draw_calls = 0;
int r_material_swaps = 0;
int r_shader_swaps = 0;
int r_shader_uniform_updates = 0;
unsigned long r_frame_vert_count = 0;


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
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG | SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 1);
	
	window = SDL_CreateWindow("wtf editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, r_width, r_height, r_window_flags);
	context = SDL_GL_CreateContext(window);
	
	
	
	SDL_GL_MakeCurrent(window, context);

	SDL_GL_SetSwapInterval(0);
	
	if(glewInit() != GLEW_NO_ERROR)
	{
		log_LogMessage(LOG_MESSAGE_ERROR, "renderer_Init: glew didn't init!");
		return 0;
		//printf("oh shit...\n");
		//exit(2);
	}
	
	log_LogMessage(LOG_MESSAGE_NONE, "Window resolution: %d x %d", r_width, r_height);
	
	
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearStencil(0);
	
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_MULTISAMPLE);
	glDepthFunc(GL_LEQUAL);
	glStencilMask(0xff);
	
	

	log_LogMessage(LOG_MESSAGE_NONE, "GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	log_LogMessage(LOG_MESSAGE_NONE, "OpenGL version: %s", glGetString(GL_VERSION));
	log_LogMessage(LOG_MESSAGE_NONE, "Vendor: %s", glGetString(GL_VENDOR));
	log_LogMessage(LOG_MESSAGE_NONE, "Renderer: %s", glGetString(GL_RENDERER));
	
	
	
	r_cbuffer.framebuffer_id = 0;
	r_bbuffer.framebuffer_id = 0;

	renderer_SetRendererResolution(r_width, r_height, 1);
	
	
	
	//r_draw_groups = malloc(sizeof(draw_group_t) * MAX_MATERIALS);
	//r_draw_cmds = malloc(sizeof(draw_command_t) * MAX_MATERIALS * MAX_ENTITIES);
	
	r_draw_command_groups = memory_Malloc(sizeof(draw_command_group_t) * (MAX_MATERIALS + 1), "renderer_Init");
	
/*	for(i = 0; i <= MAX_MATERIALS; i++)
	{
		r_draw_command_groups[i].max_draw_cmds = 8192;
		r_draw_command_groups[i].draw_cmds_count = 0;
		r_draw_command_groups[i].
	}*/
	
	r_max_draw_cmds_count = 32768;
	r_unsorted_draw_cmds = memory_Malloc(sizeof(draw_command_t) * r_max_draw_cmds_count, "renderer_Init");
	r_sorted_draw_cmds = memory_Malloc(sizeof(draw_command_t) * r_max_draw_cmds_count, "renderer_Init");
	
	r_light_buffer = memory_Malloc(sizeof(struct gpu_light_t) * MAX_VISIBLE_LIGHTS, "renderer_Init");
	
	//assert(r_draw_command_groups);
	//assert(r_draw_cmds);
		
	//r_translucent_draw_groups = malloc(sizeof(draw_group_t) * MAX_MATERIALS);
	//r_translucent_draw_cmds = malloc(sizeof(draw_command_t) * MAX_MATERIALS * MAX_ENTITIES);
	
	//r_translucent_draw_groups = memory_Malloc(sizeof(draw_group_t) * MAX_MATERIALS, "renderer_Init");
	//r_translucent_draw_cmds = memory_Malloc(sizeof(draw_command_t) * MAX_MATERIALS * MAX_ENTITIES, "renderer_Init");
		
	glGenQueries(1, &query_object);
	
	renderer_InitImediateDrawing();
	renderer_InitDebug();
	renderer_VerboseDebugOutput(1);
	
	//renderer_Debug(1, 1);

	return 1;
}

void renderer_Finish()
{	

	/*free(r_draw_cmds);
	free(r_draw_groups);
	
	free(r_translucent_draw_cmds);
	free(r_translucent_draw_groups);*/
	
	memory_Free(r_sorted_draw_cmds);
	memory_Free(r_unsorted_draw_cmds);
	memory_Free(r_draw_command_groups);
	memory_Free(r_light_buffer);
	
	renderer_FinishDebug();
	renderer_FinishImediateDrawing();
	//memory_Free(r_translucent_draw_cmds);
	//memory_Free(r_translucent_draw_groups);

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}




void renderer_SetDiffuseTexture(int texture_index)
{
	unsigned int gl_handle;
	
	if(texture_index < 0)
	{
		gl_handle = 0;
	}
	else
	{
		gl_handle = tex_textures[texture_index].gl_handle;
	}
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gl_handle);
	
	renderer_SetDefaultUniform1i(UNIFORM_texture_sampler0, 0);
	//renderer_SetUniform1i(UNIFORM_texture_sampler0, 0);
}

void renderer_SetNormalTexture(int texture_index)
{
	unsigned int gl_handle;
	
	if(texture_index < 0)
	{
		gl_handle = 0;
	}
	else
	{
		gl_handle = tex_textures[texture_index].gl_handle;
	}
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gl_handle);
	
	renderer_SetDefaultUniform1i(UNIFORM_texture_sampler1, 1);
}

void renderer_SetShadowTexture()
{
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, l_shared_shadow_map);
	
	renderer_SetDefaultUniform1i(UNIFORM_texture_sampler2, 2);
}

void renderer_SetClusterTexture()
{
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, l_cluster_texture);
	renderer_SetDefaultUniform1i(UNIFORM_cluster_texture, 3);
}

/*
======================
renderer_SetTexture

sets a texture by it's ENGINE HANDLE...

======================
*/
void renderer_SetTexture(int texture_unit, int texture_target, int texture_index)
{
	int gl_texture;
	
	if(texture_index < 0 || texture_index >= tex_texture_count)
	{
		texture_index = -1;
	}
	 
	if(tex_textures[texture_index].bm_flags & TEXTURE_INVALID)
	{
		texture_index = -1;
	}
	
	gl_texture = tex_textures[texture_index].gl_handle;
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


int renderer_BindTexture(int texture_target, int texture)
{
	/*glActiveTexture(texture_unit);
	glBindTexture(texture_target, texture);*/
}

void renderer_SetClearColor(float r, float g, float b)
{
	r_clear_color.r = r;
	r_clear_color.g = g;
	r_clear_color.b = b;
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
	renderer_SetDefaultUniformMatrix4fv(UNIFORM_projection_matrix, &r_projection_matrix.floats[0][0]);
	
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
	texture_t *texture;
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
			renderer_SetDiffuseTexture(material->diffuse_texture);
			texture_flags |= MATERIAL_USE_DIFFUSE_TEXTURE;
		}
		
		if(material->normal_texture != -1)
		{
			renderer_SetNormalTexture(material->normal_texture);
			texture_flags |= MATERIAL_USE_NORMAL_TEXTURE;
			
			texture = &tex_textures[material->normal_texture];
			
			
			if(texture->bm_flags & TEXTURE_INVERT_X)
			{
				texture_flags |= MATERIAL_INVERT_NORMAL_X;
			}
			
			if(texture->bm_flags & TEXTURE_INVERT_Y)
			{
				texture_flags |= MATERIAL_INVERT_NORMAL_Y;
			}
			
		}
		
		renderer_SetDefaultUniform1ui(UNIFORM_material_flags, texture_flags);
		
		r_material_swaps++;
	}
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



void renderer_SubmitDrawCommand(mat4_t *transform, unsigned short draw_mode, unsigned int start, unsigned int count, int material_index, int indexed_draw)
{
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
}

void renderer_SortDrawCommands()
{
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
		r_sorted_draw_cmds = memory_Malloc(sizeof(draw_command_t) * (r_max_draw_cmds_count + R_DRAW_COMMAND_LIST_RESIZE_INCREMENT), "renderer_SortDrawCommands");
		
		draw_commands = memory_Malloc(sizeof(draw_command_t) * (r_max_draw_cmds_count + R_DRAW_COMMAND_LIST_RESIZE_INCREMENT), "renderer_SortDrawCommands");
		
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
		renderer_AddAttachment(&r_cbuffer, GL_COLOR_ATTACHMENT0, GL_RGBA16F);
		renderer_AddAttachment(&r_cbuffer, GL_DEPTH_ATTACHMENT, 0);
	}
	else
	{
		renderer_ResizeFramebuffer(&r_cbuffer, r_width, r_height);
	}
	
	if(!r_bbuffer.framebuffer_id)
	{
		r_bbuffer = renderer_CreateFramebuffer(r_width, r_height);
		renderer_AddAttachment(&r_bbuffer, GL_COLOR_ATTACHMENT0, GL_RGBA8);
		renderer_AddAttachment(&r_bbuffer, GL_DEPTH_ATTACHMENT, 0);
	}
	else
	{
		renderer_ResizeFramebuffer(&r_bbuffer, r_width, r_height);
	}
	
	if(r_width > r_window_width || r_height > r_window_height)
	{
		renderer_SetWindowSize(r_width, r_height);
	}
}





void renderer_UpdateGeometryBuffer()
{
	
	/*int r_gbuffer_color_setup_status;
	int r_gbuffer_normal_setup_status;
	int r_gbuffer_depth_setup_status;
	
	
	int r_gbuffer_color_attach_status;
	int r_gbuffer_normal_attach_status;
	int r_gbuffer_depth_attach_status;
	int r_gbuffer_stencil_attach_status;
	
	if(!r_gbuffer_id)
	{
		glGenFramebuffers(1, &r_gbuffer_id);
		glGenTextures(1, &r_gbuffer_color);
		glGenTextures(1, &r_gbuffer_normal);
		glGenTextures(1, &r_gbuffer_depth);
	}
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_gbuffer_id);
	
	
	while(glGetError() != GL_NO_ERROR);
	glBindTexture(GL_TEXTURE_2D, r_gbuffer_color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, r_width, r_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	r_gbuffer_color_setup_status = glGetError();
	
	
	while(glGetError() != GL_NO_ERROR);
	glBindTexture(GL_TEXTURE_2D, r_gbuffer_normal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, r_width, r_height, 0, GL_RGBA, GL_FLOAT, NULL);
	r_gbuffer_normal_setup_status = glGetError();
	
	
	while(glGetError() != GL_NO_ERROR);
	glBindTexture(GL_TEXTURE_2D, r_gbuffer_depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, r_width, r_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	r_gbuffer_depth_setup_status = glGetError();
	
	
	while(glGetError() != GL_NO_ERROR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r_gbuffer_color, 0);
	r_gbuffer_color_attach_status = glGetError();
	
	while(glGetError() != GL_NO_ERROR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, r_gbuffer_normal, 0);
	r_gbuffer_normal_attach_status = glGetError();
	
	while(glGetError() != GL_NO_ERROR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, r_gbuffer_depth, 0);
	r_gbuffer_depth_attach_status = glGetError();
	
	while(glGetError() != GL_NO_ERROR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, r_gbuffer_depth, 0);
	r_gbuffer_stencil_attach_status = glGetError();
	
	
	if(r_gbuffer_color_setup_status != GL_NO_ERROR)
	{
		printf("renderer_GeometryBuffer: r_gbuffer_color setup failed: %x\n", r_gbuffer_color_setup_status);
	}
	
	if(r_gbuffer_normal_setup_status != GL_NO_ERROR)
	{
		printf("renderer_GeometryBuffer: r_gbuffer_normal setup failed: %x\n", r_gbuffer_normal_setup_status);
	}
	
	if(r_gbuffer_depth_setup_status != GL_NO_ERROR)
	{
		printf("renderer_GeometryBuffer: r_gbuffer_depth setup failed: %x\n", r_gbuffer_depth_setup_status);
	}
	
	
	
	
	if(r_gbuffer_color_attach_status != GL_NO_ERROR)
	{
		printf("renderer_GeometryBuffer: r_gbuffer_color attachment to framebuffer failed: %x\n", r_gbuffer_color_attach_status);
	}
	
	if(r_gbuffer_normal_attach_status != GL_NO_ERROR)
	{
		printf("renderer_GeometryBuffer: r_gbuffer_normal attachment to framebuffer failed: %x\n", r_gbuffer_normal_attach_status);
	}
	
	if(r_gbuffer_depth_attach_status != GL_NO_ERROR)
	{
		printf("renderer_GeometryBuffer: r_gbuffer_depth depth component attachment to framebuffer failed: %x\n", r_gbuffer_depth_attach_status);
	}
	
	if(r_gbuffer_stencil_attach_status != GL_NO_ERROR)
	{
		printf("renderer_GeometryBuffer: r_gbuffer_depth stencil component attachment to framebuffer failed: %x\n", r_gbuffer_stencil_attach_status);
	}	
	
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);*/
}




void renderer_UpdateColorbuffer()
{
	int target;
	int error;	
	
	int i;
	int j;
	
	if(!r_cbuffer.framebuffer_id)
	{
		//r_cbuffer = renderer_CreateFramebuffer(r_width, r_height);
		//renderer_AddAttachment(&r_cbuffer, GL_COLOR_ATTACHMENT0, GL_RGBA16F);
		//renderer_AddAttachment(&r_cbuffer, GL_DEPTH_ATTACHMENT, 0);
		
		//glGenFramebuffers(1, &r_cbuffer_id);
		//glGenFramebuffers(1, &r_bbuffer_id);
		
		/*glGenFramebuffers(1, &r_intensity_half_id);
		glGenFramebuffers(1, &r_intensity_quarter_id);
		glGenFramebuffers(1, &r_intensity_eight_id);*/
		
		/*glGenFramebuffers(1, &r_intensity_id);
		
		for(i = 0; i < 3; i++)
		{
			glGenFramebuffers(1, &r_bloom_fbs[i]);
		} 
		
		glGenTextures(1, &r_intensity_color);
		glBindTexture(GL_TEXTURE_2D, r_intensity_color);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		
		for(j = 0; j < 2; j++)
		{
			for(i = 0; i < 3; i++)
			{
				glGenTextures(1, &r_bloom_texs[j][i]);
				glBindTexture(GL_TEXTURE_2D, r_bloom_texs[j][i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			}
		}*/
		
		
		
		
		
	/*	glGenTextures(1, &r_cbuffer_color);
		glBindTexture(GL_TEXTURE_2D, r_cbuffer_color);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		
		
		glGenTextures(1, &r_cbuffer_depth);
		glBindTexture(GL_TEXTURE_2D, r_cbuffer_depth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);*/
		
		
		
		
		
		
		//glGenTextures(1, &r_bbuffer_color);
		//glBindTexture(GL_TEXTURE_2D, r_bbuffer_color);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		
		//glGenTextures(1, &r_bbuffer_depth);
		//glBindTexture(GL_TEXTURE_2D, r_bbuffer_depth);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		
		
		
		/*glGenTextures(1, &r_intensity_half_horizontal_color);
		glGenTextures(1, &r_intensity_half_vertical_color);
		glGenTextures(1, &r_intensity_quarter_horizontal_color);
		glGenTextures(1, &r_intensity_quarter_vertical_color);
		glGenTextures(1, &r_intensity_eight_horizontal_color);
		glGenTextures(1, &r_intensity_eight_vertical_color);*/
	}
	else
	{
	//	renderer_ResizeFramebuffer(&r_cbuffer, r_width, r_height);
	//	renderer_ResizeFra
	}

	
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_cbuffer_id);
	//glBindTexture(GL_TEXTURE_2D, r_cbuffer_color);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, r_width, r_height, 0, GL_RGBA, GL_FLOAT, NULL);
	//glBindTexture(GL_TEXTURE_2D, r_cbuffer_depth);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, r_width, r_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r_cbuffer_color, 0);
	//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, r_cbuffer_depth, 0);
	//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, r_cbuffer_depth, 0);
	
	//if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	//{
	//	printf("renderer_UpdateColorbuffer: r_cbuffer not complete!\n");
	//}

	
//	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_bbuffer_id);	
//	glBindTexture(GL_TEXTURE_2D, r_bbuffer_color);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, r_width, r_height, 0, GL_RGBA, GL_FLOAT, NULL);
//	glBindTexture(GL_TEXTURE_2D, r_bbuffer_depth);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, r_width, r_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
//	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r_bbuffer_color, 0);
//	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, r_bbuffer_depth, 0);
//	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, r_bbuffer_depth, 0);

//	if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//	{
//		printf("renderer_UpdateColorbuffer: r_bbuffer not complete!\n");
//	}
	
	
	
/*	glBindTexture(GL_TEXTURE_2D, r_intensity_color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, r_width, r_height, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_intensity_id);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r_intensity_color, 0);
	
	for(i = 0; i < 3; i++)
	{
		for(j = 0; j < 2; j++)
		{
			glBindTexture(GL_TEXTURE_2D, r_bloom_texs[j][i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, r_width >> (1 + i), r_height >> (1 + i), 0, GL_RGBA, GL_FLOAT, NULL);	
		}
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_bloom_fbs[i]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r_bloom_texs[0][i], 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, r_bloom_texs[1][i], 0);
		
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);*/
}



void renderer_UpdatePortalbuffer()
{
	int target;
	int error;	
	if(!r_pbuffer_id)
	{
		glGenFramebuffers(1, &r_pbuffer_id);
		
		glGenTextures(1, &r_pbuffer_color);
		glBindTexture(GL_TEXTURE_2D, r_pbuffer_color);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		
		glGenTextures(1, &r_pbuffer_depth);
		glBindTexture(GL_TEXTURE_2D, r_pbuffer_depth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	
	}
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_pbuffer_id);
	
	glBindTexture(GL_TEXTURE_2D, r_pbuffer_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, r_width, r_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, r_pbuffer_depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, r_width, r_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r_pbuffer_color, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, r_pbuffer_depth, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, r_pbuffer_depth, 0);
	
	if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("renderer_UpdateColorbuffer: r_pbuffer not complete!\n");
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}



void renderer_BindGeometryBuffer(int clear, int read)
{
	int color_attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	
	
	if(read)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, r_gbuffer_id);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}
	else
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_gbuffer_id);
		glDrawBuffers(2, color_attachments);
	}
		
	glViewport(0, 0, r_width, r_height);
	
	if(clear)
	{
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	
	
}




void renderer_BindColorbuffer(int clear, int read)
{
	/*glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_cbuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);*/
	
	if(read)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, r_cbuffer.framebuffer_id);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}
	else
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_cbuffer.framebuffer_id);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
	}
	
	/*if(r_msaa_samples > 1)
	{
		glEnable(GL_MULTISAMPLE);
	}
	else
	{
		glDisable(GL_MULTISAMPLE);
	}*/
	//glDisable(GL_MULTISAMPLE);	
	glViewport(0, 0, r_width, r_height);
	
	if(clear)
	{
		glClearColor(r_clear_color.r, r_clear_color.g, r_clear_color.b, 0.0);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	
		
}

void renderer_BindBackbuffer(int clear, int read)
{
	
	if(read)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, r_bbuffer.framebuffer_id);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}
	else
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_bbuffer.framebuffer_id);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);	
	}
	
	glViewport(0, 0, r_width, r_height);
	
	
	if(clear)
	{
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
}

void renderer_BindWindowBuffer(int clear, int read)
{
	if(read)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glReadBuffer(GL_BACK);
	}
	else
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glDrawBuffer(GL_BACK);	
	}
	
	glViewport(0, 0, r_window_width, r_window_height);
	
	
	if(clear)
	{
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
}




void renderer_Fullscreen(int enable)
{
	int w;
	int h;
	SDL_DisplayMode display_mode;
	
	if(enable)
	{
		if(r_window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
			return;
		
		SDL_GetDisplayMode(0, 0, &display_mode);
		r_window_width = display_mode.w;
		r_window_height = display_mode.h;
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
	
	renderer_UpdateColorbuffer();
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
}

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

void renderer_SetActiveView(camera_t *view)
{
	r_active_view = view;
	active_camera = view;
	
	renderer_SetProjectionMatrix(&view->view_data.projection_matrix);
	renderer_SetViewMatrix(&view->view_data.view_matrix);
}

void renderer_SetMainView(camera_t *view)
{
	r_main_view = view;
	main_view = view;
}

void renderer_SetupView(camera_t *view)
{
	//renderer_SetActiveView(view);	
	//renderer_UpdateDrawCommandGroups(&view->view_data);
	//renderer_UpdateDrawCommandGroupsFromView(view);
	//renderer_SetViewVisibleWorld(view);
	//renderer_SetViewVisibleLights(view);
	
	//light_BindCache();
}

void renderer_SetViewData(view_data_t *view_data)
{
	//gpu_BindGpuHeap();
	//light_BindCache();
	
	//renderer_SetProjectionMatrix(&view_data->projection_matrix);
	//renderer_SetViewMatrix(&view_data->view_matrix);
	//renderer_SetViewDrawCommands(view_data);
	//renderer_SetViewLightData(view_data);
}


void renderer_SetViewLightData(view_data_t *view_data)
{
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

		lights[i].x_y = (parms->y << 16) | parms->x;
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
}

void renderer_SetViewDrawCommands(view_data_t *view_data)
{
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
		r_sorted_draw_cmds = memory_Malloc(sizeof(draw_command_t) * (r_max_draw_cmds_count + R_DRAW_COMMAND_LIST_RESIZE_INCREMENT), "renderer_SubmitDrawCommand");
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
}


void renderer_SetViewVisibleWorld(view_data_t *view_data)
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

void renderer_OpenFrame()
{
	//renderer_BindBackbuffer();
	//glClearColor(0.0, 0.0, 0.0, 1.0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//renderer_UpdateDrawGroups();
	
	r_draw_calls = 0;
	r_material_swaps = 0;
	r_shader_swaps = 0;
	r_shader_uniform_updates = 0;
	r_frame_vert_count = 0;
	r_draw_cmds_count = 0;
}

void renderer_DrawFrame()
{
	
	int i;
	
	float s;
	float e;
			 
	
	//light_BindCache();
	

	/*if(r_draw_shadow_maps)
	{
		renderer_DrawShadowMaps();
	}*/
	
	renderer_BindColorbuffer(1, 0);
	
	gpu_BindGpuHeap();
	light_BindCache();
		
	if(r_z_prepass)
	{
		renderer_ZPrePass();
	}
	
	renderer_DrawWorld();

	for(i = 0; i < pre_shading_render_function_count; i++)
	{
		renderer_PreShadingRegisteredFunction[i]();
	}
	
	
	renderer_SortDrawCommands();
	renderer_DrawOpaque();
	renderer_DrawTranslucent();
	renderer_DrawParticles();
	gpu_UnbindGpuHeap();
	light_UnbindCache();
	
	if(r_bloom && (!r_flat))
	{
		renderer_DrawBloom();	
	}
	
	if(r_tonemap && (!r_flat))
	{
		renderer_Tonemap();	
	}
	else
	{
		renderer_BlitColorbuffer();
	}
	
	
	//renderer_StartGpuTimer();
	for(i = 0; i < post_shading_render_function_count; i++)
	{
		renderer_PostShadingRegisteredFunction[i]();
	}
	//printf("renderer_PostShadingRegisteredFunctions: %f\n", renderer_StopGpuTimer());

	if(r_debug)
	{
		renderer_DrawDebug();
	}
	

	//renderer_StartGpuTimer();
	renderer_BlitBackbuffer();
	//printf("renderer_BlitBackbuffer: %f\n", renderer_StopGpuTimer());
	
	
	//renderer_StartGpuTimer();
	if(r_draw_gui)
	{
		renderer_DrawGUI();
		gui_DrawGUI();
	}
	
	//printf("renderer_DrawGUI: %f\n", renderer_StopGpuTimer());
	
		
	
	//renderer_DrawCachedString(0, 0, 0, 0, 0);
	
	/*renderer_StartGpuTimer();
	gpu_UnbindGpuHeap();
	light_UnbindCache();
	
	printf("unbind stuff: %f\n", renderer_StopGpuTimer());*/
}



void renderer_CloseFrame()
{
	/*#ifdef QUERY_STAGES
	renderer_BeginTimeElapsedQuery();
	#endif*/
	
	float s;
	float e;
	int err;
	
	/*while((err = glGetError()) != GL_NO_ERROR)
	{
		printf("%x\n", err);
	}*/
	//s = engine_GetDeltaTime();
	
//	printf("draw calls: %d	shader swaps: %d		material swaps: %d\n", r_draw_calls, r_shader_swaps, r_material_swaps);
	
	glDisable(GL_MULTISAMPLE);
	SDL_GL_SwapWindow(window);
	r_frame++;
	
	//printf("renderer_CloseFrame\n");
	
	//#ifdef QUERY_STAGES
	//renderer_EndTimeElapsedQuery(RENDERER_SWAP_BUFFERS_STAGE);
	//renderer_ReportQueryResults();
//	#endif
	
	
	//e = engine_GetDeltaTime();
	
	//printf("%f\n", e - s);
	
	
}


void renderer_ZPrePass()
{
	
	//return;
	
	
	int i;
	int c;
	camera_t *active_camera = camera_GetActiveCamera();	
	material_t *material;
	//mat4_t transform;
	//mat4_t temp_transform;
	//mat4_t camera_transform;
	//mat4_t weapon_transform;
	//mat4_t model_view_matrix;	
	//mat3_t weapon_rot = mat3_t_id();
	//vec3_t weapon_position;
	//triangle_group_t *triangle_group;
	batch_t *batch;
	
	float color[4];
	
	
	
	/*#ifdef QUERY_STAGES
	unsigned int elapsed;
	glBeginQuery(GL_TIME_ELAPSED, renderer_ZPrePassQueryObject);
	#endif*/
	
	/*#ifdef QUERY_STAGES
	renderer_BeginTimeElapsedQuery();
	#endif*/
	
	
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//glDrawBuffer(GL_BACK);
	
	//glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world_element_buffer);
	//shader_UseShader(z_pre_pass_shader);
	renderer_SetShader(r_z_pre_pass_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->position);
	
	renderer_SetProjectionMatrix(&active_camera->view_data.projection_matrix);
	renderer_SetViewMatrix(&active_camera->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);
	
	renderer_UpdateMatrices();
	
	
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	//glDepthMask(GL_TRUE);
	//glEnable(GL_STENCIL_TEST);
	glEnable(GL_CULL_FACE);
	//glStencilFunc(GL_ALWAYS, 0x1, 0xff);
	//glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	
	//glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	c = w_world_batch_count;
	
	for(i = 0; i < c; i++)
	{
		//triangle_group = &world_triangle_groups[i];
		batch = &w_world_batches[i];
		renderer_DrawElements(GL_TRIANGLES, batch->next, GL_UNSIGNED_INT, (void *)((batch->start + w_world_index_start) * sizeof(int)));
		//glDrawElements(GL_TRIANGLES, batch->next, GL_UNSIGNED_INT, (void *)((batch->start + w_world_index_start) * sizeof(int)));	
	}
	
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//glDisable(GL_STENCIL_TEST);
	
	//glDisable(GL_STENCIL_TEST);
	
	
	/*if(active_player)
	{
		mat4_t_compose(&camera_transform, &active_camera->world_orientation, active_camera->world_position);
		weapon_position = vec3(1.0, -1.0, -1.0);
		weapon_position.z += active_player->weapon_z_shift;
		weapon_position.x += active_player->weapon_x_shift;
		weapon_position.y -= active_player->weapon_y_shift;
				
		mat4_t_compose(&transform, &weapon_rot, weapon_position);
		mat4_t_scale(&transform, vec3(0.0, 0.0, 1.0), 0.5);
		
		mat4_t_mult_fast(&weapon_transform, &transform, &camera_transform);
		mat4_t_mult_fast(&model_view_matrix, &weapon_transform, &active_camera->world_to_camera_matrix);
		
		
		glStencilFunc(GL_ALWAYS, 1, 0xff);
		glStencilOp(GL_INCR, GL_INCR, GL_INCR);
		
		glLoadMatrixf(&model_view_matrix.floats[0][0]);
		glDrawArrays(GL_TRIANGLES, active_player->weapon_start, active_player->weapon_count);
	}*/
	
	/*c = player_count;
	
	glStencilFunc(GL_NOTEQUAL, 1, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	weapon_position = vec3(1.0, -1.0, -2.0);
	
	for(i = 0; i < c; i++)
	{
		if(&players[i] != active_player)
		{
			//mat4_t_compose(&transform, &players[i].player_orientation, players[i].player_position);		
			//mat4_t_mult_fast(&model_view_matrix, &transform, &active_camera->world_to_camera_matrix);
			
			
			//glLoadMatrixf(&model_view_matrix.floats[0][0]);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, body_color);
			
			glLoadMatrixf(&visible_players_body_transforms[i].floats[0][0]);
			glDrawArrays(GL_TRIANGLES, players[i].body_start, players[i].body_count);
			
			
			
			//mat4_t_compose(&transform, &players[i].player_camera->world_orientation, players[i].player_camera->world_position);
			
			//mat4_t_compose(&weapon_transform, &weapon_rot, weapon_position);
			//mat4_t_mult_fast(&temp_transform, &weapon_transform, &transform);
			//mat4_t_mult_fast(&model_view_matrix, &temp_transform, &active_camera->world_to_camera_matrix);
			
			
			//glLoadMatrixf(&model_view_matrix.floats[0][0]);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, weapon_color);
			glLoadMatrixf(&visible_players_weapon_transforms[i].floats[0][0]);
			glDrawArrays(GL_TRIANGLES, players[i].weapon_start, players[i].weapon_count);
			
		}
	}*/
	

	
	
	
	
/*	c = triangle_group_count;
	
	for(i = 0; i < c; i++)
	{
		triangle_group = &triangle_groups[i];
		glDrawElements(GL_TRIANGLES, triangle_group->vertex_count, GL_UNSIGNED_INT, (void *)(triangle_group->start * sizeof(int)));
	}*/
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	
/*	#ifdef QUERY_STAGES
	renderer_EndTimeElapsedQuery(RENDERER_Z_PREPASS_STAGE);
	#endif*/
	
	/*#ifdef QUERY_STAGES
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectuiv(query0, GL_QUERY_RESULT, &elapsed);	
	query_results[0] = (float)elapsed / 1000000.0;
	#endif*/
	
	
}


void renderer_DrawWorld()
{
	camera_t *active_camera = camera_GetActiveCamera();	
	//triangle_group_t *triangle_group;
	batch_t *batch;
	int i;
	int c;
	
	float s;
	float e;
	
	glViewport(0, 0, r_width, r_height);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world_element_buffer);
	
	c = w_world_batch_count;
	
	//if(!r_deferred)
	//{	
	
	if(r_flat)
	{
		renderer_SetShader(r_flat_pass_shader);
	}		
	else
	{
		renderer_SetShader(r_forward_pass_shader);
		renderer_SetClusterTexture();
			
		renderer_SetDefaultUniform1i(UNIFORM_r_width, r_width);
		renderer_SetDefaultUniform1i(UNIFORM_r_height, r_height);
		renderer_SetDefaultUniform1i(UNIFORM_r_frame, r_frame);
		//renderer_SetUniform4fv(UNIFORM_active_camera_position, &active_camera->world_position.floats[0]);
	}
		
	//}
	/*else
	{	
		renderer_SetShader(r_geometry_pass_shader);	
	}*/
	
	/*renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->position);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->normal);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->tangent);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, s izeof(vertex_t), &((vertex_t *)0)->tex_coord);*/	
	
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(compact_vertex_t), &((compact_vertex_t *)0)->position);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(compact_vertex_t), &((compact_vertex_t *)0)->normal);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(compact_vertex_t), &((compact_vertex_t *)0)->tangent);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, sizeof(compact_vertex_t), &((compact_vertex_t *)0)->tex_coord);
	
	renderer_SetProjectionMatrix(&active_camera->view_data.projection_matrix);
	renderer_SetViewMatrix(&active_camera->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_UpdateMatrices();

	for(i = 0; i < c; i++)
	{				
		batch = &w_world_batches[i];		
		renderer_SetMaterial(batch->material_index);
		renderer_DrawElements(GL_TRIANGLES, batch->next, GL_UNSIGNED_INT, (void *)((batch->start + w_world_index_start) * sizeof(int)));
	}
	
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);	
}


void renderer_ExecuteDrawCmds()
{
	int i;
	int j;
	int draw_cmd_count;
	
	draw_command_t *draw_cmds;

	renderer_SetClusterTexture();
	renderer_SetDefaultUniform1i(UNIFORM_r_width, r_width);
	renderer_SetDefaultUniform1i(UNIFORM_r_height, r_height);
	renderer_SetDefaultUniform1i(UNIFORM_r_frame, r_frame);

	for(i = 0; i < r_draw_command_group_count; i++)
	{
		renderer_SetMaterial(r_draw_command_groups[i].material_index);
		draw_cmd_count = r_draw_command_groups[i].draw_cmds_count;
		draw_cmds = r_draw_command_groups[i].draw_cmds;
		
		//printf("%d\n",draw_cmd_count);
				
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


void renderer_DrawOpaque()
{
	//camera_t *active_camera = camera_GetActiveCamera();
	mat4_t view_projection_matrix;
	mat4_t model_matrix;
	int i;
	int j;
	int draw_cmd_count;
	unsigned int draw_mode;
	unsigned int vert_start;
	unsigned int vert_count;
	float s;
	float e;
	
	draw_command_t *draw_cmds;
	
	if(r_flat)
	{
		renderer_SetShader(r_flat_pass_shader);
	}
	else
	{
		renderer_SetShader(r_forward_pass_shader);
	}
	
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(compact_vertex_t), &(((compact_vertex_t *)0)->position));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(compact_vertex_t), &(((compact_vertex_t *)0)->normal));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(compact_vertex_t), &(((compact_vertex_t *)0)->tangent));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, sizeof(compact_vertex_t), &(((compact_vertex_t *)0)->tex_coord));
	
	renderer_ExecuteDrawCmds();	
}


void renderer_DrawTranslucent()
{
	
}



void renderer_DrawShadowMaps()
{
	camera_t *active_camera = camera_GetActiveCamera();	
	triangle_group_t *triangle_group;
	material_t *material;
	light_position_t *pos;
	light_params_t *parms;
	int i;
	//int c = brush_count;
	mat4_t mvm;
	
	int j;
	int k;
	int light_index;
	
	int l;
	int m;
	
	int *next;
	int start;
	int cache_index;
	int shadow_map_index;
	
	int x;
	int y;
	
	int start_x;
	int start_y;
	
	float s;
	float e;
	
	int *r;
	
	float color[] = {1.0, 1.0, 1.0, 1.0};
	
	if(!w_world_leaves)
		return;
	
	
	/*#ifdef QUERY_STAGE
	renderer_BeginTimeElapsedQuery();
	#endif*/
	
	
	//gpu_BindGpuHeap();
	//shader_UseShader(shadow_pass_shader);
	
	//s = engine_GetDeltaTime();
	
	renderer_SetShader(r_shadow_pass_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->position);
	
	
	/*e = engine_GetDeltaTime();
	
	
	printf("[1: %f] ", e - s);
	
	
	s = e;*/
	
	//shader_SetCurrentShaderUniform1i(UNIFORM_LIGHT_COUNT, visible_light_count);
	//shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_FLAGS, 0);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&l_shadow_map_projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	/*e = engine_GetDeltaTime();
	
	printf("[2: %f] ", e - s);
	
	
	s = e;*/

	//while(glGetError() != GL_NO_ERROR);	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, l_shadow_map_frame_buffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	
/*	e = engine_GetDeltaTime();
	
	printf("[3: %f] ", e - s);
	
	s = e;*/
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l_light_cache_shadow_element_buffer);
	
	glClearColor(LIGHT_MAX_RADIUS, LIGHT_MAX_RADIUS, LIGHT_MAX_RADIUS, LIGHT_MAX_RADIUS);

	/*e = engine_GetDeltaTime();
	
	
	printf("[4: %f] ", e - s);
	
	s = e;*/
	
	k = w_visible_lights_count;
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	glEnable(GL_SCISSOR_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_MIN);
	/*
	e = engine_GetDeltaTime();
	
	printf("[5: %f]\n", e - s);*/
	
	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(1.0, 2.0);

	

	//printf(">>%x\n", glGetError());
	while(glGetError() != GL_NO_ERROR);
	for(j = 0; j < k; j++)
	{
		light_index = w_visible_lights[j];
		pos = &l_light_positions[light_index];
		parms = &l_light_params[light_index];
		
		if(!(parms->bm_flags & LIGHT_GENERATE_SHADOWS))
			continue;
		
		if(!(parms->bm_flags & LIGHT_UPDATE_SHADOW_MAP))
			continue;
		
		if(parms->bm_flags & LIGHT_DROPPED_SHADOW)
			continue;	
		
		parms->bm_flags &= ~LIGHT_UPDATE_SHADOW_MAP;
		
		
//		light_SetLight(light_index);
		cache_index = l_light_params[light_index].cache;
		start = l_light_cache[cache_index].offset * MAX_INDEXES_PER_FRUSTUM * 6;
		next = l_light_cache_frustum_counts + l_light_cache[cache_index].offset * 6;
		//shadow_map_index = light_cache[cache_index].offset;
		
		start_x = parms->x;
		start_y = parms->y;
		
		for(i = 0; i < 6; i++)
		{				
			mvm = l_shadow_map_mats[i];
			
			mvm.floats[3][0] = mvm.floats[0][0] * (-pos->position.x) + mvm.floats[1][0] * (-pos->position.y) + mvm.floats[2][0] * (-pos->position.z);
			mvm.floats[3][1] = mvm.floats[0][1] * (-pos->position.x) + mvm.floats[1][1] * (-pos->position.y) + mvm.floats[2][1] * (-pos->position.z);
			mvm.floats[3][2] = mvm.floats[0][2] * (-pos->position.x) + mvm.floats[1][2] * (-pos->position.y) + mvm.floats[2][2] * (-pos->position.z);
			mvm.floats[3][3] = 1.0;
		
		
			switch(i)
			{
				/* +X */
				case 0:
					x = start_x;
					y = start_y;
				break;
				
				/* -X */
				case 1:
					x = start_x;
					y = start_y + SHADOW_MAP_RESOLUTION;
				break;
				
				/* +Y */
				case 2:
					x = start_x + SHADOW_MAP_RESOLUTION;
					y = start_y;
				break;
				
				/* -Y */
				case 3:
					x = start_x + SHADOW_MAP_RESOLUTION;
					y = start_y + SHADOW_MAP_RESOLUTION;
				break;
				
				/* +Z */
				case 4:
					x = start_x + SHADOW_MAP_RESOLUTION * 2;
					y = start_y;
				break;
				
				/* -Z */
				case 5:
					x = start_x + SHADOW_MAP_RESOLUTION * 2;
					y = start_y + SHADOW_MAP_RESOLUTION;
				break;
			}
		
			//while(glGetError() != GL_NO_ERROR);
			//printf("1:%x\n", glGetError());
            glViewport(x, y, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
			//printf("2:%x\n", glGetError());
			glScissor(x, y, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
			//printf("3:%x\n", glGetError());
			glLoadMatrixf(&mvm.floats[0][0]);
			//printf("4:%x\n", glGetError());
			
			
			glClear(GL_COLOR_BUFFER_BIT);
			
			glDrawElements(GL_TRIANGLES, next[i], GL_UNSIGNED_INT, (void *)(start * sizeof(int)));
			start += MAX_INDEXES_PER_FRUSTUM;
			//printf("5:%x\n", glGetError());
			
			
		}
	}
	

	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	
	
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);
	glBlendEquation(GL_FUNC_ADD);
	//glCullFace(GL_BACK);
	
	glDepthMask(GL_TRUE);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	glViewport(0, 0, r_window_width, r_window_height);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	
	//renderer_BindBackbuffer();
/*	
	#ifdef QUERY_STAGES
	renderer_EndTimeElapsedQuery(RENDERER_DRAW_SHADOW_MAPS_STAGE);
	#endif*/
	
}


void renderer_DrawParticles()
{
	int i;
	int c;
	camera_t *active_camera;
	struct particle_system_t *ps;
	struct particle_system_t *particle_systems;
	
	active_camera = camera_GetActiveCamera();
	
	renderer_SetShader(r_particle_forward_pass_shader);
	renderer_SetClusterTexture();
	renderer_SetDefaultUniform1i(UNIFORM_r_width, r_width);
	renderer_SetDefaultUniform1i(UNIFORM_r_height, r_height);
	
	renderer_SetProjectionMatrix(&active_camera->view_data.projection_matrix);
	renderer_SetViewMatrix(&active_camera->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_UpdateMatrices();
	
	glDisable(GL_CULL_FACE);
	
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
		
		renderer_SetTexture(GL_TEXTURE0, GL_TEXTURE_2D_ARRAY, -1);
		renderer_SetDefaultUniform4fv(UNIFORM_particle_positions, ps->particle_count, (float *)ps->particle_positions);
		renderer_SetDefaultUniform1iv(UNIFORM_particle_frames, ps->particle_count, ps->particle_frames);
		renderer_SetDefaultUniform1i(UNIFORM_texture_array_sampler0, 0);
		renderer_DrawArraysInstanced(GL_QUADS, ps_particle_quad_start, 4, ps->particle_count);
	}
	
	glEnable(GL_CULL_FACE);
	
}



void renderer_DrawGUI()
{
	widget_t *w;
	widget_t *selected;
	
	int widget_stack_top = -1;
	int b_do_top = 0;
	widget_t *widget_stack[128];
	button_t *button;
	checkbox_t *checkbox;
	dropdown_t *dropdown;
	option_list_t *options;
	option_t *option;
	widget_bar_t *bar;
	text_field_t *field;
	slider_t *slider;
	wsurface_t *surface;
	item_list_t *list;
	
	short x = 0;
	short y = 0;
	float y_offset;
	
	
/*	#ifdef QUERY_STAGES
	renderer_BeginTimeElapsedQuery();
	#endif*/
	
	renderer_BindWindowBuffer(0, 0);
	
	CreateOrthographicMatrix(&gui_projection_matrix, -(r_window_width >> 1), r_window_width >> 1, r_window_height >> 1, -(r_window_height >> 1), -10.0, 10.0, NULL);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&gui_projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glUseProgram(0);
	renderer_SetShader(-1);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0);
	
	
	
	
	w = last_widget;
	
	_do_top:
	
	while(w)
	{
				
		if(w->bm_flags & WIDGET_INVISIBLE)
			goto _advance_widget;
	
			
		if(w->bm_flags & WIDGET_OUT_OF_BOUNDS)
			goto _advance_widget;
		
				
		
		/*if(w->bm_flags & WIDGET_IGNORE_EDGE_CLIPPING)
			glDisable(GL_STENCIL_TEST);
		else
			glEnable(GL_STENCIL_TEST);*/
			
		if(w->bm_flags & WIDGET_SHOW_NAME)
		{
			renderer_BlitSurface(w->rendered_name, w->x + x - w->w + r_window_width * 0.5 + 1, w->y + y + w->h + 18.0 + r_window_height * 0.5 - 2);
		}	
		
		switch(w->type)
		{
			case WIDGET_BASE:
				
				selected = NULL;
				
				renderer_PushScissorRect((int)((x + w->x - w->w) + r_window_width * 0.5) - 1, (int)((y + w->y - w->h) + r_window_height * 0.5) - 1, ((int)w->w) * 2 + 1, ((int)w->h) * 2 + 1, 1);
				
				if(w->parent)
				{
					if(w->parent->type == WIDGET_ITEM_LIST)
					{
						list = (item_list_t *)w->parent;
						
						if(list->flags & ITEM_LIST_DOUBLE_CLICK_SELECTION)
						{
							selected = list->selected_item;
						}
						
					}
				}
		
		
				if(selected == w)
				{
					glColor3f(0.2, 0.2, 1.0);
				}
				else if(!(w->bm_flags & WIDGET_NO_HIGHLIGHT))
				{
					if(w->bm_flags & WIDGET_MOUSE_OVER || (w->edge_flags & (WIDGET_RIGHT_EDGE_GRABBED | 
																			WIDGET_LEFT_EDGE_GRABBED | 
																			WIDGET_TOP_EDGE_GRABBED | 
																		    WIDGET_BOTTOM_EDGE_GRABBED)))
					{
						glColor3f(0.5, 0.5, 0.5);
					}
					else
					{
						glColor3f(0.4, 0.4, 0.4);
					}
				}
				
				else
				{
					glColor3f(0.4, 0.4, 0.4);
				}
				
				glRectf(w->x - w->w + x, w->y - w->h + y, w->x + w->w + x, w->y + w->h + y);
				
				if(w->edge_flags & (WIDGET_MOUSE_OVER_RIGHT_EDGE | WIDGET_RIGHT_EDGE_GRABBED))
				{
					glColor3f(0.7, 0.7, 0.7);
					glRectf(w->x + w->w + x - WIDGET_BORDER_WIDTH, w->y - w->h + y, w->x + w->w + x, w->y + w->h + y);
				}
				/*else if(w->left_edge_of)
				{
					glColor3f(0.3, 0.3, 0.3);
					glRectf(w->x + w->w + x - WIDGET_BORDER_WIDTH, w->y - w->h + y, w->x + w->w + x, w->y + w->h + y);
				}*/
				
				if(w->edge_flags & (WIDGET_MOUSE_OVER_LEFT_EDGE | WIDGET_LEFT_EDGE_GRABBED))
				{
					glColor3f(0.7, 0.7, 0.7);
					glRectf(w->x - w->w + x, w->y - w->h + y, w->x - w->w + x + WIDGET_BORDER_WIDTH, w->y + w->h + y);
				}
				
				if(w->edge_flags & (WIDGET_MOUSE_OVER_TOP_EDGE | WIDGET_TOP_EDGE_GRABBED))
				{
					glColor3f(0.7, 0.7, 0.7);
					glRectf(w->x - w->w + x, w->y + w->h + y - WIDGET_BORDER_WIDTH, w->x + w->w + x, w->y + w->h + y);
				}
				
				if(w->edge_flags & (WIDGET_MOUSE_OVER_BOTTOM_EDGE | WIDGET_BOTTOM_EDGE_GRABBED))
				{
					glColor3f(0.7, 0.7, 0.7);
					glRectf(w->x - w->w + x, w->y - w->h + y, w->x + w->w + x, w->y - w->h + y + WIDGET_BORDER_WIDTH);
				}
				
				if(w->bm_flags & WIDGET_DRAW_OUTLINE)
				{
					glColor3f(0.0, 0.0, 0.0);
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glRectf(w->x - w->w + x, w->y - w->h + y, w->x + w->w + x, w->y + w->h + y);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				
				/*if(w->bm_flags & WIDGET_SHOW_NAME)
				{
					renderer_BlitSurface(w->rendered_name, w->x + x - w->w + r_window_width * 0.5 + 1, w->y + y + w->h + 18.0 + r_window_height * 0.5 - 2);
				}*/
				
				
				if(w->nestled)
				{
					x += w->x;
					y += w->y;
					
					widget_stack_top++;
					widget_stack[widget_stack_top] = w;
					w = w->last_nestled;
					continue;
				}
			break;
			
			case WIDGET_ITEM_LIST:
				
				/*glBegin(GL_QUADS);
				glColor3f(0.4, 0.4, 0.4);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();*/
				
				renderer_PushScissorRect((int)((x + w->x - w->w) + r_window_width * 0.5) - 1, (int)((y + w->y - w->h) + r_window_height * 0.5) - 1, ((int)w->w) * 2 + 1, ((int)w->h) * 2 + 1, 1);
				
				glColor3f(0.4, 0.4, 0.4);
				glRectf(x + w->x - w->w, y + w->y - w->h, x + w->x + w->w, y + w->y + w->h);
				
				/*if(w->bm_flags & WIDGET_SHOW_NAME)
				{
					renderer_BlitSurface(w->rendered_name, w->x + x - w->w + r_window_width * 0.5 + 1, w->y + y + w->h + 18.0 + r_window_height * 0.5 - 2);
				}*/
				
				
				if(w->nestled)
				{
					
					x += w->x;
					y += w->y;
					
					widget_stack_top++;
					widget_stack[widget_stack_top] = w;
					w = w->last_nestled;
					continue;
				}
			break;
			
			case WIDGET_BUTTON:
				button = (button_t *)w;
				//glBegin(GL_QUADS);
				
				renderer_PushScissorRect((int)((x + w->x - w->w) + r_window_width * 0.5) - 1, (int)((y + w->y - w->h) + r_window_height * 0.5) - 1, ((int)w->w) * 2 + 1, ((int)w->h) * 2 + 1, 1);
				
				if(button->bm_button_flags & BUTTON_PRESSED)
				{
					glColor3f(0.35, 0.35, 0.35);
				}
				else
				{
					if(w->bm_flags & WIDGET_MOUSE_OVER)
					{
						glColor3f(0.5, 0.5, 0.5);
					}
					else
					{
						glColor3f(0.4, 0.4, 0.4);
					}
				}
				
				//glRectf(w->x + w->w, w->y - w->h, w->x - w->w, w->y + w->h);
				glRectf(x + w->x - w->w, y + w->y - w->h, x + w->x + w->w, y + w->y + w->h);
				/*glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();*/
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0 ,0.0);
				glRectf(x + w->x - w->w, y + w->y - w->h, x + w->x + w->w, y + w->y + w->h);
				/*glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();*/
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				
				renderer_BlitSurface(button->rendered_text, w->x + x - w->w + r_window_width * 0.5 + 1, w->y + y + w->h + r_window_height * 0.5 - 2);
				
				renderer_PopScissorRect();
				
			break;
			
			case WIDGET_SLIDER:
				slider = (slider_t *)w;
				
				//glBegin(GL_QUADS);
				
				/*if(button->bm_button_flags & BUTTON_PRESSED)
				{
					glColor3f(0.35, 0.35, 0.35);
				}
				else
				{*/
				if(w->bm_flags & WIDGET_MOUSE_OVER)
				{
					glColor3f(0.5, 0.5, 0.5);
				}
				else
				{
					glColor3f(0.4, 0.4, 0.4);
				}
				//}
				
				//glRectf(w->x + w->w, w->y - w->h, w->x - w->w, w->y + w->h);
				glRectf(x + w->x - w->w, y + w->y - w->h, x + w->x + w->w, y + w->y + w->h);
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
				glRectf(x + w->x - w->w, y + w->y - w->h, x + w->x + w->w, y + w->y + w->h);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				
				
				glColor3f(0.25, 0.25, 0.25);
				glPointSize(SLIDER_HANDLE_SIZE);
				
				glBegin(GL_POINTS);
				glVertex3f(x + w->x - w->w + w->w * (1.0 - slider->slider_position) * 2.0, y + w->y, 0.0);
				glEnd();
				
				/*if(w->bm_flags & WIDGET_SHOW_NAME)
				{
					renderer_BlitSurface(w->rendered_name, w->x + x - w->w + r_window_width * 0.5 + 1, w->y + y + w->h + 18.0 + r_window_height * 0.5 - 2);
				}*/
				
			break;	
			
			case WIDGET_CHECKBOX:
				checkbox = (checkbox_t *)w;
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.4, 0.4, 0.4);
				glLineWidth(2.0);
				glRectf(x + w->x - w->w, y + w->y - w->h, x + w->x + w->w, y + w->y + w->h);
				/*glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();*/
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glLineWidth(1.0);
				
				if(checkbox->bm_checkbox_flags & CHECKBOX_CHECKED)
				{
					glColor3f(0.9, 0.9, 0.9);
					glRectf(x + w->x - w->w, y + w->y - w->h, x + w->x + w->w, y + w->y + w->h);
					/*glBegin(GL_QUADS);
					glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
					glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
					glEnd();*/
				}
			break;
			
			case WIDGET_DROPDOWN:
				dropdown = (dropdown_t *)w;
				
				renderer_PushScissorRect((int)((x + w->x - w->w) + r_window_width * 0.5) - 1, (int)((y + w->y - w->h) + r_window_height * 0.5) - 1, ((int)w->w) * 2 + 1, ((int)w->h) * 2 + 1, 1);
				
				if(dropdown->bm_dropdown_flags & DROPDOWN_DROPPED)
				{
					glColor3f(0.35, 0.35, 0.35);
				}
				else
				{
					if(w->bm_flags & WIDGET_MOUSE_OVER)
					{
						glColor3f(0.5, 0.5, 0.5);
					}
					else
					{
						glColor3f(0.4, 0.4, 0.4);
					}
					
				}
				
				/*glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();*/
				
				glRectf(x + w->x - w->w, y + w->y - w->h, x + w->x + w->w, y + w->y + w->h);
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
				
				/*glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();*/
				
				glRectf(x + w->x - w->w, y + w->y - w->h, x + w->x + w->w, y + w->y + w->h);	
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				
				renderer_BlitSurface(dropdown->rendered_text, w->x + x - w->w + r_window_width * 0.5 + 1, w->y + y + w->h + r_window_height * 0.5 - 2);
				
				renderer_PopScissorRect();
				
				if(w->nestled)
				{
					if(dropdown->bm_dropdown_flags & DROPDOWN_DROPPED)
					{
						x += w->x;
						y += w->y;
						
						widget_stack_top++;
						widget_stack[widget_stack_top] = w;
						w = w->last_nestled;
						continue;
					}
					
				}
				
				
				
			break;	
			
			case WIDGET_OPTION_LIST:
				
				options = (option_list_t *)w;
				
				/*glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();*/
				
				renderer_PushScissorRect((int)((x + w->x - w->w) + r_window_width * 0.5) - 1, (int)((y + w->y - w->h) + r_window_height * 0.5) - 1, ((int)w->w) * 2 + 1, ((int)w->h) * 2 + 1, 0);
				
				if(options->bm_option_list_flags & OPTION_LIST_SCROLLER)
				{
					glColor3f(0.3, 0.3, 0.3);
					glRectf(w->x + w->w + x - 10, w->y - w->h + y, w->x + w->w + x, w->y + w->h + y);
					
					/*y_offset = ((float)(options->y_offset / OPTION_HEIGHT) / (float)(options->option_count));
					printf("%f\n", y_offset);
					
					y_offset = y_offset * options->widget.h - OPTION_HEIGHT;
					
					
					glColor3f(0.25, 0.25, 0.25);
					glRectf(w->x + w->w + x - 10, w->y + w->h + y - 15 - y_offset, w->x + w->w + x, w->y + w->h + y - y_offset);*/
					
					/*glBegin(GL_QUADS);
					glVertex3f(w->x + w->w + x - 10, w->y + w->h + y, 0.0);
					glVertex3f(w->x + w->w + x - 10, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
					glEnd();*/
				}
				
				
				
				if(w->bm_flags & WIDGET_DRAW_OUTLINE)
				{
					glColor3f(0.0, 0.0, 0.0);
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glRectf(w->x - w->w + x, w->y - w->h + y, w->x + w->w + x, w->y + w->h + y);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				
				if(w->nestled)
				{
					x += w->x;
					y += w->y;
					
					widget_stack_top++;
					widget_stack[widget_stack_top] = w;
					w = w->last_nestled;
					continue;
				}
				
				renderer_PopScissorRect();
			
			break;
			
			case WIDGET_OPTION:
				option = (option_t *)w;
				options = (option_list_t *)w->parent;
				
				renderer_PushScissorRect((int)((x + w->x - w->w) + r_window_width * 0.5) - 1, (int)((y + w->y - w->h) + r_window_height * 0.5) - 1, ((int)w->w) * 2 + 1, ((int)w->h) * 2 + 1, 1);
				//printf("%d\n", option->widget.bm_flags  & WIDGET_OUT_OF_BOUNDS);
						
				//if(w->bm_flags & WIDGET_MOUSE_OVER)
				if(option == (option_t *)options->selected_option)
				{
					glColor3f(0.2, 0.2, 1.0);
				}
				else if(option == (option_t *)options->active_option)
				{
					glColor3f(0.5, 0.5, 0.5);
				}
				else
				{
					glColor3f(0.4, 0.4, 0.4);
				}
				
				glRectf(w->x - w->w + x, w->y - w->h + y, w->x + w->w + x, w->y + w->h + y);
				
				/*glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();*/
				
				if(!(options->bm_option_list_flags & OPTION_LIST_NO_OPTION_DIVISIONS))
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor3f(0.0, 0.0, 0.0);
					glRectf(x + w->x - w->w, y + w->y - w->h, x + w->x + w->w, y + w->y + w->h);
					/*glBegin(GL_QUADS);
					glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
					glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
					glEnd();*/
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				
				//glEnable(GL_SCISSOR_TEST);
				
				renderer_DrawRenderedString(option->rendered_string, w->x + x - w->w, w->y + y + w->h);
				
				//glDisable(GL_SCISSOR_TEST);
			
				if(w->nestled)
				{
					if(option == (option_t *)options->selected_option)
					{
						glColor3f(1.0, 1.0, 1.0);
					}
					else if(option == (option_t *)options->active_option)
					{
						glColor3f(0.7, 0.7, 0.7);
					}
					else
					{
						glColor3f(0.2, 0.2 ,0.2);
					}
					
					glBegin(GL_TRIANGLES);
					glVertex3f(w->x + w->w + x, w->y + y, 0.0);
					glVertex3f(w->x + w->w + x - 15.0, w->y + w->h + y - 5.0, 0.0);
					glVertex3f(w->x + w->w + x - 15.0, w->y - w->h + y + 5.0, 0.0);
					glEnd();
					
					if(option == (option_t *)options->active_option)
					{
						x += w->x;
						y += w->y;
						
						widget_stack_top++;
						widget_stack[widget_stack_top] = w;
						w = w->last_nestled;
						continue;
					}
				
				}
				
				renderer_PopScissorRect();
				
			break;
				
			case WIDGET_BAR:
				bar = (widget_bar_t *)w;
					
					
				renderer_PushScissorRect((int)((x + w->x - w->w) + r_window_width * 0.5), (int)((y + w->y - w->h) + r_window_height * 0.5), ((int)w->w) * 2 + 1, ((int)w->h) * 2 + 1, 1);
					/*if(button->bm_button_flags & BUTTON_PRESSED)
					{
						glColor3f(0.35, 0.35, 0.35);
					}
					else
					{
						if(w->bm_flags & WIDGET_MOUSE_OVER)
						{
							glColor3f(0.5, 0.5, 0.5);
						}
						else
						{
							glColor3f(0.4, 0.4, 0.4);
						}
					}*/
					
					//glRectf(w->x + w->w, w->y - w->h, w->x - w->w, w->y + w->h);
					
				if(w->bm_flags & WIDGET_MOUSE_OVER)
				{
					glColor3f(0.5, 0.5, 0.5);
				}
				else
				{
					glColor3f(0.4, 0.4, 0.4);
				}
					
				/*glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();*/
				glRectf(x + w->x - w->w, y + w->y - w->h, x + w->x + w->w, y + w->y + w->h);
				
				if(w->nestled)
				{
					x += w->x;
					y += w->y;
					
					widget_stack_top++;
					widget_stack[widget_stack_top] = w;
					w = w->last_nestled;
					continue;
				}
				
				renderer_PopScissorRect();
					
				/*glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0 ,0.0);
					glBegin(GL_QUADS);
					glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
					glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
					glEnd();
					
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);*/
					
			
				
			break;
			
			case WIDGET_TEXT_FIELD:
				
				field = (text_field_t *)w;
				
				renderer_PushScissorRect((int)((x + w->x - w->w) + r_window_width * 0.5) - 1, (int)((y + w->y - w->h) + r_window_height * 0.5) - 1, ((int)w->w) * 2 + 1, ((int)w->h) * 2 + 1, 1);
				
				if(w->bm_flags & WIDGET_MOUSE_OVER)
				{
					glColor3f(0.4, 0.4, 0.4);
				}
				else
				{
					glColor3f(0.3, 0.3, 0.3);
				}
				
				
				
				/*glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();*/
				
				glRectf(x + w->x - w->w, y + w->y - w->h, x + w->x + w->w, y + w->y + w->h);
				
				if(field->bm_text_field_flags & TEXT_FIELD_CURSOR_VISIBLE)
				{
					glColor3f(1.0, 1.0, 1.0);
					glBegin(GL_QUADS);
					glVertex3f(w->x - w->w + x, w->y + w->h + y - 1.0, 0.0);
					glVertex3f(w->x - w->w + x, w->y - w->h + y + 1.0, 0.0);
					glVertex3f(w->x - w->w + x + 2.0, w->y - w->h + y + 1.0, 0.0);
					glVertex3f(w->x - w->w + x + 2.0, w->y + w->h + y - 1.0, 0.0);
					glEnd();
				}
				
				if(field->bm_text_field_flags & TEXT_FIELD_DRAW_TEXT_SELECTED)
				{
					glColor3f(0.6, 0.6, 0.6);
					glBegin(GL_QUADS);
					glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
					glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
					glEnd();
				}
				
				//glEnable(GL_SCISSOR_TEST);
				//glScissor((x + w->x - w->w) + r_window_width * 0.5, (y + w->y - w->h) + r_window_height * 0.5, w->w * 2.0, w->h * 2.0);
				//renderer_PushScissorRect((int)((x + w->x - w->w) + r_window_width * 0.5), (int)((y + w->y - w->h) + r_window_height * 0.5), ((int)w->w) * 2, ((int)w->h) * 2, 1);
				renderer_DrawRenderedString(field->rendered_string, w->x + x - w->w, w->y + y + w->h);
				renderer_PopScissorRect();
				//glDisable(GL_SCISSOR_TEST);
				
				//renderer_BlitSurface(field->rendered_text, w->x + x - w->w + r_window_width * 0.5 + 1, w->y + y + w->h + r_window_height * 0.5 - 2);
				
			break;
			
			
			case WIDGET_SURFACE:
				surface = (wsurface_t *)w;
				
				
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, surface->color_texture);
				
				
				glEnable(GL_TEXTURE_2D);
				glColor3f(0.6, 0.6, 0.6);
				glBegin(GL_QUADS);
				
				glTexCoord2f(0.0, 1.0);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				
				glTexCoord2f(0.0, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				
				glTexCoord2f(1.0, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				
				glTexCoord2f(1.0, 1.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				
				glEnd();
				glDisable(GL_TEXTURE_2D);
			break;
		}
		
		
		/* If this is the top widget, and everything
		nestled in it was already processed, quit the loop
		to avoid rerendering anything on the list... */
		/*if(b_do_top && widget_stack_top < 0)
		{
			break;
		}*/
		
		_advance_widget:
		
		w = w->prev;
		
		if(!w)
		{
			if(widget_stack_top >= 0)
			{
				renderer_PopScissorRect();
				w = widget_stack[widget_stack_top];
				x -= w->x;
				y -= w->y;
				widget_stack_top--;
				goto _advance_widget;
			}
		}
		
	}
	
	
	renderer_ClearScissorRectStack();
	//renderer_DrawCachedString(0, -500, 0);
	
	/* All the list (excluding the top widget) have been
	drawn, so repeat the loop just for the top widget 
	(and any eventual nestled widget)... */
	/*if(!b_do_top)
	{
		b_do_top = 1;
		w = top_widget;
		goto _do_top;
	}*/
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	
	
	
/*	#ifdef QUERY_STAGES
	renderer_EndTimeElapsedQuery(RENDERER_DRAW_GUI);
	#endif*/
	
	
}


void renderer_DrawBloom()
{
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
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_bbuffer.framebuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glViewport(0, 0, r_width, r_height);
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, r_cbuffer.framebuffer_id);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	
	//while(glGetError() != GL_NO_ERROR);
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_width, r_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_width, r_height, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	//printf("renderer_BlitColorbuffer: %x\n", glGetError());
}

void renderer_BlitBackbuffer()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, r_window_width, r_window_height);
	
	renderer_SetShader(r_blit_texture_shader);
	renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_bbuffer.color_attachments[0].handle);
	renderer_SetDefaultUniform1i(UNIFORM_texture_sampler0, 0);
	glRectf(-1.0, -1.0, 1.0, 1.0);
	
	//renderer_SetProjectionMatrix(NULL);
	//renderer_SetViewMatrix(NULL);
	//renderer_SetModelMatrix(NULL);
	//renderer
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, r_bbuffer_id);
	//glReadBuffer(GL_COLOR_ATTACHMENT0);
	
	//while(glGetError() != GL_NO_ERROR);
	//glBlitFramebuffer(0, 0, r_width - 1, r_height - 1, 0, 0, r_window_width - 1, r_window_height - 1, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	//printf("renderer_BlitBackbuffer: %x\n", glGetError());
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





void renderer_Tonemap()
{
	
	
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//glDrawBuffer(GL_BACK);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_bbuffer.framebuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	//renderer_BindBackbuffer(1, 0);
	//renderer_BindColorbuffer(0, 1);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, r_cbuffer.framebuffer_id);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	
	glViewport(0, 0, r_width, r_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//while(glGetError() != GL_NO_ERROR);
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_width, r_height, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	//glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_width, r_height, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	//printf("renderer_Tonemap: %x\n", glGetError());
	
	renderer_SetShader(r_tonemap_shader);
	renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_cbuffer.color_attachments[0].handle);
	renderer_SetDefaultUniform1i(UNIFORM_texture_sampler0, 0);
	
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glRectf(-1.0, -1.0, 1.0, 1.0);
	
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	
	
	
	
	
	
	//glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_window_width, r_window_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	
	
	
}

void renderer_Shade()
{
/*	renderer_BindColorbuffer(1, 0);
	renderer_SetShader(r_shade_pass_shader);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, r_gbuffer_color);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, r_gbuffer_normal);
	
	renderer_SetUniform1i(UNIFORM_texture_sampler0, 0);
	renderer_SetUniform1i(UNIFORM_texture_sampler1, 1);
	renderer_SetUniform1i(UNIFORM_r_width, r_width);
	renderer_SetUniform1i(UNIFORM_r_height, r_height);
	
	renderer_SetClusterTexture();
	
	glDepthMask(GL_FALSE);
	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0, 0.0);
	glVertex3f(-1.0, -1.0, 0.0);
	glVertex3f(1.0, -1.0, 0.0);
	glVertex3f(1.0, 1.0, 0.0);
	glEnd();	
	glDepthMask(GL_TRUE); 
	
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, r_gbuffer_id);
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_window_width, r_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	
	glActiveTexture(GL_TEXTURE0);		*/
}


void renderer_DrawWorldDeferred()
{
	#if 0
	camera_t *active_camera = camera_GetActiveCamera();	
	triangle_group_t *triangle_group;
	material_t *material;
	light_position_t *pos;
	light_params_t *parms;
	mat4_t camera_to_world_matrix;
	mat4_t camera_to_light_matrix;
	mat4_t mat;
	GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	int i;
	int c = brush_count;
	
	int j;
	int k;
	int light_index;
	
	int l;
	int m;
	
	int *next;
	int start;
	int cache_index;
	
	int *r;
	
	float color[] = {1.0, 1.0, 1.0, 1.0};

	

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, geometry_framebuffer);
	glDrawBuffers(2, (const GLenum *)&draw_buffers);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	glViewport(0, 0, r_width, r_height);
	
	shader_UseShader(geometry_pass_shader);
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_FLAGS, 0);
	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	//while(glGetError() != GL_NO_ERROR);
		
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world_element_buffer);
	
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_cache_element_buffer);
	c = world_triangle_group_count;
	
	//k = visible_light_count;
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_BLEND);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0x1, 0xff);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	//glDepthMask(GL_TRUE);

	
	c = world_triangle_group_count;	
		
	for(i = 0; i < c; i++)
	{	
		triangle_group = &world_triangle_groups[i];
		material = &materials[triangle_group->material_index];
			
		color[0] = (float)material->r / 255.0;
		color[1] = (float)material->g / 255.0;
		color[2] = (float)material->b / 255.0;
			//color[3] = (float)material->a / 255.0;
		color[3] = 1.0;
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
			
		glDrawElements(GL_TRIANGLES, triangle_group->next, GL_UNSIGNED_INT, (void *)(triangle_group->start * sizeof(int)));
	}
	//}
	
	//printf("%x\n", glGetError());
	
	//glDisable(GL_BLEND);	
	//glDisable(GL_STENCIL_TEST);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	/*glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);*/
	
	#endif 
}






#if 0
/* old deferred path... */
void renderer_Shade()
{
	int i;
	int c;
	
	
	unsigned int elapsed;
	float ms_elapsed;
	int light_index;
	int cache_index;
	camera_t *active_camera = camera_GetActiveCamera();
	light_position_t *pos;
	light_params_t *parms;
	mat4_t camera_to_world_matrix;
	mat4_t world_to_light_matrix;
	mat4_t camera_to_light_matrix;
	/*glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();*/
		
	//glUseProgram(0);
	
	
	glLoadIdentity();
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	
	
	
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, geometry_framebuffer);
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_window_width, r_window_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glClear(GL_STENCIL_BUFFER_BIT);
	glViewport(0, 0, r_window_width, r_window_height);
	
	
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0, 0xff);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);
	
	
	//glBindBuffer(GL_ARRAY_BUFFER, stencil_meshes);
	shader_UseShader(stencil_lights_pass_shader);
	shader_SetCurrentVertexAttribPointer(ATTRIB_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	c = visible_light_count;
	glLoadIdentity();

	
	glBeginQuery(GL_TIME_ELAPSED, query_object);
	
	for(i = 0; i < c; i++)
	{
		light_index = visible_lights[i];
		light_SetLight(light_index);
		
		glDrawArrays(GL_TRIANGLES, 0, light_stencil_mesh_vert_count);
			
		/*glCullFace(GL_BACK);
		glDrawArrays(GL_TRIANGLES, 0, light_stencil_mesh_vert_count);	
		
		glCullFace(GL_FRONT);
		glDrawArrays(GL_TRIANGLES, 0, light_stencil_mesh_vert_count);	*/
		
	}
	
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectuiv(query_object, GL_QUERY_RESULT, &elapsed);	
	ms_elapsed = (float)elapsed / 1000000.0;
	
	printf("stecil mask: %f    ", ms_elapsed);
	
	
	
	
	
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	glCullFace(GL_BACK);
	
	
	
	
	glStencilFunc(GL_NOTEQUAL, 0, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	shader_UseShader(shading_pass_shader);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_buffer);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depth_buffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, albedo_buffer);
	
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_3D, cluster_texture);
	
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_SAMPLER0, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_SAMPLER1, 1);
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_SAMPLER2, 2);
	shader_SetCurrentShaderUniform1i(UNIFORM_CLUSTER_TEXTURE, 5);
	shader_SetCurrentShaderUniform1i(UNIFORM_WIDTH, r_width);
	shader_SetCurrentShaderUniform1i(UNIFORM_HEIGHT, r_height);
	shader_SetCurrentShaderUniform1i(UNIFORM_FRAME, r_frame);
	
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBeginQuery(GL_TIME_ELAPSED, query_object);
	//for(i = 0; i < c; i++)
	{
		/*light_index = visible_lights[i];
		light_SetLight(light_index);
		
		pos = &light_positions[light_index];
		parms = &light_params[light_index];
		
		cache_index = parms->cache;
		
		if(parms->bm_flags & LIGHT_GENERATE_SHADOWS)
		{
			world_to_light_matrix.floats[3][0] = -pos->position.x;
			world_to_light_matrix.floats[3][1] = -pos->position.y;
			world_to_light_matrix.floats[3][2] = -pos->position.z;
			
			mat4_t_mult_fast(&camera_to_light_matrix, &camera_to_world_matrix, &world_to_light_matrix);
			shader_SetCurrentShaderUniformMatrix4f(UNIFORM_CAMERA_TO_LIGHT_MATRIX, &camera_to_light_matrix.floats[0][0]);
			shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_CUBE_SAMPLER0, 5);
			glBindTexture(GL_TEXTURE_CUBE_MAP, light_cache_shadow_maps[light_cache[cache_index].offset]);
		}*/
		
		/*glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(-1.0, 1.0, -0.1);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-1.0,-1.0, -0.1);
		glTexCoord2f(1.0, 0.0);
		glVertex3f( 1.0,-1.0, -0.1);
		glTexCoord2f(1.0, 1.0);
		glVertex3f( 1.0, 1.0, -0.1);
		glEnd();*/
		glRectf(-1.0, -1.0, 1.0, 1.0);
	}
	
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectuiv(query_object, GL_QUERY_RESULT, &elapsed);	
	ms_elapsed = (float)elapsed / 1000000.0;
	
	printf("shading: %f\n", ms_elapsed);
	
	glActiveTexture(GL_TEXTURE0);
	
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	/*glBegin(GL_QUADS);
	glVertex3f(-1.0, 1.0, -0.1);
	glVertex3f(-1.0,-1.0, -0.1);
	glVertex3f( 1.0,-1.0, -0.1);
	glVertex3f( 1.0, 1.0, -0.1);
	glEnd();*/
	
	/*glMatrixMode(GL_PROJECTION);
	glPopMatrix();*/

	glPopMatrix();
	
}

#endif




void renderer_BeginTimeElapsedQuery()
{
	#ifdef QUERY_STAGES
	glBeginQuery(GL_TIME_ELAPSED, query_object);
	#endif
}

void renderer_EndTimeElapsedQuery(int stage_index)
{
	#ifdef QUERY_STAGES
	unsigned int elapsed;
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectuiv(query_object, GL_QUERY_RESULT, &elapsed);
	query_results[stage_index] = (float)elapsed / 1000000.0;
	#endif
}


void renderer_ReportQueryResults()
{
	#ifdef QUERY_STAGES
	int i;
	
	for(i = 0; i < RENDERER_STAGE_COUNT; i++)
	{
		if(query_results[i] >= 0.0)
		{
			printf("%s: %f	", stage_str[i], query_results[i]);
		}
		query_results[i] = -1.0;
		
	}
	
	printf("\n");
	
	#endif
	
}

void renderer_StartGpuTimer()
{
	glBeginQuery(GL_TIME_ELAPSED, query_object);
}

float renderer_StopGpuTimer()
{
	unsigned int elapsed;
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectuiv(query_object, GL_QUERY_RESULT, &elapsed);
	return (float)elapsed / 1000000.0;
}


void renderer_GetWindowSize(int *w, int *h)
{
	*w = r_width;
	*h = r_height;
}











