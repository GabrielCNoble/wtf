#include "r_gl.h"

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









