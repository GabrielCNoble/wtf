#include <stdio.h>

#include "GL\glew.h"

#include "matrix.h"
#include "vector.h"

#include "brush.h"
#include "editor.h"
#include "camera.h"
#include "input.h"
#include "engine.h"
#include "brush.h"
#include "l_main.h"
#include "shader.h"
#include "material.h"
#include "texture.h"
//#include "collision.h"
#include "gui.h"
#include "bsp_cmp.h"
#include "bsp.h"
#include "pvs.h"
#include "indirect.h"
#include "player.h"

#include "r_main.h"
#include "r_editor.h"

#include "ed_ui.h"
#include "ed_proj.h"

static camera_t *editor_camera;
static int editor_camera_index;

/* from r_main.c */
extern int r_width;
extern int r_height;

extern int r_window_width;
extern int r_window_height;

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

/* from player.c */
extern player_t *active_player;

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
int brush_dist_shader;
int max_selections;
int selection_count;
pick_record_t *selections;
vec3_t cursor_3d_position;
vec3_t handle_3d_position;
int bm_handle_3d_flags;
int handle_3d_position_mode;
int handle_3d_mode;

int editor_state = EDITOR_EDITING;

int pie_player_index;

float editor_camera_yaw = 0.0;
float editor_camera_pitch = 0.0;

char *handle_3d_mode_str[] = 
{
	"Translation",
	"Rotation",
	"Scale",
};


void button0_callback(widget_t *widget)
{
	printf("fuck ");
	//engine_SetEngineState(ENGINE_QUIT);
}

void button1_callback(widget_t *widget)
{
	printf("you\n");
	//engine_SetEngineState(ENGINE_QUIT);
}

void checkbox_callback(widget_t *widget)
{
	checkbox_t *checkbox = (checkbox_t *)widget;
	
	if(checkbox->bm_checkbox_flags & CHECKBOX_CHECKED)
	{
		printf("check\n");
	}
	else
	{
		printf("uncheck\n");
	}
	//printf("haha\n");
}


void editor_Init()
{
	mat3_t r = mat3_t_id();
	//mat3_t r;
	
	//mat3_t_rotate(&r, vec3(1.0, 0.0, 0.0), -0.2, 1);
	//mat3_t_rotate(&r, vec3(0.0, 1.0, 0.0), 0.2, 0);
	
	editor_camera_yaw = 0.2;
	editor_camera_pitch = -0.15;
	
	editor_camera_index = camera_CreateCamera("editor_camera", vec3(12.0, 10.0, 15.0), &r, 0.68, r_width, r_height, 0.1, 500.0, CAMERA_UPDATE_ON_RESIZE);
	editor_camera = camera_GetCameraByIndex(editor_camera_index);	
	camera_SetCameraByIndex(editor_camera_index);
	
	camera_PitchYawCamera(editor_camera, editor_camera_yaw, editor_camera_pitch);
	camera_ComputeWorldToCameraMatrix(editor_camera);
	
	//r = mat3_t_id();
	
	brush_Init();
	
	
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
	input_RegisterKey(SDL_SCANCODE_P);
	input_RegisterKey(SDL_SCANCODE_R);
	input_RegisterKey(SDL_SCANCODE_G);
	
	input_RegisterKey(SDL_SCANCODE_DELETE);
	
	
	//renderer_RegisterCallback(bsp_DrawPortals, POST_SHADING_STAGE_CALLBACK);
	//renderer_RegisterFunction(indirect_DrawVolumes);
	//renderer_RegisterCallback(renderer_DrawBrushes, POST_SHADING_STAGE_CALLBACK);
	//renderer_RegisterCallback(bsp_DrawPolygons, POST_SHADING_STAGE_CALLBACK);
	//renderer_RegisterCallback(bsp_DrawExpandedBrushes, POST_SHADING_STAGE_CALLBACK);
	//renderer_RegisterCallback(bsp_DrawBevelEdges, POST_SHADING_STAGE_CALLBACK);
	//renderer_RegisterCallback(renderer_DrawLeaves, POST_SHADING_STAGE_CALLBACK);
	//renderer_RegisterFunction(renderer_DrawLightBoxes);
	//renderer_RegisterFunction(renderer_DrawSelectedLightLeaves);
	//renderer_RegisterCallback(renderer_DrawGrid, POST_SHADING_STAGE_CALLBACK);
	//renderer_RegisterCallback(renderer_DrawLights, POST_SHADING_STAGE_CALLBACK);
	//renderer_RegisterCallback(renderer_DrawSelected, POST_SHADING_STAGE_CALLBACK);
	//renderer_RegisterCallback(renderer_DrawCursors, POST_SHADING_STAGE_CALLBACK);
	
	renderer_RegisterCallback(renderer_EditorDraw, POST_SHADING_STAGE_CALLBACK);
	
	
	
	
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, r_width, r_height, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindTexture(GL_TEXTURE_2D, pick_framebuffer_depth_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, r_width,r_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, r_width, r_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	
	glBindTexture(GL_TEXTURE_2D, cursor_depth_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, r_width, r_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cursor_color_texture_id, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, cursor_depth_texture_id, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, cursor_depth_texture_id, 0);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
	
	
	editor_InitUI();
	
	
	max_selections = 64;
	selection_count = 0;
	selections = malloc(sizeof(pick_record_t ) * max_selections);
	
	
	cursor_3d_position = vec3(0.0, 0.0, 0.0);
	handle_3d_position = vec3(0.0, 0.0, 0.0);
//	mat3_t_rotate(&r, vec3(0.0, 1.0, 0.0), 0.25, 1);
	//brush_CreateBrush(vec3(0.0, 6.0, 0.0), &r, vec3(1.0, 2.0, 1.0), BRUSH_CYLINDER);
	
	int i;
	
	/*for(i = 0; i < 5; i++)
	{
		brush_CreateBrush(vec3(0.0, 0.2 + 0.25 * i, i * 0.5), &r, vec3(10.0 - i * 0.1, 0.5, 10.0), BRUSH_CUBE);
	}*/
	
	

	//brush_CreateBrush(vec3(0.0, 0.0, 0.0), &r, vec3(10.0, 2.0, 10.0), BRUSH_CUBE);
	
	//light_CreateLight("light0", &r, vec3(0.0, 12.0, 0.0), vec3(1.0, 1.0, 1.0), 35.0, 20.0);
	//brush_CreateBrush(vec3(0.0, 0.0, 0.0), &r, vec3(1.0, 1.0, 1.0), BRUSH_CUBE);
/*	brush_CreateBrush(vec3(0.0, 0.0, 0.0), &r, vec3(25.0, 0.25, 25.0), BRUSH_CUBE);*/
	
	
	
	 
	#if 0
	
	brush_CreateBrush(vec3(0.0, 10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, -10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(10.0, 0.0, 0.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-10.0, 0.0,0.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 0.0, 10.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 0.0,-10.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	
	
	mat3_t_rotate(&r, vec3(0.0, 1.0, 0.0), 0.25, 1);
	brush_CreateBrush(vec3(4.0, 0.0, 0.0), &r, vec3(1.0, 50.0, 1.0), BRUSH_CYLINDER);
	brush_CreateBrush(vec3(-4.0, 0.0, 0.0), &r, vec3(1.0, 50.0, 1.0), BRUSH_CYLINDER);
	brush_CreateBrush(vec3(0.0, 0.0, 4.0), &r, vec3(1.0, 50.0, 1.0), BRUSH_CYLINDER);
	brush_CreateBrush(vec3(0.0, 0.0, -4.0), &r, vec3(1.0, 50.0, 1.0), BRUSH_CYLINDER);
	brush_CreateBrush(vec3(5.0, 0.0,-5.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	
	#endif
	
	//brush_CreateBrush(vec3(2.0, 0.0, 0.0), &r, vec3(1.0, 1.0, 1.0), BRUSH_CUBE);
	//brush_CreateBrush(vec3(-2.0, 0.0, 0.0), &r, vec3(1.0, 1.0, 1.0), BRUSH_CUBE);
	//brush_CreateBrush(vec3(0.0, 0.0, 4.0), &r, vec3(4.0, 1.0, 1.0), BRUSH_CUBE);
	
	
	//brush_CreateBrush(vec3(0.0, -2.5, 0.0), &r, vec3(10.0, 0.5, 10.0), BRUSH_CUBE);
	//brush_CreateBrush(vec3(0.0, 2.5, -10.0), &r, vec3(10.0, 0.5, 10.0), BRUSH_CUBE);
	/*for(i = 0; i < 1; i++)
	{
		light_CreateLight("light0", &r, vec3(5.0, 5.0, 0.0), vec3(1.0, 1.0, 1.0), 40.0, 20.0);
	}*/
	
	player_CreateSpawnPoint(vec3(0.0, 20.0, 0.0), "wow");
	pie_player_index = player_CreatePlayer("pie player", vec3(0.0, 20.0, 0.0), &r);
	
	#if 0
	
	light_CreateLight("light0", &r, vec3(6.0, 12.0, -4.0), vec3(1.0, 1.0, 1.0), 35.0, 20.0, LIGHT_GENERATE_SHADOWS);
	//light_CreateLight("light0", &r, vec3(-6.0, 12.0, -4.0), vec3(1.0, 1.0, 1.0), 35.0, 20.0);
	//light_CreateLight("light1", &r, vec3(4.0, 4.0, 4.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0);
	//light_CreateLight("light2", &r, vec3(-4.0, 4.0, -4.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0);
	/*light_CreateLight("light3", &r, vec3(-4.0, 4.0, 4.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0);*/
	
	//brush_CreateBrush(vec3(0.0, 0.0, 0.0), &r, vec3(1.0, 1.0, 1.0), BRUSH_CUBE);
	
	
	brush_CreateBrush(vec3(0.0, 0.0, 0.0), &r, vec3(25.0, 1.0, 25.0), BRUSH_CUBE);
	//mat3_t_rotate(&r, vec3(0.0, 1.0, 0.0), 0.25, 1);
	//brush_CreateBrush(vec3(0.0, 0.0, 0.0), &r, vec3(1.0, 20.0, 1.0), BRUSH_CUBE);
	//brush_CreateBrush(vec3(0.0, 0.0, 0.0), &r, vec3(1.0, 20.0, 1.0), BRUSH_CYLINDER);
	brush_CreateBrush(vec3(0.0, 1.0, 1.0), &r, vec3(5.0, 1.0, 5.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 2.0, 2.0), &r, vec3(5.0, 1.0, 5.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 3.0, 3.0), &r, vec3(5.0, 1.0, 5.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 4.0, 4.0), &r, vec3(5.0, 1.0, 5.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 5.0, 5.0), &r, vec3(5.0, 1.0, 5.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 6.0, 6.0), &r, vec3(5.0, 1.0, 5.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 7.0, 7.0), &r, vec3(5.0, 1.0, 5.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 8.0, 8.0), &r, vec3(5.0, 1.0, 5.0), BRUSH_CUBE);
	
	/*mat3_t_rotate(&r, vec3(1.0, 0.0, 0.0), 0.1, 1);
	brush_CreateBrush(vec3(5.0, 1.0, 8.0), &r, vec3(5.0, 1.0, 15.0), BRUSH_CUBE);*/
	
	/*brush_CreateBrush(vec3(0.0, 9.0, 26.0), &r, vec3(5.0, 1.0, 20.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(25.0, 10.0, 26.0), &r, vec3(20.0, 1.0, 5.0), BRUSH_CUBE);
	
	
	
	brush_CreateBrush(vec3(40.0, 11.0, 26.0), &r, vec3(5.0, 1.0, 5.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(41.0, 12.0, 26.0), &r, vec3(5.0, 1.0, 5.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(42.0, 13.0, 26.0), &r, vec3(5.0, 1.0, 5.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(43.0, 14.0, 26.0), &r, vec3(5.0, 1.0, 5.0), BRUSH_CUBE);
	
	brush_CreateBrush(vec3(43.0, 14.0, 26.0), &r, vec3(50.0, 10.0, 1.0), BRUSH_CUBE);*/
	
	
	
	#endif
	
	/*mat3_t_rotate(&r, vec3(0.0, 1.0, 0.0), 0.25, 1);
	brush_CreateBrush(vec3(15.0, 0.0, 0.0), &r, vec3(1.0, 50.0, 1.0), BRUSH_CYLINDER);*/
	//brush_CreateBrush(vec3(0.0, 0.75, 0.0), &r, vec3(1.0, 2.0, 1.0), BRUSH_CUBE);
	
	
	/*brush_CreateBrush(vec3(0.0, -99.9, 0.0), &r, vec3(100.0, 1.0, 100.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(99.9, 0.0, 0.0), &r, vec3(1.0, 100.0, 100.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-99.9, 0.0, 0.0), &r, vec3(1.0, 100.0, 100.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 0.0, 99.9), &r, vec3(100.0, 100.0, 1.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 0.0, -99.9), &r, vec3(100.0, 100.0, 1.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 99.9, 0.0), &r, vec3(100.0, 1.0, 100.0), BRUSH_CUBE);*/
	
	
	/*brush_CreateBrush(vec3(0.0, -10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	
	brush_CreateBrush(vec3(10.0, 0.0, 0.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-10.0, 0.0, 0.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 0.0, 10.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 0.0, -10.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	
	brush_CreateBrush(vec3(0.0, 10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);*/
	
	//player_CreatePlayer("player", vec3(0.0, 5.0 ,0.0), &r);
	
	#if 0
	
	light_CreateLight("light0", &r, vec3(16.0, 0.0, 8.0), vec3(1.0, 0.0, 1.0), 35.0, 20.0, LIGHT_GENERATE_SHADOWS);
	//light_CreateLight("light0", &r, vec3(16.0, 0.0, -8.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0);
	light_CreateLight("light0", &r, vec3(16.0, 0.0, 20.0), vec3(1.0, 1.0, 0.0), 35.0, 20.0, LIGHT_GENERATE_SHADOWS);
	light_CreateLight("light0", &r, vec3(16.0, 0.0, -20.0), vec3(0.5, 0.5, 1.0), 35.0, 80.0, LIGHT_GENERATE_SHADOWS);
	
	light_CreateLight("light0", &r, vec3(4.0, 0.0, 8.0), vec3(1.0, 0.63, 0.24), 35.0, 20.0, LIGHT_GENERATE_SHADOWS);	
	light_CreateLight("light0", &r, vec3(4.0, 0.0, -8.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0, LIGHT_GENERATE_SHADOWS);
	light_CreateLight("light0", &r, vec3(4.0, 0.0, 20.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0, LIGHT_GENERATE_SHADOWS);
	light_CreateLight("light0", &r, vec3(4.0, 0.0, -20.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0, LIGHT_GENERATE_SHADOWS);
	
	light_CreateLight("light0", &r, vec3(-8.0, 0.0, 8.0), vec3(0.3, 0.4, 1.0), 35.0, 20.0, LIGHT_GENERATE_SHADOWS);	
	light_CreateLight("light0", &r, vec3(-8.0, 0.0, -8.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0, LIGHT_GENERATE_SHADOWS);	
	light_CreateLight("light0", &r, vec3(-8.0, 0.0, 20.0), vec3(0.2, 1.0, 0.3), 35.0, 20.0, LIGHT_GENERATE_SHADOWS);
	light_CreateLight("light0", &r, vec3(-8.0, 0.0, -20.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0, LIGHT_GENERATE_SHADOWS);
	
	light_CreateLight("light0", &r, vec3(-20.0, 0.0, 8.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0, LIGHT_GENERATE_SHADOWS);	
	light_CreateLight("light0", &r, vec3(-20.0, 0.0, -8.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0, LIGHT_GENERATE_SHADOWS);	
	light_CreateLight("light0", &r, vec3(-20.0, 0.0, 20.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0, LIGHT_GENERATE_SHADOWS);
	light_CreateLight("light0", &r, vec3(-20.0, 0.0, -20.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0, LIGHT_GENERATE_SHADOWS);
	
	
	light_CreateLight("light0", &r, vec3(16.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0, LIGHT_GENERATE_SHADOWS);	
	light_CreateLight("light0", &r, vec3(4.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0, LIGHT_GENERATE_SHADOWS);	
	light_CreateLight("light0", &r, vec3(-8.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0, LIGHT_GENERATE_SHADOWS);
	light_CreateLight("light0", &r, vec3(-20.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), 15.0, 20.0, LIGHT_GENERATE_SHADOWS);
	
	
	/*light_CreateLight("light0", &r, vec3(-55.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), 20.0, 20.0);
	light_CreateLight("light0", &r, vec3(-59.0, 0.0, -24.0), vec3(1.0, 1.0, 1.0), 20.0, 20.0);
	light_CreateLight("light0", &r, vec3(-80.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), 20.0, 20.0);
	light_CreateLight("light0", &r, vec3(-80.0, 0.0, -25.0), vec3(1.0, 1.0, 1.0), 20.0, 20.0);
	light_CreateLight("light0", &r, vec3(-80.0, 0.0, -39.0), vec3(1.0, 1.0, 1.0), 20.0, 20.0);*/
	
	
	/* walls */
	
	/*brush_CreateBrush(vec3(-40.0, 0.0, 10.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(-40.0, 0.0, -10.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, 0.0, 10.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, 0.0, -10.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(-80.0, 0.0, 10.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	
	brush_CreateBrush(vec3(-40.0, 10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-40.0, -10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, 10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, -10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-80.0, 10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-80.0, -10.0, 0.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	
	
	
	brush_CreateBrush(vec3(-90.0, 0.0, 0.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-90.0, 0.0, -20.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-50.0, 0.0, -20.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-90.0, 0.0, -40.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-50.0, 0.0, -40.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	
	
	brush_CreateBrush(vec3(-80.0, 10.0, -20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, 10.0, -20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-80.0, 10.0, -40.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, 10.0, -40.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	
	brush_CreateBrush(vec3(-80.0, -10.0, -20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, -10.0, -20.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-80.0, -10.0, -40.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, -10.0, -40.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, 0.0, -50.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(-80.0, 0.0, -50.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	
	brush_CreateBrush(vec3(-80.0, 0.0, -50.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	
	brush_CreateBrush(vec3(-90.0, 0.0, -60.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-50.0, 0.0, -60.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-90.0, 0.0, -80.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-50.0, 0.0, -80.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-90.0, -20.0, -60.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-50.0, -20.0, -60.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-90.0, -20.0, -80.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-50.0, -20.0, -80.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	
	
	brush_CreateBrush(vec3(-80.0, -20.0, -50.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, -20.0, -50.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	
	brush_CreateBrush(vec3(-80.0, -30.0, -80.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-80.0, -30.0, -60.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, -30.0, -80.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, -30.0, -60.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	
	
	brush_CreateBrush(vec3(-80.0, 10.0, -80.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-80.0, 10.0, -60.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, 10.0, -80.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, 10.0, -60.0), &r, vec3(10.0, 0.25, 10.0), BRUSH_CUBE);
	
	
	brush_CreateBrush(vec3(-80.0, -20.0, -90.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, -20.0, -90.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	
	brush_CreateBrush(vec3(-80.0, 0.0, -90.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(-60.0, 0.0, -90.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);*/
	
	//light_CreateLight("light0", &r, vec3(-70.0, -10.0, -70.0), vec3(1.0, 1.0, 1.0), 60.0, 20.0);
	
	
	
	
	
	
	brush_CreateBrush(vec3(30.0, 0.0, 0.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(30.0, 0.0, 20.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-30.0, 0.0, 20.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(30.0, 0.0, -20.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-30.0, 0.0, -20.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(-30.0, 0.0, 0.0), &r, vec3(0.25, 10.0, 10.0), BRUSH_CUBE);
	
	
	brush_CreateBrush(vec3(0.0, 0.0, 30.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(0.0, 0.0, -30.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(-20.0, 0.0, 30.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(-20.0, 0.0, -30.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(20.0, 0.0, 30.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	brush_CreateBrush(vec3(20.0, 0.0, -30.0), &r, vec3(10.0, 10.0, 0.25), BRUSH_CUBE);
	
	
	
	brush_CreateBrush(vec3(29.0, -6.0, 0.0), &r, vec3(5.0, 5.0, 10.0), BRUSH_CUBE);
	brush_CreateBrush(vec3(29.0, 6.0, 0.0), &r, vec3(5.0, 5.0, 10.0), BRUSH_CUBE);
	
	
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
	
	
	/*mat3_t_rotate(&r, vec3(0.0, 1.0, 0.0), 0.25, 1);
	brush_CreateBrush(vec3(-40.0, 0.0, 0.0), &r, vec3(1.0, 50.0, 1.0), BRUSH_CYLINDER);
	brush_CreateBrush(vec3(-60.0, 0.0, 0.0), &r, vec3(1.0, 50.0, 1.0), BRUSH_CYLINDER);
	brush_CreateBrush(vec3(-80.0, 0.0, 0.0), &r, vec3(1.0, 50.0, 1.0), BRUSH_CYLINDER);*/
	
	
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
	
	
	light_CreateLight("light12", &r, vec3(0.0, 2.5, -4.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light12", &r, vec3(0.0, 7.5, -4.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light12", &r, vec3(0.0, 12.5, -4.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light12", &r, vec3(0.0, 17.5, -4.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	/*light_CreateLight("light12", &r, vec3(0.0, 22.5, -4.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light12", &r, vec3(0.0, 27.5, -4.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light12", &r, vec3(0.0, 32.5, -4.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light12", &r, vec3(0.0, 37.5, -4.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light12", &r, vec3(0.0, 42.5, -4.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light12", &r, vec3(0.0, 47.5, -4.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light12", &r, vec3(0.0, 52.5, -4.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light12", &r, vec3(0.0, 57.5, -4.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);*/
	
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
	
	/*mat3_t_rotate(&r, vec3(0.0, 1.0, 0.0), 0.25, 1);
	brush_CreateBrush(vec3(0.0, 0.0, 0.0), &r, vec3(1.0, 50.0, 1.0), BRUSH_CYLINDER);*/
	
	
	
	
	
	/*light_CreateLight("light0", &r, vec3(0.0, 0.0, 24.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
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
	light_CreateLight("light11", &r, vec3(0.0, 60.0, -24.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);*/
	
	/*light_CreateLight("light12", &r, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light13", &r, vec3(0.0, 20.0, 0.0), vec3(1.0, 0.0, 0.0), 10.0, 20.0);
	light_CreateLight("light14", &r, vec3(0.0, 40.0, 0.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light15", &r, vec3(0.0, 60.0, 0.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light15", &r, vec3(0.0, 80.0, 0.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);
	light_CreateLight("light15", &r, vec3(0.0, 100.0, 0.0), vec3(1.0, 1.0, 1.0), 10.0, 20.0);*/
	#endif 
	
	/*widget_t *w = gui_CreateWidget("menu", 0, 0, 200, 50);
	widget_bar_t *bar = gui_AddWidgetBar(w, "bar", 0, 0, 300, 20, WIDGET_BAR_FIXED_SIZE);
	
	dropdown_t *dropdown = gui_CreateDropdown("dropdown", "wow", 0, 0, 80, 0, NULL);
	gui_AddOption(dropdown, "option0", "option0");
	gui_AddOption(dropdown, "option1", "option1");
	gui_AddOption(dropdown, "option2", "option2");
	gui_AddWidgetToBar((widget_t *)dropdown, bar);
	
	dropdown = gui_CreateDropdown("dropdown", "wow1", 0, 0, 80, 0, NULL);
	gui_AddOption(dropdown, "option0", "option0");
	gui_AddOption(dropdown, "option1", "option1");
	gui_AddOption(dropdown, "option2", "option2");
	gui_AddOption(dropdown, "option3", "option3");
	gui_AddWidgetToBar((widget_t *)dropdown, bar);
	
	dropdown = gui_CreateDropdown("dropdown", "wow2", 0, 0, 80, 0, NULL);
	gui_AddOption(dropdown, "option0", "option0");
	gui_AddOption(dropdown, "option1", "option1");
	gui_AddOption(dropdown, "option2", "option2");
	gui_AddWidgetToBar((widget_t *)dropdown, bar);*/
	
	/*button_t *button = gui_CreateButton("button0", 0, 0, 30, 30, BUTTON_TOGGLE, NULL);
	gui_AddWidgetToBar((widget_t *)button, bar);
	
	button = gui_CreateButton("button1", 0, 0, 30, 30, BUTTON_TOGGLE, NULL);
	gui_AddWidgetToBar((widget_t *)button, bar);
	
	button = gui_CreateButton("button2", 0, 0, 30, 30, BUTTON_TOGGLE, NULL);
	gui_AddWidgetToBar((widget_t *)button, bar);
	
	button = gui_CreateButton("button3", 0, 0, 30, 30, BUTTON_TOGGLE, NULL);
	gui_AddWidgetToBar((widget_t *)button, bar);
	
	button = gui_CreateButton("button4", 0, 0, 60, 30, BUTTON_TOGGLE, NULL);
	gui_AddWidgetToBar((widget_t *)button, bar);*/
	
	
	//dropdown_t *dropdown = gui_AddDropDown(w, "dropdown0", 0, 0, 150, 0, NULL);
	/*widget_t *w = gui_CreateWidget("widget0", 0, 0, 400, 200);
	dropdown_t *dropdown = gui_AddDropDown(w, "dropdown0", 0, 0, 150, 0, NULL);
	gui_AddOption(dropdown, "option0", "test0");
	gui_AddOption(dropdown, "option1", "test1");
	gui_AddOption(dropdown, "option2", "test2");
	gui_AddOption(dropdown, "option3", "test3");
	gui_AddOption(dropdown, "option4", "test4");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 1, "option0", "wow0");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 1, "option1", "wow1");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 1, "option2", "wow2");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 1, "option3", "wow3");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 1, "option4", "wow4");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 1, "option5", "wow5");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 1, "option6", "wow6");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 1, "option7", "wow7");
	
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 2, "option0", "bleh0");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 2, "option1", "bleh1");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 2, "option2", "bleh2");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 2, "option3", "bleh3");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 2, "option4", "bleh4");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 2, "option5", "bleh5");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 2, "option6", "bleh6");
	gui_NestleOption((option_list_t *)dropdown->widget.nestled, 2, "option7", "bleh7");*/
	/*gui_AddOption(dropdown, "option3");
	gui_AddOption(dropdown, "option4");
	gui_AddOption(dropdown, "option5");
	gui_AddOption(dropdown, "option6");
	gui_AddOption(dropdown, "option7");*/
	
	/*dropdown = gui_AddDropDown(w, "dropdown0", 25, -23, 150, 0, NULL);
	gui_AddOption(dropdown, "option0");
	gui_AddOption(dropdown, "option1");
	gui_AddOption(dropdown, "option2");
	gui_AddOption(dropdown, "option3");*/
	//gui_AddOption(dropdown, "option1");
	//gui_AddOption(dropdown, "option2");
	
	//gui_AddButton(w, "button0", 0, -20, 50, 50, 0, button0_callback);
	//gui_AddCheckBox(w, 0, 0, 16, 16, 0, checkbox_callback);
	//gui_AddButton(w, "button1", 100, 0, 50, 50, 0, button1_callback);
	//gui_CreateWidget("widget1", -100.0, 0.0, 400.0, 50.0);
	
		
	bm_handle_3d_flags = 0;
	handle_3d_position_mode = HANDLE_3D_MEDIAN_POINT;
	handle_3d_mode = HANDLE_3D_TRANSLATION;
	
	texture_SetPath("textures/env");
		
	/*texture_LoadCubeTexture("textures\\env\\aliencube_pos_x.jpg;"
					        "textures\\env\\aliencube_neg_x.jpg;"
					        "textures\\env\\aliencube_pos_y.jpg;"
					        "textures\\env\\aliencube_neg_y.jpg;"
					        "textures\\env\\aliencube_pos_z.jpg;"
					        "textures\\env\\aliencube_neg_z.jpg;", "env");*/
	
	//texture_LoadCubeTexture("aliencube.jpg", "env");				        
	
	
	/*texture_LoadCubeTexture("textures\\env\\cliffrt.bmp;"
					        "textures\\env\\clifflf.bmp;"
					        "textures\\env\\cliffup.bmp;"
					        "textures\\env\\cliffdn.bmp;"
					        "textures\\env\\cliffbk.bmp;"
					        "textures\\env\\cliffft.bmp;", "env");*/
					        
	
	renderer_RegisterCallback(editor_WindowResizeCallback, RENDERER_RESOLUTION_CHANGE_CALLBACK);				        
	
}

void editor_Finish()
{
	
	brush_Finish();
	
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
	//static float yaw = 0.0;
	//static float pitch = 0.0;
	
	static float r = 0.0;
	
	static int playing = 0;
	
	//printf("%f\n", delta_time);
	
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
	

	/*if(input_GetKeyStatus(SDL_SCANCODE_ESCAPE) & KEY_PRESSED)
	{
		engine_SetEngineState(ENGINE_QUIT);
		return;
	}*/
 	
 	translation = vec3(0.0, 0.0, 0.0);
 		
	if(bm_mouse & MOUSE_MIDDLE_BUTTON_CLICKED && editor_state == EDITOR_EDITING)
	{	
		engine_SetEngineState(ENGINE_PLAYING);
		
		/* avoid the camera to snap at a random direction when
		the engine change states... */
		if(last_mouse_x || last_mouse_y)
		{
			normalized_mouse_x = 0.0;
			normalized_mouse_y = 0.0;
		}
		
		
		editor_camera_yaw -= (normalized_mouse_x) * 0.5;
		editor_camera_pitch += (normalized_mouse_y) * 0.5;
		
		//printf("%f %f\n", editor_camera_yaw, editor_camera_pitch);
		
		if(editor_camera_pitch > 0.5) editor_camera_pitch = 0.5;
		else if(editor_camera_pitch < -0.5) editor_camera_pitch = -0.5;
			
		if(editor_camera_yaw > 1.0) editor_camera_yaw = -1.0 + (editor_camera_yaw - 1.0);
		else if(editor_camera_yaw < -1.0) editor_camera_yaw = 1.0 + (editor_camera_yaw + 1.0);
		
		camera_PitchYawCamera(editor_camera, editor_camera_yaw, editor_camera_pitch);
		
		forward_vector = editor_camera->world_orientation.f_axis;
		right_vector = editor_camera->world_orientation.r_axis;
				
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
			velocity.y = 0.08;
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
		
		translation.x *= camera_speed * delta_time * 0.045;
		translation.y *= camera_speed * delta_time * 0.045;
		translation.z *= camera_speed * delta_time * 0.045;
		
		velocity.x = translation.x;
		velocity.y = translation.y;
		//velocity.y -= 0.0098 * delta_time * 0.01;
		velocity.z = translation.z;
		
		//bsp_Collide(&editor_camera->world_position, translation);
		
		//bsp_Move(&editor_camera->world_position, &velocity);
		
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
		 
		camera_TranslateCamera(editor_camera, velocity, 1.0, 0);
		
		
		//editor_camera->world_position.y += 1.5;
		
		//camera_ComputeWorldToCameraMatrix(editor_camera);
		
		//editor_camera->world_position.y -= 1.5;
		
	}
	else
	{
		//if(!playing)
		//{
		if(editor_state == EDITOR_EDITING)
		{
			engine_SetEngineState(ENGINE_PAUSED);
		}
		
		editor_ProcessMouse(delta_time);
		editor_ProcessKeyboard(delta_time);
			
			/*if(input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED)
			{
				if(input_GetKeyStatus(SDL_SCANCODE_C) & KEY_JUST_PRESSED)
				{
					bsp_CompileBsp(0);
					//indirect_BuildVolumes();
				}
				else if(input_GetKeyStatus(SDL_SCANCODE_S) & KEY_JUST_PRESSED)
				{
					//renderer_Fullscreen(1);
					//renderer_SetWindowSize(1920, 1080);
				
				}
				else if(input_GetKeyStatus(SDL_SCANCODE_K) & KEY_JUST_PRESSED)
				{
					b_draw_brushes ^= 1;
					//bsp_NextPortal();
				}
				else if(input_GetKeyStatus(SDL_SCANCODE_SPACE) & KEY_JUST_PRESSED)
				{
					editor_OpenAddToWorldMenu(r_window_width * normalized_mouse_x * 0.5, r_window_height * normalized_mouse_y * 0.5);
				}
			}*/
		//}
		
		/*if(input_GetKeyStatus(SDL_SCANCODE_P) & KEY_PRESSED)
		{
			player_SetPlayerAsActive(player_GetPlayer("player"));
			engine_SetEngineState(ENGINE_PLAYING);
			playing = 1;
		}*/
			
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

void editor_RotateSelections(vec3_t axis, float amount)
{
	int i;
	int c = selection_count;
	brush_t *brush;
	light_position_t *light;
	
	vec3_t v;
	
	mat3_t rot;
	
	mat3_t_rotate(&rot, axis, amount, 1);
	
	for(i = 0; i < c; i++)
	{
		switch(selections[i].type)
		{
			case PICK_BRUSH:
				
				brush = &brushes[selections[i].index0];
				
				brush_RotateBrush(brush, axis, amount);
				v = brush->position;
				
				v.x -= handle_3d_position.x;
				v.y -= handle_3d_position.y;
				v.z -= handle_3d_position.z;
				
				mat3_t_vec3_t_mult(&rot, &v);
				//v = MultiplyVector3(&rot, v);
				
				v.x += handle_3d_position.x;
				v.y += handle_3d_position.y;
				v.z += handle_3d_position.z;
				
				
				v.x = v.x - brush->position.x;
				v.y = v.y - brush->position.y;
				v.z = v.z - brush->position.z;
				
				brush_TranslateBrush(brush, v);
				
				//brush->position = v;
			break;
			
			case PICK_LIGHT:
				light = &light_positions[selections[i].index0];
				
				v = light->position;
				
				v.x -= handle_3d_position.x;
				v.y -= handle_3d_position.y;
				v.z -= handle_3d_position.z;
				
				mat3_t_vec3_t_mult(&rot, &v);
				//v = MultiplyVector3(&rot, v);
				
				v.x += handle_3d_position.x;
				v.y += handle_3d_position.y;
				v.z += handle_3d_position.z;
				
				
				v.x = v.x - light->position.x;
				v.y = v.y - light->position.y;
				v.z = v.z - light->position.z;
				
				
				light_TranslateLight(selections[i].index0, v, 1.0);
			break;
		}
	}
}

void editor_ScaleSelections(vec3_t axis, float amount)
{
	int i;
	int c = selection_count;
	
	for(i = 0; i < c; i++)
	{
		switch(selections[i].type)
		{
			case PICK_BRUSH:
				brush_ScaleBrush(&brushes[selections[i].index0], axis, amount);
			break;
		}
	}
	
}

void editor_CopySelections()
{
	int i;
	int new_index;
	
	for(i = 0; i < selection_count; i++)
	{
		switch(selections[i].type)
		{
			case PICK_BRUSH:
				new_index = brush_CopyBrush(&brushes[selections[i].index0]);
				selections[i].index0 = new_index;
			break;
		}
	}
	
}

void editor_DeleteSelection()
{
	int i;
	
	if(!selection_count)
		return;
		
	
	for(i = 0; i < selection_count; i++)
	{
		switch(selections[i].type)
		{
			case PICK_LIGHT:
				light_DestroyLightIndex(selections[i].index0);
			break;
			
			case PICK_BRUSH:
				brush_DestroyBrush(&brushes[selections[i].index0]);
			break;
		}
	}
	
	editor_ClearSelection();
		
}


void editor_AddToWorld(int type, vec3_t position, mat3_t *orientation)
{
	
	/* when adding to the world, previous
	selections will be lost and the 3d handle
	will be set to the current selection... */
	editor_ClearSelection();
	
	switch(type)
	{
		case PICK_BRUSH:
		
		break;
		
		case PICK_LIGHT:
			light_CreateLight("light", orientation, position, vec3(1.0, 1.0, 1.0), 25.0, 20.0, LIGHT_GENERATE_SHADOWS);
		break;
		
		case PICK_SPAWN_POINT:
		
		break;
	}
}

void editor_EnablePicking()
{
	while(glGetError());
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pick_framebuffer_id);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, pick_framebuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, r_width, r_height);
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
	
	glViewport(0, 0, r_window_width, r_window_height);
	
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pick_framebuffer_id);
	//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	editor_EnablePicking();
	
	c = brush_count;
		
	for(i = 0; i < c; i++)
	{
		
		if(brushes[i].type == BRUSH_INVALID)
			continue;
		
		if(brushes[i].type == BRUSH_BOUNDS)
			continue;
		
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
		if(light_params[i].bm_flags & LIGHT_INVALID)
			continue;
		
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
	

/*	glDisable(GL_CULL_FACE);
	//glColor3f(1.0, 1.0, 1.0);
	//glBegin(GL_QUADS);
	
	for(i = 0; i < spawn_point_count; i++)
	{
		*(int *)&q[0] = PICK_LIGHT;
		*(int *)&q[1] = i + 1;
		q[2] = 0.0;
		q[3] = 0.0;	
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, q);
		
		pos = spawn_points[i].position;
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		
	}
	
	glEnd();
	glLineWidth(1.0);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4f(1.0, 1.0, 1.0, 0.1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);
	glBegin(GL_QUADS);
	
	for(i = 0; i < spawn_point_count; i++)
	{
		pos = spawn_points[i].position;
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		
	}
	
	glEnd();
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glPopMatrix();
	*/
	
	
	
	x = r_window_width * (normalized_mouse_x * 0.5 + 0.5);
	y = r_window_height * (normalized_mouse_y * 0.5 + 0.5);
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
	vec3_t v;
	
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
			case HANDLE_3D_SCALE:
				
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
				
				if(handle_3d_mode == HANDLE_3D_TRANSLATION)
				{
					glPointSize(16.0);
					glBegin(GL_POINTS);
					glColor3f(1.0, 0.0, 0.0);
					glVertex3f(right_vector.x, right_vector.y, right_vector.z);
					glColor3f(0.0, 1.0, 0.0);
					glVertex3f(up_vector.x, up_vector.y, up_vector.z);
					glColor3f(0.0, 0.0, 1.0);
					glVertex3f(forward_vector.x, forward_vector.y, forward_vector.z);
					glEnd();
				}
				else
				{
					#define SCALE_HANDLE_CUBE_EXTENT 0.08 * d
						
					glDisable(GL_CULL_FACE);
					glBegin(GL_QUADS);
					glColor3f(1.0, 0.0, 0.0);
						
					v.x = right_vector.x;
					v.y = right_vector.y;
					v.z = right_vector.z;
						
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
					
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
					
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
						
						
						
					glColor3f(0.0, 1.0, 0.0);
						
					v.x = up_vector.x;
					v.y = up_vector.y;
					v.z = up_vector.z;
						
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
						
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
						
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
						
						
					
					glColor3f(0.0, 0.0, 1.0);
						
					v.x = forward_vector.x;
					v.y = forward_vector.y;
					v.z = forward_vector.z;
						
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
						
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
						
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
					glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
					glEnd();
					glEnable(GL_CULL_FACE);
				}
							
				
							
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
			
		
		
		x = r_width * (normalized_mouse_x * 0.5 + 0.5);
		y = r_height * (normalized_mouse_y * 0.5 + 0.5);
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

void editor_Position3dCursor()
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
	
	mat4_t camera_to_world_matrix;
	
	vec4_t pos;
	
	float q[4];
	float z;
	float qr;
	float qt;
	
	gpu_BindGpuHeap();
	shader_UseShader(brush_dist_shader);	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	glViewport(0, 0, r_window_width, r_window_height);
	
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pick_framebuffer_id);
	//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	editor_EnablePicking();
	glClearColor(active_camera->frustum.zfar, active_camera->frustum.zfar, active_camera->frustum.zfar, active_camera->frustum.zfar);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	
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
	
	/*shader_UseShader(light_pick_shader);
	
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
	glPointSize(1.0);*/
	
	x = r_window_width * (normalized_mouse_x * 0.5 + 0.5);
	y = r_window_height * (normalized_mouse_y * 0.5 + 0.5);
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, q);
	
	//printf("dist: %f\n", q[0]);
	
	//printf("[%d %d %d %d]\n", *(int *)&q[0], *(int *)&q[1], *(int *)&q[2], *(int *)&q[3]);
	
	editor_DisablePicking();
	
	mat4_t_compose(&camera_to_world_matrix, &active_camera->world_orientation, active_camera->world_position);
	
	
	qr = active_camera->frustum.znear / active_camera->frustum.right;
	qt = active_camera->frustum.znear / active_camera->frustum.top;
	
	if(q[0] == active_camera->frustum.zfar)
	{
		z = -10.0;
	}
	else
	{
		z = -q[0];
	}
	
	pos.x = (normalized_mouse_x / qr) * (-z);
	pos.y = (normalized_mouse_y / qt) * (-z);
	pos.z = z;
	pos.w = 1.0;
		
	mat4_t_vec4_t_mult(&camera_to_world_matrix, &pos);
	
	
	cursor_3d_position = pos.vec3;
	
	
	/*pick_type = (*(int *)&q[0]);
	
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
	}*/
	
	//if(pick_type == PICK_LIGHT) printf("light!\n");
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


void editor_StartPIE()
{
	
	if(editor_state == EDITOR_EDITING)
	{
		editor_state = EDITOR_PIE; 
		player_SpawnPlayer(pie_player_index, 0);
		player_SetPlayerAsActiveIndex(pie_player_index);
		engine_SetEngineState(ENGINE_PLAYING);
	}
	
	
	
}

void editor_StopPIE()
{
	if(editor_state == EDITOR_PIE)
	{
		editor_state = EDITOR_EDITING;
		player_RemovePlayer(pie_player_index);
		camera_SetCamera(editor_camera);
		engine_SetEngineState(ENGINE_PAUSED);
		//printf("stop pie!\n");
		
		//printf("%d\n", active_player->bm_flags & PLAYER_IN_WORLD);
	}
	
}


void editor_WindowResizeCallback()
{
	//glGenFramebuffers(1, &pick_framebuffer_id);
	//glGenTextures(1, &pick_framebuffer_texture);
	//glGenTextures(1, &pick_framebuffer_depth_texture);
	
	glBindTexture(GL_TEXTURE_2D, pick_framebuffer_texture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, r_width, r_height, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindTexture(GL_TEXTURE_2D, pick_framebuffer_depth_texture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, r_width,r_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, pick_framebuffer_id);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pick_framebuffer_texture, 0);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, pick_framebuffer_depth_texture, 0);
	
	
	
	//glGenFramebuffers(1, &cursor_framebuffer_id);
	//glGenTextures(1, &cursor_color_texture_id);
	//glGenTextures(1, &cursor_depth_texture_id);
	
	
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cursor_framebuffer_id);
	
	glBindTexture(GL_TEXTURE_2D, cursor_color_texture_id);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, r_width, r_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	
	glBindTexture(GL_TEXTURE_2D, cursor_depth_texture_id);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, r_width, r_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	
	//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cursor_color_texture_id, 0);
	//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, cursor_depth_texture_id, 0);
	//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, cursor_depth_texture_id, 0);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}







