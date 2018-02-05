#include <stdio.h>


#include "ed_ui.h"
#include "ed_proj.h"
#include "gui.h"
#include "engine.h"
#include "bsp_cmp.h"
#include "editor.h"
#include "r_main.h"
#include "log.h"



#define MENU_BAR_HEIGHT 20
#define FILE_DROPDOWN_WIDTH 120
#define WORLD_DROPDOWN_WIDTH 220
#define WOW_DROPDOWN_WIDTH 120
#define MISC_DROPDOWN_WIDTH 200

#define FPS_DISPLAY_WIDTH 70
#define HANDLE_3D_MODE_DISPLAY_WIDTH 120


#define LIGHT_PROPERTIES_WINDOW_WIDTH	100
#define LIGHT_PROPERTIES_WINDOW_HEIGHT	100


/* from r_main.c */
extern int r_window_width;
extern int r_window_height;
extern int r_z_prepass;
extern int r_draw_shadow_maps;
extern int handle_3d_mode;


widget_t *menu_bar = NULL;
widget_bar_t *menu_dropdown_bar = NULL;
dropdown_t *file_dropdown = NULL;
dropdown_t *world_dropdown = NULL;

option_list_t *option_list;

option_list_t *add_to_world_menu;
option_list_t *brush_types_option_list;
dropdown_t *misc_dropdown = NULL;


option_list_t *delete_menu;


widget_t *save_project_window;
text_field_t *save_project_text_field;
button_t *confirm_save_project_button;
button_t *cancel_save_project_button;



widget_t *open_project_window;
text_field_t *open_project_text_field;
button_t *confirm_open_project_button;
button_t *cancel_open_project_button;


text_field_t *fps_display;
text_field_t *handle_3d_mode_display;



widget_t *light_properties_window;
slider_t *selected_light_r;
slider_t *selected_light_g;
slider_t *selected_light_b;
slider_t *selected_light_radius;
slider_t *selected_light_energy;


//dropdown_t *wow;


/* from ed_proj.c */
extern char current_project_name[];

/* from editor.c */
extern vec3_t cursor_3d_position;
extern vec3_t handle_3d_position;
extern int bm_handle_3d_flags;
extern int handle_3d_position_mode;
extern int handle_3d_mode;
extern char *handle_3d_mode_str;
extern light_ptr_t selected_light;
extern int selected_type;

extern int b_draw_brushes;
extern int b_draw_leaves;
extern int b_draw_light_leaves;
extern int b_draw_world_polygons;
extern int b_draw_brush_polygons;
extern int b_draw_portals;
extern int b_draw_pvs_steps;

int add_light_unique_index;
int add_brush_unique_index;
int add_cube_brush_unique_index;
int add_cylinder_brush_unique_index;
int add_spawn_point_unique_index;


/* from pvs.c */
extern int b_step;


/* from engine.c */
extern float fps;

float insane_float0 = 0.5;
float insane_float1 = 0.5;
float insane_float2 = 0.5;

void confirm_save_project_button_callback(widget_t *widget)
{
	editor_SetProjectName(save_project_text_field->text);
	editor_SaveProject();
	gui_SetInvisible(save_project_window);
}

void cancel_save_project_button_callback(widget_t *widget)
{
	gui_SetInvisible(save_project_window);
}

void confirm_open_project_button_callback(widget_t *widget)
{
	if(editor_OpenProject(open_project_text_field->text))
	{
		editor_SetProjectName(open_project_text_field->text);
	}
	
	gui_SetInvisible(open_project_window);
}

void cancel_open_project_button_callback(widget_t *widget)
{
	gui_SetInvisible(open_project_window);
}

void file_dropdown_callback(widget_t *widget)
{
	option_t *option;
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		switch(option->index)
		{
			case 0:
				editor_CloseProject();
				//printf("new project\n");
			break;
			
			case 1:
				editor_OpenSaveProjectWindow();
				//editor_SaveProject(current_project_name);
			break;	
			
			case 2:
				editor_OpenOpenProjectWindow();
				//editor_OpenProject(current_project_name);
			break;
			
			case 3:
				engine_SetEngineState(ENGINE_QUIT);
			break;
		}
	}
	
}


void world_dropdown_callback(widget_t *widget)
{
	option_t *option;
	
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		
		switch(option->index)
		{
			case 0:
				bsp_CompileBsp(0);
			break;
			
			case 2:
				b_draw_portals ^= 1;
			break;
			
			case 1:
				editor_ExportBsp(current_project_name);
			break;
			
			case 3:
				b_draw_brushes ^= 1;
			break;
			
			case 4:
				b_draw_leaves ^= 1;
			break;
			
			case 5:
				b_draw_light_leaves ^= 1;
			break;
			
			case 6:
				b_draw_world_polygons ^= 1;
			break;
			
			case 7:
				b_draw_brush_polygons ^= 1;
			break;
			
			case 8:
				b_draw_pvs_steps ^= 1;
			break;
			
			
		}
	}
}

void misc_dropdown_callback(widget_t *widget)
{
	option_t *option;
	
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *) widget;
		switch(option->index)
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
			break;
		}
	}
}


/*void wow_dropdown_callback(widget_t *widget)
{
	option_t *option;
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		
		switch(option->index)
		{
			case 0:
				handle_3d_mode = HANDLE_3D_TRANSLATION;
			break;
			
			case 1:
				handle_3d_mode = HANDLE_3D_ROTATION;
			break;
			
			case 2:
			
			break;
		}
	}
}*/

void add_to_world_menu_callback(widget_t *widget)
{
	option_t *option;
	mat3_t r = mat3_t_id();
	pick_record_t record;
	int light_index;
	int brush_index;
	int index;
	
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		
		if(option->unique_index == add_light_unique_index)
		{
			
			light_index = light_CreateLight("light", &r, cursor_3d_position, vec3(1.0, 1.0, 1.0), 25.0, 20.0, LIGHT_GENERATE_SHADOWS);
			
			record.type = PICK_LIGHT;
			record.index0 = light_index;
		
			editor_ClearSelection();
			editor_AddSelection(&record);
			
			handle_3d_position = cursor_3d_position;		
		}
		else if(option->unique_index == add_cube_brush_unique_index)
		{
			
			brush_index = brush_CreateBrush(cursor_3d_position, &r, vec3(1.0, 1.0, 1.0), BRUSH_CUBE);
			
			
			record.type = PICK_BRUSH;
			record.index0 = brush_index;
			
			editor_ClearSelection();
			editor_AddSelection(&record);
			
			handle_3d_position = cursor_3d_position;
		}
		else if(option->unique_index == add_cylinder_brush_unique_index)
		{
			brush_index = brush_CreateBrush(cursor_3d_position, &r, vec3(1.0, 1.0, 1.0), BRUSH_CYLINDER);
			
			
			record.type = PICK_BRUSH;
			record.index0 = brush_index;
			
			editor_ClearSelection();
			editor_AddSelection(&record);
			
			handle_3d_position = cursor_3d_position;
		}
		else if(option->unique_index == add_spawn_point_unique_index)
		{
			index = player_CreateSpawnPoint(cursor_3d_position, "spawn point");
		
	
			record.type = PICK_SPAWN_POINT;
			record.index0 = index;
			
			editor_ClearSelection();
			editor_AddSelection(&record);
			
			handle_3d_position = cursor_3d_position;
			
		}
		
		
		/*switch(option->index)
		{
			case 0:
				light_CreateLight("light", &r, vec3(0.0, 5.0, 0.0), vec3(1.0, 1.0, 1.0), 25.0, 20.0);
			break;
			
			case 1:
			
			break;
			
			case 2:
				
			break;
		}*/
	}
}


void delete_selection_menu_callback(widget_t *widget)
{
	option_t *option;
	
	switch(widget->type)
	{
		case WIDGET_OPTION:
			editor_DeleteSelection();
		break;
	}
}

char *insane_string = "INSANE!";

unsigned char insane_char = 0;
unsigned char *p_insane_char = &insane_char;

void editor_InitUI()
{
	
	option_list_t *o;
	
	menu_bar = gui_CreateWidget("menu_bar", 0, r_window_height * 0.5 - MENU_BAR_HEIGHT * 0.5, r_window_width, MENU_BAR_HEIGHT);	
	
	menu_dropdown_bar = gui_AddWidgetBar(menu_bar, "menu widget bar", 0, 0, r_window_width, MENU_BAR_HEIGHT, WIDGET_BAR_FIXED_SIZE);
	
	file_dropdown = gui_CreateDropdown("file", "file", 0, 0, FILE_DROPDOWN_WIDTH, 0, file_dropdown_callback);
	gui_AddOption(file_dropdown, "new", "new...");
	gui_AddOption(file_dropdown, "save", "save");
	gui_AddOption(file_dropdown, "load", "load");
	gui_AddOption(file_dropdown, "exit", "exit");
	
	gui_AddWidgetToBar((widget_t *) file_dropdown, menu_dropdown_bar);
	
	world_dropdown = gui_CreateDropdown("world", "world", 0, 0, WORLD_DROPDOWN_WIDTH, 0, world_dropdown_callback);
	gui_AddOption(world_dropdown, "compile bsp", "compile bsp");
	gui_AddOption(world_dropdown, "export bsp", "export bsp");
	gui_AddOption(world_dropdown, "visualize portals", "visualize portals");
	gui_AddOption(world_dropdown, "visualize brushes", "visualize brushes");
	gui_AddOption(world_dropdown, "visualize leaves", "visualize leaves");
	gui_AddOption(world_dropdown, "visualize light leaves", "visualize light leaves");
	gui_AddOption(world_dropdown, "visualize world polygons", "visualize world polygons");
	gui_AddOption(world_dropdown, "visualize brush polygons", "visualize brush polygons");
	gui_AddOption(world_dropdown, "visualize pvs steps", "visualize pvs steps");
	
	gui_AddWidgetToBar((widget_t *) world_dropdown, menu_dropdown_bar);
	
	misc_dropdown = gui_CreateDropdown("misc", "misc", 0, 0, MISC_DROPDOWN_WIDTH, 0, misc_dropdown_callback);
	gui_AddOption(misc_dropdown, "hot reload shaders", "hot reload shaders");
	gui_AddOption(misc_dropdown, "enable z prepass", "enable z prepass");
	gui_AddOption(misc_dropdown, "enable shadow mapping", "enable shadow mapping");
	gui_AddOption(misc_dropdown, "flush log", "flush log");
	gui_AddOption(misc_dropdown, "step pvs", "step pvs");
	
	gui_AddWidgetToBar((widget_t *)misc_dropdown, menu_dropdown_bar);
	
	
	
	/*dropdown_t *test_dropdown = gui_CreateDropdown("misc", "misc", 0, 0, MISC_DROPDOWN_WIDTH, 0, NULL);
	gui_AddOption(test_dropdown, "fuck", "fuck");
	gui_AddOption(test_dropdown, "piss", "piss");
	gui_AddOption(test_dropdown, "ass", "ass");
	gui_AddOption(test_dropdown, "shit", "shit");
	
	gui_AddWidgetToBar((widget_t *)test_dropdown, menu_dropdown_bar);*/
	
	//gui_var_t *test_var = gui_CreateVar("test var", GUI_VAR_STRING, &insane_string);
	//gui_TrackVar(test_var, (widget_t *)test_dropdown);
	
	
	
	
	
	add_to_world_menu = gui_CreateOptionList("add to world", 0, 0, 100, 0, add_to_world_menu_callback);
	gui_AddOptionToList(add_to_world_menu, "add light", "add light");
	gui_AddOptionToList(add_to_world_menu, "add brush", "add brush");
	gui_AddOptionToList(add_to_world_menu, "add spawn point", "add spawn point");
	
	add_light_unique_index = gui_GetOptionUniqueIndex(add_to_world_menu, 0);
	add_spawn_point_unique_index = gui_GetOptionUniqueIndex(add_to_world_menu, 2);
	
	brush_types_option_list = gui_NestleOptionList(add_to_world_menu, 1, "brush type options");
	gui_AddOptionToList(brush_types_option_list, "cube brush", "cube brush");
	gui_AddOptionToList(brush_types_option_list, "cylinder brush", "cylinder brush");
	
	
	add_cube_brush_unique_index = gui_GetOptionUniqueIndex(brush_types_option_list, 0);
	add_cylinder_brush_unique_index = gui_GetOptionUniqueIndex(brush_types_option_list, 1);
	gui_SetInvisible((widget_t *)add_to_world_menu);
	
	
	delete_menu = gui_CreateOptionList("delete", 0, 0, 100, 0, delete_selection_menu_callback);
	gui_AddOptionToList(delete_menu, "delete", "delete?");
	gui_SetInvisible((widget_t *)delete_menu);
	
	
	/*save_project_window = gui_CreateWidget("save project window", 0, 0, 400, 80);
	save_project_text_field = gui_AddTextField(save_project_window, "save project text field", 0, 30, 380, 0, NULL);
	confirm_save_project_button= gui_AddButton(save_project_window, "confirm save project button", 100, -40, 50, 50, 0, NULL);
	cancel_save_project_button= gui_AddButton(save_project_window, "cancel save project button", -100, -40, 50, 50, 0, NULL);*/

	save_project_window = gui_CreateWidget("save project window", 0, 0, 400, 80);
	save_project_text_field = gui_AddTextField(save_project_window, "save project text field", 0, 20, 380, 0, NULL);
	confirm_save_project_button = gui_AddButton(save_project_window, "confirm", 80, -15, 90, 40, 0, confirm_save_project_button_callback);
	cancel_save_project_button = gui_AddButton(save_project_window, "cancel", -80, -15, 90, 40, 0, cancel_save_project_button_callback);

	gui_SetInvisible((widget_t *)save_project_window);
	
	
	
	
	open_project_window = gui_CreateWidget("open project window", 0, 0, 400, 80);
	open_project_text_field = gui_AddTextField(open_project_window, "open project text field", 0, 20, 380, 0, NULL);
	confirm_open_project_button = gui_AddButton(open_project_window, "confirm", 80, -15, 90, 40, 0, confirm_open_project_button_callback);
	cancel_open_project_button = gui_AddButton(open_project_window, "cancel", -80, -15, 90, 40, 0, cancel_open_project_button_callback);
	gui_SetInvisible((widget_t *)open_project_window);

	
	fps_display = gui_AddTextField(NULL, "fps", r_window_width * 0.5 - FPS_DISPLAY_WIDTH * 0.5, 0, FPS_DISPLAY_WIDTH, 0 ,NULL);
	gui_TrackVar(gui_CreateVar("fps", GUI_VAR_FLOAT, &fps), (widget_t *)fps_display);
	
	handle_3d_mode_display = gui_AddTextField(NULL, "handle 3d mode", -r_window_width * 0.5 + HANDLE_3D_MODE_DISPLAY_WIDTH * 0.5, -r_window_height * 0.5 + OPTION_HEIGHT * 0.5, HANDLE_3D_MODE_DISPLAY_WIDTH, 0, NULL);
	gui_TrackVar(gui_CreateVar("handle 3d mode", GUI_VAR_STRING, &handle_3d_mode_str), (widget_t *)handle_3d_mode_display);



	
		
	gui_var_t *var0 = gui_CreateVar("selected light r", GUI_VAR_POINTER_TO_UNSIGNED_CHAR, &selected_light.r);
	gui_var_t *var1 = gui_CreateVar("selected light g", GUI_VAR_POINTER_TO_UNSIGNED_CHAR, &selected_light.g);
	gui_var_t *var2 = gui_CreateVar("selected light b", GUI_VAR_POINTER_TO_UNSIGNED_CHAR, &selected_light.b);
	
	light_properties_window = gui_CreateWidget("wow", r_window_width * 0.5 - LIGHT_PROPERTIES_WINDOW_WIDTH * 0.5, 0, LIGHT_PROPERTIES_WINDOW_WIDTH, LIGHT_PROPERTIES_WINDOW_HEIGHT);
 	selected_light_r = gui_AddSlider(light_properties_window, "light r", 0, 20, 90, 0, NULL, var0, gui_MakeUnsignedCharVar(0), gui_MakeUnsignedCharVar(255));
	selected_light_g = gui_AddSlider(light_properties_window, "light g", 0, 0, 90, 0, NULL, var1, gui_MakeUnsignedCharVar(0), gui_MakeUnsignedCharVar(255));
	selected_light_b = gui_AddSlider(light_properties_window, "light b", 0, -20, 90, 0, NULL, var2, gui_MakeUnsignedCharVar(0), gui_MakeUnsignedCharVar(255));


	//gui_AddTextField(NULL, "text field", 0, 0, 230, 0, NULL);
	
	
	//renderer_RegisterCallback(editor_UIWindowResizeCallback, RENDERER_RESOLUTION_CHANGE_CALLBACK);
	
	
	
}

void editor_FinishUI()
{
	
}

void editor_ProcessUI()
{
	if(selected_type == PICK_LIGHT)
	{
		editor_OpenLightPropertiesWindow();
	}
	else
	{
		editor_CloseLightPropertiesWindow();
	}
}

void editor_OpenAddToWorldMenu(int x, int y)
{
	if(add_to_world_menu)
	{
		gui_SetVisible((widget_t *)add_to_world_menu);
		add_to_world_menu->widget.x = x;
		add_to_world_menu->widget.y = y;
	}
}

void editor_CloseAddToWorldMenu()
{
	if(add_to_world_menu)
	{
		gui_SetInvisible((widget_t *)add_to_world_menu);
	}
}

void editor_OpenDeleteSelectionMenu(int x, int y)
{
	if(delete_menu)
	{
		gui_SetVisible((widget_t *)delete_menu);
		delete_menu->widget.x = x;
		delete_menu->widget.y = y;
	}
}

void editor_OpenSaveProjectWindow()
{
	gui_SetText((widget_t *)save_project_text_field, current_project_name);
	gui_SetVisible(save_project_window);
}

void editor_CloseSaveProjectWindow()
{
	gui_SetInvisible(save_project_window);
}

void editor_OpenOpenProjectWindow()
{
	gui_SetVisible(open_project_window);
}

void editor_OpenLightPropertiesWindow()
{
	if(light_properties_window)
		gui_SetVisible(light_properties_window);
}

editor_CloseLightPropertiesWindow()
{
	if(light_properties_window)
		gui_SetInvisible(light_properties_window);
}

void editor_UIWindowResizeCallback()
{
	gui_DestroyWidget(menu_bar);
	gui_UpdateGUIProjectionMatrix();
	
	menu_bar = gui_CreateWidget("menu_bar", 0, r_window_height * 0.5 - MENU_BAR_HEIGHT * 0.5, r_window_width, MENU_BAR_HEIGHT);	
	
	menu_dropdown_bar = gui_AddWidgetBar(menu_bar, "menu widget bar", 0, 0, r_window_width, MENU_BAR_HEIGHT, WIDGET_BAR_FIXED_SIZE);
	
	file_dropdown = gui_CreateDropdown("file", "file", 0, 0, FILE_DROPDOWN_WIDTH, 0, file_dropdown_callback);
	gui_AddOption(file_dropdown, "new_file", "new...");
	gui_AddOption(file_dropdown, "save", "save");
	gui_AddOption(file_dropdown, "load", "load");
	gui_AddOption(file_dropdown, "exit", "exit");
	
	gui_AddWidgetToBar((widget_t *) file_dropdown, menu_dropdown_bar);
	
	world_dropdown = gui_CreateDropdown("world", "world", 0, 0, WORLD_DROPDOWN_WIDTH, 0, world_dropdown_callback);
	gui_AddOption(world_dropdown, "compile bsp", "compile bsp");
	gui_AddOption(world_dropdown, "export bsp", "export bsp");
	gui_AddOption(world_dropdown, "visualize portals", "visualize portals");
	gui_AddOption(world_dropdown, "visualize brushes", "visualize brushes");
	
	gui_AddWidgetToBar((widget_t *) world_dropdown, menu_dropdown_bar);
	
}






