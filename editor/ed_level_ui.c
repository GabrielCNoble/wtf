#include "ed_level_ui.h"
#include "..\ed_level.h"
#include "..\..\common\gui.h"
#include "..\ed_common.h"
#include "..\ed_ui.h"
#include "..\..\common\l_main.h"
#include "..\..\common\player.h"
#include "..\..\common\portal.h"
#include "..\..\common\navigation.h"
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

 
int ed_level_editor_add_to_world_menu_open = 0;
vec2_t ed_level_editor_add_to_world_menu_pos;

int ed_level_editor_delete_selections_menu_open = 0;
vec2_t ed_level_editor_delete_selections_menu_pos;


/* from ed_level.c */
extern vec3_t level_editor_3d_cursor_position;
extern int level_editor_need_to_copy_data;
extern pick_list_t level_editor_pick_list;



/*
=====================================================
=====================================================
=====================================================
*/

dropdown_t *level_editor_world_dropdown = NULL;
option_list_t *level_editor_add_to_world_menu = NULL;
option_list_t *level_editor_delete_selections_menu = NULL;

option_list_t *level_editor_waypoints_options_menu = NULL;

/*
=====================================================
=====================================================
=====================================================
*/

void editor_LevelEditorWorldDropdownCallback(widget_t *widget)
{
	option_t *option;
	
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		
		if(!strcmp(option->widget.name, "compile bsp"))
		{
			bsp_CompileBsp(1);
			level_editor_need_to_copy_data = 1;
		}
		else if(!strcmp(option->widget.name, "clear bsp"))
		{
			level_editor_need_to_copy_data = 1;
		}
	}
}

void editor_LevelEditorAddToWorldMenuCallback(widget_t *widget)
{
	option_t *option;
	pick_record_t record;
	mat3_t orientation = mat3_t_id();
	
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		
		editor_LevelEditorClearSelections();
		
		if(!strcmp(option->widget.name, "add light"))
		{
			record.index0 = light_CreateLight("light", &orientation, level_editor_3d_cursor_position, vec3_t_c(1.0, 1.0, 1.0), 25.0, 20.0, 0);
			record.type = PICK_LIGHT;
			editor_LevelEditorAddSelection(&record);
			level_editor_need_to_copy_data = 1;
		}
		else if(!strcmp(option->widget.name, "add brush cube"))
		{
			record.pointer = brush_CreateBrush(level_editor_3d_cursor_position, &orientation, vec3_t_c(1.0, 1.0, 1.0), BRUSH_CUBE, 0);
			record.type = PICK_BRUSH;
			editor_LevelEditorAddSelection(&record);
			level_editor_need_to_copy_data = 1;
		}
		else if(!strcmp(option->widget.name, "add brush cylinder"))
		{
			level_editor_need_to_copy_data = 1;
		}
		else if(!strcmp(option->widget.name, "add spawn point"))
		{
			level_editor_need_to_copy_data = 1;
		}
		else if(!strcmp(option->widget.name, "add portal"))
		{
			level_editor_need_to_copy_data = 1;
			
			record.index0 = portal_CreatePortal(level_editor_3d_cursor_position, (vec2_t){1.0, 1.0}, &orientation, 2);
			record.type = PICK_PORTAL;
			editor_LevelEditorAddSelection(&record);
		}
		else if(!strcmp(option->widget.name, "add waypoint"))
		{
			level_editor_need_to_copy_data = 1;
			
			//record.index0 = portal_CreatePortal(level_editor_3d_cursor_position, (vec2_t){1.0, 1.0}, &orientation, 2);
			record.index0 = navigation_CreateWaypoint(level_editor_3d_cursor_position);
			record.type = PICK_WAYPOINT;
			editor_LevelEditorAddSelection(&record);
		}
	}
}

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
	/*option_list_t *brush_option_list;
	
	level_editor_world_dropdown = gui_CreateDropdown("world", "World", 0, 0, WORLD_DROPDOWN_WIDTH, 0, editor_LevelEditorWorldDropdownCallback);
	gui_AddOption(level_editor_world_dropdown, "compile bsp", "Compile bsp");
	gui_AddOption(level_editor_world_dropdown, "clear bsp", "Clear bsp");
	
	gui_AddWidgetToBar((widget_t *)level_editor_world_dropdown, menu_dropdown_bar);
	gui_SetInvisible((widget_t *)level_editor_world_dropdown);
	
	level_editor_add_to_world_menu = gui_AddOptionList(NULL, "Add to world", 0, 0, 180, 0, 8, editor_LevelEditorAddToWorldMenuCallback);
	gui_AddOptionToList(level_editor_add_to_world_menu, "add light", "Add light");
	gui_AddOptionToList(level_editor_add_to_world_menu, "add brush", "Add brush");
	brush_option_list = gui_NestleOptionList(level_editor_add_to_world_menu, 1, "Add brush");
	gui_AddOptionToList(level_editor_add_to_world_menu, "add spawn point", "Add spawn point");
	gui_AddOptionToList(level_editor_add_to_world_menu, "add portal", "add portal");
	gui_AddOptionToList(level_editor_add_to_world_menu, "add waypoint", "add waypoint");
	
	gui_AddOptionToList(brush_option_list, "add brush cube", "Cube");
	gui_AddOptionToList(brush_option_list, "add brush cylinder", "Cylinder");
	
	level_editor_delete_selections_menu = gui_AddOptionList(NULL, "Delete", 0, 0, 100, 0, 2, NULL);
	gui_AddOptionToList(level_editor_delete_selections_menu, "Delete?", "Delete?");
	
	level_editor_waypoints_options_menu = gui_AddOptionList(NULL, "waypoint menu", 0, 0, 180, 0, 8, editor_LevelEditorWaypointOptionsMenuCallback);
	gui_AddOptionToList(level_editor_waypoints_options_menu, "link waypoints", "link waypoints");
	gui_AddOptionToList(level_editor_waypoints_options_menu, "unlink waypoints", "unlink waypoints");
	gui_AddOptionToList(level_editor_waypoints_options_menu, "route", "route");
	
	
	editor_LevelEditorCloseAddToWorldMenu();
	editor_LevelEditorCloseDeleteSelectionsMenu();
	editor_LevelEditorCloseWaypointOptionMenu();*/
}

void editor_LevelEditorUIFinish()
{
	
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
	editor_LevelEditorWorldMenu();
	editor_LevelEditorLightOptionsMenu();
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

void editor_LevelEditorLightOptionsMenu()
{
	light_ptr_t light_ptr;
	pick_record_t *pick;
	
	int r;
	int g;
	int b;
	int radius;
	int energy;
		
	if(level_editor_pick_list.record_count)
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
					gui_ImGuiBegin("Light options", NULL, ImGuiWindowFlags_AlwaysAutoResize);
					gui_ImGuiSliderInt("Red", &r, 0, 255, "%d");
					gui_ImGuiSliderInt("Green", &g, 0, 255, "%d");
					gui_ImGuiSliderInt("Blue", &b, 0, 255, "%d");
					gui_ImGuiSliderInt("Radius", &radius, 0, 0xffff, "%d");
					gui_ImGuiSliderInt("Energy", &energy, 0, 0xffff, "%d");
					
					gui_ImGuiEnd();
					
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

void editor_LevelEditorWorldMenu()
{
	if(gui_ImGuiBeginMainMenuBar())
	{
		if(gui_ImGuiBeginMenu("World"))
		{
			if(gui_ImGuiMenuItem("Compile bsp", NULL, NULL, 1))
			{
				bsp_CompileBsp(0);
			}
			if(gui_ImGuiMenuItem("Clear bsp", NULL, NULL, 1))
			{
				//world_Clear();
			}
			gui_ImGuiEndMenu();
		}
		
		gui_ImGuiEndMainMenuBar();
	}
	
}

void editor_LevelEditorAddToWorldMenu()
{
	int keep_open = 1;
	pick_record_t record;
	mat3_t orientation = mat3_t_id();
	
	if(ed_level_editor_add_to_world_menu_open)
	{
		gui_ImGuiSetNextWindowPos(vec2(ed_level_editor_add_to_world_menu_pos.x, ed_level_editor_add_to_world_menu_pos.y), 0, vec2(0.0, 0.0));
			
		if(gui_ImGuiBeginPopup("Add to world menu", 0))
		{
			if(gui_ImGuiMenuItem("Add light", NULL, NULL, 1))
			{
				record.index0 = light_CreateLight("light", &orientation, level_editor_3d_cursor_position, vec3_t_c(1.0, 1.0, 1.0), 25.0, 20.0, 0);
				record.type = PICK_LIGHT;
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
				editor_LevelEditorAddSelection(&record);
				keep_open = 0;
			}
			
			ed_level_editor_add_to_world_menu_open = keep_open;
			gui_ImGuiEndPopup();
		}
	}
}

void editor_LevelEditorDeleteSelectionsMenu()
{
	if(ed_level_editor_delete_selections_menu_open)
	{
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
	}
}

void editor_LevelEditorOpenAddToWorldMenu(int x, int y)
{		
	editor_LevelEditorCloseAllMenus();
		
	ed_level_editor_add_to_world_menu_open = 1;
	gui_ImGuiOpenPopup("Add to world menu");
		
	ed_level_editor_add_to_world_menu_pos.x = x;
	ed_level_editor_add_to_world_menu_pos.y = y;
}

void editor_LevelEditorCloseAddToWorldMenu()
{
	if(level_editor_add_to_world_menu)
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
	if(level_editor_delete_selections_menu)
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










