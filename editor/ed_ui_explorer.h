#ifndef ED_UI_EXPLORER
#define ED_UI_EXPLORER


enum EXPLORER_TYPE
{
	EXPLORER_SAVE_PROJECT_FILE = 1,
	EXPLORER_OPEN_PROJECT_FILE,
	EXPLORER_OPEN_TEXTURE_FILE,
	EXPLORER_OPEN_SOUND_FILE,
	EXPLORER_OPEN_MODEL_FILE,
};

enum EXPLORER_FILE_MODE
{
	EXPLORER_FILE_MODE_READ = 0,
	EXPLORER_FILE_MODE_WRITE,
};

typedef struct cached_path_t
{
	struct cached_path_t *next;
	struct cached_path_t *prev;


	char *path;
	int refs;
}cached_path_t;


/*
*********************************************
*********************************************
*********************************************
*/
void editor_InitExplorerUI();

void editor_FinishExplorerUI();

void editor_UpdateExplorer();
/*
*********************************************
*********************************************
*********************************************
*/


/*
*********************************************
*********************************************
*********************************************
*/
void editor_OpenExplorerWindow(char *dir, int mode);

void editor_UpdateExplorerCachedPaths();

void editor_UpdateExplorerWindow();

void editor_CloseExplorerWindow();

void editor_SetExplorerFileMode(int mode);



void editor_ExplorerReadFile(char *directory, char *file_name);

void editor_ExplorerWriteFile(char *directory, char *file_name);


/*
*********************************************
*********************************************
*********************************************
*/
void editor_SetExplorerFileClickCallback(void (*file_click_callback)());

void editor_SetExplorerReadFileCallback(int (*read_file_callback)(char *directory, char *file_name));

void editor_SetExplorerWriteFileCallback(int (*write_file_callback)(char *file_path, char *file_name, void **file_buffer, int *file_buffer_size));

void editor_ClearExplorerFileCallbacks();

void editor_AddExplorerExtensionFilter(char *ext);

void editor_ClearExplorerExtensionFilters();
/*
*********************************************
*********************************************
*********************************************
*/


void editor_AddExplorerFileSelection(char *file);

void editor_RemoveExplorerFileSelection(char *file);

void editor_ClearExplorerFileSelections();

int editor_GetExplorerSelectedFileCount();

char **editor_GetExplorerSelectedFiles();
/*
*********************************************
*********************************************
*********************************************
*/



/*
*********************************************
*********************************************
*********************************************
*/
void editor_CachePath(char *path);

cached_path_t *editor_GetCachedPath(char *path);

void editor_DropCachedPath(char *path);

void editor_ClearCachedPaths();
/*
*********************************************
*********************************************
*********************************************
*/





#endif
