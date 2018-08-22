#include "multi_list.h"
#include "c_memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


struct multi_list_t multi_list_create(int initial_size, int list_count, ...)
{
	struct multi_list_t list;
	va_list args;

	int i;

	int element_size;

	va_start(args, list_count);

	memset(&list, 0, sizeof(struct multi_list_t));

	list.max_elements = initial_size;

	list.list_count = list_count;
	list.lists = memory_Calloc(list_count, sizeof(struct list_data_t));

	for(i = 0; i < list_count; i++)
	{
		element_size = va_arg(args, int);
		list.lists[i].element_size = element_size;
		list.lists[i].data = memory_Calloc(initial_size, element_size);
	}

	return list;
}

void multi_list_destroy(struct multi_list_t *list)
{
	int i;

	for(i = 0; i < list->list_count; i++)
	{
		memory_Free(list->lists[i].data);
	}

	memory_Free(list->lists);

	memset(list, 0, sizeof(struct multi_list_t));
}

void multi_list_advance(struct multi_list_t *list)
{

}

void multi_list_recede(struct multi_list_t *list)
{

}

int multi_list_add(struct multi_list_t *list, int list_index, void *data)
{

}

void multi_list_resize(struct multi_list_t *list, int increment)
{
    int i;
    void *new_data;

    struct list_data_t *list_data;

    for(i = 0; i < list->list_count; i++)
	{
		list_data = list->lists + i;

        new_data = memory_Calloc(list->max_elements + increment, list_data->element_size);
		memcpy(new_data, list_data->data, list->max_elements * list_data->element_size);
		memory_Free(list_data->data);

		list_data->data = new_data;
	}

	list->max_elements += increment;
}









