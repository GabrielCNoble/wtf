#ifndef PATH_H
#define PATH_H


enum SEARCH_PATH_TYPE
{
	SEARCH_PATH_TEXTURE = 0,
	SEARCH_PATH_MATERIAL,
	SEARCH_PATH_SHADER,
	SEARCH_PATH_SOUND,
	SEARCH_PATH_FONT,
	SEARCH_PATH_SAVE,
	SEARCH_PATH_MODEL,
	SEARCH_PATH_ALL,
};

enum DIR_ELEM_TYPE
{
	DIR_ELEM_DIRECTORY = 1,
	DIR_ELEM_GO_UP,
	DIR_ELEM_SELF,
	DIR_ELEM_FILE
};

typedef struct search_paths_t
{
	int max_paths;
	int path_count;
	char **paths;
}search_paths_t;

typedef struct dir_element_t
{
	char *name;
	int type;
}dir_element_t;


void path_Init(char *path);

void path_Finish();

void path_AddSearchPath(char *path, int type);

void path_ClearSearchPaths();

char *path_GetPathToFile(char *file_name);

int path_SetDir(char *dir);

int path_CheckDir(char *dir);

int path_CheckSubDir(char *dir);

void path_GoUp();

void path_GoDown(char *dir_name);

char *path_GetCurrentDirectory();

char *path_GetBasePath();

char *path_GetFileNameFromPath(char *path);

char *path_FormatPath(char *path);









#endif
