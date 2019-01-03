#ifndef R_VERTS_H
#define R_VERTS_H


//#include "gpu.h"




struct gpu_alloc_handle_t
{
    unsigned is_index_alloc : 1;
    unsigned alloc_index : 31;
};

#define INVALID_GPU_ALLOC_INDEX 0x7fffffff
#define INVALID_GPU_ALLOC_HANDLE (struct gpu_alloc_handle_t){1, INVALID_GPU_ALLOC_INDEX}

struct gpu_head_t
{
	int start;
	int size;
	int align_offset;
	//unsigned short alignment;
};

#ifdef __cplusplus
extern "C"
{
#endif

void renderer_InitVerts();

void renderer_FinishVerts();

struct gpu_alloc_handle_t renderer_AllocVerticesAlign(int size, int alignment);

struct gpu_alloc_handle_t renderer_AllocIndexesAlign(int size, int alignment);

struct gpu_alloc_handle_t renderer_AllocAlign(int size, int alignment, int index_alloc);

struct gpu_alloc_handle_t renderer_Realloc(struct gpu_alloc_handle_t handle, int size);

int renderer_IsAllocValid(struct gpu_alloc_handle_t handle);

/*
========================================================
========================================================
========================================================
*/

void renderer_FreeVertices(struct gpu_alloc_handle_t handle);

void renderer_FreeIndexes(struct gpu_alloc_handle_t handle);

void renderer_Free(struct gpu_alloc_handle_t handle);

/*
========================================================
========================================================
========================================================
*/


void renderer_ClearHeap();

int renderer_GetAllocStart(struct gpu_alloc_handle_t handle);

int renderer_GetAllocSize(struct gpu_alloc_handle_t handle);

int renderer_GetVertexOffset(struct gpu_alloc_handle_t handle);


/* THOSE FUNCTIONS MODIFY THE CURRENT MAPPED BUFFERS */
void renderer_Read(struct gpu_alloc_handle_t handle, int offset, void *buffer, int count, int direct);

void renderer_Write(struct gpu_alloc_handle_t handle, int offset, void *buffer, int count);

void renderer_WriteNonMapped(struct gpu_alloc_handle_t handle, int offset, void *buffer, int count);

void *renderer_MapAlloc(struct gpu_alloc_handle_t handle, int acess);

void renderer_UnmapAlloc(struct gpu_alloc_handle_t handle);

void renderer_EnableVertexReads();

void renderer_DisableVertexReads();

#ifdef __cplusplus
}
#endif

#endif // R_VERTS_H
