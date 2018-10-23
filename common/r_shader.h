#ifndef R_SHADER_H
#define R_SHADER_H

#include "shader.h"

#ifdef __cplusplus
extern "C"
{
#endif


void renderer_SetShader(int shader_index);

void renderer_SetUniform1i(uniform_t *uniform, int value);

void renderer_SetUniform1iv(uniform_t *uniform, int count, int *value);

void renderer_SetUniform1ui(uniform_t *uniform, unsigned int value);

void renderer_SetUniform1f(uniform_t *uniform, float value);

void renderer_SetUniform4fv(uniform_t *uniform, int count, float *value);

void renderer_SetUniformMatrix4fv(uniform_t *uniform, float *value);

/*
==============================================================
==============================================================
==============================================================
*/

void renderer_SetDefaultUniform1i(int uniform, int value);

void renderer_SetNamedUniform1i(char *uniform, int value);



void renderer_SetDefaultUniform1iv(int uniform, int count, int *value);



void renderer_SetDefaultUniform1ui(int uniform, unsigned int value);


void renderer_SetDefaultUniform1f(int uniform, float value);

void renderer_SetNamedUniform1f(char *uniform, float value);


void renderer_SetDefaultUniform4fv(int uniform, int count, float *value);

void renderer_SetNamedUniform4fv(char *uniform, int count, float *value);


void renderer_SetDefaultUniformMatrix4fv(int uniform, float *value);

void renderer_SetNamedUniformMatrix4fv(char *uniform, float *value);

/*
==============================================================
==============================================================
==============================================================
*/

void renderer_SetVertexAttribPointer(int attrib, int size, int type, int normalized, int stride, void *pointer);

void renderer_ClearVertexAttribPointers();

struct shader_t *renderer_GetActiveShaderPointer();

void renderer_SetCurrentVertexFormat(int format);


/*
==============================================================
==============================================================
==============================================================
*/

#ifdef __cplusplus
}
#endif


#endif
