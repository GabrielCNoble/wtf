#include "scr_array.h"
#include "c_memory.h"
#include "script.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "angelscript.h"


struct script_array_t *script_array_Constructor(void *type_info)
{
	struct script_array_t *array;
	asITypeInfo *tinfo;
	//asITypeInfo *stinfo;
	int element_size = 0;

	element_size = script_GetTypeSize(type_info);
	tinfo = (asITypeInfo *)type_info;

	array = (struct script_array_t *)memory_Calloc(1, sizeof(struct script_array_t));

	array->type_info = type_info;
	array->element_size = element_size;
	array->element_count = 0;
	array->max_elements = 0;
	array->buffer = NULL;

    script_array_AddRef(array);

	//tinfo->AddRef();

	return array;
}

struct script_array_t *script_array_Constructor_Sized(void *type_info, int size)
{
	struct script_array_t *array;
	array = script_array_Constructor(type_info);

	if(size)
	{
		array->element_count = 0;
		array->max_elements = size;
		array->buffer = memory_Malloc(array->element_size * array->max_elements);
	}

	return array;
}

void script_array_Destructor(void *this_pointer)
{
	struct script_array_t *array;
	asITypeInfo *type_info;

	array = (struct script_array_t *)this_pointer;

	type_info = (asITypeInfo *)array->type_info;

	if(!array->extern_array)
	{
		if(type_info)
		{
			//type_info->Release();
		}

		if((!array->extern_buffer) && array->buffer)
		{
			memory_Free(array->buffer);
		}

		memory_Free(array);
	}
}

void script_array_AddRef(void *this_pointer)
{
	struct script_array_t *array;
	asITypeInfo *type_info;

	array = (struct script_array_t *)this_pointer;
	type_info = (asITypeInfo *)array->type_info;

	if(!array->extern_array)
	{
		array->ref_count++;

		assert(array->ref_count < 1000000);

		if(type_info)
		{
			//type_info->AddRef();
		}
	}
}

void script_array_Release(void *this_pointer)
{
	struct script_array_t *array;
	asITypeInfo *type_info;

	array = (struct script_array_t *)this_pointer;
	type_info = (asITypeInfo *)array->type_info;

	if(!array->extern_array)
	{
		assert(array->ref_count < 1000000);

		if(array->ref_count)
		{
			array->ref_count--;

			if(!array->ref_count)
			{
				script_array_Destructor(this_pointer);
			}
		}
	}
}

void *script_array_ElementAt(void *this_pointer, int index)
{
	struct script_array_t *array;
	array = (struct script_array_t *)this_pointer;

	return (char *)array->buffer + array->element_size * index;
}

void *script_array_OpAssign(void *this_pointer, void *other)
{
    struct script_array_t *this_array;
    struct script_array_t *other_array;




    other_array = (struct script_array_t *)other;
    this_array = (struct script_array_t *)this_pointer;

    this_array->buffer = other_array->buffer;
    this_array->element_count = other_array->element_count;
    this_array->element_size = other_array->element_size;
    this_array->extern_buffer = other_array->extern_buffer;

	/* not sure why this is needed here, but avoids the
	script engine calling the array destructor after it
	was destroyed... */
    //script_array_AddRef(this_array);

	return this_pointer;
}

void script_array_Clear(void *this_pointer)
{
	struct script_array_t *array;

	array = (struct script_array_t *)this_pointer;
	array->element_count = 0;
}

void script_array_Append(void *this_pointer, void *element)
{
	struct script_array_t *array;

	void *data;

	array = (struct script_array_t *)this_pointer;

	if(array->element_count >= array->max_elements)
	{
        data = memory_Calloc(array->max_elements + 64, array->element_size);
		memcpy(data, array->buffer, array->max_elements * array->element_size);
		memory_Free(array->buffer);

		array->buffer = data;
		array->max_elements += 64;
	}

	memcpy((char *)array->buffer + array->element_size * array->element_count, element, array->element_size);

	array->element_count++;
}

int script_array_Count(void *this_pointer)
{
	struct script_array_t *array;
	array = (struct script_array_t *)this_pointer;

	return array->element_count;
}




