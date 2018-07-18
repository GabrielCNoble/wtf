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
#include "..\..\common\memory.h"

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

widget_t *menu_bar = NULL;
widget_bar_t *menu_dropdown_bar = NULL;
dropdown_t *file_dropdown = NULL;
dropdown_t *editor_dropdown = NULL;
option_list_t *editors_list = NULL;

dropdown_t *world_dropdown = NULL;

option_list_t *option_list = NULL;

option_list_t *ed_add_to_world_menu = NULL;
option_list_t *brush_types_option_list = NULL;
option_list_t *entity_defs_option_list = NULL;
dropdown_t *misc_dropdown = NULL;


option_list_t *ed_delete_menu = NULL;


widget_t *save_project_window = NULL;
text_field_t *save_project_text_field = NULL;
button_t *confirm_save_project_button = NULL;
button_t *cancel_save_project_button = NULL;



widget_t *open_project_window = NULL;
text_field_t *open_project_text_field = NULL;
button_t *confirm_open_project_button = NULL;
button_t *cancel_open_project_button = NULL;


text_field_t *fps_display = NULL;
text_field_t *handle_3d_mode_display = NULL;

int ed_light_properties_window_open = 0;
widget_t *light_properties_window = NULL;
slider_t *selected_light_r = NULL;
slider_t *selected_light_g = NULL;
slider_t *selected_light_b = NULL;
slider_t *selected_light_radius = NULL;
slider_t *selected_light_energy = NULL;


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

//static char *no_tex = "None";


/*
************************************************************************
************************************************************************
************************************************************************
*/

/*void confirm_save_project_button_callback(widget_t *widget)
{
	editor_SetProjectName(save_project_text_field->text);
	editor_SaveProject();
	gui_SetInvisible(save_project_window);
}*/

/*void cancel_save_project_button_callback(widget_t *widget)
{
	gui_SetInvisible(save_project_window);
}*/

//void confirm_open_project_button_callback(widget_t *widget)
//{
	//if(editor_OpenProject(open_project_text_field->text))
	/*{
		editor_SetProjectName(open_project_text_field->text);
	}*/
	
//	editor_OpenProject(open_project_text_field->text);
//	gui_SetInvisible(open_project_window);
//}

//void cancel_open_project_button_callback(widget_t *widget)
//{
//	gui_SetInvisible(open_project_window);
//}

void editor_FileDropdownCallback(widget_t *widget)
{
	option_t *option;
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		switch(option->index)
		{
			case 0:
//				editor_NewProject();
			break;
			
			case 1:
				//editor_OpenSaveProjectWindow();
				editor_OpenExplorerWindow(NULL, EXPLORER_FILE_MODE_WRITE);
			break;	
			
			case 2:
				//editor_OpenOpenProjectWindow();
				editor_OpenExplorerWindow(NULL, EXPLORER_FILE_MODE_READ);
			break;
			
			case 3:
				engine_SetEngineState(ENGINE_QUIT);
			break;
		}
	}
	
}

void editor_EditorsDropdownCallback(widget_t *widget)
{
	option_t *option;
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		editor_StartEditor((char *)option->widget.data);
	}
}


void editor_WorldDropdownCallback(widget_t *widget)
{
	option_t *option;
	
	#if 0
	
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		
		switch(option->index)
		{
			case 0:
				brush_SetAllInvisible();
				bsp_CompileBsp(0);
			break;
			
			case 1:
				bsp_Stop();
			break;
			
			case 2:
				
				
				bsp_DeleteBsp();
				brush_SetAllVisible();
			break;
			
			case 3:
//				editor_ExportBsp(ed_full_project_name);
			break;
			
			case 4:
				b_draw_portals ^= 1;
			break;
			
			case 5:
				b_draw_brushes ^= 1;
			break;
			
			case 6:
				b_draw_leaves ^= 1;
			break;
			
			case 7:
				b_draw_light_leaves ^= 1;
			break;
			
			case 8:
				b_draw_world_polygons ^= 1;
			break;
			
			case 9:
				b_draw_brush_polygons ^= 1;
			break;
			
			case 10:
				b_draw_collision_polygons ^= 1;
			break;
			/*case 10:
				b_draw_pvs_steps ^= 1;
			break;*/
			
			
		}
	}
	#endif
}

void editor_MiscDropdownProcessCallback(widget_t *widget)
{
	option_t *option;
	
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		
		if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
		{
			if(!strcmp(option->widget.name, "reload shaders"))
			{
				shader_HotReload();
			}
			else if(!strcmp(option->widget.name, "reload scripts"))
			{
				script_ReloadScripts();
			}
			else if(!strcmp(option->widget.name, "verbose debug"))
			{
				if(r_debug_verbose)
				{
					renderer_VerboseDebugOutput(0);
				}
				else
				{
					renderer_VerboseDebugOutput(1);
				}
			}
			else if(!strcmp(option->widget.name, "draw portal outlines"))
			{
				r_debug_draw_portal_outlines ^= 1;
				
				if(r_debug_draw_portal_outlines)
				{
					gui_SetOptionText(option, "disable draw portal outlines");
				}
				else
				{
					gui_SetOptionText(option, "enable draw portal outlines");
				}
				
			}
			else if(!strcmp(option->widget.name, "draw views"))
			{
				r_debug_draw_views ^= 1;
				
				if(r_debug_draw_views)
				{
					gui_SetOptionText(option, "disable draw views");
				}
				else
				{
					gui_SetOptionText(option, "enable draw views");
				}
				
			}
			else if(!strcmp(option->widget.name, "draw flat"))
			{
				r_flat ^= 1;
				
				if(r_flat)
				{
					gui_SetOptionText(option, "disable fullbright");
				}
				else
				{
					gui_SetOptionText(option, "enable fullbright");
				}
			}
			
		}
		
		
	}
}

void editor_MiscDropdownCallback(widget_t *widget)
{
	option_t *option;
	
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *) widget;
		
		if(!strcmp(option->widget.name, "reload shaders"))
		{
			shader_HotReload();
		}
		
		//else if(!strcmp(option->widget.name, ""))
		
		/*switch(option->index)
		{
			case 0:
				shader_HotReload();
			break;
			
			case 1:
				r_z_prepass ^= 1;
			break;
			
			case 2:
				r_draw_shadow_maps ^= 1;
			break;
			
			case 3:
				log_FlushLog();
			break;
			
			case 4:
				b_step ^= 1;
				
				if(!SDL_SemValue(step_semaphore))
					SDL_SemPost(step_semaphore);
			break;
			
			case 5:
				r_bloom ^= 1;
			break;
			
			case 6:
				r_tonemap ^= 1;
			break;
			
			case 7:
				r_deferred ^= 1;
			break;
			
			case 8:
				r_draw_gui ^= 1;
			break;
		}*/
	}
}




/*
************************************************************************
************************************************************************
************************************************************************
*/

//int awesome_int = 0;

void editor_InitUI()
{
	 
	//int i;
	//widget_t *w;
	//editor_t *editor;
	//option_t *option;
	//option_list_t *o;
	
	//menu_bar = gui_CreateWidget("menu_bar", 0, r_window_height * 0.5 - MENU_BAR_HEIGHT * 0.5, r_window_width, MENU_BAR_HEIGHT);	
/*	menu_bar = gui_AddWidget(NULL, "menu_bar", 0, r_window_height * 0.5 - MENU_BAR_HEIGHT * 0.5, r_window_width, MENU_BAR_HEIGHT);
	
	menu_dropdown_bar = gui_AddWidgetBar(menu_bar, "menu widget bar", 0, 0, r_window_width, MENU_BAR_HEIGHT, WIDGET_BAR_FIXED_SIZE);
	 
	file_dropdown = gui_CreateDropdown("file", "file", 0, 0, FILE_DROPDOWN_WIDTH, 0, editor_FileDropdownCallback);
	gui_AddOption(file_dropdown, "new", "new...");
	gui_AddOption(file_dropdown, "save", "save");
	gui_AddOption(file_dropdown, "open", "open");
	gui_AddOption(file_dropdown, "exit", "exit");
	
	gui_AddWidgetToBar((widget_t *) file_dropdown, menu_dropdown_bar);
	
	 
	
	misc_dropdown = gui_CreateDropdown("misc", "misc", 0, 0, MISC_DROPDOWN_WIDTH, 0, NULL);
	
	misc_dropdown->widget.process_callback = editor_MiscDropdownProcessCallback;
	
	gui_AddOption(misc_dropdown, "reload shaders", "reload shaders");
	gui_AddOption(misc_dropdown, "reload scripts", "reload scripts");
	gui_AddOption(misc_dropdown, "draw portal outlines", "draw portal outlines");
	gui_AddOption(misc_dropdown, "draw views", "draw views");
	gui_AddOption(misc_dropdown, "draw flat", "enable fullbright");
	gui_AddOption(misc_dropdown, "verbose debug", "verbose debug");
	gui_AddWidgetToBar((widget_t *) misc_dropdown, menu_dropdown_bar);
	
	
	
	
	editor_dropdown = gui_CreateDropdown("Editors", "Editors", 0, 0, EDITORS_DROPDOWN_WIDTH, 0, editor_EditorsDropdownCallback);
	editors_list = gui_AddOptionList((widget_t *)editor_dropdown, "editors", 0, 0, EDITORS_DROPDOWN_WIDTH, 0, 8, editor_EditorsDropdownCallback);*/

	
	//gui_AddWidgetToBar((widget_t *)editor_dropdown, menu_dropdown_bar);
	editor_InitExplorerUI();	
}

void editor_FinishUI()
{
	editor_FinishExplorerUI();
}

void editor_ProcessUI()
{
	/*menu_bar->y = r_window_height * 0.5 - MENU_BAR_HEIGHT * 0.5;
	menu_bar->w = r_window_width;
		
	menu_dropdown_bar->widget.w = r_window_width * 0.5;
	menu_dropdown_bar->bm_flags |= WIDGET_BAR_ADJUST_WIDGETS;*/
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
	gui_SetIgnoreMouse((widget_t *)menu_bar);
}

void editor_UnlockUI()
{
	gui_SetReceiveMouse((widget_t *)menu_bar);
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













