#include "ed_level.h"
#include "ed_level_draw.h"
#include "ed_level_ui.h"
#include "..\..\common\r_main.h"

#include "..\..\common\gmath\matrix.h"
#include "..\..\common\gmath\vector.h"

#include "..\ed_common.h"
#include "..\ed_cursors.h"
#include "..\editor.h"
#include "..\ed_selection.h"
#include "..\brush.h"

#include "..\..\common\camera.h"
#include "..\..\common\input.h"
#include "..\..\common\memory.h"
#include "..\..\common\l_main.h"
#include "..\..\common\player.h"
#include "..\..\common\entity.h"
#include "..\..\common\world.h"
#include "..\..\common\entity.h"
#include "..\..\common\ent_common.h"
#include "..\..\common\bsp.h"
#include "..\..\common\engine.h"
#include "..\..\common\portal.h"
#include "..\..\common\particle.h"
#include "..\..\common\script.h"
#include "..\..\common\navigation.h"

#include "..\..\common\GLEW\include\GL\glew.h"


/* from r_main.c */
extern int r_width;
extern int r_height;
extern int r_window_width;
extern int r_window_height;

/* from input.c */
extern float normalized_mouse_x;
extern float normalized_mouse_y;
extern float last_mouse_x;
extern float last_mouse_y;
extern float mouse_dx;
extern float mouse_dy;
extern int bm_mouse;


/* from world.c */
extern int w_world_vertices_count;
extern vertex_t *w_world_vertices;
	
extern int w_world_nodes_count;
extern bsp_pnode_t *w_world_nodes;
	
extern int w_world_leaves_count;
extern bsp_dleaf_t *w_world_leaves;
	
extern int w_world_batch_count;
extern batch_t *w_world_batches;
	

/* from l_main.c */
extern int l_light_list_cursor;
extern int l_light_list_size;
extern int l_free_position_stack_top;
extern int *l_free_position_stack;
extern light_params_t *l_light_params;
extern light_position_t *l_light_positions;

/* from player.c */
extern spawn_point_t *spawn_points;

/* from entity.c */
extern int ent_entity_list_cursor;
extern int ent_entity_list_size;
extern int ent_free_stack_top;
extern int *ent_free_stack;
extern struct entity_t *ent_entities;
extern struct entity_aabb_t *ent_aabbs;

/* from physics.c */
extern int collider_list_cursor;
extern int max_colliders;
int colliders_free_positions_stack_top;
int *colliders_free_positions_stack;
collider_t *colliders;


/* from navigation.c */
extern int nav_waypoint_count;
extern struct waypoint_t *nav_waypoints;
 

/* from portal.c */
extern int ptl_portals_list_cursor;
extern portal_t *ptl_portals;

/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/

float level_editor_camera_pitch = 0.0;
float level_editor_camera_yaw = 0.0;
float level_editor_camera_speed_multiplier = 1.0;
camera_t *level_editor_camera = NULL;


/*int level_editor_max_selections = 0;
int level_editor_selection_count = 0;
pick_record_t *level_editor_selections = NULL;
int level_editor_last_selection_type = PICK_NONE;*/

pick_list_t level_editor_pick_list;
pick_list_t level_editor_brush_face_pick_list;


int level_editor_3d_handle_transform_mode = ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION;
int level_editor_3d_handle_transform_orientation = ED_3D_HANDLE_TRANSFORM_ORIENTATION_GLOBAL;
int level_editor_3d_handle_pivot_mode = ED_3D_HANDLE_PIVOT_MODE_MEDIAN_POINT;
int level_editor_3d_handle_flags = 0;


vec3_t level_editor_3d_handle_position;
vec3_t level_editor_3d_cursor_position;


float level_editor_linear_snap_value;
float level_editor_angular_snap_value;


int level_editor_editing_mode = EDITING_MODE_OBJECT;
int level_editor_state = EDITOR_EDITING;



int level_editor_draw_brushes = 1;
int level_editor_draw_grid = 1;
int level_editor_draw_selected = 1;
int level_editor_draw_cursors = 1;
int level_editor_draw_lights = 1;
int level_editor_draw_spawn_points = 1;
int level_editor_draw_leaves = 1;
int level_editor_draw_world_polygons = 1;
int level_editor_draw_clipped_polygons = 1;
int level_editor_draw_entities_aabbs = 1;
int level_editor_draw_3d_handle = 0;


/* the level editor plain copies all the data
that exists in the world... */
int level_editor_has_copied_data = 0;
int level_editor_need_to_copy_data = 1;

int level_editor_world_vertices_count = 0;
vertex_t *level_editor_world_vertices = NULL;

int level_editor_world_nodes_count = 0;
bsp_pnode_t *level_editor_world_nodes = NULL;

int level_editor_world_leaves_count = 0;
bsp_dleaf_t *level_editor_world_leaves = NULL;

int level_editor_world_batch_count = 0;
batch_t *level_editor_world_batches = NULL;

int level_editor_entity_list_cursor = 0;
int level_editor_entity_list_size = 0;
int level_editor_entity_free_stack_top = -1;
int *level_editor_entity_free_stack = NULL;
struct entity_t *level_editor_entities = NULL;
struct entity_aabb_t *level_editor_entity_aabbs = NULL;

int level_editor_collider_list_cursor = 0;
int level_editor_collider_list_size = 0;
int level_editor_collider_free_stack_top = -1;
int *level_editor_collider_free_stack = NULL;
collider_t *level_editor_colliders = NULL;

int level_editor_light_list_cursor = 0;
int level_editor_light_list_size = 0;
int level_editor_light_free_stack_top = -1;
int *level_editor_light_free_stack = NULL;
light_position_t *level_editor_light_positions = NULL;
light_params_t *level_editor_light_params = NULL;

int level_editor_portal_list_cursor = 0;
int level_editor_portal_list_size = 0;
int level_editor_portal_free_stack_top = -1;
int *level_editor_portal_free_stack = NULL;
portal_t *level_editor_portals = NULL;


/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
 
int portal0;
int portal1; 
 
void editor_LevelEditorInit()
{
	mat3_t r = mat3_t_id();
	int i;
	level_editor_camera = camera_CreateCamera("level editor camera", vec3(20.0, 10.0, 15.0), &r, 0.68, r_width, r_height, 0.1, 500.0, 0);
	//level_editor_camera = camera_CreateCamera("level editor camera", vec3(0.0, 7.0, 0.0), &r, 0.68, r_width, r_height, 0.1, 500.0, 0);
	
	camera_Deactivate(level_editor_camera);
	level_editor_camera_yaw = 0.2;
	level_editor_camera_pitch = -0.15;
	
	//level_editor_camera_yaw = 0.0;
	//level_editor_camera_pitch = 0.0;
	
	level_editor_pick_list.max_records = 1024;
	level_editor_pick_list.record_count = 0;
	level_editor_pick_list.records = memory_Malloc(sizeof(pick_record_t) * level_editor_pick_list.max_records, "editor_LevelEditorInit");  
	
	
	level_editor_brush_face_pick_list.max_records = 10;
	level_editor_brush_face_pick_list.record_count = 0;
	level_editor_brush_face_pick_list.records = memory_Malloc(sizeof(pick_record_t) * level_editor_brush_face_pick_list.max_records, "editor_LevelEditorInit");
	
		
	camera_PitchYawCamera(level_editor_camera, level_editor_camera_yaw, level_editor_camera_pitch);
	camera_ComputeWorldToCameraMatrix(level_editor_camera);
	 
	editor_LevelEditorUIInit();
	 
	brush_Init();
	
	//camera_t *camera = camera_CreateCamera("entity_camera", vec3(0.0, 0.0, 0.0), &r, 0.68, 1366.0, 768.0, 0.1, 500.0, 0);
	
	//struct particle_system_script_t *script = particle_LoadParticleSystemScript("scripts/particle_system.as", "particle_system_test_script");
	//script_t *particle_system_script = script_LoadScript("scripts/particle_system.as", "particle_system_test_script");
	
	//int ps_def = particle_CreateParticleSystemDef("test", 500, 500, 1, 0, 0, script);
	
	//particle_SpawnParticleSystem(vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), &r, ps_def);
	
	struct script_controller_component_t *ai_controller;
	struct controller_script_t *ai_script = entity_LoadScript("scripts/ai_script.as", "ai_script");	
	collider_def_t *def = physics_CreateCharacterColliderDef("test collider", 1.0, 0.5, 0.3, 0.15, 0.7);
	int model_index = model_LoadModel("toilet2.mpk", "toilet");
	
	struct entity_handle_t entity_def = entity_CreateEntityDef("test player");

	entity_AddComponent(entity_def, COMPONENT_TYPE_CAMERA);
	entity_AddComponent(entity_def, COMPONENT_TYPE_SCRIPT_CONTROLLER);
	entity_AddComponent(entity_def, COMPONENT_TYPE_MODEL);
	
	
	
	entity_SetModel(entity_def, model_index);
	entity_SetCollider(entity_def, def);
	entity_SetControllerScript(entity_def, ai_script);
	mat3_t_rotate(&r, vec3(0.0, 1.0, 0.0), -0.5, 1);
	entity_SetCameraTransform(entity_def, &r, vec3(0.0, 2.0, 0.0));
	
	r = mat3_t_id();
	
	entity_SpawnEntity(&r, vec3(0.0, 2.0, -10.0), vec3(1.0, 1.0, 1.0), entity_def, "test player");	 
	//entity_SpawnEntity(&r, vec3(2.0, 2.0, -10.0), vec3(1.0, 1.0, 1.0), entity_def, "test player1");
	//entity_SpawnEntity(&r, vec3(-2.0, 2.0, -10.0), vec3(1.0, 1.0, 1.0), entity_def, "test player2");
	//entity_SpawnEntity(&r, vec3(4.0, 2.0, -10.0), vec3(1.0, 1.0, 1.0), entity_def, "test player1");
	//entity_SpawnEntity(&r, vec3(-4.0, 2.0, -10.0), vec3(1.0, 1.0, 1.0), entity_def, "test player2");
}
 
void editor_LevelEditorFinish()
{
	editor_LevelEditorUIFinish();
	brush_Finish();
	
	memory_Free(level_editor_pick_list.records);
	memory_Free(level_editor_brush_face_pick_list.records);
		
	if(level_editor_has_copied_data)
	{
		memory_Free(level_editor_entities);
		memory_Free(level_editor_entity_aabbs);
		memory_Free(level_editor_entity_free_stack);
		
		memory_Free(level_editor_collider_free_stack);
		memory_Free(level_editor_colliders);
		
		memory_Free(level_editor_light_positions);
		memory_Free(level_editor_light_params);
		memory_Free(level_editor_light_free_stack);
		
		//memory_Free(level_editor_portal_free_stack);
		//memory_Free(level_editor_portals);
	}
	
}

void editor_LevelEditorSetup()
{	
	camera_SetCamera(level_editor_camera);
	camera_SetMainViewCamera(level_editor_camera);

	renderer_RegisterCallback(editor_LevelEditorPreDraw, PRE_SHADING_STAGE_CALLBACK);
	renderer_RegisterCallback(editor_LevelEditorPostDraw, POST_SHADING_STAGE_CALLBACK);
	editor_LevelEditorUISetup();
	editor_LevelEditorRestoreLevelData();
}

void editor_LevelEditorShutdown()
{
	
	editor_LevelEditorCloseAddToWorldMenu();
	editor_LevelEditorCloseDeleteSelectionsMenu();
	
	renderer_ClearRegisteredCallbacks();
	editor_LevelEditorUIShutdown();
	editor_LevelEditorCopyLevelData();
	editor_LevelEditorClearLevel();
	
	camera_Deactivate(level_editor_camera);
}

void editor_LevelEditorRestart()
{
	
}

void editor_LevelEditorMain(float delta_time)
{
	camera_t *active_camera;
	
	portal_t *portal;
	
	//portal = portal_GetPortalPointerIndex(portal0);
	
	static float r = -1.0;
	static int d = 0;
	
	//portal->position.z = -7.0 + sin(r * 3.14159265) * 5.0;
	
	//mat3_t_rotate(&portal->orientation, vec3(0.0, 1.0, 0.0), r, 1);
	
	r += 0.005;
	/*if(d)
	{
		r += 0.0001;
		if(r > -0.8)
		{
			d = 0;
		}
	}
	else
	{
		r -= 0.0001;
		if(r < -1.2)
		{
			d = 1;
		}
	}*/
	
	
	//active_camera = camera_GetActiveCamera();
	
	//active_camera = camera_GetMainViewCamera();
	//if(active_camera == level_editor_camera)
	{
		CreatePerspectiveMatrix(&level_editor_camera->view_data.projection_matrix, 0.68, (float)r_window_width / (float)r_window_height, 0.1, 500.0, 0.0, 0.0, &level_editor_camera->frustum);
	}
	
	
	if(bm_mouse & MOUSE_MIDDLE_BUTTON_CLICKED)
	{
		editor_LevelEditorFly(delta_time);
	}
	else
	{
		editor_LevelEditorEdit(delta_time);
	}
	
}


pick_record_t editor_LevelEditorPickObject(float mouse_x, float mouse_y)
{
	pick_record_t record;
	int lshift;
	
	
	if(level_editor_editing_mode == EDITING_MODE_OBJECT)
	{
		record = editor_PickObject(mouse_x, mouse_y);
		lshift = input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED;
		
		if(record.type == level_editor_pick_list.records[level_editor_pick_list.record_count - 1].type)
		{
			if(record.type == PICK_BRUSH)
			{
				if(record.pointer == level_editor_pick_list.records[level_editor_pick_list.record_count - 1].pointer)
				{
					goto _same_selection;
				}
				
				goto _add_new_selection;
			}
			
			/* if the just picked thing is the same as the last picked thing... */	
			if(record.index0 == level_editor_pick_list.records[level_editor_pick_list.record_count - 1].index0)
			{
				_same_selection:
				/* this record already exists in the list, so drop it... */
				editor_DropSelection(&record, &level_editor_pick_list);
				//editor_LevelEditorDropSelection(&record);
				
				if(!lshift)
				{
					if(level_editor_pick_list.record_count)
					{
						goto _add_new_selection;
					}
				}
				
				goto _set_handle_3d_position;				
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
				editor_ClearSelection(&level_editor_pick_list);
				//editor_LevelEditorClearSelections();
			}
											
			editor_AddSelection(&record, &level_editor_pick_list);
			//editor_LevelEditorAddSelection(&record);
								
			_set_handle_3d_position:
			
			editor_LevelEditorUpdate3dHandlePosition();			
		}
	}
	else
	{
		record = editor_GetLastSelection(&level_editor_pick_list);
		
		if(record.type == PICK_BRUSH)
		{
			record = editor_PickBrushFace((brush_t *)record.pointer, mouse_x, mouse_y);
			
			editor_ClearSelection(&level_editor_brush_face_pick_list);
			editor_AddSelection(&record, &level_editor_brush_face_pick_list);
		}
	}
	
	
	
	
}

int editor_LevelEditorPickBrushFace(brush_t *brush, float mouse_x, float mouse_y)
{
	
}


/*
====================================================================
====================================================================
====================================================================
*/


void editor_LevelEditorCheck3dHandle(float mouse_x, float mouse_y)
{
	level_editor_3d_handle_flags = editor_Check3dHandle(mouse_x, mouse_y, level_editor_3d_handle_position, level_editor_3d_handle_transform_mode);
}

void editor_LevelEditorSet3dCursorPosition(float mouse_x, float mouse_y)
{
	level_editor_3d_cursor_position = editor_3dCursorPosition(mouse_x, mouse_y);
}

void editor_LevelEditorUpdate3dHandlePosition()
{
	int i;
	
	level_editor_3d_handle_position.x = 0.0;
	level_editor_3d_handle_position.y = 0.0;
	level_editor_3d_handle_position.z = 0.0;
	brush_t *brush;
	
	for(i = 0; i < level_editor_pick_list.record_count; i++)
	{	
		switch(level_editor_pick_list.records[i].type)
		{
			case PICK_LIGHT:
				level_editor_3d_handle_position.x += l_light_positions[level_editor_pick_list.records[i].index0].position.x;
				level_editor_3d_handle_position.y += l_light_positions[level_editor_pick_list.records[i].index0].position.y;
				level_editor_3d_handle_position.z += l_light_positions[level_editor_pick_list.records[i].index0].position.z;
			break;
			
			case PICK_BRUSH:
				brush = (brush_t *)level_editor_pick_list.records[i].pointer;
				level_editor_3d_handle_position.x += brush->position.x;
				level_editor_3d_handle_position.y += brush->position.y;
				level_editor_3d_handle_position.z += brush->position.z;
			break;	
			
			case PICK_SPAWN_POINT:
//				level_editor_3d_handle_position.x += spawn_points[level_editor_pick_list.records[i].index0].position.x;
	//			level_editor_3d_handle_position.y += spawn_points[level_editor_pick_list.records[i].index0].position.y;
//				level_editor_3d_handle_position.z += spawn_points[level_editor_pick_list.records[i].index0].position.z;
			break;
			
			/*case PICK_ENTITY:
				level_editor_3d_handle_position.x += ent_entities[level_editor_pick_list.records[i].index0].position.x;
				level_editor_3d_handle_position.y += ent_entities[level_editor_pick_list.records[i].index0].position.y;
				level_editor_3d_handle_position.z += ent_entities[level_editor_pick_list.records[i].index0].position.z;
			break;	*/
			
			case PICK_PORTAL:
				level_editor_3d_handle_position.x += ptl_portals[level_editor_pick_list.records[i].index0].position.x;
				level_editor_3d_handle_position.y += ptl_portals[level_editor_pick_list.records[i].index0].position.y;
				level_editor_3d_handle_position.z += ptl_portals[level_editor_pick_list.records[i].index0].position.z;
			break;
			
			case PICK_WAYPOINT:
				level_editor_3d_handle_position.x += nav_waypoints[level_editor_pick_list.records[i].index0].position.x;
				level_editor_3d_handle_position.y += nav_waypoints[level_editor_pick_list.records[i].index0].position.y;
				level_editor_3d_handle_position.z += nav_waypoints[level_editor_pick_list.records[i].index0].position.z;
			break;
		}
	
	}
	
	level_editor_3d_handle_position.x /= (float)level_editor_pick_list.record_count;
	level_editor_3d_handle_position.y /= (float)level_editor_pick_list.record_count;
	level_editor_3d_handle_position.z /= (float)level_editor_pick_list.record_count;
}

void editor_LevelEditorSet3dHandleTransformMode(int mode)
{
	switch(mode)
	{
		case ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION:
		case ED_3D_HANDLE_TRANSFORM_MODE_ROTATION:
		case ED_3D_HANDLE_TRANSFORM_MODE_SCALE:
			level_editor_3d_handle_transform_mode = mode;
		break;
	}
}

void editor_LevelEditorSetEditingMode(int mode)
{
	switch(mode)
	{
		case EDITING_MODE_BRUSH:
			editor_ClearSelection(&level_editor_brush_face_pick_list);
		case EDITING_MODE_OBJECT:
		case EDITING_MODE_UV:
			level_editor_editing_mode = mode;
		break;
	}
}


/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorEdit(float delta_time)
{
	pick_record_t record; 
	camera_t *active_camera = camera_GetActiveCamera();
	mat4_t model_view_projection_matrix;
	vec4_t p;
	vec4_t sp;
	vec3_t direction;
	int i;
	
	int lshift;
	float d;
	
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
	
	static float prev_dx;
	static float prev_dy;
	
	float amount;
	float z;
	
	brush_ProcessBrushes();
	
	//if(level_editor_editing_mode == EDITOR_EDITING)
	//{
	if(bm_mouse & MOUSE_OVER_WIDGET)
		return;
		
	if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
	{
	//	if(!(bm_mouse & MOUSE_OVER_WIDGET))
	//	{
		editor_LevelEditorCheck3dHandle(normalized_mouse_x, normalized_mouse_y);		
			
		if(!level_editor_3d_handle_flags)
		{
			editor_LevelEditorSet3dCursorPosition(normalized_mouse_x, normalized_mouse_y);
		}
	}
		
	if(!(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
	{
		level_editor_3d_handle_flags = 0;
	}
	else if(level_editor_3d_handle_flags)
	{
		p.vec3 = level_editor_3d_handle_position;
		p.w = 1.0;
		mat4_t_mult_fast(&model_view_projection_matrix, &active_camera->view_data.view_matrix, &active_camera->view_data.projection_matrix);
		mat4_t_vec4_t_mult(&model_view_projection_matrix, &p);
			
				
		p.x /= p.w;
		p.y /= p.w;
		z = p.z;
				
		if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
		{
			if(level_editor_3d_handle_transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_SCALE || level_editor_3d_handle_transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION)
			{
				grab_screen_offset_x = normalized_mouse_x - p.x;
				grab_screen_offset_y = normalized_mouse_y - p.y;
			}
			else
			{
				grab_screen_offset_x = 0.0;
				grab_screen_offset_y = 0.0;
			}
						
		}
				
		screen_dx = normalized_mouse_x - p.x - grab_screen_offset_x;
		screen_dy = normalized_mouse_y - p.y - grab_screen_offset_y;
			
		if(level_editor_3d_handle_flags & ED_3D_HANDLE_X_AXIS_GRABBED)
		{
			direction = vec3(1.0, 0.0, 0.0);
		}
		else if(level_editor_3d_handle_flags & ED_3D_HANDLE_Y_AXIS_GRABBED)
		{
			direction = vec3(0.0, 1.0, 0.0);
		}
		else if(level_editor_3d_handle_flags & ED_3D_HANDLE_Z_AXIS_GRABBED)
		{
			direction = vec3(0.0, 0.0, 1.0);
		}
			
		if(level_editor_3d_handle_transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_SCALE || level_editor_3d_handle_transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION)
		{
			sp = p;	
			p.vec3 = direction;
			p.w = 0.0;
				
			mat4_t_vec4_t_mult(&active_camera->view_data.view_matrix, &p);
			
			screen_x = p.x;
			screen_y = p.y;
				
			amount = sqrt(screen_x * screen_x + screen_y * screen_y);
					
			screen_x /= amount;
			screen_y /= amount;
			
			amount = (screen_dx * screen_x + screen_dy * screen_y) * z;
				
				
			if(level_editor_linear_snap_value > 0.0)
			{
				d = amount / level_editor_linear_snap_value;
		 		amount = level_editor_linear_snap_value * (int)d;
			}
				
				
			if(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED)
			{
				if(level_editor_3d_handle_transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_SCALE)
				{
					if(fabs(amount) > 0.0)
					{
						grab_screen_offset_x = normalized_mouse_x - sp.x;
						grab_screen_offset_y = normalized_mouse_y - sp.y;
					}
						
				}
			}
				
		}
		else
		{
				
			amount = sqrt(screen_dx * screen_dx + screen_dy * screen_dy);
				
			screen_dx /= amount;
			screen_dy /= amount;
				
			if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
			{
				prev_dx = screen_dx;
				prev_dy = screen_dy;
			}
					
			amount = asin(prev_dx * screen_dy - prev_dy * screen_dx);
				
			d = dot3(direction, active_camera->world_orientation.f_axis);
				
			if(d < 0)
			{
				amount = -amount;
			}
			else if(d == 0)
			{
				d = dot3(direction, active_camera->world_orientation.r_axis);
					
				if(d < 0)
				{
					amount = -amount;
				}
			}
				
			if(level_editor_angular_snap_value > 0.0)
			{
				d = amount / level_editor_angular_snap_value;
				amount = level_editor_angular_snap_value * (int)d;
			}
				
			if(amount != 0.0)
			{
				if(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED)
				{
					prev_dx = screen_dx;
					prev_dy = screen_dy;
				}
			}
		}
			 
		if(level_editor_editing_mode == EDITING_MODE_OBJECT)
		{
			switch(level_editor_3d_handle_transform_mode)
			{
				case ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION:
					editor_LevelEditorTranslateSelections(direction, amount);		
				break;
					
				case ED_3D_HANDLE_TRANSFORM_MODE_ROTATION:
					editor_LevelEditorRotateSelections(direction, amount);
				break;
					
				case ED_3D_HANDLE_TRANSFORM_MODE_SCALE:
					editor_LevelEditorScaleSelections(direction, amount);
				break;
			}
			
		}
				
	}
	
	if(level_editor_pick_list.record_count)
	{
		level_editor_draw_3d_handle = 1;
		editor_LevelEditorUpdate3dHandlePosition();
	}
	else
	{
		level_editor_draw_3d_handle = 0;
	}
			
	if(bm_mouse & MOUSE_RIGHT_BUTTON_JUST_CLICKED)
	{
		
		
		//if(level_editor_editing_mode == EDITING_MODE_OBJECT)
		{
			//editor_PickObject();	
			editor_LevelEditorPickObject(normalized_mouse_x, normalized_mouse_y);
		}
		/*else
		{
			if(level_editor_editing_mode == EDITING_MODE_UV)
			{
				editor_CloseBrushFaceUVWindow();
			}
			i = level_editor_selections[level_editor_selection_count - 1].index0;
				
			editor_PickOnBrush(&brushes[i]);
		}*/
			
	}
	
	
	
	
	
	//if(editor_state == EDITOR_EDITING)
	//{
	if(input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED)
	{
		if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_JUST_PRESSED)
		{
			if(level_editor_editing_mode == EDITING_MODE_OBJECT)
			{
				editor_LevelEditorOpenAddToWorldMenu(r_window_width * normalized_mouse_x * 0.5, r_window_height * normalized_mouse_y * 0.5);
				//editor_OpenAddToWorldMenu(r_window_width * normalized_mouse_x * 0.5, r_window_height * normalized_mouse_y * 0.5);
			}
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_M) & KEY_JUST_PRESSED)
		{
			//editor_OpenMaterialWindow();
			//editor_ToggleMaterialWindow();
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_T) & KEY_JUST_PRESSED)
		{
			//editor_ToggleTextureWindow();
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_V) & KEY_JUST_PRESSED)
		{
			//editor_ToggleEntityDefViewerWindow();
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_D) & KEY_JUST_PRESSED)
		{
			if(level_editor_editing_mode == EDITING_MODE_OBJECT)
			{
				//editor_CopySelections(&level_editor_pick_list);
				editor_LevelEditorCopySelections();
			}
				
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_L) & KEY_JUST_PRESSED)
		{
			editor_LevelEditorOpenWaypointOptionMenu(r_window_width * normalized_mouse_x * 0.5, r_window_height * normalized_mouse_y * 0.5);
		}
		
		if(input_GetKeyStatus(SDL_SCANCODE_SPACE) & KEY_JUST_PRESSED)
		{
			renderer_ToggleFullscreen();
		}
	}
	else if(input_GetKeyStatus(SDL_SCANCODE_G) & KEY_JUST_PRESSED)
	{
		editor_LevelEditorSet3dHandleTransformMode(ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION);
	}
	else if(input_GetKeyStatus(SDL_SCANCODE_R) & KEY_JUST_PRESSED)
	{
		editor_LevelEditorSet3dHandleTransformMode(ED_3D_HANDLE_TRANSFORM_MODE_ROTATION);
	}
	else if(input_GetKeyStatus(SDL_SCANCODE_S) & KEY_JUST_PRESSED)
	{
		editor_LevelEditorSet3dHandleTransformMode(ED_3D_HANDLE_TRANSFORM_MODE_SCALE);
	}
	else if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_JUST_PRESSED)
	{
		if(level_editor_editing_mode == EDITING_MODE_OBJECT)
		{
			editor_LevelEditorClearSelections();
			//editor_CloseLightPropertiesWindow();
			//editor_CloseBrushPropertiesWindow();
		}
			
	}
	else if(input_GetKeyStatus(SDL_SCANCODE_P) & KEY_JUST_PRESSED)
	{
		editor_LevelEditorStartPIE();
	}
	else if(input_GetKeyStatus(SDL_SCANCODE_DELETE) & KEY_JUST_PRESSED)
	{
		if(level_editor_editing_mode == EDITING_MODE_OBJECT)
		{
			editor_LevelEditorOpenDeleteSelectionsMenu(r_window_width * normalized_mouse_x * 0.5, r_window_height * normalized_mouse_y * 0.5);
		}
	}
	else if(input_GetKeyStatus(SDL_SCANCODE_TAB) & KEY_JUST_PRESSED)
	{
		record = editor_GetLastSelection(&level_editor_pick_list);
		
		if(record.type == PICK_BRUSH)
		{
			if(level_editor_editing_mode == EDITING_MODE_BRUSH || level_editor_editing_mode == EDITING_MODE_UV)
			{
				editor_LevelEditorSetEditingMode(EDITING_MODE_OBJECT);
			}
			else
			{
				editor_LevelEditorSetEditingMode(EDITING_MODE_BRUSH);
			}
		}
		
		
		/*if(ed_editing_mode == EDITING_MODE_BRUSH || ed_editing_mode == EDITING_MODE_UV)
		{
			editor_SetEditingMode(EDITING_MODE_OBJECT);
			ed_selected_brush_polygon_index = -1;
		}
		else
		{
			if(ed_selection_count)
			{
				if(ed_selections[ed_selection_count - 1].type == PICK_BRUSH)
				{
					editor_SetEditingMode(EDITING_MODE_BRUSH);
				}
			}
		}*/
	}		
	else if(input_GetKeyStatus(SDL_SCANCODE_ESCAPE) & KEY_JUST_PRESSED)
	{
		editor_LevelEditorStopPIE();
	}
	
	
	
	//}
}



void editor_LevelEditorFly(float delta_time)
{	
	/* avoid the camera to snap at a random direction when
	the engine change states... */
	if(last_mouse_x || last_mouse_y)
	{
		//normalized_mouse_x = 0.0;
		//normalized_mouse_y = 0.0;
	//	mouse_dx = 0.0;
	// 	mouse_dy = 0.0;
	}
	
	vec3_t forward_vector;
	vec3_t right_vector;
	vec3_t translation = {0.0, 0.0, 0.0};
	vec3_t velocity = {0.0, 0.0, 0.0};
		 
		
	level_editor_camera_yaw -= (mouse_dx) * 0.5;
	level_editor_camera_pitch += (mouse_dy) * 0.5;
		
		
	if(level_editor_camera_pitch > 0.5) level_editor_camera_pitch = 0.5;
	else if(level_editor_camera_pitch < -0.5) level_editor_camera_pitch = -0.5;
			
	if(level_editor_camera_yaw > 1.0) level_editor_camera_yaw = -1.0 + (level_editor_camera_yaw - 1.0);
	else if(level_editor_camera_yaw < -1.0) level_editor_camera_yaw = 1.0 + (level_editor_camera_yaw + 1.0);
		
	camera_PitchYawCamera(level_editor_camera, level_editor_camera_yaw, level_editor_camera_pitch);
	
	forward_vector = level_editor_camera->world_orientation.f_axis;
	right_vector = level_editor_camera->world_orientation.r_axis;
				
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
		level_editor_camera_speed_multiplier += 0.05;
	}
	else if(bm_mouse & MOUSE_WHEEL_DOWN)
	{
		level_editor_camera_speed_multiplier -= 0.05;
		if(level_editor_camera_speed_multiplier < 0.05) level_editor_camera_speed_multiplier = 0.05;
	}
		
	translation.x *= level_editor_camera_speed_multiplier * delta_time * 0.045;
	translation.y *= level_editor_camera_speed_multiplier * delta_time * 0.045;
	translation.z *= level_editor_camera_speed_multiplier * delta_time * 0.045;
		
	velocity.x = translation.x;
	velocity.y = translation.y;
	velocity.z = translation.z;
				 
	camera_TranslateCamera(level_editor_camera, velocity, 1.0, 0);		
}


/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorAddSelection(pick_record_t *record)
{
	editor_AddSelection(record, &level_editor_pick_list);
}

void editor_LevelEditorDropSelection(pick_record_t *record)
{
	editor_DropSelection(record, &level_editor_pick_list);
}

void editor_LevelEditorClearSelections()
{
	editor_ClearSelection(&level_editor_pick_list);
}

void editor_LevelEditorCopySelections()
{
	editor_CopySelections(&level_editor_pick_list);
}

void editor_LevelEditorDestroySelections()
{

}

pick_record_t editor_LevelEditorGetLastSelection()
{
	if(level_editor_pick_list.record_count)
	{
		return level_editor_pick_list.records[level_editor_pick_list.record_count - 1];
	}
	
	return (pick_record_t){PICK_NONE, 0, NULL, 0, 0};
	
}

/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorTranslateSelections(vec3_t direction, float amount)
{
	editor_TranslateSelections(&level_editor_pick_list, direction, amount);
	level_editor_need_to_copy_data = 1;
}

void editor_LevelEditorRotateSelections(vec3_t axis, float amount)
{
	editor_RotateSelections(&level_editor_pick_list, axis, amount, level_editor_3d_handle_position);
	level_editor_need_to_copy_data = 1;
}

void editor_LevelEditorScaleSelections(vec3_t axis, float amount)
{
	editor_ScaleSelections(&level_editor_pick_list, axis, amount);
	level_editor_need_to_copy_data = 1;
}

/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorStartPIE()
{
	if(level_editor_need_to_copy_data)
	{
		editor_LevelEditorCopyLevelData();
	}
	
	engine_SetEngineState(ENGINE_PLAYING);
	level_editor_state = EDITOR_PIE;
}

void editor_LevelEditorStopPIE()
{
	engine_SetEngineState(ENGINE_PAUSED);
	editor_LevelEditorRestoreLevelData();
	level_editor_state = EDITOR_EDITING;
	camera_SetCamera(level_editor_camera);
}

/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorCopyLevelData()
{
	int i;
	
	level_editor_world_vertices_count = w_world_vertices_count;
	level_editor_world_vertices = w_world_vertices;
	
	level_editor_world_nodes_count = w_world_nodes_count;
	level_editor_world_nodes = w_world_nodes;
	
	level_editor_world_leaves_count = w_world_leaves_count;
	level_editor_world_leaves = w_world_leaves;
	
	level_editor_world_batch_count = w_world_batch_count;
	level_editor_world_batches = w_world_batches;
	
	/************************************************************************************************/
	
	if(ent_entity_list_size >= level_editor_entity_list_size)
	{
		if(level_editor_entities)
		{
			memory_Free(level_editor_entities);
			memory_Free(level_editor_entity_aabbs);
			memory_Free(level_editor_entity_free_stack);
		}
		level_editor_entities = memory_Malloc(sizeof(struct entity_t ) * ent_entity_list_size, "editor_LevelEditorCopyLevelData");
		level_editor_entity_aabbs = memory_Malloc(sizeof(struct entity_aabb_t) * ent_entity_list_size, "editor_LevelEditorCopyLevelData");
		level_editor_entity_free_stack = memory_Malloc(sizeof(int) * ent_entity_list_size, "editor_LevelEditorCopyLevelData");
		level_editor_entity_list_size = ent_entity_list_size;
	}
	
	for(i = 0; i < ent_entity_list_cursor; i++)
	{
		level_editor_entities[i] = ent_entities[i];
//		level_editor_entity_aabbs[i] = ent_aabbs[i];
		level_editor_entity_free_stack[i] = ent_free_stack[i];
	}
	
	level_editor_entity_list_cursor = ent_entity_list_cursor;
	level_editor_entity_free_stack_top = ent_free_stack_top;
	
	/************************************************************************************************/
	
	if(max_colliders >= level_editor_collider_list_size)
	{
		if(level_editor_colliders)
		{
			memory_Free(level_editor_colliders);
			memory_Free(level_editor_collider_free_stack);
		}
		
		level_editor_colliders = memory_Malloc(sizeof(collider_t) * max_colliders, "editor_LevelEditorCopyLevelData");
		level_editor_collider_free_stack = memory_Malloc(sizeof(int) * max_colliders, "editor_LevelEditorCopyLevelData");
	}
	
	for(i = 0; i < collider_list_cursor; i++)
	{
		level_editor_colliders[i] = colliders[i];
		level_editor_collider_free_stack[i] = colliders_free_positions_stack[i];
	}
	
	level_editor_collider_list_size = max_colliders;
	level_editor_collider_list_cursor = collider_list_cursor;
	level_editor_collider_free_stack_top = colliders_free_positions_stack_top;
	
	
	
	/************************************************************************************************/
	
	if(l_light_list_size >= level_editor_light_list_size)
	{
		if(level_editor_light_params)
		{
			memory_Free(level_editor_light_params);
			memory_Free(level_editor_light_positions);
			memory_Free(level_editor_light_free_stack);
		}
		
		level_editor_light_params = memory_Malloc(sizeof(light_params_t) * l_light_list_size, "editor_LevelEditorCopyLevelData");
		level_editor_light_positions = memory_Malloc(sizeof(light_position_t) * l_light_list_size, "editor_LevelEditorCopyLevelData");
		level_editor_light_free_stack = memory_Malloc(sizeof(int) * l_light_list_size, "editor_LevelEditorCopyLevelData");
		
		level_editor_light_list_size = l_light_list_size;
	}
	
	for(i = 0; i < l_light_list_cursor; i++)
	{
		level_editor_light_positions[i] = l_light_positions[i];
		level_editor_light_params[i] = l_light_params[i];
		level_editor_light_free_stack[i] = l_free_position_stack[i];
	}
	
	level_editor_light_list_cursor = l_light_list_cursor;
	level_editor_light_free_stack_top = l_free_position_stack_top;
	
	/************************************************************************************************/
	
	level_editor_has_copied_data = 1;
	level_editor_need_to_copy_data = 0;
}

void editor_LevelEditorClearLevel()
{
	if(level_editor_has_copied_data)
	{
		w_world_vertices_count = 0;
		w_world_vertices = NULL;
		
		w_world_nodes_count = 0;
		w_world_nodes = NULL;
		
		w_world_leaves_count = 0;
		w_world_leaves = NULL;
		
		w_world_batch_count = 0;
		w_world_batches = NULL;
		
		/************************************************************************************************/
		
		ent_entity_list_cursor = 0;
		ent_free_stack_top = -1;
		
		/************************************************************************************************/
		
		collider_list_cursor = 0;
		colliders_free_positions_stack_top = -1;
		
		/************************************************************************************************/
		
		l_light_list_cursor = 0;
		l_free_position_stack_top = -1;
	}
	
}

void editor_LevelEditorRestoreLevelData()
{
	int i;
	
	struct entity_def_t *entity_def;
	struct entity_t *entity;
	collider_def_t *collider_def;
	collider_t *collider;
	
	if(level_editor_has_copied_data)
	{
		w_world_vertices_count = level_editor_world_vertices_count;
		w_world_vertices = level_editor_world_vertices;
		
		w_world_nodes_count = level_editor_world_nodes_count;
		w_world_nodes = level_editor_world_nodes;
		
		w_world_leaves_count = level_editor_world_leaves_count;
		w_world_leaves = level_editor_world_leaves;
		
		w_world_batch_count = level_editor_world_batch_count;
		w_world_batches = level_editor_world_batches;
		
		/************************************************************************************************/
		
		for(i = 0; i < level_editor_collider_list_cursor; i++)
		{
			colliders[i] = level_editor_colliders[i];
			colliders_free_positions_stack[i] = level_editor_collider_free_stack[i];
		}
		
		collider_list_cursor = level_editor_collider_list_cursor;
		colliders_free_positions_stack_top = level_editor_collider_free_stack_top;
		
		/************************************************************************************************/
		
		for(i = 0; i < level_editor_entity_list_cursor; i++)
		{
			ent_entities[i] = level_editor_entities[i];
//			ent_aabbs[i] = level_editor_entity_aabbs[i];
			ent_free_stack[i] = level_editor_entity_free_stack[i];
			
			#if 0
			
			if(!(ent_entities[i].flags & ENTITY_INVALID))
			{
				entity = &ent_entities[i];
				entity_def = entity_GetEntityDefPointerIndex(entity->def_index);
				
				/* make sure entities spawned without a collider get one created
				once their def got a collider def... */
				if(entity->collider_index == -1 && entity_def->collider_def)
				{
					entity->collider_index = physics_CreateCollider(&entity->orientation, entity->position, entity->scale, entity_def->collider_def, 0);
					level_editor_need_to_copy_data = 1;
				}
				else
				{
					//collider = physics_GetColliderPointerIndex(entity->collider_index);
					if(entity_def->collider_def)
					{
						physics_SetColliderPosition(entity->collider_index, entity->position);
						physics_SetColliderOrientation(entity->collider_index, &entity->orientation);
						physics_SetColliderScale(entity->collider_index, entity->scale);
					}
					else
					{
						physics_DestroyColliderIndex(entity->collider_index);
						entity->collider_index = -1;
					}	
				}
			}
			
			#endif
		}
		
		ent_entity_list_cursor = level_editor_entity_list_cursor;
		ent_free_stack_top = level_editor_entity_free_stack_top;
		
		for(i = 0; i < level_editor_light_list_cursor; i++)
		{
			l_light_params[i] = level_editor_light_params[i];
			l_light_positions[i] = level_editor_light_positions[i];
			l_free_position_stack[i] = level_editor_light_free_stack[i];
		}
		
		l_light_list_cursor = level_editor_light_list_cursor;
		l_free_position_stack_top = level_editor_light_free_stack_top;
		
	}
	
	
	
}











