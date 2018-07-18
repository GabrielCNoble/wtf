
#include "..\ed_ui.h"
#include "ed_level_ui_texture.h"
#include "..\ed_ui_explorer.h"
#include "..\..\common\gui.h"
#include "..\..\common\texture.h"
#include "..\ed_proj.h"


int ed_texture_window_open = 0;
widget_t *texture_window = NULL;
option_list_t *texture_window_texture_list = NULL;
button_t *texture_window_load_texture_button = NULL;
button_t *texture_window_delete_texture_button = NULL;


int ed_texture_properties_window_open = 0; 
widget_t *texture_properties_window = NULL;
text_field_t *texture_properties_window_texture_name_text_field = NULL;
checkbox_t *texture_properties_window_invert_texture_x_checkbox = NULL;
checkbox_t *texture_properties_window_invert_texture_y_checkbox = NULL;
checkbox_t *texture_properties_window_save_texture_to_project_checkbox = NULL;

button_t *texture_properties_window_destroy_texture_button = NULL;
wsurface_t *texture_properties_window_texture_preview_surface = NULL;


dropdown_t *texture_properties_window_wrap_s_dropdown = NULL;
option_list_t *texture_properties_window_wrap_s_options = NULL;
dropdown_t *texture_properties_window_wrap_t_dropdown = NULL;
option_list_t *texture_properties_window_wrap_t_options = NULL;


#define TEXTURE_PROPERTIES_WINDOW_DESTROY_TEXTURE_BUTTON_HEIGHT 50 


 


int ed_texture_properties_window_current_texture_index = -1;


/* from r_main.c */
extern int r_window_width;
extern int r_window_height;


extern int tex_texture_count;
extern texture_t *tex_textures;
extern texture_info_t *tex_texture_info;


extern char *ed_explorer_selected_file;


void editor_TextureWindowLoadTextureButtonCallback(widget_t *widget)
{
	if(widget->type == WIDGET_BUTTON)
	{
		editor_OpenTextureExplorer();
	}
}

void editor_TextureWindowTextureListCallback(widget_t *widget)
{
	option_t *option;
	int texture_index;
	
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		texture_index = (int)option->widget.data;
		
		editor_CloseTextureWindow();
		editor_OpenTexturePropertiesWindow(texture_index);
	}
}

void editor_TexturePropertiesWindowDestroyTextureButtonCallback(widget_t *widget)
{
	texture_DeleteTextureByIndex(ed_texture_properties_window_current_texture_index);
	ed_texture_properties_window_current_texture_index = -1;
	editor_CloseTexturePropertiesWindow();
	editor_OpenTextureWindow();
}

void editor_InitTextureUI()
{
	//texture_window = gui_CreateWidget("Textures", -r_window_width * 0.5 + TEXTURE_WINDOW_WIDTH * 0.5, -r_window_height * 0.5 + TEXTURE_WINDOW_HEIGHT * 0.5 + 20, TEXTURE_WINDOW_WIDTH, TEXTURE_WINDOW_HEIGHT);
	texture_window = gui_AddWidget(NULL, "Textures", -r_window_width * 0.5 + TEXTURE_WINDOW_WIDTH * 0.5, -r_window_height * 0.5 + TEXTURE_WINDOW_HEIGHT * 0.5 + 20, TEXTURE_WINDOW_WIDTH, TEXTURE_WINDOW_HEIGHT);
	texture_window->bm_flags |= WIDGET_SHOW_NAME | WIDGET_RENDER_NAME;
	texture_window_texture_list = gui_AddOptionList(texture_window, "texture list", 0, TEXTURE_WINDOW_HEIGHT * 0.5 - 20, TEXTURE_WINDOW_WIDTH - 10, 0, 8, editor_TextureWindowTextureListCallback);
	texture_window_load_texture_button = gui_AddButton(texture_window, "Load texture", 0, -70, TEXTURE_WINDOW_WIDTH - 10, 40, 0, editor_TextureWindowLoadTextureButtonCallback);
	editor_CloseTextureWindow();
	
	
	
	
	texture_properties_window = gui_AddWidget(NULL, "Texture properties", -r_window_width * 0.5 + TEXTURE_WINDOW_WIDTH * 0.5, -r_window_height * 0.5 + TEXTURE_WINDOW_HEIGHT * 0.5 + 20, TEXTURE_WINDOW_WIDTH, TEXTURE_WINDOW_HEIGHT);
	texture_properties_window->bm_flags |= WIDGET_SHOW_NAME | WIDGET_RENDER_NAME;
	texture_properties_window_texture_name_text_field = gui_AddTextField(texture_properties_window, "Texture name", 0, TEXTURE_WINDOW_HEIGHT * 0.5 - 20, TEXTURE_WINDOW_WIDTH - 10, 0, NULL);
	button_t *texture_properties_window_destroy_texture_button = gui_AddButton(texture_properties_window, "Delete texture", 0, -TEXTURE_WINDOW_HEIGHT * 0.5 + TEXTURE_PROPERTIES_WINDOW_DESTROY_TEXTURE_BUTTON_HEIGHT * 0.5 + 10, TEXTURE_WINDOW_WIDTH - 10, TEXTURE_PROPERTIES_WINDOW_DESTROY_TEXTURE_BUTTON_HEIGHT, 0, editor_TexturePropertiesWindowDestroyTextureButtonCallback);
	
	texture_properties_window_invert_texture_x_checkbox = gui_AddCheckbox(texture_properties_window, -30, 0, 20, 20, 0, NULL);
	texture_properties_window_invert_texture_x_checkbox->bm_check_flags = TEXTURE_INVERT_X;
	texture_properties_window_invert_texture_y_checkbox = gui_AddCheckbox(texture_properties_window, 30, 0, 20, 20, 0, NULL);
	texture_properties_window_invert_texture_y_checkbox->bm_check_flags = TEXTURE_INVERT_Y;
	
	
	
	gui_var_t *var = gui_CreateVar("current editing texture name", GUI_VAR_STRING, NULL, NULL, 0);
	gui_TrackVar(var, (widget_t *)texture_properties_window_texture_name_text_field);
	
	gui_var_t *x_flag = gui_CreateVar("texture invert x flag", GUI_VAR_INT, NULL, NULL, 0);
	gui_var_t *y_flag = gui_CreateVar("texture invert y flag", GUI_VAR_INT, NULL, NULL, 0);
	
	gui_TrackVar(x_flag, (widget_t *)texture_properties_window_invert_texture_x_checkbox);
	gui_TrackVar(y_flag, (widget_t *)texture_properties_window_invert_texture_y_checkbox);
	
	editor_CloseTexturePropertiesWindow();
}

void editor_OpenTextureWindow()
{
	if(texture_window)
	{
		editor_CloseTexturePropertiesWindow();
		editor_EnumerateTextures();
		gui_SetVisible(texture_window);
		ed_texture_window_open = 1;
	}
}

void editor_CloseTextureWindow()
{
	if(texture_window)
	{
		editor_CloseTexturePropertiesWindow();
		gui_SetInvisible(texture_window);
		ed_texture_window_open = 0;
	}
}

void editor_ToggleTextureWindow()
{
	if(ed_texture_window_open)
	{
		editor_CloseTextureWindow();
	}
	else
	{
		editor_OpenTextureWindow();
	}
}

void editor_EnumerateTextures()
{
	int i;
	option_t *option;
	gui_RemoveAllOptions(texture_window_texture_list);
	
	for(i = 0; i < tex_texture_count; i++)
	{
		if(!tex_textures[i].gl_handle)
			continue;
		
		option = gui_AddOptionToList(texture_window_texture_list, tex_texture_info[i].name, tex_texture_info[i].name);
		option->widget.data = (void *)i;
	}
}

void editor_OpenTexturePropertiesWindow(int texture_index)
{
	if(texture_properties_window)
	{
		gui_SetVisible(texture_properties_window);
		ed_texture_properties_window_current_texture_index = texture_index;
		ed_texture_properties_window_open = 1;
		texture_properties_window_texture_name_text_field->widget.var->addr = &tex_texture_info[texture_index].name;
		texture_properties_window_invert_texture_x_checkbox->widget.var->addr = &tex_textures[texture_index].bm_flags;
		texture_properties_window_invert_texture_y_checkbox->widget.var->addr = &tex_textures[texture_index].bm_flags;
		
		//editor_CloseTextureWindow();
	}
}

void editor_CloseTexturePropertiesWindow()
{
	if(texture_properties_window)
	{
		gui_SetInvisible(texture_properties_window);
		ed_texture_properties_window_open = 0;
		texture_properties_window_texture_name_text_field->widget.var->addr = NULL;
		texture_properties_window_invert_texture_x_checkbox->widget.var->addr = NULL;
		texture_properties_window_invert_texture_y_checkbox->widget.var->addr = NULL;
		
		//editor_OpenTextureWindow();
	}
}
 
void editor_OpenTextureExplorer()
{
	editor_SetExplorerFileClickCallback(editor_TextureWindowExplorerFileClickCallback);
	
	editor_ClearExplorerExtensionFilters();
	editor_ExplorerFileNameField(0);
	editor_AddExplorerExtensionFilter(".jpg");
	editor_AddExplorerExtensionFilter(".png");
	editor_AddExplorerExtensionFilter(".bmp");
	editor_AddExplorerExtensionFilter(".tga");
	editor_AddExplorerExtensionFilter(".jpeg");
	
	editor_OpenExplorerWindow(NULL, 0);
}

void editor_CloseTextureExplorer()
{
	editor_CloseExplorerWindow();
}

void editor_TextureWindowExplorerFileClickCallback()
{
	/*char *file;
	editor_CloseTextureExplorer();
	
	while(file = editor_GetExplorerNextSelectedFile())
	{
		texture_LoadTexture(file, path_GetFileNameFromPath(file), 0);
	}
	
	editor_EnumerateTextures();*/
}


void editor_UpdateTextureUI()
{
	texture_window->x = -r_window_width * 0.5 + TEXTURE_WINDOW_WIDTH * 0.5;
	texture_window->y = -r_window_height * 0.5 + TEXTURE_WINDOW_HEIGHT * 0.5 + 20;
	
	
	texture_properties_window->x = -r_window_width * 0.5 + TEXTURE_PROPERTIES_WINDOW_WIDTH * 0.5;
	texture_properties_window->y = -r_window_height * 0.5 + TEXTURE_PROPERTIES_WINDOW_HEIGHT * 0.5 + 20;
}










