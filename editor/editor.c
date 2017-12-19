#include <stdio.h>

#include "GL\glew.h"

#include "matrix.h"
#include "vector.h"

#include "editor.h"
#include "camera.h"
#include "input.h"
#include "engine.h"
#include "brush.h"
#include "l_main.h"
#include "shader.h"
#include "material.h"
#include "texture.h"
#include "collision.h"
#include "gui.h"
#include "bsp_cmp.h"
#include "bsp.h"
#include "pvs.h"
#include "indirect.h"

#include "r_main.h"
#include "r_editor.h"

static camera_t *editor_camera;
static int editor_camera_index;

/* from renderer.c */
extern int window_width;
extern int window_height;


/* from light.c */
extern light_position_t *visible_light_positions;
extern light_params_t *visible_light_params;
extern light_position_t *light_positions;
extern light_params_t *light_params;
extern char **light_names;
extern int visible_light_count;
extern int light_count;




/* from input.c */
extern float normalized_mouse_x;
extern float normalized_mouse_y;
extern float last_mouse_x;
extern float last_mouse_y;
extern int bm_mouse;


/* from brush.c */
extern int brush_count;
extern brush_t *brushes;


/* from material.c */
extern int material_count;
extern material_t *materials;
extern char **material_names;

/* from collision.c */
extern int collision_geometry_vertice_count;
extern vec3_t *collision_geometry_positions;
extern vec3_t *collision_geometry_normals;
extern int *collision_geometry_quantized_normals;


float camera_speed = 0.6;
static unsigned int pick_framebuffer_id;
static unsigned int pick_framebuffer_texture;
static unsigned int pick_framebuffer_depth_texture;

unsigned int cursor_framebuffer_id;
unsigned int cursor_color_texture_id;
unsigned int cursor_depth_texture_id;

int brush_pick_shader;
int light_pick_shader;
int max_selections;
int selection_count;
pick_record_t *selections;
vec3_t cursor_3d_position;
vec3_t handle_3d_position;
int bm_handle_3d_flags;
int handle_3d_position_mode;
int handle_3d_mode;


void editor_Init()
{
	mat3_t r = mat3_t_id();
	editor_camera_index = camera_CreateCamera("editor_camera", vec3(5.0, 10.0, 0.0), &r, 0.68, window_width, window_height, 0.1, 500.0);
	editor_camera = camera_GetCameraByIndex(editor_camera_index);
	
	camera_SetCameraByIndex(editor_camera_index);
	
	input_RegisterKey(SDL_SCANCODE_ESCAPE);
	input_RegisterKey(SDL_SCANCODE_W);
	input_RegisterKey(SDL_SCANCODE_S);
	input_RegisterKey(SDL_SCANCODE_A);
	input_RegisterKey(SDL_SCANCODE_D);
	input_RegisterKey(SDL_SCANCODE_K);
	input_RegisterKey(SDL_SCANCODE_C);
	input_RegisterKey(SDL_SCANCODE_SPACE);
	input_RegisterKey(SDL_SCANCODE_LSHIFT);
	
	
	input_RegisterKey(SDL_SCANCODE_H);
	input_RegisterKey(SDL_SCANCODE_J);
	input_RegisterKey(SDL_SCANCODE_L);
	
	//renderer_RegisterFunction(bsp_Draw);
	
	//renderer_RegisterFunction(bsp_DrawPortals);
	//renderer_RegisterFunction(indirect_DrawVolumes);
	renderer_RegisterFunction(renderer_DrawBrushes);
	//renderer_RegisterFunction(renderer_DrawLeaves);
	renderer_RegisterFunction(renderer_DrawGrid);
	renderer_RegisterFunction(renderer_DrawLights);
	renderer_RegisterFunction(renderer_DrawSelected);
	renderer_RegisterFunction(renderer_DrawCursors);
	
	
	
	
	
	glGenFramebuffers(1, &pick_framebuffer_id);
	glGenTextures(1, &pick_framebuffer_texture);
	glGenTextures(1, &pick_framebuffer_depth_texture);
	
	glBindTexture(GL_TEXTURE_2D, pick_framebuffer_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, window_width, window_height, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindTexture(GL_TEXTURE_2D, pick_framebuffer_depth_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, window_width, window_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, pick_framebuffer_id);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pick_framebuffer_texture, 0);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, pick_framebuffer_depth_texture, 0);
	
	
	
	glGenFramebuffers(1, &cursor_framebuffer_id);
	glGenTextures(1, &cursor_color_texture_id);
	glGenTextures(1, &cursor_depth_texture_id);
	
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cursor_framebuffer_id);
	
	glBindTexture(GL_TEXTURE_2D, cursor_color_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	
	glBindTexture(GL_TEXTURE_2D, cursor_depth_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, window_width, window_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cursor_color_texture_id, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, cursor_depth_texture_id, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, cursor_depth_texture_id, 0);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
	
	
	
	max_selections = 64;
	selection_count = 0;
	selections = malloc(sizeof(pick_record_t ) * max_selections);
	
	
	cursor_3d_position = vec3(0.0, 0.0, 0.0);
	handle_3d_position = vec3(0.0, 0.0, 0.0);
	mat3_t_rotate(&r, vec3(1.0, 0.0, 0.0), 0.0, 1);
	//brush_CreateBrush(vec3(0.0, 6.0, 0.0), &r, vec3(1.0, 2.0, 1.0), BRUSH_CYLINDER);
	
	int i;
	
	/*for(i = 0; i < 20; i++)
	{
		brush_CreateBrush(vec3(0.0, 0.2 + 0.25 * i, i * 0.5), &r, vec3(10.0 - i * 0.1, 0.5, 10.0), BRUSH_CUBE);
	}*/
	
	

	//brush_CreateBrush(vec3(0.0, 0.0, 0.0), &r, vec3(10.0, 2.0, 10.0), BRUSH_CUBE);
	
	//brush_CreateBrush(vec3(0.0, 5.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	//brush_CreateBrush(vec3(0.0, -5.0, -10.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	/*brush_CreateBrush(vec3(0.0, -1.0, 0.0), &r, vec3(10.0, 1.0, 10.0), BRUSH_CUBE);*/
	//brush_CreateBrush(vec3(2.0, 0.0, 0.0), &r, vec3(1.0, 1.0, 1.0), BRUSH_CUBE);
	//brush_CreateBrush(vec3(-2.0, 0.0, 0.0), &r, vec3(1.0, 1.0, 1.0), BRUSH_CUBE);
	//brush_CreateBrush(vec3(0.0, 0.0, 4.0), &r, vec3(4.0, 1.0, 1.0), BRUSH_CUBE);
	
	
	//brush_CreateBrush(vec3(0.0, -2.5, 0.0), &r, vec3(10.0, 0.5, 10.0), BRUSH_CUBE);
	//brush_CreateBrush(vec3(0.0, 2.5, -10.0), &r, vec3(10.0, 0.5, 10.0), BRUSH_CUBE);
	light_CreateLight("light0", &r, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), 40.0, 20.0);
	
	/*brush_CreateBrush(vec3(0.0, -10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	
	brush_CreateBrush(vec3(10.0, 0.0, 0.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-10.0, 0.0, 0.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 0.0, 10.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 0.0, -10.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	
	brush_CreateBrush(vec3(0.0, 10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);*/
	
	
	#if 1
	/* walls */
	brush_CreateBrush(vec3(30.0, 0.0, 0.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-30.0, 0.0, 0.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(30.0, 0.0, 20.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-30.0, 0.0, 20.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(30.0, 0.0, -20.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-30.0, 0.0, -20.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	
	
	brush_CreateBrush(vec3(0.0, 0.0, 30.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 0.0, -30.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(-20.0, 0.0, 30.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(-20.0, 0.0, -30.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(20.0, 0.0, 30.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(20.0, 0.0, -30.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	
	
	
	brush_CreateBrush(vec3(29.0, -6.0, 0.0), &r, vec3(5.0, 5.0, 10.0), BRUSH_CUBE);
	
	
	/* floor */
	brush_CreateBrush(vec3(20.0, -10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, -10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-20.0, -10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(20.0, -10.0, 20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, -10.0, 20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-20.0, -10.0, 20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(20.0, -10.0, -20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, -10.0, -20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-20.0, -10.0, -20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	
	/* ceiling */
	brush_CreateBrush(vec3(20.0, 10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-20.0, 10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(20.0, 10.0, 20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 10.0, 20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-20.0, 10.0, 20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(20.0, 10.0, -20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 10.0, -20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-20.0, 10.0, -20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	
	#endif
	
	#if 0
	
	brush_CreateBrush(vec3(0.0, 5.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 10.0, -10.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 15.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 20.0, -10.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 25.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 30.0, -10.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 35.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 40.0, -10.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 45.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 50.0, -10.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 55.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 60.0, -10.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 65.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	
	
	brush_CreateBrush(vec3(5.0, 0.0, 0.0), &r, vec3(0.25, 10.0, 15.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-5.0, 0.0, 0.0), &r, vec3(0.25, 10.0, 15.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(5.0, 20.0, 0.0), &r, vec3(0.25, 10.0, 15.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-5.0, 20.0, 0.0), &r, vec3(0.25, 10.0, 15.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(5.0, 40.0, 0.0), &r, vec3(0.25, 10.0, 15.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-5.0, 40.0, 0.0), &r, vec3(0.25, 10.0, 15.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(5.0, 60.0, 0.0), &r, vec3(0.25, 10.0, 15.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-5.0, 60.0, 0.0), &r, vec3(0.25, 10.0, 15.0), BRUSH_CUBE);
	
	
	brush_CreateBrush(vec3(0.0, 0.0, 5.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 0.0, -15.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 20.0, 5.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 20.0, -15.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 40.0, 5.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 40.0, -15.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 60.0, 5.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 60.0, -15.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);	
	
	mat3_t_rotate(&r, vec3(0.0, 1.0, 0.0), 0.25, 1);
	brush_CreateBrush(vec3(0.0, 0.0, 0.0), &r, vec3(1.0, 50.0, 1.0), BRUSH_CYLINDER);
	
	
	light_CreateLight("light0", &r, vec3(0.0, 0.0, 24.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light1", &r, vec3(24.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0), 10.0, 20.0);
	light_CreateLight("light2", &r, vec3(-24.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light3", &r, vec3(0.0, 0.0, -24.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	
	
	light_CreateLight("light4", &r, vec3(0.0, 20.0, 24.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light5", &r, vec3(24.0, 20.0, 0.0), vec3(1.0, 0.0, 0.0), 10.0, 20.0);
	light_CreateLight("light6", &r, vec3(-24.0, 20.0, 0.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light7", &r, vec3(0.0, 20.0, -24.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	
	
	light_CreateLight("light8", &r, vec3(0.0, 40.0, 24.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light9", &r, vec3(24.0, 40.0, 0.0), vec3(1.0, 0.0, 0.0), 10.0, 20.0);
	light_CreateLight("light10", &r, vec3(-24.0, 40.0, 0.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light11", &r, vec3(0.0, 40.0, -24.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	
	light_CreateLight("light8", &r, vec3(0.0, 60.0, 24.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light9", &r, vec3(24.0, 60.0, 0.0), vec3(1.0, 0.0, 0.0), 10.0, 20.0);
	light_CreateLight("light10", &r, vec3(-24.0, 60.0, 0.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light11", &r, vec3(0.0, 60.0, -24.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	
	#endif 
	
	//widget_t *w = gui_CreateWidget("widget0", 100, 0, 400, 50);
	//gui_AddButtonToWidget(w, "button0", 0, 0, 100, 40, 0);
	//gui_CreateWidget("widget1", -100.0, 0.0, 400.0, 50.0);
		
	bm_handle_3d_flags = 0;
	handle_3d_position_mode = HANDLE_3D_MEDIAN_POINT;
	handle_3d_mode = HANDLE_3D_TRANSLATION;
	
	
	/*bsp_triangle_t tri;
	bsp_triangle_t splitter;
	bsp_triangle_t *ret;
	
	
	tri.a = vec3(-1.0, 0.0, 0.0);
	tri.b = vec3(0.2, 1.0, 0.0);
	tri.c = vec3(1.0, 0.0, 0.0);
	tri.normal = vec3(0.0, 0.0, 1.0);
	
	splitter.a = vec3(0.0, 0.0, -1.0);
	splitter.b = vec3(0.0, 0.0, 1.0);
	splitter.c = vec3(0.0, 1.0, 0.0);
	splitter.normal = vec3(-1.0, 0.0, 0.0);
	
	bsp_SplitTriangle(&tri, &splitter, &ret);*/
	
	
	/*bsp_edge_t front;
	bsp_edge_t back;
	int q;
	
	q = bsp_ClipEdge(vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0), &front, &back);
	
	if(q & POINT_FRONT)
	{
		printf("edge in front of plane!\n[%f %f %f]   [%f %f %f]\n", front.a.x, front.a.y, front.a.z, front.b.x, front.b.y, front.b.z);
	}
	
	if(q & POINT_BACK)
	{
		printf("edge in back of plane!\n[%f %f %f]   [%f %f %f]\n", back.a.x, back.a.y, back.a.z, back.b.x, back.b.y, back.b.z);
	}*/
	
	//vec3_t rrr = cross(vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0));
	
	
	//printf("[%f %f %f]\n", rrr.x, rrr.y, rrr.z);
	
	//printf("%f\n", get_angle(vec3(0.0, -0.70710659, 0.707106948), vec3(0.0, -0.707106769, -0.707106769), vec3(1.0, 0.0, 0.0)));
	
	//printf("%f\n", get_angle(vec3(0.0, 0.0, 1.0), vec3(-1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0)));
	
	//printf("%f\n", get_angle(vec3(0.0, 0.70710665, 0.707106829), vec3(0.0, -0.70710665, 0.707106829), vec3(-1.0, 0.0, 0.0)));
	
	
	/*texture_LoadTexture("textures\\env\\aliencube_neg_z.jpg", "negz");
	texture_LoadTexture("textures\\env\\aliencube_pos_z.jpg", "posz");
	texture_LoadTexture("textures\\env\\aliencube_neg_x.jpg", "negx");
	texture_LoadTexture("textures\\env\\aliencube_pos_x.jpg", "posx");
	texture_LoadTexture("textures\\env\\aliencube_neg_y.jpg", "negy");
	texture_LoadTexture("textures\\env\\aliencube_pos_y.jpg", "posy");*/
	
	
	texture_LoadCubeTexture("textures\\env\\aliencube_pos_x.jpg;"
					        "textures\\env\\aliencube_neg_x.jpg;"
					        "textures\\env\\aliencube_pos_y.jpg;"
					        "textures\\env\\aliencube_neg_y.jpg;"
					        "textures\\env\\aliencube_pos_z.jpg;"
					        "textures\\env\\aliencube_neg_z.jpg;", "env");
	
	
	/*texture_LoadCubeTexture("textures\\env\\cliffrt.bmp;"
					        "textures\\env\\clifflf.bmp;"
					        "textures\\env\\cliffup.bmp;"
					        "textures\\env\\cliffdn.bmp;"
					        "textures\\env\\cliffbk.bmp;"
					        "textures\\env\\cliffft.bmp;", "env");*/
	
}

void editor_Finish()
{
	glDeleteFramebuffers(1, &pick_framebuffer_id);
	glDeleteTextures(1, &pick_framebuffer_texture);
	glDeleteTextures(1, &pick_framebuffer_depth_texture);
	
	glDeleteFramebuffers(1, &cursor_framebuffer_id);
	glDeleteTextures(1, &cursor_color_texture_id);
	glDeleteTextures(1, &cursor_depth_texture_id);
	
	free(selections);
}

extern int b_draw_brushes;

void editor_Main(float delta_time)
{
	static float yaw = 0.0;
	static float pitch = 0.0;
	
	static float r = 0.0;
	
	mat3_t rot = mat3_t_id();
	
	camera_t *active_camera = camera_GetActiveCamera();
	
	vec3_t translation;
	vec3_t position;
	
	static vec3_t velocity = {0.0, 0.0, 0.0};
	
	vec3_t forward_vector;
	vec3_t right_vector;
	
	int mouse_bm;
	
	float intersection;
	int i;
	
	pick_record_t record;
	

	if(input_GetKeyStatus(SDL_SCANCODE_ESCAPE) & KEY_PRESSED)
	{
		engine_SetEngineState(ENGINE_QUIT);
		return;
	}
 	
 	translation = vec3(0.0, 0.0, 0.0);
	
	if(bm_mouse & MOUSE_MIDDLE_BUTTON_CLICKED)
	{	
		engine_SetEngineState(ENGINE_PLAYING);
		
		/* avoid the camera to snap at a random direction when
		the engine change states... */
		if(last_mouse_x || last_mouse_y)
		{
			normalized_mouse_x = 0.0;
			normalized_mouse_y = 0.0;
		}
		
		
		yaw -= normalized_mouse_x * 0.25;
		pitch += normalized_mouse_y * 0.25;
		
		//printf("%f %f\n", yaw, pitch);
		
		if(pitch > 0.5) pitch = 0.5;
		else if(pitch < -0.5) pitch = -0.5;
			
		if(yaw > 1.0) yaw = -1.0 + (yaw - 1.0);
		else if(yaw < -1.0) yaw = 1.0 + (yaw + 1.0);
		
		camera_PitchYawCamera(editor_camera, yaw, pitch);
		
		forward_vector = editor_camera->world_orientation.f_axis;
		right_vector = editor_camera->world_orientation.r_axis;
		
		//translation = vec3(0.0, 0.0, 0.0);
		
		
		if(input_GetKeyStatus(SDL_SCANCODE_H) & KEY_JUST_PRESSED)
		{
			brush_CreateBrush(vec3(0.0, 0.0, 0.0), &rot, vec3(10.0, 0.5, 10.0), BRUSH_CUBE);
		}
		
		if(input_GetKeyStatus(SDL_SCANCODE_J) & KEY_JUST_PRESSED)
		{
			brush_CreateBrush(vec3(0.0, 0.0, 0.0), &rot, vec3(10.0, 10.0, 0.5), BRUSH_CUBE);
		}
		
		if(input_GetKeyStatus(SDL_SCANCODE_K) & KEY_JUST_PRESSED)
		{
			brush_CreateBrush(vec3(0.0, 0.0, 0.0), &rot, vec3(0.5, 10.0, 10.0), BRUSH_CUBE);
		}
		
		
		
		
		if(input_GetKeyStatus(SDL_SCANCODE_W) & KEY_PRESSED)
		{
			translation.x -= forward_vector.x;
			translation.y -= forward_vector.y;
			translation.z -= forward_vector.z;
		}
		
		if(input_GetKeyStatus(SDL_SCANCODE_S) & KEY_PRESSED)
		{
			translation.x += forward_vector.x;
			translation.y += forward_vector.y;
			translation.z += forward_vector.z;
		}
		
		if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_PRESSED)
		{
			translation.x -= right_vector.x;
			translation.y -= right_vector.y;
			translation.z -= right_vector.z;
		}
		
		if(input_GetKeyStatus(SDL_SCANCODE_D) & KEY_PRESSED)
		{
			translation.x += right_vector.x;
			translation.y += right_vector.y;
			translation.z += right_vector.z;
		}
		
		if(input_GetKeyStatus(SDL_SCANCODE_SPACE) & KEY_PRESSED)
		{
			translation.y += 0.8;
		}
		
		if(input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED)
		{
			translation.y -= 0.8;
		}
		
		if(input_GetKeyStatus(SDL_SCANCODE_SPACE) & KEY_JUST_PRESSED)
		{
			velocity.y = 0.2;
		}
		
		if(bm_mouse & MOUSE_WHEEL_UP)
		{
			camera_speed += 0.05;
		}
		else if(bm_mouse & MOUSE_WHEEL_DOWN)
		{
			camera_speed -= 0.05;
			if(camera_speed < 0.05) camera_speed = 0.05;
		}
		
		
		
		//position = editor_camera->world_position; 
		
		//printf("before: [%f %f %f]\n", editor_camera->world_position.x, editor_camera->world_position.y, editor_camera->world_position.z);
		
		translation.x *= camera_speed;
		translation.y *= camera_speed;
		translation.z *= camera_speed;
		
		velocity.x = translation.x;
		velocity.y = translation.y;
		//velocity.y -= 0.0098;
		velocity.z = translation.z;
		
		//bsp_Collide(&editor_camera->world_position, translation);
		
		bsp_Move(&editor_camera->world_position, &velocity);
		
		//position = vec3(22.0, 5.0, -12.0);
		//velocity = vec3(0.0, -10.0, 0.0);
		
		//bsp_Move(&position, &velocity);
		
		//printf("after: [%f %f %f]\n\n", editor_camera->world_position.x, editor_camera->world_position.y, editor_camera->world_position.z);
		
		//editor_camera->world_position = position;
		
		//if(bsp_Collide(&position, translation))
		//{
			
			/*translation.x *= intersection;
			translation.y *= intersection;
			translation.z *= intersection;*/
			
			//translation.y = 0.0;
			
			/*editor_camera->world_position.x += translation.x * intersection;
			editor_camera->world_position.y += translation.y * intersection;
			editor_camera->world_position.z += translation.z * intersection;*/
		//}
		
		//camera_TranslateCamera(editor_camera, translation, 1.0, 0);
		
		
		//editor_camera->world_position.y += 3.5;
		
		camera_ComputeWorldToCameraMatrix(editor_camera);
		
		//editor_camera->world_position.y -= 3.5;
		
	}
	else
	{
		engine_SetEngineState(ENGINE_PAUSED);
		editor_ProcessMouse();
		
		if(input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED)
		{
			if(input_GetKeyStatus(SDL_SCANCODE_C) & KEY_JUST_PRESSED)
			{
				bsp_CompileBsp();
				//indirect_BuildVolumes();
			}
			else if(input_GetKeyStatus(SDL_SCANCODE_S) & KEY_JUST_PRESSED)
			{
				/* save project... */
			}
			else if(input_GetKeyStatus(SDL_SCANCODE_SPACE) & KEY_JUST_PRESSED)
			{
				b_draw_brushes ^= 1;
				//bsp_NextPortal();
			}
		}
			
	}
	
	
	/*else
	{
		printf("nope!\n");
	}*/
	
	
	//bsp_BuildBspFromBrushes();
	
	//bsp_Draw(editor_camera->world_position);
}

void editor_ProcessMouse()
{
	pick_record_t record; 
	camera_t *active_camera = camera_GetActiveCamera();
	mat4_t model_view_projection_matrix;
	vec4_t p;
	vec3_t direction;
	int i;
	
	/*float handle_3d_screen_x;
	float handle_3d_screen_y;
	
	float mouse_screen_x;
	float mouse_screen_y;*/
	
	float screen_x;
	float screen_y;
	
	float screen_dx;
	float screen_dy;
	
	static float grab_screen_offset_x;
	static float grab_screen_offset_y;
	
	float amount;
	float z;
	
	if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
	{
		editor_Check3dHandle();		
	}
	
	if(!(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
	{
		bm_handle_3d_flags = 0;
	}
	else if(bm_handle_3d_flags)
	{
		p.vec3 = handle_3d_position;
		p.w = 1.0;
		mat4_t_mult_fast(&model_view_projection_matrix, &active_camera->world_to_camera_matrix, &active_camera->projection_matrix);
		mat4_t_vec4_t_mult(&model_view_projection_matrix, &p);
			
		p.x /= p.w;
		p.y /= p.w;
		z = p.z;
			
		if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
		{
			grab_screen_offset_x = normalized_mouse_x - p.x;
			grab_screen_offset_y = normalized_mouse_y - p.y;
		}
			
		screen_dx = normalized_mouse_x - p.x - grab_screen_offset_x;
		screen_dy = normalized_mouse_y - p.y - grab_screen_offset_y;
		
		//amount = sqrt(screen_dx * screen_dx + screen_dy * screen_dy);
		
		if(bm_handle_3d_flags & HANDLE_3D_GRABBED_X_AXIS)
		{
			
			direction = vec3(1.0, 0.0, 0.0);
			//direction = active_camera->world_to_camera_matrix.r_axis;
		}
		else if(bm_handle_3d_flags & HANDLE_3D_GRABBED_Y_AXIS)
		{
			direction = vec3(0.0, 1.0, 0.0);
			//direction = active_camera->world_to_camera_matrix.u_axis;
		}
		else if(bm_handle_3d_flags & HANDLE_3D_GRABBED_Z_AXIS)
		{
			direction = vec3(0.0, 0.0, 1.0);
			//direction = active_camera->world_to_camera_matrix.f_axis;
		}
		
		
		p.vec3 = direction;
		p.w = 0.0;
		
		mat4_t_vec4_t_mult(&active_camera->world_to_camera_matrix, &p);
		
		screen_x = p.x;
		screen_y = p.y;
		
		amount = sqrt(screen_x * screen_x + screen_y * screen_y);
		
		screen_x /= amount;
		screen_y /= amount;
		
		amount = (screen_dx * screen_x + screen_dy * screen_y) * z;
		
		switch(handle_3d_mode)
		{
			case HANDLE_3D_TRANSLATION:			
				editor_TranslateSelections(direction, amount);
			break;
			
			case HANDLE_3D_ROTATION:
				
			break;
		}

	}
		
	
		
	if(bm_mouse & MOUSE_RIGHT_BUTTON_JUST_CLICKED)
	{
		
		if(editor_Pick(&record))
		{	
		
				
			if(record.type == selections[selection_count - 1].type)
			{
				/* if the just picked thing is the same as the last picked thing... */	
				if(record.index0 == selections[selection_count - 1].index0)
				{
						
					/* if this selection is the last in the list (meaning
					this object is the active object), drop it, set
					whatever comes before it as the active object and then jump
					to the code that sets the 3d handle position... */
						
					editor_DropSelection(&record);
					if(selection_count)
					{
						record = selections[selection_count - 1];
						goto _set_handle_3d_position;
					}
					
				}
				else/* if(input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED)*/
				{
					/* if this record  is not equal to the last in the list,
					append it to the list or set it as the only active object... */
					goto _add_new_selection;
				}
				
			}
			else
			{
				_add_new_selection:
				/* holding shift enables selecting multiple objects... */			
				if(!(input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED))
				{
					editor_ClearSelection();
				}
									
				editor_AddSelection(&record);
						
				_set_handle_3d_position:
				
				if(handle_3d_position_mode == HANDLE_3D_MEDIAN_POINT)
				{
					handle_3d_position = vec3(0.0, 0.0, 0.0);
					for(i = 0; i < selection_count; i++)
					{
						switch(selections[i].type)
						{
							case PICK_BRUSH:
								handle_3d_position.x += brushes[selections[i].index0].position.x;
								handle_3d_position.y += brushes[selections[i].index0].position.y;
								handle_3d_position.z += brushes[selections[i].index0].position.z;
							break;
									
							case PICK_LIGHT:
								handle_3d_position.x += light_positions[selections[i].index0].position.x;
								handle_3d_position.y += light_positions[selections[i].index0].position.y;
								handle_3d_position.z += light_positions[selections[i].index0].position.z;
							break;
						}
					}
					
					handle_3d_position.x /= selection_count;
					handle_3d_position.y /= selection_count;
					handle_3d_position.z /= selection_count;
				}
				else
				{
					switch(record.type)
					{
						case PICK_BRUSH:
							handle_3d_position = brushes[record.index0].position;	
						break;
									
						case PICK_LIGHT:
							handle_3d_position = light_positions[record.index0].position;
						break;
					}
				}
					
			}
		}
			
	}
}

void editor_TranslateSelections(vec3_t direction, float amount)
{
	int i;
	int c = selection_count;
	
	vec3_t v;
	
	for(i = 0; i < c; i++)
	{
		switch(selections[i].type)
		{
			case PICK_BRUSH:
				
				v.x = direction.x * amount;
				v.y = direction.y * amount;
				v.z = direction.z * amount;
				
				brush_TranslateBrush(&brushes[selections[i].index0], v);
			break;
			
			case  PICK_LIGHT:
				light_TranslateLight(selections[i].index0, direction, amount);
			break;
		}
	}
	
	handle_3d_position.x += direction.x * amount;
	handle_3d_position.y += direction.y * amount;
	handle_3d_position.z += direction.z * amount;
}

void editor_RotateSelections(vec3_t axis, float amount, int individual_origins)
{
	int i;
	int c = selection_count;
	
	for(i = 0; i < c; i++)
	{
		
	}
}

void editor_EnablePicking()
{
	while(glGetError());
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pick_framebuffer_id);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, pick_framebuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, window_width, window_height);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);	
}

void editor_DisablePicking()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
}

int editor_Pick(pick_record_t *record)
{
	int i;
	int c;
	int j;
	int k;
	int x;
	int y;
	
	int pick_type;
	int pick_index0;
	int pick_index1;
	int pick_index2;
	
	camera_t *active_camera = camera_GetActiveCamera();
	triangle_group_t *triangle_group;
	material_t *material;
	
	float q[4];
	
	gpu_BindGpuHeap();
	shader_UseShader(brush_pick_shader);	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pick_framebuffer_id);
	//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	editor_EnablePicking();
	
	c = brush_count;
		
	for(i = 0; i < c; i++)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brushes[i].element_buffer);
		k = brushes[i].triangle_group_count;
		
		triangle_group = brushes[i].triangle_groups;
		
		for(j = 0; j < k; j++)
		{		
			*(int *)&q[0] = PICK_BRUSH;
			*(int *)&q[1] = i + 1;
			q[2] = 0.0;
			q[3] = 0.0;
			
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, q);
			glDrawElements(GL_TRIANGLES, triangle_group[j].next, GL_UNSIGNED_INT, (void *)(triangle_group[j].start * sizeof(int)));
		}
		
	}
	
	gpu_UnbindGpuHeap();
	
	shader_UseShader(light_pick_shader);
	
	c = light_count;
	glPointSize(24.0);
	glEnable(GL_POINT_SMOOTH);
	
	for(i = 0; i < c; i++)
	{
		*(int *)&q[0] = PICK_LIGHT;
		*(int *)&q[1] = i + 1;
		q[2] = 0.0;
		q[3] = 0.0;	
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, q);
		
		glBegin(GL_POINTS);
		glVertex3f(light_positions[i].position.x, light_positions[i].position.y, light_positions[i].position.z);
		glEnd();
	}
	
	glDisable(GL_POINT_SMOOTH);
	glPointSize(1.0);
	
	x = window_width * (normalized_mouse_x * 0.5 + 0.5);
	y = window_height * (normalized_mouse_y * 0.5 + 0.5);
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, q);
	
	//printf("[%d %d %d %d]\n", *(int *)&q[0], *(int *)&q[1], *(int *)&q[2], *(int *)&q[3]);
	
	editor_DisablePicking();
	
	pick_type = (*(int *)&q[0]);
	
	switch(pick_type)
	{
		case PICK_BRUSH:
		case PICK_LIGHT:
			record->type = pick_type;
			record->index0 = (*(int *)&q[1]) - 1;
			record->index1 = (*(int *)&q[2]);
			record->index2 = (*(int *)&q[3]);
			return 1;
		break;
		
		case PICK_NONE:
			return 0;
		break;
	}
	
	//if(pick_type == PICK_LIGHT) printf("light!\n");
		
}

int editor_Check3dHandle()
{
	camera_t *active_camera = camera_GetActiveCamera();
	vec4_t cursor_position;
	vec3_t right_vector;
	vec3_t up_vector;
	vec3_t forward_vector;
	
	int i;
	float step = (2.0 * 3.14159265) / ROTATION_HANDLE_DIVS;
	float angle = 0.0;
	
	float angles_lut[ROTATION_HANDLE_DIVS][2];
	
	editor_EnablePicking();
	
	float nzfar = -active_camera->frustum.zfar;
	float nznear = -active_camera->frustum.znear;
	float ntop = active_camera->frustum.top;
	float nright = active_camera->frustum.right;
	float qt = nznear / ntop;
	float qr = nznear / nright;
	float d;
	
	float s;
	float c;
	
	int x;
	int y;
	
	float q[4];
	
	cursor_position.vec3 = handle_3d_position;
	cursor_position.w = 1.0;
		
	mat4_t_vec4_t_mult(&active_camera->world_to_camera_matrix, &cursor_position);		
		
	if(cursor_position.z < nznear)
	{
		glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
		glEnable(GL_POINT_SMOOTH);

			
		d = sqrt(cursor_position.x * cursor_position.x + cursor_position.y * cursor_position.y + cursor_position.z * cursor_position.z) * 0.2;
		
		
		
		switch(handle_3d_mode)
		{
			case HANDLE_3D_TRANSLATION:
				
				right_vector = vec3(1.0, 0.0, 0.0);
				up_vector = vec3(0.0, 1.0, 0.0);
				forward_vector = vec3(0.0, 0.0, 1.0);
						
				right_vector.x *= d;
				up_vector.y *= d;
				forward_vector.z *= d;
				
				right_vector.x += handle_3d_position.x;
				right_vector.y += handle_3d_position.y;
				right_vector.z += handle_3d_position.z;
					
				up_vector.x += handle_3d_position.x;
				up_vector.y += handle_3d_position.y;
				up_vector.z += handle_3d_position.z;
					
				forward_vector.x += handle_3d_position.x;
				forward_vector.y += handle_3d_position.y;
				forward_vector.z += handle_3d_position.z;
					
				glPointSize(16.0);
				glBegin(GL_POINTS);
				glColor3f(1.0, 1.0, 1.0);
				glVertex3f(handle_3d_position.x, handle_3d_position.y, handle_3d_position.z);
				glEnd();
							
				glPointSize(16.0);
				glBegin(GL_POINTS);
				glColor3f(1.0, 0.0, 0.0);
				glVertex3f(right_vector.x, right_vector.y, right_vector.z);
				glColor3f(0.0, 1.0, 0.0);
				glVertex3f(up_vector.x, up_vector.y, up_vector.z);
				glColor3f(0.0, 0.0, 1.0);
				glVertex3f(forward_vector.x, forward_vector.y, forward_vector.z);
				glEnd();
							
				glLineWidth(16.0);
				glBegin(GL_LINES);
				glColor3f(1.0, 0.0, 0.0);
				glVertex3f(handle_3d_position.x, handle_3d_position.y, handle_3d_position.z);
				glVertex3f(right_vector.x, right_vector.y, right_vector.z);
							
				glColor3f(0.0, 1.0, 0.0);
				glVertex3f(handle_3d_position.x, handle_3d_position.y, handle_3d_position.z);
				glVertex3f(up_vector.x, up_vector.y, up_vector.z);
						
				glColor3f(0.0, 0.0, 1.0);
				glVertex3f(handle_3d_position.x, handle_3d_position.y, handle_3d_position.z);
				glVertex3f(forward_vector.x, forward_vector.y, forward_vector.z);
				glEnd();
			break;
			
			
			case HANDLE_3D_ROTATION:
				angle = 0.0;
				for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
				{
					s = sin(angle);
					c = cos(angle);
					angles_lut[i][0] = sin(angle) * d;
					angles_lut[i][1] = cos(angle) * d;	
					angle += step;
				}
					
				glLineWidth(6.0);
				glBegin(GL_LINE_LOOP);
				glColor3f(1.0, 0.0, 0.0);
				for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
				{
					glVertex3f(handle_3d_position.x, handle_3d_position.y + angles_lut[i][0], handle_3d_position.z + angles_lut[i][1]);
				}		
				glEnd();
				glPointSize(4.0);
				glBegin(GL_POINTS);
				for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
				{
					glVertex3f(handle_3d_position.x, handle_3d_position.y + angles_lut[i][0], handle_3d_position.z + angles_lut[i][1]);
				}		
				glEnd();
				
					
				glBegin(GL_LINE_LOOP);
				glColor3f(0.0, 1.0, 0.0);
				for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
				{
					glVertex3f(handle_3d_position.x + angles_lut[i][1], handle_3d_position.y, handle_3d_position.z + angles_lut[i][0]);
				}		
				glEnd();
				glBegin(GL_POINTS);
				for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
				{
					glVertex3f(handle_3d_position.x + angles_lut[i][1], handle_3d_position.y, handle_3d_position.z + angles_lut[i][0]);
				}		
				glEnd();
				
					
				glBegin(GL_LINE_LOOP);
				glColor3f(0.0, 0.0, 1.0);
				for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
				{
					glVertex3f(handle_3d_position.x + angles_lut[i][1], handle_3d_position.y + angles_lut[i][0], handle_3d_position.z);
				}		
				glEnd();
				glBegin(GL_POINTS);
				for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
				{
					glVertex3f(handle_3d_position.x + angles_lut[i][1], handle_3d_position.y + angles_lut[i][0], handle_3d_position.z);
				}		
				glEnd();
			break;
		}
			
		
		
		x = window_width * (normalized_mouse_x * 0.5 + 0.5);
		y = window_height * (normalized_mouse_y * 0.5 + 0.5);
		glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, q);
		
		bm_handle_3d_flags;
		
		if(q[0])
		{
			bm_handle_3d_flags |= HANDLE_3D_GRABBED_X_AXIS;
			//printf("picked x axis!\n");
		}
		
		if(q[1])
		{
			bm_handle_3d_flags |= HANDLE_3D_GRABBED_Y_AXIS;
			//printf("picked y axis!\n");
		}
		
		if(q[2])
		{
			bm_handle_3d_flags |= HANDLE_3D_GRABBED_Z_AXIS;
			//printf("picked z axis!\n");
		}
		
		
		glLineWidth(1.0);
		glPointSize(1.0);
		glDisable(GL_BLEND);
		glDisable(GL_POINT_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		
		
		//glMatrixMode(GL_PROJECTION);
		//glPopMatrix();
		//glMatrixMode(GL_MODELVIEW);
		
		
	}
	
	
	
	editor_DisablePicking();
}

void editor_AddSelection(pick_record_t *record)
{
	/* try to drop this selection, to make sure
	there's just one selection per object... */
	editor_DropSelection(record);
	
	selections[selection_count++] = *record;
}

void editor_DropSelection(pick_record_t *record)
{
	int i;
	int c = selection_count;
	int j;
	int k;
	
	
	for(i = 0; i < c; i++)
	{
		if(record->type == selections[i].type)
		{
			if(record->index0 == selections[i].index0)
			{
				for(j = i; j < c - 1; j++)
				{
					selections[j] = selections[j + 1];
				}
				
				selection_count--;
			}
		}
	}
}

void editor_ClearSelection()
{
	selection_count = 0;
}

void editor_ExportMap(char *file_name)
{
	
	#if 0
	
	int i;
	int c = brush_count;
	
	int j;
	int k;
	
	int l;
	int m;
	int bm_material_flags;
	
	FILE *f;
	
	int total_vertex_count = 0;
	int total_material_count = 0;
	int total_light_count = 0;
	
	float light_radius;
	float light_energy;
	
	triangle_group_t *triangle_groups;
	triangle_group_t *group;
	brush_triangle_t *triangle;
	vertex_t *vertices;
	
	vec4_t diffuse_color;
	
	#define MAX_MATERIAL_NAME_LEN 64
	#define MAX_LIGHT_NAME_LEN 64
	
	char material_name[MAX_MATERIAL_NAME_LEN];
	char light_name[MAX_LIGHT_NAME_LEN];
	char output_name[128];
	
	for(i = 0; i < c; i++)
	{
		total_vertex_count += brushes[i].vertex_count;
		total_material_count += brushes[i].triangle_group_count;
	}
	
	/* A bit conservative, but whatever... */
	triangle_groups = malloc(sizeof(triangle_group_t) * total_material_count);
	vertices = malloc(sizeof(vertex_t) * total_vertex_count);
	
	
	total_material_count = 0;
	
	for(i = 0; i < c; i++)
	{
		k = brushes[i].triangle_group_count;
		
		for(j = 0; j < k; j++)
		{
			group = &brushes[i].triangle_groups[j];
			
			/* look for this group in the list... */
			for(l = 0; l < total_material_count; l++)
			{
				if(group->material_index == triangle_groups[l].material_index)
				{
					/* this triangle group was already added to this list, so just add the vertex count
					and bail out... */
					triangle_groups[l].vertex_count += group->vertex_count;
					break;
				}
			}
			
			/* group wasn't in the list, so it is new... */
			if(l == total_material_count)
			{
				triangle_groups[total_material_count].vertex_count = group->vertex_count;
				triangle_groups[total_material_count].material_index = group->material_index;
				total_material_count++;	
			}
		}
	}
	
	/* Set up start information, necessary for placing the vertices within
	the proper partitions... */
	triangle_groups[0].start = 0;
	triangle_groups[0].next = 0;
	for(i = 0; i < total_material_count - 1; i++)
	{
		triangle_groups[i + 1].start = triangle_groups[i].start + triangle_groups[i].vertex_count;
		triangle_groups[i + 1].next = 0;
	}
	
	c = brush_count;
	
	for(i = 0; i < c; i++)
	{
		k = brushes[i].vertex_count / 3;
		
		for(j = 0; j < k; j++)
		{
			triangle = &brushes[i].triangles[j];
			group = &brushes[i].triangle_groups[triangle->triangle_group_index];
			for(l = 0; l < total_material_count; l++)
			{
				if(group->material_index == triangle_groups[l].material_index)
				{
					m = triangle_groups[l].start + triangle_groups[l].next;
					triangle_groups[l].next += 3;
					
					l = triangle->first_vertex;
					
					vertices[m	  ] = brushes[i].vertices[l	   ];
					vertices[m + 1] = brushes[i].vertices[l + 1];
					vertices[m + 2] = brushes[i].vertices[l + 2];
					
					break;
				}
			}
		}
	}
	
	strcpy(output_name, file_name);
	c = strlen(output_name);
	i = 0;
	while(i < c && output_name[i] != '.') i++;
	output_name[i] = '\0';
	
	strcat(output_name, ".fms");

	
	
	f = fopen(output_name, "wb");
	
	fwrite(&total_vertex_count, sizeof(int), 1, f);		/* vertice count */
	fwrite(&total_material_count, sizeof(int), 1, f);	/* material count */
	fwrite(&light_count, sizeof(int), 1, f);			/* light count */
	
	for(i = 0; i < total_material_count; i++)
	{
		m = triangle_groups[i].material_index;
		
		strcpy(material_name, material_names[m]);		
		fwrite(material_name, 1, MAX_MATERIAL_NAME_LEN, f);		/* material name */
		
		int bm_material_flags = 0;
		
		fwrite(&bm_material_flags, sizeof(int), 1, f);			/* material flags */
		
		diffuse_color.r = (float)materials[m].r / 255.0;
		diffuse_color.g = (float)materials[m].g / 255.0;
		diffuse_color.b = (float)materials[m].b / 255.0;
		diffuse_color.a = (float)materials[m].a / 255.0;
		
		
		fwrite(&diffuse_color, sizeof(vec4_t), 1, f);			/* base color */
	}
	
	/* write vertex group information */
	for(i = 0; i < total_material_count; i++)
	{
		fwrite(&triangle_groups[i].vertex_count, sizeof(int), 1, f);	/* how many vertices in this group */
		m = triangle_groups[i].start;
		for(j = 0; j < triangle_groups[i].vertex_count; j++)
		{
			fwrite(&vertices[m + j].position, sizeof(vec3_t), 1, f);	/* positions */
		}
	}
	
	/* normals are written contiguously, as they don't get linked
	to any triangle group */
	for(i = 0; i < total_material_count; i++)
	{
		m = triangle_groups[i].start;
		for(j = 0; j < triangle_groups[i].vertex_count; j++)
		{
			fwrite(&vertices[m + j].normal, sizeof(vec3_t), 1, f);		
		}
	}
	
	
	/* UVs are written contiguously, as they don't get linked
	to any triangle group */
	for(i = 0; i < total_material_count; i++)
	{
		m = triangle_groups[i].start;
		for(j = 0; j < triangle_groups[i].vertex_count; j++)
		{
			fwrite(&vertices[m + j].tex_coord, sizeof(vec2_t), 1, f);
		}
	}
	
	for(i = 0; i < light_count; i++)
	{
		strcpy(light_name, light_names[i]);
		
		fwrite(light_name, MAX_LIGHT_NAME_LEN, 1, f);									/* light name */
		fwrite(&light_positions[i].orientation, sizeof(mat3_t), 1, f);					/* light orientation */
		fwrite(&light_positions[i].position, sizeof(vec3_t), 1, f);						/* light position */
		
		diffuse_color.r = (float)light_params[i].r / 255.0;
		diffuse_color.g = (float)light_params[i].g / 255.0;
		diffuse_color.b = (float)light_params[i].b / 255.0;
		
		light_radius = LIGHT_MAX_RADIUS * ((float)light_params[i].radius / 0xffff);
		light_energy = LIGHT_MAX_ENERGY * ((float)light_params[i].energy / 0xffff);
		
		fwrite(&diffuse_color, sizeof(vec3_t), 1, f);									/* light color */
		fwrite(&light_radius, sizeof(float), 1, f);										/* light radius */
		fwrite(&light_energy, sizeof(float), 1, f);										/* light energy */
	}

	collision_GenerateCollisionGeometry();
	
	
	
	fwrite(&collision_geometry_vertice_count, sizeof(int), 1, f);
	fwrite(collision_geometry_positions, sizeof(vec3_t), collision_geometry_vertice_count, f);
	//fwrite(collision_geometry_quantized_normals, sizeof(int), collision_geometry_vertice_count, f);
	fwrite(collision_geometry_normals, sizeof(vec3_t), collision_geometry_vertice_count, f);
	
	
	
	
	
	fclose(f);
	
	
	
	free(triangle_groups);
	free(vertices);
	
	#endif
	
}








