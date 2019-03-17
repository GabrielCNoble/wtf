#ifndef RENDERER_GL_H
#define RENDERER_GL_H

#include "r_common.h"
#include "GL/glew.h"

#ifdef __cplusplus
extern "C"
{
#endif


void renderer_InitGL();

void renderer_FinishGL();

/*
===============================================================================
===============================================================================
===============================================================================
*/

void renderer_PushStates(GLenum cap);

void renderer_PopStates(GLenum cap);

void renderer_CheckFunctionPointers();

/*
===============================================================================
===============================================================================
===============================================================================
*/

void renderer_PushScissorRect(int x, int y, int w, int h, int clip_rect);

void renderer_PopScissorRect();

void renderer_ClearScissorRectStack();


/*
===============================================================================
===============================================================================
===============================================================================
*/

void renderer_DrawArrays(int mode, int first, int count);

void renderer_DrawElements(int mode, int count, int type, void *indices);

void renderer_DrawArraysInstanced(int mode, int first, int count, int primcount);

/*
===============================================================================
===============================================================================
===============================================================================
*/

struct framebuffer_t renderer_CreateFramebuffer(int width, int height);

void renderer_DestroyFramebuffer(struct framebuffer_t *framebuffer);

void renderer_ResizeFramebuffer(struct framebuffer_t *framebuffer, int width, int height);

void renderer_AddAttachment(struct framebuffer_t *framebuffer, int attachment, int internal_format, int samples, int sampling);

void renderer_ShareAttachment(struct framebuffer_t *from, struct framebuffer_t *to, int attachment);

void renderer_PushFramebuffer(struct framebuffer_t *framebuffer);

void renderer_PopFramebuffer();

void renderer_BindFramebuffer(int target, struct framebuffer_t *framebuffer);

void renderer_SampleFramebuffer(double x, double y, void *sample);

/*
===============================================================================
===============================================================================
===============================================================================
*/

void renderer_PushCurrentViewport();

void renderer_PushViewport(int x, int y, int width, int height);

void renderer_PopViewport();

void renderer_GetViewport(int *x, int *y, int *w, int *h);

/*
===============================================================================
===============================================================================
===============================================================================
*/

unsigned int renderer_GenGLTexture(int target, int min_filter, int mag_filter, int wrap_s, int wrap_t, int wrap_r, int base_level, int max_level, int samples);

/*
===============================================================================
===============================================================================
===============================================================================
*/

char *renderer_GetGLEnumString(GLenum name);

#ifdef __cplusplus
}
#endif

#endif
