#ifndef SHADER_H
#define SHADER_H

#define LIGHT_PARAMS_UNIFORM_BUFFER_BINDING 0
#define TEST_UNIFORM_BUFFER_BINDING 1

enum SHADER_UNIFORMS
{
	UNIFORM_TEXTURE_SAMPLER0,
	UNIFORM_TEXTURE_SAMPLER1,
	UNIFORM_TEXTURE_SAMPLER2,
	UNIFORM_TEXTURE_CUBE_SAMPLER0,
	UNIFORM_SHADOW_SAMPLER,
	UNIFORM_TEXTURE_FLAGS,
	UNIFORM_FRAME,
	UNIFORM_WIDTH,
	UNIFORM_HEIGHT,
	UNIFORM_LIGHT_COUNT,
	UNIFORM_LIGHT_INDEX,
	UNIFORM_CAMERA_TO_LIGHT_MATRIX,
	UNIFORM_CLUSTER_TEXTURE,
	UNIFORM_ACTIVE_CAMERA_POSITION,
};

enum SHADER_ATTRIBS
{
	ATTRIB_VERTEX_POSITION = 1,
	ATTRIB_VERTEX_NORMAL,
	ATTRIB_VERTEX_TANGENT,
	ATTRIB_VERTEX_TEX_COORDS,
};

enum SHADER_FLAGS
{
	SHADER_INVALID = 1,
};

typedef struct
{
	
	char *name;
	char *file_name;
	
	unsigned int shader_program;
	
	unsigned char vertex_position;
	unsigned char vertex_normal;
	unsigned char vertex_tangent;
	unsigned char vertex_tex_coords;
	
	unsigned short texture_sampler0;
	unsigned short texture_sampler1;
	unsigned short texture_sampler2;
	unsigned short texture_cube_sampler0;
	unsigned short shadow_sampler;
	unsigned short camera_to_light_matrix;
	unsigned short frame;
	unsigned short width;
	unsigned short height;
	unsigned short active_camera_position;
	//unsigned short render_target_width;
	//unsigned short render_target_height;
	unsigned short cluster_texture;
	unsigned short texture_flags;
	//unsigned short light_count;
	unsigned short shader_index;
	unsigned short light_index;
	
	unsigned short bm_flags;
	//unsigned short align0;
	

}shader_t;




int shader_Init(char *shader_path);

void shader_Finish();

int shader_LoadShader(char *file_name);

void shader_ReloadShader(int shader_index);

int shader_GetShaderIndex(char *shader_name);

void shader_DeleteShaderIndex(int shader_index);

void shader_HotReload();

void shader_DeleteShaderByIndex(int shader_index);

void shader_UseShader(int shader_index);

void shader_SetCurrentShaderUniform1i(int uniform, int value);

void shader_SetCurrentShaderUniform1f(int uniform, float value);

void shader_SetCurrentShaderUniform4fv(int uniform, float *value);

void shader_SetCurrentShaderUniformMatrix4fv(int uniform, float *value);

void shader_SetCurrentShaderVertexAttribPointer(int attrib, int size, int type, int normalized, int stride, void *pointer);


#endif
