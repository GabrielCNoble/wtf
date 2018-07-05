#include "ed_level_ui.h"
#include "ed_level.h"
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

#include "..\..\common\gmath\vector.h"
#include "..\..\common\gmath\matrix.h"

#include <stdio.h>
#include <string.h>

/* from ed_ui.c */
extern widget_bar_t *menu_dropdown_bar;

 



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
			record.index0 = light_CreateLight("light", &orientation, level_editor_3d_cursor_position, vec3(1.0, 1.0, 1.0), 25.0, 20.0, 0);
			record.type = PICK_LIGHT;
			editor_LevelEditorAddSelection(&record);
			level_editor_need_to_copy_data = 1;
		}
		else if(!strcmp(option->widget.name, "add brush cube"))
		{
			record.pointer = brush_CreateBrush(level_editor_3d_cursor_position, &orientation, vec3(1.0, 1.0, 1.0), BRUSH_CUBE, 0);
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
	option_list_t *brush_option_list;
	
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
	editor_LevelEditorCloseWaypointOptionMenu();
}

void editor_LevelEditorUIFinish()
{
	
}

void editor_LevelEditorUISetup()
{
	gui_SetVisible((widget_t *)level_editor_world_dropdown);
}

void editor_LevelEditorUIShutdown()
{
	gui_SetInvisible((widget_t *)level_editor_world_dropdown);
}

void editor_LevelEditorUpdateUI()
{
	
}


/*
====================================================================
====================================================================
====================================================================
*/


void editor_LevelEditorOpenAddToWorldMenu(int x, int y)
{
	if(level_editor_add_to_world_menu)
	{
		gui_SetVisible((widget_t *)level_editor_add_to_world_menu);
		
		level_editor_add_to_world_menu->widget.x = x + level_editor_add_to_world_menu->widget.w;
		level_editor_add_to_world_menu->widget.y = y - level_editor_add_to_world_menu->widget.h;
	}
}

void editor_LevelEditorCloseAddToWorldMenu()
{
	if(level_editor_add_to_world_menu)
	{
		gui_SetInvisible((widget_t *)level_editor_add_to_world_menu);
	}
}

void editor_LevelEditorOpenDeleteSelectionsMenu(int x, int y)
{
	if(level_editor_delete_selections_menu)
	{
		gui_SetVisible((widget_t *)level_editor_delete_selections_menu);
		
		level_editor_delete_selections_menu->widget.x = x;
		level_editor_delete_selections_menu->widget.y = y;
	}
}

void editor_LevelEditorCloseDeleteSelectionsMenu()
{
	if(level_editor_delete_selections_menu)
	{
		gui_SetInvisible((widget_t *)level_editor_delete_selections_menu);
	}
} 

void editor_LevelEditorOpenWaypointOptionMenu(int x, int y)
{
	if(level_editor_waypoints_options_menu)
	{
		gui_SetVisible((widget_t *)level_editor_waypoints_options_menu);
		
		level_editor_waypoints_options_menu->widget.x = x;
		level_editor_waypoints_options_menu->widget.y = y;
	}
}

void editor_LevelEditorCloseWaypointOptionMenu()
{
	if(level_editor_waypoints_options_menu)
	{
		gui_SetInvisible((widget_t *)level_editor_waypoints_options_menu);
	}
}










