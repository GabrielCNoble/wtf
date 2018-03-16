#include <stdio.h>


#include "ed_ui.h"
#include "ed_proj.h"
#include "gui.h"
#include "engine.h"
#include "bsp_cmp.h"
#include "editor.h"
#include "r_main.h"
#include "log.h"
#include "material.h"
#include "texture.h"

#include "GL\glew.h"


#include "ed_ui_brush.h"
#include "ed_ui_material.h"
#include "ed_ui_texture.h"
#include "ed_ui_explorer.h"

 


/* from material.c */
extern int material_count;
extern material_t *materials;
extern char **material_names;

/* from l_main.c */
extern light_params_t *light_params;
extern light_position_t *light_positions;

/* from texture.c */
extern int texture_count;
extern texture_t *textures;
extern texture_info_t *texture_info;

/* from brush.c */
extern brush_t *brushes;


/* from r_main.c */
extern int r_window_width;
extern int r_window_height;
extern int r_z_prepass;
extern int r_draw_shadow_maps;
extern int r_bloom;
extern int r_tonemap;
extern int handle_3d_mode;
extern int forward_pass_shader;


widget_t *menu_bar = NULL;
widget_bar_t *menu_dropdown_bar = NULL;
dropdown_t *file_dropdown = NULL;
dropdown_t *world_dropdown = NULL;

option_list_t *option_list = NULL;

option_list_t *add_to_world_menu = NULL;
option_list_t *brush_types_option_list = NULL;
option_list_t *entity_defs_option_list = NULL;
dropdown_t *misc_dropdown = NULL;


option_list_t *delete_menu = NULL;


widget_t *save_project_window = NULL;
text_field_t *save_project_text_field = NULL;
button_t *confirm_save_project_button = NULL;
button_t *cancel_save_project_button = NULL;



widget_t *open_project_window = NULL;
text_field_t *open_project_text_field = NULL;
button_t *confirm_open_project_button = NULL;
button_t *cancel_open_project_button = NULL;


text_field_t *fps_display = NULL;
text_field_t *handle_3d_mode_display = NULL;



int ed_light_properties_window_open = 0;
widget_t *light_properties_window = NULL;
slider_t *selected_light_r = NULL;
slider_t *selected_light_g = NULL;
slider_t *selected_light_b = NULL;
slider_t *selected_light_radius = NULL;
slider_t *selected_light_energy = NULL;






/*widget_t *materials_window = NULL;
option_list_t *material_list = NULL;


int ed_cur_editing_material_index = 0;
widget_t *edit_material_window = NULL;
text_field_t *edit_material_material_name_text_field = NULL;
text_field_t *edit_material_material_red = NULL;
text_field_t *edit_material_material_green = NULL;
text_field_t *edit_material_material_blue = NULL;
dropdown_t *edit_material_diffuse_texture_dropdown = NULL;
dropdown_t *edit_material_normal_texture_dropdown = NULL;
option_list_t *edit_material_diffuse_texture_list = NULL;
option_list_t *edit_material_normal_texture_list = NULL;
button_t *edit_material_done_button = NULL;
button_t *edit_material_delete_material_button = NULL;
button_t *edit_material_create_material = NULL;*/




dropdown_t *snap_value_dropdown = NULL;
option_list_t *snap_values_list = NULL;






//dropdown_t *wow;


/* from ed_proj.c */
extern char current_project_name[];

/* from editor.c */
extern vec3_t cursor_3d_position;
extern vec3_t handle_3d_position;
extern int bm_handle_3d_flags;
extern int handle_3d_position_mode;
extern int ed_handle_3d_mode;
extern char *ed_handle_3d_mode_str;
extern light_ptr_t selected_light;
extern int selected_type;
extern light_params_t *ed_selected_light_params;
extern light_position_t *ed_selected_light_position;
extern brush_t *ed_selected_brush;
extern int ed_selected_brush_polygon_index;

extern float ed_editor_linear_snap_values[];
extern char *ed_editor_linear_snap_values_str[];
extern float ed_editor_angular_snap_values[];
extern char *ed_editor_angular_snap_values_str[];
extern float ed_editor_linear_snap_value;
extern int ed_editor_linear_snap_value_index;
extern float ed_editor_angular_snap_value;
extern int ed_editor_angular_snap_value_index;
extern char *ed_editor_snap_value_str;

//int ed_edit_material_index;

extern int b_draw_brushes;
extern int b_draw_leaves;
extern int b_draw_light_leaves;
extern int b_draw_world_polygons;
extern int b_draw_brush_polygons;
extern int b_draw_portals;
extern int b_draw_pvs_steps;
extern int b_draw_collision_polygons;

int add_light_unique_index;
int add_brush_unique_index;
int add_cube_brush_unique_index;
int add_cylinder_brush_unique_index;
int add_spawn_point_unique_index;


/* from pvs.c */
extern int b_step;
extern SDL_sem *step_semaphore;


/* from engine.c */
extern float fps;

float insane_float0 = 0.5;
float insane_float1 = 0.5;
float insane_float2 = 0.5;

/* from input.c */
extern int bm_mouse;

//static char *no_tex = "None";


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
	//if(editor_OpenProject(open_project_text_field->text))
	/*{
		editor_SetProjectName(open_project_text_field->text);
	}*/
	
	editor_OpenProject(open_project_text_field->text);
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
				editor_NewProject();
			break;
			
			case 1:
				editor_OpenSaveProjectWindow();
			break;	
			
			case 2:
				editor_OpenOpenProjectWindow();
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
				brush_SetAllInvisible();
				bsp_CompileBsp(0);
			break;
			
			case 1:
				bsp_Stop();
			break;
			
			case 2:
				bsp_DeleteBsp();
				brush_SetAllVisible();
			break;
			
			case 3:
				editor_ExportBsp(current_project_name);
			break;
			
			case 4:
				b_draw_portals ^= 1;
			break;
			
			case 5:
				b_draw_brushes ^= 1;
			break;
			
			case 6:
				b_draw_leaves ^= 1;
			break;
			
			case 7:
				b_draw_light_leaves ^= 1;
			break;
			
			case 8:
				b_draw_world_polygons ^= 1;
			break;
			
			case 9:
				b_draw_brush_polygons ^= 1;
			break;
			
			case 10:
				b_draw_collision_polygons ^= 1;
			break;
			/*case 10:
				b_draw_pvs_steps ^= 1;
			break;*/
			
			
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
				
				if(!SDL_SemValue(step_semaphore))
					SDL_SemPost(step_semaphore);
			break;
			
			case 5:
				r_bloom ^= 1;
			break;
			
			case 6:
				r_tonemap ^= 1;
			break;
		}
	}
}


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
			
			brush_index = brush_CreateBrush(cursor_3d_position, &r, vec3(1.0, 1.0, 1.0), BRUSH_CUBE, 0);
			
			
			record.type = PICK_BRUSH;
			record.index0 = brush_index;
			
			editor_ClearSelection();
			editor_AddSelection(&record);
			
			handle_3d_position = cursor_3d_position;
		}
		else if(option->unique_index == add_cylinder_brush_unique_index)
		{
			brush_index = brush_CreateBrush(cursor_3d_position, &r, vec3(1.0, 1.0, 1.0), BRUSH_CYLINDER, 0);
			
			
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


/*void material_window_callback(widget_t *widget)
{
	option_t *option;
	int material_index;
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		
		material_index = (int)option->widget.data;
		
		editor_CloseMaterialWindow();		
		editor_OpenEditMaterialWindow(material_index);
	}
	
}*/

/*void material_window_create_material_callback(widget_t *widget)
{
	button_t *button;
	
	if(widget->type == WIDGET_BUTTON)
	{
		button = (button_t *)widget;
		
		material_CreateMaterial("new_material", vec4(1.0, 1.0, 1.0, 1.0), 1.0, 1.0, forward_pass_shader, -1, -1);
		editor_CloseMaterialWindow();
		editor_OpenMaterialWindow();
	}
}
*/
/*void edit_material_window_callback(widget_t *widget)
{
	button_t *button;	
	if(widget->type == WIDGET_BUTTON)
	{
		if(widget == (widget_t *)edit_material_delete_material_button)
		{
			material_DestroyMaterialIndex(ed_edit_material_index);
		}
		editor_OpenMaterialWindow();
	}
}*/

/*void edit_material_text_field_callback(widget_t *widget)
{
	text_field_t *field;
	
	if(widget->type == WIDGET_TEXT_FIELD)
	{
		field = (text_field_t *)widget;
		
		if(field->bm_text_field_flags & TEXT_FIELD_UPDATED)
		{
			if(!material_SetMaterialName(field->text, ed_edit_material_index))
			{
				field->bm_text_field_flags &= ~TEXT_FIELD_UPDATED;
				gui_SetText(widget, material_names[ed_edit_material_index]);
			}
		}
	}
}*/

/*void set_material_diffuse_texture_callback(widget_t *widget)
{
	option_t *option;
	material_t *material;
	int texture_index;
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		texture_index = (int)option->widget.data;
		
		material = &materials[ed_edit_material_index];
		material->diffuse_texture = texture_index;
			
		if(texture_index > -1)
		{
			edit_material_diffuse_texture_dropdown->widget.var->addr = &texture_info[texture_index].name;
		}
		else
		{
			edit_material_diffuse_texture_dropdown->widget.var->addr = &no_tex;
		}
		
	}
	
}



void set_material_normal_texture_callback(widget_t *widget)
{
	option_t *option;
	material_t *material;
	int texture_index;
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		texture_index = (int)option->widget.data;
		
		material = &materials[ed_edit_material_index];
		material->normal_texture = texture_index;
		
		if(texture_index > -1)
		{
			edit_material_normal_texture_dropdown->widget.var->addr = &texture_info[texture_index].name;
		}
		else
		{
			edit_material_normal_texture_dropdown->widget.var->addr = &no_tex;
		}
		
		
	}
	
}*/

void snap_value_dropdown_callbac(widget_t *widget)
{
	dropdown_t *dropdown;
	int i = 0;
	if(widget->type == WIDGET_DROPDOWN)
	{
		dropdown = (dropdown_t *)widget;
		
		if(!(dropdown->bm_dropdown_flags & DROPDOWN_DROPPED))
			return;
		
		gui_RemoveAllOptions(snap_values_list);
		switch(ed_handle_3d_mode)
		{
			case HANDLE_3D_TRANSLATION:
			case HANDLE_3D_SCALE:
				while(ed_editor_linear_snap_values_str[i])
				{
					gui_AddOptionToList(snap_values_list, ed_editor_linear_snap_values_str[i], ed_editor_linear_snap_values_str[i]);
					i++;
				}
			break;
			
			case HANDLE_3D_ROTATION:
				while(ed_editor_angular_snap_values_str[i])
				{
					gui_AddOptionToList(snap_values_list, ed_editor_angular_snap_values_str[i], ed_editor_angular_snap_values_str[i]);
					i++;
				}
			break;
		}
		
	}
}

void set_snap_value_callback(widget_t *widget)
{
	option_t *option;
	int i;
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;	
		
		//gui_RemoveAllOptions(snap_values_list);
		
		switch(ed_handle_3d_mode)
		{
			case HANDLE_3D_TRANSLATION:
			case HANDLE_3D_SCALE:
				ed_editor_linear_snap_value = ed_editor_linear_snap_values[option->index];
				ed_editor_snap_value_str = ed_editor_linear_snap_values_str[option->index];
				ed_editor_linear_snap_value_index = option->index;
			break;
			
			case HANDLE_3D_ROTATION:
				ed_editor_angular_snap_value = ed_editor_angular_snap_values[option->index];
				ed_editor_snap_value_str = ed_editor_angular_snap_values_str[option->index];
				ed_editor_angular_snap_value_index = option->index;
			break;
		}
	}
	
}


int awesome_int = 0;

void editor_InitUI()
{
	
	int i;
	//widget_t *w;
	
	option_list_t *o;
	
	//menu_bar = gui_CreateWidget("menu_bar", 0, r_window_height * 0.5 - MENU_BAR_HEIGHT * 0.5, r_window_width, MENU_BAR_HEIGHT);	
	menu_bar = gui_AddWidget(NULL, "menu_bar", 0, r_window_height * 0.5 - MENU_BAR_HEIGHT * 0.5, r_window_width, MENU_BAR_HEIGHT);
	
	menu_dropdown_bar = gui_AddWidgetBar(menu_bar, "menu widget bar", 0, 0, r_window_width, MENU_BAR_HEIGHT, WIDGET_BAR_FIXED_SIZE);
	 
	file_dropdown = gui_CreateDropdown("file", "file", 0, 0, FILE_DROPDOWN_WIDTH, 0, file_dropdown_callback);
	gui_AddOption(file_dropdown, "new", "new...");
	gui_AddOption(file_dropdown, "save", "save");
	gui_AddOption(file_dropdown, "load", "load");
	gui_AddOption(file_dropdown, "exit", "exit");
	
	gui_AddWidgetToBar((widget_t *) file_dropdown, menu_dropdown_bar);
	
	world_dropdown = gui_CreateDropdown("world", "world", 0, 0, WORLD_DROPDOWN_WIDTH, 0, world_dropdown_callback);
	gui_AddOption(world_dropdown, "compile bsp", "compile bsp");
	gui_AddOption(world_dropdown, "stop pvs calculation", "stop pvs calculation");
	gui_AddOption(world_dropdown, "clear bsp", "clear bsp");
	gui_AddOption(world_dropdown, "export bsp", "export bsp");
	gui_AddOption(world_dropdown, "draw portals", "draw portals");
	gui_AddOption(world_dropdown, "draw brushes", "draw brushes");
	gui_AddOption(world_dropdown, "draw leaves", "draw leaves");
	gui_AddOption(world_dropdown, "draw light leaves", "draw light leaves");
	gui_AddOption(world_dropdown, "draw world polygons", "draw world polygons");
	gui_AddOption(world_dropdown, "draw brush polygons", "draw brush polygons");
	gui_AddOption(world_dropdown, "draw collision polygons", "draw collision polygons");
	//gui_AddOption(world_dropdown, "visualize pvs steps", "visualize pvs steps");
	//gui_InvalidOption((option_list_t *)world_dropdown->widget.nestled, 10);
	
	gui_AddWidgetToBar((widget_t *) world_dropdown, menu_dropdown_bar);
	
	misc_dropdown = gui_CreateDropdown("misc", "misc", 0, 0, MISC_DROPDOWN_WIDTH, 0, misc_dropdown_callback);
	gui_AddOption(misc_dropdown, "hot reload shaders", "hot reload shaders");
	gui_AddOption(misc_dropdown, "enable z prepass", "enable z prepass");
	gui_AddOption(misc_dropdown, "enable shadow mapping", "enable shadow mapping");
	gui_AddOption(misc_dropdown, "flush log", "flush log");
	gui_AddOption(misc_dropdown, "step pvs", "step pvs");
	gui_AddOption(misc_dropdown, "enable bloom", "enable bloom");
	gui_AddOption(misc_dropdown, "enable tonemapping", "enable tonemapping");
	
	gui_AddWidgetToBar((widget_t *)misc_dropdown, menu_dropdown_bar);
	
	//checkbox_t *checkbox = gui_AddCheckbox(menu_bar, 200, 0, 25, 25, 0, NULL);
	//checkbox->bm_check_flags = 256;
	
	//gui_var_t *v = gui_CreateVar("int", GUI_VAR_INT, &awesome_int, NULL, 0);
	
	//gui_TrackVar(v, checkbox);
		
	
	add_to_world_menu = gui_AddOptionList(NULL, "add to world", 0, 0, 200, 0, 8, add_to_world_menu_callback);
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
	
	
	delete_menu = gui_AddOptionList(NULL, "delete", 0, 0, 100, 0, 8, delete_selection_menu_callback);
	gui_AddOptionToList(delete_menu, "delete", "delete?");
	gui_SetInvisible((widget_t *)delete_menu);
	

	//save_project_window = gui_CreateWidget("Save map", 0, 0, 590, 80);
	
	save_project_window = gui_AddWidget(NULL, "Save map", 0, 0, 590, 80);
	
	save_project_text_field = gui_AddTextField(save_project_window, "save project text field", 0, 20, 580, 0, NULL);
	confirm_save_project_button = gui_AddButton(save_project_window, "confirm", 80, -15, 90, 40, 0, confirm_save_project_button_callback);
	cancel_save_project_button = gui_AddButton(save_project_window, "cancel", -80, -15, 90, 40, 0, cancel_save_project_button_callback);
	save_project_window->bm_flags |= WIDGET_SHOW_NAME | WIDGET_RENDER_NAME;

	gui_SetInvisible((widget_t *)save_project_window);
	
	//open_project_window = gui_CreateWidget("Open map", 0, 0, 590, 80);
	open_project_window = gui_AddWidget(NULL, "Open map", 0, 0, 590, 80);
	open_project_text_field = gui_AddTextField(open_project_window, "open project text field", 0, 20, 580, 0, NULL);
	confirm_open_project_button = gui_AddButton(open_project_window, "confirm", 80, -15, 90, 40, 0, confirm_open_project_button_callback);
	cancel_open_project_button = gui_AddButton(open_project_window, "cancel", -80, -15, 90, 40, 0, cancel_open_project_button_callback);
	open_project_window->bm_flags |= WIDGET_SHOW_NAME | WIDGET_RENDER_NAME;
	gui_SetInvisible((widget_t *)open_project_window);

	
	//fps_display = gui_AddTextField(NULL, "fps", r_window_wi/dth * 0.5 - FPS_DISPLAY_WIDTH * 0.5, 0, FPS_DISPLAY_WIDTH, 0 ,NULL);
	//gui_TrackVar(gui_CreateVar("fps", GUI_VAR_FLOAT, &fps), (widget_t *)fps_display);
	
	handle_3d_mode_display = gui_AddTextField(NULL, "handle 3d mode", -r_window_width * 0.5 + HANDLE_3D_MODE_DISPLAY_WIDTH * 0.5, -r_window_height * 0.5 + OPTION_HEIGHT * 0.5, HANDLE_3D_MODE_DISPLAY_WIDTH, 0, NULL);
	gui_TrackVar(gui_CreateVar("handle 3d mode", GUI_VAR_STRING, &ed_handle_3d_mode_str, NULL, 0), (widget_t *)handle_3d_mode_display);
	snap_value_dropdown = gui_AddDropdown(NULL, "snap value dropdown", "snap value dropdown", -r_window_width * 0.5 + HANDLE_3D_MODE_DISPLAY_WIDTH + SNAP_VALUE_DROPDOWN_WIDTH * 0.5, -r_window_height * 0.5 + OPTION_HEIGHT * 0.5, SNAP_VALUE_DROPDOWN_WIDTH, 0, snap_value_dropdown_callbac);
	snap_values_list = gui_AddOptionList((widget_t *)snap_value_dropdown, "snap values list", 0, OPTION_HEIGHT * 0.5, SNAP_VALUE_DROPDOWN_WIDTH, 0, 8, set_snap_value_callback);
	/*gui_AddOptionToList(snap_values_list, snap_values_str[0], snap_values_str[0]);
	gui_AddOptionToList(snap_values_list, snap_values_str[1], snap_values_str[1]);
	gui_AddOptionToList(snap_values_list, snap_values_str[2], snap_values_str[2]);
	gui_AddOptionToList(snap_values_list, snap_values_str[3], snap_values_str[3]);*/
	
	gui_var_t *current_snap_value_str = gui_CreateVar("current snap value str", GUI_VAR_STRING, &ed_editor_snap_value_str, NULL, 0);
	gui_TrackVar(current_snap_value_str, (widget_t *)snap_value_dropdown);
	
//	gui_AddOptionToList(snap_values_list, snap_values_str[4], snap_values_str[4]);


	
		
	gui_var_t *var0 = gui_CreateVar("selected light r", GUI_VAR_UNSIGNED_CHAR, NULL, NULL, 0);
	gui_var_t *var1 = gui_CreateVar("selected light g", GUI_VAR_UNSIGNED_CHAR, NULL, NULL, 0);
	gui_var_t *var2 = gui_CreateVar("selected light b", GUI_VAR_UNSIGNED_CHAR, NULL, NULL, 0);
	gui_var_t *radius_var = gui_CreateVar("selected light radius", GUI_VAR_UNSIGNED_SHORT, NULL, NULL, 0);
	gui_var_t *energy_var = gui_CreateVar("selected light energy", GUI_VAR_UNSIGNED_SHORT, NULL, NULL, 0);
	
	//light_properties_window = gui_CreateWidget("Light properties", r_window_width * 0.5 - LIGHT_PROPERTIES_WINDOW_WIDTH * 0.5, 0, LIGHT_PROPERTIES_WINDOW_WIDTH, LIGHT_PROPERTIES_WINDOW_HEIGHT);
	light_properties_window = gui_AddWidget(NULL, "Light properties", r_window_width * 0.5 - LIGHT_PROPERTIES_WINDOW_WIDTH * 0.5, 0, LIGHT_PROPERTIES_WINDOW_WIDTH, LIGHT_PROPERTIES_WINDOW_HEIGHT);
	light_properties_window->bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
 	selected_light_r = gui_AddSlider(light_properties_window, "Red", 0, LIGHT_PROPERTIES_WINDOW_HEIGHT * 0.5 - 40, 150, 0, NULL, var0, gui_MakeUnsignedCharVar(0), gui_MakeUnsignedCharVar(255));
 	selected_light_r->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
 	
	selected_light_g = gui_AddSlider(light_properties_window, "Green", 0, LIGHT_PROPERTIES_WINDOW_HEIGHT * 0.5 - 80, 150, 0, NULL, var1, gui_MakeUnsignedCharVar(0), gui_MakeUnsignedCharVar(255));
	selected_light_g->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	selected_light_b = gui_AddSlider(light_properties_window, "Blue", 0, LIGHT_PROPERTIES_WINDOW_HEIGHT * 0.5 - 120, 150, 0, NULL, var2, gui_MakeUnsignedCharVar(0), gui_MakeUnsignedCharVar(255));
	selected_light_b->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	selected_light_radius = gui_AddSlider(light_properties_window, "Radius", 0, LIGHT_PROPERTIES_WINDOW_HEIGHT * 0.5 - 160, 150, 0, NULL, radius_var, gui_MakeUnsignedShortVar(0), gui_MakeUnsignedShortVar(0xffff));
	selected_light_radius->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	selected_light_energy = gui_AddSlider(light_properties_window, "Energy", 0, LIGHT_PROPERTIES_WINDOW_HEIGHT * 0.5 - 200, 150, 0, NULL, energy_var, gui_MakeUnsignedShortVar(0), gui_MakeUnsignedShortVar(0xffff));
	selected_light_energy->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;


	
	
	/*
	
	
	materials_window = gui_CreateWidget("Materials", r_window_width * 0.5 - MATERIAL_WINDOW_WIDTH * 0.5, -r_window_height * 0.5 + MATERIAL_WINDOW_HEIGHT * 0.5, MATERIAL_WINDOW_WIDTH, MATERIAL_WINDOW_HEIGHT);
	materials_window->bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	material_list = gui_AddOptionList(materials_window, "material list", 0, MATERIAL_WINDOW_HEIGHT * 0.5 - OPTION_HEIGHT * 0.5, MATERIAL_WINDOW_WIDTH, 0, material_window_callback);
	edit_material_create_material = gui_AddButton(materials_window, "Create material", 0, -100, MATERIAL_WINDOW_WIDTH - 20, EDIT_MATERIAL_WINDOW_DELETE_MATERIAL_BUTTON_HEIGHT, 0, material_window_create_material_callback);
	
	editor_CloseMaterialWindow();
	
	gui_var_t *material_name = gui_CreateVar("material name", GUI_VAR_ALLOCD_STRING, NULL, NULL, 0);
	gui_var_t *material_red = gui_CreateVar("material red", GUI_VAR_UNSIGNED_CHAR, NULL, NULL, 0);
	gui_var_t *material_green = gui_CreateVar("material green", GUI_VAR_UNSIGNED_CHAR, NULL, NULL, 0);
	gui_var_t *material_blue = gui_CreateVar("material blue", GUI_VAR_UNSIGNED_CHAR, NULL, NULL, 0);
	gui_var_t *material_diffuse_texture = gui_CreateVar("material diffuse texture", GUI_VAR_STRING, NULL, NULL, 0);
	gui_var_t *material_normal_texture = gui_CreateVar("material normal texture", GUI_VAR_STRING, NULL, NULL, 0);
	
	
	edit_material_window = gui_CreateWidget("Edit material", r_window_width * 0.5 - MATERIAL_WINDOW_WIDTH * 0.5, -r_window_height * 0.5 + MATERIAL_WINDOW_HEIGHT * 0.5, MATERIAL_WINDOW_WIDTH, MATERIAL_WINDOW_HEIGHT);
	edit_material_window->bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	edit_material_done_button = gui_AddButton(edit_material_window, "Done", -MATERIAL_WINDOW_WIDTH * 0.5 + EDIT_MATERIAL_WINDOW_RETURN_BUTTON_WIDTH * 0.5, -MATERIAL_WINDOW_HEIGHT * 0.5 + EDIT_MATERIAL_WINDOW_RETURN_BUTTON_HEIGHT * 0.5, EDIT_MATERIAL_WINDOW_RETURN_BUTTON_WIDTH, EDIT_MATERIAL_WINDOW_RETURN_BUTTON_HEIGHT, 0, edit_material_window_callback);
	edit_material_delete_material_button = gui_AddButton(edit_material_window, "Delete material", 0, -50, MATERIAL_WINDOW_WIDTH - 20, EDIT_MATERIAL_WINDOW_DELETE_MATERIAL_BUTTON_HEIGHT, 0, edit_material_window_callback);
	
	
	edit_material_material_name_text_field = gui_AddTextField(edit_material_window, "Material name", 0, MATERIAL_WINDOW_HEIGHT * 0.5 - OPTION_HEIGHT * 0.5 - 20, MATERIAL_WINDOW_WIDTH - 10, 0, edit_material_text_field_callback);
	edit_material_material_name_text_field->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	edit_material_material_red = gui_AddTextField(edit_material_window, "Red", -MATERIAL_WINDOW_WIDTH * 0.5 + 55, MATERIAL_WINDOW_HEIGHT * 0.5 - OPTION_HEIGHT * 0.5 - 60, 100, 0, NULL);
	edit_material_material_red->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	edit_material_material_green = gui_AddTextField(edit_material_window, "Green", -MATERIAL_WINDOW_WIDTH * 0.5 + 55, MATERIAL_WINDOW_HEIGHT * 0.5 - OPTION_HEIGHT * 0.5 - 100, 100, 0, NULL);
	edit_material_material_green->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	edit_material_material_blue = gui_AddTextField(edit_material_window, "Blue", -MATERIAL_WINDOW_WIDTH * 0.5 + 55, MATERIAL_WINDOW_HEIGHT * 0.5 - OPTION_HEIGHT * 0.5 - 140, 100, 0, NULL);
	edit_material_material_blue->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	edit_material_diffuse_texture_dropdown = gui_AddDropdown(edit_material_window, "Diffuse texture", "Diffuse texture", 50, 80, 200, 0, NULL);
	edit_material_diffuse_texture_dropdown->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	edit_material_diffuse_texture_list = gui_AddOptionList((widget_t *)edit_material_diffuse_texture_dropdown, "diffuse texture list", 0, -OPTION_HEIGHT * 0.5, 200, 0, set_material_diffuse_texture_callback);
	
	
	edit_material_normal_texture_dropdown = gui_AddDropdown(edit_material_window, "Normal texture", "Normal texture", 50, 30, 200, 0, NULL);
	edit_material_normal_texture_dropdown->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	edit_material_normal_texture_list = gui_AddOptionList((widget_t *)edit_material_normal_texture_dropdown, "normal texture list", 0, -OPTION_HEIGHT * 0.5, 200, 0, set_material_normal_texture_callback);
	
	editor_CloseEditMaterialWindow();
	
	gui_TrackVar(material_name, (widget_t *)edit_material_material_name_text_field);
	gui_TrackVar(material_red, (widget_t *)edit_material_material_red);
	gui_TrackVar(material_green, (widget_t *)edit_material_material_green);
	gui_TrackVar(material_blue, (widget_t *)edit_material_material_blue);
	gui_TrackVar(material_diffuse_texture, (widget_t *)edit_material_diffuse_texture_dropdown);
	gui_TrackVar(material_normal_texture, (widget_t *)edit_material_normal_texture_dropdown);*/
	
	
	/*option_list_t *test_options = gui_AddOptionList(NULL, "test", 0, 0, 200, 0, 6, NULL);
	gui_AddOptionToList(test_options, "wow0", "wow0");
	gui_AddOptionToList(test_options, "wow1", "wow1");
	gui_AddOptionToList(test_options, "wow2", "wow2");
	gui_AddOptionToList(test_options, "wow3", "wow3");
	gui_AddOptionToList(test_options, "wow4", "wow4");
	gui_AddOptionToList(test_options, "wow5", "wow5");
	gui_AddOptionToList(test_options, "wow6", "wow6");
	gui_AddOptionToList(test_options, "wow7", "wow7");
	gui_AddOptionToList(test_options, "wow8", "wow8");
	gui_AddOptionToList(test_options, "wow9", "wow9");
	gui_AddOptionToList(test_options, "wow10", "wow10");
	gui_AddOptionToList(test_options, "wow11", "wow11");
	gui_AddOptionToList(test_options, "wow12", "wow12");
	gui_AddOptionToList(test_options, "wow13", "wow13");
	gui_AddOptionToList(test_options, "wow14", "wow14");
	gui_AddOptionToList(test_options, "wow15", "wow15");
	gui_AddOptionToList(test_options, "wow16", "wow16");
	gui_AddOptionToList(test_options, "wow17", "wow17");*/
	
	
	/*item_list_t *test_list = gui_AddItemList(NULL, "test item list", 0, 0, 800, 600, ITEM_LIST_HORIZONTAL_ORDER, NULL);
	wsurface_t *surface;
	widget_t *widget;
	
	for(i = 0; i < 40; i++)
	{
		widget = gui_CreateWidget("test item", 0, 0, 100, 100);
		surface = gui_AddSurface(widget, "test surface", 0, 0, 80, 80, 0, NULL);
		gui_ClearSurface(surface);
		
		gui_AddItemToList(widget, test_list);
	}*/
	
	
	
	editor_InitBrushUI();
	editor_InitMaterialUI();
	editor_InitTextureUI();	
	editor_InitExplorerUI();
	editor_InitEntityUI();
	
	
	editor_CloseLightPropertiesWindow();
	editor_CloseBrushPropertiesWindow();
	//editor_CloseBrushFacePropertiesWindow();
	
}

void editor_FinishUI()
{
	
}

void editor_ProcessUI()
{
	menu_bar->y = r_window_height * 0.5 - MENU_BAR_HEIGHT * 0.5;
	menu_bar->w = r_window_width;
		
	menu_dropdown_bar->widget.w = r_window_width * 0.5;
	menu_dropdown_bar->bm_flags |= WIDGET_BAR_ADJUST_WIDGETS;

	handle_3d_mode_display->widget.x = -r_window_width * 0.5 + HANDLE_3D_MODE_DISPLAY_WIDTH * 0.5; 
	handle_3d_mode_display->widget.y = -r_window_height * 0.5 + OPTION_HEIGHT * 0.5;
	
	snap_value_dropdown->widget.x = -r_window_width * 0.5 + HANDLE_3D_MODE_DISPLAY_WIDTH + SNAP_VALUE_DROPDOWN_WIDTH * 0.5;
	snap_value_dropdown->widget.y = -r_window_height * 0.5 + OPTION_HEIGHT * 0.5;
	
	editor_UpdateEntityUI();
	editor_UpdateTextureUI();
	editor_UpdateMaterialUI();
	editor_UpdateExplorerUI();
		
}

void editor_HideUI()
{

}

void editor_ShowUI()
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

void editor_OpenLightPropertiesWindow(int light_index)
{
	if(light_properties_window)
	{
		gui_SetVisible(light_properties_window);
		
		/*selected_light_r->widget.var->addr = &light_params[light_index].r;
		selected_light_g->widget.var->addr = &light_params[light_index].g;
		selected_light_b->widget.var->addr = &light_params[light_index].b;
		selected_light_radius->widget.var->addr = &light_params[light_index].radius;
		selected_light_energy->widget.var->addr = &light_params[light_index].energy;*/
		
		selected_light_r->widget.var->addr = &ed_selected_light_params->r;
		selected_light_g->widget.var->addr = &ed_selected_light_params->g;
		selected_light_b->widget.var->addr = &ed_selected_light_params->b;
		selected_light_radius->widget.var->addr = &ed_selected_light_params->radius;
		selected_light_energy->widget.var->addr = &ed_selected_light_params->energy;
		
		ed_light_properties_window_open = 1;
	}
		
}

void editor_CloseLightPropertiesWindow()
{
	if(light_properties_window)
	{
		gui_SetInvisible(light_properties_window);
		
		selected_light_r->widget.var->addr = NULL;
		selected_light_g->widget.var->addr = NULL;
		selected_light_b->widget.var->addr = NULL;
		selected_light_radius->widget.var->addr = NULL;
		selected_light_energy->widget.var->addr = NULL;
		
		ed_light_properties_window_open = 0;
	}
		
}


/*void editor_OpenMaterialWindow()
{
	if(materials_window)
	{
		editor_CloseEditMaterialWindow();
		editor_EnumerateMaterials();
		gui_SetVisible(materials_window);
	}
		
}*/

/*void editor_CloseMaterialWindow()
{
	if(materials_window)
		gui_SetInvisible(materials_window);
}*/
/*
void editor_ToggleMaterialWindow()
{
	if(materials_window->bm_flags & WIDGET_INVISIBLE)
	{
		editor_OpenMaterialWindow();
	}
	else
	{
		editor_CloseMaterialWindow();
	}
}

void editor_EnumerateMaterials()
{
	int i;
	gui_RemoveAllOptions(material_list);
	option_t *option;
	
	for(i = 0; i < material_count; i++)
	{
		if(materials[i].material_index < 0)
			continue;
			
		option = gui_AddOptionToList(material_list, material_names[i], material_names[i]);
		
		option->widget.data = (void *)i;
	}
}

void editor_OpenEditMaterialWindow(int material_index)
{
	int i;
	option_t *option;
	
	if(edit_material_window)
	{
		ed_edit_material_index = material_index;
		
		gui_SetVisible(edit_material_window);
		
		gui_RemoveAllOptions(edit_material_diffuse_texture_list);
		gui_RemoveAllOptions(edit_material_normal_texture_list);
		
		edit_material_material_name_text_field->widget.var->addr = &material_names[material_index];
		edit_material_material_red->widget.var->addr = &materials[material_index].r;
		edit_material_material_green->widget.var->addr = &materials[material_index].g;
		edit_material_material_blue->widget.var->addr = &materials[material_index].b;
		
		if(materials[material_index].diffuse_texture > -1)
		{
			edit_material_diffuse_texture_dropdown->widget.var->addr = &texture_info[materials[material_index].diffuse_texture].name;
		}
		else
		{
			edit_material_diffuse_texture_dropdown->widget.var->addr = &no_tex;
		}
		
		
		if(materials[material_index].normal_texture > -1)
		{
			edit_material_normal_texture_dropdown->widget.var->addr = &texture_info[materials[material_index].normal_texture].name;
		}
		else
		{
			edit_material_normal_texture_dropdown->widget.var->addr = &no_tex;
		}
		
		
		option = gui_AddOptionToList(edit_material_diffuse_texture_list, "None", "None");
		option->widget.data = (void *) -1;
		
		option = gui_AddOptionToList(edit_material_normal_texture_list, "None", "None");
		option->widget.data = (void *) -1;
		
		for(i = 0; i < texture_count; i++)
		{
			
			if(!textures[i].gl_handle)
				continue;
			
			option = gui_AddOptionToList(edit_material_diffuse_texture_list, texture_info[i].name, texture_info[i].name);
			option->widget.data = (void *)i;
			
			option = gui_AddOptionToList(edit_material_normal_texture_list, texture_info[i].name, texture_info[i].name);
			option->widget.data = (void *)i;
		}
		
		
		
		
		//gui_SetText((widget_t *)edit_material_material_name_text_field, material_names[material_index]);
	}
}

void editor_CloseEditMaterialWindow()
{
	if(edit_material_window)
	{
		gui_SetInvisible(edit_material_window);
	}
}*/

void editor_UIWindowResizeCallback()
{
	gui_DestroyWidget(menu_bar);
	gui_UpdateGUIProjectionMatrix();
	
	menu_bar = gui_CreateWidget("menu_bar", 0, r_window_height * 0.5 - MENU_BAR_HEIGHT * 0.5, r_window_width, MENU_BAR_HEIGHT, sizeof(widget_t ));	
	
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






