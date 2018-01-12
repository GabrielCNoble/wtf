#include <stdio.h>


#include "ed_ui.h"
#include "ed_proj.h"
#include "gui.h"
#include "engine.h"
#include "bsp_cmp.h"
#include "editor.h"


#define MENU_BAR_HEIGHT 20
#define FILE_DROPDOWN_WIDTH 120
#define WORLD_DROPDOWN_WIDTH 160
#define WOW_DROPDOWN_WIDTH 120


/* from r_main.c */
extern int r_window_width;
extern int r_window_height;

extern int handle_3d_mode;


widget_t *menu_bar;
widget_bar_t *menu_dropdown_bar;
dropdown_t *file_dropdown;
dropdown_t *world_dropdown;
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


void editor_InitUI()
{
	menu_bar = gui_CreateWidget("menu_bar", 0, r_window_height * 0.5 - MENU_BAR_HEIGHT * 0.5, r_window_width, MENU_BAR_HEIGHT);	
	
	menu_dropdown_bar = gui_AddWidgetBar(menu_bar, "menu widget bar", 0, 0, r_window_width, MENU_BAR_HEIGHT, WIDGET_BAR_FIXED_SIZE);
	
	//file_dropdown = gui_AddDropdown(menu_bar, "file", "file", -r_window_width * 0.5 + FILE_DROPDOWN_WIDTH * 0.5, 0, FILE_DROPDOWN_WIDTH, 0, file_dropdown_callback);
	file_dropdown = gui_CreateDropdown("file", "file", 0, 0, FILE_DROPDOWN_WIDTH, 0, file_dropdown_callback);
	gui_AddOption(file_dropdown, "new_file", "new...");
	gui_AddOption(file_dropdown, "save", "save");
	gui_AddOption(file_dropdown, "load", "load");
	gui_AddOption(file_dropdown, "exit", "exit");
	
	gui_AddWidgetToBar((widget_t *) file_dropdown, menu_dropdown_bar);
	
	//world_dropdown = gui_AddDropdown(menu_bar, "world", "world", -r_window_width * 0.5 + FILE_DROPDOWN_WIDTH + WORLD_DROPDOWN_WIDTH * 0.5, 0, WORLD_DROPDOWN_WIDTH, 0, world_dropdown_callback);
	world_dropdown = gui_CreateDropdown("world", "world", 0, 0, WORLD_DROPDOWN_WIDTH, 0, world_dropdown_callback);
	gui_AddOption(world_dropdown, "compile bsp", "compile bsp");
	gui_AddOption(world_dropdown, "export bsp", "export bsp");
	gui_AddOption(world_dropdown, "visualize portals", "visualize portals");
	
	gui_AddWidgetToBar((widget_t *) world_dropdown, menu_dropdown_bar);
	
	
	/*wow = gui_AddDropDown(menu_bar, "wow", "wow", -r_window_width * 0.5 + FILE_DROPDOWN_WIDTH + WORLD_DROPDOWN_WIDTH + WOW_DROPDOWN_WIDTH * 0.5, 0, WOW_DROPDOWN_WIDTH, 0, wow_dropdown_callback);
	gui_AddOption(wow, "translation", "translation");
	gui_AddOption(wow, "rotation", "rotation");
	gui_AddOption(wow, "scale", "scale");*/
	
	
	
}

void editor_FinishUI()
{
	
}






