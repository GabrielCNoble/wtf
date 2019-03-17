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

void path_Init();

void path_Finish();

void path_ReadCfg(char *path);

void path_ReadFile(char *file_name, void **file_buffer, int *file_size);

void path_WriteFile(char *file_name, void *file_buffer, int file_size);

int path_CopyFile(char *from, char *to);

void path_AddSearchPath(char *path);

void path_ClearSearchPaths();

char *path_GetPathToFile(char *file_name);

int path_SetDir(char *dir);

int path_MakeDir(char *dir);

int path_CheckDir(char *dir);

int path_CheckSubDir(char *dir);

int path_GoUp();

int path_GoDown(char *dir_name);

char *path_GetCurrentDirectory();

char *path_GetBasePath();

char *path_GetUserDocumentsDirectory();

char *path_GetFileNameFromPath(char *path);

char *path_GetFileExtension(char *file_name);

char *path_GetNameNoExt(char *file_name);

char *path_AddExtToName(char *file_name, char *ext);

char *path_FormatPath(char *path);

char *path_GetPathSegment(char *path, int segment);

char *path_GetLastPathSegment(char *path);

char *path_DropPathSegment(char *path, int segment);


FILE *path_TryOpenFile(char *file_name);

int path_FileExists(char *file_name);

unsigned long path_GetFileSize(FILE *file);


#ifdef __cplusplus
}
#endif








#endif
