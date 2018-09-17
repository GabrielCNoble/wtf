#include "resource.h"
#include "path.h"
#include "ent_common.h"
#include "w_common.h"
#include "world.h"
#include "entity.h"
#include "sound.h"
#include "log.h"

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

	file_exts[RESOURCE_TYPE_SOUND][0] = "wav";
	file_exts[RESOURCE_TYPE_SOUND][1] = "ogg";


	file_exts[RESOURCE_TYPE_MODEL][0] = "mpk";

	file_exts[RESOURCE_TYPE_ENTITY][0] = ENTITY_FILE_EXTENSION;


	//file_exts[RESOURCE_TYPE_SCRIPT][0] = "as";
	file_exts[RESOURCE_TYPE_SCRIPT][0] = ENTITY_SCRIPT_FILE_EXTENSION;
	file_exts[RESOURCE_TYPE_SCRIPT][1] = WORLD_SCRIPT_FILE_EXTENSION;

	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "%s: subsystem initialized properly!", __func__);

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

	struct script_t *(*load_script_fn)(char *file_name, char *script_name);

	struct entity_handle_t entity_handle;
	struct sound_handle_t sound_handle;

	int success = 0;

	ext = path_GetFileExtension(file_name);

	if(!ext[0])
	{
		printf("resource_LoadResource: file name without extension; cannot determine the type of the resource\n");
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
						if(texture_LoadTexture(file_name, path_GetNameNoExt(path_GetFileNameFromPath(file_name)), 0) != -1)
						{
							success = 1;
						}
					break;

					case RESOURCE_TYPE_MODEL:
						if(model_LoadModel(file_name, path_GetNameNoExt(path_GetFileNameFromPath(file_name))) != -1)
						{
							success = 1;
						}
					break;

					case RESOURCE_TYPE_SOUND:

						sound_handle = sound_LoadSound(file_name, path_GetNameNoExt(path_GetFileNameFromPath(file_name)));

						if(sound_handle.sound_index != INVALID_SOUND_INDEX)
						{
							success = 1;
						}
					break;

					case RESOURCE_TYPE_ENTITY:

						entity_handle = entity_LoadEntityDef(file_name);

                        if(entity_handle.entity_index != INVALID_ENTITY_INDEX)
						{
                            success = 1;
						}
					break;

					case RESOURCE_TYPE_SCRIPT:

						if(!strcmp(ext, ENTITY_SCRIPT_FILE_EXTENSION))
						{
							load_script_fn = (struct script_t *(*)(char *, char *))entity_LoadScript;
						}
						else if(!strcmp(ext, WORLD_SCRIPT_FILE_EXTENSION))
                        {
                            load_script_fn = (struct script_t *(*)(char *, char *))world_LoadScript;
                        }

						if(load_script_fn(file_name, path_GetNameNoExt(path_GetFileNameFromPath(file_name))))
						{
                            success = 1;
						}
					break;
				}

				if(success)
				{
					printf("resource_LoadResource: file [%s] loaded successfully\n", file_name);
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




