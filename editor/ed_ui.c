#include <stdio.h>


#include "ed_ui.h"
#include "ed_proj.h"
#include "gui.h"
#include "engine.h"
#include "bsp_cmp.h"
#include "editor.h"
#include "r_main.h"


#define MENU_BAR_HEIGHT 20
#define FILE_DROPDOWN_WIDTH 120
#define WORLD_DROPDOWN_WIDTH 160
#define WOW_DROPDOWN_WIDTH 120


/* from r_main.c */
extern int r_window_width;
extern int r_window_height;

extern int handle_3d_mode;


widget_t *menu_bar = NULL;
widget_bar_t *menu_dropdown_bar = NULL;
dropdown_t *file_dropdown = NULL;
dropdown_t *world_dropdown = NULL;

option_list_t *option_list;

option_list_t *add_to_world_menu;
//dropdown_t *wow;


/* from ed_proj.c */
extern char current_project_name[];

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
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		
		switch(option->index)
		{
			case 0:
				light_CreateLight("light", &r, vec3(0.0, 5.0, 0.0), vec3(1.0, 1.0, 1.0), 25.0, 20.0);
			break;
			
			case 1:
			
			break;
			
			case 2:
				
			break;
		}
	}
}


void editor_InitUI()
{
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
	
	gui_AddWidgetToBar((widget_t *) world_dropdown, menu_dropdown_bar);
	
	
	add_to_world_menu = gui_CreateOptionList("add to world", 0, 0, 100, 0, add_to_world_menu_callback);
	gui_AddOptionToList(add_to_world_menu, "add light", "add light");
	
	gui_SetInvisible((widget_t *)add_to_world_menu);
	
	/*option_list = gui_CreateOptionList("option_list", 0, 0, 100, 0, NULL);
	gui_AddOptionToList(option_list, "option0", "option0");
	gui_AddOptionToList(option_list, "option1", "option1");
	gui_AddOptionToList(option_list, "option2", "option2");
	
	gui_NestleOption(option_list, 1, "sub_option0", "sub_option0");
	gui_NestleOption(option_list, 1, "sub_option1", "sub_option1");
	gui_NestleOption(option_list, 1, "sub_option2", "sub_option2");
	gui_NestleOption(option_list, 1, "sub_option3", "sub_option3");
	
	gui_NestleOption(option_list, 2, "sub_option0", "sub_option0");
	gui_NestleOption(option_list, 2, "sub_option1", "sub_option1");
	gui_NestleOption(option_list, 2, "sub_option2", "sub_option2");
	gui_NestleOption(option_list, 2, "sub_option3", "sub_option3");
	gui_NestleOption(option_list, 2, "sub_option4", "sub_option4");
	gui_NestleOption(option_list, 2, "sub_option5", "sub_option5");
	gui_NestleOption(option_list, 2, "sub_option6", "sub_option6");
	gui_NestleOption(option_list, 2, "sub_option7", "sub_option7");*/
	/*wow = gui_AddDropDown(menu_bar, "wow", "wow", -r_window_width * 0.5 + FILE_DROPDOWN_WIDTH + WORLD_DROPDOWN_WIDTH + WOW_DROPDOWN_WIDTH * 0.5, 0, WOW_DROPDOWN_WIDTH, 0, wow_dropdown_callback);
	gui_AddOption(wow, "translation", "translation");
	gui_AddOption(wow, "rotation", "rotation");
	gui_AddOption(wow, "scale", "scale");*/
	
	
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
	
	gui_AddWidgetToBar((widget_t *) world_dropdown, menu_dropdown_bar);
	
}






