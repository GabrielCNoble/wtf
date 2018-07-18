#include "resource_loader.h"
#include "path.h"

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_RESORCE_EXTS 8
char *file_exts[RESOURCE_TYPE_LAST][MAX_RESORCE_EXTS] = {NULL};


int resource_Init()
{
	
	file_exts[RESOURCE_TYPE_TEXTURE][0] = "png";
	file_exts[RESOURCE_TYPE_TEXTURE][1] = "jpg";
	file_exts[RESOURCE_TYPE_TEXTURE][2] = "tga";
	
	
	file_exts[RESOURCE_TYPE_MODEL][0] = "mpk";
	
	return 1;
}

void resource_Finish()
{
	
}

void resource_LoadResource(char *file_name)
{
	int res_type;
	int ext_index;
	char *ext;
	
	int success = 0;
	
	ext = path_GetFileExtension(file_name);
	
	if(!ext[0])
	{
		printf("resource_LoadResource: file name without extention; cannot determine the type of the resource\n");
		return;
	}
	
	for(res_type = RESOURCE_TYPE_TEXTURE; res_type < RESOURCE_TYPE_LAST; res_type++)
	{
		for(ext_index = 0; file_exts[res_type][ext_index]; ext_index++)
		{
			if(!strcmp(ext, file_exts[res_type][ext_index]))
			{
				switch(res_type)
				{
					case RESOURCE_TYPE_TEXTURE:
						if(texture_LoadTexture(file_name, path_GetFileNameFromPath(file_name), 0) != -1)
						{
							success = 1;
						}
					break;
					
					case RESOURCE_TYPE_MODEL:
						if(model_LoadModel(file_name, path_GetFileNameFromPath(file_name)) != -1)
						{
							success = 1;
						}
					break;
				}
				
				if(success)
				{
					printf("resource_LoadResource: file [%s] loaded succesfully\n", file_name);
				}
				else
				{
					printf("resource_LoadResource: couldn't load file [%s]\n", file_name);
				}
				
				return;
			}
		}
	}
	
	printf("resurce_LoadResource: file [%s] is of unknown type\n", file_name);
}

#ifdef __cplusplus
}
#endif




