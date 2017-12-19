#ifndef SHADER_H
#define SHADER_H

#define LIGHT_PARAMS_UNIFORM_BUFFER_BINDING 1

enum SHADER_UNIFORMS
{
	UNIFORM_TEXTURE_SAMPLER0,
	UNIFORM_TEXTURE_SAMPLER1,
	UNIFORM_TEXTURE_CUBE_SAMPLER0,
	UNIFORM_TEXTURE_FLAGS,
	UNIFORM_LIGHT_COUNT,
	UNIFORM_LIGHT_INDEX,
};

typedef struct
{
	
	char *name;
	
	unsigned int shader_program;
	
	unsigned char vertex_position;
	unsigned char vertex_normal;
	unsigned char vertex_tangent;
	unsigned char vertex_tex_coords;
	
	unsigned short texture_sampler0;
	unsigned short texture_sampler1;
	unsigned short texture_cube_sampler0;
	//unsigned short cluster_texture;
	unsigned short texture_flags;
	//unsigned short light_count;
	unsigned short shader_index;
	unsigned short light_index;
	unsigned short align0;
	

}shader_t;




void shader_Init();

void shader_Finish();

int shader_LoadShader(char *file_name);

void shader_DeleteShaderByIndex(int shader_index);

void shader_UseShader(int shader_index);

void shader_SetCurrentShaderUniform1i(int uniform, int value);


#endif
