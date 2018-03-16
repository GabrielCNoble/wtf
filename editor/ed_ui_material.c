#include "ed_ui_material.h"
#include "ed_ui.h"
#include "ed_ui_explorer.h"

#include "gui.h"
#include "material.h"
#include "texture.h"


widget_t *materials_window = NULL;
option_list_t *material_list = NULL;


int ed_cur_editing_material_index = 0;
int ed_edit_material_index = 0;
widget_t *edit_material_window = NULL;
text_field_t *edit_material_window_material_name_text_field = NULL;
text_field_t *edit_material_window_material_red = NULL;
text_field_t *edit_material_window_material_green = NULL;
text_field_t *edit_material_window_material_blue = NULL;
dropdown_t *edit_material_window_diffuse_texture_dropdown = NULL;
dropdown_t *edit_material_window_normal_texture_dropdown = NULL;
option_list_t *edit_material_window_diffuse_texture_list = NULL;
button_t *edit_material_window_load_diffuse_texture_button = NULL;
option_list_t *edit_material_window_normal_texture_list = NULL;
button_t *edit_material_window_load_normal_texture_button = NULL;
button_t *edit_material_window_done_button = NULL;
button_t *edit_material_window_delete_material_button = NULL;
button_t *edit_material_window_create_material_button = NULL;
 

/* from material.c */
extern int material_count;
extern material_t *materials;
extern char **material_names;


/* from texture.c */
extern int texture_count;
extern texture_t *textures;
extern texture_info_t *texture_info;


/* from r_main.c */
extern int forward_pass_shader;
extern int r_window_width;
extern int r_window_height;

static char *no_tex = "None";

extern char *ed_explorer_selected_file;



//void material_window_callback(widget_t *widget)
void editor_MaterialWindowCallback(widget_t *widget)
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
	
}


//void material_window_create_material_callback(widget_t *widget)
void editor_MaterialWindowCreateMaterialButtonCallback(widget_t *widget)
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


//void edit_material_text_field_callback(widget_t *widget)
void editor_EditMaterialWindowMaterialNameTextFieldCallback(widget_t *widget)
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
}



//void edit_material_window_callback(widget_t *widget)
void editor_EditMaterialWindowCallback(widget_t *widget)
{
	button_t *button;	
	if(widget->type == WIDGET_BUTTON)
	{
		if(widget == (widget_t *)edit_material_window_delete_material_button)
		{
			material_DestroyMaterialIndex(ed_edit_material_index);
		}
		editor_OpenMaterialWindow();
	}
}


//void set_material_diffuse_texture_callback(widget_t *widget)
void editor_EditMaterialWindowSetMaterialDiffuseTextureCallback(widget_t *widget)
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
			edit_material_window_diffuse_texture_dropdown->widget.var->addr = &texture_info[texture_index].name;
		}
		else
		{
			edit_material_window_diffuse_texture_dropdown->widget.var->addr = &no_tex;
		}
		
	}
	
}



void editor_EditMaterialWindowSetMaterialNormalTextureCallback(widget_t *widget)
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
			edit_material_window_normal_texture_dropdown->widget.var->addr = &texture_info[texture_index].name;
		}
		else
		{
			edit_material_window_normal_texture_dropdown->widget.var->addr = &no_tex;
		}
	}
	
}



void editor_EditMaterialWindowLoadMaterialDiffuseTextureButtonCallback(widget_t *widget)
{
	editor_OpenEditMaterialWindowDiffuseTextureExplorer();
}



void editor_EditMaterialWindowLoadMaterialNormalTextureButtonCallback(widget_t *widget)
{
	editor_OpenEditMaterialWindowNormalTextureExplorer();
}



void editor_InitMaterialUI()
{
	//materials_window = gui_CreateWidget("Materials", r_window_width * 0.5 - MATERIAL_WINDOW_WIDTH * 0.5, -r_window_height * 0.5 + MATERIAL_WINDOW_HEIGHT * 0.5, MATERIAL_WINDOW_WIDTH, MATERIAL_WINDOW_HEIGHT);
	materials_window = gui_AddWidget(NULL, "Materials", r_window_width * 0.5 - MATERIAL_WINDOW_WIDTH * 0.5, -r_window_height * 0.5 + MATERIAL_WINDOW_HEIGHT * 0.5, MATERIAL_WINDOW_WIDTH, MATERIAL_WINDOW_HEIGHT);
	materials_window->bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	material_list = gui_AddOptionList(materials_window, "material list", 0, MATERIAL_WINDOW_HEIGHT * 0.5 - OPTION_HEIGHT * 0.5, MATERIAL_WINDOW_WIDTH, 0, 8, editor_MaterialWindowCallback);
	edit_material_window_create_material_button = gui_AddButton(materials_window, "Create material", 0, -100, MATERIAL_WINDOW_WIDTH - 20, EDIT_MATERIAL_WINDOW_DELETE_MATERIAL_BUTTON_HEIGHT, 0, editor_MaterialWindowCreateMaterialButtonCallback);
	
	editor_CloseMaterialWindow();
	
	gui_var_t *material_name = gui_CreateVar("material name", GUI_VAR_ALLOCD_STRING, NULL, NULL, 0);
	gui_var_t *material_red = gui_CreateVar("material red", GUI_VAR_UNSIGNED_CHAR, NULL, NULL, 0);
	gui_var_t *material_green = gui_CreateVar("material green", GUI_VAR_UNSIGNED_CHAR, NULL, NULL, 0);
	gui_var_t *material_blue = gui_CreateVar("material blue", GUI_VAR_UNSIGNED_CHAR, NULL, NULL, 0);
	gui_var_t *material_diffuse_texture = gui_CreateVar("material diffuse texture", GUI_VAR_STRING, NULL, NULL, 0);
	gui_var_t *material_normal_texture = gui_CreateVar("material normal texture", GUI_VAR_STRING, NULL, NULL, 0);
	
	
	//edit_material_window = gui_CreateWidget("Edit material", r_window_width * 0.5 - MATERIAL_WINDOW_WIDTH * 0.5, -r_window_height * 0.5 + MATERIAL_WINDOW_HEIGHT * 0.5, MATERIAL_WINDOW_WIDTH, MATERIAL_WINDOW_HEIGHT);
	edit_material_window = gui_AddWidget(NULL, "Edit material", r_window_width * 0.5 - MATERIAL_WINDOW_WIDTH * 0.5, -r_window_height * 0.5 + MATERIAL_WINDOW_HEIGHT * 0.5, MATERIAL_WINDOW_WIDTH, MATERIAL_WINDOW_HEIGHT);
	edit_material_window->bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	edit_material_window_done_button = gui_AddButton(edit_material_window, "Done", -MATERIAL_WINDOW_WIDTH * 0.5 + EDIT_MATERIAL_WINDOW_RETURN_BUTTON_WIDTH * 0.5, -MATERIAL_WINDOW_HEIGHT * 0.5 + EDIT_MATERIAL_WINDOW_RETURN_BUTTON_HEIGHT * 0.5, EDIT_MATERIAL_WINDOW_RETURN_BUTTON_WIDTH, EDIT_MATERIAL_WINDOW_RETURN_BUTTON_HEIGHT, 0, editor_EditMaterialWindowCallback);
	edit_material_window_delete_material_button = gui_AddButton(edit_material_window, "Delete material", 0, -50, MATERIAL_WINDOW_WIDTH - 20, EDIT_MATERIAL_WINDOW_DELETE_MATERIAL_BUTTON_HEIGHT, 0, editor_EditMaterialWindowCallback);
	
	
	edit_material_window_material_name_text_field = gui_AddTextField(edit_material_window, "Material name", 0, MATERIAL_WINDOW_HEIGHT * 0.5 - OPTION_HEIGHT * 0.5 - 20, MATERIAL_WINDOW_WIDTH - 10, 0, editor_EditMaterialWindowMaterialNameTextFieldCallback);
	edit_material_window_material_name_text_field->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	edit_material_window_material_red = gui_AddTextField(edit_material_window, "Red", -MATERIAL_WINDOW_WIDTH * 0.5 + 55, MATERIAL_WINDOW_HEIGHT * 0.5 - OPTION_HEIGHT * 0.5 - 60, 100, 0, NULL);
	edit_material_window_material_red->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	edit_material_window_material_green = gui_AddTextField(edit_material_window, "Green", -MATERIAL_WINDOW_WIDTH * 0.5 + 55, MATERIAL_WINDOW_HEIGHT * 0.5 - OPTION_HEIGHT * 0.5 - 100, 100, 0, NULL);
	edit_material_window_material_green->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	edit_material_window_material_blue = gui_AddTextField(edit_material_window, "Blue", -MATERIAL_WINDOW_WIDTH * 0.5 + 55, MATERIAL_WINDOW_HEIGHT * 0.5 - OPTION_HEIGHT * 0.5 - 140, 100, 0, NULL);
	edit_material_window_material_blue->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	edit_material_window_diffuse_texture_dropdown = gui_AddDropdown(edit_material_window, "Diffuse texture", "Diffuse texture", 100, 80, 200, 0, NULL);
	edit_material_window_diffuse_texture_dropdown->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	edit_material_window_diffuse_texture_list = gui_AddOptionList((widget_t *)edit_material_window_diffuse_texture_dropdown, "diffuse texture list", 0, -OPTION_HEIGHT * 0.5, 200, 0, 8, editor_EditMaterialWindowSetMaterialDiffuseTextureCallback);
	
	
	edit_material_window_load_diffuse_texture_button = gui_AddButton(edit_material_window, "Load", -20, 80, 40, OPTION_HEIGHT, 0, editor_EditMaterialWindowLoadMaterialDiffuseTextureButtonCallback);
	//edit_material_window_load_diffuse_texture_button->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	
	edit_material_window_normal_texture_dropdown = gui_AddDropdown(edit_material_window, "Normal texture", "Normal texture", 100, 30, 200, 0, NULL);
	edit_material_window_normal_texture_dropdown->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	edit_material_window_normal_texture_list = gui_AddOptionList((widget_t *)edit_material_window_normal_texture_dropdown, "normal texture list", 0, -OPTION_HEIGHT * 0.5, 200, 0, 8, editor_EditMaterialWindowSetMaterialNormalTextureCallback);
	
	
	edit_material_window_load_normal_texture_button = gui_AddButton(edit_material_window, "Load", -20, 30, 40, OPTION_HEIGHT, 0, editor_EditMaterialWindowLoadMaterialNormalTextureButtonCallback);
	//edit_material_window_load_normal_texture_button->widget.bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	
	editor_CloseEditMaterialWindow();
	
	gui_TrackVar(material_name, (widget_t *)edit_material_window_material_name_text_field);
	gui_TrackVar(material_red, (widget_t *)edit_material_window_material_red);
	gui_TrackVar(material_green, (widget_t *)edit_material_window_material_green);
	gui_TrackVar(material_blue, (widget_t *)edit_material_window_material_blue);
	gui_TrackVar(material_diffuse_texture, (widget_t *)edit_material_window_diffuse_texture_dropdown);
	gui_TrackVar(material_normal_texture, (widget_t *)edit_material_window_normal_texture_dropdown);
}

void editor_OpenMaterialWindow()
{
	if(materials_window)
	{
		editor_CloseEditMaterialWindow();
		editor_EnumerateMaterials();
		gui_SetVisible(materials_window);
	}
}


void editor_CloseMaterialWindow()
{
	if(materials_window)
		gui_SetInvisible(materials_window);
}


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
	
	for(i = -1; i < material_count; i++)
	{
		if(materials[i].flags & MATERIAL_INVALID)
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
		
		gui_RemoveAllOptions(edit_material_window_diffuse_texture_list);
		gui_RemoveAllOptions(edit_material_window_normal_texture_list);
		
		edit_material_window_material_name_text_field->widget.var->addr = &material_names[material_index];
		edit_material_window_material_red->widget.var->addr = &materials[material_index].r;
		edit_material_window_material_green->widget.var->addr = &materials[material_index].g;
		edit_material_window_material_blue->widget.var->addr = &materials[material_index].b;
		
		if(materials[material_index].diffuse_texture > -1)
		{
			edit_material_window_diffuse_texture_dropdown->widget.var->addr = &texture_info[materials[material_index].diffuse_texture].name;
		}
		else
		{
			edit_material_window_diffuse_texture_dropdown->widget.var->addr = &no_tex;
		}
		
		
		if(materials[material_index].normal_texture > -1)
		{
			edit_material_window_normal_texture_dropdown->widget.var->addr = &texture_info[materials[material_index].normal_texture].name;
		}
		else
		{
			edit_material_window_normal_texture_dropdown->widget.var->addr = &no_tex;
		}
		
		
		option = gui_AddOptionToList(edit_material_window_diffuse_texture_list, "None", "None");
		option->widget.data = (void *) -1;
		
		option = gui_AddOptionToList(edit_material_window_normal_texture_list, "None", "None");
		option->widget.data = (void *) -1;
		
		for(i = 0; i < texture_count; i++)
		{
			
			if(!textures[i].gl_handle)
				continue;
			
			option = gui_AddOptionToList(edit_material_window_diffuse_texture_list, texture_info[i].name, texture_info[i].name);
			option->widget.data = (void *)i;
			
			option = gui_AddOptionToList(edit_material_window_normal_texture_list, texture_info[i].name, texture_info[i].name);
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
}
 
void editor_EditMaterialWindowDiffuseTextureFileClickCallback()
{
	int texture_index;
	material_t *material;
	
	texture_index = texture_LoadTexture(ed_explorer_selected_file, path_GetFileNameFromPath(ed_explorer_selected_file), 0);
	editor_CloseEditMaterialWindowTextureExplorer();
	
	if(texture_index >= 0)
	{
		material = &materials[ed_edit_material_index];
		material->diffuse_texture = texture_index;
		editor_OpenEditMaterialWindow(ed_edit_material_index);
	}
	
}


void editor_EditMaterialWindowNormalTextureFileClickCallback()
{
	int texture_index;
	material_t *material;
	
	texture_index = texture_LoadTexture(ed_explorer_selected_file, path_GetFileNameFromPath(ed_explorer_selected_file), 0);
	editor_CloseEditMaterialWindowTextureExplorer();
	
	if(texture_index >= 0)
	{
		material = &materials[ed_edit_material_index];
		material->normal_texture = texture_index;
		editor_OpenEditMaterialWindow(ed_edit_material_index);
	}
	
}


void editor_OpenEditMaterialWindowDiffuseTextureExplorer()
{
	editor_OpenTextureExplorer(NULL);
	editor_SetExplorerFileClickCallback(editor_EditMaterialWindowDiffuseTextureFileClickCallback);
	editor_ExplorerMultiFileSelection(0);
}


void editor_OpenEditMaterialWindowNormalTextureExplorer()
{
	editor_OpenTextureExplorer(NULL);
	editor_SetExplorerFileClickCallback(editor_EditMaterialWindowNormalTextureFileClickCallback);
	editor_ExplorerMultiFileSelection(0);
	//editor_OpenExplorerWindow("C:/Users", EXPLORER_OPEN_TEXTURE_FILE);
}

void editor_CloseEditMaterialWindowTextureExplorer()
{
	editor_CloseExplorerWindow();
}

void editor_UpdateMaterialUI()
{
	materials_window->x = r_window_width * 0.5 - MATERIAL_WINDOW_WIDTH * 0.5;
	materials_window->y = -r_window_height * 0.5 + MATERIAL_WINDOW_HEIGHT * 0.5;
	
	edit_material_window->x = r_window_width * 0.5 - MATERIAL_WINDOW_WIDTH * 0.5;
	edit_material_window->y = -r_window_height * 0.5 + MATERIAL_WINDOW_HEIGHT * 0.5;
}







