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
extern int r_wireframe_pass_shader;
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
extern int r_cluster_debug_shader;

//extern int r_clustered_forward_pass_shader;

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
	SHADER_UNIFORM(UNIFORM_r_world_vertices_count, GL_INT);
	SHADER_UNIFORM(UNIFORM_r_bsp_node_count, GL_INT);
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

	r_z_pre_pass_shader = shader_LoadShader("engine/z_pre_pass", "z prepass");
	r_forward_pass_shader = shader_LoadShader("engine/forward_pass", "forward pass");
    //r_clustered_forward_pass_shader = shader_LoadShader("engine/clustered_forward", "clustered forward");

    shader_AddDefine("NO_SHADOWS", NULL, 1);
	r_forward_pass_no_shadow_shader = shader_LoadShader("engine/forward_pass", "forward pass no shadows");
	shader_DropDefine("NO_SHADOWS", 1);

	r_particle_forward_pass_shader = shader_LoadShader("engine/particle_forward_pass", "particle forward pass");
	//r_forward_pass_portal_shader = shader_LoadShader("engine/forward_pass_portal", "portal forward pass");
	r_flat_pass_shader = shader_LoadShader("engine/flat_pass", "flat pass");
	r_wireframe_pass_shader = shader_LoadShader("engine/wireframe_pass", "wireframe pass");
	//r_geometry_pass_shader = shader_LoadShader("engine/geometry_pass");
	//r_shade_pass_shader = shader_LoadShader("engine/shade_pass");
	//r_stencil_lights_pass_shader = shader_LoadShader("engine/stencil_light_pass");
	r_shadow_pass_shader = shader_LoadShader("engine/shadow_pass", "shadow pass");
	//r_skybox_shader = shader_LoadShader("engine/skybox");

	r_bloom0_shader = shader_LoadShader("engine/bloom0", "bloom pass 0");
	r_bloom1_shader = shader_LoadShader("engine/bloom1", "bloom pass 1");
	r_tonemap_shader = shader_LoadShader("engine/tonemap", "tonemap pass");

	r_blit_texture_shader = shader_LoadShader("engine/blit_texture", "blit texture");
	//r_portal_shader = shader_LoadShader("engine/portal");

	r_gui_shader = shader_LoadShader("engine/gui/gui", "gui");

	r_imediate_color_shader = shader_LoadShader("engine/imediate draw/imediate_color", "imediate color");

	r_cluster_debug_shader = shader_LoadShader("engine/cluster_debug", "cluster debug");

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

	//log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "**************************");
	log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "shader_GetShaderDefaultUniformsLocations: shader [%s] uniforms:", shader->name);

	for(i = 0; i < UNIFORM_LAST_UNIFORM; i++)
	{
		uniform_location = glGetUniformLocation(shader->shader_program, default_uniforms[i]);

		shader->default_uniforms[i].location = uniform_location;
		shader->default_uniforms[i].type = GL_NONE;

		if(shader->default_uniforms[i].location != 0xffffffff)
		{
			//glGetActiveUniformsiv(shader->shader_program, 1, &uniform_location, GL_UNIFORM_TYPE, &uniform_type);
			//shader->default_uniforms[i].type = uniform_type;
			shader->default_uniforms[i].type = default_uniform_types[i];


			log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "shader_GetShaderDefaultUniformsLocations: default uniform [%s (%s)] at [%d]", default_uniforms[i], renderer_GetGLEnumString(shader->default_uniforms[i].type), shader->default_uniforms[i].location);
		}
	}


	if((i = glGetUniformBlockIndex(shader->shader_program, "r_lights_uniform_block")) != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(shader->shader_program, i, R_LIGHT_UNIFORM_BUFFER_BINDING);
	}

	if((i = glGetUniformBlockIndex(shader->shader_program, "r_bsp_uniform_block")) != GL_INVALID_INDEX)
    {
        glUniformBlockBinding(shader->shader_program, i, R_BSP_UNIFORM_BUFFER_BINDING);
    }

    if((i = glGetUniformBlockIndex(shader->shader_program, "r_world_vertices_uniform_block")) != GL_INVALID_INDEX)
    {
        glUniformBlockBinding(shader->shader_program, i, R_WORLD_VERTICES_UNIFORM_BUFFER_BINDING);
    }

	//log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "**************************\n");
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

	log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "shader_CompileShaderSource: compiling shader [%s]", shader->name);

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
		log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "shader_CompileShaderSource: [%s] shader vertex shader compilation stage info log:\n%s", shader->name, shader_log);
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
		log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "shader_CompileShaderSource: [%s] shader fragment shader compilation stage info log:\n%s", shader->name, shader_log);
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
		log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "shader_CompileShaderSource: [%s] shader link stage info log:\n%s", shader->name, shader_log);
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

	log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "shader_CompileShaderSource: shader [%s] compiled successfully", shader->name);

	return 1;
}



int shader_LoadShader(char *file_name, char *shader_name)
{
	int shader_index = -1;
	struct shader_t *shader;

	char *vertex_shader_source;
	char *fragment_shader_source;


	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "shader_LoadShader: loading shader [%s]", shader_name);


    if(shader_LoadShaderSource(file_name, &vertex_shader_source, &fragment_shader_source))
	{
        shader_index = shader_CreateEmptyShader(shader_name);
        shader = shader_GetShaderPointerIndex(shader_index);

        shader->file_name = memory_Strdup(file_name);

		if(!shader_CompileShaderSource(shader, &vertex_shader_source, &fragment_shader_source))
		{
			shader_DestroyShaderIndex(shader_index);
			shader_index = -1;
		}

        memory_Free(vertex_shader_source);
        memory_Free(fragment_shader_source);
	}

	if(shader_index == -1)
	{
		log_LogMessage(LOG_MESSAGE_ERROR, 1, "shader_LoadShader: couldn't load shader [%s]\n", shader_name);
	}
	else
	{
		log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "shader_LoadShader: shader [%s] loaded successfully\n", shader_name);
	}

	return shader_index;

}


void shader_ReloadShader(int shader_index)
{
    struct shader_t *shader;
    char *vertex_shader_source;
    char *fragment_shader_source;

    shader = shader_GetShaderPointerIndex(shader_index);

    if(shader)
	{
		if(shader->file_name)
		{
			if(shader_LoadShaderSource(shader->file_name, &vertex_shader_source, &fragment_shader_source))
			{
                shader_CompileShaderSource(shader, &vertex_shader_source, &fragment_shader_source);

				memory_Free(vertex_shader_source);
				memory_Free(fragment_shader_source);
			}
		}
	}
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

	for(i = 0; i < shd_shaders.element_count; i++)
	{
		shader_ReloadShader(i);
	}

	printf("****************** SHADER HOT RELOAD ******************\n\n");

}






