#include "ed_ui_explorer.h"
#include "ed_ui.h"
#include "..\..\common\gui.h"
#include "..\..\common\path.h"
//#include "ed_proj.h"
#include "..\..\common\c_memory.h"
#include "..\..\common\resource.h"

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
/*

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
static button_t *explorer_window_action_on_file_button = NULL;*/

//char *ed_explorer_selected_file;

static int ed_explorer_selected_file_count = 0;
static int ed_explorer_selected_file_read_index = 0;
int ed_explorer_open = 0;
char ed_explorer_path_text_buffer[PATH_MAX];
char ed_explorer_file_text_buffer[PATH_MAX];
static char *ed_explorer_selected_files[EXPLORER_MAX_SELECTIONS];


cached_path_t *explorer_cached_paths = NULL;
cached_path_t *explorer_last_cached_path = NULL;


void (*editor_FileClickCallback)() = NULL;
int (*editor_ExplorerReadFileCallback)(char *file_path, char *file_name) = NULL;
int (*editor_ExplorerWriteFileCallback)(char *file_path, char *file_name, void **file_buffer, int *file_buffer_size) = NULL;

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

/*void editor_ExplorerPathTextFieldCallback(widget_t *widget)
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
}*/

/*void editor_ExplorerListCallback(widget_t *widget)
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

}*/

/*void editor_ExplorerCachedPathListCallback(widget_t *widget)
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
}*/

/*void editor_ExplorerCancelButtonCallback(widget_t *widget)
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
}*/

/*void editor_ExplorerActionOnFileButtonCallback(widget_t *widget)
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
}*/


/*void editor_ExplorerDialogBoxProcessCallback(widget_t *widget)
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
}*/

/*
=======================================
=======================================
=======================================
*/

/*void editor_ExplorerProcessCallback(widget_t *widget)
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

}*/

/*
******************************************************************************************
******************************************************************************************
******************************************************************************************
*/

/*void dummy_read(char *directory, char *file_name)
{
	printf("file %s read from %s\n", file_name, directory);
}

void dummy_write(char *directory, char *file_name)
{
	printf("file %s written to %s\n", file_name, directory);
}*/

void editor_InitExplorerUI()
{
	/*int i;

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
	explorer_dialog_box_cancel_button->widget.process_callback = editor_ExplorerDialogBoxProcessCallback;*/



/*	editor_CloseExplorerDialogBox();*/


	//gui_var_t *current_directory = gui_CreateVar("Current directory", GUI_VAR_STRING, NULL, NULL, 0);
	//gui_TrackVar(current_directory, (widget_t *)explorer_window_path_text_field);
	/*editor_CachePath("test/path");
	editor_CachePath("test1/path");
	editor_CachePath("test2/path");
	editor_CachePath("test3/path");
	editor_CachePath("test4/path");*/

//	editor_OpenExplorerWindow(path_GetUserDocumentsDirectory(), EXPLORER_FILE_MODE_READ);
	//editor_OpenExplorerDialogBox(NULL);
	//ed_explorer_selected_file = calloc(512, 1);
	//ed_explorer_selected_file = memory_Calloc(512, 1, "editor_InitExplorerUI");

	int i;

	for(i = 0; i < EXPLORER_MAX_SELECTIONS; i++)
	{
		ed_explorer_selected_files[i] = memory_Malloc(PATH_MAX);
		ed_explorer_selected_files[i][0] = '\0';
	}


	/*editor_SetExplorerReadFileCallback(dummy_read);
	editor_CloseExplorerWindow();*/


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

void editor_ExplorerTopWindow()
{
	vec2_t window_size;

	window_size = gui_ImGuiGetContentRegionMax();
	window_size.y = 120.0;

	gui_ImGuiBeginChild("Top window", window_size, 1, 0);
	{
		/* Path text field... */
		gui_ImGuiNewLine();
		gui_ImGuiSameLine(r_window_width - window_size.x * 0.4 - 150.0, -1.0);
		gui_ImGuiPushItemWidth(window_size.x * 0.4);
		if(gui_ImGuiInputText("Path", ed_explorer_path_text_buffer, PATH_MAX, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			/* if user types in the path field, try to
			change to the new path... */
			path_SetDir(ed_explorer_path_text_buffer);
		}
		gui_ImGuiPopItemWidth();

		/* Close button... */
		gui_ImGuiSameLine(0.0, -1.0);

		if(gui_ImGuiButton("Close", vec2(80.0, 22.0)))
		{
			ed_explorer_open = 0;
		}


		/*=================================================*/


		/* File text field... */
		gui_ImGuiNewLine();
		gui_ImGuiSameLine(r_window_width - window_size.x * 0.4 - 150.0, -1.0);
		gui_ImGuiPushItemWidth(window_size.x * 0.4);
		if(gui_ImGuiInputText("File", ed_explorer_file_text_buffer, PATH_MAX, ImGuiInputTextFlags_EnterReturnsTrue))
		{

		}
		gui_ImGuiPopItemWidth();

		/* Open/save button... */
		gui_ImGuiSameLine(0.0, -1.0);

		switch(explorer_file_mode)
		{
			case EXPLORER_FILE_MODE_READ:
				if(gui_ImGuiButton("Open", vec2(80.0, 22.0)))
				{
					//if(ed_explorer_file_text_buffer[0])
					//{
						/* only open the file if there's a valid name string... */
					editor_ExplorerReadFile(ed_explorer_path_text_buffer, ed_explorer_file_text_buffer);
					//}
					ed_explorer_open = 0;
				}
			break;

			case EXPLORER_FILE_MODE_WRITE:
				if(gui_ImGuiButton("Save", vec2(80.0, 22.0)))
				{
					if(ed_explorer_file_text_buffer[0])
					{
						/* only save the file if there's a valid name string... */
						editor_ExplorerWriteFile(ed_explorer_path_text_buffer, ed_explorer_file_text_buffer);
					}
					ed_explorer_open = 0;
				}
			break;
		}
	}
	gui_ImGuiEndChild();
}

void editor_ExplorerCachedDirListWindow()
{
	/* cached dirs list... */

	cached_path_t *path;
	cached_path_t *paths;
	cached_path_t *selected_cached_path;

	vec2_t window_size;

	window_size.x = gui_ImGuiGetColumnOffset(-1);
	window_size.y = 0.0;

	selected_cached_path = NULL;

	gui_ImGuiBeginChild("Chached dirs window", window_size, 1, 0);
	{
		path = explorer_cached_paths;

		while(path)
		{
			if(gui_ImGuiMenuItem(path->path, NULL, NULL, 1) && selected_cached_path == NULL)
			{
				selected_cached_path = path;
			}

			path = path->next;
		}
	}
	gui_ImGuiEndChild();

	if(selected_cached_path)
	{
		path_SetDir(selected_cached_path->path);
	}
}

void editor_ExplorerDirListWindow()
{
	vec2_t window_size;
	vec2_t option_size;


	dir_element_t *selected_dir_element = NULL;

	int i;
	int go_up;

	int clicked;
	int double_clicked = 0;

	char option_text[PATH_MAX];
	char cache_path[PATH_MAX];

	char selected = 1;
	/* dir list... */
	window_size.x = r_window_width - gui_ImGuiGetColumnOffset(-1) - 20;
	window_size.y = 0.0;

	strcpy(ed_explorer_path_text_buffer, path_GetCurrentDirectory());

	gui_ImGuiBeginChild("Dirs window", window_size, 1, 0);
	{
		option_size = gui_ImGuiGetWindowSize();
		option_size.x = r_window_width - option_size.x - 30;
		option_size.y = 16.0;

		go_up = 0;

		double_clicked = 0;

		for(i = 0; i < pth_dir_element_count; i++)
		{
			switch(pth_dir_elements[i].type)
			{
				case DIR_ELEMENT_TYPE_DIRECTORY:
					strcpy(option_text, "<DIR> ");
				break;

				case DIR_ELEMENT_TYPE_PARENT:
					option_text[0] = '\0';
				break;

				case DIR_ELEMENT_TYPE_FILE:
					strcpy(option_text, "<FILE> ");
				break;
			}

			strcat(option_text, pth_dir_elements[i].name);

			//if(gui_ImGuiIsMouseDoubleClicked(0))
			//{
			//if(gui_ImGuiMenuItem(option_text, NULL, NULL, 1) && selected_dir_element == NULL)
			if(gui_ImGuiSelectable(option_text, ImGuiSelectableFlags_AllowDoubleClick, option_size) && selected_dir_element == NULL)
			{
				if(gui_ImGuiIsMouseDoubleClicked(0))
				{
					if(pth_dir_elements[i].type == DIR_ELEMENT_TYPE_PARENT)
					{
						go_up = 1;
					}
					else
					{
						selected_dir_element = pth_dir_elements + i;
					}

					double_clicked = 1;
				}
				else
				{
					if(pth_dir_elements[i].type == DIR_ELEMENT_TYPE_FILE)
					{
                        selected_dir_element = pth_dir_elements + i;
					}
				}
			}

		}

		if(go_up)
		{
			path_GoUp();
		}
		else if(selected_dir_element)
		{

			if(selected_dir_element->type == DIR_ELEMENT_TYPE_DIRECTORY)
			{
				path_GoDown(selected_dir_element->name);
			}
			else
			{
                switch(explorer_file_mode)
                {
					case EXPLORER_FILE_MODE_WRITE:
                        strcpy(ed_explorer_file_text_buffer, selected_dir_element->name);
					break;

					case EXPLORER_FILE_MODE_READ:

						/* if we only click over a file, it's name will
                        be put into the file text field... */
						strcpy(ed_explorer_file_text_buffer, selected_dir_element->name);

                        if(double_clicked)
						{
							/* if we double click on it, try to open the file... */
							editor_ExplorerReadFile(ed_explorer_path_text_buffer, ed_explorer_file_text_buffer);
							ed_explorer_open = 0;
						}

					break;
                }

				//strcpy(cache_path, ed_explorer_path_text_buffer);
				//editor_CachePath(cache_path);

				//strcat(cache_path, "/");
				//strcat(cache_path, selected_dir_element->name);

				/* load file here... */

				//resource_LoadResource(cache_path);


				//ed_explorer_open = 0;
			}
		}

	}
	gui_ImGuiEndChild();
}

void editor_UpdateExplorerUI()
{
	char option_text[512];
	char cache_path[PATH_MAX];
	dir_element_t *selected_dir_element = NULL;
	vec2_t dir_list_pos;
	vec2_t option_size;
	vec2_t window_size;
	char selected;
	int go_up = 0;
	int i;

	cached_path_t *path;
	cached_path_t *selected_cached_path;
	cached_path_t *paths;

	if(ed_explorer_open)
	{
		gui_ImGuiSetNextWindowSize(vec2(r_window_width, r_window_height - 18.0), 0);
		gui_ImGuiSetNextWindowPos(vec2(0.0, 18.0), 0, vec2(0.0, 0.0));
		gui_ImGuiPushStyleVarf(ImGuiStyleVar_WindowRounding, 0.0);
		gui_ImGuiBegin("Explorer", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
		gui_ImGuiPopStyleVar();
		{
			editor_ExplorerTopWindow();

			gui_ImGuiColumns(2, NULL, 1);
			{
				editor_ExplorerCachedDirListWindow();
			}
			gui_ImGuiNextColumn();
			{
				editor_ExplorerDirListWindow();
			}
			gui_ImGuiNextColumn();

		}
		gui_ImGuiEnd();
	}
}


void editor_OpenExplorerWindow(char *dir, int mode)
{
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

	explorer_file_mode = mode;
	/* make sure we have the file text field
	every time the explorer gets open... */
	ed_explorer_file_text_buffer[0] = '\0';
	ed_explorer_open = 1;
}

void editor_UpdateExplorerCachedPaths()
{
	/*option_t *option;
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
	}*/
}

void editor_UpdateExplorerWindow()
{
	/*int i;
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

	editor_UpdateExplorerCachedPaths();*/


}

void editor_CloseExplorerWindow()
{
	/*if(explorer_window)
	{
		gui_SetInvisible(explorer_window);
		editor_CloseExplorerDialogBox(NULL);
	}*/
}

void editor_SetExplorerFileMode(int mode)
{
	/*if(explorer_window_action_on_file_button)
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
	}*/
}

void editor_ExplorerReadFile(char *directory, char *file_name)
{

	char file_path[PATH_MAX];

	if(directory[0] && file_name[0])
	{
		editor_CachePath(directory);

		if(editor_ExplorerReadFileCallback && editor_ExplorerReadFileCallback(directory, file_name))
		{
			return;
		}

		strcpy(file_path, directory);
		strcat(file_path, "/");
		strcat(file_path, file_name);

		resource_LoadResource(file_path);
	}
}

void editor_ExplorerWriteFile(char *directory, char *file_name)
{
	FILE *file;
	void *file_buffer;
	int file_buffer_size;

	char final_path[PATH_MAX];

	if(editor_ExplorerWriteFileCallback)
	{
		editor_CachePath(directory);

		if(editor_ExplorerWriteFileCallback(directory, file_name, &file_buffer, &file_buffer_size))
		{
			strcpy(final_path, directory);
			strcat(final_path, "/");
			strcat(final_path, file_name);

            file = fopen(final_path, "wb");

			if(!file)
			{
				printf("editor_ExplorerWriteFile: couldn't open a file called [%s] for writing\n", file_name);
				return;
			}

			fwrite(file_buffer, file_buffer_size, 1, file);
			fclose(file);

			memory_Free(file_buffer);
 		}
	}
}

void editor_OpenExplorerDialogBox(char *text)
{
/*	if(explorer_dialog_box)
	{
		gui_SetVisible(explorer_dialog_box);
		gui_SetAsTop(explorer_dialog_box);
		editor_LockExplorerUI();
		editor_LockUI();
	}*/

}

void editor_CloseExplorerDialogBox()
{

/*
	if(explorer_dialog_box)
	{
		gui_SetInvisible(explorer_dialog_box);
		editor_UnlockExplorerUI();
		editor_UnlockUI();
	}*/
}

void editor_LockExplorerUI()
{
	//gui_SetIgnoreMouse(explorer_window);
}

void editor_UnlockExplorerUI()
{
//	gui_SetReceiveMouse(explorer_window);
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

//void editor_SetExplorerFileClickCallback(void (*file_click_callback)())
//{
	//editor_FileClickCallback = file_click_callback;
//}

void editor_SetExplorerReadFileCallback(int (*read_file_callback)(char *directory, char *file_name))
{
	editor_ExplorerReadFileCallback = read_file_callback;
}

void editor_SetExplorerWriteFileCallback(int (*write_file_callback)(char *file_path, char *file_name, void **file_buffer, int *file_buffer_size))
{
	editor_ExplorerWriteFileCallback = write_file_callback;
}

void editor_ClearExplorerFileCallbacks()
{
    editor_ExplorerReadFileCallback = NULL;
    editor_ExplorerWriteFileCallback = NULL;
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
		new_path = memory_Malloc(sizeof(cached_path_t));

		new_path->next = NULL;
		new_path->prev = NULL;
		new_path->path = memory_Strdup(path);
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












