#include "ed_entity_ui.h"
#include "..\..\common\gui.h"
#include "..\ed_ui.h"
#include "..\ed_common.h"

#include "..\..\common\containers\stack_list.h"

#include <stdio.h>
#include <string.h>

#include "..\..\common\entity.h"
#include "..\..\common\physics.h"

#include "..\..\common\gmath\vector.h"
#include "..\..\common\gmath\matrix.h"

//extern struct entity_def_t *entity_editor_current_entity_def;

extern int r_window_width;
extern int r_window_height;


extern int mouse_x;
extern int mouse_y;


extern struct entity_handle_t ed_entity_editor_entity_def;
extern struct entity_handle_t ed_entity_editor_preview_entity;
extern int ed_entity_editor_update_preview_entity;

extern collider_def_t *entity_editor_current_collider_def;
extern vec3_t entity_editor_3d_cursor_position;
extern vec3_t entity_editor_3d_handle_position;

/* from entity.c */
extern struct stack_list_t ent_entities[2];



extern struct stack_list_t mdl_models;


char *ed_entity_editor_add_component_menu_popup_name = "Add component menu";
int ed_entity_editor_add_component_menu_open = 0;
vec2_t ed_entity_editor_add_component_menu_pos;
struct entity_handle_t ed_entity_editor_selected_def = {1, INVALID_ENTITY_INDEX};



char *ed_entity_editor_set_component_value_menu_name = "Set component value";
int ed_entity_editor_set_component_value_menu_open = 0;
vec2_t ed_entity_editor_set_component_value_menu_pos;
int ed_entity_editor_set_component_type = COMPONENT_TYPE_NONE;



char *ed_entity_editor_defs_menu_name = "Entity defs";
int ed_entity_editor_defs_menu_open = 0;
vec2_t ed_entity_editor_defs_menu_pos;



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
	
	//if(!ed_entity_editor_current_entity_def)
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


/*
===============================================================
===============================================================
===============================================================
*/


void editor_EntityEditorInitUI()
{
/*	add_collider_primitive_menu = gui_AddOptionList(NULL, "add collider primitive menu", 0, 0, 220, 0, 8, editor_EntityEditorAddColliderPrimitiveMenuCallback);
	gui_AddOptionToList(add_collider_primitive_menu, "add cube", "Cube collision shape");
	gui_AddOptionToList(add_collider_primitive_menu, "add cylinder", "Cylinder collision shape");
	gui_AddOptionToList(add_collider_primitive_menu, "add sphere", "Sphere collision shape");
	
	
	
	add_component_menu = gui_AddOptionList(NULL, "add component menu", 0, 0, 310, 0, 16, editor_EntityEditorAddComponentMenuCallback);
	gui_AddOptionToList(add_component_menu, "add model component", "add model component");
	gui_AddOptionToList(add_component_menu, "add physics controller component", "add physics controller component");
	
	
	delete_selection_menu = gui_AddOptionList(NULL, "destroy selection menu", 0, 0, 50, 0, 8, editor_EntityEditorDestroySelectionMenuCallback);
	gui_AddOptionToList(delete_selection_menu, "delete", "Delete?");*/
	
	
//	editor_EntityEditorCloseAddColliderPrimitiveMenu();
//	editor_EntityEditorCloseAddComponentMenu();
//	editor_EntityEditorCloseDestroySelectionMenu();
}

void editor_EntityEditorFinishUI()
{
	
}

void editor_EntityEditorUpdateUI()
{
	editor_EntityEditorDefTree();
	editor_EntityEditorAddComponentMenu();
	editor_EntityEditorSetComponentValueMenu();
	editor_EntityEditorDefsMenu(); 
}

void editor_EntityEditorCloseAllMenus()
{
	
}

void editor_EntityEditorAddComponentMenu()
{
	int component_type;
	
	if(ed_entity_editor_add_component_menu_open)
	{
		if(ed_entity_editor_add_component_menu_open == 1)
		{
			gui_ImGuiOpenPopup(ed_entity_editor_add_component_menu_popup_name);
			ed_entity_editor_add_component_menu_open = 2;
		}
		
		if(!gui_ImGuiIsPopupOpen(ed_entity_editor_add_component_menu_popup_name))
		{
			/* If we got here it means this popup was closed by
			a mouse click outside of it. This means the flag 
			'ed_entity_editor_add_component_menu_open' didn't
			get cleared, since no option was clicked. So, we
			clear it here to make sure everything works correctly... */
			ed_entity_editor_add_component_menu_open = 0;
			return;
		}
		
		gui_ImGuiSetNextWindowPos(ed_entity_editor_add_component_menu_pos, 0, vec2(0.0, 0.0));
		
		if(gui_ImGuiBeginPopup(ed_entity_editor_add_component_menu_popup_name, ImGuiWindowFlags_AlwaysAutoResize))
		{	
			if(gui_ImGuiMenuItem("Add physics component", NULL, NULL, 1))
			{
				component_type = COMPONENT_TYPE_PHYSICS;
				ed_entity_editor_add_component_menu_open = 0;
			}
			if(gui_ImGuiMenuItem("Add model component", NULL, NULL, 1))
			{
				component_type = COMPONENT_TYPE_MODEL;
				ed_entity_editor_add_component_menu_open = 0;
			}
			if(gui_ImGuiMenuItem("Add script component", NULL, NULL, 1))
			{
				component_type = COMPONENT_TYPE_SCRIPT;
				ed_entity_editor_add_component_menu_open = 0;
			}
			if(gui_ImGuiMenuItem("Add camera component", NULL, NULL, 1))
			{
				component_type = COMPONENT_TYPE_CAMERA;
				ed_entity_editor_add_component_menu_open = 0;
			}
			if(gui_ImGuiMenuItem("Add light component", NULL, NULL, 1))
			{
				component_type = COMPONENT_TYPE_LIGHT;
				ed_entity_editor_add_component_menu_open = 0;
			}
			
			gui_ImGuiEndPopup();
			
			if(!ed_entity_editor_add_component_menu_open)
			{
				entity_AddComponent(ed_entity_editor_selected_def, component_type);
				ed_entity_editor_selected_def.entity_index = INVALID_ENTITY_INDEX;
			}
		}
	}
}



int editor_EntityEditorSetModelComponentValue(struct entity_handle_t entity)
{
	int keep_open = 2;
	
	struct model_t *model;
	int model_count;
	int selected_model_index = -1;
	int model_index;
	
	
	
	
	model_count = mdl_models.element_count;
	
	for(model_index = 0; model_index < model_count; model_index++)
	{
		model = model_GetModelPointerIndex(model_index);
		
		if(model)
		{
			if(gui_ImGuiMenuItem(model->name, NULL, NULL, 1) && selected_model_index == -1)
			{
				selected_model_index = model_index;
			}
		}
	}
	
	if(selected_model_index != -1)
	{
		keep_open = 0;
		entity_SetModel(entity, selected_model_index);
		ed_entity_editor_update_preview_entity = 1;
	}
	
	return keep_open;
}

int editor_EntityEditorSetScriptComponentValue()
{
	int keep_open = 2;
	
	
	
	return keep_open;
}


void editor_EntityEditorSetComponentValueMenu()
{
	int component_type;
	
	if(ed_entity_editor_set_component_value_menu_open)
	{
		if(ed_entity_editor_set_component_value_menu_open == 1)
		{
			gui_ImGuiOpenPopup(ed_entity_editor_set_component_value_menu_name);
			ed_entity_editor_set_component_value_menu_open = 2;
		}
		
		if(!gui_ImGuiIsPopupOpen(ed_entity_editor_set_component_value_menu_name))
		{
			/* If we got here it means this popup was closed by
			a mouse click outside of it. This means the flag 
			'ed_entity_editor_set_component_value_menu_open' didn't
			get cleared, since no option was clicked. So, we
			clear it here to make sure everything works correctly... */
			ed_entity_editor_set_component_value_menu_open = 0;
			return;
		}
		
		if(ed_entity_editor_set_component_type != COMPONENT_TYPE_NONE)
		{
			if(gui_ImGuiBeginPopup(ed_entity_editor_set_component_value_menu_name, ImGuiWindowFlags_AlwaysAutoResize))
			{
				
				switch(ed_entity_editor_set_component_type)
				{
					case COMPONENT_TYPE_MODEL:
						ed_entity_editor_set_component_value_menu_open = editor_EntityEditorSetModelComponentValue(ed_entity_editor_selected_def);
					break;
					
					case COMPONENT_TYPE_SCRIPT:
						ed_entity_editor_set_component_value_menu_open = editor_EntityEditorSetScriptComponentValue();
					break;
					
				}
				gui_ImGuiEndPopup();
			}
		}
		
		
	}
}

void editor_EntityEditorDefsMenu()
{
	struct entity_t *entity;
	
	int i;
	int c;
	
	int selected = -1;
	
	if(ed_entity_editor_defs_menu_open)
	{
		gui_ImGuiSetNextWindowPos(vec2(0.0, 200.0), ImGuiCond_Once, vec2(0.0, 0.0));
		//gui_ImGuiSetNextWindowSize(vec2(300.0, 250.0), 0);
		
		c = ent_entities[1].element_count;
		
		//if(gui_ImGuiBeginPopup(ed_entity_editor_add_component_menu_popup_name, ImGuiWindowFlags_AlwaysAutoResize))
		gui_ImGuiBegin(ed_entity_editor_defs_menu_name, NULL, ImGuiWindowFlags_AlwaysAutoResize);
	
		for(i = 0; i < c; i++)
		{
			entity = entity_GetEntityDefPointerIndex(i);
				
			if(entity)
			{
				if(gui_ImGuiMenuItem(entity->name, NULL, NULL, 1) && selected == -1)
				{
					selected = i;					
					ed_entity_editor_entity_def.entity_index = i;
					ed_entity_editor_update_preview_entity = 1;
				}
					
			}
		}
		gui_ImGuiEnd();
	}
}

/*
====================================================
====================================================
====================================================
*/

void editor_EntityEditorTransformComponent(struct transform_component_t *transform_component)
{
	gui_ImGuiText("Orientation: ");
	gui_ImGuiSameLine(0.0, -1.0);
	gui_ImGuiText("[%f %f %f]\n[%f %f %f]\n[%f %f %f]", transform_component->orientation.floats[0][0],
														transform_component->orientation.floats[0][1],
														transform_component->orientation.floats[0][2],
																								 
														transform_component->orientation.floats[1][0],
														transform_component->orientation.floats[1][1],
														transform_component->orientation.floats[1][2],
																								 
														transform_component->orientation.floats[2][0],
														transform_component->orientation.floats[2][1],
														transform_component->orientation.floats[2][2]);
								
	gui_ImGuiNewLine();				
	if(gui_ImGuiDragFloat3("Position", &transform_component->position.x, 0.001, 0.0, 0.0, "%0.3f", 1.0))
	{
		ed_entity_editor_update_preview_entity = 1;
	}								
		
	gui_ImGuiNewLine();	
	if(gui_ImGuiDragFloat3("Scale", &transform_component->scale.x, 0.001, 0.0, 0.0, "%0.3f", 1.0))
	{
		ed_entity_editor_update_preview_entity = 1;
	}					
}


void editor_EntityEditorModelComponent(struct model_component_t *model_component)
{	
	struct model_t *model;							
	model = model_GetModelPointerIndex(model_component->model_index);
	gui_ImGuiText("Model: %s", model->name);
}

void editor_EntityEditorCameraComponent(struct camera_component_t *camera_component)
{
	struct transform_component_t *transform_component;
	
	transform_component = entity_GetComponentPointer(camera_component->transform);
	
	editor_EntityEditorTransformComponent(transform_component);
}

void editor_EntityEditorScriptComponent(struct script_component_t *script_component)
{
	char *script_name;
	
	if(script_component->script)
	{
		script_name = script_component->script->name;
	}
	else
	{	
		script_name = "None";
	}
	gui_ImGuiText("Script: %s", script_name);
}

void editor_EntityEditorRecursiveDefTree(struct entity_handle_t entity)
{
	struct entity_t *entity_ptr;
	struct component_t *component_ptr;
	struct transform_component_t *transform_component;
	struct model_component_t *model_component;
	struct transform_component_t *child_transform;
	struct script_component_t *script_component;
	struct camera_component_t *camera_component;
		
	struct model_t *model;
	struct model_t *selected_model;
	int selected_model_index;
	
	char selected;
	int i;
	int c;
	char *component_name;
	char component_id[512];
	char *script_name;
	
	static int depth_level = -1;
	
	
	depth_level++;
	
	
	if(entity.entity_index != INVALID_ENTITY_INDEX)
	{
		entity_ptr = entity_GetEntityPointerHandle(entity);
		
		gui_ImGuiPushStyleColor(ImGuiCol_Text, vec4(1.0, 1.0, 0.0, 1.0));
		//if(gui_ImGuiTreeNodeEx(entity_ptr->name, ImGuiTreeNodeFlags_DefaultOpen, "[%s]", entity_ptr->name))
		if(gui_ImGuiTreeNodeEx(entity_ptr->name, ImGuiTreeNodeFlags_DefaultOpen, ""))
		{
			if(gui_ImGuiIsItemClicked(1))
			{
				/* add component to entity popup... */
				editor_EntityEditorOpenAddComponentMenu(mouse_x, r_window_height - mouse_y, entity);
			}	
		
			gui_ImGuiSameLine(0.0, -1.0);
			
			if(gui_ImGuiInputText(" ", entity_ptr->name, ENTITY_NAME_MAX_LEN, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				
			}
		
			gui_ImGuiPopStyleColor();
					
			for(i = 0; i < COMPONENT_TYPE_LAST; i++)
			{
				component_ptr = entity_GetComponentPointer(entity_ptr->components[i]);
				
				if(component_ptr)
				{				
					switch(component_ptr->type)
					{
						case COMPONENT_TYPE_TRANSFORM:
							component_name = "Transform component";
							transform_component = (struct transform_component_t *)component_ptr;
							sprintf(component_id, "%d-%s", depth_level, component_name);
							
							if(gui_ImGuiTreeNode(component_id, "%s", component_name))
							{		
								editor_EntityEditorTransformComponent(transform_component);
								gui_ImGuiTreePop();	
							}
						break;
						
						case COMPONENT_TYPE_MODEL:
							component_name = "Model component";
							model_component = (struct model_component_t *)component_ptr;
							sprintf(component_id, "%d-%s", depth_level, component_name);
							
							if(gui_ImGuiTreeNode(component_id, "%s", component_name))
							{
								if(gui_ImGuiIsItemClicked(1))
								{
									/* set component value popup... */
									editor_EntityEditorOpenSetComponentValueMenu(mouse_x, r_window_height - mouse_y, entity, component_ptr->type);
								}
								editor_EntityEditorModelComponent(model_component);
								gui_ImGuiTreePop();
							}
						break;
						
						case COMPONENT_TYPE_PHYSICS:
							component_name = "Physics component";
						break;
						
						case COMPONENT_TYPE_CAMERA:
							component_name = "Camera component";
							camera_component = (struct camera_component_t *)component_ptr;
							sprintf(component_id, "%d-%s", depth_level, component_name);
							
							if(gui_ImGuiTreeNode(component_id, "%s", component_name))
							{		
								editor_EntityEditorCameraComponent(camera_component);
								gui_ImGuiTreePop();	
							}
						break;
						
						case COMPONENT_TYPE_SCRIPT:
							component_name = "Script component";
							script_component = (struct script_component_t *)component_ptr;
							sprintf(component_id, "%d-%s", depth_level, component_name);
							
							if(gui_ImGuiTreeNode(component_id, "%s", component_name))
							{
								
								if(gui_ImGuiIsItemClicked(1))
								{
									/* set component value popup... */
									editor_EntityEditorOpenSetComponentValueMenu(mouse_x, r_window_height - mouse_y, entity, component_ptr->type);
								}
								
								editor_EntityEditorScriptComponent(script_component);
								gui_ImGuiTreePop();
							}
						break;
						
						default:
							component_name = "Nope";
						break;
					}
				}
			}
			
			transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
			
			for(i = 0; i < transform_component->children_count; i++)
			{
				child_transform = entity_GetComponentPointer(transform_component->child_transforms[i]);
				
				if(child_transform->base.entity.entity_index != INVALID_ENTITY_INDEX)
				{
					editor_EntityEditorRecursiveDefTree(child_transform->base.entity);
				}
			}
			
			gui_ImGuiTreePop();
		}
		else
		{
			gui_ImGuiPopStyleColor();
		}
	}
	
	depth_level--;
}

#define ENTITY_DEF_TREE_WINDOW_WIDTH 380.0

void editor_EntityEditorDefTree()
{	
	gui_ImGuiSetNextWindowPos(vec2(r_window_width - ENTITY_DEF_TREE_WINDOW_WIDTH, 0.0), 0, vec2(0.0, 0.0));
	gui_ImGuiSetNextWindowSize(vec2(ENTITY_DEF_TREE_WINDOW_WIDTH, 550.0), 0);
	
	if(gui_ImGuiBegin("Entity def tree", NULL, ImGuiWindowFlags_AlwaysHorizontalScrollbar))
	{
		editor_EntityEditorRecursiveDefTree(ed_entity_editor_entity_def);
	}
	
	gui_ImGuiEnd();
}


/*
====================================================
====================================================
====================================================
*/









void editor_EntityEditorOpenAddComponentMenu(int x, int y, struct entity_handle_t entity)
{
	ed_entity_editor_add_component_menu_open = 1;
	ed_entity_editor_add_component_menu_pos.x = x;
	ed_entity_editor_add_component_menu_pos.y = y;
	ed_entity_editor_selected_def = entity;
}

void editor_EntityEditorOpenSetComponentValueMenu(int x, int y, struct entity_handle_t entity, int component_type)
{
	ed_entity_editor_set_component_value_menu_open = 1;
	ed_entity_editor_set_component_value_menu_pos.x = x;
	ed_entity_editor_set_component_value_menu_pos.y = y;
	ed_entity_editor_selected_def = entity;
	ed_entity_editor_set_component_type = component_type;
}


void editor_EntityEditorToggleDefsMenu()
{
	ed_entity_editor_defs_menu_open ^= 1;
	
	if(ed_entity_editor_defs_menu_open)
	{
		ed_entity_editor_defs_menu_pos.x = 0.0;
		ed_entity_editor_defs_menu_pos.y = 200;
		
		//gui_ImGuiOpenPopup(ed_entity_editor_add_component_menu_popup_name);
	}
	
	
	
}



/*void editor_EntityEditorCloseAddComponentMenu()
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
}*/








