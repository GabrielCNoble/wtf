#ifndef SHADER_H
#define SHADER_H

#include "shd_common.h"



int shader_Init();

void shader_Finish();

int shader_FindShaderNamedUniformIndex(struct shader_t *shader, char *uniform_name);

int shader_GetShaderNamedUniformIndex(struct shader_t *shader, char *uniform_name);



int shader_CreateEmptyShader(char *shader_name);

void shader_DestroyShaderIndex(int shader_index);

int shader_LoadShaderSource(char *file_name, char **vertex_shader_source, char **fragment_shader_source);

int shader_CompileShaderSource(struct shader_t *shader, char **vertex_shader_source, char **fragment_shader_source);


int shader_LoadShader(char *file_name);

void shader_ReloadShader(int shader_index);

int shader_GetShaderIndex(char *shader_name);

struct shader_t *shader_GetShaderPointerIndex(int shader_index);

void shader_DeleteShaderIndex(int shader_index);

void shader_HotReload();

void shader_DeleteShaderByIndex(int shader_index);

//void shader_UseShader(int shader_index);

void shader_SetCurrentShaderUniform1i(int uniform, int value);

void shader_SetCurrentShaderUniform1f(int uniform, float value);

void shader_SetCurrentShaderUniform4fv(int uniform, float *value);

void shader_SetCurrentShaderUniformMatrix4fv(int uniform, float *value);

void shader_SetCurrentShaderVertexAttribPointer(int attrib, int size, int type, int normalized, int stride, void *pointer);


#endif
