#include "stack_list.h"

#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif 


__forceinline int stack_list_add(struct stack_list_t *stack_list, void *data)
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

__forceinline void stack_list_remove(struct stack_list_t *stack_list, int index)
{
	if(stack_list->free_stack_top < stack_list->max_elements - 1)
	{
		stack_list->free_stack_top++;
		stack_list->free_stack[stack_list->free_stack_top] = index;
	}
}

__forceinline void *stack_list_get(struct stack_list_t *stack_list, int index)
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



