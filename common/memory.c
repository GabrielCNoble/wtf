#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "log.h"


#define MAX_CALLER_NAME 20

typedef struct alloc_header_t
{
	struct alloc_header_t *next;
	struct alloc_header_t *prev;
	char caller[MAX_CALLER_NAME];
	unsigned int size;	
}alloc_header_t;
 

alloc_header_t *head = NULL;
alloc_header_t *tail = NULL;


void *memory_Malloc(size_t size, char *caller)
{
	void *mem;
	alloc_header_t *header;
	mem = malloc(size + sizeof(alloc_header_t));
	int i;
	
	if(!mem)
	{
		if(caller)
		{
			log_LogMessage(LOG_MESSAGE_ERROR, "memory_Malloc: call at %s returned a null pointer!\n", caller);
		}
		else
		{
			log_LogMessage(LOG_MESSAGE_ERROR, "memory_Malloc: call returned a null pointer!\n");
		}
	}
	else
	{ 
		header = mem;
		mem = (char *)mem + sizeof(alloc_header_t);
		
		//header->base = header;
		//header->memory = mem;
		header->size = size;
		header->next = NULL;
		header->prev = NULL;
		
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
			strcpy(header->caller, "non-declared caller");
		}
		
		if(!head)
		{
			head = header;
		}
		else
		{
			tail->next = header;
			header->prev = tail;
		}
		
		tail = header;
		
	}
	
	return mem;
	
}

void *memory_Calloc(size_t count, size_t elem_size, char *caller)
{
	void *mem;
	alloc_header_t *header;
	
	if(count < 1 || elem_size < 1)
	{
		if(caller)
		{
			log_LogMessage(LOG_MESSAGE_ERROR, "memory_Calloc: bad params from %s!\n", caller);
		}
		else
		{
			log_LogMessage(LOG_MESSAGE_ERROR, "memory_Calloc: bad params!\n");
		}
		
		return NULL;
	}
	
	mem = calloc(1, count * elem_size + sizeof(alloc_header_t));
	int i;
	
	if(!mem)
	{
		if(caller)
		{
			log_LogMessage(LOG_MESSAGE_ERROR, "memory_Calloc: call from %s returned a null pointer!\n", caller);
		}
		else
		{
			log_LogMessage(LOG_MESSAGE_ERROR, "memory_Calloc: call returned a null pointer!\n");
		}
	}
	else
	{ 
		header = mem;
		mem = (char *)mem + sizeof(alloc_header_t);
		
		header->size = count * elem_size;
		header->next = NULL;
		header->prev = NULL;
		
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
			strcpy(header->caller, "non-declared caller");
		}
		
		if(!head)
		{
			head = header;
		}
		else
		{
			tail->next = header;
			header->prev = tail;
		}
		
		tail = header;
		
	}
	
	return mem;
}

void memory_Free(void *memory)
{
	alloc_header_t *header;
	header = (alloc_header_t *)((char *)memory - sizeof(alloc_header_t));
	
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
	
	free(header);
}

char *memory_Strdup(char *src, char *caller)
{
	int len;
	char *memory;
	len = strlen(src);
	
	memory = memory_Malloc(len + 1, caller);
	strcpy(memory, src);
	
	return memory;
}

void memory_Report()
{
	alloc_header_t *header;
	size_t total_allocd = 0;
	header = head;
	
	while(header)
	{
		printf("memory_Report: alloc of %d bytes from %s\n", header->size, header->caller);
		log_LogMessage(LOG_MESSAGE_NOTIFY, "memory_Report: alloc of %d bytes from %s", header->size, header->caller);
		total_allocd += header->size;
		header = header->next;
	}
	
	printf("memory_Report: %d allocd bytes remain\n", total_allocd);
	log_LogMessage(LOG_MESSAGE_NOTIFY, "memory_Report: %d allocd bytes remain", total_allocd);
	
}








