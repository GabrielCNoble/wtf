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
extern int handle_3d_mode;
extern int handle_3d_position_mode;
extern vec3_t handle_3d_position;
extern int selection_count;
extern pick_record_t *selections;
extern int editor_state;


extern int b_draw_brushes;

void editor_Input(float delta_time)
{
	
}

void editor_ProcessMouse(float delta_time)
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
	
	static float prev_dx;
	static float prev_dy;
	
	float amount;
	float z;
	
	if(editor_state == EDITOR_EDITING)
	{
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
				if(handle_3d_mode == HANDLE_3D_SCALE || handle_3d_mode == HANDLE_3D_TRANSLATION)
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
			
			if(handle_3d_mode == HANDLE_3D_SCALE || handle_3d_mode == HANDLE_3D_TRANSLATION)
			{
				
				if(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED)
				{
					if(handle_3d_mode == HANDLE_3D_SCALE)
					{
						grab_screen_offset_x = normalized_mouse_x - p.x;
						grab_screen_offset_y = normalized_mouse_y - p.y;
						
						//prev_dx = screen_dx;
						//prev_dy = screen_dy;
					}
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
				
				//printf("%f\n", amount);
				
				
				if(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED)
				{
					prev_dx = screen_dx;
					prev_dy = screen_dy;
				}
			}
			 
			
				
			switch(handle_3d_mode)
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
	
	
}

void editor_ProcessKeyboard(float delta_time)
{
	
	if(editor_state == EDITOR_EDITING)
	{
		if(input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED)
		{
			/*if(input_GetKeyStatus(SDL_SCANCODE_C) & KEY_JUST_PRESSED)
			{
				bsp_CompileBsp(0);
				//indirect_BuildVolumes();
			}
			else if(input_GetKeyStatus(SDL_SCANCODE_S) & KEY_JUST_PRESSED)
			{
				//renderer_Fullscreen(1);
				//renderer_SetWindowSize(1920, 1080);
			}*/
	
			if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_JUST_PRESSED)
			{
				editor_OpenAddToWorldMenu(r_window_width * normalized_mouse_x * 0.5, r_window_height * normalized_mouse_y * 0.5);
			}
			else if(input_GetKeyStatus(SDL_SCANCODE_D) & KEY_JUST_PRESSED)
			{
				editor_CopySelections();
			}
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_G) & KEY_JUST_PRESSED)
		{
			//handle_3d_mode = HANDLE_3D_TRANSLATION;
			editor_Set3dHandleMode(HANDLE_3D_TRANSLATION);
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_R) & KEY_JUST_PRESSED)
		{
			//handle_3d_mode = HANDLE_3D_ROTATION;
			editor_Set3dHandleMode(HANDLE_3D_ROTATION);
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_S) & KEY_JUST_PRESSED)
		{
			editor_Set3dHandleMode(HANDLE_3D_SCALE);
			//handle_3d_mode = HANDLE_3D_SCALE;
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_JUST_PRESSED)
		{
			editor_ClearSelection();
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_P) & KEY_JUST_PRESSED)
		{
			editor_StartPIE();
		}
		else if(input_GetKeyStatus(SDL_SCANCODE_DELETE) & KEY_JUST_PRESSED)
		{
			editor_OpenDeleteSelectionMenu(r_window_width * normalized_mouse_x * 0.5, r_window_height * normalized_mouse_y * 0.5);
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










