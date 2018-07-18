#include "ed_entity.h"
#include "ed_entity_draw.h"
#include "ed_entity_ui.h"

#include <stdio.h>

#include "..\ed_common.h"
#include "..\ed_selection.h"
#include "..\editor.h"

#include "..\..\common\camera.h"
#include "..\..\common\gmath\vector.h"
#include "..\..\common\gmath\matrix.h"
#include "..\..\common\input.h"
#include "..\..\common\r_main.h"
#include "..\..\common\entity.h"
#include "..\..\common\model.h"
#include "..\..\common\l_main.h"
#include "..\..\common\memory.h"
#include "..\..\common\GLEW\include\GL\glew.h"


/* from input.c */
extern int bm_mouse;
extern float normalized_mouse_x;
extern float normalized_mouse_y;
extern float mouse_dx;
extern float mouse_dy;
extern int mouse_x;
extern int mouse_y;

/* from r_main.c */
extern int r_window_width;
extern int r_window_height;


/* from physics.c */
extern collider_def_t *collider_defs;


/*
==================================================================
==================================================================
==================================================================
*/


camera_t *entity_editor_camera = NULL;
int entity_editor_light_index = -1;

pick_list_t entity_editor_pick_list;


vec3_t entity_editor_3d_cursor_position = {0.0, 0.0, 0.0};
vec3_t entity_editor_3d_handle_position = {0.0, 0.0, 0.0};
int entity_editor_3d_handle_transform_mode = ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION;
int entity_editor_3d_handle_flags = 0;


vec3_t entity_editor_camera_pivot_center = {0.0, 0.0, 0.0};
float entity_editor_camera_distance = 10.0;
float entity_editor_camera_v_angle = 0.0;
float entity_editor_camera_h_angle = 0.0;
float entity_editor_orthographic_v_angle = 0.0;
float entity_editor_orthographic_h_angle = 0.0;
float entity_editor_linear_snap_value = 0.0;
float entity_editor_angular_snap_value = 0.0;


int entity_editor_draw_3d_handle = 0;
int entity_editor_draw_3d_cursor = 1;
int entity_editor_draw_grid = 1;
int entity_editor_draw_collider_def = 1;
int entity_editor_draw_entity_def = 1;
int entity_editor_orthographic_mode = 0;



//struct entity_def_t *entity_editor_current_entity_def = NULL;

struct entity_handle_t ed_entity_editor_entity_def = {1, INVALID_ENTITY_INDEX};
struct entity_handle_t ed_entity_editor_preview_entity = {1, INVALID_ENTITY_INDEX};
int ed_entity_editor_update_preview_entity = 0;


/*
==================================================================
==================================================================
==================================================================
*/

void editor_EntityEditorInit()
{
	mat3_t orientation = mat3_t_id();
	entity_editor_camera = camera_CreateCamera("entity editor camera", vec3_t_c(0.0, 0.0, 0.0), &orientation, 0.68, r_window_width, r_window_height, 0.1, 100.0, 0);
	
	camera_Deactivate(entity_editor_camera);
	
	entity_editor_camera_v_angle = -0.15;
	entity_editor_camera_h_angle = 0.20;
	entity_editor_camera_distance = 20.0;
	
	editor_EntityEditorMoveCamera();
	
	
	entity_editor_pick_list.max_records = 1024;
	entity_editor_pick_list.record_count = 0;
	entity_editor_pick_list.records = memory_Malloc(sizeof(pick_record_t) * entity_editor_pick_list.max_records, "editor_EntityEditorInit");
	entity_editor_pick_list.last_selection_type = PICK_NONE;
	
	editor_EntityEditorInitUI();
	
}
 
void editor_EntityEditorFinish()
{
	editor_EntityEditorFinishUI();
	memory_Free(entity_editor_pick_list.records);
}

void editor_EntityEditorSetup()
{
	mat3_t light_orientation = mat3_t_id();
	camera_SetCamera(entity_editor_camera);
	camera_SetMainViewCamera(entity_editor_camera);
	
	renderer_RegisterCallback(editor_EntityEditorPreDraw, PRE_SHADING_STAGE_CALLBACK);
	renderer_RegisterCallback(editor_EntityEditorPostDraw, POST_SHADING_STAGE_CALLBACK);
	renderer_SetClearColor(1.0, 1.0, 2.8);
	
	//entity_editor_current_entity_def = entity_GetEntityDefPointer("toilet");
	//entity_editor_current_entity_def = entity_GetEntityPointer("toilet", 1);
	entity_editor_light_index = light_CreateLight("entity editor light", &light_orientation, vec3_t_c(0.0, 0.0, 0.0), vec3_t_c(1.0, 1.0, 1.0), 10.0, 10.0, 0);
	
	
	
	
}

void editor_EntityEditorShutdown()
{
	renderer_ClearRegisteredCallbacks();
	renderer_SetClearColor(0.0, 0.0, 0.0);
	light_DestroyLightIndex(entity_editor_light_index);
	camera_Deactivate(entity_editor_camera);
	
	
	if(ed_entity_editor_preview_entity.entity_index != INVALID_ENTITY_INDEX)
	{
		entity_RemoveEntity(ed_entity_editor_preview_entity);
		ed_entity_editor_preview_entity.entity_index = INVALID_ENTITY_INDEX;
		ed_entity_editor_update_preview_entity = 1;
	}
	
	
}

void editor_EntityEditorRestart()
{
	
}

void editor_EntityEditorMain(float delta_time)
{
	
	if(entity_editor_orthographic_mode)
	{
		CreateOrthographicMatrix(&entity_editor_camera->view_data.projection_matrix, -r_window_width * 0.005 * entity_editor_camera_distance, r_window_width * 0.005 * entity_editor_camera_distance, r_window_height * 0.005 * entity_editor_camera_distance, -r_window_height * 0.005 * entity_editor_camera_distance, -100.0, 100.0, &entity_editor_camera->frustum);
	}
	else
	{
		CreatePerspectiveMatrix(&entity_editor_camera->view_data.projection_matrix, 0.68, (float)r_window_width/(float)r_window_height, 0.1, 100.0, 0.0, 0.0, &entity_editor_camera->frustum);
	}
	
	
	
	if(!(bm_mouse & MOUSE_OVER_WIDGET))
	{
		if(bm_mouse & MOUSE_MIDDLE_BUTTON_CLICKED)
		{
			//if(!entity_editor_orthographic_mode)
			{
				editor_EntityEditorMoveCamera();
			}
		}
		else
		{
			editor_EntityEditorEdit();
		}
		
		editor_EntityEditorUpdateCamera();
	}	
	
	editor_EntityEditorUpdatePreviewEntity();
	editor_EntityEditorUpdateUI();
}


/*
===============================================================
===============================================================
===============================================================
*/


void editor_EntityEditorCheck3dHandle(float mouse_x, float mouse_y)
{
	entity_editor_3d_handle_flags = editor_Check3dHandle(mouse_x, mouse_y, entity_editor_3d_handle_position, entity_editor_3d_handle_transform_mode);
}

void editor_EntityEditorSet3dCursorPosition(float mouse_x, float mouse_y)
{
	entity_editor_3d_cursor_position = editor_3dCursorPosition(mouse_x, mouse_y);
}
 
void editor_EntityEditorUpdate3dHandlePosition()
{
	int i;
	
	collision_shape_t *collision_shapes;
	
	//if(entity_editor_current_entity_def)
	{
		/*if(entity_editor_current_entity_def->collider_def)
		{
			
			collision_shapes = entity_editor_current_entity_def->collider_def->collider_data.generic_collider_data.collision_shape;
			
			entity_editor_3d_handle_position.x = 0.0;
			entity_editor_3d_handle_position.y = 0.0;
			entity_editor_3d_handle_position.z = 0.0;
			
			for(i = 0; i < entity_editor_pick_list.record_count; i++)
			{
				entity_editor_3d_handle_position.x += collision_shapes[entity_editor_pick_list.records[i].index0].position.x;
				entity_editor_3d_handle_position.y += collision_shapes[entity_editor_pick_list.records[i].index0].position.y;
				entity_editor_3d_handle_position.z += collision_shapes[entity_editor_pick_list.records[i].index0].position.z;
			}
			
			entity_editor_3d_handle_position.x /= entity_editor_pick_list.record_count;
			entity_editor_3d_handle_position.y /= entity_editor_pick_list.record_count;
			entity_editor_3d_handle_position.z /= entity_editor_pick_list.record_count;
		}*/
	}
	
}

void editor_EntityEditorSet3dHandleTransformMode(int mode)
{
	switch(mode)
	{
		case ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION:
		case ED_3D_HANDLE_TRANSFORM_MODE_ROTATION:
		case ED_3D_HANDLE_TRANSFORM_MODE_SCALE:
		
		break;
		
		default:
			return;	
	}
	
	entity_editor_3d_handle_transform_mode = mode;
}


/*
===============================================================
===============================================================
===============================================================
*/


void editor_EntityEditorMoveCamera()
{
	mat3_t orientation = mat3_t_id();
	
	if(input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED)
	{
		entity_editor_camera_pivot_center.x += -entity_editor_camera->world_orientation.r_axis.floats[0] * mouse_dx * entity_editor_camera_distance - 
												entity_editor_camera->world_orientation.u_axis.floats[0] * mouse_dy * entity_editor_camera_distance;
												
												
		entity_editor_camera_pivot_center.y += -entity_editor_camera->world_orientation.r_axis.floats[1] * mouse_dx * entity_editor_camera_distance - 
												entity_editor_camera->world_orientation.u_axis.floats[1] * mouse_dy * entity_editor_camera_distance;
												
												
		entity_editor_camera_pivot_center.z += -entity_editor_camera->world_orientation.r_axis.floats[2] * mouse_dx * entity_editor_camera_distance - 
												entity_editor_camera->world_orientation.u_axis.floats[2] * mouse_dy * entity_editor_camera_distance;
	}
	else
	{
		entity_editor_camera_v_angle += mouse_dy * 0.5;
		entity_editor_camera_h_angle -= mouse_dx * 0.5;
		
		/*if(entity_editor_orthographic_mode)
		{
			mat3_t_rotate(&orientation, vec3(1.0, 0.0, 0.0), entity_editor_orthographic_v_angle, 1);
			mat3_t_rotate(&orientation, vec3(0.0, 1.0, 0.0), entity_editor_orthographic_h_angle, 0);
		}
		else*/
		{
			if(entity_editor_camera_v_angle > 1.0) entity_editor_camera_v_angle = -1.0 + (entity_editor_camera_v_angle - 1.0);
			else if(entity_editor_camera_v_angle < -1.0) entity_editor_camera_v_angle = 1.0 - (entity_editor_camera_v_angle + 1.0);
			
			if(entity_editor_camera_h_angle > 1.0) entity_editor_camera_h_angle = -1.0 + (entity_editor_camera_h_angle - 1.0);
			else if(entity_editor_camera_h_angle < -1.0) entity_editor_camera_h_angle = 1.0 - (entity_editor_camera_h_angle + 1.0);
			
			mat3_t_rotate(&orientation, vec3_t_c(1.0, 0.0, 0.0), entity_editor_camera_v_angle, 1);
			mat3_t_rotate(&orientation, vec3_t_c(0.0, 1.0, 0.0), entity_editor_camera_h_angle, 0);
		}
		
		
		entity_editor_camera->world_orientation = orientation;
	}
	
}


void editor_EntityEditorUpdateCamera()
{
	
	light_ptr_t entity_editor_light;
	
	if(bm_mouse & MOUSE_WHEEL_DOWN)
	{
		entity_editor_camera_distance += 0.5;
	}
	else if(bm_mouse & MOUSE_WHEEL_UP)
	{
		entity_editor_camera_distance -= 0.5;
		if(entity_editor_camera_distance < 0.5)
		{
			entity_editor_camera_distance = 0.5;
		}
	}
	
	entity_editor_light = light_GetLightPointerIndex(entity_editor_light_index);
	
	if(!entity_editor_orthographic_mode)
	{
		entity_editor_camera->world_position.x = entity_editor_camera->world_orientation.f_axis.floats[0] * entity_editor_camera_distance + entity_editor_camera_pivot_center.x;
		entity_editor_camera->world_position.y = entity_editor_camera->world_orientation.f_axis.floats[1] * entity_editor_camera_distance + entity_editor_camera_pivot_center.y;
		entity_editor_camera->world_position.z = entity_editor_camera->world_orientation.f_axis.floats[2] * entity_editor_camera_distance + entity_editor_camera_pivot_center.z;
	}
	
	
	 
	entity_editor_light.position->position = entity_editor_camera->world_position;
	entity_editor_light.params->bm_flags |= LIGHT_MOVED;
	entity_editor_light.params->radius = SET_LIGHT_RADIUS(entity_editor_camera_distance);
	entity_editor_light.params->energy = SET_LIGHT_ENERGY((entity_editor_camera_distance * entity_editor_camera_distance + 10.0));
	
	//printf("%d\n", entity_editor_light.params->energy);
		
	camera_ComputeWorldToCameraMatrix(entity_editor_camera);	
}


void editor_EntityEditorUpdatePreviewEntity()
{
	if(ed_entity_editor_entity_def.entity_index != INVALID_ENTITY_INDEX)
	{
		//if(ed_entity_editor_preview_entity.entity_index != ed_entity_editor_entity_def.entity_index)
		if(ed_entity_editor_update_preview_entity)
		{
			if(ed_entity_editor_preview_entity.entity_index != INVALID_ENTITY_INDEX)
			{
				entity_RemoveEntity(ed_entity_editor_preview_entity);
				ed_entity_editor_preview_entity.entity_index = INVALID_ENTITY_INDEX;
			}
			
			ed_entity_editor_preview_entity = entity_SpawnEntity(NULL, vec3_t_c(0.0, 0.0, 0.0), vec3_t_c(1.0, 1.0, 1.0), ed_entity_editor_entity_def, "preview");
			
			ed_entity_editor_update_preview_entity = 0;
		}
	}
}



void editor_EntityEditorEdit()
{
	pick_record_t record;
	int lshift;
	
	vec4_t p;
	vec4_t sp;
	float z;
	double amount;
	float d;
	
	static float grab_screen_offset_x = 0.0;
	static float grab_screen_offset_y = 0.0;
	static float screen_dx = 0.0;
	static float screen_dy = 0.0;
	static float screen_x = 0.0;
	static float screen_y = 0.0;
	static float prev_dx = 0.0;
	static float prev_dy = 0.0;
	
	mat4_t model_view_projection_matrix;
	camera_t *active_camera = camera_GetActiveCamera();
	vec3_t direction;
	
	if(bm_mouse & MOUSE_OVER_WIDGET)
	{
		return;
	}
	
	lshift = input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED;
	
	if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
	{
		entity_editor_3d_handle_flags = 0;
		
		if(entity_editor_draw_3d_handle)
		{
			editor_EntityEditorCheck3dHandle(normalized_mouse_x, normalized_mouse_y);
		}
		
		if(!entity_editor_3d_handle_flags)
		{
			editor_EntityEditorSet3dCursorPosition(normalized_mouse_x, normalized_mouse_y);
		}
	}
	
	if(!(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
	{
		entity_editor_3d_handle_flags = 0;
	}
	
	if(entity_editor_3d_handle_flags) 
	{
		
		if(entity_editor_3d_handle_flags & ED_3D_HANDLE_X_AXIS_GRABBED)
		{
			direction = vec3_t_c(1.0, 0.0, 0.0);
		}
		else if(entity_editor_3d_handle_flags & ED_3D_HANDLE_Y_AXIS_GRABBED)
		{
			direction = vec3_t_c(0.0, 1.0, 0.0);
		}
		else if(entity_editor_3d_handle_flags & ED_3D_HANDLE_Z_AXIS_GRABBED)
		{
			direction = vec3_t_c(0.0, 0.0, 1.0);
		}
		
		amount = editor_GetMouseOffsetFrom3dHandle(normalized_mouse_x, normalized_mouse_y, entity_editor_3d_handle_position, direction, entity_editor_3d_handle_transform_mode, entity_editor_linear_snap_value, entity_editor_angular_snap_value);
				 
		//if(entity_editor_editing_mode == EDITING_MODE_OBJECT)
		{
			switch(entity_editor_3d_handle_transform_mode)
			{
				case ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION:
					editor_EntityEditorTranslateSelections(direction, amount);		
				break;
					
				case ED_3D_HANDLE_TRANSFORM_MODE_ROTATION:
					editor_EntityEditorRotateSelections(direction, amount);
				break;
						
				case ED_3D_HANDLE_TRANSFORM_MODE_SCALE:
					editor_EntityEditorScaleSelections(direction, amount);
				break;
			}
			
		}
	}
	
	else if(bm_mouse & MOUSE_RIGHT_BUTTON_JUST_CLICKED)
	{
		record = editor_EntityEditorPickColliderPrimitive(normalized_mouse_x, normalized_mouse_y);
		
		if(record.type != PICK_NONE)
		{
		
			//printf("%d\n", record.index0);	
		
			if(record.type == entity_editor_pick_list.records[entity_editor_pick_list.record_count - 1].type)
			{	
				/* if the just picked thing is the same as the last picked thing... */	
				if(record.index0 == entity_editor_pick_list.records[entity_editor_pick_list.record_count - 1].index0)
				{
					_same_selection:
					/* this record already exists in the list, so drop it... */
					editor_EntityEditorDropSelection(&record);
					
					if(!lshift)
					{
						if(entity_editor_pick_list.record_count)
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
					editor_EntityEditorClearSelections();
				}
												
				editor_EntityEditorAddSelection(&record);
									
				_set_handle_3d_position:
					
				editor_EntityEditorUpdate3dHandlePosition();
							
			}
		}
	}
	
	if(entity_editor_pick_list.record_count)
	{
		entity_editor_draw_3d_handle = 1;
		editor_EntityEditorUpdate3dHandlePosition();
	}
	else
	{
		entity_editor_draw_3d_handle = 0;
	}
	
	if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_JUST_PRESSED)
	{
		if(lshift)
		{
			//editor_EntityEditorOpenAddComponentMenu(mouse_x, r_window_height - mouse_y);
			//editor_EntityEditorOpenAddColliderPrimitiveMenu(r_window_width * 0.5 * normalized_mouse_x, r_window_height * 0.5 * normalized_mouse_y);
		}
		else
		{
			editor_EntityEditorClearSelections();
		}
		
	}
	else if(input_GetKeyStatus(SDL_SCANCODE_T) & KEY_JUST_PRESSED)
	{
		if(lshift)
		{
			editor_EntityEditorToggleDefsMenu();
		}
	}
	else if(input_GetKeyStatus(SDL_SCANCODE_G) & KEY_JUST_PRESSED)
	{
		editor_EntityEditorSet3dHandleTransformMode(ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION);
	}
	else if(input_GetKeyStatus(SDL_SCANCODE_S) & KEY_JUST_PRESSED)
	{
		editor_EntityEditorSet3dHandleTransformMode(ED_3D_HANDLE_TRANSFORM_MODE_SCALE);
	}
	else if(input_GetKeyStatus(SDL_SCANCODE_DELETE) & KEY_JUST_PRESSED)
	{
		//editor_EntityEditorOpenDestroySelectionMenu(r_window_width * 0.5 * normalized_mouse_x, r_window_height * 0.5 * normalized_mouse_y);
	}
	
	
	
	
	
	
}


/*
===============================================================
===============================================================
===============================================================
*/


pick_record_t editor_EntityEditorPickColliderPrimitive(float mouse_x, float mouse_y)
{

	int x;
	int y;
	
	int pick_type;

	
	pick_record_t record;
	float q[4];
	
	record.type = PICK_NONE;
	
	//if(entity_editor_current_entity_def)
	{
		/*if(entity_editor_current_entity_def->collider_def)
		{
			editor_EnablePicking();
			editor_EntityEditorDrawColliderDef(1);
				
						
			x = r_window_width * (mouse_x * 0.5 + 0.5);
			y = r_window_height * (mouse_y * 0.5 + 0.5);
			glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, q);
					
			editor_DisablePicking();
			
			record.type = *(int *)&q[0];
		
				
			switch(record.type)
			{
				case PICK_COLLIDER_PRIMITIVE:
					record.pointer = entity_editor_current_entity_def->collider_def;
					record.index0 = (*(int *)&q[1]) - 1;
				break;
			}
		}*/
		
	}
	
	return record;
}

void editor_EntityEditorAddSelection(pick_record_t *record)
{
	editor_AddSelection(record, &entity_editor_pick_list);
}

void editor_EntityEditorDropSelection(pick_record_t *record)
{
	editor_DropSelection(record, &entity_editor_pick_list);
}

void editor_EntityEditorClearSelections()
{
	editor_ClearSelection(&entity_editor_pick_list);
}

void editor_EntityEditorCopySelections()
{
	
}

void editor_EntityEditorDestroySelections()
{
	int i;
	collision_shape_t *collision_shapes;
	
	//if(!entity_editor_current_entity_def)
	{
		return;
	}
	
/*	if(!entity_editor_current_entity_def->collider_def)
	{
		return;
	}
	
	collision_shapes = entity_editor_current_entity_def->collider_def->collider_data.generic_collider_data.collision_shape;
	
	for(i = 0; i < entity_editor_pick_list.record_count; i++)
	{
		switch(entity_editor_pick_list.records[i].type)
		{
			case PICK_COLLIDER_PRIMITIVE:
				physics_RemoveCollisionShape(entity_editor_current_entity_def->collider_def, entity_editor_pick_list.records[i].index0);
			break;
		}
	}
	
	if(!entity_editor_current_entity_def->collider_def->collider_data.generic_collider_data.collision_shape_count)
	{
		physics_DestroyColliderDef(entity_editor_current_entity_def->collider_def->name);
		entity_editor_current_entity_def->collider_def = NULL;
	}*/
}

void editor_EntityEditorTranslateSelections(vec3_t direction, float amount)
{
	int i;
	collision_shape_t *collision_shapes;
	vec3_t translation;
	
	translation.x = direction.x * amount;
	translation.y = direction.y * amount;
	translation.z = direction.z * amount;
	
	//if(!entity_editor_current_entity_def)
	{
		return;
	}
	
	/*if(!entity_editor_current_entity_def->collider_def)
	{
		return;
	}
	
	collision_shapes = entity_editor_current_entity_def->collider_def->collider_data.generic_collider_data.collision_shape;
	
	for(i = 0; i < entity_editor_pick_list.record_count; i++)
	{
		switch(entity_editor_pick_list.records[i].type)
		{
			case PICK_COLLIDER_PRIMITIVE:
				physics_TranslateCollisionShape(entity_editor_current_entity_def->collider_def, translation, entity_editor_pick_list.records[i].index0);	
			break;
		}
	}*/
	//editor_TranslateSelections(&entity_editor_pick_list, direction, amount);
}

void editor_EntityEditorRotateSelections(vec3_t axis, float amount)
{
	//editor_RotateSelections(&entity_editor_pick_list, axis, amount);
}

void editor_EntityEditorScaleSelections(vec3_t axis, float amount)
{
	int i;
	collision_shape_t *collision_shapes;
	vec3_t scale;
	
	scale.x = axis.x * amount;
	scale.y = axis.y * amount;
	scale.z = axis.z * amount;
	
	//if(!entity_editor_current_entity_def)
	{
		return;
	}
	
	/*if(!entity_editor_current_entity_def->collider_def)
	{
		return;
	}
	
	collision_shapes = entity_editor_current_entity_def->collider_def->collider_data.generic_collider_data.collision_shape;
	
	for(i = 0; i < entity_editor_pick_list.record_count; i++)
	{
		switch(entity_editor_pick_list.records[i].type)
		{
			case PICK_COLLIDER_PRIMITIVE:
				physics_ScaleCollisionShape(entity_editor_current_entity_def->collider_def, scale, entity_editor_pick_list.records[i].index0);
				//physics_TranslateCollisionShape(entity_editor_current_entity_def->collider_def, translation, entity_editor_pick_list.records[i].index0);	
			break;
		}
	}*/
	//editor_ScaleSelections(&entity_editor_pick_list, axis, amount);
}







