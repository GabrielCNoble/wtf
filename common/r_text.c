#include <stdarg.h>
#include <stdio.h>

#include "r_text.h"

extern int r_window_width;
extern int r_window_height;


static char formated_str[8192];

void renderer_BlitSurface(SDL_Surface *surface, float x, float y)
{
	
	if(!surface)
		return;
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	
	glRasterPos2f(((float)x / (float)r_window_width) * 2.0 - 1.0, ((float)(y) / (float)r_window_height) * 2.0 - 1.0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glUseProgram(0);
	glEnable(GL_BLEND);
	glPixelZoom(1.0, -1.0);
	glColor3f(1.0, 0.0, 0.0);
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


void renderer_DrawString(font_t *font, int line_length, int x, int y, vec3_t color, char *str, ...)
{
	
	char cparm[64];
		
	
	int iparm;
	float fparm;
	char *strparm;
	
	va_list args;
	va_start(args, str);
	
	int i;
	int decimal;
	int desired_decimal;
	int sign;
	//void *parms = ((char *)&str) + sizeof(char *);
	char *p = str;
	char *o = formated_str;
	char *q = cparm;
	char *t;
	
	vsprintf(formated_str, str, args);
	
	//if(size > MAX_FONT_PSIZE) size = MAX_FONT_PSIZE;
	//else if(size < MIN_FONT_PSIZE) size = MIN_FONT_PSIZE;
	
	//float zoom = size * FONT_ZOOM_STEP;
	
	/*while(*p)
	{
		if(*p == '%')
		{
			p++;
			q = cparm;
			desired_decimal = 999;
			switch(*p)
			{
				case 'd':
					iparm = va_arg(args, int);
					itoa(iparm, q, 10);
					while(*q)
					{
						*o++ = *q++;
					}
					p++;
				break;
				
				case 'f':
					_do_float:
						
					fparm = va_arg(args, double);
					
					i = 0;
					q = ecvt(fparm, 12, &decimal, &sign);
					if(sign)
					{
						*o++ = '-';
					}
					if(decimal <= 0)
					{
						*o++ = '0';
						*o++ = '.';
						while(*q && desired_decimal > 0)
						{
							*o++ = *q++;
							desired_decimal--;
						}
					}
					else
					{
						while(*q && i < decimal)
						{
							*o++ = *q++;
							i++;
						}
						*o++ = '.';
						while(*q && desired_decimal > 0)
						{
							*o++ = *q++;
							i++;
							desired_decimal--;
						}
					}
					p++;
				break;
				
				case 's':
					strparm = va_arg(args, char *);
					while(*strparm)
					{
						*o++ = *strparm++;
					}
					p++;
				break;
				
				case '.':
					i = 0;
					p++;
					while(*p >= '0' && *p <= '9')
					{
						cparm[i++] = *p++;
					}
					cparm[i] = '\0';
					desired_decimal = atoi(cparm);
					goto _do_float;
				break;
			}
		}
		else
		{
			if(*p == '\t')
			{
				*o++ = ' ';
				*o++ = ' ';
				*o++ = ' ';
				*o++ = ' ';
				p++;
			}
			*o++ = *p++;
		}
	}
	
	*o = '\0';*/
	
	if(color.r > 1.0) color.r = 1.0;
	else if(color.r < 0.0) color.r = 0.0;
	
	if(color.g > 1.0) color.g = 1.0;
	else if(color.g < 0.0) color.g = 0.0;
	
	if(color.b > 1.0) color.b = 1.0;
	else if(color.b < 0.0) color.b = 0.0;
	
	
	SDL_Color f = {255 * color.r, 255 * color.g, 255 * color.b, 255};
	SDL_Surface *s = TTF_RenderUTF8_Blended_Wrapped(font->font, formated_str, f, line_length);
	
	renderer_BlitSurface(s, x, y);
	
	SDL_FreeSurface(s);
	
}
