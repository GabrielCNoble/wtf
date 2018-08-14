#include <stdio.h>


#include "ed_ui.h"
#include "ed_proj.h"
#include "engine.h"
#include "bsp_cmp.h"
#include "editor.h"
#include "r_main.h"
#include "log.h"
#include "material.h"
#include "texture.h"
#include "..\..\common\script\script.h"
#include "..\..\common\gui.h"
#include "..\..\common\c_memory.h"

#include "GL\glew.h"


//#include "ed_ui_brush.h"
//#include "ed_ui_material.h"
//#include "ed_ui_texture.h"
#include "ed_ui_explorer.h"




/* from material.c */
extern int material_count;
extern material_t *materials;
extern char **material_names;

/* from l_main.c */
extern light_params_t *light_params;
extern light_position_t *light_positions;

/* from texture.c */
extern int texture_count;
extern texture_t *textures;
extern texture_info_t *texture_info;

/* from brush.c */
extern brush_t *brushes;


/* from r_main.c */
extern int r_max_batch_size;
extern int r_window_width;
extern int r_window_height;
extern int r_z_prepass;
extern int r_draw_shadow_maps;
extern int r_bloom;
extern int r_tonemap;
extern int r_deferred;
extern int r_flat;
extern int r_debug;
extern int r_debug_verbose;
extern int r_debug_draw_portal_outlines;
extern int r_debug_draw_views;
extern int handle_3d_mode;
extern int forward_pass_shader;


extern editor_t *ed_editors;



/*
************************************************************************
************************************************************************
************************************************************************
*/




dropdown_t *snap_value_dropdown = NULL;
option_list_t *snap_values_list = NULL;






//dropdown_t *wow;


/* from ed_proj.c */
extern char ed_full_project_name[];


#include "ed_globals.h"

extern char *ed_handle_3d_mode_str;
//extern light_ptr_t selected_light;
extern int selected_type;
extern light_params_t *ed_selected_light_params;
extern light_position_t *ed_selected_light_position;
extern brush_t *ed_selected_brush;
extern int ed_selected_brush_polygon_index;

extern float ed_editor_linear_snap_values[];
extern char *ed_editor_linear_snap_values_str[];
extern float ed_editor_angular_snap_values[];
extern char *ed_editor_angular_snap_values_str[];
extern float ed_editor_linear_snap_value;
extern int ed_editor_linear_snap_value_index;
extern float ed_editor_angular_snap_value;
extern int ed_editor_angular_snap_value_index;
extern char *ed_editor_snap_value_str;

//int ed_edit_material_index;

extern int b_draw_brushes;
extern int b_draw_leaves;
extern int b_draw_light_leaves;
extern int b_draw_world_polygons;
extern int b_draw_brush_polygons;
extern int b_draw_portals;
extern int b_draw_pvs_steps;
extern int b_draw_collision_polygons;
extern int r_draw_gui;

int add_light_unique_index;
int add_brush_unique_index;
int add_cube_brush_unique_index;
int add_cylinder_brush_unique_index;
int add_spawn_point_unique_index;


/* from pvs.c */
extern int b_step;
extern SDL_sem *step_semaphore;


/* from engine.c */
extern float fps;

float insane_float0 = 0.5;
float insane_float1 = 0.5;
float insane_float2 = 0.5;

/* from input.c */
extern int bm_mouse;

void editor_InitUI()
{
	editor_InitExplorerUI();
}

void editor_FinishUI()
{
	editor_FinishExplorerUI();
}

void editor_ProcessUI()
{
	editor_t *editor;
	editor_t *selected_editor;


	if(gui_ImGuiBeginMainMenuBar())
	{
		if(gui_ImGuiBeginMenu("File"))
		{
			if(gui_ImGuiMenuItem("New...", NULL, NULL, 1))
			{

			}
			if(gui_ImGuiMenuItem("Save...", NULL, NULL, 1))
			{
				editor_OpenExplorerWindow(NULL, EXPLORER_FILE_MODE_WRITE);
			}
			if(gui_ImGuiMenuItem("Open...", NULL, NULL, 1))
			{
				editor_OpenExplorerWindow(NULL, EXPLORER_FILE_MODE_READ);
			}
			if(gui_ImGuiMenuItem("Exit", NULL, NULL, 1))
			{
				engine_SetEngineState(ENGINE_QUIT);
			}

			gui_ImGuiEndMenu();
		}

		if(gui_ImGuiBeginMenu("Renderer"))
		{
			if(r_flat)
			{
				if(gui_ImGuiMenuItem("Disable fullbright", NULL, NULL, 1))
				{
					r_flat = 0;
				}
			}
			else
			{
				if(gui_ImGuiMenuItem("Enable fullbright", NULL, NULL, 1))
				{
					r_flat = 1;
				}
			}

			//if(gui_ImGuiBeginMenu("Renderer batch size"))
			//{
             //   gui_ImGuiBeginChild("Renderer batch size", vec2(520.0, 50.0), 0, 0);

             //   gui_ImGuiSliderInt("Batch size", &r_max_batch_size, 0, 1000, "%d");

                /*gui_ImGuiText("Current batch size: %d triangles\n", r_max_batch_size);

                if(gui_ImGuiButton("+", vec2(20.0, 20.0)))
				{
					r_max_batch_size++;
				}
				if(gui_ImGuiButton("-", vec2(20.0, 20.0)))
				{
					if(r_max_batch_size)
					{
						r_max_batch_size--;
					}
				}*/

             //   gui_ImGuiEndChild();
			//	gui_ImGuiEndMenu();
			//}




			gui_ImGuiEndMenu();
		}

		if(gui_ImGuiBeginMenu("Misc"))
		{
			if(gui_ImGuiMenuItem("Reload shaders", NULL, NULL, 1))
			{
				shader_HotReload();
			}
			if(gui_ImGuiMenuItem("Reload scripts", NULL, NULL, 1))
			{
				script_ReloadScripts();
			}
			if(gui_ImGuiMenuItem("Check memory", NULL, NULL, 1))
			{
				memory_CheckCorrupted();
			}
			if(gui_ImGuiMenuItem("Report memory", NULL, NULL, 1))
			{
				memory_Report();
			}
			if(gui_ImGuiMenuItem("Reload path cfg", NULL, NULL, 1))
			{
				path_ReadCfg();
			}

			if(r_debug)
			{
				if(gui_ImGuiMenuItem("Disable debug", NULL, NULL, 1))
				{
					renderer_Debug(0, r_debug_verbose);
				}
			}
			else
			{
				if(gui_ImGuiMenuItem("Enable debug", NULL, NULL, 1))
				{
					renderer_Debug(1, r_debug_verbose);
				}
			}


			if(r_debug_verbose)
			{
				if(gui_ImGuiMenuItem("Disable verbose debug", NULL, NULL, 1))
				{
					renderer_Debug(r_debug, 0);
				}
			}
			else
			{
				if(gui_ImGuiMenuItem("Enable verbose debug", NULL, NULL, 1))
				{
					renderer_Debug(r_debug, 1);
				}
			}

			gui_ImGuiEndMenu();
		}

		if(gui_ImGuiBeginMenu("Editors"))
		{
			editor = ed_editors;
			selected_editor = NULL;
			while(editor)
			{
				if(gui_ImGuiMenuItem(editor->name, NULL, NULL, 1) && !selected_editor)
				{
					selected_editor = editor;
				}

				editor = editor->next;
			}
			gui_ImGuiEndMenu();

			if(selected_editor)
			{
				editor_StartEditor(selected_editor->name);
			}
		}
		gui_ImGuiEndMainMenuBar();
	}


	editor_UpdateExplorerUI();
}

void editor_LockUI()
{
	//gui_SetIgnoreMouse((widget_t *)menu_bar);
}

void editor_UnlockUI()
{
	//gui_SetReceiveMouse((widget_t *)menu_bar);
}

void editor_HideUI()
{

}

void editor_ShowUI()
{

}

void editor_EnumerateEditors()
{
	/*editor_t *editor;
	editor = ed_editors;
	option_t *option;

	gui_RemoveAllOptions(editors_list);

	while(editor)
	{
		option = gui_AddOptionToList(editors_list, editor->name, editor->name);
		option->widget.data = editor->name;
		editor = editor->next;
	}*/
}













