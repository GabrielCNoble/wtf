#include "r_main.h"
#include "r_imediate.h"
#include "r_shader.h"
#include "r_debug.h"
#include "gpu.h"
#include "c_memory.h"
#include <stdio.h>

#include "vector.h"



#define MAX_IMEDIATE_VERTS 16000

GLenum r_imediate_current_mode = GL_INVALID_OPERATION;

int r_imediate_color_shader;
int r_glff_fixed_function_texture_shader;

//int r_glff_fixed_function_enabled = 0;
int r_imediate_drawing_enabled = 0;
int r_imediate_input_enabled = 0;
//int r_glff_texturing_enabled = 0;

int r_imediate_verts_count = 0;
vec4_t *r_imediate_verts = NULL;

int r_imediate_norms_count = 0;
vec4_t r_imediate_current_normal;
vec4_t *r_imediate_norms = NULL;

int r_imediate_tex_coords_count = 0;
vec4_t r_imediate_current_tex_coords;
vec4_t *r_imediate_tex_coords = NULL;

int r_imediate_color_count = 0;
vec4_t r_imediate_current_color;
vec4_t *r_imediate_colors = NULL;

int r_imediate_max_vert_count = 0;
int r_imediate_verts_per_primitive = 0;
int r_imediate_inverse_relative_vert_index = 0;


/* from r_shader.c */
extern int r_current_vertex_format;


void renderer_InitImediateDrawing()
{
	//r_glff_fixed_function_shader = shader_LoadShader("fixed_function");
	r_imediate_verts = memory_Malloc(sizeof(vec4_t) * MAX_IMEDIATE_VERTS, "renderer_InitImediateDrawing");
	r_imediate_norms = memory_Malloc(sizeof(vec4_t) * MAX_IMEDIATE_VERTS, "renderer_InitImediateDrawing");
	r_imediate_tex_coords = memory_Malloc(sizeof(vec4_t) * MAX_IMEDIATE_VERTS, "renderer_InitImediateDrawing");
	r_imediate_colors = memory_Malloc(sizeof(vec4_t) * MAX_IMEDIATE_VERTS, "renderer_InitImediateDrawing");
}

void renderer_FinishImediateDrawing()
{
	memory_Free(r_imediate_verts);
	memory_Free(r_imediate_norms);
	memory_Free(r_imediate_tex_coords);
	memory_Free(r_imediate_colors);
}

unsigned int r_bound_array_buffer = 0;
unsigned int r_bound_element_array_buffer = 0;

void renderer_EnableImediateDrawing()
{
	R_DBG_PUSH_FUNCTION_NAME();

	r_imediate_drawing_enabled = 1;

	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &r_bound_array_buffer);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &r_bound_element_array_buffer);

	/* TODO: preserve those states... */
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	R_DBG_POP_FUNCTION_NAME();
}

void renderer_DisableImediateDrawing()
{
	R_DBG_PUSH_FUNCTION_NAME();

	r_imediate_drawing_enabled = 0;
	r_imediate_input_enabled = 0;
	r_imediate_current_mode = GL_INVALID_OPERATION;

	glBindBuffer(GL_ARRAY_BUFFER, r_bound_array_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r_bound_element_array_buffer);

	renderer_SetCurrentVertexFormat(r_current_vertex_format);

	R_DBG_POP_FUNCTION_NAME();
}


void renderer_Begin(GLenum mode)
{

	R_DBG_PUSH_FUNCTION_NAME();

	int verts_per_primitive = 0;
	if(r_imediate_drawing_enabled)
	{
		switch(mode)
		{
			case GL_POINTS:
				r_imediate_verts_per_primitive = 1;
			break;

			case GL_LINES:
				r_imediate_verts_per_primitive = 2;
			break;

			case GL_LINE_LOOP:
				r_imediate_verts_per_primitive = 2;
			break;

			case GL_LINE_STRIP:
				r_imediate_verts_per_primitive = 2;
			break;

			case GL_TRIANGLES:
				r_imediate_verts_per_primitive = 3;
			break;

			case GL_QUADS:
				r_imediate_verts_per_primitive = 4;
			break;

			case GL_TRIANGLE_FAN:
				r_imediate_verts_per_primitive = 4;
			break;

			case GL_TRIANGLE_STRIP:
				r_imediate_verts_per_primitive = 4;
			break;

			case GL_QUAD_STRIP:
				r_imediate_verts_per_primitive = 4;
			break;

			case GL_ENGINE_TRIANGLE_MESH:
				r_imediate_verts_per_primitive = 3;
			break;

			default:
				printf("renderer_Begin: incorrect mode!\n");
				return;

		}

		r_imediate_verts_count = 0;
		r_imediate_norms_count = 0;
		r_imediate_tex_coords_count = 0;
		r_imediate_color_count = 0;

		r_imediate_max_vert_count = (MAX_IMEDIATE_VERTS - (r_imediate_verts_per_primitive - 1)) & (~((r_imediate_verts_per_primitive - 1)));

		if(r_imediate_current_mode != GL_INVALID_OPERATION)
		{
			printf("renderer_Begin: function called without a previous renderer_End!\n");

			R_DBG_POP_FUNCTION_NAME();

			return;
		}

		r_imediate_input_enabled = 1;
		r_imediate_current_mode = mode;
	}

	R_DBG_POP_FUNCTION_NAME();

}

void renderer_End()
{
	R_DBG_PUSH_FUNCTION_NAME();

	if(r_imediate_drawing_enabled)
	{
		if(r_imediate_current_mode != GL_INVALID_OPERATION)
		{
			if(r_imediate_verts_count)
			{
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				renderer_ClearVertexAttribPointers();
				renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 4, GL_FLOAT, 0, 0, r_imediate_verts);
				renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 4, GL_FLOAT, 0, 0, r_imediate_norms);
				renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 4, GL_FLOAT, 0, 0, r_imediate_tex_coords);
				renderer_SetVertexAttribPointer(VERTEX_ATTRIB_COLOR, 4, GL_FLOAT, 0, 0, r_imediate_colors);

				renderer_UpdateMatrices();
				glDrawArrays(r_imediate_current_mode, 0, r_imediate_verts_count);
			}

			r_imediate_current_mode = GL_INVALID_OPERATION;
			r_imediate_input_enabled = 0;
		}
		else
		{
			printf("renderer_End: function called without a previous renderer_Begin!\n");
		}
	}

	R_DBG_POP_FUNCTION_NAME();
}


/*
==============================================================
==============================================================
==============================================================
*/

void renderer_Vertex3f(double x, double y, double z)
{
	GLenum cur_mode;

	R_DBG_PUSH_FUNCTION_NAME();
	if(r_imediate_input_enabled)
	{
		r_imediate_inverse_relative_vert_index = r_imediate_verts_per_primitive - (r_imediate_verts_count % r_imediate_verts_per_primitive);

		if(r_imediate_verts_count + r_imediate_inverse_relative_vert_index >= r_imediate_max_vert_count)
		{
			cur_mode = r_imediate_current_mode;
			renderer_End();
			renderer_Begin(cur_mode);
		}

		r_imediate_verts[r_imediate_verts_count].x = x;
		r_imediate_verts[r_imediate_verts_count].y = y;
		r_imediate_verts[r_imediate_verts_count].z = z;
		r_imediate_verts[r_imediate_verts_count].w = 1.0f;

		//printf("[%f %f %f]\n", x, y, z);



		r_imediate_verts_count++;

		r_imediate_norms[r_imediate_norms_count] = r_imediate_current_normal;
		r_imediate_norms_count++;

		r_imediate_tex_coords[r_imediate_tex_coords_count] = r_imediate_current_tex_coords;
		r_imediate_tex_coords_count++;

		r_imediate_colors[r_imediate_color_count] = r_imediate_current_color;
		r_imediate_color_count++;
	}

	R_DBG_POP_FUNCTION_NAME();

}

void renderer_Normal3f(double nx, double ny, double nz)
{
	r_imediate_current_normal.x = nx;
	r_imediate_current_normal.y = ny;
	r_imediate_current_normal.z = nz;
	r_imediate_current_normal.w = 1.0;
}

void renderer_TexCoord2f(double s, double t)
{
	r_imediate_current_tex_coords.x = s;
	r_imediate_current_tex_coords.y = t;
	r_imediate_current_tex_coords.z = 0.0;
	r_imediate_current_tex_coords.w = 0.0;
}

void renderer_Color3f(double r, double g, double b)
{
	r_imediate_current_color.r = r;
	r_imediate_current_color.g = g;
	r_imediate_current_color.b = b;
	r_imediate_current_color.a = 1.0;
}

void renderer_Color4f(double r, double g, double b, double a)
{
	r_imediate_current_color.r = r;
	r_imediate_current_color.g = g;
	r_imediate_current_color.b = b;
	r_imediate_current_color.a = a;
}

void renderer_Rectf(double x0, double y0, double x1, double y1)
{
	int i;
	GLenum cur_mode;

	R_DBG_PUSH_FUNCTION_NAME();

	if(r_imediate_drawing_enabled)
	{
		renderer_Begin(GL_QUADS);

		/*r_imediate_inverse_relative_vert_index = r_imediate_verts_per_primitive - (r_imediate_verts_count % r_imediate_verts_per_primitive);

		if(r_imediate_verts_count + r_imediate_inverse_relative_vert_index >= r_imediate_max_vert_count)
		{
			cur_mode = r_imediate_current_mode;
			renderer_End();
			renderer_Begin(cur_mode);
		}*/

		for(i = 0; i < 4; i++)
		{
			r_imediate_norms[r_imediate_norms_count] = r_imediate_current_normal;
			r_imediate_norms_count++;
		}

		for(i = 0; i < 4; i++)
		{
			r_imediate_tex_coords[r_imediate_tex_coords_count] = r_imediate_current_tex_coords;
			r_imediate_tex_coords_count++;
		}

		for(i = 0; i < 4; i++)
		{
			r_imediate_colors[r_imediate_color_count] = r_imediate_current_color;
			r_imediate_color_count++;
		}



		r_imediate_verts[r_imediate_verts_count].x = x0;
		r_imediate_verts[r_imediate_verts_count].y = y1;
		r_imediate_verts[r_imediate_verts_count].z = 0.5;
		r_imediate_verts[r_imediate_verts_count].w = 1.0f;
		r_imediate_verts_count++;

		r_imediate_verts[r_imediate_verts_count].x = x0;
		r_imediate_verts[r_imediate_verts_count].y = y0;
		r_imediate_verts[r_imediate_verts_count].z = 0.5;
		r_imediate_verts[r_imediate_verts_count].w = 1.0f;
		r_imediate_verts_count++;

		r_imediate_verts[r_imediate_verts_count].x = x1;
		r_imediate_verts[r_imediate_verts_count].y = y0;
		r_imediate_verts[r_imediate_verts_count].z = 0.5;
		r_imediate_verts[r_imediate_verts_count].w = 1.0f;
		r_imediate_verts_count++;

		r_imediate_verts[r_imediate_verts_count].x = x1;
		r_imediate_verts[r_imediate_verts_count].y = y1;
		r_imediate_verts[r_imediate_verts_count].z = 0.5;
		r_imediate_verts[r_imediate_verts_count].w = 1.0f;
		r_imediate_verts_count++;

		renderer_End();
	}

	R_DBG_POP_FUNCTION_NAME();


}

void renderer_DrawVerts(GLenum mode, int count, int size, int stride, void *vertices)
{
	int i;
//	vec3_t *verts3;
	float *verts;
	GLenum cur_mode;
	if(count)
	{

		if(!stride)
		{
			stride = size * 4;
		}

		renderer_Begin(mode);
		//verts3 = (vec3_t *)verts;
		for(i = 0; i < count; i++)
		{
			r_imediate_inverse_relative_vert_index = r_imediate_verts_per_primitive - (r_imediate_verts_count % r_imediate_verts_per_primitive);

			if(r_imediate_verts_count + r_imediate_inverse_relative_vert_index >= r_imediate_max_vert_count)
			{
				cur_mode = r_imediate_current_mode;
				renderer_End();
				renderer_Begin(cur_mode);
			}

			verts = (float *)((char *)vertices + stride * i);

			r_imediate_verts[r_imediate_verts_count].x = verts[0];
			r_imediate_verts[r_imediate_verts_count].y = verts[1];
			r_imediate_verts[r_imediate_verts_count].z = verts[2];
			r_imediate_verts[r_imediate_verts_count].w = 1.0f;

			r_imediate_verts_count++;

			r_imediate_norms[r_imediate_norms_count] = r_imediate_current_normal;
			r_imediate_norms_count++;

			r_imediate_tex_coords[r_imediate_tex_coords_count] = r_imediate_current_tex_coords;
			r_imediate_tex_coords_count++;

			r_imediate_colors[r_imediate_color_count] = r_imediate_current_color;
			r_imediate_color_count++;
		}

		renderer_End();
	}
}

void renderer_DrawVertsIndexed(GLenum mode, int count, int size, int stride, void *vertices, void *indexes)
{
	int i;
	int index;
	vec3_t *verts3;
	float *verts;
	int *inds;
	GLenum cur_mode;
	if(count)
	{
		renderer_Begin(mode);
		//verts3 = (vec3_t *)verts;
		inds = (int *)indexes;
		for(i = 0; i < count; i++)
		{
			r_imediate_inverse_relative_vert_index = r_imediate_verts_per_primitive - (r_imediate_verts_count % r_imediate_verts_per_primitive);

			if(r_imediate_verts_count + r_imediate_inverse_relative_vert_index >= r_imediate_max_vert_count)
			{
				cur_mode = r_imediate_current_mode;
				renderer_End();
				renderer_Begin(cur_mode);
			}

			index = inds[i];

			verts = (float *)((char *)vertices + stride * index);

			/*r_imediate_verts[r_imediate_verts_count].x = verts3[index].x;
			r_imediate_verts[r_imediate_verts_count].y = verts3[index].y;
			r_imediate_verts[r_imediate_verts_count].z = verts3[index].z;*/

			r_imediate_verts[r_imediate_verts_count].x = verts[0];
			r_imediate_verts[r_imediate_verts_count].y = verts[1];
			r_imediate_verts[r_imediate_verts_count].z = verts[2];
			r_imediate_verts[r_imediate_verts_count].w = 1.0f;

			r_imediate_verts_count++;

			r_imediate_norms[r_imediate_norms_count] = r_imediate_current_normal;
			r_imediate_norms_count++;

			r_imediate_tex_coords[r_imediate_tex_coords_count] = r_imediate_current_tex_coords;
			r_imediate_tex_coords_count++;

			r_imediate_colors[r_imediate_color_count] = r_imediate_current_color;
			r_imediate_color_count++;
		}

		renderer_End();
	}
}


/*
==============================================================
==============================================================
==============================================================
*/




