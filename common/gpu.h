#ifndef GPU_H
#define GPU_H

typedef struct
{
	int size;
	unsigned int hint;
	unsigned int type;
	unsigned int buffer_ID;
}gpu_buffer_t;

#define INVALID_GPU_ALLOC_INDEX 0x7fffffff

struct gpu_alloc_handle_t
{
    unsigned index_alloc : 1;
    unsigned alloc_index : 31;
};

#define INVALID_GPU_ALLOC_HANDLE (struct gpu_alloc_handle_t){1, INVALID_GPU_ALLOC_INDEX}

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

struct gpu_alloc_handle_t gpu_AllocVerticesAlign(int size, int alignment);

struct gpu_alloc_handle_t gpu_AllocIndexesAlign(int size, int alignment);

struct gpu_alloc_handle_t gpu_AllocAlign(int size, int alignment, int index_alloc);

struct gpu_alloc_handle_t gpu_Realloc(struct gpu_alloc_handle_t handle, int size);

/*
========================================================
========================================================
========================================================
*/

void gpu_FreeVertices(struct gpu_alloc_handle_t handle);

void gpu_FreeIndexes(struct gpu_alloc_handle_t handle);

void gpu_Free(struct gpu_alloc_handle_t handle);

/*
========================================================
========================================================
========================================================
*/


void gpu_ClearHeap();

int gpu_GetAllocStart(struct gpu_alloc_handle_t handle);

int gpu_GetAllocSize(struct gpu_alloc_handle_t handle);

int gpu_GetVertexOffset(struct gpu_alloc_handle_t handle);


/* THOSE FUNCTIONS MODIFY THE CURRENT MAPPED BUFFERS */
void gpu_Read(struct gpu_alloc_handle_t handle, int offset, void *buffer, int count, int direct);

void gpu_Write(struct gpu_alloc_handle_t handle, int offset, void *buffer, int count);

void gpu_WriteNonMapped(struct gpu_alloc_handle_t handle, int offset, void *buffer, int count);

void *gpu_MapAlloc(struct gpu_alloc_handle_t handle, int acess);

void gpu_UnmapAlloc(struct gpu_alloc_handle_t handle);

void gpu_BindGpuHeap();

void gpu_UnbindGpuHeap();


#ifdef __cplusplus
}
#endif




#endif /* GPU_H */




