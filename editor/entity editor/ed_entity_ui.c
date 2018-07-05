#include "ed_entity_ui.h"
#include "..\..\common\gui.h"
#include "..\ed_ui.h"
#include "..\ed_common.h"

#include <stdio.h>
#include <string.h>

#include "..\..\common\entity.h"
#include "..\..\common\physics.h"

#include "..\..\common\gmath\vector.h"
#include "..\..\common\gmath\matrix.h"

extern struct entity_def_t *entity_editor_current_entity_def;
extern collider_def_t *entity_editor_current_collider_def;
extern vec3_t entity_editor_3d_cursor_position;
extern vec3_t entity_editor_3d_handle_position;

/*
===============================================================
===============================================================
===============================================================
*/

option_list_t *add_collider_primitive_menu = NULL;
option_list_t *delete_selection_menu = NULL;

option_list_t *add_component_menu = NULL;


/*
===============================================================
===============================================================
===============================================================
*/

void editor_EntityEditorAddColliderPrimitiveMenuCallback(widget_t *widget)
{
	option_t *option;
	char collider_def_name[512];
	int shape;
	mat3_t relative_orientation = mat3_t_id();
	
	if(!entity_editor_current_entity_def)
	{
		return;
	}
	
	/*if(!entity_editor_current_entity_def->collider_def)
	{
		strcpy(collider_def_name, entity_editor_current_entity_def->name);
		strcat(collider_def_name, ".collider");
		entity_editor_current_entity_def->collider_def = physics_CreateColliderDef(collider_def_name);
	}
	 
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		
		if(!strcmp(option->widget.name, "add cube"))
		{
			shape = COLLISION_SHAPE_BOX;
		}
		else if(!strcmp(option->widget.name, "add cylinder"))
		{
			shape = COLLISION_SHAPE_CYLINDER;
		}
		else if(!strcmp(option->widget.name, "add sphere"))
		{
			shape = COLLISION_SHAPE_SPHERE;
		}
		else
		{
			return;
		}
		
		physics_AddCollisionShape(entity_editor_current_entity_def->collider_def, vec3(1.0, 1.0, 1.0), entity_editor_3d_cursor_position, &relative_orientation, shape);
	}*/
}

void editor_EntityEditorDestroySelectionMenuCallback(widget_t *widget)
{
	option_t *option;
	
	if(widget->type == WIDGET_OPTION)
	{
		editor_EntityEditorDestroySelections();
	}
}


void editor_EntityEditorAddComponentMenuCallback(widget_t *widget)
{
	option_t *option;
}

/*
===============================================================
===============================================================
===============================================================
*/


void editor_EntityEditorInitUI()
{
	add_collider_primitive_menu = gui_AddOptionList(NULL, "add collider primitive menu", 0, 0, 220, 0, 8, editor_EntityEditorAddColliderPrimitiveMenuCallback);
	gui_AddOptionToList(add_collider_primitive_menu, "add cube", "Cube collision shape");
	gui_AddOptionToList(add_collider_primitive_menu, "add cylinder", "Cylinder collision shape");
	gui_AddOptionToList(add_collider_primitive_menu, "add sphere", "Sphere collision shape");
	
	
	
	add_component_menu = gui_AddOptionList(NULL, "add component menu", 0, 0, 310, 0, 16, editor_EntityEditorAddComponentMenuCallback);
	gui_AddOptionToList(add_component_menu, "add model component", "add model component");
	gui_AddOptionToList(add_component_menu, "add physics controller component", "add physics controller component");
	
	
	delete_selection_menu = gui_AddOptionList(NULL, "destroy selection menu", 0, 0, 50, 0, 8, editor_EntityEditorDestroySelectionMenuCallback);
	gui_AddOptionToList(delete_selection_menu, "delete", "Delete?");
	
	
	editor_EntityEditorCloseAddColliderPrimitiveMenu();
	editor_EntityEditorCloseAddComponentMenu();
	editor_EntityEditorCloseDestroySelectionMenu();
}

void editor_EntityEditorFinishUI()
{
	
}



void editor_EntityEditorOpenAddComponentMenu(int x, int y)
{
	if(add_component_menu)
	{
		gui_SetVisible((widget_t *)add_component_menu);
		
		add_component_menu->widget.x = x + add_component_menu->widget.w;
		add_component_menu->widget.y = y - add_component_menu->widget.h;
	}
}

void editor_EntityEditorCloseAddComponentMenu()
{
	if(add_component_menu)
	{
		gui_SetInvisible((widget_t *)add_component_menu);
	}
}



void editor_EntityEditorOpenAddColliderPrimitiveMenu(int x, int y)
{
	if(add_collider_primitive_menu)
	{
		gui_SetVisible((widget_t *)add_collider_primitive_menu);
		
		add_collider_primitive_menu->widget.x = x + add_collider_primitive_menu->widget.w;
		add_collider_primitive_menu->widget.y = y - add_collider_primitive_menu->widget.h;
	}
}

void editor_EntityEditorCloseAddColliderPrimitiveMenu()
{
	if(add_collider_primitive_menu)
	{
		gui_SetInvisible((widget_t *)add_collider_primitive_menu);
	}
} 

void editor_EntityEditorOpenDestroySelectionMenu(int x, int y)
{
	if(delete_selection_menu)
	{
		gui_SetVisible((widget_t *)delete_selection_menu);
		
		delete_selection_menu->widget.x = x;
		delete_selection_menu->widget.y = y;
	}
}

void editor_EntityEditorCloseDestroySelectionMenu()
{
	if(delete_selection_menu)
	{
		gui_SetInvisible((widget_t *)delete_selection_menu);
	}
}








