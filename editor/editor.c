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
extern int forward_pass_shader;
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
extern int spawn_point_count;
extern spawn_point_t *spawn_points;

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
int spawn_point_pick_shader;
int brush_dist_shader;
int max_selections;
int selection_count;
pick_record_t *selections;
vec3_t cursor_3d_position;
vec3_t handle_3d_position;
int bm_handle_3d_flags;
int handle_3d_position_mode;
int handle_3d_mode;


//light_position_t *selected_light_position;
//light_params_t *selected_light_params;


light_ptr_t selected_light;
brush_ptr_t selected_brush;



int selected_type = PICK_NONE;
int editor_state = EDITOR_EDITING;
int pie_player_index;

float editor_camera_yaw = 0.0;
float editor_camera_pitch = 0.0;

float editor_snap = 0.0;

int default_material;
//int red_default_material;

char *handle_3d_mode_strs[] = 
{
	"Translation",
	"Rotation",
	"Scale",
};

char *handle_3d_mode_str = "Translation";



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
	
	default_material = material_CreateMaterial("default_material", vec4(1.0, 1.0, 1.0, 1.0), 1.0, 1.0, forward_pass_shader, -1, -1);
	
	camera_PitchYawCamera(editor_camera, editor_camera_yaw, editor_camera_pitch);
	camera_ComputeWorldToCameraMatrix(editor_camera);
	
	r = mat3_t_id();
	
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
	
	
	max_selections = 1024;
	selection_count = 0;
	selections = malloc(sizeof(pick_record_t ) * max_selections);
	
	pie_player_index = player_CreatePlayer("pie player", vec3(0, 0, 0), &r);
	
	cursor_3d_position = vec3(0.0, 0.0, 0.0);
	handle_3d_position = vec3(0.0, 0.0, 0.0);
//	mat3_t_rotate(&r, vec3(0.0, 1.0, 0.0), 0.25, 1);
	//brush_CreateBrush(vec3(0.0, 6.0, 0.0), &r, vec3(1.0, 2.0, 1.0), BRUSH_CYLINDER);
	
	int i;
			
	bm_handle_3d_flags = 0;
	handle_3d_position_mode = HANDLE_3D_MEDIAN_POINT;
	handle_3d_mode = HANDLE_3D_TRANSLATION;
	
	texture_SetPath("textures/env");
	
	editor_SetProjectName("untitled.wtf");
		
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

void editor_RestartEditor()
{
	mat3_t r = mat3_t_id();
	
	editor_camera_yaw = 0.2;
	editor_camera_pitch = -0.15;
	
	editor_camera_index = camera_CreateCamera("editor_camera", vec3(12.0, 10.0, 15.0), &r, 0.68, r_width, r_height, 0.1, 500.0, CAMERA_UPDATE_ON_RESIZE);
	editor_camera = camera_GetCameraByIndex(editor_camera_index);	
	camera_SetCameraByIndex(editor_camera_index);
	
	default_material = material_CreateMaterial("default_material", vec4(1.0, 1.0, 1.0, 1.0), 1.0, 1.0, forward_pass_shader, -1, -1);
	
	camera_PitchYawCamera(editor_camera, editor_camera_yaw, editor_camera_pitch);
	camera_ComputeWorldToCameraMatrix(editor_camera);
		
	cursor_3d_position = vec3(0.0, 0.0, 0.0);
	handle_3d_position = vec3(0.0, 0.0, 0.0);

		
	bm_handle_3d_flags = 0;
	
	editor_Set3dHandleMode(HANDLE_3D_TRANSLATION);
	editor_Set3dHandlePivotMode(HANDLE_3D_MEDIAN_POINT);
	//handle_3d_position_mode = HANDLE_3D_MEDIAN_POINT;
	//handle_3d_mode = HANDLE_3D_TRANSLATION;
	
	editor_ClearSelection();
	
	editor_SetProjectName("untitled.wtf");
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
extern float insane_float0;
extern float insane_float1;
extern float insane_float2;

extern unsigned char insane_char;

void editor_Main(float delta_time)
{
	//static float yaw = 0.0;
	//static float pitch = 0.0;
	
	//static float r = 0.0;
	
	//static int playing = 0;
	
	//printf("%f\n", delta_time);
	
	mat3_t rot = mat3_t_id();
	
	camera_t *active_camera = camera_GetActiveCamera();
	
	vec3_t translation;
	vec3_t position;
	
	static vec3_t velocity = {0.0, 0.0, 0.0};
	
	vec3_t forward_vector;
	vec3_t right_vector;
	
	//int mouse_bm;
	
	//float intersection;
	//int i;
	
	//pick_record_t record;
	 	
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
		
		translation.x *= camera_speed * delta_time * 0.045;
		translation.y *= camera_speed * delta_time * 0.045;
		translation.z *= camera_speed * delta_time * 0.045;
		
		velocity.x = translation.x;
		velocity.y = translation.y;
		velocity.z = translation.z;
				 
		camera_TranslateCamera(editor_camera, velocity, 1.0, 0);		
	}
	else
	{

		if(editor_state == EDITOR_EDITING)
		{
			engine_SetEngineState(ENGINE_PAUSED);
		}
		
		editor_ProcessMouse(delta_time);
		editor_ProcessKeyboard(delta_time);
			
	}
	
	editor_ProcessUI();
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
	
	vec3_t pos;
	
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
	
	shader_UseShader(spawn_point_pick_shader);
	
	glDisable(GL_CULL_FACE);
	//glColor3f(1.0, 1.0, 1.0);
	//glBegin(GL_QUADS);
	
	for(i = 0; i < spawn_point_count; i++)
	{
		*(int *)&q[0] = PICK_SPAWN_POINT;
		*(int *)&q[1] = i + 1;
		q[2] = 0.0;
		q[3] = 0.0;	
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, q);
		
		
		
		pos = spawn_points[i].position;
		
		glBegin(GL_QUADS);
		
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
		
		glEnd();
		
	}
	
	/*glEnd();
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
		case PICK_SPAWN_POINT:
			record->type = pick_type;
			record->index0 = (*(int *)&q[1]) - 1;
			record->index1 = (*(int *)&q[2]);
			record->index2 = (*(int *)&q[3]);
			return 1;
		break;
		
		/*case PICK_SPAWN_POINT:
			printf("fuck you...\n");
			return 0;
		break;
		*/
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

void editor_Set3dHandleMode(int mode)
{
	switch(mode)
	{
		case HANDLE_3D_TRANSLATION:
		case HANDLE_3D_ROTATION:
		case HANDLE_3D_SCALE:
			handle_3d_mode = mode;
			handle_3d_mode_str = handle_3d_mode_strs[mode];	
		break;
	}
}

void editor_Set3dHandlePivotMode(int mode)
{
	switch(mode)
	{
		case HANDLE_3D_ACTIVE_OBJECT_ORIGIN:
		case HANDLE_3D_MEDIAN_POINT:
			handle_3d_position_mode = mode;
		break;
	}
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
		
		if(brushes[i].type == BRUSH_INVALID)
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
	
	selected_light.r = NULL;
	selected_light.g = NULL;
	selected_light.b = NULL;
	selected_light.radius = NULL;
	selected_light.energy = NULL;
	
	
	switch(record->type)
	{
		case PICK_LIGHT:
			selected_light.r = &light_params[record->index0].r;
			selected_light.g = &light_params[record->index0].g;
			selected_light.b = &light_params[record->index0].b;
			selected_light.radius = &light_params[record->index0].radius;
			selected_light.energy = &light_params[record->index0].energy;
		break;
		
		case PICK_BRUSH:
			selected_brush.vertex_count = &brushes[record->index0].vertex_count;
			selected_brush.triangle_group_count = &brushes[record->index0].triangle_group_count;
			selected_brush.type = &brushes[record->index0].type;
			selected_brush.polygon_count = &brushes[record->index0].polygon_count;
		break;
	}
	
	selected_type = record->type;
	
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
				break;
			}
		}
	}
	
	selected_light.r = NULL;
	selected_light.g = NULL;
	selected_light.b = NULL;
	selected_light.radius = NULL;
	selected_light.energy = NULL;
	
	if(selection_count)
	{
		switch(selections[selection_count - 1].type)
		{
			case PICK_LIGHT:
				selected_light.r = &light_params[selections[selection_count - 1].index0].r;
				selected_light.g = &light_params[selections[selection_count - 1].index0].g;
				selected_light.b = &light_params[selections[selection_count - 1].index0].b;
				selected_light.radius = &light_params[selections[selection_count - 1].index0].radius;
				selected_light.energy = &light_params[selections[selection_count - 1].index0].energy;
			break;
						
			case PICK_BRUSH:
				selected_brush.vertex_count = &brushes[selections[selection_count - 1].index0].vertex_count;
				selected_brush.triangle_group_count = &brushes[selections[selection_count - 1].index0].triangle_group_count;
				selected_brush.type = &brushes[selections[selection_count - 1].index0].type;
				selected_brush.polygon_count = &brushes[selections[selection_count - 1].index0].polygon_count;	
			break;
		}
					
		selected_type = selections[selection_count - 1].type;
	}
	else
	{
		selected_type = PICK_NONE;
	}
}

void editor_ClearSelection()
{
	selection_count = 0;
	selected_type = PICK_NONE;
	
	selected_light.r = NULL;
	selected_light.g = NULL;
	selected_light.b = NULL;
	selected_light.radius = NULL;
	selected_light.energy = NULL;
	
	
	selected_brush.polygon_count = NULL;
	selected_brush.vertex_count = NULL;
	selected_brush.type = NULL;
	selected_brush.triangle_group_count = NULL;
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
			
			case PICK_SPAWN_POINT:
				spawn_points[selections[i].index0].position.x += direction.x * amount;
				spawn_points[selections[i].index0].position.y += direction.y * amount;
				spawn_points[selections[i].index0].position.z += direction.z * amount;
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
			
			case PICK_SPAWN_POINT:
				v = spawn_points[selections[i].index0].position;
				
				v.x -= handle_3d_position.x;
				v.y -= handle_3d_position.y;
				v.z -= handle_3d_position.z;
				
				mat3_t_vec3_t_mult(&rot, &v);
				
				
				v.x += handle_3d_position.x;
				v.y += handle_3d_position.y;
				v.z += handle_3d_position.z;
				
				spawn_points[selections[i].index0].position = v;
				
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
	
	light_position_t *light_pos;
	light_params_t *light_parms;
	
	for(i = 0; i < selection_count; i++)
	{
		switch(selections[i].type)
		{
			case PICK_BRUSH:
				new_index = brush_CopyBrush(&brushes[selections[i].index0]);
				selections[i].index0 = new_index;
			break;
			
			case PICK_LIGHT:
				
				light_pos = &light_positions[selections[i].index0];
				light_parms = &light_params[selections[i].index0];
				
				new_index = light_CreateLight("copy_light", &light_pos->orientation, light_pos->position, vec3((float)light_parms->r / 255.0, (float)light_parms->g / 255.0, (float)light_parms->b / 255.0), LIGHT_ENERGY(light_parms->energy), LIGHT_RADIUS(light_parms->radius), light_parms->bm_flags);
				
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
			
			case PICK_SPAWN_POINT:
				player_DestroySpawnPoint(selections[i].index0);
			break;
		}
	}
	
	editor_ClearSelection();
		
}

void editor_StartPIE()
{
	
	if(editor_state == EDITOR_EDITING)
	{
		editor_state = EDITOR_PIE; 
		player_SpawnPlayer(pie_player_index, -1);
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







