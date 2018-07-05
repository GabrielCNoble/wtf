#ifndef R_IMEDIATE_H
#define R_IMEDIATE_H


#include "GL/glew.h"
#include "vector.h"
#include "matrix.h"

#define GL_ENGINE_TRIANGLE_MESH 0x0040
#define GL_ENGINE_TRIANGLE_CUBE 0x0041

void renderer_InitImediateDrawing();

void renderer_FinishImediateDrawing();

void renderer_EnableImediateDrawing();

void renderer_DisableImediateDrawing();


/*
==============================================================
==============================================================
==============================================================
*/

void renderer_Begin(GLenum mode);

void renderer_End();

void renderer_Vertex3f(float x, float y, float z);

void renderer_Normal3f(float nx, float ny, float nz);

void renderer_TexCoord2f(float s, float t);

void renderer_Color3f(float r, float g, float b);

void renderer_Color4f(float r, float g, float b, float a);

void renderer_Rectf(float x0, float y0, float x1, float y1);

void renderer_DrawVerts(GLenum mode, int count, int size, int stride, void *verts);

void renderer_DrawVertsIndexed(GLenum mode, int count, int size, int stride, void *vertices, void *indexes);

/*
==============================================================
==============================================================
==============================================================
*/


/*
==============================================================
==============================================================
==============================================================
*/


#endif
