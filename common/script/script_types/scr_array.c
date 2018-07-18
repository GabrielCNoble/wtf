#include "scr_array.h"
#include "memory.h"

#include "angelscript.h"


struct script_array_t *script_array_Constructor(void *type_info)
{
	struct script_array_t *array;
	asITypeInfo *tinfo;
	asITypeInfo *stinfo;
	int element_size = 0;
	
	tinfo = (asITypeInfo *)type_info; 
	
	stinfo = tinfo->GetSubType();
	
	if(!stinfo)
	{
		/* NULL subtype typeinfo means the 
		subtime is a primitive type... */
		switch(tinfo->GetSubTypeId())
		{
			/* NOTE: this can cause problems. 8 bit
			bools in angel script might not be the case
			all the times... */
			case asTYPEID_BOOL:
				element_size = 1;
			break;
			
			case asTYPEID_INT8:
				element_size = 1;
			break;
			
			case asTYPEID_INT16:
				element_size = 2;
			break;
			
			case asTYPEID_INT32:
			case asTYPEID_FLOAT:	
				element_size = 4;
			break;
			
			case asTYPEID_INT64:
			case asTYPEID_DOUBLE:
				element_size = 8;
			break;
		}
		 
	}
	else
	{
		element_size = stinfo->GetSize();
	}
	
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
	
	array->element_count = size;
	array->buffer = memory_Malloc(array->element_size * array->element_count, "script_generic_array_Constructor_Sized");
	
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
	
	memory_Free(array);
}

void script_array_AddRef(void *this_pointer)
{
	struct script_array_t *array;
	asITypeInfo *type_info;
	
	array = (struct script_array_t *)this_pointer;
	type_info = (asITypeInfo *)array->type_info;
	
	type_info->AddRef();
}

void script_array_Release(void *this_pointer)
{
	struct script_array_t *array;
	asITypeInfo *type_info;
	
	array = (struct script_array_t *)this_pointer;
	type_info = (asITypeInfo *)array->type_info;
	
	if(!type_info->Release())
	{
		script_array_Destructor(this_pointer);
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




