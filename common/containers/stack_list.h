#ifndef STACK_LIST_H
#define STACK_LIST_H

#include <stdio.h>


struct stack_list_t
{
	int element_size;
	int element_count;
	int max_elements;

	int free_stack_top;

	void *elements;
	int *free_stack;

	void (*dispose_callback)(void *element);
};


#ifdef __cplusplus
extern "C"
{
#endif

/*
=====================================================
=====================================================
=====================================================
*/

struct stack_list_t stack_list_create(int element_size, int max_elements, void (*dispose_callback)(void *element));

void stack_list_destroy(struct stack_list_t *stack_list);

void stack_list_resize(struct stack_list_t *stack_list, int increment);

void stack_list_copy(struct stack_list_t *from, struct stack_list_t *to);

int stack_list_add(struct stack_list_t *stack_list, void *data);

void stack_list_remove(struct stack_list_t *stack_list, int index);

void *stack_list_get(struct stack_list_t *stack_list, int index);


//#include "stack_list.inl"


/*
=====================================================
=====================================================
=====================================================
*/

#ifdef __cplusplus
}
#endif

#endif
