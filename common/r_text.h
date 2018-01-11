#ifndef R_TEXT_H
#define R_TEXT_H

#include "r_common.h"
#include "font.h"
#include "SDL2\SDL_surface.h"
#include "vector.h"

void renderer_BlitSurface(SDL_Surface *surface, float x, float y);

void renderer_DrawString(font_t *font, int line_length, int x, int y, vec3_t color, char *str, ...);




#endif
