#ifndef RENDERER_GL_H
#define RENDERER_GL_H

#include "r_common.h"
#include "GL/glew.h"

void renderer_PushStates(GLenum cap);

void renderer_PopStates(GLenum cap);

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


char *renderer_GetGLEnumString(GLenum name);

#endif
