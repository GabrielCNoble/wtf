#include "r_gl.h"
#include "SDL2/SDL.h"
#include "GL/glew.h"

#include <stdio.h>
#include <assert.h>

/* from r_main.c */
extern int r_draw_calls;
extern int r_frame_vert_count;
extern int r_width;
extern int r_height;
extern int r_window_width;
extern int r_window_height;

int prev_depth_test_state = GL_TRUE;
int prev_stencil_test_state = GL_FALSE;
int prev_scissor_test_state = GL_FALSE;




/*
======================================================
======================================================
======================================================
*/

#define STENCIL_STATES_STACK_DEPTH 32

static struct
{
	int state;

	int stencil_func;
	int stencil_func_ref;
	int stencil_func_mask;

	int stencil_write_mask;

	int stencil_fail_action;
	int stencil_zfail_action;
	int stencil_zpass_action;

	int stencil_clear;

}stencil_states_stack[STENCIL_STATES_STACK_DEPTH];

static int stencil_states_stack_top = -1;



#define DEPTH_STATES_STACK_DEPTH 32

static struct
{
	int state;

	int depth_func;
	int depth_write_mask;

	float depth_clear;

}depth_states_stack[DEPTH_STATES_STACK_DEPTH];

static int depth_states_stack_top = -1;
/*
======================================================
======================================================
======================================================
*/


void renderer_PushStates(GLenum cap)
{
	switch(cap)
	{
		case GL_STENCIL_TEST:

			if(stencil_states_stack_top < STENCIL_STATES_STACK_DEPTH - 1)
			{
				stencil_states_stack_top++;

				glGetIntegerv(GL_STENCIL_TEST, &stencil_states_stack[stencil_states_stack_top].state);

				glGetIntegerv(GL_STENCIL_FUNC, &stencil_states_stack[stencil_states_stack_top].stencil_func);
				glGetIntegerv(GL_STENCIL_REF, &stencil_states_stack[stencil_states_stack_top].stencil_func_ref);
				glGetIntegerv(GL_STENCIL_VALUE_MASK, &stencil_states_stack[stencil_states_stack_top].stencil_func_mask);

				glGetIntegerv(GL_STENCIL_WRITEMASK, &stencil_states_stack[stencil_states_stack_top].stencil_write_mask);

				glGetIntegerv(GL_STENCIL_FAIL, &stencil_states_stack[stencil_states_stack_top].stencil_fail_action);
				glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &stencil_states_stack[stencil_states_stack_top].stencil_zfail_action);
				glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &stencil_states_stack[stencil_states_stack_top].stencil_zpass_action);

				glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &stencil_states_stack[stencil_states_stack_top].stencil_clear);
			}
		break;

		case GL_DEPTH_TEST:
			if(depth_states_stack_top < DEPTH_STATES_STACK_DEPTH - 1)
			{
				depth_states_stack_top++;

				glGetIntegerv(GL_DEPTH_TEST, &depth_states_stack[depth_states_stack_top].state);
				glGetIntegerv(GL_DEPTH_FUNC, &depth_states_stack[depth_states_stack_top].depth_func);
				glGetIntegerv(GL_DEPTH_WRITEMASK, &depth_states_stack[depth_states_stack_top].depth_write_mask);

				glGetFloatv(GL_DEPTH_CLEAR_VALUE, &depth_states_stack[depth_states_stack_top].depth_clear);
			}
		break;


		case GL_COLOR_WRITEMASK:

		break;
	}
}

void renderer_PopStates(GLenum cap)
{
	switch(cap)
	{
		case GL_STENCIL_TEST:
			if(stencil_states_stack_top >= 0)
			{
				if(stencil_states_stack[stencil_states_stack_top].state)
				{
					glEnable(GL_STENCIL_TEST);
				}
				else
				{
					glDisable(GL_STENCIL_TEST);
				}

				glStencilOp(stencil_states_stack[stencil_states_stack_top].stencil_fail_action,
							stencil_states_stack[stencil_states_stack_top].stencil_zfail_action,
							stencil_states_stack[stencil_states_stack_top].stencil_zpass_action);


				glStencilFunc(stencil_states_stack[stencil_states_stack_top].stencil_func,
							  stencil_states_stack[stencil_states_stack_top].stencil_func_ref,
							  stencil_states_stack[stencil_states_stack_top].stencil_func_mask);

				glClearStencil(stencil_states_stack[stencil_states_stack_top].stencil_clear);

				stencil_states_stack_top--;
			}
		break;

		case GL_DEPTH_TEST:
			if(depth_states_stack_top >= 0)
			{
				if(depth_states_stack[depth_states_stack_top].state)
				{
					glEnable(GL_DEPTH_TEST);
				}
				else
				{
					glDisable(GL_DEPTH_TEST);
				}


				glDepthFunc(depth_states_stack[depth_states_stack_top].depth_func);
				glDepthMask(depth_states_stack[depth_states_stack_top].depth_write_mask);

				glClearDepth(depth_states_stack[depth_states_stack_top].depth_clear);

				depth_states_stack_top--;
			}
		break;
	}
}

void renderer_CheckFunctionPointers()
{
    //assert(glDrawArraysInstanced);

    if(!__glewDrawArraysInstanced)
    {
		__glewDrawArraysInstanced = SDL_GL_GetProcAddress("glDrawArraysInstanced");

        __glewDrawArraysInstanced = SDL_GL_GetProcAddress("glDrawArraysInstancedARB");

        __glewDrawArraysInstanced = SDL_GL_GetProcAddress("glDrawArraysInstancedEXT");
    }
}

/*
===============================================================================
===============================================================================
===============================================================================
*/

struct scissor_rect
{
	int x;
	int y;
	int w;
	int h;
};

#define SCISSOR_RECT_STACK_DEPTH 512

int r_scissor_rect_stack_top = -1;
struct scissor_rect r_scissor_rect_stack[SCISSOR_RECT_STACK_DEPTH];


void renderer_PushScissorRect(int x, int y, int w, int h, int clip_rect)
{
	struct scissor_rect *rect;
	struct scissor_rect *prev_rect;

	int x0;
	int x1;
	int y0;
	int y1;

	if(r_scissor_rect_stack_top < SCISSOR_RECT_STACK_DEPTH)
	{
		r_scissor_rect_stack_top++;
		rect = &r_scissor_rect_stack[r_scissor_rect_stack_top];

		if(w > r_window_width)
		{
			w = r_window_width;
		}

		if(h > r_window_height)
		{
			h = r_window_height;
		}

		if(x < 0)
		{
			x = 0;
		}

		if(y < 0)
		{
			y = 0;
		}

		rect->x = x;
		rect->y = y;
		rect->w = w;
		rect->h = h;

		if(r_scissor_rect_stack_top > 0 && clip_rect)
		{
			prev_rect = &r_scissor_rect_stack[r_scissor_rect_stack_top - 1];

			if(rect->y < prev_rect->y)
			{
				rect->y = prev_rect->y;
			}
			else if(rect->y > prev_rect->y + prev_rect->h)
			{
				rect->y = prev_rect->y + prev_rect->h;
			}

			y0 = rect->y + rect->h;
			y1 = prev_rect->y + prev_rect->h;

			if(y0 > y1)
			{
				rect->h -= y0 - y1;
			}



			if(rect->x < prev_rect->x)
			{
				rect->x = prev_rect->x;
			}
			else if(rect->x > prev_rect->x + prev_rect->w)
			{
				rect->x = prev_rect->x + prev_rect->w;
			}

			x0 = rect->x + rect->w;
			x1 = prev_rect->x + prev_rect->w;

			if(x0 > x1)
			{
				rect->w -= x0 - x1;
			}
		}

		glScissor(rect->x, rect->y, rect->w, rect->h);

	}
}

void renderer_PopScissorRect()
{
	struct scissor_rect *rect;

	if(r_scissor_rect_stack_top >= 0)
	{
		r_scissor_rect_stack_top--;

		if(r_scissor_rect_stack_top < 0)
		{
			return;
		}

		rect = &r_scissor_rect_stack[r_scissor_rect_stack_top];
		glScissor(rect->x, rect->y, rect->w, rect->h);
	}
}

void renderer_ClearScissorRectStack()
{
	r_scissor_rect_stack_top = -1;
}


/*
===============================================================================
===============================================================================
===============================================================================
*/

void renderer_DrawArrays(int mode, int first, int count)
{
	glDrawArrays(mode, first, count);
	r_draw_calls++;
	r_frame_vert_count += count;
}

void renderer_DrawElements(int mode, int count, int type, void *indices)
{
	glDrawElements(mode, count, type, indices);
	r_draw_calls++;
	r_frame_vert_count += count;
}

void renderer_DrawArraysInstanced(int mode, int first, int count, int primcount)
{
	glDrawArraysInstanced(mode, first, count, primcount);
	r_draw_calls++;
	r_frame_vert_count += count * primcount;
}

/*
===============================================================================
===============================================================================
===============================================================================
*/

struct framebuffer_t renderer_CreateFramebuffer(int width, int height)
{
	int i;
	struct framebuffer_t framebuffer;
	glGenFramebuffers(1, &framebuffer.framebuffer_id);

	framebuffer.width = width;
	framebuffer.height = height;

	for(i = 0; i < FRAMEBUFFER_MAX_COLOR_ATTACHMENTS; i++)
	{
		framebuffer.color_attachments[i].handle = 0;
	}

	framebuffer.depth_attachment = 0;
	framebuffer.stencil_attachment = 0;

	return framebuffer;
}

void renderer_DestroyFramebuffer(struct framebuffer_t *framebuffer)
{
	int i;

	for(i = 0; i < FRAMEBUFFER_MAX_COLOR_ATTACHMENTS; i++)
	{
		if(framebuffer->color_attachments[i].handle)
		{
			glDeleteTextures(1, &framebuffer->color_attachments[i].handle);
			framebuffer->color_attachments[i].handle = 0;
		}
	}

	if(framebuffer->depth_attachment)
	{
		glDeleteTextures(1, &framebuffer->depth_attachment);
		framebuffer->depth_attachment = 0;
		framebuffer->stencil_attachment = 0;
	}

	glDeleteFramebuffers(1, &framebuffer->framebuffer_id);
	framebuffer->framebuffer_id = 0;
}

void renderer_ResizeFramebuffer(struct framebuffer_t *framebuffer, int width, int height)
{
	int i;

	if(framebuffer->width == width && framebuffer->height == height)
	{
		return;
	}

	if(width < RENDERER_MIN_WIDTH)
	{
		width = RENDERER_MIN_WIDTH;
	}
	else if(width > RENDERER_MAX_WIDTH)
	{
		width = RENDERER_MAX_WIDTH;
	}

	if(height < RENDERER_MIN_HEIGHT)
	{
		height = RENDERER_MIN_HEIGHT;
	}
	else if(height > RENDERER_MAX_HEIGHT)
	{
		height = RENDERER_MAX_HEIGHT;
	}

	for(i = 0; i < FRAMEBUFFER_MAX_COLOR_ATTACHMENTS; i++)
	{
		if(framebuffer->color_attachments[i].handle)
		{
			glBindTexture(GL_TEXTURE_2D, framebuffer->color_attachments[i].handle);
			glTexImage2D(GL_TEXTURE_2D, 0, framebuffer->color_attachments[i].internal_format, width, height, 0, framebuffer->color_attachments[i].format, framebuffer->color_attachments[i].type, NULL);
		}
	}

	if(framebuffer->depth_attachment)
	{
		glBindTexture(GL_TEXTURE_2D, framebuffer->depth_attachment);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, framebuffer->width, framebuffer->height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	}

	framebuffer->width = width;
	framebuffer->height = height;

	glBindTexture(GL_TEXTURE_2D, 0);
}

void renderer_AddAttachment(struct framebuffer_t *framebuffer, int attachment, int internal_format)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer->framebuffer_id);

	unsigned int tex_handle;

	int format;
	int type;
	int attachment_index;

	switch(internal_format)
	{
		case GL_RGBA8:
			format = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		break;

		case GL_RGBA32I:
			format = GL_RGBA;
			type = GL_INT;
		break;

		case GL_RGBA16F:
		case GL_RGBA32F:
			format = GL_RGBA;
			type = GL_FLOAT;
		break;

		case GL_RGB8:
			format = GL_RGB;
			type = GL_UNSIGNED_BYTE;
		break;

		default:
			if(attachment != GL_DEPTH_ATTACHMENT && attachment != GL_DEPTH_STENCIL_ATTACHMENT)
			{
				printf("renderer_AddAttachment: bad internal format\n");
				return;
			}
	}

	switch(attachment)
	{
		case GL_COLOR_ATTACHMENT0:
		case GL_COLOR_ATTACHMENT1:
		case GL_COLOR_ATTACHMENT2:
		case GL_COLOR_ATTACHMENT3:
		case GL_COLOR_ATTACHMENT4:

			tex_handle = renderer_GenGLTexture(GL_TEXTURE_2D, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, GL_REPEAT, 0, 0);

			glBindTexture(GL_TEXTURE_2D, tex_handle);
			glTexImage2D(GL_TEXTURE_2D, 0, internal_format, framebuffer->width, framebuffer->height, 0, format, type, NULL);
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex_handle, 0);

			attachment_index = attachment - GL_COLOR_ATTACHMENT0;

			framebuffer->color_attachments[attachment_index].handle = tex_handle;
			framebuffer->color_attachments[attachment_index].internal_format = internal_format;
			framebuffer->color_attachments[attachment_index].format = format;
			framebuffer->color_attachments[attachment_index].type = type;

		break;


		case GL_DEPTH_STENCIL_ATTACHMENT:
		case GL_DEPTH_ATTACHMENT:
			tex_handle = renderer_GenGLTexture(GL_TEXTURE_2D, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT, GL_REPEAT, 0, 0);

			glBindTexture(GL_TEXTURE_2D, tex_handle);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, framebuffer->width, framebuffer->height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_handle, 0);
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tex_handle, 0);

			framebuffer->stencil_attachment = tex_handle;
			framebuffer->depth_attachment = tex_handle;
		break;

		default:
			printf("renderer_AddComponent: bad attachment\n");
			return;
	}

	if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("renderer_AddComponent: framebuffer is not complete\n");
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}


#define FRAMEBUFFER_STACK_DEPTH 64

int framebuffer_stack_top = -1;

struct framebuffer_t framebuffer_stack[FRAMEBUFFER_STACK_DEPTH];

void renderer_PushFramebuffer(struct framebuffer_t *framebuffer)
{
	struct framebuffer_t *fb;

	if(framebuffer_stack_top < FRAMEBUFFER_STACK_DEPTH - 1)
	{
		framebuffer_stack_top++;
		fb = &framebuffer_stack[framebuffer_stack_top];

		*fb = *framebuffer;

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb->framebuffer_id);
		renderer_PushViewport(0, 0, fb->width, fb->height);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
	}

}

void renderer_PopFramebuffer()
{
	struct framebuffer_t *framebuffer;

	int id;
	int width;
	int height;
	int attachment;

	if(framebuffer_stack_top - 1 >= 0)
	{
		framebuffer_stack_top--;
		framebuffer = &framebuffer_stack[framebuffer_stack_top];

		id = framebuffer->framebuffer_id;
		width = framebuffer->width;
		height = framebuffer->height;
		attachment = GL_COLOR_ATTACHMENT0;
	}
	else
	{
		id = 0;
		width = r_window_width;
		height = r_window_height;
		attachment = GL_BACK;
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id);
	glDrawBuffer(attachment);
	renderer_PopViewport();
	//glViewport(0, 0, width, height);
}

void renderer_SampleFramebuffer(double x, double y, void *sample)
{
	struct framebuffer_t *cur_framebuffer;

	int cur_x;
	int cur_y;
	int cur_w;
	int cur_h;

	renderer_GetViewport(&cur_x, &cur_y, &cur_w, &cur_h);

	if(framebuffer_stack_top >= 0)
	{
		cur_framebuffer = &framebuffer_stack[framebuffer_stack_top];
		glBindFramebuffer(GL_READ_FRAMEBUFFER, cur_framebuffer->framebuffer_id);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(cur_x + cur_w * (x + 1.0) * 0.5, cur_y + cur_h * (y + 1.0) * 0.5, 1, 1, cur_framebuffer->color_attachments[0].format, cur_framebuffer->color_attachments[0].type, sample);
	}
	else
	{

	}

}

/*
===============================================================================
===============================================================================
===============================================================================
*/

#define VIEWPORT_STACK_DEPTH 64

int viewport_stack_top = -1;

struct
{
	int x;
	int y;
	int w;
	int h;
}viewport_stack[VIEWPORT_STACK_DEPTH];

void renderer_PushViewport(int x, int y, int width, int height)
{
	if(viewport_stack_top < VIEWPORT_STACK_DEPTH - 1)
	{
		viewport_stack_top++;
		viewport_stack[viewport_stack_top].x = x;
		viewport_stack[viewport_stack_top].y = y;
		viewport_stack[viewport_stack_top].w = width;
		viewport_stack[viewport_stack_top].h = height;

		glViewport(x, y, width, height);
	}
}

void renderer_PopViewport()
{
	if(viewport_stack_top - 1 >= 0)
	{
		viewport_stack_top--;

		glViewport(viewport_stack[viewport_stack_top].x,
				   viewport_stack[viewport_stack_top].y,
				   viewport_stack[viewport_stack_top].w,
				   viewport_stack[viewport_stack_top].h);
	}
}

void renderer_GetViewport(int *x, int *y, int *w, int *h)
{
	*x = viewport_stack[viewport_stack_top].x;
	*y = viewport_stack[viewport_stack_top].y;
	*w = viewport_stack[viewport_stack_top].w;
	*h = viewport_stack[viewport_stack_top].h;
}

/*
===============================================================================
===============================================================================
===============================================================================
*/

unsigned int renderer_GenGLTexture(int target, int min_filter, int mag_filter, int wrap_s, int wrap_t, int wrap_r, int base_level, int max_level)
{
	unsigned int handle;
	int temp;
	int i;
	int wrap_mode_test_count;
	int wrap_mode;

	int filter_mode;

	switch(target)
	{
		case GL_TEXTURE_1D:
		case GL_TEXTURE_1D_ARRAY:
			wrap_mode_test_count = 1;
		break;

		case GL_TEXTURE_2D:
		case GL_TEXTURE_2D_ARRAY:
			wrap_mode_test_count = 2;
		break;

		case GL_TEXTURE_3D:
			wrap_mode_test_count = 3;

		break;

		default:
			printf("renderer_GetGLTexture: invalid target!\n");
			return 0;
		break;
	}

	for(i = 0; i < 2; i++)
	{
		switch(i)
		{
			case 0:
				filter_mode = min_filter;
			break;

			case 1:
				filter_mode = mag_filter;
			break;
		}


		switch(filter_mode )
		{
			case GL_NEAREST:
			case GL_NEAREST_MIPMAP_NEAREST:
			case GL_NEAREST_MIPMAP_LINEAR:

			case GL_LINEAR:
			case GL_LINEAR_MIPMAP_NEAREST:
			case GL_LINEAR_MIPMAP_LINEAR:

			break;

			default:
				printf("renderer_GenGLTexture: invalid filtering mode!\n");
				return 0;

		}


	}


	for(i = 0; i < wrap_mode_test_count; i++)
	{
		switch(i)
		{
			case 0:
				wrap_mode = wrap_s;
			break;

			case 1:
				wrap_mode = wrap_t;
			break;

			case 2:
				wrap_mode = wrap_r;
			break;
		}

		switch(wrap_mode)
		{
			case GL_CLAMP:
			case GL_CLAMP_TO_BORDER:
			case GL_REPEAT:

			break;

			default:
				printf("renderer_GenGLTexture: invalid wrap mode!\n");
				return 0;

		}

	}


	if(base_level < 0)
	{
		printf("renderer_GenGLTexture: negative base level. Clamping to zero...\n");
		base_level = 0;
	}

	if(max_level < 0)
	{
		printf("renderer_GenGLTexture: negative max level. Clamping to zero...\n");
		max_level = 0;
	}

	if(base_level > max_level)
	{
		printf("renderer_GenGLTexture: base level greater than max level...\n");
		temp = base_level;
		base_level = max_level;
		max_level = temp;
	}


	glGenTextures(1, &handle);
	glBindTexture(target, handle);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_t);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap_r);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, base_level);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, max_level);

	if(max_level)
	{
		glGenerateMipmap(target);
	}

	glBindTexture(target, 0);


	return handle;
}

/*
===============================================================================
===============================================================================
===============================================================================
*/

char *renderer_GetGLEnumString(GLenum name)
{
	switch(name)
	{
		case GL_LINEAR: return "GL_LINEAR";
		case GL_NEAREST: return "GL_NEAREST";
		case GL_LINEAR_MIPMAP_LINEAR: return "GL_LINEAR_MIPMAP_LINEAR";
		case GL_NEAREST_MIPMAP_LINEAR: return "GL_NEAREST_MIPMAP_LINEAR";
		case GL_NEAREST_MIPMAP_NEAREST: return "GL_NEAREST_MIPMAP_NEAREST";
		case GL_CLAMP: return "GL_CLAMP";
		case GL_REPEAT: return "GL_REPEAT";
		case GL_TEXTURE_2D: return "GL_TEXTURE_2D";
		case GL_TEXTURE_3D: return "GL_TEXTURE_3D";


		case GL_INT_2_10_10_10_REV: return "GL_INT_2_10_10_10_REV";
		case GL_INT: return "GL_INT";
		case GL_UNSIGNED_INT: return "GL_UNSIGNED_INT";
		case GL_FLOAT: return "GL_FLOAT";

		case GL_FLOAT_VEC2: return "GL_FLOAT_VEC2";
		case GL_FLOAT_VEC3: return "GL_FLOAT_VEC3";
		case GL_FLOAT_VEC4: return "GL_FLOAT_VEC4";
		case GL_FLOAT_MAT2: return "GL_FLOAT_MAT2";
		case GL_FLOAT_MAT3: return "GL_FLOAT_MAT3";
		case GL_FLOAT_MAT4: return "GL_FLOAT_MAT4";


		case GL_SAMPLER_1D: return "GL_SAMPLER_1D";
		case GL_SAMPLER_1D_ARRAY: return "GL_SAMPLER_1D_ARRAY";
		case GL_UNSIGNED_INT_SAMPLER_1D: return "GL_UNSIGNED_INT_SAMPLER_1D";
		case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY: return "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY";

		case GL_SAMPLER_2D: return "GL_SAMPLER_2D";
		case GL_SAMPLER_2D_ARRAY: return "GL_SAMPLER_2D_ARRAY";
		case GL_UNSIGNED_INT_SAMPLER_2D: return "GL_UNSIGNED_INT_SAMPLER_2D";

		case GL_SAMPLER_3D: return "GL_SAMPLER_3D";
		case GL_UNSIGNED_INT_SAMPLER_3D: return "GL_UNSIGNED_INT_SAMPLER_3D";

		case GL_SAMPLER_CUBE: return "GL_SAMPLER_CUBE";
		case GL_SAMPLER_CUBE_MAP_ARRAY: return "GL_SAMPLER_CUBE_MAP_ARRAY";

		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";


		default: return "unkown";
	}
}









