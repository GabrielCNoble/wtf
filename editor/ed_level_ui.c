#include "ed_level_ui.h"
#include "..\ed_level.h"
#include "..\..\common\gui.h"
#include "..\ed_common.h"
#include "..\ed_ui.h"
#include "..\..\common\l_main.h"
#include "..\..\common\player.h"
#include "..\..\common\portal.h"
#include "..\..\common\navigation.h"
#include "..\..\common\material.h"
#include "..\common\r_main.h"
#include "..\common\r_gl.h"
#include "..\common\r_shader.h"
#include "..\common\input.h"
#include "..\brush.h"
#include "..\bsp_cmp.h"
#include "..\pvs.h"

#include "vector.h"

#include "..\..\common\gmath\vector.h"
#include "..\..\common\gmath\matrix.h"

#include <stdio.h>
#include <string.h>

/* from ed_ui.c */
extern widget_bar_t *menu_dropdown_bar;

int ed_level_editor_menu_window_open = 0;
vec2_t ed_level_editor_menu_window_pos;
 
int ed_level_editor_add_to_world_menu_open = 0;
vec2_t ed_level_editor_add_to_world_menu_pos;

int ed_level_editor_light_options_menu_open = 0;
vec2_t ed_level_editor_light_options_menu_pos;

int ed_level_editor_brush_options_menu_open = 0;
int ed_level_editor_brush_uv_menu_open = 0;
int ed_level_editor_brush_face_option_open = 0;
vec2_t ed_level_editor_brush_options_menu_pos;
vec2_t ed_level_editor_brush_uv_menu_pos;
vec2_t ed_level_editor_brush_uv_window_size = {500.0, 500.0};
struct framebuffer_t ed_level_editor_brush_uv_menu_framebuffer;

int ed_level_editor_delete_selections_menu_open = 0;
vec2_t ed_level_editor_delete_selections_menu_pos;


/* from ed_level.c */
extern vec3_t level_editor_3d_cursor_position;
extern int level_editor_need_to_copy_data;
extern pick_list_t level_editor_pick_list;


/* from r_main.c */
extern int r_window_width;
extern int r_window_height;
extern int r_imediate_color_shader;

/* from ed_level.c */
extern pick_list_t level_editor_pick_list;
extern pick_list_t level_editor_brush_face_pick_list;
extern pick_list_t level_editor_brush_face_uv_pick_list;
extern int level_editor_editing_mode;

/* from editor.c */
extern int ed_pick_shader;

/* from material.c */
extern int mat_material_count;
extern material_t *mat_materials;
extern char **mat_material_names;



/*
=====================================================
=====================================================
=====================================================
*/

void editor_LevelEditorWaypointOptionsMenuCallback(widget_t *widget)
{
	option_t *option;
	int i;
	int waypoint_a;
	int waypoint_b;
	struct waypoint_t *a;
	struct waypoint_t *b;
	struct waypoint_t **route;
	int waypoint_count;
	
	
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		
		if(level_editor_pick_list.record_count >= 2)
		{
			waypoint_a = -1;
			waypoint_b = -1;
			
			if(!strcmp(option->widget.name, "link waypoints"))
			{
				for(i = 0; i < level_editor_pick_list.record_count; i++)
				{
					if(level_editor_pick_list.records[i].type == PICK_WAYPOINT)
					{
						if(waypoint_a < 0)
						{
							waypoint_a = level_editor_pick_list.records[i].index0;
						}
						else
						{
							waypoint_b = level_editor_pick_list.records[i].index0;
						}
					}
					
					if(waypoint_a >= 0 && waypoint_b >= 0)
					{
						navigation_LinkWaypoints(waypoint_a, waypoint_b);	
						waypoint_a = waypoint_b;
						waypoint_b = -1;
					}
				}
			}
			else if(!strcmp(option->widget.name, "unlink waypoints"))
			{
				for(i = 0; i < level_editor_pick_list.record_count; i++)
				{
					if(level_editor_pick_list.records[i].type == PICK_WAYPOINT)
					{
						if(waypoint_a < 0)
						{
							waypoint_a = level_editor_pick_list.records[i].index0;
						}
						else
						{
							waypoint_b = level_editor_pick_list.records[i].index0;
						}
					}
					
					if(waypoint_a >= 0 && waypoint_b >= 0)
					{
						navigation_UnlinkWaypoints(waypoint_a, waypoint_b);	
						waypoint_a = waypoint_b;
						waypoint_b = -1;
					}
				}
			}
			else if(!strcmp(option->widget.name, "route"))
			{
				if(level_editor_pick_list.records[0].type == PICK_WAYPOINT)
				{
					waypoint_a = level_editor_pick_list.records[0].index0;
				}
				
				if(level_editor_pick_list.records[1].type == PICK_WAYPOINT)
				{
					waypoint_b = level_editor_pick_list.records[1].index0;
				}
				
				if(waypoint_a >= 0 && waypoint_b >= 0)
				{
					
					a = navigation_GetWaypointPointer(waypoint_a);
					b = navigation_GetWaypointPointer(waypoint_b);
					
					route = navigation_FindPath(&waypoint_count, a->position, b->position);
					
					if(route)
					{
						for(i = 0; i < waypoint_count; i++)
						{
							printf("[%f %f %f]\n", route[i]->position.x, route[i]->position.y, route[i]->position.z);
						}
					}
				}
			}
			
		}	
	}
}
 
/*
=====================================================
=====================================================
=====================================================
*/ 

void editor_LevelEditorUIInit()
{
	ed_level_editor_brush_uv_menu_framebuffer = renderer_CreateFramebuffer(ed_level_editor_brush_uv_window_size.x, ed_level_editor_brush_uv_window_size.y);
	renderer_AddAttachment(&ed_level_editor_brush_uv_menu_framebuffer, GL_COLOR_ATTACHMENT0, GL_RGBA32F);
	renderer_AddAttachment(&ed_level_editor_brush_uv_menu_framebuffer, GL_DEPTH_ATTACHMENT, 0);
}

void editor_LevelEditorUIFinish()
{
	renderer_DestroyFramebuffer(&ed_level_editor_brush_uv_menu_framebuffer);
}

void editor_LevelEditorUISetup()
{
//	gui_SetVisible((widget_t *)level_editor_world_dropdown);
}

void editor_LevelEditorUIShutdown()
{
//	gui_SetInvisible((widget_t *)level_editor_world_dropdown);
}

void editor_LevelEditorUpdateUI()
{
	editor_LevelEditorMenuWindow();
	editor_LevelEditorWorldMenu();
	editor_LevelEditorAddToWorldMenu();
	editor_LevelEditorDeleteSelectionsMenu();
}


/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorCloseAllMenus()
{
	ed_level_editor_add_to_world_menu_open = 0;
	ed_level_editor_delete_selections_menu_open = 0;
}

void editor_LevelEditorMenuWindow()
{
	ed_level_editor_menu_window_open = 0;
	ed_level_editor_light_options_menu_open = 0;
	ed_level_editor_brush_options_menu_open = 0;
	
	pick_record_t *pick;
	
	if(level_editor_pick_list.record_count)
	{
		pick = &level_editor_pick_list.records[level_editor_pick_list.record_count - 1];
		
		switch(pick->type)
		{
			case PICK_LIGHT:
				ed_level_editor_light_options_menu_open = 1;
			break;
			
			case PICK_BRUSH:
				ed_level_editor_brush_options_menu_open = 1;
			break;
		}
		
		ed_level_editor_menu_window_open = 1;
	}
	
	if(level_editor_brush_face_pick_list.record_count && level_editor_editing_mode == EDITING_MODE_BRUSH)
	{
		ed_level_editor_brush_face_option_open = 1;
	}
	else
	{
		ed_level_editor_brush_face_option_open = 0;
		ed_level_editor_brush_uv_menu_open = 0;
	}
	
	
	if(ed_level_editor_menu_window_open)
	{
	//	gui_ImGuiPushID("LevelEditorMenuWindow");
		gui_ImGuiSetNextWindowSize(vec2(350.0, r_window_height - 20.0), 0);
		gui_ImGuiSetNextWindowPos(vec2(r_window_width - 350.0, 20.0), 0, vec2(0.0, 0.0));
		gui_ImGuiBegin("Menu window", NULL, ImGuiWindowFlags_NoResize);
		
		editor_LevelEditorBrushOptionsMenu();
		editor_LevelEditorLightOptionsMenu();
		
		
		gui_ImGuiEnd();
		
		//gui_ImGuiPopID();
	}
}

void editor_LevelEditorEntityDefsMenu()
{
	
}

void editor_LevelEditorLightOptionsMenu()
{
	light_ptr_t light_ptr;
	pick_record_t *pick;
	
	int r;
	int g;
	int b;
	int radius;
	int energy;
	
	if(ed_level_editor_light_options_menu_open)
	{		
		pick = &level_editor_pick_list.records[level_editor_pick_list.record_count - 1];
		
		switch(pick->type)
		{
			case PICK_LIGHT:
				light_ptr = light_GetLightPointerIndex(pick->index0);
				
				if(light_ptr.params)
				{
					r = light_ptr.params->r;
					g = light_ptr.params->g;
					b = light_ptr.params->b;
					 
					radius = light_ptr.params->radius;
					energy = light_ptr.params->energy;
					 
					gui_ImGuiSetNextWindowPos(vec2(0.0, 0.0), ImGuiCond_Once, vec2(0.0, 0.0));
					gui_ImGuiBeginChild("Light options", vec2(0.0, 0.0), 0, ImGuiWindowFlags_AlwaysAutoResize);
					gui_ImGuiSliderInt("Red", &r, 0, 255, "%d");
					gui_ImGuiSliderInt("Green", &g, 0, 255, "%d");
					gui_ImGuiSliderInt("Blue", &b, 0, 255, "%d");
					gui_ImGuiSliderInt("Radius", &radius, 0, 0xffff, "%d");
					gui_ImGuiSliderInt("Energy", &energy, 0, 0xffff, "%d");
					gui_ImGuiEndChild();
					
					light_ptr.params->r = r;
					light_ptr.params->g = g;
					light_ptr.params->b = b;
					
					light_ptr.params->radius = radius;
					light_ptr.params->energy = energy;
				}
				
			break;
		}
	}
}
 
void editor_LevelEditorBrushOptionsMenu()
{
	pick_record_t *pick;
	brush_t *brush;
	int brush_face_index;
	bsp_polygon_t *brush_face;
	material_t *brush_face_material;
	
	char *brush_face_material_name;
	int i;
	
	if(ed_level_editor_brush_options_menu_open)
	{
		//pick = editor_LevelEditorGetLastSelection();
		pick = &level_editor_pick_list.records[level_editor_pick_list.record_count - 1];
		brush = (brush_t *)pick->pointer;
		
		gui_ImGuiBeginChild("Brush options", vec2(0.0, 0.0), 0, ImGuiWindowFlags_AlwaysAutoResize);
		gui_ImGuiText("Vertices: %d", brush->clipped_polygons_vert_count);
		gui_ImGuiText("Faces: %d", brush->clipped_polygon_count);
		
		if(ed_level_editor_brush_face_option_open)
		{
			pick = &level_editor_brush_face_pick_list.records[level_editor_brush_face_pick_list.record_count - 1];
			brush_face_index = pick->index0;
			
			brush_face = &brush->base_polygons[brush_face_index];
			
			brush_face_material = material_GetMaterialPointerIndex(brush_face->material_index);
			brush_face_material_name = material_GetMaterialName(brush_face->material_index);
			
			gui_ImGuiText("Face material: ");
			gui_ImGuiSameLine(0.0, -1.0);
			
			gui_ImGuiPushItemWidth(120.0);
			
			if(gui_ImGuiBeginCombo(" ", brush_face_material_name, 0))
			{				
				for(i = -1; i < mat_material_count; i++)
				{
					if(gui_ImGuiSelectable(mat_material_names[i], 0, vec2(120.0, 16.0)))
					{
						brush_SetFaceMaterial(brush, brush_face_index, i);
					}
				}
				
				gui_ImGuiEndCombo();
			}
			
			gui_ImGuiPopItemWidth(120.0);
			
			if(gui_ImGuiButton("UV window", vec2(70.0, 30.0)))
			{
				ed_level_editor_brush_uv_menu_open ^= 1;
			}	 
			
			editor_LevelEditorBrushUVMenu();		
		}		
		
		gui_ImGuiEndChild();
	}
}

void editor_LevelEditorBrushUVMenu()
{
	float uv_mouse_x;
	float uv_mouse_y;
	
	vec2_t window_size;
	vec2_t cursor_pos;
	vec2_t mouse_pos;
	vec2_t window_pos;
	vec2_t drag_delta;
	
	pick_record_t uv_pick;
	
	brush_t *brush;
	bsp_polygon_t *face;
	int face_index;
	
	static float zoom_out = 2.0;
	
	static float view_offset_x = 0.0;
	static float view_offset_y = 0.0;
	
	mat4_t projection_matrix;
	mat4_t view_matrix;
	
	static int ed_level_editor_scroll_uv_window = 0;
	static int ed_level_editor_edit_uvs = 0;
	static int ed_level_editor_edit_uvs_direction = 3;
	int ed_level_editor_mouse_over = 0;
	
	int i;
	int j;
	int k;
	int value;
	
	int total_verts;
	
	int color[4];
	
	vec3_t v;
	
	if(ed_level_editor_brush_uv_menu_open)
	{
		gui_ImGuiSetNextWindowSize(vec2(0.0, 0.0), 0);
		gui_ImGuiBegin("UV window", NULL, ImGuiWindowFlags_AlwaysAutoResize);
		
		//window_size = gui_ImGuiGetContentRegionAvail();
		window_size = ed_level_editor_brush_uv_window_size;
		window_pos = gui_ImGuiGetWindowPos();
		mouse_pos = input_GetMousePosition();
		cursor_pos = gui_ImGuiGetCursorPos();
		
		window_pos.x += cursor_pos.x;
		window_pos.y += cursor_pos.y;
			
		uv_mouse_x = (((mouse_pos.x - window_pos.x) / window_size.x) - 0.5) * 2.0;
		uv_mouse_y = -((((r_window_height - mouse_pos.y) - window_pos.y) / window_size.y) - 0.5) * 2.0;
		
		gui_ImGuiInvisibleButton("UV button", window_size);
			
		CreateOrthographicMatrix(&projection_matrix, -zoom_out, zoom_out, zoom_out, -zoom_out, 10.0, -10.0, NULL);
		
		view_matrix = mat4_t_id();
		
		if(gui_ImGuiIsItemHovered(0))
		{
			ed_level_editor_mouse_over = 1;
			
			if(gui_ImGuiIsMouseClicked(2, 0))
			{
				ed_level_editor_scroll_uv_window = 1;
			}
			
			if(input_GetMouseButton(MOUSE_BUTTON_WHEEL) & MOUSE_WHEEL_DOWN)
			{
				zoom_out += 0.5; 
			}
			else if(input_GetMouseButton(MOUSE_BUTTON_WHEEL) & MOUSE_WHEEL_UP)
			{
				if(zoom_out > 0.5)
				{
					zoom_out -= 0.5;
				}	
			}
		}
		
		if(ed_level_editor_scroll_uv_window)
		{
			if(gui_ImGuiIsMouseDown(2))
			{
				drag_delta = input_GetMouseDelta();
				view_offset_x -= drag_delta.x * zoom_out * 2.0;
				view_offset_y += drag_delta.y * zoom_out * 2.0;
			}
			else
			{
				ed_level_editor_scroll_uv_window = 0;
			}
		}
		
		view_matrix.floats[3][0] = -view_offset_x;
		view_matrix.floats[3][1] = -view_offset_y;
		
		renderer_EnableImediateDrawing();
		renderer_SetProjectionMatrix(&projection_matrix);
		renderer_SetViewMatrix(&view_matrix);
		renderer_SetModelMatrix(NULL);
		renderer_PushFramebuffer(&ed_level_editor_brush_uv_menu_framebuffer);
		
		
		if(ed_level_editor_mouse_over)
		{
			if(gui_ImGuiIsMouseClicked(0, 0))
			{
				/*==============================================================================*/
				renderer_SetShader(ed_pick_shader);
				glClearColor(0.0, 0.0, 0.0, 0.0);
				glDisable(GL_DEPTH_TEST);
				glPointSize(12.0);
				
				value = PICK_UV_VERTEX;
				renderer_SetNamedUniform1f("pick_type", *(float *)&value);
				renderer_Color3f(0.0, 1.0, 0.0);
				
				total_verts = 0;
				
				for(j = 0; j < level_editor_brush_face_pick_list.record_count; j++)
				{
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					
					brush = (brush_t *)level_editor_brush_face_pick_list.records[j].pointer;
					face_index = level_editor_brush_face_pick_list.records[j].index0;
					face = &brush->base_polygons[face_index];
					
					for(i = 0; i < face->vert_count; i++)
					{
						value = i + 1;
						renderer_SetNamedUniform1f("pick_index", *(float *)&value);
						renderer_Begin(GL_POINTS);
						renderer_Vertex3f((face->vertices[i].tex_coord.x - 0.5) * 2.0, (face->vertices[i].tex_coord.y - 0.5) * 2.0, 0.0);
						renderer_End();
					}
					
					renderer_SampleFramebuffer(uv_mouse_x, -uv_mouse_y, color);
						
					if(color[1])
					{
						if(!(input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED))
						{
							editor_ClearSelection(&level_editor_brush_face_uv_pick_list);
						}
						
						uv_pick.type = PICK_UV_VERTEX;
						uv_pick.pointer = face;
						uv_pick.index0 = total_verts * 2.0 + color[1];
						uv_pick.index1 = color[1] - 1;
						
						editor_AddSelection(&uv_pick, &level_editor_brush_face_uv_pick_list);
						
						ed_level_editor_edit_uvs = 1;
						
						total_verts += face->vert_count;
					}
				}
				glEnable(GL_DEPTH_TEST);
				
				/*==============================================================================*/
			}
		}
		
		if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_JUST_PRESSED)
		{
			editor_ClearSelection(&level_editor_brush_face_uv_pick_list);
			ed_level_editor_edit_uvs = 0;
			ed_level_editor_edit_uvs_direction = 3;
		}
		
		if(ed_level_editor_edit_uvs)
		{
			if(input_GetKeyStatus(SDL_SCANCODE_X) & KEY_JUST_PRESSED)
			{
				if(ed_level_editor_edit_uvs_direction != 1)
				{
					ed_level_editor_edit_uvs_direction = 1;
				}
				else
				{
					ed_level_editor_edit_uvs_direction = 3;
				}
			}
			
			if(input_GetKeyStatus(SDL_SCANCODE_Y) & KEY_JUST_PRESSED)
			{
				if(ed_level_editor_edit_uvs_direction != 2)
				{
					ed_level_editor_edit_uvs_direction = 2;
				}
				else
				{
					ed_level_editor_edit_uvs_direction = 3;
				}
			}
			
		}
		
		
		if(level_editor_brush_face_uv_pick_list.record_count && (input_GetMouseButton(MOUSE_BUTTON_LEFT) & MOUSE_LEFT_BUTTON_CLICKED))
		{
			drag_delta = input_GetMouseDelta();
			
			for(j = 0; j < level_editor_brush_face_pick_list.record_count; j++)
			{
				brush = (brush_t *)level_editor_brush_face_pick_list.records[j].pointer;
				brush->bm_flags |= BRUSH_MOVED | BRUSH_CLIP_POLYGONS;
			}
			
		//	printf("%d\n", level_editor_brush_face_uv_pick_list.record_count);
				
			for(i = 0; i < level_editor_brush_face_uv_pick_list.record_count; i++)
			{
				uv_pick = level_editor_brush_face_uv_pick_list.records[i];
				face = (bsp_polygon_t *)uv_pick.pointer;
				
				if(ed_level_editor_edit_uvs_direction & 1)
				{
					face->vertices[uv_pick.index1].tex_coord.x += drag_delta.x * zoom_out * 2.0;
				}
				
				if(ed_level_editor_edit_uvs_direction & 2)
				{
					face->vertices[uv_pick.index1].tex_coord.y -= drag_delta.y * zoom_out * 2.0;
				}
			}
		}
		
		
		
		
		renderer_SetShader(r_imediate_color_shader);
		
		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		
		
		glPointSize(12.0);
		renderer_Begin(GL_POINTS);
		renderer_Color3f(1.0, 1.0, 1.0);
		for(j = 0; j < level_editor_brush_face_uv_pick_list.record_count; j++)
		{
			uv_pick = level_editor_brush_face_uv_pick_list.records[j];
			face = (bsp_polygon_t *)uv_pick.pointer;
			renderer_Vertex3f((face->vertices[uv_pick.index1].tex_coord.x - 0.5) * 2.0, (face->vertices[uv_pick.index1].tex_coord.y - 0.5) * 2.0, 0.0);
		}
		renderer_End();
		
		for(j = 0; j < level_editor_brush_face_pick_list.record_count; j++)
		{
			brush = (brush_t *)level_editor_brush_face_pick_list.records[j].pointer;
			face_index = level_editor_brush_face_pick_list.records[j].index0;
			face = &brush->base_polygons[face_index];
			
			renderer_Begin(GL_LINE_LOOP);
			renderer_Color3f(1.0f, 0.8f, 0.0f);
			for(i = 0; i < face->vert_count; i++)
			{
				renderer_Vertex3f((face->vertices[i].tex_coord.x - 0.5) * 2.0, (face->vertices[i].tex_coord.y - 0.5) * 2.0, 0.0);
			}
			renderer_End();
			
			glPointSize(8.0);
			renderer_Begin(GL_POINTS);
			renderer_Color3f(0.0f, 1.0f, 0.0f);
			for(i = 0; i < face->vert_count; i++)
			{
				renderer_Vertex3f((face->vertices[i].tex_coord.x - 0.5) * 2.0, (face->vertices[i].tex_coord.y - 0.5) * 2.0, 0.0);
			}
			renderer_End();
		}
				
		
		
		
		
		renderer_PopFramebuffer();
		renderer_DisableImediateDrawing();
		
		
		
		gui_ImGuiSetCursorPos(cursor_pos);
		gui_ImGuiImage(ed_level_editor_brush_uv_menu_framebuffer.color_attachments[0].handle, window_size, vec2(0.0, 0.0), vec2(1.0, 1.0), vec4(1.0, 1.0, 1.0, 1.0), vec4(1.0, 1.0, 1.0, 1.0));
		
		gui_ImGuiSetCursorPos(cursor_pos);
		
		if(ed_level_editor_edit_uvs_direction & 1)
		{
			gui_ImGuiText("X: free");
		}
		else
		{
			gui_ImGuiText("X: locked");
		}
		
		gui_ImGuiSameLine(0.0, -1.0);
		if(ed_level_editor_edit_uvs_direction & 2)
		{
			gui_ImGuiText("Y: free");
		}
		else
		{
			gui_ImGuiText("Y: locked");
		}
		
		gui_ImGuiEnd();
	}
	else
	{
		view_offset_x = 0.0;
		view_offset_y = 0.0;
		
		ed_level_editor_edit_uvs_direction = 3;
		
		editor_ClearSelection(&level_editor_brush_face_uv_pick_list);
	}
}

void editor_LevelEditorWorldMenu()
{
	//gui_ImGuiPushID("LevelEditorWorldMenu");
	
	if(gui_ImGuiBeginMainMenuBar())
	{
		if(gui_ImGuiBeginMenu("World"))
		{
			if(gui_ImGuiMenuItem("Compile bsp", NULL, NULL, 1))
			{
				bsp_CompileBsp(0);
				level_editor_need_to_copy_data = 1;
			}
			if(gui_ImGuiMenuItem("Clear bsp", NULL, NULL, 1))
			{
				//world_Clear();
				level_editor_need_to_copy_data = 1;
			}
			gui_ImGuiEndMenu();
		}
		
		gui_ImGuiEndMainMenuBar();
	}
	
	//gui_ImGuiPopID();
}

void editor_LevelEditorAddToWorldMenu()
{
	int keep_open = 1;
	pick_record_t record;
	mat3_t orientation = mat3_t_id();
	
	if(ed_level_editor_add_to_world_menu_open)
	{
		//gui_ImGuiPushID("LevelEditorAddToWorldMenu");
		
		gui_ImGuiSetNextWindowPos(vec2(ed_level_editor_add_to_world_menu_pos.x, ed_level_editor_add_to_world_menu_pos.y), 0, vec2(0.0, 0.0));
			
		if(gui_ImGuiBeginPopup("Add to world menu", 0))
		{
			if(gui_ImGuiMenuItem("Add light", NULL, NULL, 1))
			{
				record.index0 = light_CreateLight("light", &orientation, level_editor_3d_cursor_position, vec3_t_c(1.0, 1.0, 1.0), 25.0, 20.0, 0);
				record.type = PICK_LIGHT;
				editor_ClearSelection(&level_editor_pick_list);
				editor_LevelEditorAddSelection(&record);
				level_editor_need_to_copy_data = 1;
				keep_open = 0;
			}
			if(gui_ImGuiBeginMenu("Brush"))
			{
				if(gui_ImGuiMenuItem("Cube", NULL, NULL, 1))
				{
					record.pointer = brush_CreateBrush(level_editor_3d_cursor_position, &orientation, vec3_t_c(1.0, 1.0, 1.0), BRUSH_CUBE, 0);
					record.type = PICK_BRUSH;
					editor_ClearSelection(&level_editor_pick_list);
					editor_LevelEditorAddSelection(&record);
					level_editor_need_to_copy_data = 1;
					keep_open = 0;
				}
				
				gui_ImGuiEndMenu();
			}
			if(gui_ImGuiMenuItem("Waypoint", NULL, NULL, 1))
			{
				level_editor_need_to_copy_data = 1;
				record.index0 = navigation_CreateWaypoint(level_editor_3d_cursor_position);
				record.type = PICK_WAYPOINT;
				editor_ClearSelection(&level_editor_pick_list);
				editor_LevelEditorAddSelection(&record);
				keep_open = 0;
			}
			
			ed_level_editor_add_to_world_menu_open = keep_open;
			gui_ImGuiEndPopup();
			
			//if(!keep_open)
			//{
			//	editor_ClearSelection(&level_editor_pick_list);
			//}
			
		}
		
		//gui_ImGuiPopID();
	}
}

void editor_LevelEditorDeleteSelectionsMenu()
{
	if(ed_level_editor_delete_selections_menu_open)
	{
	//	gui_ImGuiPushID("LevelEditorDeleteSelectionsMenu");
		
		gui_ImGuiSetNextWindowPos(vec2(ed_level_editor_delete_selections_menu_pos.x, ed_level_editor_delete_selections_menu_pos.y), 0, vec2(0.0, 0.0));
		
		if(gui_ImGuiBeginPopup("Delete selections menu", 0))
		{
			if(gui_ImGuiMenuItem("Delete?", NULL, NULL, 1))
			{
				ed_level_editor_delete_selections_menu_open = 0;
				editor_LevelEditorDestroySelections();
			}
			gui_ImGuiEndPopup();
		}
		
	//	gui_ImGuiPopID();
	}
}

void editor_LevelEditorOpenAddToWorldMenu(int x, int y)
{		
	editor_LevelEditorCloseAllMenus();
		
	ed_level_editor_add_to_world_menu_open = 1;
//	gui_ImGuiPushID("LevelEditorAddToWorldMenu");
	gui_ImGuiOpenPopup("Add to world menu");
//	gui_ImGuiPopID();
		
	ed_level_editor_add_to_world_menu_pos.x = x;
	ed_level_editor_add_to_world_menu_pos.y = y;
}

void editor_LevelEditorCloseAddToWorldMenu()
{
	//if(level_editor_add_to_world_menu)
	{
		//gui_SetInvisible((widget_t *)level_editor_add_to_world_menu);
	}
}

void editor_LevelEditorOpenDeleteSelectionsMenu(int x, int y)
{
	editor_LevelEditorCloseAllMenus();
		
	gui_ImGuiOpenPopup("Delete selections menu");
		
	ed_level_editor_delete_selections_menu_open = 1;
	ed_level_editor_delete_selections_menu_pos.x = x;
	ed_level_editor_delete_selections_menu_pos.y = y;
}

void editor_LevelEditorCloseDeleteSelectionsMenu()
{
	//if(level_editor_delete_selections_menu)
	{
		//gui_SetInvisible((widget_t *)level_editor_delete_selections_menu);
	}
} 

void editor_LevelEditorOpenWaypointOptionMenu(int x, int y)
{
	/*if(level_editor_waypoints_options_menu)
	{
		gui_SetVisible((widget_t *)level_editor_waypoints_options_menu);
		
		level_editor_waypoints_options_menu->widget.x = x;
		level_editor_waypoints_options_menu->widget.y = y;
	}*/
}

void editor_LevelEditorCloseWaypointOptionMenu()
{
	/*if(level_editor_waypoints_options_menu)
	{
		gui_SetInvisible((widget_t *)level_editor_waypoints_options_menu);
	}*/
}










