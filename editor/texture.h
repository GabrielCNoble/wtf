#ifndef TEXTURE_H
#define TEXTURE_H


typedef struct
{
	unsigned int gl_handle;
	short texture_index;
	unsigned short texture_unit;
	unsigned short tex_type;
}texture_t;


typedef struct
{
	unsigned int bound_texture;
	unsigned int bound_target;
	unsigned short tex_unit;
	
}tex_unit_t;


void texture_Init();

void texture_Finish();

int texture_LoadTexture(char *file_name, char *name);

int texture_LoadCubeTexture(char *files, char *name);

void texture_DeleteTextureByIndex(int texture_index);

int texture_GetTexture(char *name);

void texture_BindTexture(int texture_index, int tex_unit);






#endif
