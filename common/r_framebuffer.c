#include "r_framebuffer.h"
#include "GL/glew.h"
#include <stdarg.h>

int renderer_CreateFramebuffer(int width, int height)
{
	
	va_list start;
	
	framebuffer_t f;
	
	if(width < RENDERER_MIN_WIDTH) width = RENDERER_MIN_WIDTH;
	else if(width > RENDERER_MAX_WIDTH) width = RENDERER_MAX_WIDTH;
	
	if(height < RENDERER_MIN_HEIGHT) height = RENDERER_MIN_HEIGHT;
	else if(height > RENDERER_MAX_HEIGHT) height = RENDERER_MAX_HEIGHT;
	
	glGenFramebuffers(1, &f.framebuffer_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.framebuffer_id);
	
	glGenTextures(1, &f.color_attachment);
	glBindTexture(GL_TEXTURE_2D, f.color_attachment);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	
	
	glGenTextures(1, &f.depth_attachment);
	glBindTexture(GL_TEXTURE_2D, f.color_attachment);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	
		
}

void renderer_DestroyFramebuffer(int framebuffer)
{
	
}

void renderer_BindFramebuffer(int framebuffer)
{
	
}




