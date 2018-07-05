#include <stdarg.h>
#include <stdio.h>

#include "r_text.h"

#include "SDL2\SDL.h"
#include "GL\glew.h"

extern int r_window_width;
extern int r_window_height;


static char formated_str[8192];

extern rendered_string_t *ft_rendered_strings;

/*
================
renderer_BlitSurface

plain blits a surface to the screen.
Problems arise when the surface gets 
outside the left or the bottom of the
screen because the raster position gets
clipped away, and no blitting happens...

================
*/
void renderer_BlitSurface(SDL_Surface *surface, float x, float y)
{
	
	float raster_x;
	float raster_x_offset;
	float raster_y;
	
	if(!surface)
		return;
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	raster_x = ((float)x / (float)r_window_width) * 2.0 - 1.0;
	raster_y = ((float)(y) / (float)r_window_height) * 2.0 - 1.0;
	
	/*if(raster_x < -1.0)
	{
		raster_x_offset = -raster_x - 1.0;
		glTranslatef(raster_x_offset, 0.0, 0.0);
	}*/
	 
	glRasterPos2f(raster_x, raster_y);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	//glUseProgram(0);
	renderer_SetShader(-1);
	glEnable(GL_BLEND);
	glPixelZoom(1.0, -1.0);
	glColor3f(1.0, 1.0, 1.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawPixels(surface->w, surface->h, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);

	//SDL_FreeSurface(s);
	glPixelZoom(1.0, 1.0);
	glDisable(GL_BLEND);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}



void renderer_DrawRenderedString(int string_index, float x, float y)
{
	rendered_string_t *string;
	
	string = &ft_rendered_strings[string_index];
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, string->gl_tex_handle);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glBegin(GL_QUADS);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(x, y, 0.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(x, y - string->height, 0.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(x + string->width, y - string->height, 0.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(x + string->width, y, 0.0);
	glEnd();
	
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);
	
}









