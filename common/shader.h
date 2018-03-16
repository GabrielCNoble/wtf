#ifndef SHADER_H
#define SHADER_H

#define LIGHT_PARAMS_UNIFORM_BUFFER_BINDING 0
#define TEST_UNIFORM_BUFFER_BINDING 1

enum SHADER_UNIFORMS
{
	UNIFORM_texture_sampler0 = 0,
	UNIFORM_texture_sampler1,
	UNIFORM_texture_sampler2,
	UNIFORM_texture_cube_sampler0,
	UNIFORM_r_frame,
	UNIFORM_r_width,
	UNIFORM_r_height,
	UNIFORM_r_bloom_radius,
	UNIFORM_cluster_texture,
	UNIFORM_active_camera_position,
	UNIFORM_material_flags,
	UNIFORM_projection_matrix,
	UNIFORM_view_matrix,
	UNIFORM_model_matrix,
	UNIFORM_view_projection_matrix,
	UNIFORM_model_view_projection_matrix,
	UNIFORM_LAST_UNIFORM
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
	unsigned short location;
//	unsigned short type;
}uniform_t;

typedef struct
{
	
	char *name;
	char *file_name;
	
	unsigned int shader_program;
	
	unsigned char vertex_position;
	unsigned char vertex_normal;
	unsigned char vertex_tangent;
	unsigned char vertex_tex_coords;
	
	uniform_t *uniforms;
	
	unsigned short shader_index;
	unsigned short bm_flags;

}shader_t;




int shader_Init();

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
