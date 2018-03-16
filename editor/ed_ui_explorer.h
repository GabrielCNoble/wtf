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


void editor_InitExplorerUI();

void editor_OpenExplorerWindow(char *dir);

void editor_UpdateExplorerWindow();

void editor_CloseExplorerWindow();


void editor_ExplorerPathField(int enable);

void editor_ExplorerFileNameField(int enable);

void editor_ExplorerMultiFileSelection(int enable);






void editor_SetExplorerFileClickCallback(void (*file_click_callback)());

void editor_AddExplorerExtensionFilter(char *ext);

void editor_ClearExplorerExtensionFilters();

void editor_UpdateExplorerUI();



void editor_AddExplorerFileSelection(char *file);

void editor_RemoveExplorerFileSelection(char *file);

void editor_ClearExplorerFileSelections();

int editor_GetExplorerSelectedFileCount();

char **editor_GetExplorerSelectedFiles();

char *editor_GetExplorerNextSelectedFile();

void editor_RewindExplorerSelectedFiles();




#endif
