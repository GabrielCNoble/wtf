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
}gpu_head_t;

typedef struct
{
	int size;
	int cursor;
	gpu_head_t *list;
	int free_stack_top;
	int free_stack_size;
	int *free_stack;
}gpu_heap_list_t;


void gpu_Init();

void gpu_Finish();

//gpu_buffer_t gpu_CreateGPUBuffer(int size, int type, int hint);

//void gpu_BindGPUBuffer(gpu_buffer_t *buffer);

//void gpu_FillGPUBuffer(gpu_buffer_t *buffer, int size, int count, void *data);

//void *gpu_MapGPUBuffer(gpu_buffer_t *buffer, int access);

//void gpu_UnmapGPUBuffer(gpu_buffer_t *buffer);

//void gpu_DeleteGPUBuffer(gpu_buffer_t *buffer);

int gpu_Alloc(int size);

int gpu_Realloc(int handle, int size);

void gpu_Free(int handle);

void gpu_ClearHeap();

int gpu_GetAllocStart(int handle);

int gpu_GetAllocSize(int handle);


/* THOSE FUNCTIONS MODIFY THE CURRENT MAPPED VERTEX ARRAY BUFFER */
void gpu_Read(int handle, int offset, void *buffer, int count, int direct);

void gpu_Write(int handle, int offset, void *buffer, int count, int direct);

void gpu_BindGpuHeap();

void gpu_UnbindGpuHeap();



/* those should ALWAYS be used, for the gpu heap management
depens upon info updated by those functions */
void gpu_BindBuffer(int target, unsigned int id);

void *gpu_MapBuffer(int target, int access);

void gpu_UnmapBuffer(int target);







#endif /* GPU_H */




