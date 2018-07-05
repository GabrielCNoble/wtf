#include "gui_surface.h"
#include "r_main.h"

#include "GL/glew.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



extern widget_t *widgets;
extern widget_t *last_widget;

#ifdef __cplusplus
extern "C"
{
#endif

wsurface_t *gui_CreateSurface(char *name, short x, short y, short w, short h, short bm_flags, void (*surface_callback)(widget_t *))
{
	wsurface_t *surface;
	 
	unsigned int prev_framebuffer;
	unsigned int prev_texture;
	unsigned int prev_read_buffer;
	unsigned int prev_tex;
	
	//int viewport[4];
	
	/*surface = malloc(sizeof(wsurface_t));
	
	if(name)
		surface->widget.name = strdup(name);
	else
		surface->widget.name = NULL;
		
	surface->widget.next = NULL;
	surface->widget.prev = NULL;
	surface->widget.parent = NULL;
	surface->widget.nestled = NULL;
	surface->widget.last_nestled = NULL;
	surface->widget.x = x;
	surface->widget.y = y;
	surface->widget.w = w * 0.5;
	surface->widget.h = h * 0.5;
	surface->widget.var = NULL;
	surface->widget.type = WIDGET_SURFACE;
	surface->widget.widget_callback = surface_callback;
	surface->widget.bm_flags = 0;
	surface->widget.rendered_name = NULL;
	surface->widget.process_callback = NULL;*/
	
	surface = (wsurface_t *)gui_CreateWidget(name, x, y, w, h, WIDGET_SURFACE);
		
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prev_framebuffer);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_tex);
	
	glGenFramebuffers(1, &surface->framebuffer_id);
	glGenTextures(1, &surface->color_texture);
	glGenTextures(1, &surface->depth_texture);
	
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, surface->framebuffer_id);
	
	glBindTexture(GL_TEXTURE_2D, surface->color_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glBindTexture(GL_TEXTURE_2D, surface->depth_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	
	
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, surface->color_texture, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, surface->depth_texture, 0);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prev_framebuffer);
	glBindTexture(GL_TEXTURE_2D, prev_tex);
	
	return surface;
}

wsurface_t *gui_AddSurface(widget_t *widget, char *name, short x, short y, short w, short h, short bm_flags, void (*surface_callback)(widget_t *))
{
	wsurface_t *surface;
	surface = gui_CreateSurface(name, x, y, w, h, bm_flags, surface_callback);
	
	if(widget)
	{
		if(!widget->nestled)
		{
			widget->nestled = (widget_t *)surface;
		}
		else
		{
			widget->last_nestled->next = (widget_t *)surface;
			surface->widget.prev = widget->last_nestled;
		}
		
		widget->last_nestled = (widget_t *)surface;
		surface->widget.parent = widget;
	}
	else
	{
		if(!widgets)
		{
			widgets = (widget_t *)surface;
		}
		else
		{
			last_widget->next = (widget_t *)surface;
			surface->widget.prev = last_widget;
		}
		
		last_widget = (widget_t *)surface;
		surface->widget.parent = NULL;
	}
	
	return surface;
}

void gui_SaveCurrentStates(widget_t *widget)
{
	wsurface_t *surface;
	
	if(widget->type == WIDGET_SURFACE)
	{
		surface = (wsurface_t *)widget;
		
		glGetFloatv(GL_COLOR_CLEAR_VALUE, &surface->prev_clear_color.floats[0]);
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &surface->prev_draw_framebuffer);
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &surface->prev_read_framebuffer);
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &surface->prev_texture);
		glGetIntegerv(GL_VIEWPORT, surface->prev_viewport);
	}
}

void gui_SetUpStates(widget_t *widget)
{
	wsurface_t *surface;
	
	if(widget->type == WIDGET_SURFACE)
	{
		surface = (wsurface_t *)widget;
		
		gui_SaveCurrentStates(widget);
		
		/*glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &surface->prev_draw_framebuffer);
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &surface->prev_read_framebuffer);
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &surface->prev_texture);
		glGetIntegerv(GL_VIEWPORT, surface->prev_viewport);*/
		
		glClearColor(surface->clear_color.r, surface->clear_color.g, surface->clear_color.b, surface->clear_color.a);
		
		//glClearColor(0.0, 0.0, 0.0, 0.0);
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, surface->framebuffer_id);
		glViewport(0, 0, surface->widget.w * 2.0, surface->widget.h * 2.0);
	}
}

void gui_RestorePrevStates(widget_t *widget)
{
	wsurface_t *surface;
	
	if(widget->type == WIDGET_SURFACE)
	{
		surface = (wsurface_t *)widget;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, surface->prev_draw_framebuffer);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, surface->prev_read_framebuffer);
		glViewport(surface->prev_viewport[0], surface->prev_viewport[1], surface->prev_viewport[2], surface->prev_viewport[3]);
		glClearColor(surface->prev_clear_color.r, surface->prev_clear_color.g, surface->prev_clear_color.b, surface->prev_clear_color.a);
	}
}

void gui_ClearSurface(widget_t *widget)
{
	wsurface_t *surface;
	if(widget->type == WIDGET_SURFACE)
	{
		gui_SetUpStates(widget);
		glClear(GL_COLOR_BUFFER_BIT);
		gui_RestorePrevStates(widget);
	}
}

void gui_UpdateSurface(widget_t *widget)
{
	wsurface_t *surface;
	
	if(widget->type == WIDGET_SURFACE)
	{
		
		if(widget->widget_callback)
		{
			widget->widget_callback(widget);
		}
		
		/*surface = (wsurface_t *)widget;
		
		gui_SetUpStates(widget);
		renderer_SetShader(-1);
		
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		
		glClearColor(1.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		
		glBegin(GL_QUADS);
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(-0.5, 0.5, -0.5);
		glVertex3f(-0.5, -0.5, -0.5);
		glVertex3f(0.5, -0.5, -0.5);
		glVertex3f(0.5, 0.5, -0.5);
		glEnd();
		
		
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glEnable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		
		gui_RestorePrevStates(widget);*/
	}
	
}

void gui_PostUpdateSurface(widget_t *widget)
{
	
}

void gui_EnablePicking(widget_t *widget)
{
	wsurface_t *surface;
	
	if(widget->type == WIDGET_SURFACE)
	{
		surface = (wsurface_t *)widget;
		
		gui_SaveCurrentStates((widget_t *)surface);
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, surface->framebuffer_id);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, surface->framebuffer_id);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glViewport(0, 0, surface->widget.w * 2.0, surface->widget.h * 2.0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}
}

void gui_DisablePicking(widget_t *widget)
{
	wsurface_t *surface;
	
	if(widget->type == WIDGET_SURFACE)
	{
		surface = (wsurface_t *)widget;
		
		gui_RestorePrevStates(widget);
		/*gui_SaveCurrentStates(surface);
		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, surface->framebuffer_id);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, surface->framebuffer_id);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glViewport(0, 0, surface->widget.w * 2.0, surface->widget.h * 2.0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);*/
	}
}

#ifdef __cplusplus
}
#endif














