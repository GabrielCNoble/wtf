#ifndef R_TEXT_H
#define R_TEXT_H

#include "r_common.h"
#include "font.h"
#include "SDL2\SDL_surface.h"
#include "vector.h"

void renderer_BlitSurface(SDL_Surface *surface, float x, float y);

void renderer_DrawRenderedString(int string_index, float x, float y);





#endif
