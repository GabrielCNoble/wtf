#include "ed_common.h"
#include "ed_in.h"
#include "editor.h"
#include "ed_ui.h"

#include "vector.h"
#include "matrix.h"

#include "input.h"
#include "camera.h"
#include "brush.h"
#include "l_main.h"
 
#include "player.h"
#include "pvs.h"

#include "r_main.h"

#include <stdio.h>

#include "ed_ui_material.h"
#include "ed_ui_texture.h"
#include "ed_ui_explorer.h"
#include "ed_ui_brush.h"
#include "ed_ui_entity.h"


extern int r_width;
extern int r_height;
extern int r_window_width;
extern int r_window_height;


/* from l_main.c */
extern int light_count;
extern light_position_t *light_positions;
extern light_params_t *light_params;


/* from input.c */
extern float normalized_mouse_x;
extern float normalized_mouse_y;
extern float last_mouse_x;
extern float last_mouse_y;
extern int bm_mouse;


/* from brush.c */
extern int brush_count;
extern brush_t *brushes;


/* from editor.c */
extern int bm_handle_3d_flags;
extern int ed_handle_3d_mode;
extern int handle_3d_position_mode;
extern vec3_t handle_3d_position;
extern int selection_count;
extern pick_record_t *selections;
extern int editor_state;
extern int ed_editing_mode;
extern int ed_handle_3d_tranform_mode;
extern int ed_selected_brush_polygon_index;
extern float ed_editor_linear_snap_value;
extern float ed_editor_angular_snap_value;


/* from player.c */
extern int spawn_point_count;
extern spawn_point_t *spawn_points;


extern int b_draw_brushes;

/* from pvs.c */
extern int b_step;
extern int b_calculating_pvs;
extern SDL_mutex *step_mutex;
extern SDL_sem *step_semaphore;
extern pvs_for_leaf_stack_t pvs_for_leaf_stack;

void editor_Input(float delta_time)
{
	
}

void editor_ProcessMouse(float delta_time)
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
	
	if(editor_state == EDITOR_EDITING)
	{
		if(bm_mouse & MOUSE_OVER_WIDGET)
			return;
		
		if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
		{
			if(!(bm_mouse & MOUSE_OVER_WIDGET))
			{
				editor_Check3dHandle();		
			
				if(!bm_handle_3d_flags)
				{
					editor_Position3dCursor();
				}
			}
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
				if(ed_handle_3d_mode == HANDLE_3D_SCALE || ed_handle_3d_mode == HANDLE_3D_TRANSLATION)
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
				
			
			
			
			//printf("%f %f\n", screen_dx, screen_dy);
			
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
			
			if(ed_handle_3d_mode == HANDLE_3D_SCALE || ed_handle_3d_mode == HANDLE_3D_TRANSLATION)
			{
				sp = p;	
				p.vec3 = direction;
				p.w = 0.0;
				
				mat4_t_vec4_t_mult(&active_camera->world_to_camera_matrix, &p);
				
				screen_x = p.x;
				screen_y = p.y;
				
				amount = sqrt(screen_x * screen_x + screen_y * screen_y);
					
				screen_x /= amount;
				screen_y /= amount;
				
				amount = (screen_dx * screen_x + screen_dy * screen_y) * z;
				
				
				if(ed_editor_linear_snap_value > 0.0)
				{
					d = amount / ed_editor_linear_snap_value;
			 		amount = ed_editor_linear_snap_value * (int)d;
				}
				
				
				if(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED)
				{
					if(ed_handle_3d_mode == HANDLE_3D_SCALE)
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
				
				if(ed_editor_angular_snap_value > 0.0)
				{
					d = amount / ed_editor_angular_snap_value;
					amount = ed_editor_angular_snap_value * (int)d;
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
			 
			if(ed_editing_mode == EDITING_MODE_OBJECT)
			{
				switch(ed_handle_3d_mode)
				{
					case HANDLE_3D_TRANSLATION:
						editor_TranslateSelections(direction, amount);		
					break;
					
					case HANDLE_3D_ROTATION:
						editor_RotateSelections(direction, amount);
					break;
					
					case HANDLE_3D_SCALE:
						editor_ScaleSelections(direction, amount);
					break;
				}
				
				editor_Position3dHandle();
			}
				
			
	
		}
			
		
			
		if(bm_mouse & MOUSE_RIGHT_BUTTON_JUST_CLICKED)
		{
			if(ed_editing_mode == EDITING_MODE_OBJECT)
			{
				editor_PickObject();	
			}
			else
			{
				if(ed_editing_mode == EDITING_MODE_UV)
				{
					editor_CloseBrushFaceUVWindow();
				}
				i = selections[selection_count - 1].index0;
				
				editor_PickOnBrush(&brushes[i]);
			}
			
		}
	}
	
	
}

void editor_ProcessKeyboard(float delta_time)
{
	
	if(editor_state == EDITOR_EDITING)
	{
		if(input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED)
		{
			if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_JUST_PRESSED)
			{
				if(ed_editing_mode == EDITING_MODE_OBJECT)
				{
					editor_OpenAddToWorldMenu(r_window_width * normalized_mouse_x * 0.5, r_window_height * normalized_mouse_y * 0.5);
				}
			}
			else if(input_GetKeyStatus(SDL_SCANCODE_M) & KEY_JUST_PRESSED)
			{
				//editor_OpenMaterialWindow();
				editor_ToggleMaterialWindow();
			}
			else if(input_GetKeyStatus(SDL_SCANCODE_T) & KEY_JUST_PRESSED)
			{
				editor_ToggleTextureWindow();
			}
			else if(input_GetKeyStatus(SDL_SCANCODE_V) & KEY_JUST_PRESSED)
			{
				editor_ToggleEntityDefViewerWindow();
			}
			else if(input_GetKeyStatus(SDL_SCANCODE_D) & KEY_JUST_PRESSED)
			{
				if(ed_editing_mode == EDITING_MODE_OBJECT)
				{
					editor_CopySelections();
				}
				
			}
			if(input_GetKeyStatus(SDL_SCANCODE_SPACE) & KEY_JUST_PRESSED)
			{
				renderer_ToggleFullscreen();
			}
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_G) & KEY_JUST_PRESSED)
		{
			editor_Set3dHandleMode(HANDLE_3D_TRANSLATION);
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_R) & KEY_JUST_PRESSED)
		{
			editor_Set3dHandleMode(HANDLE_3D_ROTATION);
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_S) & KEY_JUST_PRESSED)
		{
			editor_Set3dHandleMode(HANDLE_3D_SCALE);
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_JUST_PRESSED)
		{
			if(ed_editing_mode == EDITING_MODE_OBJECT)
			{
				editor_ClearSelection();
				editor_CloseLightPropertiesWindow();
				editor_CloseBrushPropertiesWindow();
			}
			
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_P) & KEY_JUST_PRESSED)
		{
			editor_StartPIE();
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_DELETE) & KEY_JUST_PRESSED)
		{
			if(ed_editing_mode == EDITING_MODE_OBJECT)
			{
				editor_OpenDeleteSelectionMenu(r_window_width * normalized_mouse_x * 0.5, r_window_height * normalized_mouse_y * 0.5);
			}
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_TAB) & KEY_JUST_PRESSED)
		{
			if(ed_editing_mode == EDITING_MODE_BRUSH || ed_editing_mode == EDITING_MODE_UV)
			{
				editor_SetEditingMode(EDITING_MODE_OBJECT);
				ed_selected_brush_polygon_index = -1;
			}
			else
			{
				if(selection_count)
				{
					if(selections[selection_count - 1].type == PICK_BRUSH)
					{
						editor_SetEditingMode(EDITING_MODE_BRUSH);
					}
				}
			}
		}
		
		if(input_GetKeyStatus(SDL_SCANCODE_SPACE) & KEY_JUST_PRESSED)
		{
			if(b_step && b_calculating_pvs)
			{
				//SDL_UnlockMutex(step_mutex);
				//printf("pvs stack pointer: %d\n", pvs_for_leaf_stack.recursive_stack_pointer);
				SDL_SemPost(step_semaphore);
			}
			
		}
	
		
	}
	else
	{
		if(input_GetKeyStatus(SDL_SCANCODE_ESCAPE) & KEY_JUST_PRESSED)
		{
			editor_StopPIE();
		}
	}
	
	
	
	
	
}










