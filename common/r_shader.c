#include "r_common.h"
#include "r_shader.h"
#include "r_debug.h"
#include "shader.h"
#include "model.h"

#include "containers/stack_list.h"

#include "GL/glew.h"


int r_active_shader = -1;
int r_current_vertex_format = VERTEX_FORMAT_CUSTOM;

/* from shader.c */
//extern struct shader_t *shaders;

extern struct stack_list_t shd_shaders;


/* from r_main.c */
extern struct renderer_t r_renderer;
//extern int r_view_matrix_changed;
//extern int r_projection_matrix_changed;
//extern int r_shader_swaps;
//extern int r_shader_uniform_updates;


#ifdef __cplusplus
extern "C"
{
#endif

void renderer_SetShader(int shader_index)
{
	struct shader_t *shader;
	struct shader_t *current_shader = NULL;
	unsigned int program;
	unsigned int diffuse_tex;
	unsigned int normal_tex;
	int i;

	if(shader_index == r_active_shader)
		return;

    R_DBG_PUSH_FUNCTION_NAME();

	if(shader_index < 0)
	{
		program = 0;
	}
	else
	{
		//shader = &shaders[shader_index];
		shader = shader_GetShaderPointerIndex(shader_index);
		program = shader->shader_program;
	}

	if(r_active_shader > -1)
	{
		//current_shader = &shaders[r_active_shader];

		current_shader = shader_GetShaderPointerIndex(r_active_shader);

		if(current_shader->vertex_position != 0xffffffff)
		{
			glDisableVertexAttribArray(current_shader->vertex_position);
		}

		if(current_shader->vertex_normal != 0xffffffff)
		{
			glDisableVertexAttribArray(current_shader->vertex_normal);
		}

		if(current_shader->vertex_tangent != 0xffffffff)
		{
			glDisableVertexAttribArray(current_shader->vertex_tangent);
		}

		if(current_shader->vertex_tex_coords != 0xffffffff)
		{
			glDisableVertexAttribArray(current_shader->vertex_tex_coords);
		}
	}

	glUseProgram(program);
	r_active_shader = shader_index;

	r_renderer.r_view_matrix_changed = 1;
	r_renderer.r_projection_matrix_changed = 1;
	r_renderer.r_statistics.r_shader_swaps++;

	R_DBG_POP_FUNCTION_NAME();
}

/*
===============
renderer_SetUniform1i
===============
*/
//void renderer_SetUniform1i(int uniform, int value)
void renderer_SetUniform1i(uniform_t *uniform, int value)
{
	switch(uniform->type)
	{
		case GL_INT:

		case GL_SAMPLER_1D:
		case GL_SAMPLER_1D_ARRAY:
		case GL_INT_SAMPLER_1D:
		case GL_INT_SAMPLER_1D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_1D:
		case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:

		case GL_SAMPLER_2D:
		case GL_SAMPLER_2D_ARRAY:
		case GL_INT_SAMPLER_2D:
		case GL_INT_SAMPLER_2D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D:
		case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:

		case GL_SAMPLER_3D:
		case GL_INT_SAMPLER_3D:
		case GL_UNSIGNED_INT_SAMPLER_3D:

		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_CUBE_MAP_ARRAY:

			glUniform1i(uniform->location, value);
			r_renderer.r_statistics.r_shader_uniform_updates++;

		break;
	}
}

void renderer_SetUniform1iv(uniform_t *uniform, int count, int *value)
{
	switch(uniform->type)
	{
		case GL_INT:

		case GL_SAMPLER_1D:
		case GL_SAMPLER_1D_ARRAY:
		case GL_INT_SAMPLER_1D:
		case GL_INT_SAMPLER_1D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_1D:
		case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:

		case GL_SAMPLER_2D:
		case GL_SAMPLER_2D_ARRAY:
		case GL_INT_SAMPLER_2D:
		case GL_INT_SAMPLER_2D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D:
		case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:

		case GL_SAMPLER_3D:
		case GL_INT_SAMPLER_3D:
		case GL_UNSIGNED_INT_SAMPLER_3D:

		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_CUBE_MAP_ARRAY:

			glUniform1iv(uniform->location, count, value);
			r_renderer.r_statistics.r_shader_uniform_updates++;

		break;
	}
}

/*
===============
renderer_SetUniform1ui
===============
*/
void renderer_SetUniform1ui(uniform_t *uniform, unsigned int value)
{
	switch(uniform->type)
	{
		case GL_UNSIGNED_INT:
			glUniform1ui(uniform->location, value);
			r_renderer.r_statistics.r_shader_uniform_updates++;
		break;
	}
}



/*
===============
renderer_SetUniform1f
===============
*/
void renderer_SetUniform1f(uniform_t *uniform, float value)
{

	switch(uniform->type)
	{
		case GL_FLOAT:
			glUniform1f(uniform->location, value);
			r_renderer.r_statistics.r_shader_uniform_updates++;
		break;
	}
}



/*
===============
renderer_SetUniform4fv
===============
*/
void renderer_SetUniform4fv(uniform_t *uniform, int count, float *value)
{

	switch(uniform->type)
	{
		case GL_FLOAT_VEC4:
			glUniform4fv(uniform->location, count, value);
			r_renderer.r_statistics.r_shader_uniform_updates++;
		break;
	}
}


/*
===============
renderer_SetUniformMatrix4fv
===============
*/
void renderer_SetUniformMatrix4fv(uniform_t *uniform, float *value)
{
	/* nvidia driver 391.35 for gt630 is
	sometimes returning the wrong uniform type... */
	switch(uniform->type)
	{
		case GL_FLOAT_MAT4:
			glUniformMatrix4fv(uniform->location, 1, GL_FALSE, value);
			r_renderer.r_statistics.r_shader_uniform_updates++;
		break;
	}
}


/*
==============================================================
==============================================================
==============================================================
*/



void renderer_SetDefaultUniform1i(int uniform, int value)
{
	struct shader_t *shader;

	if(r_active_shader >= 0)
	{
		shader = shader_GetShaderPointerIndex(r_active_shader);

		if(uniform < UNIFORM_LAST_UNIFORM && uniform >= 0)
		{
			if(shader->default_uniforms[uniform].location != 0xffff)
			{
				renderer_SetUniform1i(&shader->default_uniforms[uniform], value);
			}
		}
	}


}

/*
===============
renderer_SetNamedUniform1i
===============
*/
void renderer_SetNamedUniform1i(char *uniform, int value)
{
	struct shader_t *shader;
	named_uniform_t *u;

	int uniform_index;

	if(r_active_shader < 0)
	{
		return;
	}

	//shader = &shaders[r_active_shader];
	shader = shader_GetShaderPointerIndex(r_active_shader);

	uniform_index = shader_GetShaderNamedUniformIndex(shader, uniform);

	if(uniform_index >= 0)
	{
		renderer_SetUniform1i(&shader->named_uniforms[uniform_index].uniform, value);
	}
}

void renderer_SetDefaultUniform1iv(int uniform, int count, int *value)
{
	struct shader_t *shader;

	if(r_active_shader >= 0)
	{
		//shader = &shaders[r_active_shader];
		shader = shader_GetShaderPointerIndex(r_active_shader);

		if(uniform < UNIFORM_LAST_UNIFORM && uniform >= 0)
		{
			if(shader->default_uniforms[uniform].location != 0xffff)
			{
				renderer_SetUniform1iv(&shader->default_uniforms[uniform], count, value);
			}
		}
	}

}



void renderer_SetDefaultUniform1ui(int uniform, unsigned int value)
{
	struct shader_t *shader;

	if(r_active_shader >= 0)
	{
		//shader = &shaders[r_active_shader];
		shader = shader_GetShaderPointerIndex(r_active_shader);

		if(uniform < UNIFORM_LAST_UNIFORM && uniform >= 0)
		{
			if(shader->default_uniforms[uniform].location != 0xffff)
			{
				renderer_SetUniform1ui(&shader->default_uniforms[uniform], value);
			}
		}
	}

}



void renderer_SetDefaultUniform1f(int uniform, float value)
{
	struct shader_t *shader;

	if(r_active_shader >= 0)
	{
		//shader = &shaders[r_active_shader];
		shader = shader_GetShaderPointerIndex(r_active_shader);

		if(uniform < UNIFORM_LAST_UNIFORM && uniform >= 0)
		{
			if(shader->default_uniforms[uniform].location != 0xffff)
			{
				renderer_SetUniform1f(&shader->default_uniforms[uniform], value);
			}
		}
	}
}



/*
===============
renderer_SetNamedUniform1i
===============
*/
void renderer_SetNamedUniform1f(char *uniform, float value)
{
	struct shader_t *shader;
	named_uniform_t *u;

	int uniform_index;

	if(r_active_shader < 0)
	{
		return;
	}

	//shader = &shaders[r_active_shader];

	shader = shader_GetShaderPointerIndex(r_active_shader);

	uniform_index = shader_GetShaderNamedUniformIndex(shader, uniform);

	if(uniform_index >= 0)
	{
		renderer_SetUniform1f(&shader->named_uniforms[uniform_index].uniform, value);
	}
}




void renderer_SetDefaultUniform4fv(int uniform, int count, float *value)
{
	struct shader_t *shader;

	if(r_active_shader >= 0)
	{
		//shader = &shaders[r_active_shader];

		shader = shader_GetShaderPointerIndex(r_active_shader);

		if(uniform < UNIFORM_LAST_UNIFORM && uniform >= 0)
		{
			if(shader->default_uniforms[uniform].location != 0xffff)
			{
				renderer_SetUniform4fv(&shader->default_uniforms[uniform], count, value);
			}

		}
	}
}


/*
===============
renderer_SetNamedUniform4fv
===============
*/
void renderer_SetNamedUniform4fv(char *uniform, int count, float *value)
{
	struct shader_t *shader;
	named_uniform_t *u;

	int uniform_index;
	int i;

	if(r_active_shader < 0)
	{
		return;
	}

	//shader = &shaders[r_active_shader];
	shader = shader_GetShaderPointerIndex(r_active_shader);

	uniform_index = shader_GetShaderNamedUniformIndex(shader, uniform);

	if(uniform_index >= 0)
	{
		renderer_SetUniform4fv(&shader->named_uniforms[uniform_index].uniform, count, value);
	}

}




void renderer_SetDefaultUniformMatrix4fv(int uniform, float *value)
{
	struct shader_t *shader;

	if(r_active_shader >= 0)
	{
		//shader = &shaders[r_active_shader];
		shader = shader_GetShaderPointerIndex(r_active_shader);

		if(uniform < UNIFORM_LAST_UNIFORM && uniform >= 0)
		{
			if(shader->default_uniforms[uniform].location != 0xffff)
			{
				renderer_SetUniformMatrix4fv(&shader->default_uniforms[uniform], value);
			}
		}
	}



}

/*
===============
renderer_SetNamedUniformMatrix4fv
===============
*/
void renderer_SetNamedUniformMatrix4fv(char *uniform, float *value)
{
	struct shader_t *shader;
	named_uniform_t *u;

	int uniform_index;
	int i;

	if(r_active_shader < 0)
	{
		return;
	}

	//sshader = &shaders[r_active_shader];

	shader = shader_GetShaderPointerIndex(r_active_shader);

	uniform_index = shader_GetShaderNamedUniformIndex(shader, uniform);

	if(uniform_index >= 0)
	{
		renderer_SetUniformMatrix4fv(&shader->named_uniforms[uniform_index].uniform, value);
	}
}


/*
==============================================================
==============================================================
==============================================================
*/


/*
===============
renderer_SetVertexAttribPointer

set a vertex attribute to be used
with following rendering commands...

===============
*/
void renderer_SetVertexAttribPointer(int attrib, int size, int type, int normalized, int stride, void *pointer)
{
	struct shader_t *shader;

	if(r_active_shader < 0)
		return;

//	shader = &shaders[r_active_shader];
	shader = shader_GetShaderPointerIndex(r_active_shader);

	switch(attrib)
	{
		case VERTEX_ATTRIB_POSITION:
			attrib = shader->vertex_position;
		break;

		case VERTEX_ATTRIB_NORMAL:
			attrib = shader->vertex_normal;
		break;

		case VERTEX_ATTRIB_TANGENT:
			attrib = shader->vertex_tangent;
		break;

		case VERTEX_ATTRIB_TEX_COORDS:
			attrib = shader->vertex_tex_coords;
		break;

		case VERTEX_ATTRIB_COLOR:
			attrib = shader->vertex_color;
		break;
	}

	if(attrib == 0xffffffff)
	{
		return;
	}

	r_current_vertex_format = VERTEX_FORMAT_CUSTOM;

	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib, size, type, normalized, stride, pointer);
}

void renderer_ClearVertexAttribPointers()
{
	struct shader_t *shader;

	if(r_active_shader > -1)
	{
		//shader = &shaders[r_active_shader];
		shader = shader_GetShaderPointerIndex(r_active_shader);

		if(shader->vertex_position != 0xffffffff)
		{
			glDisableVertexAttribArray(shader->vertex_position);
		}

		if(shader->vertex_normal != 0xffffffff)
		{
			glDisableVertexAttribArray(shader->vertex_normal);
		}

		if(shader->vertex_tangent != 0xffffffff)
		{
			glDisableVertexAttribArray(shader->vertex_tangent);
		}

		if(shader->vertex_tex_coords != 0xffffffff)
		{
			glDisableVertexAttribArray(shader->vertex_tex_coords);
		}

		if(shader->vertex_color != 0xffffffff)
		{
			glDisableVertexAttribArray(shader->vertex_color);
		}
	}
}

struct shader_t *renderer_GetActiveShaderPointer()
{
	if(r_active_shader >= 0)
	{
		//return &shaders[r_active_shader];
		return shader_GetShaderPointerIndex(r_active_shader);
	}
}

void renderer_SetCurrentVertexFormat(int format)
{
	switch(format)
	{
		case VERTEX_FORMAT_V3F_N4IREV_T4IREV_TC2F:
			renderer_ClearVertexAttribPointers();
			renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(struct compact_vertex_t), &(((struct compact_vertex_t *)0)->position));
			renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(struct compact_vertex_t), &(((struct compact_vertex_t *)0)->normal));
			renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(struct compact_vertex_t), &(((struct compact_vertex_t *)0)->tangent));
			renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, sizeof(struct compact_vertex_t), &(((struct compact_vertex_t *)0)->tex_coord));
		break;

		case VERTEX_FORMAT_V3F:
			renderer_ClearVertexAttribPointers();
			renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		break;

		default:
			return;
		break;
	}

	r_current_vertex_format = format;
}

#ifdef __cplusplus
}
#endif












