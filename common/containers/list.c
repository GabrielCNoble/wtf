#include "list.h"
#include "c_memory.h"

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct list_t list_create(int element_size, int max_elements, void (*dispose_callback)(void *element))
{
	struct list_t list;

	list.element_size = element_size;
	list.element_count = 0;
	list.max_elements = max_elements;
//	list.dispose_callback = dispose_callback;

	list.elements = memory_Calloc(list.max_elements, list.element_size);

	return list;
}

void list_destroy(struct list_t *list)
{
	int i;

//	if(list->dispose_callback)
//	{
//		for(i = 0; i < list->element_count; i++)
//		{
//			list->dispose_callback((char *)list->elements + list->element_size * i);
//		}
//	}

	memory_Free(list->elements);
}

int list_add(struct list_t *list, void *data)
{
	void *items;
	int item_index;

	item_index = list->element_count;
	list->element_count++;

	if(item_index >= list->max_elements)
	{
		items = memory_Calloc(list->max_elements + 64, list->element_size);
		memcpy(items, list->elements, list->element_size * list->element_count);

		memory_Free(list->elements);

		list->elements = items;
		list->max_elements += 64;
	}

	if(data)
	{
		memcpy((char *)list->elements + list->element_size * item_index, data, list->element_size);
	}

	return item_index;
}

void list_remove(struct list_t *list, int index)
{
	if(index < list->element_count - 1)
	{
		memcpy((char *)list->elements + list->element_size * index, (char *)list->elements + list->element_size * (list->element_count - 1), list->element_size);
	}
	list->element_count--;
}

void *list_get(struct list_t *list, int index)
{
	if(index >= 0 && index < list->element_count)
	{
		return (char *)list->elements + list->element_size * index;
	}

	return NULL;
}

int list_get_count(struct list_t *list)
{
	return list->element_count;
}

void list_resize(struct list_t *list, int new_size)
{
	void *elems;

	int new_count;

	if(new_size <= 0)
	{
		return;
	}

	//new_size = (new_size + 3) & (~3);

	if(new_size > list->max_elements)
	{
		elems = memory_Calloc(new_size, list->element_size);
		memcpy(elems, list->elements, list->element_size * list->max_elements);
		memory_Free(list->elements);
		list->elements = elems;
	}

	list->max_elements = new_size;
}

#ifdef __cplusplus
}
#endif







