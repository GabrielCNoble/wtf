#include <stdio.h>
#include <string.h>

#include "r_common.h"
#include "r_main.h"
#include "r_text.h"


#include "camera.h"
#include "player.h"
#include "shader.h"
//#include "physics.h"
#include "mesh.h"
#include "world.h"
#include "material.h"
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


int r_width;
int r_height;
int r_window_width;
int r_window_height;
int r_window_flags;
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


int r_draw_shadow_maps = 0;
int r_z_prepass = 0;
int r_query_stages = 1;



#ifdef QUERY_STAGES
char *stage_str[RENDERER_STAGE_COUNT];
float query_results[RENDERER_STAGE_COUNT];
#endif




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
	
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
	
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
	
	sub_str = strstr(ext_str, "GL_ARB_seamless_cube_map");
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
	}
	
	
	/*glGenQueries(1, &renderer_DrawWorldQueryObject);
	glGenQueries(1, &renderer_ZPrePassQueryObject);
	glGenqueries(1, &renderer_DrawShadowMapsQueryObject);*/
	
	glGenQueries(1, &query_object);
	
	
		
	/*glGenTextures(1, &albedo_buffer);
	glBindTexture(GL_TEXTURE_2D, albedo_buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, r_width, r_height, 0, GL_RGBA, GL_BYTE, NULL);
	
	glGenTextures(1, &normal_buffer);
	glBindTexture(GL_TEXTURE_2D, normal_buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, r_width, r_height, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glGenTextures(1, &depth_buffer);
	glBindTexture(GL_TEXTURE_2D, depth_buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, r_width, r_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	
	glGenFramebuffers(1, &geometry_framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, geometry_framebuffer);
	
	
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, albedo_buffer, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_buffer, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_buffer, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_buffer, 0);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
	glGenQueries(1, &query_object);*/
	
	
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
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
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
		//r_window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		SDL_SetWindowSize(window, r_window_width, r_window_height);
		//SDL_SetWindowFullscreen(window, r_window_flags);
		renderer_SetRendererResolution(r_window_width, r_window_height);	
	}
	else
	{
		if(!(r_window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP))
			return;
		
		r_window_flags &= ~SDL_WINDOW_FULLSCREEN_DESKTOP;
		
		SDL_SetWindowSize(window, r_width, r_height);
		
		renderer_SetRendererResolution(r_width, r_height);
		
		r_window_width = r_width;
		r_window_height = r_height;
		
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
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void renderer_DrawFrame()
{
	
	
	#ifdef QUERY_STAGES
	renderer_BeginTimeElapsedQuery();
	#endif
	
	int i;
	
	float s;
	float e;
	
	
	s = engine_GetDeltaTime();
	
	light_BindCache();
	gpu_BindGpuHeap();

	
	if(r_draw_shadow_maps)
		renderer_DrawShadowMaps();
		
	if(r_z_prepass)
		renderer_ZPrePass();
	//renderer_DrawSkyBox();
	renderer_DrawWorld();
	renderer_DrawPlayers();
	renderer_DrawParticles();
	
	
	//renderer_DrawActivePlayer();
	//renderer_DrawPlayers();
	/*renderer_DrawWorldDeferred();	

	for(i = 0; i < pre_shading_render_function_count; i++)
	{
		renderer_PreShadingRegisteredFunction[i]();
	}*/
	
	
	//renderer_Shade();
	//renderer_DrawSkyBox();
	
	
	for(i = 0; i < post_shading_render_function_count; i++)
	{
		renderer_PostShadingRegisteredFunction[i]();
	}
		
	renderer_DrawGUI();
	gpu_UnbindGpuHeap();
	light_UnbindCache();
	
	#ifdef QUERY_STAGES
	renderer_EndTimeElapsedQuery(RENDERER_DRAW_FRAME);
	#endif
	
	
	e = engine_GetDeltaTime();
	
	
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
	triangle_group_t *triangle_group;
	
	float color[4];
	
	
	
	/*#ifdef QUERY_STAGES
	unsigned int elapsed;
	glBeginQuery(GL_TIME_ELAPSED, renderer_ZPrePassQueryObject);
	#endif*/
	
	/*#ifdef QUERY_STAGES
	renderer_BeginTimeElapsedQuery();
	#endif*/
	
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	
	//glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world_element_buffer);
	shader_UseShader(z_pre_pass_shader);
	
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_CULL_FACE);
	glStencilFunc(GL_ALWAYS, 0x1, 0xff);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	c = world_triangle_group_count;
	
	for(i = 0; i < c; i++)
	{
	
		triangle_group = &world_triangle_groups[i];
		/*material = &materials[triangle_group->material_index];
			
		color[0] = (float)material->r / 255.0;
		color[1] = (float)material->g / 255.0;
		color[2] = (float)material->b / 255.0;
		color[3] = (float)material->a / 255.0;
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);*/
		glDrawElements(GL_TRIANGLES, triangle_group->next, GL_UNSIGNED_INT, (void *)(triangle_group->start * sizeof(int)));	
	
		//triangle_group = &world_triangle_groups[i];
		//glDrawElements(GL_TRIANGLES, triangle_group->next, GL_UNSIGNED_INT, (void *)(triangle_group->start * sizeof(int)));
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
	
	int *r;
	
	float color[] = {1.0, 1.0, 1.0, 1.0};
	
	if(!world_leaves)
		return;
	
	
	/*#ifdef QUERY_STAGE
	renderer_BeginTimeElapsedQuery();
	#endif*/
	
	
	//gpu_BindGpuHeap();
	shader_UseShader(shadow_pass_shader);
	//shader_SetCurrentShaderUniform1i(UNIFORM_LIGHT_COUNT, visible_light_count);
	//shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_FLAGS, 0);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&shadow_map_projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	//while(glGetError() != GL_NO_ERROR);	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadow_map_frame_buffer);
	//printf("==%x\n", glGetError());
	//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shared_shadow_map, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_cache_shadow_element_buffer);
	
	
	//glViewport(0, 0, SHARED_SHADOW_MAP_WIDTH, SHARED_SHADOW_MAP_HEIGHT);
	glClearColor(LIGHT_MAX_RADIUS, LIGHT_MAX_RADIUS, LIGHT_MAX_RADIUS, LIGHT_MAX_RADIUS);
	//glClear(GL_COLOR_BUFFER_BIT);
	
	k = visible_light_count;
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_SCISSOR_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_MIN);

	

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
	
	glDepthMask(GL_TRUE);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	glViewport(0, 0, r_window_width, r_window_height);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
/*	
	#ifdef QUERY_STAGES
	renderer_EndTimeElapsedQuery(RENDERER_DRAW_SHADOW_MAPS_STAGE);
	#endif*/
	
}



void renderer_DrawGUI()
{
	widget_t *w;
	
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
	
	short x = 0;
	short y = 0;
	
	
/*	#ifdef QUERY_STAGES
	renderer_BeginTimeElapsedQuery();
	#endif*/
	
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&gui_projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glUseProgram(0);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	
	
	
	
	w = last_widget;
	
	_do_top:
	
	while(w)
	{
				
		if(w->bm_flags & WIDGET_INVISIBLE)
			goto _advance_widget;
		
		/*if(w->bm_flags & WIDGET_IGNORE_EDGE_CLIPPING)
			glDisable(GL_STENCIL_TEST);
		else
			glEnable(GL_STENCIL_TEST);*/
		
		switch(w->type)
		{
			case WIDGET_NONE:
				glBegin(GL_QUADS);
				if(w->bm_flags & WIDGET_MOUSE_OVER)
				{
					glColor3f(0.5, 0.5, 0.5);
				}
				else
				{
					glColor3f(0.4, 0.4, 0.4);
				}
						
				//glRectf(w->x + w->w, w->y - w->h, w->x - w->w, w->y + w->h);
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
					glColor3f(0.5, 0.5, 0.5);
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
			
			break;
			
			case WIDGET_OPTION:
				option = (option_t *)w;
				options = (option_list_t *)w->parent;
				
				//if(w->bm_flags & WIDGET_MOUSE_OVER)
				if(option == (option_t *)options->active_option)
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
				
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor3f(0.0, 0.0, 0.0);
				glBegin(GL_QUADS);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				
				renderer_BlitSurface(option->rendered_text, w->x + x - w->w + r_window_width * 0.5 + 1, w->y + y + w->h + r_window_height * 0.5 - 2);
				//draw_DrawString(ui_font, 16, (button->swidget.x + x - hw) + renderer.screen_width * 0.5 + 1,  (button->swidget.y + y + hh) + renderer.screen_height * 0.5 + 1, 500, vec3(1.0, 1.0, 1.0), button->swidget.name);
				
				if(w->nestled)
				{
					if(option == (option_t *)options->active_option)
					{
						glColor3f(1.0, 1.0, 1.0);
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
	
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
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
	glDepthMask(GL_TRUE);
			
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

void renderer_DrawWorld()
{
	
	
	camera_t *active_camera = camera_GetActiveCamera();	
	triangle_group_t *triangle_group;
	material_t *material;
	light_position_t *pos;
	light_params_t *parms;
	mat4_t camera_to_world_matrix;
	mat4_t camera_to_light_matrix;
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
	glViewport(0, 0, r_window_width, r_window_height);
	//glClear(GL_STENCIL_BUFFER_BIT);
	
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);
	//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	//glDepthMask(GL_FALSE);
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	/*shader_UseShader(stencil_lights_pass_shader);
	shader_SetCurrentShaderVertexAttribPointer(ATTRIB_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	c = visible_light_count;
		
	for(i = 0; i < c; i++)
	{
		light_index = visible_lights[i];
		light_SetLight(light_index);
		glDrawArrays(GL_TRIANGLES, stencil_light_mesh_start, stencil_light_mesh_vert_count);		
	}*/
	

	

	
	
	
	
	//gpu_BindGpuHeap();
	shader_UseShader(forward_pass_shader);
	//shader_SetCurrentShaderUniform1i(UNIFORM_LIGHT_COUNT, visible_light_count);
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_FLAGS, 0);
	shader_SetCurrentShaderUniform4fv(UNIFORM_ACTIVE_CAMERA_POSITION, &active_camera->world_position.floats[0]);
	
	
	
	//while(glGetError() != GL_NO_ERROR);
		
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world_element_buffer);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_cache_element_buffer);
	c = world_triangle_group_count;
	
	k = visible_light_count;
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glDisable(GL_BLEND);
	//glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0x1, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	//glDepthMask(GL_TRUE);
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//glBlendFunc(GL_ONE, GL_ONE);
	
	
//	mat4_t_compose(&camera_to_world_matrix, &active_camera->world_orientation, active_camera->world_position);
	
	
	/*for(j = 0; j < k; j++)
	{
		light_index = visible_lights[j];
		light_SetLight(light_index);
		cache_index = light_params[light_index].cache;
		start = light_cache[cache_index].offset * MAX_INDEXES_PER_GROUP * world_triangle_group_count;
		next = light_cache_groups_next + light_cache[cache_index].offset * world_triangle_group_count;
		
		pos = &light_positions[light_index];
		parms = &light_params[light_index];
		
		if(parms->bm_flags & LIGHT_GENERATE_SHADOWS)
		{
			
			mat = mat4_t_id();
			mat.floats[3][0] = -pos->position.x;
			mat.floats[3][1] = -pos->position.y;
			mat.floats[3][2] = -pos->position.z;
			
			mat4_t_mult_fast(&camera_to_light_matrix, &camera_to_world_matrix, &mat);
			shader_SetCurrentShaderUniformMatrix4f(UNIFORM_CAMERA_TO_LIGHT_MATRIX, &camera_to_light_matrix.floats[0][0]);
			shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_CUBE_SAMPLER0, 5);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, light_cache_shadow_maps[light_cache[cache_index].offset]);
			glActiveTexture(GL_TEXTURE0);
		}	*/
	
	
	//glBeginQuery(GL_TIME_ELAPSED, query1);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shared_shadow_map);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, cluster_texture);
	shader_SetCurrentShaderUniform1i(UNIFORM_CLUSTER_TEXTURE, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_SAMPLER2, 1);
	shader_SetCurrentShaderUniform1i(UNIFORM_WIDTH, r_window_width);
	shader_SetCurrentShaderUniform1i(UNIFORM_HEIGHT, r_window_height);
	shader_SetCurrentShaderUniform1i(UNIFORM_FRAME, r_frame);
		
	/*glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectuiv(query1, GL_QUERY_RESULT, &elapsed);
	ms_elapsed = (float)elapsed / 1000000.0;	
	printf("%f		", ms_elapsed);*/	
	//if(query0_state == 1)
	//{
	//glBeginQuery(GL_TIME_ELAPSED, query0);
	//query0_state = 2;
	//}	
		
	for(i = 0; i < c; i++)
	{	
			
		triangle_group = &world_triangle_groups[i];
		material = &materials[triangle_group->material_index];
			
		color[0] = (float)material->r / 255.0;
		color[1] = (float)material->g / 255.0;
		color[2] = (float)material->b / 255.0;
		color[3] = (float)material->a / 255.0;
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
		glDrawElements(GL_TRIANGLES, triangle_group->next, GL_UNSIGNED_INT, (void *)(triangle_group->start * sizeof(int)));
	}
	
	//if(query0_state == 2)
	//{
	/*glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectuiv(query0, GL_QUERY_RESULT, &elapsed);	
	ms_elapsed = (float)elapsed / 1000000.0;
		
	printf("%f\n", ms_elapsed);*/
	//	query0_state = 1;
		//query0_state = 3;
	//}
	
	
	
	
	
	
	
	
	//}
	
	//printf("%x\n", glGetError());
	
	//glDisable(GL_BLEND);	
	glDisable(GL_STENCIL_TEST);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	
/*	#ifdef QUERY_STAGES
	renderer_EndTimeElapsedQuery(RENDERER_DRAW_WORLD_STAGE);
	#endif*/
	
}


void renderer_DrawWorldDeferred()
{
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
}



void renderer_DrawSkyBox()
{
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
	
	//s = engine_GetDeltaTime();
	
	SDL_GL_SwapWindow(window);
	r_frame++;
	
	#ifdef QUERY_STAGES
	//renderer_EndTimeElapsedQuery(RENDERER_SWAP_BUFFERS_STAGE);
	renderer_ReportQueryResults();
	#endif
	
	
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





