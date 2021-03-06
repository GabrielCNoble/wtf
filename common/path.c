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
	".eas",
	".was",
	".pas",
	NULL
};


int pth_max_dir_elements = 0;
int pth_dir_element_count = 0;
dir_element_t *pth_dir_elements = NULL;

#ifdef __cplusplus
extern "C"
{
#endif

void path_Init()
{

	int i;
	int j;

	//int path_len;




	pth_max_search_paths = 128;
	pth_search_paths = memory_Calloc(sizeof(search_path_t), pth_max_search_paths);

	//strcpy(pth_search_paths[0].path, "shaders");
	//strcpy(pth_search_paths[1].path, "shaders/engine");
	//strcpy(pth_search_paths[2].path, "sounds");
	//strcpy(pth_search_paths[3].path, "sprites");
	//strcpy(pth_search_paths[4].path, "fonts");

	//printf("%s\n", getenv("USERPROFILE"));

	//pth_search_paths[4].path[0] = '\0';
	//pth_search_paths[5].path[0] = '\0';
	//pth_search_paths[6].path[0] = '\0';
	//pth_search_paths[7].path[0] = '\0';

	//printf("path_Init: %s\n", _getdcwd(_getdrive(),NULL, 0));


	pth_search_paths += DEFAULT_SEARCH_PATH_COUNT;
	strcpy(pth_base_path, path_FormatPath(getcwd(NULL, 0)));
	strcpy(pth_user_documents_directory, path_FormatPath(getenv("USERPROFILE")));
	/* this seems to be windows only... */
	strcat(pth_user_documents_directory, "/Documents");


	pth_max_dir_elements = 1024;
	pth_dir_elements = memory_Malloc(sizeof(dir_element_t) * pth_max_dir_elements);

	path_SetDir(pth_base_path);

	path_ReadCfg(NULL);
}

void path_Finish()
{
	int i;
	int j;

	pth_search_paths -= DEFAULT_SEARCH_PATH_COUNT;
	memory_Free(pth_search_paths);
	memory_Free(pth_dir_elements);
}

void path_ReadCfg(char *path)
{
	FILE *file;
	char *file_buffer;
	int file_size;

	char file_path[PATH_MAX];
	char path_buffer[PATH_MAX];

	int path_start;
	int path_end;



	/*file = fopen("path.cfg", "r");

	if(!file)
	{
		printf("path_ReadCfg: couldn't locate the file path.cfg\n");
		return;
	}

	file_size = path_GetFileSize(file);

    file_buffer = memory_Calloc(file_size, 1);
    fread(file_buffer, file_size, 1, file);
    fclose(file);*/

    file_path[0] = '\0';

    if(path)
    {
        strcpy(file_path, path);
        strcat(file_path, "/");
    }

    strcat(file_path, "path.cfg");

    path_ReadFile(file_path, (void **)&file_buffer, &file_size);

    if(!file_size)
    {
        printf("path_ReadCfg: couldn't locate path.cfg\n");
		return;
    }

	path_ClearSearchPaths();

    path_start = 0;

    while(file_buffer[path_start])
	{
		if(file_buffer[path_start] == '[')
		{
			path_start++;
			path_end = path_start;

            while(file_buffer[path_end] != ']')
			{
                if(file_buffer[path_end] == '[' || file_buffer[path_end] == '\0')
				{
					path_start = -1;
                    break;
				}

				path_end++;
			}

			if(path_start >= 0)
			{
				file_buffer[path_end] = '\0';
				path_buffer[0] = '\0';

                if(path)
                {
                    strcpy(path_buffer, path);
                    strcat(path_buffer, "/");
                }

                strcat(path_buffer, file_buffer + path_start);

				//path_AddSearchPath(file_buffer + path_start);
				path_AddSearchPath(path_buffer);
				path_end++;
			}

			path_start = path_end;
		}
		else
		{
			path_start++;
		}
	}

	memory_Free(file_buffer);
}

void path_ReadFile(char *file_name, void **file_buffer, int *file_size)
{
    FILE *file;

    *file_buffer = NULL;
    *file_size = 0;

    int buffer_size;
    void *buffer;

    file = fopen(file_name, "rb");

    if(file)
    {
        buffer_size = path_GetFileSize(file);

        buffer = memory_Calloc(1, buffer_size);
        fread(buffer, 1, buffer_size, file);
        fclose(file);

        *file_buffer = buffer;
        *file_size = buffer_size;
    }
}

void path_WriteFile(char *file_name, void *file_buffer, int file_size)
{
    FILE *file;

    file = fopen(file_name, "wb");

    if(file)
    {
        fwrite(file_buffer, 1, file_size, file);
        fclose(file);
    }
}

int path_CopyFile(char *from, char *to)
{
    void *file_buffer = NULL;
    int file_buffer_size = 0;

    if(strcmp(from, to))
    {
        /* only copy if source and destination aren't the same... */
        path_ReadFile(from, &file_buffer, &file_buffer_size);

        if(file_buffer_size)
        {
            path_WriteFile(to, file_buffer, file_buffer_size);
            memory_Free(file_buffer);
        }
    }

    return file_buffer_size;
}


void path_AddSearchPath(char *path)
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
		paths = memory_Malloc(sizeof(search_path_t) * (pth_max_search_paths + 32));
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

	/* try to open the file by the name suplied... */
	file = fopen(file_name, "rb");


	if(file)
	{
		fclose(file);
		/* good enough... */
		return file_name;
	}


	for(i = -DEFAULT_SEARCH_PATH_COUNT; i < pth_search_path_count; i++)
	{
		if(!pth_search_paths[i].path[0])
		{
			continue;
		}

		//strcpy(path_to_file, pth_base_path);
		//strcat(path_to_file, "/");
		strcpy(path_to_file, pth_search_paths[i].path);
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

	//current_dir_len = strlen(current_dir->dd_name);
	//strcpy(pth_current_directory, path_FormatPath(current_dir->dd_name));
	current_dir_len = strlen(dir);
	strcpy(pth_current_directory, path_FormatPath(dir));
	//pth_current_directory[current_dir_len - 2] = '\0';
	//printf("path_SetDir: dir set to %s\n", pth_current_directory);


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

int path_MakeDir(char *dir)
{
    return !mkdir(dir);
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
		    /* this file name already has
		    an extension... */


		    if(ext[0] != '.')
            {
                /* if the extension string
                doesn't have a '.', step
                forward to test the extension
                of the file name without its '.' ... */
                i++;
            }

		    if(!strcmp(ret + i, ext))
            {
                /* this file name already
                has this extension, so just
                get out of here... */
                break;
            }
		}
	}

	if(ret[i] == '\0')
	{
	    /* if we got to the end
	    of the file name string, it
	    means we need to add the
	    extension... */

		if(ext[0] != '.')
		{
		    /* ext string doesn't have a '.', so add
		    one to the file name before concatenaing
		    it... */
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

char *path_GetPathSegment(char *path, int segment)
{
    static char rtrn[1024];

    //int end = 0;
    int i = 0;
    int j = 0;

    while(path[i]) i++;

    while(i)
    {
        if(path[i] == '\\' || path[i] == '/' || (!i))
        {
            /* we hit a separator (or we got to the beginning of the string)... */
            segment--;

            if(segment < 0)
            {
                /* if we're not at the end of this string,
                increment the index by one to step forward
                (and away from the separator that got us here). If
                this is the end of this string however, don't
                step forward... */
                i += (i != 0);

                while(path[i] != '\\' && path[i] != '/' && path[i] != '\0')
                {
                    rtrn[j] = path[i];
                    i++;
                    j++;
                }

                break;
            }

        }
        i--;
    }

    rtrn[j] = '\0';

    return rtrn;
}

char *path_GetLastPathSegment(char *path)
{
    return path_GetPathSegment(path, 0);
}

char *path_DropPathSegment(char *path, int segment)
{
    static char rtrn[1024];

    int end = 0;
    int i = 0;
    int j = 0;

    while(path[end]) end++;

    i = end;

    while(i)
    {
        if(path[i] == '\\' || path[i] == '/')
        {
            /* we hit a separator (or we got to the beginning of the string)... */
            segment--;

            if(segment < 0)
            {

                for(j = 0; j < i; j++)
                {
                    rtrn[j] = path[j];
                }

                i++;

                while(path[i] != '\\' && path[i] != '/' && path[i])i++;

                while(path[i])
                {
                    rtrn[j] = path[i];
                    i++;
                    j++;
                }

                break;
            }

        }
        i--;
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

int path_FileExists(char *file_name)
{

}

unsigned long path_GetFileSize(FILE *file)
{
	unsigned long file_offset;
    unsigned long file_size = 0;

    if(file)
	{
		file_offset = ftell(file);
        fseek(file, 0, SEEK_END);
		file_size = ftell(file);
		fseek(file, file_offset, SEEK_SET);
	}

	return file_size;
}

#ifdef __cplusplus
}
#endif







