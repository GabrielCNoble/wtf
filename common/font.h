#ifndef FONT_H
#define FONT_H

#include "SDL2/SDL_ttf.h"

typedef struct
{
	TTF_Font *font;
	short size;
	short align0;
	char *name;
}font_t;



int font_Init();

void font_Finish();

void font_LoadFont(char *file_name, char *name, int size);

#endif
