#ifndef PATH_H
#define PATH_H

#include <limits.h>
#include <stdio.h>

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

enum DIR_ELEMEMENT_TYPE
{
	DIR_ELEMENT_TYPE_DIRECTORY = 1,
	DIR_ELEMENT_TYPE_PARENT,
	DIR_ELEMENT_TYPE_SELF,
	DIR_ELEMENT_TYPE_FILE
};

typedef struct search_paths_set_t
{
	int max_paths;
	int path_count;
	char **paths;
}search_paths_set_t;

typedef struct search_path_t
{
	char path[PATH_MAX];
}search_path_t;

typedef struct dir_element_t
{
	int type;
	char name[PATH_MAX];
}dir_element_t;


#ifdef __cplusplus
extern "C"
{
#endif

void path_Init(char *path);

void path_Finish();

void path_AddSearchPath(char *path, int type);

void path_ClearSearchPaths();

char *path_GetPathToFile(char *file_name);

int path_SetDir(char *dir);

int path_CheckDir(char *dir);

int path_CheckSubDir(char *dir);

int path_GoUp();

int path_GoDown(char *dir_name);

char *path_GetCurrentDirectory();

char *path_GetBasePath();

char *path_GetUserDocumentsDirectory();

char *path_GetFileNameFromPath(char *path);

char *path_GetFileExtension(char *file_name);

char *path_FormatPath(char *path);


FILE *path_TryOpenFile(char *file_name);


#ifdef __cplusplus
}
#endif








#endif
