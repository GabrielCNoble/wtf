#include "ed_ui_explorer.h"
#include "gui.h"
#include "path.h"
#include "ed_proj.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ed_ui_texture.h"


#define EXPLORER_WINDOW_CURRENT_DIRECTORY_TEXT_FIELD_Y 280
#define EXPLORER_WINDOW_FILE_NAME_TEXT_FIELD_Y 250


#define EXPLORER_WINDOW_LIST_WO_FILE_NAME_TEXT_FIELD 250
#define EXPLORER_WINDOW_LIST_W_FILE_NAME_TEXT_FIELD 220


#define EXPLORER_MAX_SELECTIONS 512


widget_t *explorer_window = NULL;
text_field_t *explorer_window_path_text_field = NULL;
text_field_t *explorer_window_file_name_text_field = NULL;
option_list_t *explorer_window_list = NULL;
button_t *explorer_window_close_button = NULL;

char *ed_explorer_selected_file;

int ed_explorer_selected_file_count = 0;
int ed_explorer_selected_file_read_index = 0;
char *ed_explorer_selected_files[EXPLORER_MAX_SELECTIONS];
 

/* from path.c */
extern int dir_elements_count;
extern dir_element_t *dir_elements;
extern char *current_directory;


/* from r_main.c */
extern int r_window_width;
extern int r_window_height;


void (*editor_FileClickCallback)() = NULL;

#define MAX_EXT_FILTERS 64
#define EXT_FILTER_MAX_LEN 12
int ext_filter_count = 0;
char ext_filters[MAX_EXT_FILTERS][EXT_FILTER_MAX_LEN];


void editor_ExplorerCurrentDirectoryTextFieldCallback(widget_t *widget)
{
	text_field_t *field;
	
	if(widget->type == WIDGET_TEXT_FIELD)
	{
		field = (text_field_t *)widget;
		
		if(!path_SetDir(field->text))
		{
			gui_SetText(widget, current_directory);
			return;
		}
		
		editor_UpdateExplorerWindow();
	}
}

//void editor_

void editor_ExplorerListCallback(widget_t *widget)
{
	option_t *option;
	int index;
	
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		index = (int)option->widget.data;
		
		if(dir_elements[index].type == DIR_ELEM_GO_UP)
		{
			path_GoUp();
			editor_UpdateExplorerWindow();
		}
			
		else if(dir_elements[index].type == DIR_ELEM_DIRECTORY)
		{
			//editor_CloseExplorerWindow();
			path_GoDown(dir_elements[index].name);
			editor_UpdateExplorerWindow();
			//editor_OpenExplorerWindow(path_GetCurrentDirectory());
		}
		
		else
		{
			strcpy(ed_explorer_selected_file, current_directory);
			strcat(ed_explorer_selected_file, "/");
			strcat(ed_explorer_selected_file, dir_elements[index].name);
			
			editor_AddExplorerFileSelection(ed_explorer_selected_file);
					
			if(editor_FileClickCallback)
			{
				editor_FileClickCallback();
			}
			
		}
		
		
	}
}

void editor_ExplorerWindowCloseButtonCallback(widget_t *widget)
{
	editor_CloseExplorerWindow();
}




void editor_InitExplorerUI()
{
	int i;
	
	explorer_window = gui_AddWidget(NULL, "Explorer", 0, 0, 800, 600);
	explorer_window_path_text_field = gui_AddTextField(explorer_window, "Path", 0, EXPLORER_WINDOW_CURRENT_DIRECTORY_TEXT_FIELD_Y, 770, 0, editor_ExplorerCurrentDirectoryTextFieldCallback);
	explorer_window_file_name_text_field = gui_AddTextField(explorer_window, "File name", 0, EXPLORER_WINDOW_FILE_NAME_TEXT_FIELD_Y, 770, 0, NULL);
	explorer_window_list = gui_AddOptionList(explorer_window, "Explorer list", 0, EXPLORER_WINDOW_LIST_W_FILE_NAME_TEXT_FIELD, 770, OPTION_LIST_NO_OPTION_DIVISIONS | OPTION_LIST_DOUBLE_CLICK_SELECTION, 20, editor_ExplorerListCallback);
	explorer_window_close_button = gui_AddButton(explorer_window, "Close", 0, -250, 770, 40, 0, editor_ExplorerWindowCloseButtonCallback);
	
	gui_var_t *current_directory = gui_CreateVar("Current directory", GUI_VAR_STRING, NULL, NULL, 0);
	gui_TrackVar(current_directory, (widget_t *)explorer_window_path_text_field);
	
	
	editor_OpenExplorerWindow("C:/Users");
	//editor_CloseExplorerWindow();
	
	ed_explorer_selected_file = calloc(512, 1);
	
	
	for(i = 0; i < EXPLORER_MAX_SELECTIONS; i++)
	{
		ed_explorer_selected_files[i] = malloc(BSP_FILE_MAX_NAME_LEN);
		ed_explorer_selected_files[i][0] = '\0';
	}
	
	
	editor_CloseExplorerWindow();
}

void editor_OpenExplorerWindow(char *dir)
{
	int i;
	
	char item_text[512];
	option_t *option;
	if(explorer_window)
	{
		gui_SetVisible(explorer_window);
		gui_SetAsTop(explorer_window);
		
		if(!dir)
		{
			path_SetDir(editor_GetAbsolutePathToProject());
		}
		else
		{
			if(!path_SetDir(dir))
			{
				printf("editor_OpenExplorerWindow: couldn't set path %s!\n", dir);
			}
		}
		
		
		editor_UpdateExplorerWindow();
		
		/*if(path_SetDir(dir))
		{
			
			editor_ClearExplorerExtensionFilters();
			gui_SetAsTop(explorer_window);
			switch(type)
			{
				case EXPLORER_OPEN_TEXTURE_FILE:
					editor_ExplorerFileNameField(0);
					editor_AddExplorerExtensionFilter(".jpg");
					editor_AddExplorerExtensionFilter(".png");
					editor_AddExplorerExtensionFilter(".bmp");
					editor_AddExplorerExtensionFilter(".tga");
					editor_AddExplorerExtensionFilter(".jpeg");
						
				break;
				
				case EXPLORER_OPEN_MODEL_FILE:
					editor_ExplorerFileNameField(1);
					editor_AddExplorerExtensionFilter(".mpk");
				break;
				
				case EXPLORER_SAVE_PROJECT_FILE:
					editor_ExplorerFileNameField(0);
					editor_AddExplorerExtensionFilter(".wtf");
				break;
				
				case EXPLORER_OPEN_PROJECT_FILE:
					editor_ExplorerFileNameField(1);
					editor_AddExplorerExtensionFilter(".wtf");
				break;
			}
			
			
			
			editor_UpdateExplorerWindow();
		}*/
	}
}

void editor_UpdateExplorerWindow()
{
	int i;
	int j;
	int k;
	char item_text[512];
	option_t *option;
	if(explorer_window)
	{
		
		/*ed_explorer_selected_file_count = 0;
		ed_explorer_selected_file_read_index = 0;*/
		
		editor_ClearExplorerFileSelections();
		
		gui_RemoveAllOptions(explorer_window_list);
		
		explorer_window_path_text_field->widget.var->addr = &current_directory;
		
		for(i = 0; i < dir_elements_count; i++)
		{
			
			if(dir_elements[i].type == DIR_ELEM_SELF)
				continue;
			
			strcpy(item_text, dir_elements[i].name);
			if(dir_elements[i].type == DIR_ELEM_DIRECTORY)
			{
				strcat(item_text, "    <DIR>");
			}
			else if(dir_elements[i].type == DIR_ELEM_FILE)
			{
				
				j = strlen(dir_elements[i].name);
				while(dir_elements[i].name[j] != '.' && j > 0) j--;
				
				if(ext_filter_count)
				{
					for(k = 0; k < ext_filter_count; k++)
					{
						if(!strcmp(ext_filters[k], dir_elements[i].name + j))
						{
							break;
						}
					}
					
					if(k >= ext_filter_count)
						continue;
				}
				
				
				strcat(item_text, "    <FILE>");
			}
			
			
			option = gui_AddOptionToList(explorer_window_list, dir_elements[i].name, item_text);
			option->widget.data = (void *)i;
		}
	}
}

void editor_CloseExplorerWindow()
{
	if(explorer_window)
	{
		gui_SetInvisible(explorer_window);
	}
}

void editor_ExplorerPathField(int enable)
{
	
}

void editor_ExplorerFileNameField(int enable)
{
	if(enable)
	{
		gui_SetVisible((widget_t *)explorer_window_file_name_text_field);
		explorer_window_list->widget.y = EXPLORER_WINDOW_LIST_W_FILE_NAME_TEXT_FIELD;
		explorer_window_list->first_y = EXPLORER_WINDOW_LIST_W_FILE_NAME_TEXT_FIELD;
	}
	else
	{
		gui_SetInvisible((widget_t *)explorer_window_file_name_text_field);
		explorer_window_list->widget.y = EXPLORER_WINDOW_LIST_WO_FILE_NAME_TEXT_FIELD;
		explorer_window_list->first_y = EXPLORER_WINDOW_LIST_WO_FILE_NAME_TEXT_FIELD;
	}
}

void editor_ExplorerMultiFileSelection(int enable)
{
	if(enable)
	{
		
	}
	else
	{
		
	}
}

void editor_SetExplorerFileClickCallback(void (*file_click_callback)())
{
	editor_FileClickCallback = file_click_callback;
}

void editor_AddExplorerExtensionFilter(char *ext)
{
	strcpy(ext_filters[ext_filter_count], ext);
	ext_filter_count++;
}

void editor_ClearExplorerExtensionFilters()
{
	ext_filter_count = 0;
}

void editor_UpdateExplorerUI()
{
	explorer_window->w = r_window_width;
	explorer_window->h = r_window_height;
	explorer_window->x = 0;
	explorer_window->y = 0;
}



void editor_AddExplorerFileSelection(char *file)
{
	int name_len;
	if(ed_explorer_selected_file_count < EXPLORER_MAX_SELECTIONS)
	{
		name_len = strlen(file) + 1;
		
		if(name_len >= BSP_MAX_NAME_LEN)
		{
			printf("editor_AddExplorerFileSelection: file name too long!\n");
			return;
		}
		
		
		strcpy(ed_explorer_selected_files[ed_explorer_selected_file_count], file);
		ed_explorer_selected_file_count++;
		
	}
}

void editor_RemoveExplorerFileSelection(char *file)
{
	int i;
	
	for(i = 0; i < ed_explorer_selected_file_count; i++)
	{
		if(!strcmp(ed_explorer_selected_files[i], file))
		{
			if(i < ed_explorer_selected_file_count - 1)
			{
				strcpy(ed_explorer_selected_files[i], ed_explorer_selected_files[ed_explorer_selected_file_count - 1]);
			}
			
			ed_explorer_selected_file_count--;
			return;
		}
	}
	
}

void editor_ClearExplorerFileSelections()
{
	ed_explorer_selected_file_count = 0;
	ed_explorer_selected_file_read_index = 0;
}

int editor_GetExplorerSelectedFileCount()
{
	return ed_explorer_selected_file_count;
}

char **editor_GetExplorerSelectedFiles()
{
	return ed_explorer_selected_files;
}

char *editor_GetExplorerNextSelectedFile()
{
	char *file = NULL;
	
	if(ed_explorer_selected_file_read_index < ed_explorer_selected_file_count)
	{
		file = ed_explorer_selected_files[ed_explorer_selected_file_read_index];
		ed_explorer_selected_file_read_index++;
	}
	
	return file;
}

void editor_RewindExplorerSelectedFiles()
{
	ed_explorer_selected_file_read_index = 0;
}













