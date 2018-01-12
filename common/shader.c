#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shader.h"
#include "mesh.h"
#include "l_common.h"
#include "GL\glew.h"



static int shader_list_size;
static int shader_count;
static int free_position_stack_top;
static int *free_position_stack;
static shader_t *shaders;
static int active_shader;

/* from r_main.c */
extern int z_pre_pass_shader;
extern int forward_pass_shader;
extern int geometry_pass_shader;
extern int shading_pass_shader;
extern int stencil_lights_pass_shader;
extern int shadow_pass_shader;
extern int skybox_shader;


extern int forward_pass_brush_shader;


extern int draw_bsp_shader;


/* from editor.c */
extern int brush_pick_shader;
extern int light_pick_shader;

static char shader_base_path[512];

void shader_Init(char *shader_path)
{
	int i;
	int c;
	
	strcpy(shader_base_path, shader_path);
	c = strlen(shader_base_path);
	
	i = c;
	while(shader_base_path[i] == ' ' || shader_base_path[i] == '\\')i--;
	shader_base_path[i] = '\0';	
		
	shader_list_size = 16;
	shader_count = 0;
	free_position_stack_top = -1;
	shaders = malloc(sizeof(shader_t ) * shader_list_size);
	free_position_stack = malloc(sizeof(int) * shader_list_size);
	
	z_pre_pass_shader = shader_LoadShader("z_pre_pass");
	forward_pass_shader = shader_LoadShader("forward_pass");
	forward_pass_brush_shader = shader_LoadShader("forward_pass_brush");
	geometry_pass_shader = shader_LoadShader("geometry_buffer");
	shading_pass_shader = shader_LoadShader("shade_pass");
	stencil_lights_pass_shader = shader_LoadShader("stencil_light_pass");
	shadow_pass_shader = shader_LoadShader("shadow_pass");
	skybox_shader = shader_LoadShader("skybox");
	brush_pick_shader = shader_LoadShader("brush_pick");
	light_pick_shader = shader_LoadShader("light_pick");
	draw_bsp_shader = shader_LoadShader("draw_bsp");
}

void shader_Finish()
{
	int i;
	for(i = 0; i < shader_count; i++)
	{
		free(shaders[i].name);
		glDeleteProgram(shaders[i].shader_program);
	}
	
	free(shaders);
	free(free_position_stack);
}

int shader_LoadShader(char *file_name)
{
	FILE *f;
	shader_t *shader;
	int *itemp;
	
	unsigned int vertex_shader_object;
	unsigned int fragment_shader_object;
	unsigned int shader_program;
	
	int shader_index;
	int status;
	int error_log_len;
	int i;
	
	unsigned long file_size;
	
	char *vertex_shader_source;
	char *fragment_shader_source;
	char *error_log;
		
	char vertex_shader_file_name[128];
	char fragment_shader_file_name[128];
	char shader_file_full_path[512];
	

	strcpy(vertex_shader_file_name, file_name);
	strcat(vertex_shader_file_name, ".vert");
	
	strcpy(shader_file_full_path, shader_base_path);
	strcat(shader_file_full_path, "\\");
	strcat(shader_file_full_path, vertex_shader_file_name);
	
	if(!(f = fopen(shader_file_full_path, "r")))
	{
		printf("couldn't load vertex shader file for shader %s!!\n", file_name);
		return -1;
	}
	
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);
	
	vertex_shader_source = calloc(file_size + 2, 1);
	fread(vertex_shader_source, 1, file_size, f);
	vertex_shader_source[file_size - 1] = '\0';
	vertex_shader_source[file_size] = '\0';
	
	fclose(f);
	
	vertex_shader_object = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader_object, 1, (const GLchar **)&vertex_shader_source, NULL);
	glCompileShader(vertex_shader_object);
	glGetShaderiv(vertex_shader_object, GL_COMPILE_STATUS, &status);
	free(vertex_shader_source);
	
	glGetShaderiv(vertex_shader_object, GL_INFO_LOG_LENGTH, &error_log_len);
	
	if(error_log_len > 0)
	{
		error_log = malloc(error_log_len);
		glGetShaderInfoLog(vertex_shader_object, error_log_len, NULL, error_log);
		printf("********************************************\n[%s] shader vertex shader compilation stage info log:\n%s\n", file_name, error_log);
		free(error_log);	
	}
		
	
	if(!status)
	{
		glDeleteShader(vertex_shader_object);
		return -1;
	}
	
	
	strcpy(fragment_shader_file_name, file_name);
	strcat(fragment_shader_file_name, ".frag");

	strcpy(shader_file_full_path, shader_base_path);
	strcat(shader_file_full_path, "\\");
	strcat(shader_file_full_path, fragment_shader_file_name);
	
	if(!(f = fopen(shader_file_full_path, "r")))
	{
		printf("couldn't load fragment shader file for shader %s!!\n", file_name);
		return -1;
	}
	
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);
	
	fragment_shader_source = calloc(file_size + 2, 1);
	fread(fragment_shader_source, 1, file_size, f);
	fragment_shader_source[file_size - 1] = '\0';
	fragment_shader_source[file_size] = '\0';
	
	fclose(f);
	
	fragment_shader_object = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader_object, 1, (const GLchar **)&fragment_shader_source, NULL);
	glCompileShader(fragment_shader_object);
	glGetShaderiv(fragment_shader_object, GL_COMPILE_STATUS, &status);
	free(fragment_shader_source);
	
	glGetShaderiv(fragment_shader_object, GL_INFO_LOG_LENGTH, &error_log_len);
	
	if(error_log_len > 0)
	{
		error_log = malloc(error_log_len);
		glGetShaderInfoLog(fragment_shader_object, error_log_len, NULL, error_log);
		printf("********************************************\n[%s] shader fragment shader compilation stage info log:\n%s\n", file_name, error_log);
		free(error_log);
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
		return -1;
	}

	
	shader_program = glCreateProgram();
	
	glAttachShader(shader_program, vertex_shader_object);
	glAttachShader(shader_program, fragment_shader_object);
	glLinkProgram(shader_program);
	
	glDeleteShader(vertex_shader_object);
	glDeleteShader(fragment_shader_object);
	
	glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
	
	glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &error_log_len);
	
	if(error_log_len > 0)
	{
		error_log = malloc(error_log_len);
		glGetProgramInfoLog(shader_program, error_log_len, NULL, error_log);
		printf("********************************************\n[%s] shader link stage info log:\n%s\n", file_name, error_log);
		free(error_log);
	}
	

	if(!status)
	{
		glDeleteShader(vertex_shader_object);
		glDeleteShader(fragment_shader_object);
		glDeleteProgram(shader_program);
		return -1;
	}
	
	
	
	
	if(free_position_stack_top >= 0)
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
	}
	
	
	
	
	
	shader = &shaders[shader_index];
	shader->shader_program = shader_program;
	shader->name = strdup(file_name);	
	
	shader->vertex_position = glGetAttribLocation(shader->shader_program, "vertex_position");
	shader->vertex_normal = glGetAttribLocation(shader->shader_program, "vertex_normal");
	shader->vertex_tangent = glGetAttribLocation(shader->shader_program, "vertex_tangent");
	shader->vertex_tex_coords = glGetAttribLocation(shader->shader_program, "vertex_tex_coords");	
	
	shader->texture_sampler0 = glGetUniformLocation(shader->shader_program, "texture_sampler0");
	shader->texture_sampler1 = glGetUniformLocation(shader->shader_program, "texture_sampler1");
	shader->texture_sampler2 = glGetUniformLocation(shader->shader_program, "texture_sampler2");
	shader->texture_cube_sampler0 = glGetUniformLocation(shader->shader_program, "texture_cube_sampler0");
	shader->shadow_sampler = glGetUniformLocation(shader->shader_program, "shadow_sampler");
	shader->camera_to_light_matrix = glGetUniformLocation(shader->shader_program, "camera_to_light_matrix");
	shader->frame = glGetUniformLocation(shader->shader_program, "r_frame");
	shader->width = glGetUniformLocation(shader->shader_program, "r_width");
	shader->height = glGetUniformLocation(shader->shader_program, "r_height");
	shader->cluster_texture = glGetUniformLocation(shader->shader_program, "cluster_texture");
	shader->texture_flags = glGetUniformLocation(shader->shader_program, "texture_flags");
	shader->active_camera_position = glGetUniformLocation(shader->shader_program, "active_camera_position");
	//shader->light_count = glGetUniformLocation(shader->shader_program, "light_count");
	shader->light_index = glGetUniformLocation(shader->shader_program, "light_index");
	
	printf("%s: %d %d %d\n", file_name, shader->width, shader->height, shader->cluster_texture);
	
	/*if((i = glGetUniformBlockIndex(shader->shader_program, "camera_params_uniform_block")) != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(shader->shader_program, i, 0);
	}*/
	
	if((i = glGetUniformBlockIndex(shader->shader_program, "light_params_uniform_block")) != GL_INVALID_INDEX)
	{
		glUniformBlockBinding(shader->shader_program, i, LIGHT_PARAMS_UNIFORM_BUFFER_BINDING);
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


	return shader_index;
	
}

void shader_DeleteShaderByIndex(int shader_index)
{
	if(shader_index >= 0 && shader_index < shader_count)
	{
		if(shaders[shader_index].shader_index != -1)
		{
			shaders[shader_index].shader_index = -1;
			free_position_stack_top++;
			free_position_stack[free_position_stack_top] = shader_index;
		}
	}
}

void shader_UseShader(int shader_index)
{	
	shader_t *shader;
	active_shader = shader_index;
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
	

	
	/*glEnableVertexAttribArray(shader->vertex_tangent);
	glVertexAttribPointer(shader->vertex_tangent, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)&(((vertex_t *)0)->tangent));*/
	
	
	
	
}


void shader_SetCurrentShaderUniform1i(int uniform, int value)
{	
	switch(uniform)
	{
		case UNIFORM_TEXTURE_SAMPLER0:
			uniform = shaders[active_shader].texture_sampler0;
		break;
		
		case UNIFORM_TEXTURE_SAMPLER1:
			uniform = shaders[active_shader].texture_sampler1;
		break;
		
		case UNIFORM_TEXTURE_SAMPLER2:
			uniform = shaders[active_shader].texture_sampler2;
		break;
		
		case UNIFORM_TEXTURE_CUBE_SAMPLER0:
			uniform = shaders[active_shader].texture_cube_sampler0;
		break;
		
		case UNIFORM_SHADOW_SAMPLER:
			uniform = shaders[active_shader].shadow_sampler;
		break;
		
		case UNIFORM_TEXTURE_FLAGS:
			uniform = shaders[active_shader].texture_flags;
		break;
		
		case UNIFORM_LIGHT_INDEX:
			uniform = shaders[active_shader].light_index;
		break;
		
		case UNIFORM_FRAME:
			uniform = shaders[active_shader].frame;
		break;
		
		case UNIFORM_WIDTH:
			uniform = shaders[active_shader].width;
		break;
		
		case UNIFORM_HEIGHT:
			uniform = shaders[active_shader].height;
		break;
		
		case UNIFORM_CLUSTER_TEXTURE:
			uniform = shaders[active_shader].cluster_texture;
		break;
		
		default:
			return;
		break;
	}
	
	glUniform1i(uniform, value);
}

void shader_SetCurrentShaderUniform1f(int uniform, float value)
{
	switch(uniform)
	{
		case UNIFORM_WIDTH:
			uniform = shaders[active_shader].width;
		break;
		
		case UNIFORM_HEIGHT:
			uniform = shaders[active_shader].height;
		break;
		
		default:
			return;
	}
	
	glUniform1f(uniform, value);
}

void shader_SetCurrentShaderUniform4fv(int uniform, float *value)
{
	switch(uniform)
	{
		case UNIFORM_ACTIVE_CAMERA_POSITION:
			uniform = shaders[active_shader].active_camera_position;
		break;
		
		default:
			return;
	}
	
	glUniform4fv(uniform, 1, value);
}

void shader_SetCurrentShaderUniformMatrix4fv(int uniform, float *value)
{
	switch(uniform)
	{
		case UNIFORM_CAMERA_TO_LIGHT_MATRIX:
			uniform = shaders[active_shader].camera_to_light_matrix;
		break;
		
		default:
			return;
		break;
	}
	
	glUniformMatrix4fv(uniform, 1, GL_FALSE, value);
}

void shader_SetCurrentShaderVertexAttribPointer(int attrib, int size, int type, int normalized, int stride, void *pointer)
{
	int att;
	switch(attrib)
	{
		case ATTRIB_VERTEX_POSITION:
			att = shaders[active_shader].vertex_position;
		break;
		
		default:
			return;
		break;
	}
	
	glVertexAttribPointer(att, size, type, normalized, stride, (const void *)pointer);
}







