#ifndef FONT_H
#define FONT_H

#include "SDL2/SDL_ttf.h"
#include "vector.h"

#define CACHED_STRINGS_ATLAS_WIDTH 2048
#define CACHED_STRINGS_ATLAS_HEIGHT 2048
#define CACHED_STRING_MIN_WIDTH 8
#define CACHED_STRING_MIN_HEIGHT 8

typedef struct
{
	TTF_Font *font;
	short size;
	short align0;
	char *name;
}font_t;


typedef struct
{
	unsigned int gl_tex_handle;
	char *str;
	unsigned short str_len;
	unsigned short width;
	unsigned short height;
}rendered_string_t;


#ifdef __cplusplus
extern "C"
{
#endif

int font_Init();

void font_Finish();

void font_LoadFont(char *file_name, char *name, int size);

int font_RenderString(font_t *font, char *str, int line_length, vec4_t foreground, int overwrite);

int font_GetRenderedString(char *str);

void font_DropRenderedString(int index);

#ifdef __cplusplus
}
#endif

#endif





