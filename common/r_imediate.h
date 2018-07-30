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

/* 

NOTE:

Those functions could do just fine with float params only, but for
some reason gcc is passing floats (4 bytes) as double (8 bytes), which
makes the function read garbage from the stack when retrieving its params.

Not sure this is a expected behaviour, given that this won't happen in 
some places. In some calls, gcc pushes the params of those functions 
onto the stack by copying its value into eax and then eax into the stack.

Where it happens however, gcc is fld'ing a double (from a QWORD 
address) and fstp'ing it into the stack to a (QWORD address), which means 
that the type it's reading is 64 bytes long...

*/

void renderer_Vertex3f(double x, double y, double z);

void renderer_Normal3f(double nx, double ny, double nz);

void renderer_TexCoord2f(double s, double t);

void renderer_Color3f(double r, double g, double b);

void renderer_Color4f(double r, double g, double b, double a);

void renderer_Rectf(double x0, double y0, double x1, double y1);

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
