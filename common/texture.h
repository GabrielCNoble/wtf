#ifndef TEXTURE_H
#define TEXTURE_H

#include <limits.h>

typedef struct
{
	unsigned int gl_handle;
	unsigned int bm_flags;
	
	unsigned short frame_count;
	unsigned short target;
	
}texture_t;

typedef struct
{
	char *name;
	char *file_name;
	char *full_path;
	short width;
	short height;
	short internal_format;
	short format;
	unsigned short wrap_s;
	unsigned short wrap_t;
	unsigned short min_filter;
	unsigned short mag_filter;
	unsigned char base_level;
	unsigned char max_level;
	unsigned short align;
	int ref_count;
}texture_info_t;



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


enum TEXTURE_FLAGS
{
	TEXTURE_INVALID = 1,
	TEXTURE_INVERT_X = 1 << 1,
	TEXTURE_INVERT_Y = 1 << 2,
};

enum TEXTURE_WRAP_MODE
{
	TEXTURE_REPEAT = 0,
	TEXTURE_CLAMP = 1,
};

enum TEXTURE_SCALE_FILTER
{
	TEXTURE_NEAREST = 0,
	TEXTURE_LINEAR,
	TEXTURE_NEAREST_MIPMAP_NEAREST,
	TEXTURE_LINEAR_MIPMAP_NEAREST,
	TEXTURE_NEAREST_MIPMAP_LINEAR,
	TEXTURE_LINEAR_MIPMAP_LINEAR,
};


#ifdef __cplusplus
extern "C"
{
#endif

int texture_Init();

void texture_Finish();

void texture_SetPath(char *path);

unsigned int texture_GenEmptyGLTexture(int target, int min_filter, int mag_filter, int wrap_s, int wrap_t, int wrap_r, int base_level, int max_level);

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
