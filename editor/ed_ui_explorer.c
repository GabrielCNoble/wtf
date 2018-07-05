#include "ed_ui_explorer.h"
#include "ed_ui.h"
#include "gui.h"
#include "path.h"
//#include "ed_proj.h"
#include "memory.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//#include "ed_ui_texture.h"


#define EXPLORER_WINDOW_CURRENT_DIRECTORY_TEXT_FIELD_Y 280
#define EXPLORER_WINDOW_FILE_NAME_TEXT_FIELD_Y 250


#define EXPLORER_WINDOW_LIST_WO_FILE_NAME_TEXT_FIELD 250
#define EXPLORER_WINDOW_LIST_W_FILE_NAME_TEXT_FIELD 220


#define EXPLORER_MAX_SELECTIONS 512

/*
******************************************************************************************
******************************************************************************************
******************************************************************************************
*/


#define EXPLORER_DIALOG_BOX_WIDTH 400
#define EXPLORER_DIALOG_BOX_HEIGHT 300
static widget_t *explorer_dialog_box = NULL;
static widget_t *explorer_dialog_box_text_field = NULL;
static button_t *explorer_dialog_box_confirm_button = NULL;
static button_t *explorer_dialog_box_cancel_button = NULL;



static widget_t *explorer_window = NULL;
static text_field_t *explorer_window_path_text_field = NULL;
static text_field_t *explorer_window_file_name_text_field = NULL;


#define EXPLORER_WINDOW_LIST_WIDTH 950
#define EXPLORER_WINDOW_LIST_HEIGHT 200
static widget_t *explorer_list_widget = NULL;
static option_list_t *explorer_window_list = NULL;

#define EXPLORER_WINDOW_CACHED_PATHS_LIST_WIDTH 200
#define EXPLORER_WINDOW_CACHED_PATHS_LIST_HEIGHT 250
static widget_t *explorer_cached_paths_list_widget = NULL;
static option_list_t *explorer_cached_paths_list = NULL;

#define EXPLORER_WINDOW_CANCEL_BUTTON_WIDTH 100
#define EXPLORER_WINDOW_CANCEL_BUTTON_HEIGHT 20
static button_t *explorer_window_cancel_button = NULL;

#define EXPLORER_WINDOW_ACTION_ON_FILE_BUTTON_WIDTH 100
#define EXPLORER_WINDOW_ACTION_ON_FILE_BUTTON_HEIGHT 20
static button_t *explorer_window_action_on_file_button = NULL;

//char *ed_explorer_selected_file;

static int ed_explorer_selected_file_count = 0;
static int ed_explorer_selected_file_read_index = 0;
static char *ed_explorer_selected_files[EXPLORER_MAX_SELECTIONS];


cached_path_t *explorer_cached_paths = NULL;
cached_path_t *explorer_last_cached_path = NULL;


void (*editor_FileClickCallback)() = NULL;
void (*editor_ExplorerReadFileCallback)(char *, char *) = NULL;
void (*editor_ExplorerWriteFileCallback)(char *, char *) = NULL;

#define MAX_EXT_FILTERS 64
#define EXT_FILTER_MAX_LEN 12
int ext_filter_count = 0;
char ext_filters[MAX_EXT_FILTERS][EXT_FILTER_MAX_LEN];


int explorer_file_mode = -1;


/*
******************************************************************************************
******************************************************************************************
******************************************************************************************
*/
 

/* from path.c */
extern int pth_dir_element_count;
extern dir_element_t *pth_dir_elements;
//extern char *current_directory;


/* from r_main.c */
extern int r_window_width;
extern int r_window_height;





/*
******************************************************************************************
******************************************************************************************
******************************************************************************************
*/

char full_path_to_file[1024];


/*
=======================================
=======================================
=======================================
*/

void editor_ExplorerPathTextFieldCallback(widget_t *widget)
{
	text_field_t *field;
	char *path;
	
	if(!widget)
	{
		return;
	}
	
	field = (text_field_t *)widget;
	
	if(field->bm_text_field_flags & TEXT_FIELD_UPDATED)
	{
		if(field->text[0])
		{
			path = path_FormatPath(field->text);
			
			if(!path_CheckDir(path))
			{
				gui_SetText((widget_t *)field, path_GetCurrentDirectory());
			}
			else
			{
				editor_OpenExplorerWindow(path, explorer_file_mode);
			}	
		}
	}
}

void editor_ExplorerListCallback(widget_t *widget)
{
	option_list_t *option_list;
	option_t *option;
	dir_element_t *dir_element;
	
	if(!widget)
	{
		return;
	}
	
	option_list = (option_list_t *)widget;
	option = (option_t *)option_list->active_option;
	
	if(!option)
	{
		return;
	}
	
	dir_element = (dir_element_t *)option->widget.data;
							
	if(option_list->widget.bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_DOUBLE_CLICK)
	{						
		if(option->widget.bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_DOUBLE_CLICK)
		{
			switch(dir_element->type)
			{
				case DIR_ELEMENT_TYPE_PARENT:
					path_GoUp();
					editor_UpdateExplorerWindow();
				break;
								
				case DIR_ELEMENT_TYPE_DIRECTORY:
					path_GoDown(dir_element->name);
					editor_UpdateExplorerWindow();
				break;
								
				case DIR_ELEMENT_TYPE_FILE:
					
					if(explorer_file_mode == EXPLORER_FILE_MODE_READ)
					{
						editor_ExplorerReadFile(path_GetCurrentDirectory(), dir_element->name);
					}
					else
					{
						editor_ExplorerWriteFile(path_GetCurrentDirectory(), dir_element->name);
					}
					
					editor_CloseExplorerWindow();
				break;
			}
		}
	}
	else if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
	{
		if(option->widget.bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
		{
			if(dir_element->type == DIR_ELEMENT_TYPE_FILE)
			{
				gui_SetText((widget_t *)explorer_window_file_name_text_field, dir_element->name);
			}
		}
	}
	
}

void editor_ExplorerCachedPathListCallback(widget_t *widget)
{
	option_list_t *option_list;
	option_t *option;
	
	if(!widget)
	{
		return;
	}
	
	option_list = (option_list_t *)widget;
	option = (option_t *)option_list->active_option;
		
	if(option)
	{
		if(option->widget.bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
		{
			editor_OpenExplorerWindow((char *)option->widget.data, explorer_file_mode);
		}
	}
}

void editor_ExplorerCancelButtonCallback(widget_t *widget)
{
	button_t *button;
	
	if(!widget)
	{
		return;
	}
	
	if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
	{
		editor_CloseExplorerWindow();
	}
}

void editor_ExplorerActionOnFileButtonCallback(widget_t *widget)
{
	char *ext;
	
	if(!widget)
	{
		return;
	}


	if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
	{	
		if(!explorer_window_file_name_text_field->text)
		{
			return;
		}		
				
		if(!explorer_window_file_name_text_field->text[0])
		{
			return;
		}
		
		switch(explorer_file_mode)
		{
			case EXPLORER_FILE_MODE_READ:		
				editor_ExplorerReadFile(path_GetCurrentDirectory(), explorer_window_file_name_text_field->text);
				editor_CloseExplorerWindow();	
			break;
				
			case EXPLORER_FILE_MODE_WRITE:
				editor_ExplorerWriteFile(path_GetCurrentDirectory(), explorer_window_file_name_text_field->text);
				editor_CloseExplorerWindow();
			break;
		}
	}
}


void editor_ExplorerDialogBoxProcessCallback(widget_t *widget)
{
	button_t *button;
	
	if(widget->type == WIDGET_BUTTON)
	{
		button = (button_t *)widget;
		
		if(button->widget.bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
		{
			if(!strcmp(button->widget.name, "Cancel"))
			{
				editor_CloseExplorerDialogBox();
			}
		}
		
	}
}

/*
=======================================
=======================================
=======================================
*/

void editor_ExplorerProcessCallback(widget_t *widget)
{
	option_list_t *option_list;
	option_t *option;
	dir_element_t *dir_element;
	button_t *button;
	int i;

		
	if((option_list_t *)widget == explorer_window_list)
	{	
		editor_ExplorerListCallback(widget);
	}
	else if((option_list_t *)widget == explorer_cached_paths_list)
	{
		editor_ExplorerCachedPathListCallback(widget);
	}
	else if((button_t *)widget == explorer_window_cancel_button)
	{
		editor_ExplorerCancelButtonCallback(widget);
	}
	else if((button_t *)widget == explorer_window_action_on_file_button)
	{
		editor_ExplorerActionOnFileButtonCallback(widget);
	}
	else if((text_field_t *)widget == explorer_window_path_text_field)
	{
		editor_ExplorerPathTextFieldCallback(widget);
	}
	
}

/*
******************************************************************************************
******************************************************************************************
******************************************************************************************
*/

void dummy_read(char *directory, char *file_name)
{
	printf("file %s read from %s\n", file_name, directory);
}

void dummy_write(char *directory, char *file_name)
{
	printf("file %s written to %s\n", file_name, directory);
}

void editor_InitExplorerUI()
{
	int i;
	
	linked_edge_t *linked;
	
	explorer_window = gui_AddWidget(NULL, "Explorer", 0, 0, 800, 600);
	explorer_window->process_callback = editor_ExplorerProcessCallback;
	 
	
	explorer_list_widget = gui_AddWidget(explorer_window, "List widget", 0, 0, EXPLORER_WINDOW_LIST_WIDTH, EXPLORER_WINDOW_LIST_HEIGHT);
	explorer_list_widget->edge_flags |= WIDGET_LEFT_EDGE_ENABLED;
	explorer_list_widget->bm_flags |= WIDGET_NO_HIGHLIGHT | WIDGET_DRAW_OUTLINE;
	
	explorer_window_list = gui_AddOptionList(explorer_list_widget, "Explorer list", 0, EXPLORER_WINDOW_LIST_HEIGHT * 0.5, EXPLORER_WINDOW_LIST_WIDTH, OPTION_LIST_NO_OPTION_DIVISIONS | OPTION_LIST_DOUBLE_CLICK_SELECTION, 20, NULL);
	explorer_window_list->widget.process_callback = editor_ExplorerProcessCallback;
	explorer_window_list->widget.bm_flags |= WIDGET_DRAW_OUTLINE;
	
	
	explorer_cached_paths_list_widget = gui_AddWidget(explorer_window, "Cached paths widget", 0, 0, EXPLORER_WINDOW_CACHED_PATHS_LIST_WIDTH, EXPLORER_WINDOW_CACHED_PATHS_LIST_HEIGHT);
	explorer_cached_paths_list_widget->bm_flags |= WIDGET_NO_HIGHLIGHT | WIDGET_DRAW_OUTLINE;
	explorer_cached_paths_list = gui_AddOptionList(explorer_cached_paths_list_widget, "Cached paths", 0, EXPLORER_WINDOW_CACHED_PATHS_LIST_HEIGHT * 0.5, EXPLORER_WINDOW_CACHED_PATHS_LIST_WIDTH, OPTION_LIST_NO_OPTION_DIVISIONS, 20, NULL);
	explorer_cached_paths_list->widget.process_callback = editor_ExplorerProcessCallback;
	explorer_cached_paths_list->widget.bm_flags |= WIDGET_DRAW_OUTLINE;
	
	explorer_window_cancel_button = gui_AddButton(explorer_list_widget, "Cancel", 0, -250, EXPLORER_WINDOW_CANCEL_BUTTON_WIDTH, EXPLORER_WINDOW_CANCEL_BUTTON_HEIGHT, 0, NULL);
	explorer_window_cancel_button->widget.process_callback = editor_ExplorerProcessCallback;
	explorer_window_cancel_button->button_text = memory_Strdup("Cancel", "editor_InitExplorerUI");
	explorer_window_cancel_button->widget.bm_flags |= WIDGET_RENDER_TEXT;
	
	explorer_window_action_on_file_button = gui_AddButton(explorer_list_widget, "Open", 0, -250, EXPLORER_WINDOW_ACTION_ON_FILE_BUTTON_WIDTH, EXPLORER_WINDOW_ACTION_ON_FILE_BUTTON_HEIGHT, 0, NULL);
	explorer_window_action_on_file_button->widget.process_callback = editor_ExplorerProcessCallback;
	
	linked = gui_LinkEdges((widget_t *)explorer_window_list, explorer_list_widget, WIDGET_LEFT_EDGE, WIDGET_LEFT_EDGE);
	linked->offset = 10;
	
	linked = gui_LinkEdges((widget_t *)explorer_window_list, explorer_list_widget, WIDGET_RIGHT_EDGE, WIDGET_RIGHT_EDGE);
	linked->offset = 10;
	
	gui_LinkEdges((widget_t *)explorer_window_list, explorer_list_widget, WIDGET_TOP_EDGE, WIDGET_TOP_EDGE);
	
	gui_LinkEdges(explorer_cached_paths_list_widget, explorer_list_widget, WIDGET_RIGHT_EDGE, WIDGET_LEFT_EDGE);
	gui_LinkEdges(explorer_cached_paths_list_widget, explorer_list_widget, WIDGET_TOP_EDGE, WIDGET_TOP_EDGE);
	
	
	gui_LinkEdges((widget_t *)explorer_cached_paths_list, explorer_cached_paths_list_widget, WIDGET_LEFT_EDGE, WIDGET_LEFT_EDGE);
	gui_LinkEdges((widget_t *)explorer_cached_paths_list, explorer_cached_paths_list_widget, WIDGET_RIGHT_EDGE, WIDGET_RIGHT_EDGE);
	
	
	explorer_window_path_text_field = gui_AddTextField(explorer_list_widget, "Current directory", 0, 0, 10, 0, NULL);
	explorer_window_path_text_field->widget.process_callback = editor_ExplorerPathTextFieldCallback;
	
	explorer_window_file_name_text_field = gui_AddTextField(explorer_list_widget, "File name", 0, 0, 10, 0, NULL);
	
	
	linked = gui_LinkEdges((widget_t *)explorer_window_path_text_field, explorer_list_widget, WIDGET_LEFT_EDGE, WIDGET_LEFT_EDGE);
	linked->offset = 10;
	
	linked = gui_LinkEdges((widget_t *)explorer_window_path_text_field, explorer_list_widget, WIDGET_RIGHT_EDGE, WIDGET_RIGHT_EDGE);
	linked->offset = 120;
	
	
	linked = gui_LinkEdges((widget_t *)explorer_window_file_name_text_field, explorer_list_widget, WIDGET_LEFT_EDGE, WIDGET_LEFT_EDGE);
	linked->offset = 10;
	
	linked = gui_LinkEdges((widget_t *)explorer_window_file_name_text_field, explorer_list_widget, WIDGET_RIGHT_EDGE, WIDGET_RIGHT_EDGE);
	linked->offset = 120;
	
	
	
	
	explorer_dialog_box = gui_AddWidget(NULL, "explorer dialog box", 0, 0, EXPLORER_DIALOG_BOX_WIDTH, EXPLORER_DIALOG_BOX_HEIGHT);
	explorer_dialog_box->bm_flags |= WIDGET_DRAW_OUTLINE;
	explorer_dialog_box->process_callback = editor_ExplorerDialogBoxProcessCallback;
	
	explorer_dialog_box_confirm_button = gui_AddButton(explorer_dialog_box, "Comfirm", 100, -80, 120, 40, 0, NULL);
	gui_SetButtonText(explorer_dialog_box_confirm_button, "Comfirm");
	explorer_dialog_box_confirm_button->widget.process_callback = editor_ExplorerDialogBoxProcessCallback;
	
	explorer_dialog_box_cancel_button = gui_AddButton(explorer_dialog_box, "Cancel", -100, -80, 120, 40, 0, NULL);
	gui_SetButtonText(explorer_dialog_box_cancel_button, "Cancel");
	explorer_dialog_box_cancel_button->widget.process_callback = editor_ExplorerDialogBoxProcessCallback;
	
	
	
	editor_CloseExplorerDialogBox();
	
	
	//gui_var_t *current_directory = gui_CreateVar("Current directory", GUI_VAR_STRING, NULL, NULL, 0);
	//gui_TrackVar(current_directory, (widget_t *)explorer_window_path_text_field);
	/*editor_CachePath("test/path");
	editor_CachePath("test1/path");
	editor_CachePath("test2/path");
	editor_CachePath("test3/path");
	editor_CachePath("test4/path");*/
	
	editor_OpenExplorerWindow(path_GetUserDocumentsDirectory(), EXPLORER_FILE_MODE_READ);
	//editor_OpenExplorerDialogBox(NULL);	
	//ed_explorer_selected_file = calloc(512, 1);
	//ed_explorer_selected_file = memory_Calloc(512, 1, "editor_InitExplorerUI");
	
	
	for(i = 0; i < EXPLORER_MAX_SELECTIONS; i++)
	{
		//ed_explorer_selected_files[i] = malloc(BSP_FILE_MAX_NAME_LEN);
		ed_explorer_selected_files[i] = memory_Malloc(PATH_MAX, "editor_InitExplorerUI");
		ed_explorer_selected_files[i][0] = '\0';
	}
	
	
	editor_SetExplorerReadFileCallback(dummy_read);
	editor_CloseExplorerWindow();
	
	
	//editor_OpenExplorerWindow(path_GetUserDocumentsDirectory(), EXPLORER_FILE_MODE_READ);
	//editor_OpenExplorerDialogBox(NULL);
	
}

void editor_FinishExplorerUI()
{
	int i;
	
	for(i = 0; i < EXPLORER_MAX_SELECTIONS; i++)
	{
		memory_Free(ed_explorer_selected_files[i]);
	}
	
	editor_ClearCachedPaths();
}

void editor_UpdateExplorerUI()
{
	widget_t *parent;
	if(explorer_window)
	{
		explorer_window->w = r_window_width * 0.5;
		explorer_window->h = r_window_height * 0.5 - MENU_BAR_HEIGHT * 0.5;
		explorer_window->x = 0;
		explorer_window->y = -MENU_BAR_HEIGHT * 0.5;
	}
	
	if(explorer_window_list)
	{		
		explorer_list_widget->h = explorer_window->h - 10;
		
		explorer_list_widget->x = explorer_window->w - explorer_list_widget->w;
		explorer_list_widget->y = -explorer_window->h + explorer_list_widget->h;
		
		explorer_window_list->first_y = explorer_list_widget->h - 80;
		explorer_window_list->bm_option_list_flags |= OPTION_LIST_UPDATE_EXTENTS;
		explorer_window_list->max_visible_options = (explorer_list_widget->h * 2.0) / OPTION_HEIGHT;
	}
	
	if(explorer_cached_paths_list)
	{
		explorer_cached_paths_list_widget->x = -explorer_window->w + explorer_cached_paths_list_widget->w;
		explorer_cached_paths_list->bm_option_list_flags |= OPTION_LIST_UPDATE_EXTENTS;
		
		explorer_cached_paths_list->first_y = explorer_cached_paths_list_widget->h - 50;
		explorer_cached_paths_list->widget.w = explorer_cached_paths_list_widget->w - 4;
	}
	
	if(explorer_window_cancel_button)
	{
		parent = explorer_window_cancel_button->widget.parent;	
		explorer_window_cancel_button->widget.x = parent->w - explorer_window_cancel_button->widget.w - 10;
		explorer_window_cancel_button->widget.y = parent->h - explorer_window_cancel_button->widget.h - 10 - 40;
	}
	
	if(explorer_window_action_on_file_button)
	{
		parent = explorer_window_action_on_file_button->widget.parent;
		explorer_window_action_on_file_button->widget.x = parent->w - explorer_window_action_on_file_button->widget.w - 10;
		explorer_window_action_on_file_button->widget.y = parent->h - explorer_window_action_on_file_button->widget.h - 10 - 10;
	}
	
	if(explorer_window_path_text_field && explorer_window_file_name_text_field)
	{
		parent = explorer_window_path_text_field->widget.parent;
		explorer_window_path_text_field->widget.y = explorer_window_action_on_file_button->widget.y;
		explorer_window_file_name_text_field->widget.y = explorer_window_cancel_button->widget.y;
	}
	
	if(explorer_dialog_box)
	{
	
	}
	
}


void editor_OpenExplorerWindow(char *dir, int mode)
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
			path_SetDir(path_GetUserDocumentsDirectory());
		}
		else
		{
			if(!path_SetDir(dir))
			{
				printf("editor_OpenExplorerWindow: couldn't set path to %s!\n", dir);
			}
		}
		
		editor_SetExplorerFileMode(mode);
		editor_UpdateExplorerWindow();
		
		//editor_OpenExplorerDialogBox(NULL);
	}
}

void editor_UpdateExplorerCachedPaths()
{
	option_t *option;
	cached_path_t *cached;
	
	if(explorer_cached_paths_list)
	{
		gui_RemoveAllOptions(explorer_cached_paths_list);
		
		cached = explorer_cached_paths;
		
		while(cached)
		{
			option = gui_AddOptionToList(explorer_cached_paths_list, "cached", cached->path);
			option->widget.data = cached->path;
			cached = cached->next;
		}
	}
}

void editor_UpdateExplorerWindow()
{
	int i;
	option_t *option;
	char option_text[512];
	cached_path_t *cached;
	
	if(explorer_window_list)
	{
		gui_RemoveAllOptions(explorer_window_list);
		
		for(i = 0; i < pth_dir_element_count; i++)
		{
			switch(pth_dir_elements[i].type)
			{
				case DIR_ELEMENT_TYPE_PARENT:
				case DIR_ELEMENT_TYPE_DIRECTORY:
					strcpy(option_text, "<DIR> ");
				break;	
				
				case DIR_ELEMENT_TYPE_FILE:
					strcpy(option_text, "<FILE> ");
				break;
			}
			
			strcat(option_text, pth_dir_elements[i].name);
			option = gui_AddOptionToList(explorer_window_list, option_text, option_text);
			option->widget.data = &pth_dir_elements[i];
		}	
	}
	
	if(explorer_window_path_text_field)
	{
		gui_SetText((widget_t *)explorer_window_path_text_field, path_GetCurrentDirectory());
		gui_SetText((widget_t *)explorer_window_file_name_text_field, "");
	}
	
	editor_UpdateExplorerCachedPaths();
	
	
}

void editor_CloseExplorerWindow()
{
	if(explorer_window)
	{
		gui_SetInvisible(explorer_window);
		editor_CloseExplorerDialogBox(NULL);
	}
}

void editor_SetExplorerFileMode(int mode)
{
	if(explorer_window_action_on_file_button)
	{
		switch(mode)
		{
			case EXPLORER_FILE_MODE_READ:
			case EXPLORER_FILE_MODE_WRITE:
				
				if(mode == EXPLORER_FILE_MODE_READ)
				{
					gui_SetButtonText(explorer_window_action_on_file_button, "Open");
				}
				else
				{
					gui_SetButtonText(explorer_window_action_on_file_button, "Save");
				}
				
				explorer_file_mode = mode;
			break;
		}
	}
}

void editor_ExplorerReadFile(char *directory, char *file_name)
{
	if(editor_ExplorerReadFileCallback)
	{		
		editor_CachePath(directory);
		editor_ExplorerReadFileCallback(directory, file_name);
	}
}

void editor_ExplorerWriteFile(char *directory, char *file_name)
{
	if(editor_ExplorerWriteFileCallback)
	{		
		editor_CachePath(directory);
		editor_ExplorerWriteFileCallback(directory, file_name);
	}
}

void editor_OpenExplorerDialogBox(char *text)
{
	if(explorer_dialog_box)
	{
		gui_SetVisible(explorer_dialog_box);
		gui_SetAsTop(explorer_dialog_box);
		editor_LockExplorerUI();
		editor_LockUI();
	}
	
}

void editor_CloseExplorerDialogBox()
{
	
	
	if(explorer_dialog_box)
	{
		gui_SetInvisible(explorer_dialog_box);
		editor_UnlockExplorerUI();
		editor_UnlockUI();
	}
}

void editor_LockExplorerUI()
{		
	gui_SetIgnoreMouse(explorer_window);
}

void editor_UnlockExplorerUI()
{	
	gui_SetReceiveMouse(explorer_window);
}

void editor_ExplorerPathField(int enable)
{
	
}

void editor_ExplorerFileNameField(int enable)
{
	
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

void editor_SetExplorerReadFileCallback(void (*read_file_callback)(char *directory, char *file_name))
{
	editor_ExplorerReadFileCallback = read_file_callback;
}

void editor_SetExplorerWriteFileCallback(void (*write_file_callback)(char *directory, char *file_name))
{
	editor_ExplorerReadFileCallback = write_file_callback;
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





void editor_AddExplorerFileSelection(char *file)
{
	int name_len;
	if(ed_explorer_selected_file_count < EXPLORER_MAX_SELECTIONS)
	{
		name_len = strlen(file) + 1;
		
		if(name_len >= PATH_MAX)
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


void editor_CachePath(char *path)
{
	cached_path_t *new_path;
	
	
	new_path = editor_GetCachedPath(path);
	
	if(new_path)
	{
		new_path->refs++;
	}
	else
	{
		new_path = memory_Malloc(sizeof(cached_path_t), "editor_CachePath");
	
		new_path->next = NULL;
		new_path->prev = NULL;
		new_path->path = memory_Strdup(path, "editor_CachePath");
		new_path->refs = 1;
		
		if(!explorer_cached_paths)
		{
			explorer_cached_paths = new_path;
		}
		else
		{
			explorer_last_cached_path->next = new_path;
			new_path->prev = explorer_last_cached_path;
		}
		
		explorer_last_cached_path = new_path;
	}
	
}

cached_path_t *editor_GetCachedPath(char *path)
{
	cached_path_t *seek_path;
	
	seek_path = explorer_cached_paths;
	
	
	while(seek_path)
	{
		if(!strcmp(path, seek_path->path))
		{
			return seek_path;
		}
		
		seek_path = seek_path->next;
	}
	
	return NULL;
}

void editor_DropCachedPath(char *path)
{
	cached_path_t *seek_path;
	
	seek_path = editor_GetCachedPath(path);
	
	if(seek_path)
	{
		if(!seek_path->prev)
		{
			explorer_cached_paths = seek_path->next;
			if(explorer_cached_paths)
			{
				explorer_cached_paths->prev = NULL;
			}
		}
		else
		{
			seek_path->prev->next = seek_path->next;
			
			if(seek_path->next)
			{
				seek_path->next->prev = seek_path->prev;
			}
			else
			{
				explorer_last_cached_path = explorer_last_cached_path->prev;
			}
		}
			
		memory_Free(seek_path->path);
		memory_Free(seek_path);
	}
}

void editor_ClearCachedPaths()
{
	while(explorer_cached_paths)
	{
		explorer_last_cached_path = explorer_cached_paths->next;
		memory_Free(explorer_cached_paths->path);
		memory_Free(explorer_cached_paths);
		explorer_cached_paths = explorer_last_cached_path;
	}
}












