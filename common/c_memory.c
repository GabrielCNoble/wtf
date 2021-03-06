#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "c_memory.h"
#include "log.h"
#include "SDL2\SDL_mutex.h"


#define USE_GUARD_BYTES
#define USE_LOTS_OF_BYTES
#define KEEP_CALLER_NAME
#define USE_LONG_CALLER_NAME

//#define LOG_OUTPUT
#define STDOUT_OUTPUT


#ifdef USE_LONG_CALLER_NAME
	#define MAX_CALLER_NAME 64
#else
	#define MAX_CALLER_NAME 20
#endif


#ifdef USE_GUARD_BYTES
	#ifdef USE_LOTS_OF_BYTES
		#define GUARD_BYTE_COUNT 32
	#else
		#define GUARD_BYTE_COUNT 4
	#endif
#else
	#define GUARD_BYTE_COUNT 0
#endif


static int mem_alloc_count = 0;

unsigned long mem_total_bytes = 0;

typedef struct alloc_header_t
{
	#ifdef USE_GUARD_BYTES
	unsigned char header_guard_bytes[GUARD_BYTE_COUNT];
	#endif

	struct alloc_header_t *next;
	struct alloc_header_t *prev;

	#ifdef KEEP_CALLER_NAME
	char caller[MAX_CALLER_NAME];
	#endif

	unsigned int size;
}alloc_header_t;


 static int mem_initialized = 0;
 static SDL_mutex *mem_mutex = NULL;


static alloc_header_t *head = NULL;
static alloc_header_t *tail = NULL;

#ifdef __cplusplus
extern "C"
{
#endif

void memory_Init()
{
	if(!mem_initialized)
	{
		mem_initialized = 1;
		mem_mutex = SDL_CreateMutex();
	}
}

void memory_Finish()
{
	alloc_header_t *cur;
	alloc_header_t *next;

	if(mem_initialized)
	{
		cur = head;
		while(cur)
		{
			next = cur->next;
			memory_Free((char *)cur + sizeof(alloc_header_t));
			cur = next;
		}

		mem_initialized = 0;
		SDL_DestroyMutex(mem_mutex);
	}
}


void *memory_SetupHeader(void *memory, int size, const char *caller)
{
	alloc_header_t *header;
	int i;
	unsigned char *tail_guard_bytes;

	header = memory;
	memory = (char *)memory + sizeof(alloc_header_t);

	header->size = size;
	header->next = NULL;
	header->prev = NULL;

	#ifdef KEEP_CALLER_NAME
	if(caller)
	{
		for(i = 0; i < MAX_CALLER_NAME && caller[i]; i++)
		{
			header->caller[i] = caller[i];
		}

		if(i == MAX_CALLER_NAME) i--;

		header->caller[i] = '\0';
	}
	else
	{
		strcpy(header->caller, "non-declared");
	}
	#endif

	if(!head)
	{
		head = header;
	}
	else
	{
		tail->next = header;
		header->prev = tail;
	}

	#ifdef USE_GUARD_BYTES

	for(i = 0; i < GUARD_BYTE_COUNT; i++)
	{
		header->header_guard_bytes[i] = 0xff;
	}

	tail_guard_bytes = (unsigned char *)memory + size;

	for(i = 0; i < GUARD_BYTE_COUNT; i++)
	{
		tail_guard_bytes[i] = 0xff;
	}

	#endif


	tail = header;

	mem_alloc_count++;
	mem_total_bytes += size;

	return memory;
}


void *memory_MallocCaller(size_t size, const char *caller)
{
	void *mem = NULL;


	SDL_LockMutex(mem_mutex);

	mem = malloc(size + sizeof(alloc_header_t) + GUARD_BYTE_COUNT);

	if(!mem)
	{
		#ifdef KEEP_CALLER_NAME
		if(caller)
		{
		//	printf("memory_Malloc: call from %s returned a null pointer!\n", caller);
			log_LogMessage(LOG_MESSAGE_ERROR, 1, "memory_Malloc: call from %s returned a null pointer!", caller);
		}
		else
		{
		//	printf("memory_Malloc: call returned a null pointer!\n");
			log_LogMessage(LOG_MESSAGE_ERROR, 1, "memory_Malloc: call returned a null pointer!");
		}
		#endif
	}
	else
	{
		mem = memory_SetupHeader(mem, size, caller);
	}

	SDL_UnlockMutex(mem_mutex);

	return mem;

}

void *memory_CallocCaller(size_t count, size_t elem_size, const char *caller)
{
	void *mem;

	if(count < 1 || elem_size < 1)
	{
		#ifdef KEEP_CALLER_NAME
		if(caller)
		{
			log_LogMessage(LOG_MESSAGE_ERROR, 1, "memory_Calloc: bad params from %s!", caller);
		}
		else
		{
			log_LogMessage(LOG_MESSAGE_ERROR, 1, "memory_Calloc: bad params!");
		}
		#endif

		return NULL;
	}

	SDL_LockMutex(mem_mutex);

	mem = calloc(1, count * elem_size + sizeof(alloc_header_t) + GUARD_BYTE_COUNT);

	if(!mem)
	{
		#ifdef KEEP_CALLER_NAME
		if(caller)
		{
			log_LogMessage(LOG_MESSAGE_ERROR, 1, "memory_Calloc: call from %s returned a null pointer!", caller);
		}
		else
		{
			log_LogMessage(LOG_MESSAGE_ERROR, 1, "memory_Calloc: call returned a null pointer!");
		}
		#endif
	}
	else
	{
		mem = memory_SetupHeader(mem, count * elem_size, caller);
	}

	SDL_UnlockMutex(mem_mutex);

	return mem;
}

void memory_Free(void *memory)
{
	alloc_header_t *header;
	header = (alloc_header_t *)((char *)memory - sizeof(alloc_header_t));

	SDL_LockMutex(mem_mutex);

	if(header == head)
	{
		head = header->next;

		if(head)
		{
			head->prev = NULL;
		}
	}
	else
	{
		header->prev->next = header->next;

		if(header->next)
		{
			header->next->prev = header->prev;
		}
		else
		{
			tail = header->prev;
		}
	}

    mem_total_bytes -= header->size;
	mem_alloc_count--;

	free(header);

	SDL_UnlockMutex(mem_mutex);
}

char *memory_StrdupCaller(char *src, const char *caller)
{
	int len;
	char *memory;
	len = strlen(src) + 1;

	memory = memory_MallocCaller(len, caller);
	strcpy(memory, src);

	return memory;
}

void memory_Report(int report_allocs)
{
	alloc_header_t *header;
	size_t total_allocd = 0;
	int visited_headers = 0;

	//SDL_LockMutex(mem_mutex);
	header = head;
    if(report_allocs)
    {
        while(header)
        {
            #ifdef KEEP_CALLER_NAME
        	log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "memory_Report: alloc of %d bytes from %s", header->size, header->caller);
            #else
        	log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "memory_Report: alloc of %d bytes", header->size);
            #endif

            header = header->next;
        }
    }

	log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "memory_Report: %d allocd bytes remain", total_allocd);

//	SDL_UnlockMutex(mem_mutex);

}

void memory_ReportFromCaller(char *caller)
{
	alloc_header_t *header;
	size_t total_allocd = 0;
	int visited_headers = 0;

//	SDL_LockMutex(mem_mutex);
	header = head;

	#ifndef KEEP_CALLER_NAME
	return;
	#endif // KEEP_CALLER_NAME


	while(header)
	{
		if(!strcmp(header->caller, caller))
		{
			//printf("memory_ReportFromCaller: alloc of %d bytes from %s\n", header->size, header->caller);
			log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "memory_Report: alloc of %d bytes from %s", header->size, header->caller);
			total_allocd += header->size;
		}

		header = header->next;
	}
	log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "memory_ReportFromCaller: %d allocd bytes remain from %s", total_allocd, caller);

	//SDL_UnlockMutex(mem_mutex);
}

void memory_CheckCorrupted()
{
	alloc_header_t *header;
	unsigned char *tail_guard_bytes;
	unsigned long corrupted_count = 0;
	int corrupt;
	int i;
	int visited_headers = 0;

//	SDL_LockMutex(mem_mutex);

	header = head;

	while(header)
	{
		corrupt = 0;

		for(i = 0; i < GUARD_BYTE_COUNT; i++)
		{
			if(header->header_guard_bytes[i] != 0xff)
			{
				corrupt |= 1;
			}
		}

		tail_guard_bytes = (unsigned char *)header + sizeof(alloc_header_t) + header->size;

		for(i = 0; i < GUARD_BYTE_COUNT; i++)
		{
			if(tail_guard_bytes[i] != 0xff)
			{
				corrupt |= 2;
			}
		}


		if(corrupt)
		{
			if(corrupt & 1)
			{
				#ifdef KEEP_CALLER_NAME
				log_LogMessage(LOG_MESSAGE_WARNING, 1, "memory_CheckCorrupted: alloc at 0x%x (%s): header guard bytes are corrupt!\n", header, header->caller);
				#else
				log_LogMessage(LOG_MESSAGE_WARNING, 1, "memory_CheckCorrupted: alloc at 0x%x: header guard bytes are corrupt!\n", header);
				#endif
			}

			if(corrupt & 2)
			{
				#ifdef KEEP_CALLER_NAME
				log_LogMessage(LOG_MESSAGE_WARNING, 1, "memory_CheckCorrupted: alloc at 0x%x (%s): tail guard bytes are corrupt!\n", header, header->caller);
				#else
				log_LogMessage(LOG_MESSAGE_WARNING, 1, "memory_CheckCorrupted: alloc at 0x%x: tail guard bytes are corrupt!\n", header);
				#endif
			}

			corrupted_count++;
		}

		header = header->next;
	}


	if(corrupted_count)
	{
		log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "memory_CheckCorrupted: %d corrupted allocs found!", corrupted_count);
	}
	else
	{
		log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "memory_CheckCorrupted: no memory corruption detected...");
	}

	//SDL_UnlockMutex(mem_mutex);
}

#ifdef __cplusplus
}
#endif






