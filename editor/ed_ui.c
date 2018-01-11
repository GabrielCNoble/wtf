#include <stdio.h>


#include "ed_ui.h"
#include "ed_proj.h"
#include "gui.h"
#include "engine.h"
#include "bsp_cmp.h"


#define MENU_BAR_HEIGHT 20
#define FILE_DROPDOWN_WIDTH 120
#define WORLD_DROPDOWN_WIDTH 160


/* from r_main.c */
extern int r_window_width;
extern int r_window_height;


widget_t *menu_bar;
dropdown_t *file_dropdown;

dropdown_t *world_dropdown;

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


void editor_InitUI()
{
	menu_bar = gui_CreateWidget("menu_bar", 0, r_window_height * 0.5 - MENU_BAR_HEIGHT * 0.5, r_window_width, MENU_BAR_HEIGHT);
	file_dropdown = gui_AddDropDown(menu_bar, "file", "file", -r_window_width * 0.5 + FILE_DROPDOWN_WIDTH * 0.5, 0, FILE_DROPDOWN_WIDTH, 0, file_dropdown_callback);
	gui_AddOption(file_dropdown, "new_file", "new...");
	gui_AddOption(file_dropdown, "save", "save");
	gui_AddOption(file_dropdown, "load", "load");
	gui_AddOption(file_dropdown, "exit", "exit");
	
	world_dropdown = gui_AddDropDown(menu_bar, "world", "world", -r_window_width * 0.5 + FILE_DROPDOWN_WIDTH + WORLD_DROPDOWN_WIDTH * 0.5, 0, WORLD_DROPDOWN_WIDTH, 0, world_dropdown_callback);
	gui_AddOption(world_dropdown, "compile bsp", "compile bsp");
	gui_AddOption(world_dropdown, "export bsp", "export bsp");
	gui_AddOption(world_dropdown, "visualize portals", "visualize portals");
	
}

void editor_FinishUI()
{
	
}
