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
#include "entity.h"

#include "r_main.h"
#include "r_editor.h"
 
#include "ed_ui.h"
#include "ed_proj.h"

#include "model.h"

#include "entity.h"

static camera_t *editor_camera;
static int editor_camera_index;


//int resource_count;
//resource_t *resources;



/* from r_main.c */
extern int r_width;
extern int r_height;
extern int forward_pass_shader;
extern int r_window_width;
extern int r_window_height;
extern int r_draw_gui;

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

/* from entity.c */
extern int ent_entity_list_cursor;
extern entity_t *ent_entities;


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
int pick_brush_face_shader;
int light_pick_shader;
int spawn_point_pick_shader;
int brush_dist_shader;
int draw_cursors_shader;
int forward_pass_brush_shader;
extern int model_thumbnail_shader;

int max_selections;
int selection_count;
pick_record_t *selections;
vec3_t cursor_3d_position;
vec3_t handle_3d_position;
int bm_handle_3d_flags;
int handle_3d_position_mode;
int ed_handle_3d_mode;

int scale_handle_model_handle;
int scale_handle_model_start;
int scale_handle_model_count;

int ed_editing_mode = EDITING_MODE_OBJECT;
int ed_handle_3d_tranform_mode = HANDLE_3D_TRANFORM_LOCAL;


light_params_t *ed_selected_light_params;
light_position_t *ed_selected_light_position;


brush_t *ed_selected_brush;
int ed_selected_brush_selection_index = -1;
int ed_selected_brush_index = -1;
int ed_selected_brush_polygon_index = -1;
int ed_selected_brush_polygon_vertex_index = -1;
int ed_selected_brush_polygon_edge_index = -1;


//light_position_t *selected_light_position;
//light_params_t *selected_light_params;


light_ptr_t selected_light;
brush_ptr_t selected_brush;



int ed_selection_type = PICK_NONE;
int editor_state = EDITOR_EDITING;
int pie_player_index;

float editor_camera_yaw = 0.0;
float editor_camera_pitch = 0.0;

float editor_snap = 0.0;

int default_material;
int texture_material;
//int red_default_material;

char *ed_handle_3d_mode_strs[] = 
{
	"Translation",
	"Rotation",
	"Scale",
};

char *ed_handle_3d_mode_str = "Translation";


float ed_editor_linear_snap_values[] = 
{
	0.0,
	0.01,
	0.1,
	1.0,
};

char *ed_editor_linear_snap_values_str[] = 
{
	"None",
	"1cm",
	"10cm",
	"1m",
	NULL,
};

float ed_editor_angular_snap_values[] = 
{
	0.0,
	0.1, 
	0.2,
	0.25,
};

char *ed_editor_angular_snap_values_str[] = 
{
	"None",
	"0.1 rad",
	"0.2 rad",
	"0.25 rad",
	NULL,
};


float ed_editor_linear_snap_value;
int ed_editor_linear_snap_value_index;

float ed_editor_angular_snap_value;
int ed_editor_angular_snap_value_index;

char *ed_editor_snap_value_str;



void editor_Init(int argc, char *argv[])
{
	mat3_t r = mat3_t_id();
	int w;
	int h;
	
	SDL_DisplayMode display_mode;
	
	
	//editor_snap_value = 0.0;
	ed_editor_linear_snap_value_index = 0;
	ed_editor_angular_snap_value_index = 0;
	ed_editor_linear_snap_value = 0.0;
	ed_editor_angular_snap_value = 0.0;
	
	
	//editor_snap_value_str = snap_values_str[0];
	
	//mat3_t r;
	
	//mat3_t_rotate(&r, vec3(1.0, 0.0, 0.0), -0.2, 1);
	//mat3_t_rotate(&r, vec3(0.0, 1.0, 0.0), 0.2, 0);
	
	editor_camera_yaw = 0.2;
	editor_camera_pitch = -0.15;
	
	editor_camera_index = camera_CreateCamera("editor_camera", vec3(12.0, 10.0, 15.0), &r, 0.68, r_width, r_height, 0.1, 500.0, CAMERA_UPDATE_ON_RESIZE);
	editor_camera = camera_GetCameraByIndex(editor_camera_index);	
	camera_SetCameraByIndex(editor_camera_index);
	
	//default_material = material_CreateMaterial("default_material", vec4(1.0, 1.0, 1.0, 1.0), 1.0, 1.0, forward_pass_shader, -1, -1);
	
	path_AddSearchPath("shaders", SEARCH_PATH_SHADER);
	path_AddSearchPath("textures/world", SEARCH_PATH_TEXTURE);
	
	draw_cursors_shader = shader_LoadShader("draw_cursors");
	pick_brush_face_shader = shader_LoadShader("pick_brush_face");
	brush_pick_shader = shader_LoadShader("brush_pick");
	light_pick_shader = shader_LoadShader("light_pick");
	spawn_point_pick_shader = shader_LoadShader("spawn_point_pick");
	brush_dist_shader = shader_LoadShader("brush_dist");
	forward_pass_brush_shader = shader_LoadShader("forward_pass_brush");
	model_thumbnail_shader = shader_LoadShader("model_thumbnail");
	
	
	int model_index;// = model_LoadModel("portal_gun6.mpk", "portal gun");
	int entity_def_index;// = entity_CreateEntityDef("portal gun", ENTITY_TYPE_MOVABLE, model_index);
	
	
	
	
	/*entity_LoadModel("staircase.mpk", "staircase", "staircase", ENTITY_TYPE_STATIC);*/
	//entity_LoadModel("portal_gun6.mpk", "portal_gun", "portal_gun", ENTITY_TYPE_STATIC);
	
	
	
	entity_LoadModel("toilet.mpk", "toilet", "toilet", ENTITY_TYPE_STATIC);
	entity_LoadModel("toilet2.mpk", "toilet2", "toilet2", ENTITY_TYPE_STATIC);
	
	
	
	/*entity_LoadModel("cargo_container_01.mpk", "cargo", "cargo", ENTITY_TYPE_STATIC);
	entity_LoadModel("tetra.mpk", "tetra", "tetra", ENTITY_TYPE_STATIC);*/
	
	/*model_index =*/ 
	/*model_LoadModel("staircase.mpk", "staircase");
	model_index = model_LoadModel("portal_gun6.mpk", "portal_gun");
	model_LoadModel("toilet.mpk", "toilet");
	model_LoadModel("cargo_container_01.mpk", "cargo");
	model_LoadModel("tetra.mpk", "tetra");*/
	
	
	//entity_def_index = entity_CreateEntityDef("staircase", ENTITY_TYPE_STATIC, model_index);
	//entity_CreateEntity("staircase", vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), NULL, entity_def_index);
	
	/*entity_CreateEntity("test entity", vec3(0.0, 0.0, 0.0), vec3(0.1, 0.1, 0.1), NULL, entity_def_index);*/
	
	
	
	
	
	/*texture_LoadTexture("concrete_01_diffuse.png", "concrete_diffuse", 0);
	texture_LoadTexture("concrete_01_normal.png", "concrete_normal", 0);
	
	texture_LoadTexture("synthetic_dirt_01_diffuse.png", "dirt_diffuse", 0);
	texture_LoadTexture("synthetic_dirt_01_normal.png", "dirt_normal", 0);
	
	texture_LoadTexture("diffuse.tga", "tiles_diffuse", 0);
	texture_LoadTexture("normal.tga", "tiles_normal", 0);
	
	texture_LoadTexture("TexturesCom_Cobblestone_512_albedo.png", "cobblestone_diffuse", 0);
	texture_LoadTexture("TexturesCom_Cobblestone_512_normal.png", "cobblestone_normal", 0);
	
	texture_LoadTexture("dungeon-stone1-albedo2.png", "dungeon_diffuse", 0);
	texture_LoadTexture("dungeon-stone1-normal.png", "dungeon_normal", 0);
	
	texture_LoadTexture("oakfloor_basecolor.png", "oak_floor_diffuse", 0);
	texture_LoadTexture("oakfloor_normal.png", "oak_floor_normal", TEXTURE_INVERT_Y);*/
	
	
	/*path_SetDir("C:/Users/Noble/Documents");
	path_GoUp();
	path_GoUp();
	path_GoUp();
	path_GoUp();
	path_GoUp();
	path_GoUp();*/
	
	//path_GoDown("Documents");
	/*material_CreateMaterial("concrete", vec4(1.0, 1.0, 1.0, 1.0), 1.0, 1.0, forward_pass_shader, texture_GetTexture("concrete_diffuse"), texture_GetTexture("concrete_normal"));
	material_CreateMaterial("tiles", vec4(1.0, 1.0 ,1.0, 1.0), 1.0, 1.0, forward_pass_shader, texture_GetTexture("tiles_diffuse"), texture_GetTexture("tiles_normal")); */
	
	
//	material_CreateMaterial("material1", vec4(1.0, 1.0, 1.0, 1.0), 1.0, 1.0, forward_pass_shader, -1, -1);
//	material_CreateMaterial("material2", vec4(1.0, 1.0, 1.0, 1.0), 1.0, 1.0, forward_pass_shader, -1, -1);
//	material_CreateMaterial("material3", vec4(1.0, 1.0, 1.0, 1.0), 1.0, 1.0, forward_pass_shader, -1, -1);
	
	
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
	input_RegisterKey(SDL_SCANCODE_M);
	input_RegisterKey(SDL_SCANCODE_T);
	input_RegisterKey(SDL_SCANCODE_V);
	
	input_RegisterKey(SDL_SCANCODE_DELETE);
	
	input_RegisterKey(SDL_SCANCODE_TAB);
	
	
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
	
	renderer_RegisterCallback(renderer_EditorDraw, PRE_SHADING_STAGE_CALLBACK);
	renderer_RegisterCallback(renderer_PostDraw, POST_SHADING_STAGE_CALLBACK);
	
	
	SDL_GetDisplayMode(0, 0, &display_mode);
	
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, display_mode.w, display_mode.h, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindTexture(GL_TEXTURE_2D, pick_framebuffer_depth_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, display_mode.w, display_mode.h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, r_width, r_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	
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
	
	glClear(GL_COLOR_BUFFER_BIT);

	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	

	
	
	editor_InitUI();
	
	
	
	max_selections = 1024;
	selection_count = 0;
	selections = malloc(sizeof(pick_record_t ) * max_selections);
	
	pie_player_index = player_CreatePlayer("pie player", vec3(0, 0, 0), &r);
	
	
	//brush_CreateBrush(vec3(0, 0, 0), &r, vec3(1, 1, 1), BRUSH_CUBE, 1);
	
	
	cursor_3d_position = vec3(0.0, 0.0, 0.0);
	handle_3d_position = vec3(0.0, 0.0, 0.0);
	int i;
			
	bm_handle_3d_flags = 0;
	handle_3d_position_mode = HANDLE_3D_MEDIAN_POINT;
	//ed_handle_3d_mode = HANDLE_3D_TRANSLATION;
	editor_Set3dHandleMode(HANDLE_3D_TRANSLATION);
	editor_SetEditingMode(EDITING_MODE_OBJECT);
	
	editor_SetProjectName("untitled.wtf");
	renderer_RegisterCallback(editor_WindowResizeCallback, RENDERER_RESOLUTION_CHANGE_CALLBACK);		
	
	
	if(argc > 1)
	{
		editor_OpenProject(argv[1]);
	}
	
				        	
}

extern bsp_node_t *world_bsp;
extern bsp_node_t *collision_bsp;

void editor_RestartEditor()
{
	mat3_t r = mat3_t_id();
	
	editor_camera_yaw = 0.2;
	editor_camera_pitch = -0.15;
	
	//editor_camera_index = camera_CreateCamera("editor_camera", vec3(12.0, 10.0, 15.0), &r, 0.68, r_width, r_height, 0.1, 500.0, CAMERA_UPDATE_ON_RESIZE);
	//editor_camera = camera_GetCameraByIndex(editor_camera_index);	
	//camera_SetCameraByIndex(editor_camera_index);
	
	//default_material = material_CreateMaterial("default_material", vec4(1.0, 1.0, 1.0, 1.0), 1.0, 1.0, forward_pass_shader, -1, -1);
	
//	material_CreateMaterial("concrete", vec4(1.0, 1.0, 1.0, 1.0), 1.0, 1.0, forward_pass_shader, texture_GetTexture("dirt_diffuse"), texture_GetTexture("dirt_normal"));
//	material_CreateMaterial("tiles", vec4(1.0, 1.0 ,1.0, 1.0), 1.0, 1.0, forward_pass_shader, texture_GetTexture("tiles_diffuse"), texture_GetTexture("tiles_normal"));
	
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
	
	if(world_bsp)
	{
		bsp_DeleteSolidLeafBsp(world_bsp);
		world_bsp = NULL;
	}
	
	if(collision_bsp)
	{
		bsp_DeleteSolidLeafBsp(collision_bsp);
		collision_bsp = NULL;
	}
	
	path_ClearSearchPaths();
	
	
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

extern wsurface_t *edit_uv_window;


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
 	
 	if(active_camera == editor_camera)
	{
	 	CreatePerspectiveMatrix(&active_camera->projection_matrix, 0.68, (float)r_window_width / (float)r_window_height, 0.1, 500.0, 0.0, 0.0, &active_camera->frustum);
	}
 	 	
 	
 	
 	
 	//editor_ProcessUI();
 	
 	brush_ProcessBrushes();
 	
 		
	if(bm_mouse & MOUSE_MIDDLE_BUTTON_CLICKED && editor_state == EDITOR_EDITING && !(edit_uv_window->widget.bm_flags & WIDGET_HAS_MIDDLE_MOUSE_BUTTON))
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
	glViewport(0, 0, r_window_width, r_window_height);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);	
}

void editor_DisablePicking()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
}

 int editor_PickObject()
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
	
	pick_record_t record;
	model_t *model;
	
	int lshift;
	
	int start;
	
	vec3_t pos;
	mat4_t transform;
	
	camera_t *active_camera = camera_GetActiveCamera();
	triangle_group_t *triangle_group;
	material_t *material;
	bsp_polygon_t *polygon;
	
	float q[4];
	
	editor_EnablePicking();
	
	gpu_BindGpuHeap();
	
	renderer_SetShader(brush_pick_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, (int)&((vertex_t *)0)->position, sizeof(vertex_t));
	
	renderer_SetProjectionMatrix(&active_camera->projection_matrix);
	renderer_SetViewMatrix(&active_camera->world_to_camera_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_UpdateMatrices();	
		
	
	glViewport(0, 0, r_window_width, r_window_height);
		
		
	
		
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
	
	
	c = ent_entity_list_cursor;
	
	for(i = 0; i < c; i++)
	{
		if(ent_entities[i].flags & ENTITY_INVALID)
			continue;
		
		if(ent_entities[i].flags & ENTITY_INVISIBLE)
			continue;	
		
		model = model_GetModelPointerIndex(ent_entities[i].model_index);
		mat4_t_compose(&transform, &ent_entities[i].orientation, ent_entities[i].position);
		
		transform.floats[0][0] *= ent_entities[i].scale.x;
		transform.floats[0][1] *= ent_entities[i].scale.x;
		transform.floats[0][2] *= ent_entities[i].scale.x;
		
		
		transform.floats[1][0] *= ent_entities[i].scale.y;
		transform.floats[1][1] *= ent_entities[i].scale.y;
		transform.floats[1][2] *= ent_entities[i].scale.y;
		
		
		transform.floats[2][0] *= ent_entities[i].scale.z;
		transform.floats[2][1] *= ent_entities[i].scale.z;
		transform.floats[2][2] *= ent_entities[i].scale.z;
		
		renderer_SetModelMatrix(&transform);
		renderer_UpdateMatrices();
		
		*(int *)&q[0] = PICK_ENTITY;
		*(int *)&q[1] = i + 1;
		q[2] = 0.0;
		q[3] = 0.0;
					
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, q);
		
		glDrawArrays(GL_TRIANGLES, model->vert_start, model->vert_count);
	}	

	
	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);	
	renderer_SetShader(light_pick_shader);
		
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
	
	renderer_SetShader(spawn_point_pick_shader);
		
	glDisable(GL_CULL_FACE);
		
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
	
		
		
	x = r_window_width * (normalized_mouse_x * 0.5 + 0.5);
	y = r_window_height * (normalized_mouse_y * 0.5 + 0.5);
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, q);
			
	editor_DisablePicking();
		
	pick_type = (*(int *)&q[0]);
		
	switch(pick_type)
	{
		case PICK_BRUSH:
		case PICK_LIGHT:
		case PICK_SPAWN_POINT:
		case PICK_ENTITY:
			record.type = pick_type;
			record.index0 = (*(int *)&q[1]) - 1;
			record.index1 = (*(int *)&q[2]);
			record.index2 = (*(int *)&q[3]);
		break;

		case PICK_NONE:
			return 0;
		break;
	}
	gpu_UnbindGpuHeap();
	
	
	lshift = input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED;
				
					
	if(record.type == selections[selection_count - 1].type)
	{
		/* if the just picked thing is the same as the last picked thing... */	
		if(record.index0 == selections[selection_count - 1].index0)
		{
							
			/* if this selection is the last in the list (meaning
			this object is the active object), drop it, set
			whatever comes before it as the active object and then jump
			to the code that sets the 3d handle position... */
						
						
						
			if(!lshift && selection_count)
			{
				goto _add_new_selection;
			}
							
			editor_DropSelection(&record);
			if(selection_count)
			{
				record = selections[selection_count - 1];
				goto _set_handle_3d_position;
			}
						
		}
		else
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
		if(!lshift)
		{
			editor_ClearSelection();
		}
										
		editor_AddSelection(&record);
							
		_set_handle_3d_position:
		
		editor_Position3dHandle();			
	}
}

int editor_PickOnBrush(brush_t *brush)
{
	int i;
	int j;
	int x;
	int y;
	
	vertex_t *vertices;
	int *indexes;
	camera_t *active_camera;
	bsp_polygon_t *polygon;
	int polygon_count;
	int vertice_count;
	int index_count;
	float pick[4];
	
	
		
	if(brush)
	{
		//gpu_BindGpuHeap();
		//glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		
		renderer_SetShader(pick_brush_face_shader); 
		active_camera = camera_GetActiveCamera();
		
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
		
		editor_EnablePicking();
		
		polygon_count = brush->polygon_count;
		
		for(i = 0; i < polygon_count; i++)
		{
			polygon = brush->polygons + i;
			vertice_count = polygon->vert_count;
			
			pick[0] = i + 1;
			pick[1] = 0.0;
			pick[2] = 0.0;
			pick[3] = 0.0;
		
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pick);
			
			
			glBegin(GL_TRIANGLE_FAN);
			for(j = 0; j < vertice_count; j++)
			{
				glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
			}
			glEnd();
		}
		
		x = r_window_width * (normalized_mouse_x * 0.5 + 0.5);
		y = r_window_height * (normalized_mouse_y * 0.5 + 0.5);
		glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, pick);
		
		editor_DisablePicking();
		pick[0] -= 1;
		
		if(pick[0] < 0.0)
		{
			return;
		}
			
		
		if((int)pick[0] != ed_selected_brush_polygon_index)
		{
			ed_selected_brush_polygon_index = (int)pick[0];
			editor_OpenBrushFacePropertiesWindow(brush - brushes, ed_selected_brush_polygon_index);
		}
		else
		{
			ed_selected_brush_polygon_index = -1;
			editor_CloseBrushFacePropertiesWindow();
		}
		
		//printf("face %d\n", (int)pick[0]);
		//gpu_UnbindGpuHeap();	
		
		//printf("[%f %f %f %f]\n", pick[0], pick[1], pick[2], pick[3]);
		
		//glEnable(GL_CULL_FACE);
	}
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
	int index;
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
		
		
		
		switch(ed_handle_3d_mode)
		{
			case HANDLE_3D_TRANSLATION:
			case HANDLE_3D_SCALE:
				
				
				if(ed_handle_3d_tranform_mode == HANDLE_3D_TRANFORM_GLOBAL || ed_handle_3d_mode == HANDLE_3D_TRANSLATION)
				{
					_global_vectors:
						
					right_vector = vec3(1.0, 0.0, 0.0);
					up_vector = vec3(0.0, 1.0, 0.0);
					forward_vector = vec3(0.0, 0.0, 1.0);
				}
				else
				{
					index = selections[selection_count - 1].index0;
					
					switch(selections[selection_count - 1].type)
					{
						case PICK_BRUSH:
							right_vector = brushes[index].orientation.r_axis;
							up_vector = brushes[index].orientation.u_axis;
							forward_vector = brushes[index].orientation.f_axis;
						break;
						
						case PICK_ENTITY:
							right_vector = ent_entities[index].orientation.r_axis;
							up_vector = ent_entities[index].orientation.u_axis;
							forward_vector = ent_entities[index].orientation.f_axis;
						break;
						
						case PICK_LIGHT:
						case PICK_SPAWN_POINT:
							goto _global_vectors;
						break;
					}
				}
			
				
				right_vector.x *= d;
				right_vector.y *= d;
				right_vector.z *= d;
				
				up_vector.x *= d;
				up_vector.y *= d;
				up_vector.z *= d;
				
				forward_vector.x *= d;
				forward_vector.y *= d;
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
				
				if(ed_handle_3d_mode == HANDLE_3D_TRANSLATION)
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
			
		
		
		x = r_window_width * (normalized_mouse_x * 0.5 + 0.5);
		y = r_window_height * (normalized_mouse_y * 0.5 + 0.5);
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
		case HANDLE_3D_ROTATION:
			ed_editor_snap_value_str = ed_editor_angular_snap_values_str[ed_editor_angular_snap_value_index];
		break;
		
		case HANDLE_3D_TRANSLATION:
		case HANDLE_3D_SCALE:
			ed_editor_snap_value_str = ed_editor_linear_snap_values_str[ed_editor_linear_snap_value_index];
		break;
		
		default:
			return;
	}
	
	ed_handle_3d_mode = mode;
	ed_handle_3d_mode_str = ed_handle_3d_mode_strs[mode];	
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

void editor_SetEditingMode(int mode)
{
	switch(mode)
	{
		case EDITING_MODE_OBJECT:
			editor_CloseBrushFacePropertiesWindow();
		break;
		
		case EDITING_MODE_BRUSH:
			
		break;
		
		case EDITING_MODE_UV:
		
		break;
		
		default:
			return;
	}
	
	ed_editing_mode = mode;
}

void editor_ToggleBrushEditing()
{
	if(ed_editing_mode == EDITING_MODE_OBJECT)
	{
		editor_SetEditingMode(EDITING_MODE_BRUSH);
	}
	else
	{
		editor_SetEditingMode(EDITING_MODE_OBJECT);
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
	//shader_UseShader(brush_dist_shader);	
	renderer_SetShader(brush_dist_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, (int)&((vertex_t *)0)->position, sizeof(vertex_t));
	
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
	
	//printf("%f\n", q[0]);
	
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
	
	//printf("%f\n", z);
		
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

void editor_Position3dHandle()
{
	int i;
	
	handle_3d_position.x = 0.0;
	handle_3d_position.y = 0.0;
	handle_3d_position.z = 0.0;
	
	for(i = 0; i < selection_count; i++)
	{	
		switch(selections[i].type)
		{
			case PICK_LIGHT:
				handle_3d_position.x += light_positions[selections[i].index0].position.x;
				handle_3d_position.y += light_positions[selections[i].index0].position.y;
				handle_3d_position.z += light_positions[selections[i].index0].position.z;
			break;
			
			case PICK_BRUSH:
				handle_3d_position.x += brushes[selections[i].index0].position.x;
				handle_3d_position.y += brushes[selections[i].index0].position.y;
				handle_3d_position.z += brushes[selections[i].index0].position.z;
			break;	
			
			case PICK_SPAWN_POINT:
				handle_3d_position.x += spawn_points[selections[i].index0].position.x;
				handle_3d_position.y += spawn_points[selections[i].index0].position.y;
				handle_3d_position.z += spawn_points[selections[i].index0].position.z;
			break;
			
			case PICK_ENTITY:
				handle_3d_position.x += ent_entities[selections[i].index0].position.x;
				handle_3d_position.y += ent_entities[selections[i].index0].position.y;
				handle_3d_position.z += ent_entities[selections[i].index0].position.z;
			break;	
		}
	
	}
	
	handle_3d_position.x /= (float)selection_count;
	handle_3d_position.y /= (float)selection_count;
	handle_3d_position.z /= (float)selection_count;
}

void editor_AddSelection(pick_record_t *record)
{
	/* try to drop this selection, to make sure
	there's just one selection per object... */
	editor_DropSelection(record);
	
	/*selected_light.r = NULL;
	selected_light.g = NULL;
	selected_light.b = NULL;
	selected_light.radius = NULL;
	selected_light.energy = NULL;
	*/
	
	if(record->type != PICK_UV_VERTEX)
	{
		ed_selected_light_params = NULL;
		ed_selected_light_position = NULL;
		
		ed_selected_brush = NULL;
		ed_selected_brush_polygon_index = -1;
		ed_selected_brush_selection_index = -1;
	}
	
	
	
	switch(record->type)
	{
		case PICK_LIGHT:
			ed_selected_light_params = &light_params[record->index0];
			ed_selected_light_position = &light_positions[record->index0];
			editor_CloseBrushPropertiesWindow();
			editor_OpenLightPropertiesWindow(record->index0);
			//ed_selected_brush_selection_index = -1;
		break;
		
		case PICK_BRUSH:
			ed_selected_brush = &brushes[record->index0];
			editor_CloseLightPropertiesWindow();
			ed_selected_brush_selection_index = selection_count;
		break;
		
		case PICK_UV_VERTEX:
			
		break;
	}
	
	ed_selection_type = record->type;
	
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
	
	/*selected_light.r = NULL;
	selected_light.g = NULL;
	selected_light.b = NULL;
	selected_light.radius = NULL;
	selected_light.energy = NULL;*/
	
	/*if(selections[selection_count - 1].type != PICK_UV_VERTEX &&)
	{
		ed_selected_light_params = NULL;
		ed_selected_light_position = NULL;
		
		ed_selected_brush = NULL;
		ed_selected_brush_polygon_index = -1;
		ed_selected_brush_selection_index = -1;
	}*/
	
	
	
	if(selection_count)
	{
		switch(selections[selection_count - 1].type)
		{
			case PICK_LIGHT:
				ed_selected_light_params = &light_params[selections[selection_count - 1].index0];
				ed_selected_light_position = &light_positions[selections[selection_count - 1].index0];
				
				ed_selected_brush = NULL;
				ed_selected_brush_polygon_index = -1;
				ed_selected_brush_selection_index = -1;
				
			break;
						
			case PICK_BRUSH:
				ed_selected_brush = &brushes[selections[selection_count - 1].index0];
				ed_selected_brush_selection_index = selection_count - 1;
				
				ed_selected_light_params = NULL;
				ed_selected_light_position = NULL;
				
			break;
		}
					
		ed_selection_type = selections[selection_count - 1].type;
	}
	else
	{
		ed_selection_type = PICK_NONE;
	}
}

void editor_ClearSelection()
{
	
	if(ed_editing_mode == EDITING_MODE_UV)
	{
		selection_count = ed_selected_brush_selection_index + 1;
		ed_selection_type = PICK_BRUSH;
	}
	else
	{
		selection_count = 0;
		ed_selection_type = PICK_NONE;
		
		ed_selected_light_params = NULL;
		ed_selected_light_position = NULL;
		
		ed_selected_brush = NULL;
		ed_selected_brush_polygon_index = -1;
		ed_selected_brush_polygon_vertex_index = -1;
		ed_selected_brush_selection_index = -1;
	}
	
	editor_CloseLightPropertiesWindow();
	editor_CloseBrushPropertiesWindow();
	
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
			
			case PICK_UV_VERTEX:
			
			break;
			
			case PICK_ENTITY:
				entity_TranslateEntity(selections[i].index0, direction, amount);
			break;
		}
	}
	
	/*handle_3d_position.x += direction.x * amount;
	handle_3d_position.y += direction.y * amount;
	handle_3d_position.z += direction.z * amount;*/
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
			
			
			case PICK_ENTITY:
				v = ent_entities[selections[i].index0].position;
				
				
				entity_RotateEntity(selections[i].index0, axis, amount);
				
				v.x -= handle_3d_position.x;
				v.y -= handle_3d_position.y;
				v.z -= handle_3d_position.z;
				
				mat3_t_vec3_t_mult(&rot, &v);
				
				
				v.x += handle_3d_position.x;
				v.y += handle_3d_position.y;
				v.z += handle_3d_position.z;
				
				ent_entities[selections[i].index0].position = v;
				//ent_entities[selections[i].index0].flags |= ENTITY_HAS_MOVED;
				//spawn_points[selections[i].index0].position = v;
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
			
			case PICK_ENTITY:
				entity_ScaleEntity(selections[i].index0, axis, amount);
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
				
				new_index = light_CreateLight("copy_light", &light_pos->orientation, light_pos->position, vec3((float)light_parms->r / 255.0, (float)light_parms->g / 255.0, (float)light_parms->b / 255.0), (float)(LIGHT_ENERGY(light_parms->energy)), (float)(LIGHT_RADIUS(light_parms->radius)), light_parms->bm_flags);
				
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
		r_draw_gui = 0;
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
		r_draw_gui = 1;
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







