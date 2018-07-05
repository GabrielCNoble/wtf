#ifndef GPU_H
#define GPU_H

typedef struct
{
	int size;
	unsigned int hint;
	unsigned int type;
	unsigned int buffer_ID;
}gpu_buffer_t;

typedef struct
{
	int start;
	int size;
	int align_offset;
}gpu_head_t;

typedef struct
{
	int size;
	int cursor;
	gpu_head_t *list;
	int free_stack_top;
	//int free_stack_size;
	int *free_stack;
}gpu_heap_list_t;


#ifdef __cplusplus
extern "C"
{
#endif

int gpu_Init();

void gpu_Finish();

//gpu_buffer_t gpu_CreateGPUBuffer(int size, int type, int hint);

//void gpu_BindGPUBuffer(gpu_buffer_t *buffer);

//void gpu_FillGPUBuffer(gpu_buffer_t *buffer, int size, int count, void *data);

//void *gpu_MapGPUBuffer(gpu_buffer_t *buffer, int access);

//void gpu_UnmapGPUBuffer(gpu_buffer_t *buffer);

//void gpu_DeleteGPUBuffer(gpu_buffer_t *buffer);

//int gpu_Alloc(int size);

/*
========================================================
========================================================
========================================================
*/

int gpu_AllocVerticesAlign(int size, int alignment);

int gpu_AllocIndexesAlign(int size, int alignment);

int gpu_AllocAlign(int size, int alignment, int index_alloc);

int gpu_Realloc(int handle, int size);

/*
========================================================
========================================================
========================================================
*/

void gpu_FreeVertices(int handle);

void gpu_FreeIndexes(int handle);

void gpu_Free(int handle);

/*
========================================================
========================================================
========================================================
*/


void gpu_ClearHeap();

int gpu_GetAllocStart(int handle);

int gpu_GetAllocSize(int handle);

int gpu_GetVertexOffset(int handle);


/* THOSE FUNCTIONS MODIFY THE CURRENT MAPPED BUFFERS */
void gpu_Read(int handle, int offset, void *buffer, int count, int direct);

void gpu_Write(int handle, int offset, void *buffer, int count);

void gpu_WriteNonMapped(int handle, int offset, void *buffer, int count);

void *gpu_MapAlloc(int handle, int acess);

void gpu_UnmapAlloc(int handle);

void gpu_BindGpuHeap();

void gpu_UnbindGpuHeap();


#ifdef __cplusplus
}
#endif




#endif /* GPU_H */




