#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpu.h"
#include "vector.h"
#include "mesh.h"

#include "GL\glew.h"


#define GPU_HEAP_SIZE 33554432			/* 32 MB */
#define GPU_MIN_ALLOC sizeof(vertex_t)			
#define FREE_THRESHOLD 15

//int vcache_size;
//int next_vcache_id;

unsigned int gpu_heap;

static int frees;


static gpu_heap_list_t alloc_list;
static gpu_heap_list_t free_list;


static unsigned int mapped_array_buffer;
static int mapped_array_access;
static unsigned int mapped_index_buffer;
static int mapped_index_access;

static unsigned int bound_array_buffer;
static unsigned int bound_index_buffer;
/*...*/

/* internal use only... */
void gpu_Defrag();
void gpu_Sort(int left, int right);


int gpu_Init()
{
	//while(glGetError() != GL_NO_ERROR);
	glGenBuffers(1, &gpu_heap);
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	glBufferData(GL_ARRAY_BUFFER, GPU_HEAP_SIZE, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//printf("gpu_Init: %x\n\n", glGetError());
	
	mapped_array_buffer = 0;
	mapped_array_access = 0;
	mapped_index_buffer = 0;
	mapped_index_access = 0;
	bound_array_buffer = 0;
	bound_index_buffer = 0;
	
	frees = 0;
	
	alloc_list.cursor = 0;
	alloc_list.size = 3200;
	alloc_list.free_stack_top = -1;
	alloc_list.free_stack_size = 3200;
	alloc_list.free_stack = (int *)malloc(sizeof(int) * alloc_list.size);
	alloc_list.list = (gpu_head_t *)malloc(sizeof(gpu_head_t) * alloc_list.size);
	
	
	free_list.cursor = 1;
	free_list.size = 3200;
	free_list.list = (gpu_head_t *)malloc(sizeof(gpu_head_t) * free_list.size);
	
	
	/* the whole heap is free... */
	free_list.list[0].start = 0;
	free_list.list[0].size = GPU_HEAP_SIZE;
	
	return 1;
	
}

void gpu_Finish()
{
	glDeleteBuffers(1, &gpu_heap);
	free(alloc_list.list);
	free(alloc_list.free_stack);
	free(free_list.list);
	//free(free_list.free_stack);
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

int gpu_Alloc(int size)
{
	int h = -1;
	register int i;
	int c = free_list.cursor;
	gpu_head_t *t;
	int *q;
	int f;
	int attempt_defrag = 0;
	
	if(alloc_list.cursor >= alloc_list.size)
	{
		t = (gpu_head_t *)malloc(sizeof(gpu_head_t) * (alloc_list.size + 32));
		q = (int *)malloc(sizeof(int) * (alloc_list.size + 32));
		memcpy(t, alloc_list.list, sizeof(gpu_head_t) * alloc_list.size);
		free(alloc_list.list);
		free(alloc_list.free_stack);
		alloc_list.list = t;
		alloc_list.free_stack = q;
		alloc_list.size += 32;
	}
	
	if(size > 0)
	{
		/* round the size up to the closest multiple of the minimum allowed allocation... */
		//if(size % sizeof(vertex_t))
		while(size % sizeof(vertex_t))
		{
			size += size % sizeof(vertex_t);
			size = (size + sizeof(vertex_t) - 1) & ~(sizeof(vertex_t) - 1);
		}
		
		//size -= size % sizeof(vertex_t);
		

		_try_again:
		/* go over the list of free chunks... */	
		for(i = 0; i < c; i++)
		{
			/* ...and test to see if any fit the request... */
			if(free_list.list[i].size >= size)
			{
			
				if(alloc_list.free_stack_top > -1)
				{
					h = alloc_list.free_stack[alloc_list.free_stack_top--];
				}
				else
				{
					h = alloc_list.cursor++;
				}
				//h = f;
				
				alloc_list.list[h].size = size;
				alloc_list.list[h].start = free_list.list[i].start;
				
				if(free_list.list[i].size > size)
				{
					/* alloc is smaller than this free block, so just chopp
					a chunk off and set it as alloc'd... */
					
					free_list.list[i].start += size;
					free_list.list[i].size -= size;
				}
				else
				{
					/* free block fits perfectly the request, so just copy its info
					to the alloc_list and pull the last element of the free_list to
					the now vacant position...*/
					
					free_list.list[i].size = -1;
					if(i < free_list.cursor - 1)
					{
						free_list.list[i] = free_list.list[free_list.cursor - 1];
						free_list.cursor--;
					}
				}
				
				break;
			}
		}
		/* couldn't find a free block that could
		service the request, so try to defrag the heap and
		repeat the search... */
		if(!attempt_defrag && h == -1)
		{
			attempt_defrag = 1;
			gpu_Defrag();
			goto _try_again;
		}
	}
	return h;
}

int gpu_Realloc(int handle, int size)
{
	return handle;
}

void gpu_Free(int handle)
{
	gpu_head_t *t;
	
	if(free_list.cursor >= free_list.size)
	{
		t = (gpu_head_t *)malloc(sizeof(gpu_head_t) * (free_list.size + 32));
		memcpy(t, free_list.list, sizeof(gpu_head_t) * free_list.size);
		free(free_list.list);
		free_list.list = t;
		free_list.size += 32;
	}

	free_list.list[free_list.cursor++] = alloc_list.list[handle];
	alloc_list.free_stack[++alloc_list.free_stack_top] = handle;
	frees++;
	//printf("%d free block heads\n", free_list.cursor);
	if(frees > FREE_THRESHOLD)
	{
		gpu_Defrag();
	}
	
	//printf("%d free block heads\n", free_list.cursor);
	//printf("%d %d\n", free_list.list[0].size, free_list.list[0].start);
}

void gpu_ClearHeap()
{
	free_list.cursor = 1;
	free_list.list[0].start = 0;
	free_list.list[0].size = GPU_HEAP_SIZE;
	
	alloc_list.cursor = 0;
	alloc_list.free_stack_top = -1;
	frees = 0;
}

void gpu_Defrag()
{
	register int i;
	int c = free_list.cursor;
	int k = 0;
	int j;
	int new_count = c;
	gpu_Sort(0, free_list.cursor - 1);
	
	for(i = 1; i < c; i++)
	{
		/* go over the free chunks, testing for contiguity. If there is one free
		chunks after another, merge the two and keep merging until finding a chunk
		that doesn't begin after the end of the merged chunk. In this case, set this
		rebel chunk as the current chunk and repeat the process...  */
		if(free_list.list[k].start + free_list.list[k].size == free_list.list[i].start)
		{
			free_list.list[k].size += free_list.list[i].size;
			free_list.list[i].size = -1;	/* if this chunk was merged, mark as so... */
			new_count--;
		}
		else
		{
			k = i;
		}
	}
	
	/* go over the list of free chunks... */
	for(i = 0; i < c; i++)
	{
		/* ... and test for any chunk with negative value.
		If this happens, this chunk has been merged to another
		and is no longer valid... */
		if(free_list.list[i].size < 0)
		{
			/* starting from this invalid, negative
			sized chunk, go forward until hitting
			a valid chunk... */
			for(j = i; j < c; j++)
			{
				if(free_list.list[j].size > -1)
				{
					/* hit a valid chunk, so move it
					to this currently invalid chunk's position... */
					free_list.list[i] = free_list.list[j];
					free_list.list[j].size = -1;
					i = j - 1;
					break;
				}
			}
		}
	}
	
	free_list.cursor = new_count;
	
	//printf("%d free blocks\n", new_count);
	//printf("%d %d\n", free_list.list[0].size, free_list.list[0].start);
	
	frees = 0;
}

void gpu_Sort(int left, int right)
{
	int i = left;
	int j = right;
	int m = (right + left) / 2;
	gpu_head_t t;
	gpu_head_t q = free_list.list[m];
	while(i <= j)
	{
		for(; free_list.list[i].start < q.start && i < right; i++);
		for(; free_list.list[j].start > q.start && j > left; j--);
		
		if(i <= j)
		{
			t = free_list.list[i];
			free_list.list[i] = free_list.list[j];
			free_list.list[j] = t;
			i++;
			j--;
		} 
	}
	if(j > left) gpu_Sort(left, j);
	if(i < right) gpu_Sort(i, right);
	
	return;
}

int gpu_GetAllocStart(int handle)
{
	if(handle >= 0)
	{
		return alloc_list.list[handle].start;
	}
}

int gpu_GetAllocSize(int handle)
{
	if(handle >= 0)
	{
		return alloc_list.list[handle].size;
	}
}

void gpu_Read(int handle, int offset, void *buffer, int count, int direct)
{
	register int i;
	int c = count;
	//int start = alloc_list.list[handle].start;
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
	}
}

void gpu_Write(int handle, int offset, void *buffer, int count, int direct)
{
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
	p = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
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
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	if(bound_array_buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, bound_array_buffer);
		if(mapped_array_buffer)
		{
			glMapBuffer(GL_ARRAY_BUFFER, mapped_array_access);
		}
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void gpu_BindGpuHeap()
{
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
}

void gpu_UnbindGpuHeap()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void gpu_BindBuffer(int target, unsigned int id)
{
	switch(target)
	{
		case GL_ARRAY_BUFFER:
			bound_array_buffer = id;
		break;
			
		case GL_ELEMENT_ARRAY_BUFFER:
			bound_index_buffer = id;
		break;
	}
	
	glBindBuffer(target, id);
}

void *gpu_MapBuffer(int target, int access)
{
	void *r = glMapBuffer(target, access);
	if(r)
	{
		switch(target)
		{
			case GL_ARRAY_BUFFER:
				mapped_array_buffer = bound_array_buffer;
				mapped_array_access = access;
			break;
			
			case GL_ELEMENT_ARRAY_BUFFER:
				mapped_index_buffer = bound_index_buffer;
				mapped_index_access = access;
			break;
		}
	}
	return r;
}

void gpu_UnmapBuffer(int target)
{
	switch(target)
	{
		case GL_ARRAY_BUFFER:
			mapped_array_buffer = 0;
			mapped_array_access = 0;
		break;
			
		case GL_ELEMENT_ARRAY_BUFFER:
			mapped_index_buffer = 0;
			mapped_index_access = 0;
		break;
	}
	
	glUnmapBuffer(target);
}








