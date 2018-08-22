#ifndef ALLOC_LIST_H
#define ALLOC_LIST_H

#include "stack_list.h"

struct alloc_t
{
    unsigned int start;
    unsigned int size;
};

struct alloc_handle_t
{
    int alloc_index;
};

struct alloc_list_t
{
    //struct list_t alloc_handles;
    //struct list_t free_handles;

    struct stack_list_t allocs;

    int absolute_available_size;

    int size;

    void *memory;
};



struct alloc_list_t alloc_list_create(int initial_memory_size);

void alloc_list_destroy(struct alloc_list_t *list);

struct alloc_handle_t alloc_list_alloc(struct alloc_list_t *list, int size);

struct alloc_handle_t alloc_list_realloc(struct alloc_list_t *list, struct alloc_handle_t alloc, int new_size);

void alloc_list_free(struct alloc_list_t *list, struct alloc_handle_t alloc);






#endif // ALLOC_LIST_H
