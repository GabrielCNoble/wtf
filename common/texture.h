#ifndef TEXTURE_H
#define TEXTURE_H


typedef struct
{
	unsigned int gl_handle;
	int bm_flags;
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
	unsigned short target;
	int ref_count;
}texture_info_t;


typedef struct
{
	unsigned int bound_texture;
	unsigned int bound_target;
	unsigned short tex_unit;
	
}tex_unit_t;


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


int texture_Init();

void texture_Finish();

void texture_SetPath(char *path);

//int texture_LoadTexture(char *file_name, char *name, int bm_flags);

int texture_LoadTexture(char *file_name, char *name, int bm_flags);

int texture_LoadCubeTexture(char *files, char *name);

void texture_DeleteTextureByIndex(int texture_index);

int texture_GetTexture(char *name);

void texture_UploadTexture(int texture_index);

void texture_TexParameteri(int texture_index, int param, int value);






#endif
