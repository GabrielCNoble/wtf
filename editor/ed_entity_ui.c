#include "ed_entity_ui.h"
#include "..\..\common\gui.h"

#include "..\..\common\GLEW\include\GL\glew.h"
#include "..\..\common\camera.h"
#include "..\..\common\path.h"
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

/* from r_main.c */
extern int r_window_width;
extern int r_window_height;

/* from r_imediate.c */
extern int r_imediate_color_shader;


extern int mouse_x;
extern int mouse_y;


extern struct entity_handle_t ed_entity_editor_entity_def;
struct entity_handle_t ed_entity_editor_entity_def_comp_to_set;
extern struct entity_handle_t ed_entity_editor_preview_entity;
extern int ed_entity_editor_update_preview_entity;
extern int ed_entity_editor_draw_collider_list_cursor;
extern struct collision_shape_t *ed_entity_editor_hovered_collision_shape;
extern struct entity_handle_t ed_entity_editor_draw_collider_list[1024];


extern struct collider_def_t *entity_editor_current_collider_def;
extern vec3_t entity_editor_3d_cursor_position;
extern vec3_t entity_editor_3d_handle_position;

/* from entity.c */
extern struct stack_list_t ent_entities[2];

/* from physics.c */
extern struct collider_def_t *phy_collider_defs;
extern struct stack_list_t phy_colliders[COLLIDER_TYPE_LAST];

/* from script.c */
extern struct script_t *scr_scripts;


/* from model.c */
extern struct stack_list_t mdl_models;


char *ed_entity_editor_add_component_menu_popup_name = "Add component menu";
int ed_entity_editor_add_component_menu_open = 0;
vec2_t ed_entity_editor_add_component_menu_pos;
struct entity_handle_t ed_entity_editor_selected_def = {1, INVALID_ENTITY_INDEX};
struct component_handle_t ed_entity_editor_selected_def_transform = {COMPONENT_TYPE_NONE, 1, INVALID_COMPONENT_INDEX};



char *ed_entity_editor_set_component_value_menu_name = "Set component value";
int ed_entity_editor_set_component_value_menu_open = 0;
vec2_t ed_entity_editor_set_component_value_menu_pos;
int ed_entity_editor_set_component_type = COMPONENT_TYPE_NONE;



char *ed_entity_editor_defs_menu_name = "Entity defs";
int ed_entity_editor_defs_menu_open = 0;
vec2_t ed_entity_editor_defs_menu_pos;


int ed_entity_editor_prop_menu_open = 0;
vec2_t ed_entity_editor_prop_menu_pos;


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
	int i;
	int c;

	struct entity_t *entity_defs;
	struct entity_t *entity_def;

	struct transform_component_t *transform_component;

	struct entity_handle_t handle;

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
			if(gui_ImGuiBeginMenu("Components"))
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
				if(gui_ImGuiMenuItem("Add navigation component", NULL, NULL, 1))
                {
                    component_type = COMPONENT_TYPE_NAVIGATION;
                    ed_entity_editor_add_component_menu_open = 0;
                }

				gui_ImGuiEndMenu();


				if(!ed_entity_editor_add_component_menu_open)
				{
					entity_AddComponent(ed_entity_editor_selected_def, component_type);
					ed_entity_editor_selected_def.entity_index = INVALID_ENTITY_INDEX;
					ed_entity_editor_update_preview_entity = 1;
				}
			}

			if(gui_ImGuiBeginMenu("Entities"))
			{
				c = ent_entities[1].element_count;
				entity_defs = (struct entity_t *)ent_entities[1].elements;

				for(i = 0; i < c; i++)
				{
					entity_def = entity_defs + i;

					if(entity_def->flags & ENTITY_FLAG_INVALID)
					{
						continue;
					}

					if(gui_ImGuiMenuItem(entity_def->name, NULL, NULL, 1) && ed_entity_editor_add_component_menu_open)
					{
						handle.def = 1;
						handle.entity_index = i;

						//entity_ParentEntity(ed_entity_editor_selected_def, handle);
						if(ed_entity_editor_selected_def_transform.type == COMPONENT_TYPE_NONE)
						{
							entity_ParentEntity(ed_entity_editor_selected_def, handle);
						}
						else
						{
							entity_ParentEntityToEntityTransform(ed_entity_editor_selected_def_transform, handle);
						}

						ed_entity_editor_add_component_menu_open = 0;
						ed_entity_editor_update_preview_entity = 1;
					}
				}

				gui_ImGuiEndMenu();
			}

			if(gui_ImGuiMenuItem("Remove", NULL, NULL, 1))
			{
				transform_component = entity_GetComponentPointer(ed_entity_editor_selected_def_transform);
				entity_UnpparentEntityFromEntityTransform(transform_component->parent, ed_entity_editor_selected_def);
				ed_entity_editor_add_component_menu_open = 0;
				ed_entity_editor_update_preview_entity = 1;
			}


			gui_ImGuiEndPopup();


		}
	}
}

void editor_EntityEditorPropMenu()
{

}



int editor_EntityEditorSetModelComponentValue(struct entity_handle_t entity)
{
	int keep_open = 2;

	struct model_t *model;
	int model_count;
	int selected_model_index = -1;
	int model_index;


	char label[512];

	model_count = mdl_models.element_count;

	for(model_index = 0; model_index < model_count; model_index++)
	{
		model = model_GetModelPointerIndex(model_index);

		if(model)
		{
			sprintf(label, "Set model to: %s", model->name);

			if(gui_ImGuiMenuItem(label, NULL, NULL, 1) && selected_model_index == -1)
			{
				selected_model_index = model_index;
				entity_SetModel(entity, selected_model_index);
				ed_entity_editor_update_preview_entity = 1;
				keep_open = 0;
			}
		}
	}

	return keep_open;
}

editor_EntityEditorSetPhysicsComponentValue(struct entity_handle_t entity)
{
	int keep_open = 2;

	struct collider_def_t *collider_defs;
	struct entity_t *entity_ptr;
	struct physics_component_t *physics_component;

	int i;
	int k;
	int c;

	collider_defs = phy_collider_defs;
	entity_ptr = entity_GetEntityPointerHandle(entity);
	physics_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);

	while(collider_defs)
	{
		if(gui_ImGuiMenuItem(collider_defs->name, NULL, NULL, 1))
		{
			if(physics_component->collider.collider_def)
			{
				physics_DecColliderDefRefCount(physics_component->collider.collider_def);
			}

			physics_IncColliderDefRefCount(collider_defs);

			physics_component->collider.collider_def = collider_defs;
			ed_entity_editor_update_preview_entity = 1;
			keep_open = 0;
		}

		collider_defs = collider_defs->next;
	}

	if(keep_open)
	{
		if(gui_ImGuiBeginMenu("Create new..."))
		{
			if(gui_ImGuiMenuItem("Character collider", NULL, NULL, 1))
			{
				collider_defs = physics_CreateCharacterColliderDef("New character collider", 0.5, 0.5, 0.25, 0.5, 0.5, 2.0);
				keep_open = 0;
			}

			if(gui_ImGuiMenuItem("Rigid body collider", NULL, NULL, 1))
			{
				collider_defs = physics_CreateRigidBodyColliderDef("New character collider");
				keep_open = 0;
			}

			if(gui_ImGuiMenuItem("Projectile collider", NULL, NULL, 1))
			{
				keep_open = 0;
			}

			if(!keep_open)
			{
				if(physics_component->collider.collider_def)
				{
					physics_DecColliderDefRefCount(physics_component->collider.collider_def);

					if(!physics_component->collider.collider_def->ref_count)
					{
						physics_DestroyColliderDefPointer(physics_component->collider.collider_def);
					}
				}

				//physics_component->collider.collider_def = collider_defs;

                entity_SetCollider(entity, collider_defs);

				ed_entity_editor_update_preview_entity = 1;
			}

			gui_ImGuiEndMenu();
		}
	}

	return keep_open;
}

int editor_EntityEditorSetScriptComponentValue(struct entity_handle_t entity)
{
	int keep_open = 2;
	struct script_t *script;

	//struct entity_t *entity_ptr;
	//struct script_component_t *script_component;

	script = scr_scripts;

	//entity_ptr = entity_GetEntityPointerHandle(entity);

	while(script)
	{
		if(!strcmp(path_GetFileExtension(script->file_name), ENTITY_SCRIPT_FILE_EXTENSION))
		{
			if(gui_ImGuiMenuItem(script->name, NULL, NULL, 1))
			{
				entity_SetScript(entity, script);
				ed_entity_editor_update_preview_entity = 1;
				keep_open = 0;
			}
		}

		script = script->next;
	}


	return keep_open;
}


void editor_EntityEditorSetComponentValueMenu()
{
	int component_type;

	struct entity_t *entity;

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

					case COMPONENT_TYPE_PHYSICS:
						ed_entity_editor_set_component_value_menu_open = editor_EntityEditorSetPhysicsComponentValue(ed_entity_editor_selected_def);
					break;

					case COMPONENT_TYPE_SCRIPT:
						ed_entity_editor_set_component_value_menu_open = editor_EntityEditorSetScriptComponentValue(ed_entity_editor_selected_def);
					break;

				}

				if(gui_ImGuiMenuItem("Remove", NULL, NULL, 1))
				{
					entity_RemoveComponent(ed_entity_editor_selected_def, ed_entity_editor_set_component_type);
					ed_entity_editor_set_component_value_menu_open = 0;
					ed_entity_editor_update_preview_entity;
				}

				if(!ed_entity_editor_set_component_value_menu_open)
				{
					ed_entity_editor_update_preview_entity = 1;
				}

				gui_ImGuiEndPopup();
			}
		}


	}
}

void editor_EntityEditorDefsMenu()
{
	struct entity_t *entity;
	struct entity_handle_t entity_def;

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

		if(gui_ImGuiIsWindowHovered(0))
		{
			if(gui_ImGuiIsMouseClicked(1, 0))
			{
				printf("FUCK!\n");
			}
		}

		for(i = 0; i < c; i++)
		{
			entity = entity_GetEntityDefPointerIndex(i);

			if(entity)
			{
				if(gui_ImGuiMenuItem(entity->name, NULL, NULL, 1) && selected == -1)
				{
					selected = i;

					entity_def.def = 1;
					entity_def.entity_index = i;
					editor_EntityEditorSetCurrentEntityDef(entity_def);
				}

			}
		}

		if(gui_ImGuiMenuItem("New entity def", NULL, NULL, 1))
		{
			entity_def = entity_CreateEntityDef("Unnamed entity def");
			editor_EntityEditorSetCurrentEntityDef(entity_def);
		}

		gui_ImGuiEnd();
	}
}

/*
====================================================
====================================================
====================================================
*/

void editor_EntityEditorTransformComponent(struct transform_component_t *transform_component, struct entity_handle_t entity, int ref_on_ref)
{
	vec3_t euler;
	mat3_t_euler(&transform_component->orientation, &euler);

//	struct entity_t *entity;
//	struct transform_component_t *transform;


	euler.x /= 3.14159265;
	euler.y /= 3.14159265;
	euler.z /= 3.14159265;


/*	entity = entity_GetEntityPointerHandle(transform_component->base.entity);
	transform = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);*/

/*	if(!ref_on_ref)
	{
		gui_ImGuiPushStyleColor(ImGuiCol_Text, vec4(1.0, 0.2, 0.2, 1.0));
	}*/


	if(gui_ImGuiDragFloat3("Orientation", &euler.x, 0.001, -1.0, 1.0, "%f", 1.0))
	{
		mat3_t_rotate(&transform_component->orientation, vec3_t_c(1.0, 0.0, 0.0), euler.x, 1);
		mat3_t_rotate(&transform_component->orientation, vec3_t_c(0.0, 1.0, 0.0), euler.y, 0);
		mat3_t_rotate(&transform_component->orientation, vec3_t_c(0.0, 0.0, 1.0), euler.z, 0);

		ed_entity_editor_update_preview_entity = 1;
	}

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

/*	if(!ref_on_ref)
	{
		gui_ImGuiPopStyleColor();
	}	*/
}


void editor_EntityEditorModelComponent(struct model_component_t *model_component, struct entity_handle_t entity, int extra)
{
	struct model_t *model;
	char *model_name;

	model = model_GetModelPointerIndex(model_component->model_index);
	if(model)
	{
		model_name = model->name;
	}
	else
	{
		model_name = "None";
	}

	gui_ImGuiText("Model: %s", model_name);
}

void editor_EntityEditorPhysicsComponent(struct physics_component_t *physics_component, struct entity_handle_t entity, int extra)
{
	struct entity_t *entity_ptr;
	struct collider_def_t *collider_def;
	struct collision_shape_t *collision_shape;

	mat4_t collision_shape_transform;

	camera_t *active_camera;

	char *collision_shape_type;

	char id[512];

	int i;
	int j;

	vec3_t euler;

	vec2_t cursor_start;
	vec2_t cursor_end;
	vec2_t box_size;

	vec3_t position;

	active_camera = camera_GetActiveCamera();

	char checked = 0;

	collider_def = (struct collider_def_t *)physics_component->collider.collider_def;

	ed_entity_editor_hovered_collision_shape = NULL;

	if(collider_def)
	{
		gui_ImGuiInputText(" ", collider_def->name, COLLIDER_DEF_NAME_MAX_LEN, 0);

		switch(collider_def->type)
		{
			case COLLIDER_TYPE_CHARACTER_COLLIDER:

				gui_ImGuiText("Type: Character collider");

				if(gui_ImGuiDragFloat("height", &collider_def->height, 0.001, 0.001, 10.0, "%.03f", 1.0))
				{
					ed_entity_editor_update_preview_entity = 1;
				}

				if(gui_ImGuiDragFloat("radius", &collider_def->radius, 0.001, 0.001, 10.0, "%.03f", 1.0))
				{
					ed_entity_editor_update_preview_entity = 1;
				}

				if(gui_ImGuiDragFloat("max slope angle", &collider_def->slope_angle, 0.001, 0.0, 1.0, "%0.3f", 1.0))
				{
					ed_entity_editor_update_preview_entity = 1;
				}

				if(gui_ImGuiDragFloat("max walk speed", &collider_def->max_walk_speed, 0.01, 0.1, 10.0, "%0.3f", 1.0))
				{
					ed_entity_editor_update_preview_entity = 1;
				}

			break;

			case COLLIDER_TYPE_RIGID_BODY_COLLIDER:

				/*********************************************************************************************/
				/*********************************************************************************************/
				/*********************************************************************************************/

				ed_entity_editor_draw_collider_list[ed_entity_editor_draw_collider_list_cursor] = entity;
				ed_entity_editor_draw_collider_list_cursor++;

				if(gui_ImGuiIsItemClicked(1))
				{
					if(!gui_ImGuiIsPopupOpen("Add collision shape popup"))
					{
						gui_ImGuiOpenPopup("Add collision shape popup");
						gui_ImGuiSetNextWindowPos(vec2(mouse_x, r_window_height - mouse_y), 0, vec2(0.0, 0.0));
					}
				}

				if(gui_ImGuiBeginPopup("Add collision shape popup", 0))
				{
					for(i = 0; i < COLLISION_SHAPE_LAST; i++)
					{
						switch(i)
						{
							case COLLISION_SHAPE_BOX:
								collision_shape_type = "Box";
							break;

							case COLLISION_SHAPE_SPHERE:
							case COLLISION_SHAPE_CAPSULE:
								continue;
								//collision_shape_type = "Sphere";
							break;

							case COLLISION_SHAPE_CYLINDER:
								collision_shape_type = "Cylinder";
							break;
						}

						if(gui_ImGuiMenuItem(collision_shape_type, NULL, NULL, 1))
						{
							physics_AddCollisionShape(collider_def, vec3_t_c(1.0, 1.0, 1.0), vec3_t_c(0.0, 0.0, 0.0), NULL, i);
						}
					}
					gui_ImGuiEndPopup();
				}

				/*********************************************************************************************/
				/*********************************************************************************************/
				/*********************************************************************************************/

				gui_ImGuiText("Type: Rigid body collider");

				for(i = 0; i < collider_def->collision_shape_count; i++)
				{
					collision_shape = collider_def->collision_shape + i;

					gui_ImGuiPushIDi(i);

					gui_ImGuiBeginGroup();

					switch(collision_shape->type)
					{
						case COLLISION_SHAPE_BOX:
							collision_shape_type = "Box";
						break;

						case COLLISION_SHAPE_SPHERE:
						case COLLISION_SHAPE_CAPSULE:
							continue;
							//collision_shape_type = "Sphere";
						break;

						case COLLISION_SHAPE_CYLINDER:
							collision_shape_type = "Cylinder";
						break;
					}

					gui_ImGuiText("Collision shape type: %s", collision_shape_type);

					mat3_t_euler(&collision_shape->orientation, &euler);

					euler.x /= 3.14159265;
					euler.y /= 3.14159265;
					euler.z /= 3.14159265;

					if(gui_ImGuiDragFloat3("Orientation", &euler.x, 0.001, -1.0, 1.0, "%f", 1.0))
					{
						mat3_t_rotate(&collision_shape->orientation, vec3_t_c(1.0, 0.0, 0.0), euler.x, 1);
						mat3_t_rotate(&collision_shape->orientation, vec3_t_c(0.0, 1.0, 0.0), euler.y, 0);
						mat3_t_rotate(&collision_shape->orientation, vec3_t_c(0.0, 0.0, 1.0), euler.z, 0);

						ed_entity_editor_update_preview_entity = 1;
					}
					gui_ImGuiNewLine();

					position = collision_shape->position;
					if(gui_ImGuiDragFloat3("Position", &position.x, 0.001, 0.0, 0.0, "%0.3f", 1.0))
					{
						ed_entity_editor_update_preview_entity = 1;
						physics_SetCollisionShapePosition(collider_def, position, i);
					}
					gui_ImGuiNewLine();

					switch(collision_shape->type)
					{
						case COLLISION_SHAPE_BOX:
							if(gui_ImGuiDragFloat3("Scale", &collision_shape->scale.x, 0.001, 0.0, 0.0, "%0.3f", 1.0))
							{
								ed_entity_editor_update_preview_entity = 1;
							}
						break;

						case COLLISION_SHAPE_SPHERE:
							continue;
							//collision_shape_type = "Sphere";
						break;

						case COLLISION_SHAPE_CYLINDER:

							if(gui_ImGuiDragFloat("Height", &collision_shape->scale.y, 0.001, 0.0, 0.0, "%0.3f", 1.0))
							{
								ed_entity_editor_update_preview_entity = 1;
							}

							if(gui_ImGuiDragFloat("Radius", &collision_shape->scale.x, 0.001, 0.0, 0.0, "%0.3f", 1.0))
							{
								collision_shape->scale.z = collision_shape->scale.x;

								ed_entity_editor_update_preview_entity = 1;
							}
						break;
					}

					gui_ImGuiNewLine();
					gui_ImGuiNewLine();

					gui_ImGuiEndGroup();

					if(gui_ImGuiIsItemHovered(0) || gui_ImGuiIsItemActive())
					{
						ed_entity_editor_hovered_collision_shape = collision_shape;
					}
					gui_ImGuiPopID();
				}
			break;
		}
	}


}

void editor_EntityEditorCameraComponent(struct camera_component_t *camera_component, struct entity_handle_t entity, int extra)
{
	struct transform_component_t *transform_component;
	struct entity_t *entity_ptr;
}

void editor_EntityEditorScriptComponent(struct script_component_t *script_component, struct entity_handle_t entity, int extra)
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

void editor_EntityEditorProp(struct entity_handle_t entity)
{
    int i;
    struct entity_t *entity_ptr;
    struct entity_prop_t *prop;

    int name_len;

    entity_ptr = entity_GetEntityPointerHandle(entity);

    if(entity_ptr)
	{
		if(gui_ImGuiTreeNode("Props", "Props"))
		{
			for(i = 0; i < entity_ptr->prop_count; i++)
			{
				prop = entity_ptr->props + i;
				gui_ImGuiPushIDi(i);

				gui_ImGuiInputText(" ", prop->name, ENTITY_PROP_NAME_MAX_LEN, ImGuiInputTextFlags_EnterReturnsTrue);

				gui_ImGuiSameLine(0.0, -1.0);
				if(gui_ImGuiButton("Remove", vec2(80.0, 16.0)))
				{
					entity_RemoveProp(entity, prop->name);
				}
				gui_ImGuiPopID();
			}

			if(gui_ImGuiMenuItem("Add prop", NULL, NULL, 1))
			{
				entity_AddProp(entity, "New prop", 4);
			}

			gui_ImGuiTreePop();
		}
	}
}

void editor_EntityEditorRecursiveDefTree(struct entity_handle_t entity, struct component_handle_t transform, int ref_on_ref)
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
	char *entity_name;
	int selected_model_index;

	char selected;
	int i;
	int c;
	char *component_name;
	char node_id[128];
	char component_id[128];
	char text_field_id[128];
	char *script_name;

	int component_node_open;

	static int depth_level = -1;
	static unsigned int id = 0;

	void (*component_function)(void *component, struct entity_handle_t entity, int ref_on_ref);

	depth_level++;


	if(entity.entity_index != INVALID_ENTITY_INDEX)
	{
		entity_ptr = entity_GetEntityPointerHandle(entity);

		gui_ImGuiPushStyleColor(ImGuiCol_Text, vec4(1.0, 1.0, 0.0, 1.0));


		if(transform.type != COMPONENT_TYPE_NONE)
		{
			transform_component = entity_GetComponentPointer(transform);
			entity_name = transform_component->instance_name;
		}
		else
		{
			entity_name = entity_ptr->name;
		}

		sprintf(node_id, "Node%d%d", depth_level, transform.index);

		if(gui_ImGuiTreeNodeEx(node_id, ImGuiTreeNodeFlags_DefaultOpen, " "))
		{
			gui_ImGuiSameLine(0.0, -1.0);
			gui_ImGuiInputText(" ", entity_name, ENTITY_NAME_MAX_LEN, 0);
			gui_ImGuiPopStyleColor();

			if(gui_ImGuiIsItemClicked(1))
			{
				/* add component to entity popup... */
				editor_EntityEditorOpenAddComponentMenu(mouse_x, r_window_height - mouse_y, entity, transform);
			}

			editor_EntityEditorProp(entity);



			for(i = 0; i < COMPONENT_TYPE_LAST; i++)
			{
				component_ptr = entity_GetComponentPointer(entity_ptr->components[i]);

				if(component_ptr)
				{
					switch(component_ptr->type)
					{
						case COMPONENT_TYPE_TRANSFORM:
							component_name = "Transform component";
							component_function = (void (*)(void *, struct entity_handle_t, int ))editor_EntityEditorTransformComponent;

							if(transform.type != COMPONENT_TYPE_NONE)
							{
								component_ptr = entity_GetComponentPointer(transform);
							}

						break;

						case COMPONENT_TYPE_MODEL:
							component_name = "Model component";
							component_function = (void (*)(void *, struct entity_handle_t, int ))editor_EntityEditorModelComponent;
						break;

						case COMPONENT_TYPE_PHYSICS:
							component_name = "Physics component";
							component_function = (void (*)(void *, struct entity_handle_t, int ))editor_EntityEditorPhysicsComponent;
						break;

						case COMPONENT_TYPE_CAMERA:
							component_name = "Camera component";
							component_function = (void (*)(void *, struct entity_handle_t, int ))editor_EntityEditorCameraComponent;
						break;

						case COMPONENT_TYPE_SCRIPT:
							component_name = "Script component";
							component_function = (void (*)(void *, struct entity_handle_t, int ))editor_EntityEditorScriptComponent;
						break;

						case COMPONENT_TYPE_NAVIGATION:
                            component_name = "Navigation component";
                            component_function = NULL;
                        break;

						default:
							component_name = "Nope";
							continue;
						break;
					}

					sprintf(component_id, "%s%d", component_name, transform.index);

					component_node_open = gui_ImGuiTreeNode(component_id, "%s", component_name);

					if(gui_ImGuiIsItemClicked(1))
					{
						editor_EntityEditorOpenSetComponentValueMenu(mouse_x, r_window_height - mouse_y, entity, component_ptr->type);
					}

					if(component_node_open)
					{
					    if(component_function)
                        {
                            component_function(component_ptr, entity, ref_on_ref);
                        }
						gui_ImGuiTreePop();
					}




				}
			}

			if(transform.type != COMPONENT_TYPE_NONE)
			{

				/* This is a ref to a entity def, which means this transform
				can have stuff nestled in it... */

				transform_component = entity_GetComponentPointer(transform);

				for(i = 0; i < transform_component->children_count; i++)
				{
					child_transform = entity_GetComponentPointer(transform_component->child_transforms[i]);

					if(child_transform->base.entity.entity_index != INVALID_ENTITY_INDEX)
					{
						editor_EntityEditorRecursiveDefTree(child_transform->base.entity, transform_component->child_transforms[i], 1);
					}
				}
			}


			/* Go over what the original def has nestled... */
			transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);

			for(i = 0; i < transform_component->children_count; i++)
			{
				child_transform = entity_GetComponentPointer(transform_component->child_transforms[i]);

				if(child_transform->base.entity.entity_index != INVALID_ENTITY_INDEX)
				{
					editor_EntityEditorRecursiveDefTree(child_transform->base.entity, transform_component->child_transforms[i], 0);
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
	ed_entity_editor_draw_collider_list_cursor = 0;

	gui_ImGuiSetNextWindowPos(vec2(r_window_width - ENTITY_DEF_TREE_WINDOW_WIDTH, 0.0), 0, vec2(0.0, 0.0));
	gui_ImGuiSetNextWindowSize(vec2(ENTITY_DEF_TREE_WINDOW_WIDTH, 550.0), 0);

	if(gui_ImGuiBegin("Entity def tree", NULL, ImGuiWindowFlags_AlwaysHorizontalScrollbar))
	{
		editor_EntityEditorRecursiveDefTree(ed_entity_editor_entity_def, (struct component_handle_t){COMPONENT_TYPE_NONE, 1, INVALID_COMPONENT_INDEX}, 0);
	}

	gui_ImGuiEnd();
}


/*
====================================================
====================================================
====================================================
*/


void editor_EntityEditorOpenAddComponentMenu(int x, int y, struct entity_handle_t entity, struct component_handle_t transform)
{
	ed_entity_editor_add_component_menu_open = 1;
	ed_entity_editor_add_component_menu_pos.x = x;
	ed_entity_editor_add_component_menu_pos.y = y;
	ed_entity_editor_selected_def = entity;
	ed_entity_editor_selected_def_transform = transform;
}

void editor_EntityEditorOpenAddPropMenu(int x, int y, struct entity_handle_t entity)
{
	ed_entity_editor_prop_menu_open = 1;
	ed_entity_editor_prop_menu_pos.x = x;
	ed_entity_editor_prop_menu_pos.y = y;
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
	}
}







