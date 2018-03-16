#include <stdio.h>
#include <string.h>

#include "r_common.h"
#include "r_main.h"
#include "r_text.h"


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

#include "engine.h"



SDL_Window *window;
SDL_GLContext context;




int r_active_shader = -1;

unsigned int geometry_framebuffer;
unsigned int albedo_buffer;
unsigned int normal_buffer;
unsigned int depth_buffer;


int z_pre_pass_shader;
int forward_pass_shader;
int geometry_pass_shader;
int shading_pass_shader;
int stencil_lights_pass_shader;
int shadow_pass_shader;
int skybox_shader;
int bloom0_shader;
int bloom1_shader;
int tonemap_shader;


//unsigned int cursor_framebuffer_id;
//unsigned int cursor_color_texture_id;
//unsigned int cursor_depth_texture_id;


/* from gpu.c */
extern unsigned int gpu_heap;

/* from world.c */
extern unsigned int world_element_buffer;
extern int world_triangle_group_count;
extern triangle_group_t *world_triangle_groups;
extern int world_leaves_count;
extern bsp_dleaf_t *world_leaves;


/* from collision.c */
extern int collision_geometry_start;
extern int collision_geometry_count;
extern int collision_geometry_vertice_count;
extern vec3_t *collision_geometry_positions;
extern vec3_t *collision_geometry_normals;


/* from material.c */
extern int material_count;
extern material_t *materials;

/* from texture.c */
extern texture_t *textures;


/* from l_main.c */
//extern light_position_t *visible_light_positions;
//extern light_params_t *visible_light_params;
//extern light_position_t *light_positions;
extern int visible_lights[];
extern int visible_light_count;
extern unsigned int cluster_texture;
extern unsigned int shared_shadow_map;


extern shader_t *shaders;


/* from brush.c */
extern int brush_count;
//extern brush_t *brushes;


/* from gui.c */
extern widget_t *widgets;
extern widget_t *last_widget;
extern widget_t *top_widget;
extern mat4_t gui_projection_matrix;

/* from l_main.c */
extern light_position_t *light_positions;
extern light_params_t *light_params;
extern unsigned int shadow_map_frame_buffer;
extern mat4_t shadow_map_mats[];
extern mat4_t shadow_map_projection_matrix;
extern int stencil_light_mesh_vert_count;
extern unsigned int stencil_light_mesh_start;


/* from l_cache.c */
extern unsigned int light_cache_element_buffer;
extern unsigned int light_cache_shadow_element_buffer;
extern int *light_cache_groups_next;
extern int *light_cache_frustum_counts;
extern light_cache_slot_t light_cache[];
extern unsigned int light_cache_shadow_maps[];


/* from player.c */
extern player_count;
extern player_t *players;
extern player_t *active_player;

/* from bsp.c */
//extern bsp_node_t *world_bsp;


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

unsigned int renderer_DrawWorldQueryObject;
unsigned int renderer_ZPrePassQueryObject;
unsigned int renderer_DrawShadowMapsQueryObject;






#ifdef QUERY_STAGES
char *stage_str[RENDERER_STAGE_COUNT];
float query_results[RENDERER_STAGE_COUNT];
#endif



int r_draw_group_count = 0;
draw_group_t *r_draw_groups = NULL;
static draw_command_t *r_draw_cmds = NULL;


int r_translucent_draw_group_count = 0;
draw_group_t *r_translucent_draw_groups = NULL;
static draw_command_t *r_translucent_draw_cmds = NULL;


unsigned int r_backbuffer_id = 0;
unsigned int r_backbuffer_color = 0;
unsigned int r_backbuffer_depth = 0;


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


mat4_t r_projection_matrix;
int r_projection_matrix_changed = 1;

mat4_t r_view_matrix;
int r_view_matrix_changed = 1;

mat4_t r_model_matrix;

mat4_t r_view_projection_matrix;
mat4_t r_model_view_matrix;
mat4_t r_model_view_projection_matrix;




int renderer_Init(int width, int height, int init_mode)
{
	char *ext_str;
	char *sub_str;
	
	int i;
	
	int w;
	int h;
	  
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		log_LogMessage(LOG_MESSAGE_ERROR, "renderer_Init: SDL didn't init!");
		return 0;
		//printf("oh shit...\n");
		//exit(1);
	}
	
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
			r_window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		
	/*	else
			r_window_flags |= SDL_WINDOW_MAXIMIZED;	*/
	}
	else
	{
		r_width = width;
		r_height = height;	
		r_window_width = width;
		r_window_height = height;
	}
	
	r_active_shader = -1;
	
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
	
	window = SDL_CreateWindow("wtf editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, r_width, r_height, r_window_flags);
	context = SDL_GL_CreateContext(window);
	
	SDL_GL_MakeCurrent(window, context);

	SDL_GL_SetSwapInterval(1);
	
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
	
	
	
	/*ext_str = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	printf("%s\n", ext_str);
	
	ext_str = (char *)glGetString(GL_VERSION);
	printf("%s\n", ext_str);
	
	ext_str = (char *)glGetString(GL_VENDOR);
	printf("%s\n", ext_str);*/
	
	//printf("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	//printf("OpenGL version: %s\n", glGetString(GL_VERSION));
	//printf("Vendor: %s\n", glGetString(GL_VENDOR));
	//printf("Renderer: %s\n", glGetString(GL_RENDERER));
	
	log_LogMessage(LOG_MESSAGE_NONE, "GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	log_LogMessage(LOG_MESSAGE_NONE, "OpenGL version: %s", glGetString(GL_VERSION));
	log_LogMessage(LOG_MESSAGE_NONE, "Vendor: %s", glGetString(GL_VENDOR));
	log_LogMessage(LOG_MESSAGE_NONE, "Renderer: %s", glGetString(GL_RENDERER));
	
	
	
	
	//ext_str = (char *)glGetString(GL_EXTENSIONS);
	
	/*sub_str = strstr(ext_str, "GL_ARB_seamless_cube_map");
	if(sub_str)
	{
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}
	
	
	if(!SDL_GL_ExtensionSupported("GL_ARB_uniform_buffer_object"))
	{
		if(!SDL_GL_ExtensionSupported("GL_EXT_uniform_buffer_object"))
		{
			log_LogMessage(LOG_MESSAGE_ERROR, "renderer_Init: extension GL_ARB_uniform_buffer_object not supported by current driver! Try to update it and then run the application again.");
			return 0;
		}
	}*/
	

	/*glGenQueries(1, &renderer_DrawWorldQueryObject);
	glGenQueries(1, &renderer_ZPrePassQueryObject);
	glGenqueries(1, &renderer_DrawShadowMapsQueryObject);*/
	
	
	renderer_Backbuffer(1366, 768, 1);
	
	
	
	r_draw_groups = malloc(sizeof(draw_group_t) * MAX_MATERIALS);
	r_draw_cmds = malloc(sizeof(draw_command_t) * MAX_MATERIALS * MAX_ENTITIES);
	
	assert(r_draw_groups);
	assert(r_draw_cmds);
		
	
	r_translucent_draw_groups = malloc(sizeof(draw_group_t) * MAX_MATERIALS);
	r_translucent_draw_cmds = malloc(sizeof(draw_command_t) * MAX_MATERIALS * MAX_ENTITIES);
		
	glGenQueries(1, &query_object);
	
	#ifdef QUERY_STAGES
	stage_str[RENDERER_DRAW_SHADOW_MAPS_STAGE] = "Shadow maps";
	stage_str[RENDERER_Z_PREPASS_STAGE] = "Z prepass";
	stage_str[RENDERER_DRAW_WORLD_STAGE] = "World";
	stage_str[RENDERER_SWAP_BUFFERS_STAGE] = "Swap";
	stage_str[RENDERER_BIND_LIGHT_CACHE] = "Light cache";
	stage_str[RENDERER_DRAW_FRAME] = "Draw frame";
	stage_str[RENDERER_DRAW_GUI] = "GUI";
	#endif

	return 1;
	
	
	
}

void renderer_Finish()
{	

	free(r_draw_cmds);
	free(r_draw_groups);
	
	free(r_translucent_draw_cmds);
	free(r_translucent_draw_groups);

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
		gl_handle = textures[texture_index].gl_handle;
	}
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gl_handle);
	
	renderer_SetUniform1i(UNIFORM_texture_sampler0, 0);
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
		gl_handle = textures[texture_index].gl_handle;
	}
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gl_handle);
	
	renderer_SetUniform1i(UNIFORM_texture_sampler1, 1);
}

void renderer_SetShadowTexture()
{
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shared_shadow_map);
	
	renderer_SetUniform1i(UNIFORM_texture_sampler2, 2);
}

void renderer_SetClusterTexture()
{
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, cluster_texture);
	
	renderer_SetUniform1i(UNIFORM_cluster_texture, 3);
}

void renderer_SetTexture(int texture_unit, int texture_target, int texture_index)
{
	int gl_texture;
	if(texture_index < 0)
	{
		gl_texture = 0;
	}
	else
	{
		gl_texture = textures[texture_index].gl_handle;
	}
	glActiveTexture(texture_unit);
	glBindTexture(texture_target, gl_texture);
}

void renderer_BindTexture(int texture_unit, int texture_target, int texture)
{
	glActiveTexture(texture_unit);
	glBindTexture(texture_target, texture);
}

/*void renderer_SetViewProjectionMatrix(float *matrix)
{
	renderer_SetUniformMatrix4fv(UNIFORM_view_projection_matrix, matrix);
}*/

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

void renderer_UpdateMatrices()
{
	if(r_view_matrix_changed || r_projection_matrix_changed)
	{
		mat4_t_mult_fast(&r_view_projection_matrix, &r_view_matrix, &r_projection_matrix);
		renderer_SetUniformMatrix4fv(UNIFORM_view_projection_matrix, &r_view_projection_matrix.floats[0][0]);
	}
	
	mat4_t_mult_fast(&r_model_view_projection_matrix, &r_model_matrix, &r_view_projection_matrix);
	
	renderer_SetUniformMatrix4fv(UNIFORM_model_matrix, &r_model_matrix.floats[0][0]);
	renderer_SetUniformMatrix4fv(UNIFORM_model_view_projection_matrix, &r_model_view_projection_matrix.floats[0][0]);
	
	r_view_matrix_changed = 0;
	r_projection_matrix_changed = 0;
}



void renderer_SetShader(int shader_index)
{
	shader_t *shader;
	unsigned int program;
	unsigned int diffuse_tex;
	unsigned int normal_tex;
	int i;
	
	if(shader_index == r_active_shader)
		return;
	
	if(shader_index < 0)
	{
		program = 0;
	}
	else
	{
		shader = &shaders[shader_index];
		program = shader->shader_program;
	}
	glUseProgram(program);
	r_active_shader = shader_index;	
	
	r_view_matrix_changed = 1;
	r_projection_matrix_changed = 1;
}

void renderer_SetUniform1i(int uniform, int value)
{
	shader_t *shader;
	
	if(r_active_shader < 0)
		return;
	
	shader = &shaders[r_active_shader];	
		
	if(uniform < UNIFORM_LAST_UNIFORM && uniform >= 0)
	{
		if(shader->uniforms[uniform].location != 0xffff)
		{
			glUniform1i(shader->uniforms[uniform].location, value);
		}
	}	
}

void renderer_SetUniform1ui(int uniform, int value)
{
	shader_t *shader;
	
	if(r_active_shader < 0)
		return;
	
	shader = &shaders[r_active_shader];	
		
	if(uniform < UNIFORM_LAST_UNIFORM && uniform >= 0)
	{
		if(shader->uniforms[uniform].location != 0xffff)
		{
			glUniform1ui(shader->uniforms[uniform].location, value);
		}
	}
}

void renderer_SetUniform4fv(int uniform, float *value)
{
	shader_t *shader;
	
	if(r_active_shader < 0)
		return;
	
	shader = &shaders[r_active_shader];	
		
	if(uniform < UNIFORM_LAST_UNIFORM && uniform >= 0)
	{
		if(shader->uniforms[uniform].location != 0xffff)
		{
			glUniform4fv(shader->uniforms[uniform].location, 1, value);
		}
		
	}	
}

void renderer_SetUniformMatrix4fv(int uniform, float *value)
{
	shader_t *shader;
	
	if(r_active_shader < 0)
		return;
	
	shader = &shaders[r_active_shader];	
		
	if(uniform < UNIFORM_LAST_UNIFORM && uniform >= 0)
	{
		if(shader->uniforms[uniform].location != 0xffff)
		{
			glUniformMatrix4fv(shader->uniforms[uniform].location, 1, GL_FALSE, value);
		}
		
	}
}



void renderer_SetVertexAttribPointer(int attrib, int size, int offset, int stride)
{
	shader_t *shader;
	
	if(r_active_shader < 0)
		return;
	
	shader = &shaders[r_active_shader];
	
	switch(attrib)
	{
		case VERTEX_ATTRIB_POSITION:	
			attrib = shader->vertex_position;	
		break;
		
		case VERTEX_ATTRIB_NORMAL:
			attrib = shader->vertex_normal;
		break;
		
		case VERTEX_ATTRIB_TANGENT:
			attrib = shader->vertex_tangent;
		break;
		
		case VERTEX_ATTRIB_TEX_COORDS:
			attrib = shader->vertex_tex_coords;
		break;
	}
	
	if(attrib == 255)
	{
		return;
	}
	
	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib, size, GL_FLOAT, GL_FALSE, stride, (void *)offset);
}


//extern material_t default_material;

void renderer_SetMaterial(int material_index)
{
	material_t *material;
	texture_t *texture;
	float color[4];
	int texture_flags = 0;
	
	if(material_index <= material_count)
	{
		material = &materials[material_index];
		
		if(material->flags & MATERIAL_INVALID)
			material = &materials[-1];
		
		color[0] = (float)material->r / 255.0;
		color[1] = (float)material->g / 255.0;
		color[2] = (float)material->b / 255.0;
		color[3] = (float)material->a / 255.0;
		
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
		glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 1024 * ((float)material->roughness / 255.0));
		
		if(material->diffuse_texture != -1)
		{
			renderer_SetDiffuseTexture(material->diffuse_texture);
			texture_flags |= MATERIAL_USE_DIFFUSE_TEXTURE;
		}
		
		if(material->normal_texture != -1)
		{
			renderer_SetNormalTexture(material->normal_texture);
			texture_flags |= MATERIAL_USE_NORMAL_TEXTURE;
			
			texture = &textures[material->normal_texture];
			
			
			if(texture->bm_flags & TEXTURE_INVERT_X)
			{
				texture_flags |= MATERIAL_INVERT_NORMAL_X;
			}
			
			if(texture->bm_flags & TEXTURE_INVERT_Y)
			{
				texture_flags |= MATERIAL_INVERT_NORMAL_Y;
			}
			
		}
		
		renderer_SetUniform1ui(UNIFORM_material_flags, texture_flags);
	}
}

void renderer_UpdateDrawGroups()
{
	int i;
	int group_index = 0;
	
	r_draw_group_count = 0;
	
	/*if(default_material.ref_count)
	{
		r_draw_groups[r_draw_group_count].material_index = -1;
		r_draw_groups[r_draw_group_count].draw_cmds_count = 0;
		r_draw_groups[r_draw_group_count].max_draw_cmds = default_material.ref_count;
		r_draw_groups[r_draw_group_count].draw_cmds = r_draw_cmds;
		r_draw_group_count++;
		
		default_material.draw_group = 0;
	}*/
	
	for(i = -1; i < material_count; i++)
	{
		
		
		if(materials[i].ref_count == 0)
			continue;
		
		r_draw_groups[r_draw_group_count].material_index = i;
		r_draw_groups[r_draw_group_count].draw_cmds_count = 0;
		r_draw_groups[r_draw_group_count].max_draw_cmds = materials[i].ref_count;
		r_draw_groups[r_draw_group_count].draw_cmds = r_draw_cmds;
		
		if(r_draw_group_count)
		{
			r_draw_groups[r_draw_group_count].draw_cmds = r_draw_groups[r_draw_group_count - 1].draw_cmds + r_draw_groups[r_draw_group_count - 1].max_draw_cmds;
		}
		
		materials[i].draw_group = r_draw_group_count;
		
		r_draw_group_count++;
	}
}

void renderer_SubmitDrawCommand(mat4_t *transform, unsigned short draw_mode, unsigned int vert_start, unsigned int vert_count, int material_index)
{
	draw_group_t *draw_group;
	draw_command_t *draw_command;
	material_t *material;
	
	/*if(material_index < 0)
	{
		material = &default_material;
	}
	else
	{
		material = &materials[material_index];
	}*/
	
	material = &materials[material_index];
	
	
	draw_group = &r_draw_groups[material->draw_group];
	
	if(draw_group->draw_cmds_count < draw_group->max_draw_cmds)
	{
		draw_command = draw_group->draw_cmds + draw_group->draw_cmds_count;
		draw_command->transform = transform;
		draw_command->draw_mode = draw_mode;
		draw_command->vert_start = vert_start;
		draw_command->vert_count = vert_count;
		draw_group->draw_cmds_count++;
	}
}


void renderer_SetWindowSize(int width, int height)
{
	
	SDL_DisplayMode display_mode;
	int i;
	
	if(width < 800 || width > 1920)
		return;
		
	if(height < 600 || height > 1080)
		return;	
		
	if(r_window_width != width || r_window_height != height)
	{
		r_window_width = width;
		r_window_height = height;
		
		SDL_SetWindowSize(window, r_window_width, r_window_height);
		SDL_GetDisplayMode(0, 0, &display_mode);
		
		SDL_SetWindowPosition(window, 0, 0);
		
		
		/*if(r_window_width == display_mode.w && r_window_height == display_mode.h)
		{
			r_window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
			SDL_SetWindowFullscreen(window, r_window_flags);
		}*/
		
		renderer_SetRendererResolution(r_window_width, r_window_height);
		
		for(i = 0; i < window_resize_callback_count; i++)
		{
			renderer_WindowResizeCallback[i]();
		}
	}	
}


void renderer_Backbuffer(int width, int height, int samples)
{
	int target;
	
	
	if(samples == r_msaa_samples && width == r_width && height == r_height)
	{
		if(r_backbuffer_id)
		{
			return;
		}
	}
		
		
		
	
	if(!r_backbuffer_id)
	{
		glGenFramebuffers(1, &r_backbuffer_id);
		glGenFramebuffers(1, &r_intensity_id);
		glGenFramebuffers(1, &r_intensity_half_id);
		glGenFramebuffers(1, &r_intensity_quarter_id);
		glGenFramebuffers(1, &r_intensity_eight_id);
	}
	else
	{
		glDeleteTextures(1, &r_backbuffer_color);
		glDeleteTextures(1, &r_backbuffer_depth);
		glDeleteTextures(1, &r_intensity_color);
		glDeleteTextures(1, &r_intensity_half_horizontal_color);
		glDeleteTextures(1, &r_intensity_half_vertical_color);
		glDeleteTextures(1, &r_intensity_quarter_horizontal_color);
		glDeleteTextures(1, &r_intensity_quarter_vertical_color);
		glDeleteTextures(1, &r_intensity_eight_horizontal_color);
		glDeleteTextures(1, &r_intensity_eight_vertical_color);
	}
	
	glGenTextures(1, &r_backbuffer_color);
	glGenTextures(1, &r_intensity_color);
	glGenTextures(1, &r_intensity_half_horizontal_color);
	glGenTextures(1, &r_intensity_half_vertical_color);
	glGenTextures(1, &r_intensity_quarter_horizontal_color);
	glGenTextures(1, &r_intensity_quarter_vertical_color);
	glGenTextures(1, &r_intensity_eight_horizontal_color);
	glGenTextures(1, &r_intensity_eight_vertical_color);
	glGenTextures(1, &r_backbuffer_depth);
	
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_backbuffer_id);
	
	r_width = width;
	r_height = height;
	r_msaa_samples = samples;
	
	if(r_width < RENDERER_MIN_WIDTH)
	{
		r_width = RENDERER_MIN_WIDTH;
	}
	else if(r_width > RENDERER_MAX_WIDTH)
	{
		r_width = RENDERER_MAX_WIDTH;
	}
	
	
	if(r_height < RENDERER_MIN_HEIGHT)
	{
		r_height = RENDERER_MIN_HEIGHT;
	}
	else if(r_height > RENDERER_MAX_HEIGHT)
	{
		r_height = RENDERER_MAX_HEIGHT;
	}
	
	
	if(r_msaa_samples < RENDERER_MIN_MSAA_SAMPLES)
	{
		r_msaa_samples = RENDERER_MIN_MSAA_SAMPLES;
	}
	else if(r_msaa_samples > RENDERER_MAX_MSAA_SAMPLES)
	{
		r_msaa_samples = RENDERER_MAX_MSAA_SAMPLES;
	}
	
	
	if(r_msaa_samples > 1)
	{
		target = GL_TEXTURE_2D_MULTISAMPLE;
		//glEnable(GL_MULTISAMPLE);
	}
	else
	{
		target = GL_TEXTURE_2D;
		//glDisable(GL_MULTISAMPLE);
	}
	glBindTexture(target, r_backbuffer_color);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	if(target == GL_TEXTURE_2D_MULTISAMPLE)
	{
		glTexImage2DMultisample(target, r_msaa_samples, GL_RGBA16F, r_width, r_height, GL_TRUE);
	}
	else
	{
		glTexImage2D(target, 0, GL_RGBA16F, r_width, r_height, 0, GL_RGBA, GL_FLOAT, NULL);
	}
	
	//glGenerateMipmap(GL_TEXTURE_2D);
	
	glBindTexture(target, r_backbuffer_depth);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);

	if(target == GL_TEXTURE_2D_MULTISAMPLE)
	{
		glTexImage2DMultisample(target, r_msaa_samples, GL_DEPTH24_STENCIL8, r_width, r_height, GL_TRUE);
	}
	else
	{
		glTexImage2D(target, 0, GL_DEPTH24_STENCIL8, r_width, r_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	}
	
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, r_backbuffer_color, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, r_backbuffer_depth, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, target, r_backbuffer_depth, 0);
	
	
	glBindTexture(target, r_intensity_color);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(target, 0, GL_RGBA16F, r_width, r_height, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_intensity_id);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, r_intensity_color, 0);
	
	
	glBindTexture(target, r_intensity_half_horizontal_color);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(target, 0, GL_RGBA16F, r_width >> 1, r_height >> 1, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindTexture(target, r_intensity_half_vertical_color);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(target, 0, GL_RGBA16F, r_width >> 1, r_height >> 1, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_intensity_half_id);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, r_intensity_half_horizontal_color, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, target, r_intensity_half_vertical_color, 0);
	

	glBindTexture(target, r_intensity_quarter_horizontal_color);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(target, 0, GL_RGBA16F, r_width >> 2, r_height >> 2, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindTexture(target, r_intensity_quarter_vertical_color);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(target, 0, GL_RGBA16F, r_width >> 2, r_height >> 2, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_intensity_quarter_id);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, r_intensity_quarter_horizontal_color, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, target, r_intensity_quarter_vertical_color, 0);
	
	
	
	glBindTexture(target, r_intensity_eight_horizontal_color);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(target, 0, GL_RGBA16F, r_width >> 3, r_height >> 3, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindTexture(target, r_intensity_eight_vertical_color);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(target, 0, GL_RGBA16F, r_width >> 3, r_height >> 3, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_intensity_eight_id);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, r_intensity_eight_horizontal_color, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, target, r_intensity_eight_vertical_color, 0);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


void renderer_BindBackbuffer()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_backbuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	if(r_msaa_samples > 1)
		glEnable(GL_MULTISAMPLE);
	else
		glDisable(GL_MULTISAMPLE);
		
	glViewport(0, 0, r_width, r_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
}


void renderer_SetRendererResolution(int width, int height)
{
	int i;
	
	if(width < 800 || width > 1920)
		return;
		
	if(height < 600 || height > 1080)
		return;	
	
	if(r_width != width || r_height != height)
	{
		r_width = width;
		r_height = height;
		
		glBindTexture(GL_TEXTURE_2D, albedo_buffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, r_width, r_height, 0, GL_RGBA, GL_BYTE, NULL);
		
		glBindTexture(GL_TEXTURE_2D, normal_buffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, r_width, r_height, 0, GL_RGBA, GL_FLOAT, NULL);
		
		glBindTexture(GL_TEXTURE_2D, depth_buffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, r_width, r_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, geometry_framebuffer);
		
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, albedo_buffer, 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_buffer, 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_buffer, 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_buffer, 0);
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		
		for(i = 0; i < renderer_resolution_change_callback_count; i++)
		{
			renderer_RendererResolutionChangeCallback[i]();
		}
		
		if(r_width > r_window_width || r_height > r_window_height)
		{
			renderer_SetWindowSize(r_width, r_height);
		}
			
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
		//renderer_SetRendererResolution(r_window_width, r_window_height);	
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
		renderer_Backbuffer(r_width, r_height, 4);
	}
	else
	{
		renderer_Backbuffer(r_width, r_height, 1);
	}
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


void renderer_OpenFrame()
{
	//renderer_BindBackbuffer();
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	renderer_UpdateDrawGroups();
}

void renderer_DrawFrame()
{
	
	
	#ifdef QUERY_STAGES
	renderer_BeginTimeElapsedQuery();
	#endif
	
	int i;
	
	float s;
	float e;

	light_BindCache();
	gpu_BindGpuHeap();

	if(r_draw_shadow_maps)
		renderer_DrawShadowMaps();

	renderer_BindBackbuffer();

	if(r_z_prepass)
		renderer_ZPrePass();		
	
	renderer_DrawWorld();
	
	renderer_DrawOpaque();
	renderer_DrawTranslucent();

	//renderer_DrawPlayers();

	renderer_DrawParticles();

	for(i = 0; i < pre_shading_render_function_count; i++)
	{
		renderer_PreShadingRegisteredFunction[i]();
	}
	

	if(r_bloom)
		renderer_DrawBloom();

	if(r_tonemap)
		renderer_Tonemap();
	else
		renderer_BlitBackbuffer();
	
	for(i = 0; i < post_shading_render_function_count; i++)
	{
		renderer_PostShadingRegisteredFunction[i]();
	}

	if(r_draw_gui)	
		renderer_DrawGUI();

	gpu_UnbindGpuHeap();
	light_UnbindCache();

	
	
	
	#ifdef QUERY_STAGES
	renderer_EndTimeElapsedQuery(RENDERER_DRAW_FRAME);
	#endif

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
	triangle_group_t *triangle_group;
	
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
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world_element_buffer);
	//shader_UseShader(z_pre_pass_shader);
	renderer_SetShader(z_pre_pass_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, (int)&((vertex_t *)0)->position, sizeof(vertex_t));
	
	renderer_SetProjectionMatrix(&active_camera->projection_matrix);
	renderer_SetViewMatrix(&active_camera->world_to_camera_matrix);
	renderer_SetModelMatrix(NULL);
	
	renderer_UpdateMatrices();
	
	
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_CULL_FACE);
	glStencilFunc(GL_ALWAYS, 0x1, 0xff);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	
	//glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	c = world_triangle_group_count;
	
	for(i = 0; i < c; i++)
	{
		triangle_group = &world_triangle_groups[i];
		glDrawElements(GL_TRIANGLES, triangle_group->next, GL_UNSIGNED_INT, (void *)(triangle_group->start * sizeof(int)));	
	}
	
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
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	
/*	#ifdef QUERY_STAGES
	renderer_EndTimeElapsedQuery(RENDERER_Z_PREPASS_STAGE);
	#endif*/
	
	/*#ifdef QUERY_STAGES
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectuiv(query0, GL_QUERY_RESULT, &elapsed);	
	query_results[0] = (float)elapsed / 1000000.0;
	#endif*/
	
	
}

void renderer_BlitBackbuffer()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, r_backbuffer_id);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_window_width, r_window_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_window_width, r_window_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_window_width, r_window_height, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	
	glViewport(0, 0, r_window_width, r_window_height);
}

void renderer_DrawShadowMaps()
{
	camera_t *active_camera = camera_GetActiveCamera();	
	triangle_group_t *triangle_group;
	material_t *material;
	light_position_t *pos;
	light_params_t *parms;
	int i;
	int c = brush_count;
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
	
	if(!world_leaves)
		return;
	
	
	/*#ifdef QUERY_STAGE
	renderer_BeginTimeElapsedQuery();
	#endif*/
	
	
	//gpu_BindGpuHeap();
	//shader_UseShader(shadow_pass_shader);
	
	//s = engine_GetDeltaTime();
	
	renderer_SetShader(shadow_pass_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, (int)&((vertex_t *)0)->position, sizeof(vertex_t));
	
	
	/*e = engine_GetDeltaTime();
	
	
	printf("[1: %f] ", e - s);
	
	
	s = e;*/
	
	//shader_SetCurrentShaderUniform1i(UNIFORM_LIGHT_COUNT, visible_light_count);
	//shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_FLAGS, 0);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&shadow_map_projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	/*e = engine_GetDeltaTime();
	
	printf("[2: %f] ", e - s);
	
	
	s = e;*/

	//while(glGetError() != GL_NO_ERROR);	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadow_map_frame_buffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	
/*	e = engine_GetDeltaTime();
	
	printf("[3: %f] ", e - s);
	
	s = e;*/
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_cache_shadow_element_buffer);
	
	glClearColor(LIGHT_MAX_RADIUS, LIGHT_MAX_RADIUS, LIGHT_MAX_RADIUS, LIGHT_MAX_RADIUS);

	/*e = engine_GetDeltaTime();
	
	
	printf("[4: %f] ", e - s);
	
	s = e;*/
	
	k = visible_light_count;
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
		light_index = visible_lights[j];
		pos = &light_positions[light_index];
		parms = &light_params[light_index];
		
		if(!(parms->bm_flags & LIGHT_GENERATE_SHADOWS))
			continue;
		
		if(!(parms->bm_flags & LIGHT_UPDATE_SHADOW_MAP))
			continue;
		
		if(parms->bm_flags & LIGHT_DROPPED_SHADOW)
			continue;	
		
		parms->bm_flags &= ~LIGHT_UPDATE_SHADOW_MAP;
		
		
		light_SetLight(light_index);
		cache_index = light_params[light_index].cache;
		start = light_cache[cache_index].offset * MAX_INDEXES_PER_FRUSTUM * 6;
		next = light_cache_frustum_counts + light_cache[cache_index].offset * 6;
		//shadow_map_index = light_cache[cache_index].offset;
		
		start_x = parms->x;
		start_y = parms->y;
		
		for(i = 0; i < 6; i++)
		{				
			mvm = shadow_map_mats[i];
			
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
				else if(w->bm_flags & WIDGET_MOUSE_OVER)
				{
					glColor3f(0.5, 0.5, 0.5);
				}
				else
				{
					glColor3f(0.4, 0.4, 0.4);
				}
				//}
				
				
				glBegin(GL_QUADS);
				
						
				//glRectf(w->x + w->w, w->y - w->h, w->x - w->w, w->y + w->h);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
				
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
	
				glBegin(GL_QUADS);
				
				glColor3f(0.4, 0.4, 0.4);
				
				
				//glRectf(w->x + w->w, w->y - w->h, w->x - w->w, w->y + w->h);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
				
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
				glBegin(GL_QUADS);
				
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
				
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0 ,0.0);
				glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				
				renderer_BlitSurface(button->rendered_text, w->x + x - w->w + r_window_width * 0.5 + 1, w->y + y + w->h + r_window_height * 0.5 - 2);
				
			break;
			
			case WIDGET_SLIDER:
				slider = (slider_t *)w;
				
				glBegin(GL_QUADS);
				
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
				glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
				glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
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
				glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glLineWidth(1.0);
				
				if(checkbox->bm_checkbox_flags & CHECKBOX_CHECKED)
				{
					glColor3f(0.9, 0.9, 0.9);
					glBegin(GL_QUADS);
					glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
					glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
					glEnd();
				}
			break;
			
			case WIDGET_DROPDOWN:
				dropdown = (dropdown_t *)w;
				
				
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
				
				glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
				
				glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				
				renderer_BlitSurface(dropdown->rendered_text, w->x + x - w->w + r_window_width * 0.5 + 1, w->y + y + w->h + r_window_height * 0.5 - 2);
				
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
			
			case WIDGET_OPTION:
				option = (option_t *)w;
				options = (option_list_t *)w->parent;
				
				
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
				
				
				glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
				
				if(!(options->bm_option_list_flags & OPTION_LIST_NO_OPTION_DIVISIONS))
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glColor3f(0.0, 0.0, 0.0);
					glBegin(GL_QUADS);
					glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
					glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
					glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
					glEnd();
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				
				
				
				renderer_BlitSurface(option->rendered_text, w->x + x - w->w + r_window_width * 0.5 + 1, w->y + y + w->h + r_window_height * 0.5 - 2);
				//draw_DrawString(ui_font, 16, (button->swidget.x + x - hw) + renderer.screen_width * 0.5 + 1,  (button->swidget.y + y + hh) + renderer.screen_height * 0.5 + 1, 500, vec3(1.0, 1.0, 1.0), button->swidget.name);
				
			
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
					
					//if(w->bm_flags & WIDGET_MOUSE_OVER)
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
				
			break;
				
			case WIDGET_BAR:
				bar = (widget_bar_t *)w;
					
					
				
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
					
				glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
				
				if(w->nestled)
				{
					x += w->x;
					y += w->y;
					
					widget_stack_top++;
					widget_stack[widget_stack_top] = w;
					w = w->last_nestled;
					continue;
				}
					
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
				
				if(w->bm_flags & WIDGET_MOUSE_OVER)
				{
					glColor3f(0.4, 0.4, 0.4);
				}
				else
				{
					glColor3f(0.3, 0.3, 0.3);
				}
				
				
				
				glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
				
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
				
				
				renderer_BlitSurface(field->rendered_text, w->x + x - w->w + r_window_width * 0.5 + 1, w->y + y + w->h + r_window_height * 0.5 - 2);
				
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
				w = widget_stack[widget_stack_top];
				x -= w->x;
				y -= w->y;
				widget_stack_top--;
				goto _advance_widget;
			}
		}
		
	}
	
	
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
	
	
	
/*	#ifdef QUERY_STAGES
	renderer_EndTimeElapsedQuery(RENDERER_DRAW_GUI);
	#endif*/
	
	
}

/*void renderer_DrawCollisionGeometry()
{
	
	int i;
	int c = collision_geometry_vertice_count;
	camera_t *active_camera = camera_GetActiveCamera();
	
	glUseProgram(0);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_CULL_FACE);
	glLineWidth(2.0);
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 0.0, 0.0);
	for(i = 0; i < c;)
	{
		glVertex3f(collision_geometry_positions[i].x, collision_geometry_positions[i].y, collision_geometry_positions[i].z);
		i++;
		glVertex3f(collision_geometry_positions[i].x, collision_geometry_positions[i].y, collision_geometry_positions[i].z);
		i++;
		glVertex3f(collision_geometry_positions[i].x, collision_geometry_positions[i].y, collision_geometry_positions[i].z);
		i++;
	}
	glEnd();
	
	
	glBegin(GL_LINES);
	glColor3f(0.0, 1.0, 0.0);
	for(i = 0; i < c;)
	{
		glVertex3f(collision_geometry_positions[i].x, collision_geometry_positions[i].y, collision_geometry_positions[i].z);
		glVertex3f(collision_geometry_positions[i].x + collision_geometry_normals[i].x, 
				   collision_geometry_positions[i].y + collision_geometry_normals[i].y, 
				   collision_geometry_positions[i].z + collision_geometry_normals[i].z);
		i++;
		
		glVertex3f(collision_geometry_positions[i].x, collision_geometry_positions[i].y, collision_geometry_positions[i].z);
		glVertex3f(collision_geometry_positions[i].x + collision_geometry_normals[i].x, 
				   collision_geometry_positions[i].y + collision_geometry_normals[i].y, 
				   collision_geometry_positions[i].z + collision_geometry_normals[i].z);
		i++;
		
		glVertex3f(collision_geometry_positions[i].x, collision_geometry_positions[i].y, collision_geometry_positions[i].z);
		glVertex3f(collision_geometry_positions[i].x + collision_geometry_normals[i].x, 
				   collision_geometry_positions[i].y + collision_geometry_normals[i].y, 
				   collision_geometry_positions[i].z + collision_geometry_normals[i].z);
		i++;
		
	}
	glEnd();
	
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glLineWidth(1.0);
	//gpu_BindGpuHeap();
	
	//glDrawArrays(GL_TRIANGLES, collision_geometry_start, collision_geometry_count);
	
	
	//gpu_UnbindGpuHeap();
}*/



void renderer_DrawPlayers()
{
	
	int i;
	int c = player_count;
	
	camera_t *active_camera = camera_GetActiveCamera();
	
	glUseProgram(0);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
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
	int i;
	int c = player_count;
	camera_t *active_camera = camera_GetActiveCamera();
	float color[] = {0.0, 0.0, 1.0, 1.0};
	
	mat4_t transform;
	mat4_t camera_transform;
	mat4_t weapon_transform;
	mat4_t model_view_matrix;	
	mat3_t weapon_rot = mat3_t_id();
	vec3_t weapon_position;
	
	
	if(!active_player) return;
	
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

/*void renderer_DrawBlocks()
{
	

}

void renderer_DrawBVH()
{
	
	
}*/

/*void renderer_CullWorld()
{
	
}*/

void renderer_DrawOpaque()
{
	camera_t *active_camera = camera_GetActiveCamera();
	mat4_t view_projection_matrix;
	mat4_t model_matrix;
	int i;
	int j;
	int draw_cmd_count;
	unsigned int draw_mode;
	unsigned int vert_start;
	unsigned int vert_count;
	
	draw_command_t *draw_cmds;
	
	//mat4_t_mult_fast(&view_projection_matrix, &active_camera->world_to_camera_matrix, &active_camera->projection_matrix);
	
	  
	/*glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&view_projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);*/
	
	renderer_SetShader(forward_pass_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, (int)&(((vertex_t *)0)->position), sizeof(vertex_t));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 3, (int)&(((vertex_t *)0)->normal), sizeof(vertex_t));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TANGENT, 3, (int)&(((vertex_t *)0)->tangent), sizeof(vertex_t));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 2, (int)&(((vertex_t *)0)->tex_coord), sizeof(vertex_t));
	
	renderer_SetProjectionMatrix(&active_camera->projection_matrix);
	renderer_SetViewMatrix(&active_camera->world_to_camera_matrix);
	
	for(i = 0; i < r_draw_group_count; i++)
	{
		renderer_SetMaterial(r_draw_groups[i].material_index);
		
		draw_cmd_count = r_draw_groups[i].draw_cmds_count;
		draw_cmds = r_draw_groups[i].draw_cmds;
		
		for(j = 0; j < draw_cmd_count; j++)
		{		
			//glLoadMatrixf(&draw_cmds[j].transform->floats[0][0]);
			renderer_SetModelMatrix(draw_cmds[j].transform);
			renderer_UpdateMatrices();

			glDrawArrays(draw_cmds[j].draw_mode, draw_cmds[j].vert_start, draw_cmds[j].vert_count);
		}
		
	}

	/*glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);*/
	
}

void renderer_DrawTranslucent()
{
	
}

void renderer_DrawWorld()
{
	
	
	camera_t *active_camera = camera_GetActiveCamera();	
	triangle_group_t *triangle_group;
	material_t *material;
	light_position_t *pos;
	light_params_t *parms;
	mat4_t camera_to_world_matrix;
	mat4_t camera_to_light_matrix;
	mat4_t view_projection_matrix;
	mat4_t mat;
	int i;
	int c = brush_count;
	
	static unsigned int query0;
	static unsigned int query0_state = 0;
	
	static unsigned int query1;
	static unsigned int query1_state = 0;
	
	int j;
	int k;
	int light_index;
	
	unsigned int elapsed;
	float ms_elapsed;
	
	int l;
	int m;
	
	int *next;
	int start;
	int cache_index;
	
	int *r;
	
	float color[] = {1.0, 1.0, 1.0, 1.0};
	
	
/*	if(!query0_state)
	{
		query0_state = 1;
		glGenQueries(1, &query0);
	}
	
	if(!query1_state)
	{
		query1_state = 1;
		glGenQueries(1, &query1);
	}*/
	
	
	/*#ifdef QUERY_STAGES
	renderer_BeginTimeElapsedQuery();
	#endif*/
	
	//glEnable(GL_STENCIL_TEST);
	//glStencilFunc(GL_ALWAYS, 0x1, 0xff);
	//glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
	//glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	//glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
	glViewport(0, 0, r_width, r_height);
	//glClear(GL_STENCIL_BUFFER_BIT);
	
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);
	//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	//glDepthMask(GL_FALSE);
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	
	//mat4_t_mult_fast(&view_projection_matrix, &active_camera->world_to_camera_matrix, &active_camera->projection_matrix);
	
	/*glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&view_projection_matrix.floats[0][0]);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();*/
	
	
	
	//glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	//renderer_SetShader(stencil_lights_pass_shader);
	//renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, 0, 0);
	//shader_UseShader(stencil_lights_pass_shader);
	//shader_SetCurrentShaderVertexAttribPointer(ATTRIB_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	c = visible_light_count;
		
	/*for(i = 0; i < c; i++)
	{
		light_index = visible_lights[i];
		light_SetLight(light_index);
		glDrawArrays(GL_TRIANGLES, stencil_light_mesh_start, stencil_light_mesh_vert_count);		
	}*/
	

	renderer_SetShader(forward_pass_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, (int)&((vertex_t *)0)->position, sizeof(vertex_t));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 3, (int)&((vertex_t *)0)->normal, sizeof(vertex_t));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TANGENT, 3, (int)&((vertex_t *)0)->tangent, sizeof(vertex_t));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 2, (int)&((vertex_t *)0)->tex_coord, sizeof(vertex_t));
	
	renderer_SetShadowTexture();
	renderer_SetClusterTexture();
	
	renderer_SetUniform1i(UNIFORM_r_width, r_width);
	renderer_SetUniform1i(UNIFORM_r_height, r_height);
	renderer_SetUniform1i(UNIFORM_r_frame, r_frame);
	renderer_SetUniform4fv(UNIFORM_active_camera_position, &active_camera->world_position.floats[0]);
	
	
	renderer_SetProjectionMatrix(&active_camera->projection_matrix);
	renderer_SetViewMatrix(&active_camera->world_to_camera_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_UpdateMatrices();

		
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world_element_buffer);
	c = world_triangle_group_count;
	
	k = visible_light_count;
	glEnable(GL_CULL_FACE);
	glStencilFunc(GL_EQUAL, 0x1, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	for(i = 0; i < c; i++)
	{				
		triangle_group = &world_triangle_groups[i];		
		renderer_SetMaterial(triangle_group->material_index);
		glDrawElements(GL_TRIANGLES, triangle_group->next, GL_UNSIGNED_INT, (void *)(triangle_group->start * sizeof(int)));
	}
	
	
	//glDisable(GL_STENCIL_TEST);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
/*	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);*/
	
	
/*	#ifdef QUERY_STAGES
	renderer_EndTimeElapsedQuery(RENDERER_DRAW_WORLD_STAGE);
	#endif*/
}


void renderer_DrawBloom()
{
	renderer_SetShader(bloom0_shader);	
	renderer_BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, r_backbuffer_color);
	
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_intensity_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glBegin(GL_QUADS);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0, -0.5);
	glVertex3f(-1.0, -1.0, -0.5);
	glVertex3f(1.0, -1.0, -0.5);
	glVertex3f(1.0, 1.0, -0.5);
	glEnd();
	
	/*############################################################*/
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_intensity_half_id);
	glViewport(0, 0, r_width >> 1, r_height >> 1);
	
	renderer_SetShader(bloom1_shader);
	renderer_BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, r_intensity_color);
	renderer_SetUniform1i(UNIFORM_r_width, r_width >> 1);
	renderer_SetUniform1i(UNIFORM_r_height, r_height >> 1);
	renderer_SetUniform1i(UNIFORM_r_bloom_radius, 2);
	
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	glBegin(GL_QUADS);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0, -0.5);
	glVertex3f(-1.0, -1.0, -0.5);
	glVertex3f(1.0, -1.0, -0.5);
	glVertex3f(1.0, 1.0, -0.5);
	glEnd();
	
	renderer_SetUniform1i(UNIFORM_r_width, 0);
	renderer_BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, r_intensity_half_horizontal_color);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	
	glBegin(GL_QUADS);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0, -0.5);
	glVertex3f(-1.0, -1.0, -0.5);
	glVertex3f(1.0, -1.0, -0.5);
	glVertex3f(1.0, 1.0, -0.5);
	glEnd();
	
	/*############################################################*/
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_intensity_quarter_id);
	glViewport(0, 0, r_width >> 2, r_height >> 2);
	
	renderer_BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, r_intensity_half_vertical_color);
	renderer_SetUniform1i(UNIFORM_r_width, r_width >> 2);
	renderer_SetUniform1i(UNIFORM_r_height, r_height >> 2);
	renderer_SetUniform1i(UNIFORM_r_bloom_radius, 2);
	
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	glBegin(GL_QUADS);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0, -0.5);
	glVertex3f(-1.0, -1.0, -0.5);
	glVertex3f(1.0, -1.0, -0.5);
	glVertex3f(1.0, 1.0, -0.5);
	glEnd();
	
	renderer_SetUniform1i(UNIFORM_r_width, 0);
	renderer_BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, r_intensity_quarter_horizontal_color);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	
	glBegin(GL_QUADS);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0, -0.5);
	glVertex3f(-1.0, -1.0, -0.5);
	glVertex3f(1.0, -1.0, -0.5);
	glVertex3f(1.0, 1.0, -0.5);
	glEnd();
	
	/*############################################################*/
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_intensity_eight_id);
	glViewport(0, 0, r_width >> 3, r_height >> 3);
	glClear(GL_COLOR_BUFFER_BIT);
	
	renderer_SetShader(bloom1_shader);
	renderer_BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, r_intensity_quarter_vertical_color);
	renderer_SetUniform1i(UNIFORM_r_width, r_width >> 3);
	renderer_SetUniform1i(UNIFORM_r_height, r_height >> 3);
	renderer_SetUniform1i(UNIFORM_r_bloom_radius, 8);
	
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	glBegin(GL_QUADS);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0, -0.5);
	glVertex3f(-1.0, -1.0, -0.5);
	glVertex3f(1.0, -1.0, -0.5);
	glVertex3f(1.0, 1.0, -0.5);
	glEnd();
	
	renderer_SetUniform1i(UNIFORM_r_width, 0);
	renderer_BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, r_intensity_eight_horizontal_color);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	
	glBegin(GL_QUADS);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0, -0.5);
	glVertex3f(-1.0, -1.0, -0.5);
	glVertex3f(1.0, -1.0, -0.5);
	glVertex3f(1.0, 1.0, -0.5);
	glEnd();
	
	/*############################################################*/
	
	
	
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	renderer_BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, r_intensity_eight_vertical_color);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r_backbuffer_id);
	glViewport(0, 0, r_width, r_height);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	glEnable(GL_BLEND);

	glBegin(GL_QUADS);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0, -0.5);
	glVertex3f(-1.0, -1.0, -0.5);
	glVertex3f(1.0, -1.0, -0.5);
	glVertex3f(1.0, 1.0, -0.5);
	glEnd();
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
		
	renderer_BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, 0);
	
	
}


void renderer_Tonemap()
{
	
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, r_backbuffer_id);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	
	glViewport(0, 0, r_window_width, r_window_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_window_width, r_window_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_window_width, r_window_height, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	
	renderer_SetShader(tonemap_shader);
	renderer_BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, r_backbuffer_color);
	renderer_SetUniform1i(UNIFORM_texture_sampler0, 0);
	
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	
	glBegin(GL_QUADS);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0, -0.5);
	glVertex3f(-1.0, -1.0, -0.5);
	glVertex3f(1.0, -1.0, -0.5);
	glVertex3f(1.0, 1.0, -0.5);
	glEnd();
	
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	
	
	
	
	
	
	//glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_window_width, r_window_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	
	
	
	
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

void renderer_DrawParticles()
{
	
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
	glDisable(GL_MULTISAMPLE);
	SDL_GL_SwapWindow(window);
	r_frame++;
	
	//#ifdef QUERY_STAGES
	//renderer_EndTimeElapsedQuery(RENDERER_SWAP_BUFFERS_STAGE);
	//renderer_ReportQueryResults();
//	#endif
	
	
	//e = engine_GetDeltaTime();
	
	//printf("%f\n", e - s);
}


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


void renderer_GetWindowSize(int *w, int *h)
{
	*w = r_width;
	*h = r_height;
}


char *renderer_GetGLEnumString(int name)
{
	switch(name)
	{
		case GL_LINEAR: return "GL_LINEAR";
		case GL_NEAREST: return "GL_NEAREST";
		case GL_LINEAR_MIPMAP_LINEAR: return "GL_LINEAR_MIPMAP_LINEAR";
		case GL_NEAREST_MIPMAP_LINEAR: return "GL_NEAREST_MIPMAP_LINEAR";
		case GL_NEAREST_MIPMAP_NEAREST: return "GL_NEAREST_MIPMAP_NEAREST";
		case GL_CLAMP: return "GL_CLAMP";
		case GL_REPEAT: return "GL_REPEAT";
		case GL_TEXTURE_2D: return "GL_TEXTURE_2D";
		
		default: return "unkown";
	}
}








