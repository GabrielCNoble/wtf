#include "alloc_list.h"
#include "c_memory.h"

#include <stdlib.h>
#include <string.h>


struct alloc_list_t alloc_list_create(int initial_memory_size)
{
    struct alloc_list_t list;

    memset(&list, 0, sizeof(struct alloc_list_t));

    list.size = initial_memory_size;
    list.absolute_available_size = initial_memory_size;

    list.memory = memory_Calloc(1, initial_memory_size);

    list.allocs = stack_list_create(sizeof(struct alloc_t), 512, NULL);

    //list.alloc_handles = list_create(sizeof(struct alloc_t), 512, NULL);
    //list.free_handles = list_create(sizeof(struct alloc_t), 512, NULL);

}

void alloc_list_destroy(struct alloc_list_t *list)
{
	if(list)
	{
        memory_Free(list->memory);
		stack_list_destroy(&list->allocs);
		memset(list, 0, sizeof(struct alloc_list_t));
	}
}

struct alloc_handle_t alloc_list_alloc(struct alloc_list_t *list, int size)
{
	struct alloc_t *alloc;
	struct alloc_handle_t handle;
	int alloc_index;


    if(list)
	{
		//alloc_index = stack_list_add()
	}
}

struct alloc_handle_t alloc_list_realloc(struct alloc_list_t *list, struct alloc_handle_t alloc, int new_size)
{

}

void alloc_list_free(struct alloc_list_t *list, struct alloc_handle_t alloc)
{

}









