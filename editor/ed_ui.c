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

//dropdown_t *wow;


/* from ed_proj.c */
extern char current_project_name[];

/* from editor.c */
extern vec3_t cursor_3d_position;
extern vec3_t handle_3d_position;
extern int bm_handle_3d_flags;
extern int handle_3d_position_mode;
extern int handle_3d_mode;

extern b_draw_brushes;
extern b_draw_leaves;
extern b_draw_light_leaves;

int add_light_unique_index;
int add_brush_unique_index;
int add_cube_brush_unique_index;
int add_cylinder_brush_unique_index;
int add_spawn_point_unique_index;



void file_dropdown_callback(widget_t *widget)
{
	option_t *option;
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		switch(option->index)
		{
			case 0:
				
			break;
			
			case 1:
				editor_SaveProject(current_project_name);
			break;	
			
			case 2:
				editor_OpenProject(current_project_name);
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
			
			case 1:
				editor_ExportBsp("test.bsp");
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
			
			/*
			record.type = PICK_SPAWN_POINT;
			record.index0 = index;
			editor_ClearSelection();
			editor_AddSelection(&record);*/
			
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


void editor_InitUI()
{
	
	option_list_t *o;
	
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
	gui_AddOption(world_dropdown, "visualize leaves", "visualize leaves");
	gui_AddOption(world_dropdown, "visualize light leaves", "visualize light leaves");
	
	gui_AddWidgetToBar((widget_t *) world_dropdown, menu_dropdown_bar);
	
	misc_dropdown = gui_CreateDropdown("misc", "misc", 0, 0, MISC_DROPDOWN_WIDTH, 0, misc_dropdown_callback);
	gui_AddOption(misc_dropdown, "hot reload shaders", "hot reload shaders");
	gui_AddOption(misc_dropdown, "enable z prepass", "enable z prepass");
	gui_AddOption(misc_dropdown, "enable shadow mapping", "enable shadow mapping");
	gui_AddOption(misc_dropdown, "flush log", "flush log");
	
	gui_AddWidgetToBar((widget_t *)misc_dropdown, menu_dropdown_bar);
	
	
	
	add_to_world_menu = gui_CreateOptionList("add to world", 0, 0, 100, 0, add_to_world_menu_callback);
	gui_AddOptionToList(add_to_world_menu, "add light", "add light");
	gui_AddOptionToList(add_to_world_menu, "add brush", "add brush");
	gui_AddOptionToList(add_to_world_menu, "add spawn point", "add spawn point");
	
	add_light_unique_index = gui_GetOptionUniqueIndex(add_to_world_menu, 0);
	add_spawn_point_unique_index = gui_GetOptionUniqueIndex(add_to_world_menu, 2);
	//add_brush_unique_index = gui_GetOptionUniqueIndex(add_to_world_menu, 1);
	
	brush_types_option_list = gui_NestleOptionList(add_to_world_menu, 1, "brush type options");
	gui_AddOptionToList(brush_types_option_list, "cube brush", "cube brush");
	gui_AddOptionToList(brush_types_option_list, "cylinder brush", "cylinder brush");
	
	
	add_cube_brush_unique_index = gui_GetOptionUniqueIndex(brush_types_option_list, 0);
	add_cylinder_brush_unique_index = gui_GetOptionUniqueIndex(brush_types_option_list, 1);
		
	gui_SetInvisible((widget_t *)add_to_world_menu);
	
	
	delete_menu = gui_CreateOptionList("delete", 0, 0, 100, 0, delete_selection_menu_callback);
	gui_AddOptionToList(delete_menu, "delete", "delete?");
	
	gui_SetInvisible((widget_t *)delete_menu);
	


	//gui_AddTextField(NULL, "text field", 0, 0, 150, 0, NULL);
	
	
	//renderer_RegisterCallback(editor_UIWindowResizeCallback, RENDERER_RESOLUTION_CHANGE_CALLBACK);
	
	
	
}

void editor_FinishUI()
{
	
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






