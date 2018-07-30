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
#include "physics.h"
#include "gui.h"
#include "bsp_cmp.h"
#include "bsp.h"
#include "pvs.h"
#include "indirect.h"
#include "player.h"
#include "portal.h"
#include "..\..\common\r_debug.h"
#include "..\..\common\r_gl.h"

#include "memory.h"

#include "entity.h"

#include "r_main.h"
#include "r_debug.h"
#include "r_editor.h"
 
#include "ed_ui.h"
#include "ed_ui_explorer.h"
#include "ed_proj.h"


#include "ed_level.h"
#include "ed_entity.h"
#include "ed_selection.h"

#include "model.h"

#include "entity.h"


static camera_t *editor_camera;
static int editor_camera_index;



int ed_editor_count = 0;
editor_t *ed_editors = NULL;
editor_t *ed_current_editor = NULL;


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
extern light_position_t *l_light_positions;
extern light_params_t *l_light_params;
extern char **l_light_names;
extern int visible_light_count;
extern int l_light_list_cursor;
extern int l_light_count;






/* from input.c */
extern float normalized_mouse_x;
extern float normalized_mouse_y;
extern float mouse_dx;
extern float mouse_dy;
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
extern struct stack_list_t ent_entities[2];


/* from material.c */
extern int mat_material_count;
extern material_t *mat_materials;
extern char **mat_material_names;

float camera_speed = 0.6;
//static unsigned int ed_pick_framebuffer_id;
//static unsigned int ed_pick_framebuffer_texture;
//static unsigned int ed_pick_framebuffer_depth_texture;

//unsigned int cursor_framebuffer_id;
//unsigned int cursor_color_texture_id;
//unsigned int cursor_depth_texture_id;

/*int brush_pick_shader;
int pick_brush_face_shader;
int light_pick_shader;
int spawn_point_pick_shader;
int brush_dist_shader;
int draw_cursors_shader;
int forward_pass_brush_shader;
int model_thumbnail_shader;*/

#define MAIN_EDITOR_FILE
#include "ed_globals.h" 



//float ed_3d_rotation_handle_angles_lut[ROTATION_HANDLE_DIVS][2];


/*
int max_selections;
int selection_count;
pick_record_t *selections;
vec3_t ed_3d_cursor_position;
vec3_t ed_3d_handle_position;
int ed_3d_handle_flags;
int ed_3d_handle_pivot_mode;
int ed_3d_handle_transform_mode;*/

int scale_handle_model_handle;
int scale_handle_model_start;
int scale_handle_model_count;

//int ed_editing_mode = EDITING_MODE_OBJECT;
//int ed_handle_3d_tranform_mode = HANDLE_3D_TRANFORM_LOCAL;


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


//light_ptr_t selected_light;
//brush_ptr_t selected_brush;



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
	
	renderer_PushFunctionName("editor_Init");
	
	mat3_t r = mat3_t_id();
	int w;
	int h;
	float step = (2.0 * 3.14159265) / ROTATION_HANDLE_DIVS;
	float angle = 0.0;
	float s;
	float c;
	int i;
	GLenum status;
	
	//SDL_DisplayMode display_mode;


	for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
	{
		s = sin(angle);
		c = cos(angle);
		ed_3d_rotation_handle_angles_lut[i][0] = sin(angle);
		ed_3d_rotation_handle_angles_lut[i][1] = cos(angle);	
		angle += step;
	}
	
	
	
	//editor_snap_value = 0.0;
	ed_editor_linear_snap_value_index = 0;
	ed_editor_angular_snap_value_index = 0;
	ed_editor_linear_snap_value = 0.0;
	ed_editor_angular_snap_value = 0.0;
		
	editor_camera_yaw = 0.2;
	editor_camera_pitch = -0.15;
	
	//editor_camera = camera_CreateCamera("editor_camera", vec3(12.0, 10.0, 15.0), &r, 0.68, r_width, r_height, 0.1, 500.0, CAMERA_UPDATE_ON_RESIZE);
	//camera_SetCamera(editor_camera);
	
	engine_SetEngineState(ENGINE_PAUSED);
	
	//return;
	
	//path_AddSearchPath("shaders", SEARCH_PATH_SHADER);
	//path_AddSearchPath("textures/world", SEARCH_PATH_TEXTURE);
	
	//ed_draw_cursors_shader = shader_LoadShader("editor/draw_cursors");
	ed_pick_brush_face_shader = shader_LoadShader("editor/pick_brush_face");
	//ed_brush_pick_shader = shader_LoadShader("editor/brush_pick");
	//ed_light_pick_shader = shader_LoadShader("editor/light_pick");
	//ed_spawn_point_pick_shader = shader_LoadShader("editor/spawn_point_pick");
	ed_brush_dist_shader = shader_LoadShader("editor/brush_dist");
	//ed_forward_pass_brush_shader = shader_LoadShader("editor/forward_pass_brush");
	ed_model_thumbnail_shader = shader_LoadShader("editor/model_thumbnail");
	ed_pick_shader = shader_LoadShader("editor/pick");
	
	
	int model_index;// = model_LoadModel("portal_gun6.mpk", "portal gun");
	int entity_def_index;// = entity_CreateEntityDef("portal gun", ENTITY_TYPE_MOVABLE, model_index);
	int entity_index;
	struct entity_t *entity_def;
	struct model_component_t *model_component;
	
/*	
	collider_def_t *collider_def = physics_CreateColliderDef("cube");
	physics_AddCollisionShape(collider_def, vec3(0.5, 0.5, 0.5), vec3(0.0, 0.0, 0.0), &r, COLLISION_SHAPE_BOX);
	
	model_index = model_LoadModel("toilet2.mpk", "toilet");
	entity_def_index = entity_CreateEntityDef("toilet");
	entity_AddComponent(entity_def_index, COMPONENT_TYPE_MODEL, 1);
	entity_AddComponent(entity_def_index, COMPONENT_TYPE_PHYSICS_CONTROLLER, 1);
	
	entity_def = entity_GetEntityPointer("toilet", 1);
	entity_SetEntityModel(entity_def_index, model_index, 1);
	entity_SetEntityCollider(entity_def_index, collider_def, 1);*/
	
/*	for(i = 0; i < 500; i++)
	{
		entity_SpawnEntity(&r, vec3(0.0, 5.0 + i, 0.0), vec3(1.0, 1.0, 1.0), entity_def_index, "toilet_ent");
	}*/
	
	
	
	//mat3_t_rotate(&r, vec3(0.0, 1.0, 0.0), 0.25, 1);
	
	//entity_SpawnEntity(&r, vec3(0.0, 0.0, 3.5), vec3(1.0, 1.0, 1.0), entity_def_index, "toilet_ent2");
	
	
	
	
		
	//camera_PitchYawCamera(editor_camera, editor_camera_yaw, editor_camera_pitch);
	//camera_ComputeWorldToCameraMatrix(editor_camera);
	
	r = mat3_t_id();
	
	//mat3_t_rotate(&r, vec3(0.0, 1.0, 0.0), 0.3, 1);
	//mat3_t_rotate(&r, vec3(1.0, 0.0, 0.0), 0.3, 0);
	
	
	
	/*int entity_index = */
	
	/*for(i = 0; i < 100; i++)
	{
		entity_CreateEntity("bus stop", vec3(0.0, 5.0 + i * 2.0, 0.0), vec3(1.0, 1.0, 1.0), &r, entity_def_index);
	}*/
	
	//entity_CreateEntity("bus stop", vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), &r, entity_def_index);
	/*entity_CreateEntity("bus stop", vec3(-8.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), &r, entity_def_index);
	entity_CreateEntity("bus stop", vec3(-6.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), &r, entity_def_index);
	entity_CreateEntity("bus stop", vec3(-4.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), &r, entity_def_index);
	entity_CreateEntity("bus stop", vec3(-2.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), &r, entity_def_index);
	entity_CreateEntity("bus stop", vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), &r, entity_def_index);
	entity_CreateEntity("bus stop", vec3(2.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), &r, entity_def_index);
	entity_CreateEntity("bus stop", vec3(4.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), &r, entity_def_index);
	entity_CreateEntity("bus stop", vec3(6.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), &r, entity_def_index);
	entity_CreateEntity("bus stop", vec3(8.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), &r, entity_def_index);
	entity_CreateEntity("bus stop", vec3(10.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), &r, entity_def_index);*/
	//brush_Init();
	
	//char *data;
	//int size;
	
	
	//editor_t *editor;
	
	/*brush_CreateBrush(vec3(0.0, 0.0, 0.0), &r, vec3(1.0, 1.0, 1.0), BRUSH_CUBE, 1);
	brush_CreateBrush(vec3(5.0, 0.0, 0.0), &r, vec3(1.0, 1.0, 1.0), BRUSH_CUBE, 1);
	brush_CreateBrush(vec3(-5.0, 0.0, 0.0), &r, vec3(1.0, 1.0, 1.0), BRUSH_CUBE, 1);*/
	//brush_CreateBrush(vec3(2.0, 0.0, 0.0), &r, vec3(1.0, 1.0, 1.0), BRUSH_CUBE, 0);

	
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
	
	input_RegisterKey(SDL_SCANCODE_X);
	input_RegisterKey(SDL_SCANCODE_Y);
	
	
	input_RegisterKey(SDL_SCANCODE_DELETE);
	input_RegisterKey(SDL_SCANCODE_TAB);
		
	//renderer_RegisterCallback(renderer_EditorDraw, PRE_SHADING_STAGE_CALLBACK);
	//renderer_RegisterCallback(renderer_PostDraw, POST_SHADING_STAGE_CALLBACK);	
	
	//SDL_GetDisplayMode(0, 0, &display_mode);
	
	//glGenFramebuffers(1, &ed_pick_framebuffer_id);
	//glGenTextures(1, &ed_pick_color_texture_id);
	//glGenTextures(1, &ed_pick_depth_texture_id);
	
	ed_pick_framebuffer = renderer_CreateFramebuffer(1920, 1080);
	renderer_AddAttachment(&ed_pick_framebuffer, GL_COLOR_ATTACHMENT0, GL_RGBA32F);
	renderer_AddAttachment(&ed_pick_framebuffer, GL_DEPTH_ATTACHMENT, 0);
	
	
	//while(glGetError() != GL_NO_ERROR);
	//glBindTexture(GL_TEXTURE_2D, ed_pick_color_texture_id);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, r_window_width, r_window_height, 0, GL_RGB, GL_FLOAT, NULL);
	/*status = glGetError();
	if(status != GL_NO_ERROR)
	{
		printf("editor_Init: %s generated while initalizing ed_pick_color_texture_id.\n", renderer_GetGLEnumString(status));
	}*/
	
	
	///glBindTexture(GL_TEXTURE_2D, ed_pick_depth_texture_id);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, r_window_width, r_window_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//glBindTexture(GL_TEXTURE_2D, 0);
	
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, ed_pick_framebuffer_id);
	//glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ed_pick_color_texture_id, 0);
	//glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ed_pick_depth_texture_id, 0);
	
	
	
	glGenFramebuffers(1, &ed_cursors_framebuffer_id);
	glGenTextures(1, &ed_cursors_color_texture_id);
	glGenTextures(1, &ed_cursors_depth_texture_id);
	
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ed_cursors_framebuffer_id);
	
	glBindTexture(GL_TEXTURE_2D, ed_cursors_color_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, r_window_width, r_window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	
	glBindTexture(GL_TEXTURE_2D, ed_cursors_depth_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, r_window_width, r_window_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ed_cursors_color_texture_id, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ed_cursors_depth_texture_id, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, ed_cursors_depth_texture_id, 0);
	
	//glClear(GL_COLOR_BUFFER_BIT);

	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	

	
	
	//ed_max_selections = 1024;
	//ed_selection_count = 0;
	//ed_selections = memory_Malloc(sizeof(pick_record_t ) * ed_max_selections, "editor_Init");
	//pie_player_index = player_CreatePlayer("pie player", vec3(0, 0, 0), &r);
	
	
	//brush_CreateBrush(vec3(0, 0, 0), &r, vec3(1, 1, 1), BRUSH_CUBE, 1);
	
	
	//ed_3d_cursor_position = vec3(0.0, 0.0, 0.0);
	//ed_3d_handle_position = vec3(0.0, 0.0, 0.0);

			
	//ed_3d_handle_flags = 0;
	//ed_3d_handle_pivot_mode = ED_3D_HANDLE_PIVOT_MODE_ACTIVE_OBJECT_ORIGIN;
	//ed_handle_3d_mode = HANDLE_3D_TRANSLATION;
	//editor_Set3dHandleTransformMode(ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION);
	//editor_SetEditingMode(EDITING_MODE_OBJECT);
	
	editor_InitUI();
	
//	editor_SetProjectName("untitled.wtf");
	
	editor_RegisterEditor("Level editor", editor_LevelEditorInit, editor_LevelEditorFinish, editor_LevelEditorRestart, editor_LevelEditorSetup, editor_LevelEditorShutdown, editor_LevelEditorMain);
	editor_RegisterEditor("Entity editor", editor_EntityEditorInit, editor_EntityEditorFinish, editor_EntityEditorRestart, editor_EntityEditorSetup, editor_EntityEditorShutdown, editor_EntityEditorMain);
	
	editor_StartEditor("Level editor");
	
	
	renderer_PopFunctionName();
}

extern bsp_node_t *world_bsp;
extern bsp_node_t *collision_bsp;

void editor_RestartEditor()
{
	
	//return;
	
	//mat3_t r = mat3_t_id();
	
	//editor_camera_yaw = 0.2;
	//editor_camera_pitch = -0.15;

	
	//camera_PitchYawCamera(editor_camera, editor_camera_yaw, editor_camera_pitch);
	//camera_ComputeWorldToCameraMatrix(editor_camera);
		
	//ed_3d_cursor_position = vec3(0.0, 0.0, 0.0);
	//ed_3d_handle_position = vec3(0.0, 0.0, 0.0);

		
	//ed_3d_handle_flags = 0;
	
	//editor_Set3dHandleTransformMode(ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION);
	//editor_Set3dHandlePivotMode(ED_3D_HANDLE_PIVOT_MODE_MEDIAN_POINT);

	
	//editor_ClearSelection();
	
	//if(world_bsp)
	//{
	//	bsp_DeleteSolidLeafBsp(world_bsp);
	//	world_bsp = NULL;
	//}
	
	//if(collision_bsp)
	//{
	//	bsp_DeleteSolidLeafBsp(collision_bsp);
	//	collision_bsp = NULL;
	//}
	
	//path_ClearSearchPaths();
	//editor_SetProjectName("untitled.wtf");
}

void editor_Finish()
{
	editor_t *next;
	//brush_Finish();
	
	editor_FinishUI();
	
	//glDeleteFramebuffers(1, &ed_pick_framebuffer_id);
	//glDeleteTextures(1, &ed_pick_color_texture_id);
	//glDeleteTextures(1, &ed_pick_depth_texture_id);
	
	renderer_DestroyFramebuffer(&ed_pick_framebuffer);
	
	glDeleteFramebuffers(1, &ed_cursors_framebuffer_id);
	glDeleteTextures(1, &ed_cursors_color_texture_id);
	glDeleteTextures(1, &ed_cursors_depth_texture_id);
	
	//memory_Free(ed_selections);
	
	while(ed_editors)
	{
		next = ed_editors->next;
		editor_UnregisterEditor(ed_editors->name);
		ed_editors = next;
	}
	
}

extern int b_draw_brushes;
extern float insane_float0;
extern float insane_float1;
extern float insane_float2;

extern unsigned char insane_char;

extern wsurface_t *edit_uv_window;


void editor_Main(float delta_time)
{	
	editor_ProcessUI();
	
	if(ed_current_editor->main_callback)
	{
		ed_current_editor->main_callback(delta_time);
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
		
		renderer_SetShader(ed_pick_brush_face_shader); 
		active_camera = camera_GetActiveCamera();
		
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&active_camera->view_data.view_matrix.floats[0][0]);
		
		editor_EnablePicking();
		
		polygon_count = brush->base_polygons_count;
		
		for(i = 0; i < polygon_count; i++)
		{
			polygon = brush->base_polygons + i;
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
		
		//printf("%f\n", pick[0]);
		
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
	}
}

void editor_Set3dHandleTransformMode(int mode)
{
	switch(mode)
	{
		case ED_3D_HANDLE_TRANSFORM_MODE_ROTATION:
			//ed_editor_snap_value_str = ed_editor_angular_snap_values_str[ed_editor_angular_snap_value_index];
		break;
		
		case ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION:
		case ED_3D_HANDLE_TRANSFORM_MODE_SCALE:
			//ed_editor_snap_value_str = ed_editor_linear_snap_values_str[ed_editor_linear_snap_value_index];
		break;
		
		default:
			return;
	}
	
	ed_3d_handle_transform_mode = mode;
	//ed_handle_3d_mode_str = ed_handle_3d_mode_strs[mode];	
}

void editor_Set3dHandlePivotMode(int mode)
{
	switch(mode)
	{
		case ED_3D_HANDLE_PIVOT_MODE_ACTIVE_OBJECT_ORIGIN:
		case ED_3D_HANDLE_PIVOT_MODE_MEDIAN_POINT:
			ed_3d_handle_pivot_mode = mode;
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



/*void editor_StartPIE()
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
	}
	
}*/



void editor_RegisterEditor(char *name, void (*init_callback)(), void (*finish_callback)(), void (*restart_callback)(), void (*setup_callback)(), void (*shutdown_callback)(), void (*main_callback)(float))
{
	editor_t *editor;
	
	editor = memory_Malloc(sizeof(editor_t), "editor_RegisterEditor");
	
	editor->name = memory_Strdup(name, "editor_RegisterEditor");
	editor->init_callback = init_callback;
	editor->finish_callback = finish_callback;
	editor->restart_callback = restart_callback;
	editor->setup_callback = setup_callback;
	editor->shutdown_callback = shutdown_callback;
	editor->main_callback = main_callback;
	editor->editor_data = NULL;
	
	if(editor->init_callback)
	{
		editor->init_callback();
	}
	
	editor->next = ed_editors;
	ed_editors = editor;
	
	editor_EnumerateEditors();
}

void editor_UnregisterEditor(char *name)
{
	editor_t *editor = NULL;
	editor_t *prev_editor = NULL;
	editor = ed_editors;
	
	while(editor)
	{
		if(!strcmp(name, editor->name))
		{
			if(!prev_editor)
			{
				ed_editors = ed_editors->next;
			}
			else
			{
				prev_editor->next = editor->next;
			}
			
			if(editor->finish_callback)
			{
				editor->finish_callback();
			}
			
			memory_Free(editor->name);
			memory_Free(editor);
			
			return;
		}
		
		prev_editor = editor;
		editor = editor->next;
	}
}

editor_t *editor_GetEditor(char *name)
{
	editor_t *editor;
	
	editor = ed_editors;
	
	while(editor)
	{
		if(!strcmp(editor->name, name))
		{
			break;
		}
		
		editor = editor->next;
	}
	
	return editor;
}

void editor_InitializeEditors()
{
	editor_t *editor;
	
	editor = ed_editors;
	
	while(editor)
	{
		if(editor->init_callback)
		{
			editor->init_callback();
		}
		
		editor = editor->next;
	}
}

void editor_FinishEditors()
{
	editor_t *editor;
	
	editor = ed_editors;
	
	while(editor)
	{
		if(editor->finish_callback)
		{
			editor->finish_callback();
		}
		
		editor = editor->next;
	}
}

void editor_StartEditor(char *name)
{
	editor_t *editor;
	editor = editor_GetEditor(name);
	
	if(editor)
	{		
		if(editor->setup_callback)
		{
			if(ed_current_editor)
			{
				if(ed_current_editor->shutdown_callback)
				{
					ed_current_editor->shutdown_callback();
				}
			}
			
			editor_CloseExplorerWindow();
			
			editor->setup_callback();
			
			ed_current_editor = editor;
		}
	}
	
	return;
}








