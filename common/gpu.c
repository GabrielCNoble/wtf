#if 0


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpu.h"
#include "vector.h"
#include "log.h"
#include "model.h"
#include "containers/list.h"
#include "containers/stack_list.h"

#include "c_memory.h"

#include "GL\glew.h"

#include "r_debug.h"

#include <assert.h>


#define GPU_HEAP_SIZE 33554432*4			/* 32 MB */
//#define GPU_HEAP_SIZE 1048576*5
#define GPU_MIN_ALLOC sizeof(float)
#define FREE_THRESHOLD 15

#define INDEX_HANDLE_MASK 0x80000000
#define INVALID_HANDLE 0xffffffff

#define UNSET_INDEX_BIT(handle) (handle & (~INDEX_HANDLE_MASK))
#define SET_INDEX_BIT(handle) (handle | INDEX_HANDLE_MASK)
#define IS_INDEX_HANDLE(handle) ((handle & INDEX_HANDLE_MASK) && 1)

//int vcache_size;
//int next_vcache_id;

unsigned int gpu_vertex_heap;
unsigned int gpu_index_heap;

//static int free_count;

int vertex_frees = 0;
int index_frees = 0;

int heap_bound = 0;


//static gpu_heap_list_t vertex_alloc_list;
//static gpu_heap_list_t vertex_free_list;

static struct stack_list_t vertex_alloc_list;
static struct list_t vertex_free_list;


static struct stack_list_t index_alloc_list;
static struct list_t index_free_list;

//static gpu_heap_list_t index_alloc_list;
//static gpu_heap_list_t index_free_list;





static unsigned int mapped_array_buffer;
static int mapped_array_access;
static unsigned int mapped_index_buffer;
static int mapped_index_access;

static unsigned int bound_array_buffer;
static unsigned int bound_index_buffer;
/*...*/


#ifdef __cplusplus
extern "C"
{
#endif

/* internal use only... */
void gpu_Defrag();
void gpu_Sort(struct gpu_head_t *free_list, int left, int right);
void gpu_MergeFreeBlocks(int index_heap);


int gpu_Init()
{

	//while(glGetError() != GL_NO_ERROR);

	R_DBG_PUSH_FUNCTION_NAME();

	glGenBuffers(1, &gpu_vertex_heap);
	glBindBuffer(GL_ARRAY_BUFFER, gpu_vertex_heap);
	glBufferData(GL_ARRAY_BUFFER, GPU_HEAP_SIZE, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &gpu_index_heap);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_index_heap);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, GPU_HEAP_SIZE, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//printf("gpu_Init: %x\n\n", glGetError());

/*	mapped_array_buffer = 0;
	mapped_array_access = 0;
	mapped_index_buffer = 0;
	mapped_index_access = 0;
	bound_array_buffer = 0;
	bound_index_buffer = 0;


	frees = 0;*/

	struct gpu_head_t *allocs;


    vertex_alloc_list = stack_list_create(sizeof(struct gpu_head_t), 32, NULL);
    vertex_free_list = list_create(sizeof(struct gpu_head_t), 32, NULL);


    index_alloc_list = stack_list_create(sizeof(struct gpu_head_t), 32, NULL);
    index_free_list = list_create(sizeof(struct gpu_head_t), 32, NULL);



	vertex_free_list.element_count = 1;
	allocs = (struct gpu_head_t *)vertex_free_list.elements;
	allocs[0].align_offset = 0;
	allocs[0].size = GPU_HEAP_SIZE;
	allocs[0].start = 0;


	index_free_list.element_count = 1;
	allocs = (struct gpu_head_t *)index_free_list.elements;
	allocs[0].align_offset = 0;
	allocs[0].size = GPU_HEAP_SIZE;
	allocs[0].start = 0;



	//vertex_alloc_list.cursor = 0;
	//vertex_alloc_list.size = 3200;
	//vertex_alloc_list.free_stack_top = -1;
	//vertex_alloc_list.free_stack = memory_Malloc(sizeof(int) * vertex_alloc_list.size);
	//vertex_alloc_list.list = memory_Malloc(sizeof(gpu_head_t) * vertex_alloc_list.size);

	//vertex_free_list.cursor = 1;
	//vertex_free_list.size = 3200;
	//vertex_free_list.list = memory_Malloc(sizeof(gpu_head_t) * vertex_free_list.size);
	/* the whole heap is free... */
	//vertex_free_list.list[0].start = 0;
	//vertex_free_list.list[0].size = GPU_HEAP_SIZE;



	//index_alloc_list.cursor = 0;
	//index_alloc_list.size = 3200;
	//index_alloc_list.free_stack_top = -1;
	//index_alloc_list.free_stack = memory_Malloc(sizeof(int) * index_alloc_list.size);
	//index_alloc_list.list = memory_Malloc(sizeof(gpu_head_t) * index_alloc_list.size);

	//index_free_list.cursor = 1;
	//index_free_list.size = 3200;
	//index_free_list.list = memory_Malloc(sizeof(gpu_head_t) * index_free_list.size);
	/* the whole heap is free... */
	//index_free_list.list[0].start = 0;
	//index_free_list.list[0].size = GPU_HEAP_SIZE;

	R_DBG_POP_FUNCTION_NAME();

	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "%s: subsystem initialized properly!", __func__);

	return 1;

}

void gpu_Finish()
{
	glDeleteBuffers(1, &gpu_vertex_heap);
	glDeleteBuffers(1, &gpu_index_heap);


	stack_list_destroy(&vertex_alloc_list);
	stack_list_destroy(&index_alloc_list);

	list_destroy(&vertex_free_list);
	list_destroy(&index_free_list);


	/*memory_Free(vertex_alloc_list.list);
	memory_Free(vertex_alloc_list.free_stack);
	memory_Free(vertex_free_list.list);

	memory_Free(index_alloc_list.list);
	memory_Free(index_alloc_list.free_stack);
	memory_Free(index_free_list.list);*/
}

/*gpu_buffer_t gpu_CreateGPUBuffer(int size, int type, int hint)
{
	gpu_buffer_t buf;
	if(size>0)
	{
		glGenBuffers(1, &buf.buffer_ID);
		glBindBuffer(type, buf.buffer_ID);
		glBufferData(type, size, NULL, hint);


		buf.size=size;
		buf.hint=hint;
		buf.type=type;

		glBindBuffer(type, 0);
		return buf;
	}

	buf.buffer_ID=0;
	return buf;
}


void gpu_BindGPUBuffer(gpu_buffer_t *buffer)
{
	glBindBuffer(buffer->type ,buffer->buffer_ID);
	return;
}



void gpu_FillGPUBuffer(gpu_buffer_t *buffer, int size, int count, void *data)
{
	glBindBuffer(buffer->type, buffer->buffer_ID);
	glBufferData(buffer->type, size*count, data, buffer->hint);
	glBindBuffer(buffer->type, 0);
	return;
}


void *gpu_MapGPUBuffer(gpu_buffer_t *buffer, int access)
{
	void *rtrn;
	glBindBuffer(buffer->type, buffer->buffer_ID);
	rtrn=glMapBuffer(buffer->type, access);
	return rtrn;
}


void gpu_UnmapGPUBuffer(gpu_buffer_t *buffer)
{
	glUnmapBuffer(buffer->type);
	return;
}


void gpu_DeleteGPUBuffer(gpu_buffer_t *buffer)
{
	glDeleteBuffers(1, &buffer->buffer_ID);
	return;
}*/

//int gpu_Alloc(int size)
//{
	//return gpu_AllocAlign(size, 4);
//	return -1;
//}

/*
========================================================
========================================================
========================================================
*/


struct gpu_alloc_handle_t gpu_AllocVerticesAlign(int size, int alignment)
{
	return gpu_AllocAlign(size, alignment, 0);
}

struct gpu_alloc_handle_t gpu_AllocIndexesAlign(int size, int alignment)
{
	return gpu_AllocAlign(size, alignment, 1);
}

struct gpu_alloc_handle_t gpu_AllocAlign(int size, int alignment, int index_alloc)
{
	//int h = INVALID_HANDLE;
	struct gpu_alloc_handle_t handle = INVALID_GPU_ALLOC_HANDLE;
	int free_index;
	//int c = free_list.cursor;
	int free_count;
	struct gpu_head_t *t;
	int *q;
	int f;
	int attempt_defrag = 0;

	struct gpu_head_t *new_alloc;

	struct gpu_head_t *alloc_list;
	struct gpu_head_t *free_list;

	struct stack_list_t *allocs;
	struct list_t *frees;

	//gpu_heap_list_t *alloc_list;
	//gpu_heap_list_t *free_list;

	if(index_alloc)
	{
		allocs = &index_alloc_list;
		frees = &index_free_list;
	}
	else
	{
		allocs = &vertex_alloc_list;
		frees = &vertex_free_list;
	}


	alloc_list = (struct gpu_head_t *)allocs->elements;
	free_list = (struct gpu_head_t *)frees->elements;
	free_count = frees->element_count;

    handle.is_index_alloc = index_alloc && 1;

	if(size > 0)
	{
		/* round the size up to the closest multiple of the alignment... */
		//if(size % sizeof(vertex_t))
		while(size % alignment)
		{
			size += size % alignment;
			size = (size + alignment - 1) & ~(alignment - 1);
		}

		//size -= size % sizeof(vertex_t);


		_try_again:
		/* go over the list of free chunks... */
		for(free_index = 0; free_index < free_count; free_index++)
		{
			/* ...and test to see if any fit the request... */
			//if(free_list->list[i].size >= size)
			if(free_list[free_index].size >= size)
			{
                handle.alloc_index = stack_list_add(allocs, NULL);
                new_alloc = stack_list_get(allocs, handle.alloc_index);

				new_alloc->size = size;
				new_alloc->start = free_list[free_index].start;
				new_alloc->align_offset = 0;

				if(free_list[free_index].size > size)
				{
					/* alloc is smaller than this free block, so just chopp
					a chunk off and set it as alloc'd... */

					free_list[free_index].start += size;
					free_list[free_index].size -= size;
				}
				else
				{
					/* free block fits perfectly the request, so just copy its info
					to the alloc_list and pull the last element of the free_list to
					the now vacant position...*/

                    list_remove(frees, free_index);

				}

				if(new_alloc->start % alignment)
				{
                    new_alloc->align_offset += alignment - new_alloc->start % alignment;
				}

				break;
			}
		}
		/* couldn't find a free block that could
		service the request, so try to defrag the heap and
		repeat the search... */
		if(!attempt_defrag && handle.alloc_index == INVALID_GPU_ALLOC_INDEX)
		{
			attempt_defrag = 1;
			gpu_Defrag(index_alloc);
			goto _try_again;
		}
	}

	return handle;
}

struct gpu_alloc_handle_t gpu_Realloc(struct gpu_alloc_handle_t handle, int size)
{
	return handle;
}

int gpu_IsAllocValid(struct gpu_alloc_handle_t handle)
{
    struct gpu_head_t *alloc;

	if(handle.alloc_index == INVALID_GPU_ALLOC_INDEX)
	{
		return 0;
	}

    if(handle.is_index_alloc)
	{
		if(handle.alloc_index >= index_alloc_list.element_count)
		{
			return 0;
		}

        alloc = (struct gpu_head_t *)index_alloc_list.elements + handle.alloc_index;
	}
	else
	{
		if(handle.alloc_index >= vertex_alloc_list.element_count)
		{
			return 0;
		}

		alloc = (struct gpu_head_t *)vertex_alloc_list.elements + handle.alloc_index;
	}

	return alloc->size > 0;
}

/*
========================================================
========================================================
========================================================
*/


void gpu_FreeVertices(struct gpu_alloc_handle_t handle)
{
	gpu_Free(handle);
}

void gpu_FreeIndexes(struct gpu_alloc_handle_t handle)
{
	gpu_Free(handle);
}

void gpu_Free(struct gpu_alloc_handle_t handle)
{
	struct gpu_head_t *t;
	//gpu_heap_list_t *free_list;
	//gpu_heap_list_t *alloc_list;


	struct gpu_head_t *free_list;
	struct gpu_head_t *alloc_list;
	struct gpu_head_t *alloc;


	struct stack_list_t *allocs;
	struct list_t *frees;


	int free_count;
	int index_alloc = 0;

	if(handle.alloc_index == INVALID_GPU_ALLOC_INDEX)
	{
		printf("gpu_Free: bad handle!\n");
		return;
	}

	//if(IS_INDEX_HANDLE(handle))
	if(handle.is_index_alloc)
	{
		allocs = &index_alloc_list;
		frees = &index_free_list;
		index_frees++;
		free_count = index_frees;
	}
	else
	{
		allocs = &vertex_alloc_list;
		frees = &vertex_free_list;
		vertex_frees++;
		free_count = vertex_frees;
	}


	free_list = (struct gpu_head_t *)frees->elements;
	alloc_list = (struct gpu_head_t *)allocs->elements;


	//if(free_list->cursor >= free_list->size)
	//{
		//t = (gpu_head_t *)malloc(sizeof(gpu_head_t) * (free_list.size + 32));
	//	t = (gpu_head_t *)memory_Malloc(sizeof(gpu_head_t) * (free_list->size + 32));
	//	memcpy(t, free_list->list, sizeof(gpu_head_t) * free_list->size);
		//free(free_list.list);
	//	memory_Free(free_list->list);
	//	free_list->list = t;
	//	free_list->size += 32;
	//}


	list_add(frees, &alloc_list[handle.alloc_index]);
	alloc = stack_list_get(allocs, handle.alloc_index);

	alloc->size = -1;
	alloc->start = -1;
	alloc->align_offset = -1;

	stack_list_remove(allocs, handle.alloc_index);


	//assert(handle > -1);

	//free_list->list[free_list->cursor++] = alloc_list->list[handle.alloc_index];

	//alloc_list->free_stack_top++;

	//alloc_list->free_stack[alloc_list->free_stack_top] = handle.alloc_index;
	//frees++;
	//printf("%d free block heads\n", free_list.cursor);
	if(free_count > FREE_THRESHOLD)
	{
		gpu_Defrag(handle.is_index_alloc);
	}

	//printf("%d free block heads\n", free_list.cursor);
	//printf("%d %d\n", free_list.list[0].size, free_list.list[0].start);
}

/*
========================================================
========================================================
========================================================
*/


void gpu_ClearHeap()
{
	struct gpu_head_t *allocs;

	vertex_free_list.element_count = 1;
	allocs = (struct gpu_head_t *)vertex_free_list.elements;
    allocs[0].align_offset = 0;
    allocs[0].size = GPU_HEAP_SIZE;
    allocs[0].start = 0;

    vertex_alloc_list.element_count = 0;
    vertex_alloc_list.free_stack_top = -1;




    index_free_list.element_count = 1;
	allocs = (struct gpu_head_t *)index_free_list.elements;
    allocs[0].align_offset = 0;
    allocs[0].size = GPU_HEAP_SIZE;
    allocs[0].start = 0;

    index_alloc_list.element_count = 0;
    index_alloc_list.free_stack_top = -1;



	/*vertex_free_list.cursor = 1;
	vertex_free_list.list[0].start = 0;
	vertex_free_list.list[0].size = GPU_HEAP_SIZE;
	vertex_alloc_list.cursor = 0;
	vertex_alloc_list.free_stack_top = -1;
	vertex_frees = 0;


	index_free_list.cursor = 1;
	index_free_list.list[0].start = 0;
	index_free_list.list[0].size = GPU_HEAP_SIZE;
	index_alloc_list.cursor = 0;
	index_alloc_list.free_stack_top = -1;
	index_frees = 0;*/
}

void gpu_Defrag(int index_heap)
{
	register int i;
	//int c = free_list.cursor;
	int old_count;
	int k = 0;
	int j;
	//int new_count = c;
	int new_count;
	struct gpu_head_t *free_list;
	struct list_t *frees;

	if(index_heap)
	{
		frees = &index_free_list;
		//free_list = (struct gpu_head_t *)index_free_list.elements;
		//old_count = index_free_list.element_count;
		index_frees = 0;
	}
	else
	{
		frees = &vertex_free_list;
		//free_list = (struct gpu_head_t *)vertex_free_list.elements;
		//old_count = vertex_free_list.element_count
		vertex_frees = 0;
	}

	free_list = (struct gpu_head_t *)frees->elements;
	old_count = frees->element_count;

	new_count = old_count;

	gpu_Sort(free_list, 0, old_count - 1);

	for(i = 1; i < old_count; i++)
	{
		/* go over the free chunks, testing for contiguity. If there is one free
		chunks after another, merge the two and keep merging until finding a chunk
		that doesn't begin after the end of the last merged chunk. In this case, set this
		rebel chunk as the current chunk and repeat the process...  */
		if(free_list[k].start + free_list[k].size == free_list[i].start)
		{
			free_list[k].size += free_list[i].size;
			free_list[i].size = -1;	/* if this chunk was merged, mark it as so... */
			new_count--;
		}
		else
		{
			k = i;
		}
	}

	/* go over the list of free chunks... */
	for(i = 0; i < old_count; i++)
	{
		/* ... and test for any chunk with negative value.
		If this happens, this chunk has been merged to another
		and is no longer valid... */
		if(free_list[i].size < 0)
		{
			/* starting from this invalid, negative
			sized chunk, go forward until hitting
			a valid chunk... */
			for(j = i; j < old_count; j++)
			{
				if(free_list[j].size > -1)
				{
					/* hit a valid chunk, so move it
					to this currently invalid chunk's position... */
					free_list[i] = free_list[j];
					free_list[j].size = -1;
					i--;
					break;
				}
			}

			if(j >= old_count)
				break;
		}
	}

	//free_list->cursor = new_count;

	//for(i = 0; i < free_list->cursor; i++)
	//{
	//	printf("%d %d\n", free_list->list[i].start, free_list->list[i].size);
	//}
}

void gpu_Sort(struct gpu_head_t *free_list, int left, int right)
{
	int i = left;
	int j = right;
	int m = (right + left) / 2;
	struct gpu_head_t t;
	struct gpu_head_t q = free_list[m];
	while(i <= j)
	{
		for(; free_list[i].start < q.start && i < right; i++);
		for(; free_list[j].start > q.start && j > left; j--);

		if(i <= j)
		{
			t = free_list[i];
			free_list[i] = free_list[j];
			free_list[j] = t;
			i++;
			j--;
		}
	}
	if(j > left) gpu_Sort(free_list, left, j);
	if(i < right) gpu_Sort(free_list, i, right);

	return;
}

int gpu_GetAllocStart(struct gpu_alloc_handle_t handle)
{
	struct gpu_head_t *alloc;

	if(handle.alloc_index != INVALID_GPU_ALLOC_INDEX)
	{
		//if(IS_INDEX_HANDLE(handle))
		if(handle.is_index_alloc)
		{
			alloc = (struct gpu_head_t *)index_alloc_list.elements + handle.alloc_index;
			//return index_alloc_list.list[handle.alloc_index].start + index_alloc_list.list[handle.alloc_index].align_offset;
		}
		else
		{
			alloc = (struct gpu_head_t *)vertex_alloc_list.elements + handle.alloc_index;
			//return vertex_alloc_list.list[handle.alloc_index].start + vertex_alloc_list.list[handle.alloc_index].align_offset;
		}

		return alloc->start + alloc->align_offset;
	}

	return -1;
}

int gpu_GetAllocSize(struct gpu_alloc_handle_t handle)
{
	struct gpu_head_t *alloc;

	if(handle.alloc_index != INVALID_GPU_ALLOC_INDEX)
	{
		//if(IS_INDEX_HANDLE(handle))
		if(handle.is_index_alloc)
		{
			alloc = (struct gpu_head_t *)index_alloc_list.elements + handle.alloc_index;
			//handle = UNSET_INDEX_BIT(handle);
			//return index_alloc_list.list[handle.alloc_index].size - index_alloc_list.list[handle.alloc_index].align_offset;
		}
		else
		{
			alloc = (struct gpu_head_t *)vertex_alloc_list.elements + handle.alloc_index;
			//return vertex_alloc_list.list[handle.alloc_index].size - vertex_alloc_list.list[handle.alloc_index].align_offset;
		}

		return alloc->size;
	}

	return 0;
}

int gpu_GetVertexOffset(struct gpu_alloc_handle_t handle)
{
	return 0;
}

void gpu_Read(struct gpu_alloc_handle_t handle, int offset, void *buffer, int count, int direct)
{
	/*register int i;
	int c = count;
	int start;
	void *p;

	if(direct)
	{
		start = handle;
	}
	else
	{
		start = alloc_list.list[handle].start;
	}

	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	p = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	p = (char *)p + start + offset;
	if(!(count % 4))
	{
		c >>= 2;

		for(i = 0; i < c; i++)
		{
			*((int *)buffer + i) = *((int *)p + i);
		}
	}
	else if(!(count % 2))
	{
		c >>= 1;

		for(i = 0; i < c; i++)
		{
			*((short *)buffer + i) = *((short *)p + i);
		}
	}
	else
	{
		for(i = 0; i < c; i++)
		{
			*((char *)buffer + i) = *((char *)p + i);
		}
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	if(bound_array_buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, bound_array_buffer);
		if(mapped_array_buffer)
		{
			glMapBuffer(GL_ARRAY_BUFFER, mapped_array_access);
		}
	}*/
}

void gpu_Write(struct gpu_alloc_handle_t handle, int offset, void *buffer, int count)
{
	int start;
	int target;
	unsigned int buffer_id;
	void *p;

	struct gpu_head_t *alloc;

	//assert(handle > -1);

	/*if(direct)
	{
		start = handle;
	}
	else
	{
		start = alloc_list.list[handle].start;
		offset = alloc_list.list[handle].align_offset;
	}	*/

	R_DBG_PUSH_FUNCTION_NAME();

	if(handle.alloc_index == INVALID_GPU_ALLOC_INDEX)
	{
		printf("gpu_Write: bad handle!\n");
		R_DBG_POP_FUNCTION_NAME();
		return;
	}

	//if(index_alloc)
	//if(IS_INDEX_HANDLE(handle))
	if(handle.is_index_alloc)
	{
		//handle = UNSET_INDEX_BIT(handle);

		buffer_id = gpu_index_heap;

		alloc = (struct gpu_head_t *)index_alloc_list.elements + handle.alloc_index;

		//start = index_alloc_list.list[handle.alloc_index].start + index_alloc_list.list[handle.alloc_index].align_offset;
		//offset = index_alloc_list.list[handle].align_offset;
		target = GL_ELEMENT_ARRAY_BUFFER;
	}
	else
	{
		buffer_id = gpu_vertex_heap;
		alloc = (struct gpu_head_t *)vertex_alloc_list.elements + handle.alloc_index;
		//start = vertex_alloc_list.list[handle.alloc_index].start + vertex_alloc_list.list[handle.alloc_index].align_offset;
		//offset = vertex_alloc_list.list[handle].align_offset;
		target = GL_ARRAY_BUFFER;
	}


	start = alloc->start + alloc->align_offset;


	glBindBuffer(target, buffer_id);
	p = glMapBuffer(target, GL_WRITE_ONLY);
	p = ((char *)p) + start + offset;



	/* this could cause a major
	performance hit if there's an
	odd number of bytes to be copied,
	but the first byte starts at an even
	memory position. The odd number of
	bytes would be copied and then
	each subsequent 4-byte copy would
	happen on unaligned addresses... */
	asm
	(
		//"movl %0, %%edi\n"
		//"movl %1, %%esi\n"
		//"movl %2, %%ecx\n"

		"mov edi, %0\n"
		"mov esi, %1\n"
		"mov ecx, %2\n"

		//".intel_syntax noprefix\n"
		"test ecx, 3\n"
		"jz _even\n"
		"mov eax, ecx\n"
		"and ecx, 3\n"
		"rep movsb\n"
		"lea ecx, [eax - 3]\n"
		"_even:\n"
		"shr ecx, 2\n"
		"rep movsd\n"

		//".att_syntax prefix\n"
		:: "rm" (p), "rm" (buffer), "rm" (count) : "eax", "ecx", "edi", "esi"
	);
	/* TO REMEMBER: ALWAYS INCLUDE THE CLOBBERED REGISTERS!!!! */

	glUnmapBuffer(target);
	glBindBuffer(target, 0);

	R_DBG_POP_FUNCTION_NAME();
}

void gpu_WriteNonMapped(struct gpu_alloc_handle_t handle, int offset, void *buffer, int count)
{
	int start;
	int target;
	unsigned int current_bound_buffer_id;
	unsigned int heap_buffer_id;
	unsigned int current_bound_target;
	unsigned int buffer_id;
	void *p;

	struct gpu_head_t *alloc;

	R_DBG_PUSH_FUNCTION_NAME();

	if(handle.alloc_index == INVALID_GPU_ALLOC_INDEX)
	{
		printf("gpu_WriteNonMapped: bad handle!\n");
		R_DBG_POP_FUNCTION_NAME();
		return;
	}

	//if(index_alloc)
	//if(IS_INDEX_HANDLE(handle))
	if(handle.is_index_alloc)
	{
		//handle = UNSET_INDEX_BIT(handle);

		buffer_id = gpu_index_heap;
		alloc = (struct gpu_head_t *)index_alloc_list.elements + handle.alloc_index;
		//start = index_alloc_list.list[handle.alloc_index].start + index_alloc_list.list[handle.alloc_index].align_offset;
		target = GL_ELEMENT_ARRAY_BUFFER;
		heap_buffer_id = gpu_index_heap;
		current_bound_target = GL_ELEMENT_ARRAY_BUFFER_BINDING;
	}
	else
	{
		buffer_id = gpu_vertex_heap;
		alloc = (struct gpu_head_t *)vertex_alloc_list.elements + handle.alloc_index;
		//start = vertex_alloc_list.list[handle.alloc_index].start + vertex_alloc_list.list[handle.alloc_index].align_offset;
		target = GL_ARRAY_BUFFER;
		heap_buffer_id = gpu_vertex_heap;
		current_bound_target = GL_ARRAY_BUFFER_BINDING;
	}


	start = alloc->start + alloc->align_offset;

	glGetIntegerv(current_bound_target, &current_bound_buffer_id);

	if(current_bound_buffer_id != heap_buffer_id)
	{
		glBindBuffer(target, heap_buffer_id);
	}

	glBufferSubData(target, start, count, buffer);

	if(current_bound_buffer_id != heap_buffer_id)
	{
		glBindBuffer(target, current_bound_buffer_id);
	}


	//glBindBuffer(target, buffer_id);
	//p = glMapBuffer(target, GL_WRITE_ONLY);
	//p = ((char *)p) + start + offset;



	//glUnmapBuffer(target);
	//glBindBuffer(target, 0);

	R_DBG_POP_FUNCTION_NAME();
}

void *gpu_MapAlloc(struct gpu_alloc_handle_t handle, int access)
{
	int target;
	int buffer;
	void *p = NULL;

	R_DBG_PUSH_FUNCTION_NAME();

	if(handle.alloc_index == INVALID_GPU_ALLOC_INDEX)
	{
		printf("gpu_MapAlloc: bad handle!\n");

		R_DBG_POP_FUNCTION_NAME();

		return NULL;
	}

	//if(IS_INDEX_HANDLE(handle))
	if(handle.is_index_alloc)
	{
		target = GL_ELEMENT_ARRAY_BUFFER;
		buffer = gpu_index_heap;
	}
	else
	{
		target = GL_ARRAY_BUFFER;
		buffer = gpu_vertex_heap;
	}

	glBindBuffer(target, buffer);
	p = glMapBuffer(target, access);

	R_DBG_POP_FUNCTION_NAME();

	return p;
}

void gpu_UnmapAlloc(struct gpu_alloc_handle_t handle)
{
	int target;

	R_DBG_PUSH_FUNCTION_NAME();

	if(handle.alloc_index == INVALID_GPU_ALLOC_INDEX)
	{
		printf("gpu_UnmapAlloc: bad handle!\n");

		R_DBG_POP_FUNCTION_NAME();
		return;
	}

	//if(IS_INDEX_HANDLE(handle))
	if(handle.is_index_alloc)
	{
		target = GL_ELEMENT_ARRAY_BUFFER;
	}
	else
	{
		target = GL_ARRAY_BUFFER;
	}

	glUnmapBuffer(target);
	if(!heap_bound)
	{
		glBindBuffer(target, 0);
	}

	R_DBG_POP_FUNCTION_NAME();
}

void gpu_BindGpuHeap()
{
	R_DBG_PUSH_FUNCTION_NAME();

	glBindBuffer(GL_ARRAY_BUFFER, gpu_vertex_heap);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_index_heap);
	heap_bound = 1;

	R_DBG_POP_FUNCTION_NAME();
}

void gpu_UnbindGpuHeap()
{

	R_DBG_PUSH_FUNCTION_NAME();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	heap_bound = 0;

	R_DBG_POP_FUNCTION_NAME();
}

#ifdef __cplusplus
}
#endif


#endif



