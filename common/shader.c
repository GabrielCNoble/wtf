#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"
#include "shader.h"
#include "shd_pprc.h"
#include "model.h"
#include "l_common.h"
#include "log.h"
#include "GL\glew.h"
#include "c_memory.h"
#include "r_gl.h"
#include "r_debug.h"

#include "containers/stack_list.h"

#define strfy(x) #x

static char *default_uniforms[UNIFORM_LAST_UNIFORM];
static int default_uniform_types[UNIFORM_LAST_UNIFORM];


/* necessary because of buggy GL drivers returning the incorrect uniform type. Only works for
default uniforms, but it's better than nothing... */
#define SHADER_UNIFORM(uniform_name, uniform_type)	default_uniforms[uniform_name]=strfy(uniform_name);	\
													default_uniform_types[uniform_name]=uniform_type


//static int shader_list_size;
//int shader_count;
//static int free_position_stack_top;
//static int *free_position_stack;
//shader_t *shaders;
//static int active_shader;


struct stack_list_t shd_shaders;

/* from r_main.c */
extern int r_z_pre_pass_shader;
extern int r_forward_pass_shader;
extern int r_forward_pass_no_shadow_shader;
extern int r_particle_forward_pass_shader;
extern int r_flat_pass_shader;
extern int r_geometry_pass_shader;
extern int r_shade_pass_shader;
extern int r_stencil_lights_pass_shader;
extern int r_shadow_pass_shader;
extern int r_skybox_shader;
extern int r_active_shader;
extern int r_bloom0_shader;
extern int r_bloom1_shader;
extern int r_tonemap_shader;
extern int r_blit_texture_shader;
extern int r_portal_shader;
extern int r_forward_pass_portal_shader;

extern int r_gui_shader;

/* from r_imediate.c */
extern int r_imediate_color_shader;
//extern int r_glff_fixed_function_texture_shader;

/* from editor.c */
extern int brush_pick_shader;
extern int light_pick_shader;
extern int spawn_point_pick_shader;
extern int brush_dist_shader;
extern int forward_pass_brush_shader;
extern int subtractive_brush_shader;
extern int draw_bsp_shader;

static char shader_base_path[512];

int shader_Init()
{
	int i;
	int c;

	//shader_list_size = 16;
	//shader_count = 0;
	//free_position_stack_top = -1;

    shd_shaders = stack_list_create(sizeof(struct shader_t ), 128, NULL);

	SHADER_UNIFORM(UNIFORM_texture_sampler0, GL_SAMPLER_2D);
	SHADER_UNIFORM(UNIFORM_texture_sampler1, GL_SAMPLER_2D);
	SHADER_UNIFORM(UNIFORM_texture_sampler2, GL_SAMPLER_2D);
	SHADER_UNIFORM(UNIFORM_texture_cube_sampler0, GL_SAMPLER_CUBE);
	SHADER_UNIFORM(UNIFORM_texture_array_sampler0, GL_SAMPLER_2D_ARRAY);
	SHADER_UNIFORM(UNIFORM_texture_cube_array_sampler0, GL_SAMPLER_CUBE_MAP_ARRAY);
	SHADER_UNIFORM(UNIFORM_r_frame, GL_INT);
	SHADER_UNIFORM(UNIFORM_r_width, GL_INT);
	SHADER_UNIFORM(UNIFORM_r_height, GL_INT);
	SHADER_UNIFORM(UNIFORM_r_near, GL_FLOAT);
	SHADER_UNIFORM(UNIFORM_r_far, GL_FLOAT);
	SHADER_UNIFORM(UNIFORM_r_clusters_per_row, GL_INT);
	SHADER_UNIFORM(UNIFORM_r_cluster_rows, GL_INT);
	SHADER_UNIFORM(UNIFORM_r_cluster_layers, GL_INT);
	SHADER_UNIFORM(UNIFORM_r_bloom_radius, GL_INT);
	SHADER_UNIFORM(UNIFORM_cluster_texture, GL_UNSIGNED_INT_SAMPLER_3D);
	SHADER_UNIFORM(UNIFORM_material_flags, GL_UNSIGNED_INT);
	SHADER_UNIFORM(UNIFORM_projection_matrix, GL_FLOAT_MAT4);
	SHADER_UNIFORM(UNIFORM_view_matrix, GL_FLOAT_MAT4);
	SHADER_UNIFORM(UNIFORM_model_matrix, GL_FLOAT_MAT4);
	SHADER_UNIFORM(UNIFORM_model_view_matrix, GL_FLOAT_MAT4);
	SHADER_UNIFORM(UNIFORM_view_projection_matrix, GL_FLOAT_MAT4);
	SHADER_UNIFORM(UNIFORM_model_view_projection_matrix, GL_FLOAT_MAT4);
	SHADER_UNIFORM(UNIFORM_particle_positions, GL_FLOAT_VEC4);
	SHADER_UNIFORM(UNIFORM_particle_frames, GL_INT);



	//shaders = memory_Malloc(sizeof(shader_t ) * shader_list_size);
	//free_position_stack = memory_Malloc(sizeof(int) * shader_list_size);

	r_z_pre_pass_shader = shader_LoadShader("engine/z_pre_pass");
	r_forward_pass_shader = shader_LoadShader("engine/forward_pass");

    shader_AddDefine("NO_SHADOWS", NULL, 1);
	r_forward_pass_no_shadow_shader = shader_LoadShader("engine/forward_pass");
	shader_DropDefine("NO_SHADOWS", 1);

	r_particle_forward_pass_shader = shader_LoadShader("engine/particle_forward_pass");
	r_forward_pass_portal_shader = shader_LoadShader("engine/forward_pass_portal");
	r_flat_pass_shader = shader_LoadShader("engine/flat_pass");
	//r_geometry_pass_shader = shader_LoadShader("engine/geometry_pass");
	//r_shade_pass_shader = shader_LoadShader("engine/shade_pass");
	r_stencil_lights_pass_shader = shader_LoadShader("engine/stencil_light_pass");
	r_shadow_pass_shader = shader_LoadShader("engine/shadow_pass");
	r_skybox_shader = shader_LoadShader("engine/skybox");

	r_bloom0_shader = shader_LoadShader("engine/bloom0");
	r_bloom1_shader = shader_LoadShader("engine/bloom1");
	r_tonemap_shader = shader_LoadShader("engine/tonemap");

	r_blit_texture_shader = shader_LoadShader("engine/blit_texture");
	r_portal_shader = shader_LoadShader("engine/portal");

	r_gui_shader = shader_LoadShader("engine/gui/gui");

	r_imediate_color_shader = shader_LoadShader("engine/imediate draw/imediate_color");

	//shader_LoadShader("engine/include_test");

	//r_glff_fixed_function_texture_shader = shader_LoadShader("engine/imediate drawing/imediate_color");

	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "%s: subsystem initialized properly!", __func__);

	return 1;

}

void shader_Finish()
{
	/*int i;
	int j;
	for(i = 0; i < shader_count; i++)
	{
		//free(shaders[i].name);
		memory_Free(shaders[i].name);
		memory_Free(shaders[i].file_name);
		memory_Free(shaders[i].default_uniforms);

		if(shaders[i].named_uniforms)
		{
			for(j = 0; j < shaders[i].named_uniforms_list_cursor; j++)
			{
				memory_Free(shaders[i].named_uniforms[j].name);
			}

			memory_Free(shaders[i].named_uniforms);
		}
		glDeleteProgram(shaders[i].shader_program);
	}

	memory_Free(shaders);
	memory_Free(free_position_stack);*/
}


/*
================================================================================
================================================================================
================================================================================
*/

void shader_GetShaderDefaultAttributesLocations(struct shader_t *shader)
{
	if(shader)
	{

	}
}

void shader_GetShaderDefaultUniformsLocations(struct shader_t *shader)
{
	int i;
	unsigned int uniform_location;
	unsigned int uniform_type;
	char *uniform_type_str;

	printf("**************************\nshader [%s] uniforms:\n", shader->name);

	for(i = 0; i < UNIFORM_LAST_UNIFORM; i++)
	{
		uniform_location = glGetUniformLocation(shader->shader_program, default_uniforms[i]);

		shader->default_uniforms[i].location = uniform_location;
		shader->default_uniforms[i].type = GL_NONE;

		if(shader->default_uniforms[i].location != 0xffff)
		{
			//glGetActiveUniformsiv(shader->shader_program, 1, &uniform_location, GL_UNIFORM_TYPE, &uniform_type);
			//shader->default_uniforms[i].type = uniform_type;
			shader->default_uniforms[i].type = default_uniform_types[i];


			printf("default uniform [%s (%s)] at [%d]\n", default_uniforms[i], renderer_GetGLEnumString(shader->default_uniforms[i].type), shader->default_uniforms[i].location);
		}
	}


	if((i = glGetUniformBlockIndex(shader->shader_program, "light_params_uniform_block")) != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(shader->shader_program, i, R_LIGHT_UNIFORM_BUFFER_BINDING);
		//printf("default uniform [%s] at %d\n",  i);
	}


	printf("**************************\n");
}

int shader_FindShaderNamedUniformIndex(struct shader_t *shader, char *uniform_name)
{
	int i;
	int uniform_index;
	unsigned int uniform_location;
	unsigned int uniform_type;
	char *uniform_type_str;
	named_uniform_t *named_uniform;


	for(i = 0; i < shader->named_uniforms_list_cursor; i++)
	{
		if(!strcmp(shader->named_uniforms[i].name, uniform_name))
		{
			/* there's already a named uniform with this name... */
			break;
		}
	}

	uniform_index = i;

	uniform_location = glGetUniformLocation(shader->shader_program, uniform_name);

	if(uniform_location != 0xffffffff)
	{
		/* this uniform has already been found
		and is currently active, so just return its index...  */
		if(uniform_index < shader->named_uniforms_list_cursor)
		{
			return uniform_index;
		}

		if(uniform_index >= shader->named_uniforms_list_size)
		{
			if(shader->named_uniforms_list_size < 255)
			{
				named_uniform = memory_Malloc(sizeof(named_uniform_t) * (shader->named_uniforms_list_size + 1));

				if(shader->named_uniforms)
				{
					memcpy(named_uniform, shader->named_uniforms, sizeof(named_uniform_t) * shader->named_uniforms_list_size);
					memory_Free(shader->named_uniforms);
				}

				shader->named_uniforms = named_uniform;
				shader->named_uniforms_list_size++;
			}
			else
			{
				printf("shader_GetShaderNamedUniformLocation: too many uniforms on shader [%s]!\n", shader->name);
				return -1;
			}
		}

		named_uniform = &shader->named_uniforms[uniform_index];
		named_uniform->uniform.location = uniform_location;
		glGetActiveUniformsiv(shader->shader_program, 1, &uniform_location, GL_UNIFORM_TYPE, &uniform_type);
		named_uniform->uniform.type = uniform_type;
		named_uniform->name = memory_Strdup(uniform_name);
		shader->named_uniforms_list_cursor++;
		printf("shader [%s]: named uniform [%s (%s)] at [%d]\n", shader->name, uniform_name, renderer_GetGLEnumString(uniform_type), uniform_location);

		return uniform_index;

	}
	else if(uniform_index < shader->named_uniforms_list_cursor)
	{
		/* this uniform was active last time it was checked, but not
		anymore, so drop it from the list... */

		memory_Free(shader->named_uniforms[uniform_index].name);

		if(uniform_index < shader->named_uniforms_list_cursor - 1)
		{
			shader->named_uniforms[uniform_index] = shader->named_uniforms[shader->named_uniforms_list_cursor - 1];
		}

		shader->named_uniforms_list_cursor--;
	}

	return -1;
}

int shader_GetShaderNamedUniformIndex(struct shader_t *shader, char *uniform_name)
{
	int i;

	for(i = 0; i < shader->named_uniforms_list_cursor; i++)
	{
		if(!strcmp(shader->named_uniforms[i].name, uniform_name))
		{
			return i;
		}
	}

	return shader_FindShaderNamedUniformIndex(shader, uniform_name);
}


/*
================================================================================
================================================================================
================================================================================
*/

int shader_CreateEmptyShader(char *shader_name)
{
	struct shader_t *shader;
	int shader_index;

    shader_index = stack_list_add(&shd_shaders, NULL);
    shader = (struct shader_t *)stack_list_get(&shd_shaders, shader_index);

    shader->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    shader->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    shader->shader_program =  glCreateProgram();

    shader->default_uniforms = memory_Malloc(sizeof(uniform_t) * UNIFORM_LAST_UNIFORM);

    shader->named_uniforms = NULL;
	shader->named_uniforms_list_size = 0;
	shader->named_uniforms_list_cursor = 0;

	shader->name = memory_Strdup(shader_name);
	shader->file_name = NULL;

    return shader_index;
}

void shader_DestroyShaderIndex(int shader_index)
{
	struct shader_t *shader;

	shader = shader_GetShaderPointerIndex(shader_index);

	if(shader)
	{
        memory_Free(shader->name);

		if(shader->file_name)
		{
			memory_Free(shader->file_name);
		}

		memory_Free(shader->default_uniforms);

		if(shader->named_uniforms)
		{
			memory_Free(shader->named_uniforms);
		}

		glDeleteProgram(shader->shader_program);
		glDeleteShader(shader->vertex_shader);
		glDeleteShader(shader->fragment_shader);

		shader->shader_program = 0;

		stack_list_remove(&shd_shaders, shader_index);
	}
}


int shader_LoadShaderSource(char *file_name, char **vertex_shader_source, char **fragment_shader_source)
{
	FILE *vertex_file;
	FILE *fragment_file;

	unsigned long file_size;

	char *vertex_source;
	char *fragment_source;

	char shader_file_name[PATH_MAX];

	strcpy(shader_file_name, file_name);
	strcat(shader_file_name, ".vert");

	vertex_file = path_TryOpenFile(shader_file_name);

	if(!vertex_file)
	{
        log_LogMessage(LOG_MESSAGE_ERROR, 1, "shader_LoadShader: couldn't load vertex shader file for shader [%s]", file_name);
		return 0;
	}

	strcpy(shader_file_name, file_name);
	strcat(shader_file_name, ".frag");

	fragment_file = path_TryOpenFile(shader_file_name);

	if(!fragment_file)
	{
        log_LogMessage(LOG_MESSAGE_ERROR, 1, "shader_LoadShader: couldn't load fragment shader file for shader [%s]", file_name);
		return 0;
	}

	file_size = path_GetFileSize(vertex_file);
	vertex_source = memory_Calloc(1, file_size + 1);
	fread(vertex_source, 1, file_size, vertex_file);
	fclose(vertex_file);
	vertex_source[file_size] = '\0';

	file_size = path_GetFileSize(fragment_file);
	fragment_source = memory_Calloc(1, file_size + 1);
	fread(fragment_source, 1, file_size, fragment_file);
	fclose(fragment_file);
	fragment_source[file_size] = '\0';

	*vertex_shader_source = vertex_source;
	*fragment_shader_source = fragment_source;

	return 1;
}

int shader_CompileShaderSource(struct shader_t *shader, char **vertex_shader_source, char **fragment_shader_source)
{
	int *itemp;

	unsigned int uniform_type;
	unsigned int uniform_location;
	unsigned int uniform_size;

	char *vertex_source;
	char *fragment_source;

	int status;
	int shader_log_len;
	int i;
	char *shader_log;
	char *uniform_type_str;


	vertex_source = *vertex_shader_source;
	fragment_source = *fragment_shader_source;


	shader_AddDefine("MASSACRE_VERTEX_SHADER", NULL, 1);
	shader_Preprocess(&vertex_source);
	shader_DropDefine("MASSACRE_VERTEX_SHADER", 1);

	*vertex_shader_source = vertex_source;

	glShaderSource(shader->vertex_shader, 1, (const GLchar **)&vertex_source, NULL);
	glCompileShader(shader->vertex_shader);
	glGetShaderiv(shader->vertex_shader, GL_COMPILE_STATUS, &status);
	glGetShaderiv(shader->vertex_shader, GL_INFO_LOG_LENGTH, &shader_log_len);

	if(shader_log_len > 0)
	{
		//error_log = malloc(error_log_len);
		shader_log = memory_Malloc(shader_log_len);
		glGetShaderInfoLog(shader->vertex_shader, shader_log_len, NULL, shader_log);
		printf("********************************************\n[%s] shader vertex shader compilation stage info log:\n%s\n", shader->name, shader_log);
		memory_Free(shader_log);
	}

	if(!status)
	{
		return 0;
	}




	shader_AddDefine("MASSACRE_FRAGMENT_SHADER", NULL, 1);
	shader_Preprocess(&fragment_source);
	shader_DropDefine("MASSACRE_FRAGMENT_SHADER", 1);

	*fragment_shader_source = fragment_source;

	glShaderSource(shader->fragment_shader, 1, (const GLchar **)&fragment_source, NULL);
	glCompileShader(shader->fragment_shader);
	glGetShaderiv(shader->fragment_shader, GL_COMPILE_STATUS, &status);
	glGetShaderiv(shader->fragment_shader, GL_INFO_LOG_LENGTH, &shader_log_len);

	if(shader_log_len > 0)
	{
		//error_log = malloc(error_log_len);
		shader_log = memory_Malloc(shader_log_len);
		glGetShaderInfoLog(shader->fragment_shader, shader_log_len, NULL, shader_log);
		printf("********************************************\n[%s] shader fragment shader compilation stage info log:\n%s\n", shader->name, shader_log);
		memory_Free(shader_log);
	}

	if(!status)
	{
		return 0;
	}

	glAttachShader(shader->shader_program, shader->vertex_shader);
	glAttachShader(shader->shader_program, shader->fragment_shader);
	glLinkProgram(shader->shader_program);

	glGetProgramiv(shader->shader_program, GL_LINK_STATUS, &status);
	glGetProgramiv(shader->shader_program, GL_INFO_LOG_LENGTH, &shader_log_len);

	if(shader_log_len > 0)
	{
		shader_log = memory_Malloc(shader_log_len);
		glGetProgramInfoLog(shader->shader_program, shader_log_len, NULL, shader_log);
		printf("********************************************\n[%s] shader link stage info log:\n%s\n", shader->name, shader_log);
		memory_Free(shader_log);
	}

	if(!status)
	{
		return 0;
	}

	shader->vertex_position = glGetAttribLocation(shader->shader_program, "vertex_position");
	shader->vertex_normal = glGetAttribLocation(shader->shader_program, "vertex_normal");
	shader->vertex_tangent = glGetAttribLocation(shader->shader_program, "vertex_tangent");
	shader->vertex_tex_coords = glGetAttribLocation(shader->shader_program, "vertex_tex_coords");
	shader->vertex_color = glGetAttribLocation(shader->shader_program, "vertex_color");

	shader_GetShaderDefaultUniformsLocations(shader);

	return 1;
}



int shader_LoadShader(char *file_name)
{
	int shader_index = -1;
	struct shader_t *shader;

	char *vertex_shader_source;
	char *fragment_shader_source;


    if(shader_LoadShaderSource(file_name, &vertex_shader_source, &fragment_shader_source))
	{
        shader_index = shader_CreateEmptyShader(file_name);
        shader = shader_GetShaderPointerIndex(shader_index);

		if(!shader_CompileShaderSource(shader, &vertex_shader_source, &fragment_shader_source))
		{
			shader_DestroyShaderIndex(shader_index);
			shader_index = -1;
		}

		shader->file_name = memory_Strdup(file_name);

        memory_Free(vertex_shader_source);
        memory_Free(fragment_shader_source);
	}

	return shader_index;



	#if 0

	FILE *f;
	shader_t *shader;
	int *itemp;

	unsigned int vertex_shader_object;
	unsigned int fragment_shader_object;
	unsigned int shader_program;
	unsigned int uniform_type;
	unsigned int uniform_location;
	unsigned int uniform_size;

	int shader_index;
	int status;
	int error_log_len;
	int i;

	unsigned long file_size;

	char *vertex_shader_source;
	char *fragment_shader_source;
	char *error_log;
	char *uniform_type_str;

	char vertex_shader_file_name[128];
	char fragment_shader_file_name[128];
	char *shader_file_full_path;

	//int uniform_type;
	//int uniform_size;




	strcpy(vertex_shader_file_name, file_name);
	strcat(vertex_shader_file_name, ".vert");

	shader_file_full_path = path_GetPathToFile(vertex_shader_file_name);

	/*strcpy(shader_file_full_path, shader_base_path);
	strcat(shader_file_full_path, "\\");
	strcat(shader_file_full_path, vertex_shader_file_name);*/

	if(!(f = fopen(shader_file_full_path, "r")))
	{
		//printf("shader_LoadShader: couldn't load vertex shader file for shader %s!!\n", file_name);
		log_LogMessage(LOG_MESSAGE_ERROR, 1, "shader_LoadShader: couldn't load vertex shader file for shader %s!!\n", file_name);
		return -1;
	}

	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);

	//vertex_shader_source = calloc(file_size + 2, 1);
	vertex_shader_source = memory_Calloc(file_size + 2, 1);
	fread(vertex_shader_source, 1, file_size, f);
	vertex_shader_source[file_size - 1] = '\0';
	vertex_shader_source[file_size] = '\0';
	fclose(f);

	shader_AddDefine("MASSACRE_VERTEX_SHADER", NULL, 1);
	shader_Preprocess(&vertex_shader_source);
	shader_DropDefine("MASSACRE_VERTEX_SHADER", 1);


	vertex_shader_object = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader_object, 1, (const GLchar **)&vertex_shader_source, NULL);
	glCompileShader(vertex_shader_object);
	glGetShaderiv(vertex_shader_object, GL_COMPILE_STATUS, &status);
	//free(vertex_shader_source);
	memory_Free(vertex_shader_source);

	glGetShaderiv(vertex_shader_object, GL_INFO_LOG_LENGTH, &error_log_len);

	if(error_log_len > 0)
	{
		//error_log = malloc(error_log_len);
		error_log = memory_Malloc(error_log_len);
		glGetShaderInfoLog(vertex_shader_object, error_log_len, NULL, error_log);
		printf("********************************************\n[%s] shader vertex shader compilation stage info log:\n%s\n", file_name, error_log);
		//free(error_log);
		memory_Free(error_log);
	}


	if(!status)
	{
		glDeleteShader(vertex_shader_object);
		log_LogMessage(LOG_MESSAGE_ERROR, 1, "********************************************\n[%s] shader vertex shader compilation stage info log:\n%s\n", file_name, error_log);
		return -1;
	}


	strcpy(fragment_shader_file_name, file_name);
	strcat(fragment_shader_file_name, ".frag");

/*	strcpy(shader_file_full_path, shader_base_path);
	strcat(shader_file_full_path, "\\");
	strcat(shader_file_full_path, fragment_shader_file_name);*/

	shader_file_full_path = path_GetPathToFile(fragment_shader_file_name);

	if(!(f = fopen(shader_file_full_path, "r")))
	{
		//printf("shader_LoadShadear: couldn't load fragment shader file for shader %s!!\n", file_name);
		log_LogMessage(LOG_MESSAGE_ERROR, 1, "shader_LoadShader: couldn't load fragment shader file for shader %s!!\n", file_name);
		return -1;
	}

	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);

	//fragment_shader_source = calloc(file_size + 2, 1);
	fragment_shader_source = memory_Calloc(file_size + 2, 1);
	fread(fragment_shader_source, 1, file_size, f);
	fragment_shader_source[file_size - 1] = '\0';
	fragment_shader_source[file_size] = '\0';
	fclose(f);

	shader_AddDefine("MASSACRE_FRAGMENT_SHADER", NULL, 1);
	shader_Preprocess(&fragment_shader_source);
	shader_DropDefine("MASSACRE_FRAGMENT_SHADER", 1);

	fragment_shader_object = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader_object, 1, (const GLchar **)&fragment_shader_source, NULL);
	glCompileShader(fragment_shader_object);
	glGetShaderiv(fragment_shader_object, GL_COMPILE_STATUS, &status);
	memory_Free(fragment_shader_source);
	//free(fragment_shader_source);

	glGetShaderiv(fragment_shader_object, GL_INFO_LOG_LENGTH, &error_log_len);

	if(error_log_len > 0)
	{
		//error_log = malloc(error_log_len);
		error_log = memory_Malloc(error_log_len);
		glGetShaderInfoLog(fragment_shader_object, error_log_len, NULL, error_log);
		printf("********************************************\n[%s] shader fragment shader compilation stage info log:\n%s\n", file_name, error_log);
		//free(error_log);
		memory_Free(error_log);
	}


	if(!status)
	{
		//glGetShaderiv(fragment_shader_object, GL_INFO_LOG_LENGTH, &error_log_len);
		//error_log = malloc(error_log_len);
		//glGetShaderInfoLog(fragment_shader_object, error_log_len, NULL, error_log);
		//printf("Error compiling %s fragment shader!\n%s\n", file_name, error_log);
		//free(error_log);
		glDeleteShader(vertex_shader_object);
		glDeleteShader(fragment_shader_object);
		log_LogMessage(LOG_MESSAGE_ERROR, 1, "********************************************\n[%s] shader fragment shader compilation stage info log:\n%s\n", file_name, error_log);
		return -1;
	}



	shader_program = glCreateProgram();

	glAttachShader(shader_program, vertex_shader_object);
	glAttachShader(shader_program, fragment_shader_object);

/*
	glBindAttribLocation(shader_program, 0, "vertex_position");
	glBindAttribLocation(shader_program, 1, "vertex_normal");
	glBindAttribLocation(shader_program, 2, "vertex_tangent");
	glBindAttribLocation(shader_program, 3, "vertex_tex_coords");
	glBindAttribLocation(shader_program, 4, "vertex_color");*/


	glLinkProgram(shader_program);

	/* flag those shaders for deletion, so they
	get deleted once the shader program object gets
	deleted... */
	//glDeleteShader(vertex_shader_object);
//	glDeleteShader(fragment_shader_object);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &status);

	glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &error_log_len);

	if(error_log_len > 0)
	{
		//error_log = malloc(error_log_len);
		error_log = memory_Malloc(error_log_len);
		glGetProgramInfoLog(shader_program, error_log_len, NULL, error_log);
		printf("********************************************\n[%s] shader link stage info log:\n%s\n", file_name, error_log);
		memory_Free(error_log);
		//free(error_log);
	}


	if(!status)
	{
		glDeleteShader(vertex_shader_object);
		glDeleteShader(fragment_shader_object);
		glDeleteProgram(shader_program);
		log_LogMessage(LOG_MESSAGE_ERROR, 1, "********************************************\n[%s] shader link stage info log:\n%s\n", file_name, error_log);
		return -1;
	}






	if(free_position_stack_top >= 0)
	{
		shader_index = free_position_stack[free_position_stack_top];
		free_position_stack_top--;
	}
	else
	{
		shader_index = shader_count++;
		if(shader_index >= shader_list_size)
		{
			//shader = malloc(sizeof(shader_t ) * (shader_list_size + 16));
			shader = memory_Malloc(sizeof(shader_t ) * (shader_list_size + 16));
			memcpy(shader, shaders, sizeof(shader_t) * shader_list_size);
			memory_Free(shaders);
			//free(shaders);

			shaders = shader;
			shader_list_size += 16;
		}
	}





	shader = &shaders[shader_index];
	shader->shader_program = shader_program;
	//shader->name = strdup(file_name);
	shader->name = memory_Strdup(file_name);
	shader->file_name = memory_Strdup(file_name);
	//shader->file_name = strdup(file_name);


	/*glBindAttribLocation(shader->shader_program, 0, "vertex_position");
	glBindAttribLocation(shader->shader_program, 1, "vertex_normal");
	glBindAttribLocation(shader->shader_program, 2, "vertex_tangent");
	glBindAttribLocation(shader->shader_program, 3, "vertex_tex_coords");
	glBindAttribLocation(shader->shader_program, 4, "vertex_color");*/

	shader->vertex_position = glGetAttribLocation(shader->shader_program, "vertex_position");
	shader->vertex_normal = glGetAttribLocation(shader->shader_program, "vertex_normal");
	shader->vertex_tangent = glGetAttribLocation(shader->shader_program, "vertex_tangent");
	shader->vertex_tex_coords = glGetAttribLocation(shader->shader_program, "vertex_tex_coords");
	shader->vertex_color = glGetAttribLocation(shader->shader_program, "vertex_color");


/*	shader->vertex_position = 0;
	shader->vertex_normal = 1;
	shader->vertex_tangent = 2;
	shader->vertex_tex_coords = 3;
	shader->vertex_color = 4;*/


//	shader->uniforms = memory_Malloc(sizeof(uniform_t ) * UNIFORM_LAST_UNIFORM, "shader_LoadShader");


	shader->default_uniforms = memory_Malloc(sizeof(uniform_t) * UNIFORM_LAST_UNIFORM);

	shader_GetShaderDefaultUniformsLocations(shader);

	shader->named_uniforms = NULL;
	shader->named_uniforms_list_cursor = 0;
	shader->named_uniforms_list_size = 0;

	shader->bm_flags = 0;


	if((i = glGetUniformBlockIndex(shader->shader_program, "light_params_uniform_block")) != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(shader->shader_program, i, R_LIGHT_UNIFORM_BUFFER_BINDING);
		//printf("found light_params_uniform_block! %d\n", i);
	}

	//if((i = glGetUniformBlockIndex(shader->shader_program, "test_block")) != GL_INVALID_INDEX)
	//{
	//	glUniformBlockBinding(shader->shader_program, i, TEST_UNIFORM_BUFFER_BINDING);
		//printf("found test_block! %d\n", i);
	//}

	//printf("SHADER %s\n", file_name);

	/*if((i = glGetUniformBlockIndex(shader->shader_program, "cluster_uniform_block")) != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(shader->shader_program, i, LIGHT_CLUSTER_UNIFORM_BUFFER_BINDING);
		printf("found cluster_uniform_block! %d\n", i);
	}*/

	/*if((i = glGetUniformBlockIndex(shader->shader_program, "material_params_uniform_block")) != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(shader->shader_program, i, 2);
	}*/


	return shader_index;

	#endif

}


void shader_ReloadShader(int shader_index)
{
	#if 0
	FILE *f;
	shader_t *shader;
	int *itemp;

	unsigned int vertex_shader_object;
	unsigned int fragment_shader_object;
	unsigned int shader_program;

	int status;
	int error_log_len;
	int i;

	unsigned long file_size;
	int uniform_type;
	int uniform_size;

	char *vertex_shader_source;
	char *fragment_shader_source;
	char *error_log;

	char vertex_shader_file_name[128];
	char fragment_shader_file_name[128];
	//char shader_file_full_path[512];
	char *shader_file_full_path;


	if(shader_index < 0 || shader_index >= shader_count)
	{
		//printf("shader_ReloadShader: bad shader index [%d]\n", shader_index);
		log_LogMessage(LOG_MESSAGE_ERROR, 1, "shader_ReloadShader: bad shader index [%d]\n", shader_index);
		return;
	}

	shader = &shaders[shader_index];

	if(shader->bm_flags & SHADER_INVALID)
	{
		//printf("shader_ReloadShader: invalid shader [%d]\n", shader_index);
		log_LogMessage(LOG_MESSAGE_ERROR, 1, "shader_ReloadShader: invalid shader [%d]\n", shader_index);
		return;
	}

	strcpy(vertex_shader_file_name, shader->file_name);
	strcat(vertex_shader_file_name, ".vert");

	//strcpy(shader_file_full_path, shader_base_path);
	//strcat(shader_file_full_path, "\\");
	//strcat(shader_file_full_path, vertex_shader_file_name);

	shader_file_full_path = path_GetPathToFile(vertex_shader_file_name);

	if(!(f = fopen(shader_file_full_path, "r")))
	{
		//printf("shader_ReloadShader: couldn't load vertex shader file [%s] for shader [%s]\n", shader->file_name, shader->name);
		log_LogMessage(LOG_MESSAGE_ERROR, 1, "shader_ReloadShader: couldn't load vertex shader file [%s] for shader [%s]\n", shader->file_name, shader->name);
		return;
	}

	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);

	//vertex_shader_source = calloc(file_size + 2, 1);
	vertex_shader_source = memory_Calloc(file_size + 2, 1);
	fread(vertex_shader_source, 1, file_size, f);
	vertex_shader_source[file_size - 1] = '\0';
	vertex_shader_source[file_size] = '\0';
	fclose(f);

	shader_AddDefine("MASSACRE_VERTEX_SHADER", NULL, 1);
	shader_Preprocess(&vertex_shader_source);
	shader_DropDefine("MASSACRE_VERTEX_SHADER", 1);


	vertex_shader_object = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader_object, 1, (const GLchar **)&vertex_shader_source, NULL);
	glCompileShader(vertex_shader_object);
	glGetShaderiv(vertex_shader_object, GL_COMPILE_STATUS, &status);
	//free(vertex_shader_source);
	memory_Free(vertex_shader_source);

	glGetShaderiv(vertex_shader_object, GL_INFO_LOG_LENGTH, &error_log_len);

	if(error_log_len > 0)
	{
		//error_log = malloc(error_log_len);
		error_log = memory_Malloc(error_log_len);
		glGetShaderInfoLog(vertex_shader_object, error_log_len, NULL, error_log);
		printf("********************************************\n[%s] shader vertex shader compilation stage info log:\n%s\n", shader->file_name, error_log);
		//free(error_log);
		memory_Free(error_log);
	}


	if(!status)
	{
		glDeleteShader(vertex_shader_object);
		return;
	}


	strcpy(fragment_shader_file_name, shader->file_name);
	strcat(fragment_shader_file_name, ".frag");

	//strcpy(shader_file_full_path, shader_base_path);
	//strcat(shader_file_full_path, "\\");
	//strcat(shader_file_full_path, fragment_shader_file_name);

	shader_file_full_path = path_GetPathToFile(fragment_shader_file_name);

	if(!(f = fopen(shader_file_full_path, "r")))
	{
		printf("shader_ReloadShader: couldn't load fragment shader file [%s] for shader [%s]\n", shader->file_name, shader->name);
		return;
	}

	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);

	//fragment_shader_source = calloc(file_size + 2, 1);
	fragment_shader_source = memory_Calloc(file_size + 2, 1);
	fread(fragment_shader_source, 1, file_size, f);
	fragment_shader_source[file_size - 1] = '\0';
	fragment_shader_source[file_size] = '\0';
	fclose(f);

	shader_AddDefine("MASSACRE_FRAGMENT_SHADER", NULL, 1);
	shader_Preprocess(&fragment_shader_source);
	shader_DropDefine("MASSACRE_FRAGMENT_SHADER", 1);

	fragment_shader_object = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader_object, 1, (const GLchar **)&fragment_shader_source, NULL);
	glCompileShader(fragment_shader_object);
	glGetShaderiv(fragment_shader_object, GL_COMPILE_STATUS, &status);
	//free(fragment_shader_source);
	memory_Free(fragment_shader_source);

	glGetShaderiv(fragment_shader_object, GL_INFO_LOG_LENGTH, &error_log_len);

	if(error_log_len > 0)
	{
		//error_log = malloc(error_log_len);
		error_log = memory_Malloc(error_log_len);
		glGetShaderInfoLog(fragment_shader_object, error_log_len, NULL, error_log);
		printf("********************************************\n[%s] shader fragment shader compilation stage info log:\n%s\n", shader->file_name, error_log);
		//free(error_log);
		memory_Free(error_log);
	}


	if(!status)
	{
		glDeleteShader(vertex_shader_object);
		glDeleteShader(fragment_shader_object);
		return;
	}


	shader_program = glCreateProgram();

	glAttachShader(shader_program, vertex_shader_object);
	glAttachShader(shader_program, fragment_shader_object);
	glLinkProgram(shader_program);

	/* flag those shaders for deletion, so they
	get deleted once the shader program object gets
	deleted... */
	glDeleteShader(vertex_shader_object);
	glDeleteShader(fragment_shader_object);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &status);

	glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &error_log_len);

	if(error_log_len > 0)
	{
		//error_log = malloc(error_log_len);
		error_log = memory_Malloc(error_log_len);
		glGetProgramInfoLog(shader_program, error_log_len, NULL, error_log);
		printf("********************************************\n[%s] shader link stage info log:\n%s\n", shader->file_name, error_log);
		//free(error_log);
		memory_Free(error_log);
	}


	if(!status)
	{
		glDeleteShader(vertex_shader_object);
		glDeleteShader(fragment_shader_object);
		glDeleteProgram(shader_program);
		return;
	}


	glDeleteProgram(shader->shader_program);



	/*if(free_position_stack_top >= 0)
	{
		shader_index = free_position_stack[free_position_stack_top--];
	}
	else
	{
		shader_index = shader_count++;
		if(shader_index >= shader_list_size)
		{
			shader = malloc(sizeof(shader_t ) * (shader_list_size + 16));
			memcpy(shader, shaders, sizeof(shader_t) * shader_list_size);
			free(shaders);

			shaders = shader;
			shader_list_size += 16;
		}
	}*/





	//shader = &shaders[shader_index];
	shader->shader_program = shader_program;
	//shader->name = strdup(file_name);

	shader->vertex_position = glGetAttribLocation(shader->shader_program, "vertex_position");
	shader->vertex_normal = glGetAttribLocation(shader->shader_program, "vertex_normal");
	shader->vertex_tangent = glGetAttribLocation(shader->shader_program, "vertex_tangent");
	shader->vertex_tex_coords = glGetAttribLocation(shader->shader_program, "vertex_tex_coords");

	shader_GetShaderDefaultUniformsLocations(shader);

	shader->bm_flags = 0;

	//printf("%s: %d %d %d\n", file_name, shader->width, shader->height, shader->cluster_texture);

	/*if((i = glGetUniformBlockIndex(shader->shader_program, "camera_params_uniform_block")) != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(shader->shader_program, i, 0);
	}*/

	if((i = glGetUniformBlockIndex(shader->shader_program, "light_params_uniform_block")) != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(shader->shader_program, i, LIGHT_PARAMS_UNIFORM_BUFFER_BINDING);
		printf("found light_params_uniform_block! %d\n", i);
	}

	if((i = glGetUniformBlockIndex(shader->shader_program, "test_block")) != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(shader->shader_program, i, TEST_UNIFORM_BUFFER_BINDING);
		printf("found test_block! %d\n", i);
	}

	/*if((i = glGetUniformBlockIndex(shader->shader_program, "cluster_uniform_block")) != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(shader->shader_program, i, LIGHT_CLUSTER_UNIFORM_BUFFER_BINDING);
		printf("found cluster_uniform_block! %d\n", i);
	}*/

	/*if((i = glGetUniformBlockIndex(shader->shader_program, "material_params_uniform_block")) != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(shader->shader_program, i, 2);
	}*/


	return;

	#endif
}


int shader_GetShaderIndex(char *shader_name)
{
	int i;

	struct shader_t *shaders;

	shaders = (struct shader_t *)shd_shaders.elements;

	for(i = 0; i < shd_shaders.element_count; i++)
	{
		if(!shaders[i].shader_program)
		{
			continue;
		}

		if(!strcmp(shader_name, shaders[i].name))
		{
			return i;
		}
	}

	return -1;
}

struct shader_t *shader_GetShaderPointerIndex(int shader_index)
{
    struct shader_t *shaders;
    int i;

    if(shader_index >= 0 && shader_index < shd_shaders.element_count)
	{
		shaders = (struct shader_t *)shd_shaders.elements;

        if(shaders[shader_index].shader_program)
		{
            return shaders + shader_index;
		}
	}

	return NULL;
}



void shader_HotReload()
{
	int i;

	printf("****************** SHADER HOT RELOAD ******************\n");

	/*for(i = 0; i < shader_count; i++)
	{
		shader_ReloadShader(i);
	}*/

	printf("****************** SHADER HOT RELOAD ******************\n\n");

}


/*void shader_UseShader(int shader_index)
{
	shader_t *shader;

	if(r_active_shader == shader_index)
		return;

	r_active_shader = shader_index;

	if(shader_index < 0)
	{
		glUseProgram(0);
	}


	shader = &shaders[shader_index];
	glUseProgram(shader->shader_program);

	glEnableVertexAttribArray(shader->vertex_position);
	glVertexAttribPointer(shader->vertex_position, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)0);

	if(shader->vertex_normal != 255)
	{
		glEnableVertexAttribArray(shader->vertex_normal);
		glVertexAttribPointer(shader->vertex_normal, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)&(((vertex_t *)0)->normal));
	}

	if(shader->vertex_tangent != 255)
	{
		glEnableVertexAttribArray(shader->vertex_tex_coords);
		glVertexAttribPointer(shader->vertex_tex_coords, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)&(((vertex_t *)0)->tex_coord));
	}







}*/

#if 0

void shader_SetCurrentShaderUniform1i(int uniform, int value)
{
	if(uniform < UNIFORM_LAST_UNIFORM && uniform >= 0)
	{
		glUniform1i(uniform, value);
	}


}

void shader_SetCurrentShaderUniform1f(int uniform, float value)
{
	/*switch(uniform)
	{
		case UNIFORM_WIDTH:
			uniform = shaders[r_active_shader].width;
		break;

		case UNIFORM_HEIGHT:
			uniform = shaders[r_active_shader].height;
		break;

		default:
			return;
	}

	glUniform1f(uniform, value);*/
}

void shader_SetCurrentShaderUniform4fv(int uniform, float *value)
{
	/*switch(uniform)
	{
		case UNIFORM_ACTIVE_CAMERA_POSITION:
			uniform = shaders[r_active_shader].active_camera_position;
		break;

		default:
			return;
	}

	glUniform4fv(uniform, 1, value);*/
}

void shader_SetCurrentShaderUniformMatrix4fv(int uniform, float *value)
{
	/*switch(uniform)
	{
		case UNIFORM_CAMERA_TO_LIGHT_MATRIX:
			uniform = shaders[r_active_shader].camera_to_light_matrix;
		break;

		default:
			return;
		break;
	}

	glUniformMatrix4fv(uniform, 1, GL_FALSE, value);*/
}


void shader_SetCurrentShaderVertexAttribPointer(int attrib, int size, int type, int normalized, int stride, void *pointer)
{
	/*int att;
	switch(attrib)
	{
		case ATTRIB_VERTEX_POSITION:
			att = shaders[r_active_shader].vertex_position;
		break;

		default:
			return;
		break;
	}

	glVertexAttribPointer(att, size, type, normalized, stride, (const void *)pointer);*/
}


#endif




