#ifndef TEXTURE_H
#define TEXTURE_H

#include <limits.h>

#include "tex_common.h"



/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	int texture_count;

	int reserved0;
	int reserved1;
	int reserved2;
	int reserved3;
	int reserved4;
	int reserved5;
	int reserved6;
	int reserved7;

}texture_section_header_t;


typedef struct
{
	char texture_name[PATH_MAX];
	int flags;
}texture_record_t;

/*
===================================================================
===================================================================
===================================================================
*/




#ifdef __cplusplus
extern "C"
{
#endif

int texture_Init();

void texture_Finish();

void texture_SetPath(char *path);

unsigned int texture_GenGLTexture(int target, int min_filter, int mag_filter, int wrap_s, int wrap_t, int wrap_r, int base_level, int max_level);

//int texture_LoadTexture(char *file_name, char *name, int bm_flags);

int texture_LoadTexture(char *file_name, char *name, int bm_flags);

int texture_LoadCubeTexture(char *files, char *name);

void texture_DeleteTextureByIndex(int texture_index);

int texture_GetTexture(char *name);

char *texture_GetTextureName(int texture_index);

void texture_UploadTexture(int texture_index);

void texture_TexParameteri(int texture_index, int param, int value);


void texture_SerializeTextures(void **buffer, int *buffer_size);

void texture_DeserializeTextures(void **buffer);


#ifdef __cplusplus
}
#endif



#endif
