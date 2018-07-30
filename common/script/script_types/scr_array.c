#include "scr_array.h"
#include "memory.h"
#include "script.h"

#include "angelscript.h"


struct script_array_t *script_array_Constructor(void *type_info)
{
	struct script_array_t *array;
	asITypeInfo *tinfo;
	//asITypeInfo *stinfo;
	int element_size = 0;
	
	element_size = script_GetTypeSize(type_info);
	tinfo = (asITypeInfo *)type_info; 
	
	array = (struct script_array_t *)memory_Malloc(sizeof(struct script_array_t), "script_generic_array_Constructor");
	
	array->type_info = type_info;
	array->element_size = element_size;
	array->element_count = 0;
	array->buffer = NULL;
	
	tinfo->AddRef();
	
	return array;
}

struct script_array_t *script_array_Constructor_Sized(void *type_info, int size)
{
	struct script_array_t *array;
	array = script_array_Constructor(type_info);
	
	if(size)
	{
		array->element_count = size;
		array->buffer = memory_Malloc(array->element_size * array->element_count, "script_generic_array_Constructor_Sized");
	}

	return array;
}

void script_array_Destructor(void *this_pointer)
{
	struct script_array_t *array;
	
	array = (struct script_array_t *)this_pointer;
	
	if(array->buffer)
	{
		//memory_Free(array->buffer);
	}
	
	//memory_Free(array);
}

void script_array_AddRef(void *this_pointer)
{
	struct script_array_t *array;
	asITypeInfo *type_info;
	
	array = (struct script_array_t *)this_pointer;
	type_info = (asITypeInfo *)array->type_info;
	
	if(type_info)
	{
		type_info->AddRef();
	}
}

void script_array_Release(void *this_pointer)
{
	struct script_array_t *array;
	asITypeInfo *type_info;
	
	array = (struct script_array_t *)this_pointer;
	type_info = (asITypeInfo *)array->type_info;
	
	if(type_info)
	{
		if(!type_info->Release())
		{
			script_array_Destructor(this_pointer);
		}
	}
}

void *script_array_ElementAt(void *this_pointer, int index)
{
	struct script_array_t *array;
	array = (struct script_array_t *)this_pointer;
	
	return (char *)array->buffer + array->element_size * index;
}

int script_array_Count(void *this_pointer)
{
	struct script_array_t *array;
	array = (struct script_array_t *)this_pointer;
	
	return array->element_count;
}




