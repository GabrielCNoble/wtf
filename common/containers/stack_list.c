#include "stack_list.h"
#include "c_memory.h"

#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct stack_list_t stack_list_create(int element_size, int max_elements, void (*dispose_callback)(void *element))
{
	struct stack_list_t stack_list;

	stack_list.element_count = 0;
	stack_list.element_size = element_size;
	stack_list.max_elements = max_elements;
	stack_list.free_stack_top = -1;
	stack_list.dispose_callback = dispose_callback;

	stack_list.elements = memory_Calloc(max_elements, element_size);
	stack_list.free_stack = memory_Calloc(max_elements, sizeof(int));

	return stack_list;
}

void stack_list_destroy(struct stack_list_t *stack_list)
{
	int i;
	int j;

	if(stack_list->dispose_callback)
	{
		for(i = 0; i < stack_list->element_count; i++)
		{
			stack_list->dispose_callback((char *)stack_list->elements + stack_list->element_size * i);
		}
	}

	if(stack_list->elements)
	{
		memory_Free(stack_list->elements);
		memory_Free(stack_list->free_stack);
	}

	stack_list->elements = NULL;
	stack_list->free_stack = NULL;
}

void stack_list_resize(struct stack_list_t *stack_list, int increment)
{
	void *elements;

   	memory_Free(stack_list->free_stack);
	stack_list->free_stack = memory_Calloc(stack_list->max_elements + increment, sizeof(int));
	elements = memory_Calloc(stack_list->max_elements + increment, stack_list->element_size);

	memcpy(elements, stack_list->elements, stack_list->element_size * stack_list->max_elements);

	memory_Free(stack_list->elements);

	stack_list->elements = elements;
	stack_list->max_elements += increment;
}

void stack_list_copy(struct stack_list_t *from, struct stack_list_t *to)
{

}

int stack_list_add(struct stack_list_t *stack_list, void *data)
{
    int item_index;

	if(stack_list->free_stack_top >= 0)
	{
		item_index = stack_list->free_stack[stack_list->free_stack_top];
		stack_list->free_stack_top--;
	}
	else
	{
		item_index = stack_list->element_count;
		stack_list->element_count++;

		if(item_index >= stack_list->max_elements)
		{
			stack_list_resize(stack_list, 64);
		}
	}

	if(data)
	{
		memcpy((char *)stack_list->elements + stack_list->element_size * item_index, data, stack_list->element_size);
	}

	return item_index;
}

void stack_list_remove(struct stack_list_t *stack_list, int index)
{
    //if(stack_list->free_stack_top < stack_list->max_elements - 1)
	//{
	stack_list->free_stack_top++;
	stack_list->free_stack[stack_list->free_stack_top] = index;
	//}
}

void *stack_list_get(struct stack_list_t *stack_list, int index)
{
    if(index >= 0 && index < stack_list->element_count)
	{
		return (char *)stack_list->elements + stack_list->element_size * index;
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif
