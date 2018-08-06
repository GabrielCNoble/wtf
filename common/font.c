#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "font.h"

#include "path.h"
#include "c_memory.h"

#include "SDL2/SDL_surface.h"
#include "GL/glew.h"

static char font_path[256];

int max_fonts;
int font_count;
font_t *fonts;
font_t *gui_font = NULL;


int ft_rendered_string_count = 0;
int ft_max_rendered_strings = 0;
int ft_free_stack_top = -1;
int *ft_free_stack = NULL;
rendered_string_t *ft_rendered_strings = NULL;


#ifdef __cplusplus
extern "C"
{
#endif

int font_Init()
{

	char *full_path;

	if(TTF_Init() < 0)
	{
		printf("couldn't initialize SDL_ttf!\n");
		exit(-1);
	}

	max_fonts = 16;
	font_count = 0;
	fonts = memory_Malloc(sizeof(font_t ) * max_fonts, "font_Init");


	ft_max_rendered_strings = 512;
	ft_free_stack = memory_Malloc(sizeof(int) * ft_max_rendered_strings, "font_Init");
	ft_rendered_strings = memory_Malloc(sizeof(rendered_string_t) * ft_max_rendered_strings, "font_Init");


	full_path = path_GetPathToFile("consola.ttf");
	font_LoadFont(full_path, "gui", 16);
	gui_font = &fonts[0];

	font_RenderString(gui_font, "motherfucking test", 0, vec4(1.0, 0.0, 1.0, 1.0), 0);

	return 1;

}

void font_Finish()
{
	int i;
	for(i = 0; i < font_count; i++)
	{
		memory_Free(fonts[i].name);
		TTF_CloseFont(fonts[i].font);
	}
	memory_Free(fonts);

	for(i = 0; i < ft_rendered_string_count; i++)
	{
		memory_Free(ft_rendered_strings[i].str);
		glDeleteTextures(1, &ft_rendered_strings[i].gl_tex_handle);
	}

	memory_Free(ft_rendered_strings);
	memory_Free(ft_free_stack);

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
	font->name = memory_Strdup(name, "font_LoadFont");
	font_count++;

}


int font_UploadString(SDL_Surface *surface)
{

}


int font_RenderString(font_t *font, char *str, int line_length, vec4_t foreground, int overwrite)
{
	SDL_Surface *surface;
	SDL_Color f;
	rendered_string_t *string;
	int len;

	int index = -1;

	f.r = 255 * foreground.r;
	f.g = 255 * foreground.g;
	f.b = 255 * foreground.b;
	f.a = 255 * foreground.a;

	index = font_GetRenderedString(str);

	if((!overwrite) && index >= 0)
	{
		return index;
	}

	if(!str[0])
	{
		return -1;
	}

	if(!line_length)
	{
		surface = TTF_RenderUTF8_Blended(font->font, str, f);
	}
	else
	{
		surface = TTF_RenderUTF8_Blended_Wrapped(font->font, str, f, line_length);
	}

	if(index == -1)
	{
		if(ft_free_stack_top >= 0)
		{
			index = ft_free_stack[ft_free_stack_top];
			ft_free_stack_top--;
		}
		else
		{
			index = ft_rendered_string_count++;

			if(index >= ft_max_rendered_strings)
			{
				string = memory_Malloc(sizeof(rendered_string_t) * (ft_max_rendered_strings + 128), "font_RenderString");
				memcpy(string, ft_rendered_strings, sizeof(rendered_string_t) * ft_max_rendered_strings);

				memory_Free(ft_rendered_strings);
				memory_Free(ft_free_stack);

				ft_rendered_strings = string;
				ft_free_stack = memory_Malloc(sizeof(int) * (ft_max_rendered_strings + 128), "font_RenderString");

				ft_max_rendered_strings += 128;
			}

			string = &ft_rendered_strings[index];

			glGenTextures(1, &string->gl_tex_handle);
			glBindTexture(GL_TEXTURE_2D, string->gl_tex_handle);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

			ft_rendered_strings[index].str = NULL;
			ft_rendered_strings[index].str_len = 0;
		}
	}

	string = &ft_rendered_strings[index];

	len = strlen(str);

	/* try to reuse the already alloc'd space
	if possible... */
	if(len && len <= string->str_len)
	{
		strcpy(string->str, str);
	}
	else
	{
		if(string->str)
		{
			memory_Free(string->str);
		}

		string->str = memory_Strdup(str, "font_RenderString");
		string->str_len = len;
	}

	string->width = surface->w;
	string->height = surface->h;

	glBindTexture(GL_TEXTURE_2D, string->gl_tex_handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_FreeSurface(surface);

	return index;
}

int font_GetRenderedString(char *str)
{
	int i;

	for(i = 0; i < ft_rendered_string_count; i++)
	{
		if(!strcmp(ft_rendered_strings[i].str, str))
		{
			return i;
		}
	}

	return -1;
}

void font_DropRenderedString(int index)
{
	if(index >= 0 && index < ft_rendered_string_count)
	{
		if(ft_rendered_strings[index].width && ft_rendered_strings[index].height)
		{
			ft_rendered_strings[index].str[0] = '\0';
			ft_rendered_strings[index].width = 0;
			ft_rendered_strings[index].height = 0;

			ft_free_stack_top++;
			ft_free_stack[ft_free_stack_top] = index;

		}

	}
}


#ifdef __cplusplus
}
#endif











