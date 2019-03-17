#include "ed_ui_explorer.h"
#include "ed_ui.h"
#include "..\..\common\r_common.h"
#include "..\..\common\gui.h"
#include "..\..\common\path.h"
//#include "ed_proj.h"
#include "..\..\common\c_memory.h"
#include "..\..\common\resource.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//#include "ed_ui_texture.h"



#define EXPLORER_MAX_SELECTIONS 32

/*
******************************************************************************************
******************************************************************************************
******************************************************************************************
*/


static int ed_explorer_selected_file_count = 0;
static int ed_explorer_selected_file_read_index = 0;
int ed_explorer_open = 0;
char ed_explorer_path_text_buffer[PATH_MAX];
char ed_explorer_file_text_buffer[PATH_MAX];
static char ed_explorer_selected_files[EXPLORER_MAX_SELECTIONS][PATH_MAX];


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
extern struct renderer_t r_renderer;
//extern int r_window_width;
//extern int r_window_height;





/*
******************************************************************************************
******************************************************************************************
******************************************************************************************
*/

char full_path_to_file[1024];



void editor_InitExplorerUI()
{

}

void editor_FinishExplorerUI()
{
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
		gui_ImGuiSameLine(r_renderer.r_window_width - window_size.x * 0.4 - 150.0, -1.0);
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
		gui_ImGuiSameLine(r_renderer.r_window_width - window_size.x * 0.4 - 150.0, -1.0);
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
					//if(ed_explorer_file_text_buffer[0])
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
	window_size.x = r_renderer.r_window_width - gui_ImGuiGetColumnOffset(-1) - 20;
	window_size.y = 0.0;

	strcpy(ed_explorer_path_text_buffer, path_GetCurrentDirectory());

	gui_ImGuiBeginChild("Dirs window", window_size, 1, 0);
	{
		option_size = gui_ImGuiGetWindowSize();
		option_size.x = r_renderer.r_window_width - option_size.x - 30;
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

void editor_UpdateExplorer()
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
		gui_ImGuiSetNextWindowSize(vec2(r_renderer.r_window_width, r_renderer.r_window_height - 18.0), 0);
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












