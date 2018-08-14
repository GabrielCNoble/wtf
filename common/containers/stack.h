#ifndef STACK_H
#define STACK_H



struct stack_t
{
    int element_size;
    int stack_top;
    int stack_size;
    void *elements;
};




struct stack_t stack_create(int elem_size, int stack_size);

void stack_destroy(struct stack_t *stack);

void stack_push(struct stack_t *stack, void *value);

void *stack_pop(struct stack_t *stack);


#endif // STACK_H
