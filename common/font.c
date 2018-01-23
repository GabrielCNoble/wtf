#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "font.h"

static char font_path[256];

int max_fonts;
int font_count;
font_t *fonts;

font_t *gui_font = NULL;

int font_Init()
{
	if(TTF_Init() < 0)
	{
		printf("couldn't initialize SDL_ttf!\n");
		exit(-1);
	}
	
	max_fonts = 16;
	font_count = 0;
	fonts = malloc(sizeof(font_t ) * max_fonts);
	
	
	font_LoadFont("..\\common\\fonts\\consola.ttf", "gui", 16);
	
	gui_font = &fonts[0];
	
	return 1;
	
}

void font_Finish()
{
	int i;
	for(i = 0; i < font_count; i++)
	{
		free(fonts[i].name);
		TTF_CloseFont(fonts[i].font);
	}
	free(fonts);
	
	TTF_Quit();
}

void font_LoadFont(char *file_name, char *name, int size)
{
	//char full_path[256];
	
	font_t *font;
	/*
	strcpy(full_path, font_path);
	strcat(full_path, "\\");
	strcat(full_path, file_name);*/
	
	font = &fonts[font_count];
	
	font->font = TTF_OpenFont(file_name, size);
	font->size = size;
	font->name = strdup(name);
	font_count++;
	
}



