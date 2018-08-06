#include "path.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <io.h>

#include "c_memory.h"


#define SEARCH_PATH_MAX_LEN 512
#define DEFAULT_SEARCH_PATH_COUNT 8

static char pth_base_path[512];
static char pth_current_directory[512];
static char pth_user_documents_directory[512];



//char current_directory[512];
//static int base_path_len;
//struct search_paths_set_t *path_sets;
//char *path_ext_table[SEARCH_PATH_ALL][15];


int pth_max_search_paths = 0;
int pth_search_path_count = 0;
search_path_t *pth_search_paths = NULL;

char *vis_file_exts[] =
{
	".png",
	".jpeg",
	".jpg",
	".tga",
	".bmp"
	".obj",
	".mpk",
	".wav",
	".mp3",
	".ogg",
	".wtf",
	".vsf",
	NULL
};


int pth_max_dir_elements = 0;
int pth_dir_element_count = 0;
dir_element_t *pth_dir_elements = NULL;

#ifdef __cplusplus
extern "C"
{
#endif

void path_Init(char *path)
{

	int i;
	int j;

	//int path_len;




	pth_max_search_paths = 128;
	pth_search_paths = memory_Malloc(sizeof(search_path_t) * (pth_max_search_paths + DEFAULT_SEARCH_PATH_COUNT), "path_Init");

	strcpy(pth_search_paths[0].path, "shaders");
	strcpy(pth_search_paths[1].path, "shaders/engine");
	strcpy(pth_search_paths[2].path, "sounds");
	strcpy(pth_search_paths[3].path, "sprites");
	strcpy(pth_search_paths[4].path, "fonts");

	//printf("%s\n", getenv("USERPROFILE"));

	pth_search_paths[4].path[0] = '\0';
	pth_search_paths[5].path[0] = '\0';
	pth_search_paths[6].path[0] = '\0';
	pth_search_paths[7].path[0] = '\0';

	printf("path_Init: %s\n", _getdcwd(_getdrive(),NULL, 0));


	pth_search_paths += DEFAULT_SEARCH_PATH_COUNT;
	strcpy(pth_base_path, path_FormatPath(getcwd(NULL, 0)));
	strcpy(pth_user_documents_directory, path_FormatPath(getenv("USERPROFILE")));
	strcat(pth_user_documents_directory, "/Documents");


	pth_max_dir_elements = 1024;
	pth_dir_elements = memory_Malloc(sizeof(dir_element_t) * pth_max_dir_elements, "path_Init");

	path_SetDir(pth_base_path);

}

void path_Finish()
{
	int i;
	int j;

	pth_search_paths -= DEFAULT_SEARCH_PATH_COUNT;
	memory_Free(pth_search_paths);
	memory_Free(pth_dir_elements);
}


void path_AddSearchPath(char *path, int type)
{
	int i;

	char *formated_path;
	search_path_t *paths;

	formated_path = path_FormatPath(path);

	for(i = 0; i < pth_search_path_count; i++)
	{
		if(!strcmp(formated_path, pth_search_paths[i].path))
		{
			printf("path_AddSearchPath: search path %s already exists!", path);
			return;
		}
	}

	if(pth_search_path_count >= pth_max_search_paths)
	{
		paths = memory_Malloc(sizeof(search_path_t) * (pth_max_search_paths + 32), "path_AddSearchPath");
		memcpy(paths, pth_search_paths, sizeof(search_path_t) * pth_max_search_paths);
		memory_Free(pth_search_paths);
		pth_search_paths = paths;
		pth_max_search_paths += 32;
	}

	strcpy(pth_search_paths[pth_search_path_count].path, formated_path);
	pth_search_path_count++;
}

void path_ClearSearchPaths()
{
	int i;
	int j;

	pth_search_path_count = 0;
	//search_paths_set_t *path_set;

	/*for(i = 0; i < SEARCH_PATH_ALL; i++)
	{
		path_set = path_sets + i;
		for(j = 0; j < path_set->path_count; j++)
		{
			printf("freed search path [%s]\n", path_set->paths[j]);
			memory_Free(path_set->paths[j]);
		}

		path_set->path_count = 0;
	}*/

}

//static char path_to_file[1024];
char *path_GetPathToFile(char *file_name)
{
	int i;
	FILE *file;

	static char path_to_file[1024];

	for(i = -DEFAULT_SEARCH_PATH_COUNT; i < pth_search_path_count; i++)
	{
		if(!pth_search_paths[i].path[0])
		{
			continue;
		}

		strcpy(path_to_file, pth_base_path);
		strcat(path_to_file, "/");
		strcat(path_to_file, pth_search_paths[i].path);
		strcat(path_to_file, "/");
		strcat(path_to_file, file_name);

		file = fopen(path_to_file, "rb");

		if(file)
		{
			fclose(file);
			return path_to_file;
		}
	}

	return NULL;
}


int path_SetDir(char *dir)
{
	struct dirent *element;
	DIR *current_dir;
	DIR *probe;
	char dir_path[1024];
	current_dir = opendir(dir);
	int current_dir_len;
	int i;

	if(!current_dir)
	{
		printf("path_SetDir: couldn't change to dir %s\n", dir);
		return 0;
	}

	pth_dir_element_count = 0;

	current_dir_len = strlen(current_dir->dd_name);
	strcpy(pth_current_directory, path_FormatPath(current_dir->dd_name));
	pth_current_directory[current_dir_len - 2] = '\0';
	printf("path_SetDir: dir set to %s\n", pth_current_directory);


	element = readdir(current_dir);
	if(element)
	{
		do
		{
			if(element->d_name[0] == '.')
			{
				if(element->d_name[1] == '.')
				{
					strcpy(pth_dir_elements[pth_dir_element_count].name, "..");
					pth_dir_elements[pth_dir_element_count].type = DIR_ELEMENT_TYPE_PARENT;
					pth_dir_element_count++;
				}
				continue;
			}

			strcpy(dir_path, pth_current_directory);
			strcat(dir_path, "/");
			strcat(dir_path, element->d_name);
			probe = opendir(dir_path);

			if(probe)
			{
				closedir(probe);
				pth_dir_elements[pth_dir_element_count].type = DIR_ELEMENT_TYPE_DIRECTORY;
			}
			else
			{
				pth_dir_elements[pth_dir_element_count].type = DIR_ELEMENT_TYPE_FILE;
			}

			strcpy(pth_dir_elements[pth_dir_element_count].name, element->d_name);
			pth_dir_element_count++;

		}while(element = readdir(current_dir));
	}
	else
	{
		strcpy(pth_dir_elements[0].name, "..");
		pth_dir_elements[0].type = DIR_ELEMENT_TYPE_PARENT;
		pth_dir_element_count = 1;
	}

	closedir(current_dir);

	return 1;
}


int path_CheckDir(char *dir)
{
	DIR *d;
	d = opendir(dir);

	if(d)
	{
		closedir(d);
		return 1;
	}

	return 0;
}

int path_CheckSubDir(char *dir)
{

	DIR *d;
	char full_path[512];


	strcpy(full_path, pth_current_directory);
	strcat(full_path, "/");
	strcat(full_path, dir);

	return path_CheckDir(full_path);
}

int path_GoUp()
{
	int i;
	i = strlen(pth_current_directory);

	while(pth_current_directory[i] != '/' && i > 0)
	{
		i--;
	}

	if(i)
	{
		pth_current_directory[i] = '\0';
		return path_SetDir(pth_current_directory);
	}

	return 0;
}


//char path_GoDown_Temp[1024];
int path_GoDown(char *sub_dir)
{

	static char temp[1024];

	if(!strcmp(sub_dir, ".."))
	{
		return 0;
	}

	if(!strcmp(sub_dir, "."))
	{
		return 0;
	}

	strcpy(temp, pth_current_directory);
	strcat(temp, "/");
	strcat(temp, sub_dir);

	return path_SetDir(temp);
}


char *path_GetCurrentDirectory()
{
	return pth_current_directory;
}

char *path_GetBasePath()
{
	return pth_base_path;
}

char *path_GetUserDocumentsDirectory()
{
	return pth_user_documents_directory;
}

char *path_GetFileNameFromPath(char *path)
{
	int i;
	int len;

	if(path)
	{
		if(path[0])
		{
			len = strlen(path);

			while(path[len] != '\\' && path[len] != '/' && len) len--;

			if(len)
				len++;

			return path + len;
		}
	}

	return NULL;
}

//static char path_GetFileExtension_Return[32];

char *path_GetFileExtension(char *file_name)
{
	int i;

	static char rtrn[32];

	i = strlen(file_name);

	while(file_name[i] != '.' && i > 0) i--;

	if(i)
	{
		i++;
		strcpy(rtrn, file_name + i);
	}
	else
	{
		rtrn[0] = '\0';
	}

	return rtrn;
}

char *path_GetNameNoExt(char *file_name)
{
	int i = 0;

	static char rtrn[1024];

	while(file_name[i] && file_name[i] != '.')
	{
		i++;
	}

	if(!file_name[i])
	{
		return file_name;
	}
	else
	{
		memcpy(rtrn, file_name, i);
		rtrn[i] = '\0';
	}

	return rtrn;
}



char *path_AddExtToName(char *file_name, char *ext)
{
	static char ret[1024];

	int i;

	strcpy(ret, file_name);

	for(i = 0; ret[i] != '\0'; i++)
	{
		if(ret[i] == '.')
		{
			break;
		}
	}

	if(ret[i] == '\0')
	{
		if(ext[0] != '.')
		{
			strcat(ret, ".");
		}

		strcat(ret, ext);
	}

	return ret;
}

char *path_FormatPath(char *path)
{
	int i;
	int j;

	static char rtrn[1024];

	i = 0;
	j = 0;

	while(path[i])
	{

		if(path[i] == '\\')
		{
			if(path[i + 1] == '\\')
			{
				i++;
			}
		}

		rtrn[j] = path[i];

		if(rtrn[j] == '\\')
		{
			rtrn[j] = '/';
		}

		i++;
		j++;

	}

	rtrn[j] = '\0';

	return rtrn;
}



FILE *path_TryOpenFile(char *file_name)
{
	FILE *file = NULL;
	char *file_path;


	/* first try to open the file
	by it's file name... */

	file = fopen(file_name, "rb");

	if(!file)
	{
		file_path = path_GetPathToFile(file_name);

		if(file_path)
		{
			file = fopen(file_path, "rb");
		}
	}

	return file;
}

#ifdef __cplusplus
}
#endif







