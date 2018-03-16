#include "path.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>


#define SEARCH_PATH_MAX_LEN 512

static char base_path[512];
char *current_directory;
static int base_path_len;
struct search_paths_t *paths;
char *path_ext_table[SEARCH_PATH_ALL][15];

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
 

int max_dir_elements = 0;
int dir_elements_count = 0;
dir_element_t *dir_elements = NULL;

void path_Init(char *path)
{
	
	int i;
	int j;
	
	int path_len;
	
	path_len = strlen(path);
	
	for(i = path_len; i > 0; i--)
	{
		if(path[i] == '\\' || path[i] == '/')
		{
			break;
		}
	}
	
	if(!i)
	{
		i = path_len;
	}
	
	
	current_directory = calloc(512, 1);
	
	//memcpy(base_path, path, i);
	
	for(j = 0; j < i; j++)
	{
		base_path[j] = path[j];
	}
	
	base_path[j] = '\0';
	current_directory[0] = '\0';
	base_path_len = strlen(base_path);
	
	path_len = i;
	
	for(i = 0; i < path_len; i++)
	{
		if(base_path[i] == '\\')
		{
			base_path[i] = '/';
		}
	}
	
	//printf("%s\n", base_path);
	
	
	paths = malloc(sizeof(struct search_paths_t) * SEARCH_PATH_ALL);
	
	
	for(i = 0; i < SEARCH_PATH_ALL; i++)
	{
		paths[i].max_paths = 16;
		paths[i].path_count = 0;
		paths[i].paths = malloc(sizeof(char *) * (paths[i].max_paths + 1));
		paths[i].paths[0] = NULL;
		paths[i].paths++;
	}
	
	
	paths[SEARCH_PATH_SHADER].paths[-1] = strdup("shaders");
	
	path_ext_table[SEARCH_PATH_TEXTURE][0] = ".png"; 
	path_ext_table[SEARCH_PATH_TEXTURE][1] = ".jpg";
	path_ext_table[SEARCH_PATH_TEXTURE][2] = ".bmp";
	path_ext_table[SEARCH_PATH_TEXTURE][3] = ".tga";
	path_ext_table[SEARCH_PATH_TEXTURE][4] = NULL;
	
	path_ext_table[SEARCH_PATH_MATERIAL][0] = NULL;
	
	path_ext_table[SEARCH_PATH_SHADER][0] = ".vert";
	path_ext_table[SEARCH_PATH_SHADER][1] = ".frag";
	path_ext_table[SEARCH_PATH_SHADER][2] = NULL;
	
	path_ext_table[SEARCH_PATH_FONT][0] = ".ttf";
	path_ext_table[SEARCH_PATH_FONT][1] = NULL;
	
	path_ext_table[SEARCH_PATH_SOUND][0] = ".wav";
	path_ext_table[SEARCH_PATH_SOUND][1] = ".mp3";
	path_ext_table[SEARCH_PATH_SOUND][2] = ".ogg";
	path_ext_table[SEARCH_PATH_SOUND][3] = NULL;
	
	path_ext_table[SEARCH_PATH_SAVE][0] = ".vsf";
	path_ext_table[SEARCH_PATH_SAVE][1] = NULL;
	
	//path_ext_table[SEARCH_PATH_MODEL][0] = ".obj";
	path_ext_table[SEARCH_PATH_MODEL][0] = ".mpk";
	path_ext_table[SEARCH_PATH_MODEL][1] = NULL;
	
	
	max_dir_elements = 4096;
	dir_elements = malloc(sizeof(dir_element_t) * max_dir_elements);
	
	
	for(i = 0; i < max_dir_elements; i++)
	{
		dir_elements[i].name = malloc(PATH_MAX);
	}
	
}

void path_Finish()
{
	int i;
	int j;
	
	struct search_paths_t *search_paths;
	
	for(i = 0; i < SEARCH_PATH_ALL; i++)
	{
		search_paths = paths + i;
		
		for(j = -1; j < search_paths->path_count; j++)
		{
			if(search_paths->paths[j])
				free(search_paths->paths[j]);
		}
		
		search_paths->paths--;
		
		free(search_paths->paths);
	}
	
	for(i = 0; i < max_dir_elements; i++)
	{
		free(dir_elements[i].name);
	}
	
	free(dir_elements);
}


void path_AddSearchPath(char *path, int type)
{
	struct search_paths_t *search_paths;
	char **c;
	int path_len;
	
	switch(type)
	{
		
		case SEARCH_PATH_TEXTURE:
		case SEARCH_PATH_SHADER:
		case SEARCH_PATH_FONT:
		case SEARCH_PATH_MATERIAL:
		case SEARCH_PATH_SOUND:
		case SEARCH_PATH_SAVE:
		case SEARCH_PATH_MODEL:
			
			search_paths = paths + type;
			
			if(search_paths->path_count >= search_paths->max_paths)
			{
				c = malloc(sizeof(char *) * (search_paths->max_paths + 16));
				memcpy(c, search_paths->paths, sizeof(char *) * search_paths->max_paths);
				free(search_paths->paths);
				
				search_paths->paths = c;
				search_paths->max_paths += 16;
			}
			
		break;
		
		
		default:
			return;
		
	}
	
	path_len = strlen(path) + 2;
	
	//search_paths->paths[search_paths->path_count] = strdup(path);
	search_paths->paths[search_paths->path_count] = malloc(base_path_len + path_len);
	strcpy(search_paths->paths[search_paths->path_count], base_path);
	strcat(search_paths->paths[search_paths->path_count], "/");
	strcat(search_paths->paths[search_paths->path_count], path);
	search_paths->path_count++;
	//search_path->path = strdup(path);
	//search_path->type = type;
}

void path_ClearSearchPaths()
{
	int i;
	int j;
	
	search_paths_t *path_set;
	
	for(i = 0; i < SEARCH_PATH_ALL; i++)
	{
		path_set = paths + i;
		
		for(j = 0; j < path_set->path_count; j++)
		{
			free(path_set->paths[j]);
		}
		
		path_set->path_count = 0;
	}
	
}

static char full_path[SEARCH_PATH_MAX_LEN];

char *path_GetPathToFile(char *file_name)
{
	FILE *probe;
	int str_len;
	int i;
	int j;
	int path_type;
	int path_ext;
	char ext[32];
	
	
	full_path[0] = '\0';
	
	int search_path_count;
	struct search_paths_t *search_paths;
	
	if(!file_name)
	{
		log_LogMessage(LOG_MESSAGE_ERROR, "path_GetPathToFile: empty file name string!");
		return NULL;
	}
	
	str_len = strlen(file_name);
	
	while(str_len >= 0 && file_name[str_len] != '.') str_len--;
	
	if(!str_len)
	{
		log_LogMessage(LOG_MESSAGE_ERROR, "path_GetPathToFile: file [%s] has no extension!\n", file_name);
		return NULL;
	}
	
	str_len--;
	i = -1;
	do
	{
		str_len++;
		i++;
		
 		ext[i] = file_name[str_len];	
	}while(file_name[str_len] != '\0');
	
	for(path_type = 0; path_type < SEARCH_PATH_ALL; path_type++)
	{
		path_ext = 0;
		while(path_ext_table[path_type][path_ext])
		{
			if(!strcmp(ext, path_ext_table[path_type][path_ext]))
			{
				break;
			}
			
			path_ext++;
		}
		
 		if(path_ext_table[path_type][path_ext])
			break;
	}
	
	
	switch(path_type)
	{
		case SEARCH_PATH_TEXTURE:
		case SEARCH_PATH_SHADER:
		case SEARCH_PATH_FONT:
		case SEARCH_PATH_MATERIAL:
		case SEARCH_PATH_SOUND:
		case SEARCH_PATH_SAVE:
		case SEARCH_PATH_MODEL:
		
			search_paths = paths + path_type;
		break;
		
		default:
			return NULL;
	}
	
	
	for(i = -1; i < search_paths->path_count; i++)
	{
		if(!search_paths->paths[i])
			continue;
		
		strcpy(full_path, search_paths->paths[i]);
		strcat(full_path, "/");
		strcat(full_path, file_name);
		
		
		if((probe = fopen(full_path, "rb")))
		{
			fclose(probe);		
			return full_path;
		}
	}
	
	return NULL;
}


int path_SetDir(char *dir)
{
	DIR *d;
	DIR *probe;
	struct dirent *e;
	dir_element_t *de;
	
	char dir_path[512];
	int dir_path_len;
	int element_path_len;
	int i;
	
	d = opendir(dir);
	
	
	if(!d)
	{
		printf("path_SetDir: couldn't change to dir %s\n", dir);
		return 0;
	}
	
	
	dir_path_len = strlen(dir);
	strcpy(dir_path, dir);
	
	for(i = 0; i < dir_path_len; i++)
	{
		if(dir_path[i] == '\\')
		{
			dir_path[i] = '/';
		}
	}
	
	i = dir_path_len;
	while(dir_path[i] == ' ') i--;
	
	if(dir_path[i] == '/')
	{
		dir_path_len = i;
	}
	
	dir_path[dir_path_len] = '\0';
	dir_elements_count = 0;
	
	strcpy(current_directory, dir_path);
	
	e = readdir(d);
	
	if(e)
	{
		do
		{
			
			
			if(dir_elements_count >= max_dir_elements)
			{
				de = malloc(sizeof(dir_element_t) * (max_dir_elements + 1024));
				memcpy(de, dir_elements, sizeof(dir_element_t) * max_dir_elements);
				free(dir_elements);
				dir_elements = de;
				max_dir_elements += 1024;
				
				for(i = dir_elements_count; i < max_dir_elements; i++)
				{
					dir_elements[i].name = malloc(PATH_MAX);
				}
				
			}
			
			
			
			dir_path[dir_path_len] = '\0';
			strcat(dir_path, "/");
			strcat(dir_path, e->d_name);
			
			probe = opendir(dir_path);
			
			if(probe)
			{
				closedir(probe);
				if(e->d_name[0] == '.')
				{
					if(e->d_name[1] == '.')
					{
						dir_elements[dir_elements_count].type = DIR_ELEM_GO_UP;
					}
					else
					{
						dir_elements[dir_elements_count].type = DIR_ELEM_SELF;
					}
					
				}
				else
				{
					dir_elements[dir_elements_count].type = DIR_ELEM_DIRECTORY;
				}
				
			}
			else
			{
				element_path_len = e->d_namlen;
				while(e->d_name[element_path_len] != '.' && element_path_len > 0) element_path_len--;
				
				i = 0;
				while(vis_file_exts[i])
				{
					if(!strcmp(vis_file_exts[i], e->d_name + element_path_len))
					{
						break;
					}
					
					i++;
				}
			
				if(!vis_file_exts[i])
					continue;
				
				dir_elements[dir_elements_count].type = DIR_ELEM_FILE;
			}
			
			strcpy(dir_elements[dir_elements_count].name, e->d_name);	
			dir_elements_count++;
			
			
		}while(e = readdir(d));
	}
	else
	{
		strcpy(dir_elements[0].name, "."); 
		dir_elements[0].type = DIR_ELEM_SELF;
		
		strcpy(dir_elements[1].name, "..");
		dir_elements[1].type = DIR_ELEM_GO_UP;
		
		dir_elements_count = 2;
	}
	
	
	
	dir_path[dir_path_len] = '\0';
	
	closedir(d);
	
	
	return 1;
	
	/*printf("current directory: %s\n", current_directory);
	for(i = 0; i < dir_elements_count; i++)
	{
		switch(dir_elements[i].type)
		{
			case DIR_ELEM_DIRECTORY:
				printf("<DIR> %s\n", dir_elements[i].name);
			break;
			
			case DIR_ELEM_FILE:
				printf("<FILE> %s\n", dir_elements[i].name);
			break;
		}
	}*/
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
	
	
	strcpy(full_path, current_directory);
	strcat(full_path, "/");
	strcat(full_path, dir);
	
	return path_CheckDir(full_path);
}

void path_GoUp()
{
	int i;
	int current_dir_len;
	
	current_dir_len = strlen(current_directory);
	
	while(current_directory[current_dir_len] != '/' && current_dir_len > 0) current_dir_len--;
	
	if(current_dir_len)
	{
		current_directory[current_dir_len] = '\0';
		path_SetDir(current_directory);
	}
}



void path_GoDown(char *dir_name)
{
	
	if(dir_name)
	{
		if(dir_name[0])
		{
			strcat(current_directory, "/");
			strcat(current_directory, dir_name);
			path_SetDir(current_directory);
		}
		
	}
	
	
}


char *path_GetCurrentDirectory()
{
	return current_directory;
}

char *path_GetBasePath()
{
	return base_path;
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

static char formated_path[1024];

char *path_FormatPath(char *path)
{
	int i;
	int j;
	
	
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
		
		formated_path[j] = path[i];
		
		if(formated_path[j] == '\\')
		{
			formated_path[j] = '/';
		}
		
		i++;
		j++;
		
	}
	
	formated_path[j] = '\0';
	
	return formated_path;
}









