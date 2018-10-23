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

int shader_LoadShader(char *file_name, char *shader_name);

void shader_ReloadShader(int shader_index);

int shader_GetShaderIndex(char *shader_name);

struct shader_t *shader_GetShaderPointerIndex(int shader_index);

void shader_DeleteShaderIndex(int shader_index);

void shader_HotReload();


#endif
