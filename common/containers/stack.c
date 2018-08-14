#include "stack.h"
#include "c_memory.h"
#include <stdlib.h>
#include <string.h>



struct stack_t stack_create(int elem_size, int stack_size)
{
	struct stack_t stack;

	if(elem_size > 0 && stack_size > 0)
	{
		stack.element_size = elem_size;
		stack.stack_size = stack_size;
		stack.stack_top = -1;
		stack.elements = memory_Calloc(elem_size, stack_size);
	}
	else
	{
		stack.elements = NULL;
	}

	return stack;
}

void stack_destroy(struct stack_t *stack)
{
    if(stack)
	{
		if(stack->elements)
		{
			memory_Free(stack->elements);
		}
	}
}

void stack_push(struct stack_t *stack, void *value)
{
    if(stack && value)
	{
        if(stack->stack_top < stack->stack_size)
		{
            stack->stack_top++;
			memcpy((char *)stack->elements + stack->element_size * stack->stack_top, value, stack->element_size);
		}
	}
}

void *stack_pop(struct stack_t *stack)
{
	void *value = NULL;

	if(stack)
	{
		if(stack->stack_top >= 0)
		{
			value = (char *)stack->elements + stack->stack_top;
			stack->stack_top--;
		}
	}

	return value;
}















