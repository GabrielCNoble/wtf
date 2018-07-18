#include "stack_list.h"
#include "memory.h"

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
	
	stack_list.elements = memory_Calloc(max_elements, element_size, "create_stack_list");
	stack_list.free_stack = memory_Calloc(max_elements, sizeof(int), "create_stack_list");
	
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
	stack_list->free_stack = memory_Calloc(stack_list->max_elements + increment, sizeof(int), "stack_list_add");
			
	elements = memory_Calloc(stack_list->max_elements + increment, stack_list->element_size, "stack_list_add");
	//memmove(elements, stack_list->elements, stack_list->element_size * stack_list->max_elements);
	memcpy(elements, stack_list->elements, stack_list->element_size * stack_list->max_elements);
			
	memory_Free(stack_list->elements);
			
	stack_list->elements = elements;
	stack_list->max_elements += increment;
}

void stack_list_copy(struct stack_list_t *from, struct stack_list_t *to)
{
	
}

#ifdef __cplusplus
}
#endif
