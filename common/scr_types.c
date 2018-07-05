#include "scr_types.h"
#include "angelscript.h"
#include "memory.h"

#include <stdio.h>

extern asIScriptEngine *scr_virtual_machine;


script_generic_array_t *script_generic_array_Constructor(void *type_info)
{
	script_generic_array_t *array;
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
	
	array = (script_generic_array_t *)memory_Malloc(sizeof(script_generic_array_t), "script_generic_array_Constructor");
	
	array->type_info = type_info;
	array->element_size = element_size;
	array->element_count = 0;
	array->buffer = NULL;
	
	tinfo->AddRef();
	
	return array;
}

script_generic_array_t *script_generic_array_Constructor_Sized(void *type_info, int size)
{
	script_generic_array_t *array;
	array = script_generic_array_Constructor(type_info);
	
	array->element_count = size;
	array->buffer = memory_Malloc(array->element_size * array->element_count, "script_generic_array_Constructor_Sized");
	
	return array;
}

void script_generic_array_Destructor(void *this_pointer)
{
	script_generic_array_t *array;
	
	array = (script_generic_array_t *)this_pointer;
	
	if(array->buffer)
	{
		memory_Free(array->buffer);
	}
	
	memory_Free(array);
}

void script_generic_array_AddRef(void *this_pointer)
{
	script_generic_array_t *array;
	asITypeInfo *type_info;
	
	array = (script_generic_array_t *)this_pointer;
	type_info = (asITypeInfo *)array->type_info;
	
	type_info->AddRef();
}

void script_generic_array_Release(void *this_pointer)
{
	script_generic_array_t *array;
	asITypeInfo *type_info;
	
	array = (script_generic_array_t *)this_pointer;
	type_info = (asITypeInfo *)array->type_info;
	
	if(!type_info->Release())
	{
		script_generic_array_Destructor(this_pointer);
	}
}

void *script_generic_array_ElementAt(void *this_pointer, int index)
{
	script_generic_array_t *array;
	array = (script_generic_array_t *)this_pointer;
	
	return (char *)array->buffer + array->element_size * index;
}

int script_generic_array_Count(void *this_pointer)
{
	script_generic_array_t *array;
	array = (script_generic_array_t *)this_pointer;
	
	return array->element_count;
}




/*
====================================================================
====================================================================
====================================================================
*/

void script_vec2_t_Constructor()
{
	/* vec2_t is a value type, hence it gets
	allocated in the stack... */
}

void script_vec2_t_Destructor(vec2_t *this_pointer)
{
	/* vec2_t is a value type, hence it gets
	allocated in the stack... */
}

void script_vec2_t_OpAssign(vec2_t *other, vec2_t *this_pointer)
{
	*this_pointer = *other;
}

/*
====================================================================
====================================================================
====================================================================
*/

script_generic_reference_t *script_generic_reference_Constructor()
{
	script_generic_reference_t *ref;
	ref = (script_generic_reference_t *)memory_Malloc(sizeof(script_generic_reference_t), "script_reference_Constructor");
	ref->object = NULL;
	
	ref->ref_count = 0;
	return ref;
}

void script_generic_reference_Destructor(script_generic_reference_t *this_pointer)
{
	//script_generic_reference_t *ref;
	//ref = (script_generic_reference_t *)this_pointer;
	memory_Free(this_pointer);
}

void script_generic_reference_AddRef(script_generic_reference_t *this_pointer)
{
	//this_pointer->ref_count++;
}

void script_generic_reference_Release(script_generic_reference_t *this_pointer)
{
	/*this_pointer->ref_count--;
	if(!this_pointer->ref_count)
	{
		
	}*/
}

void script_generic_reference_OpHndlAssign(script_generic_reference_t *other, script_generic_reference_t *this_pointer)
{
	
}

char script_generic_reference_OpEquals(script_generic_reference_t *object)
{

}

void script_generic_reference_OpCast(script_generic_reference_t *object)
{
	
}







void *script_DummyDefaultConstructor()
{
	return NULL;
}

void *script_DummyConstructor(void *type_info, void *this_pointer)
{
	return NULL;
}

void script_DummyDestructor(void *this_pointer)
{
	
}



 




test_type_t * script_test_type_Constructor(void *type_info)
{
//	test_type_t *t;
//	t = (test_type_t *)memory_Malloc(sizeof(test_type_t), "script_test_type_Constructor");
//	t->type_info = scr_virtual_machine->GetTypeInfoByName("test_type_t");
	
//	script_test_type_AddRef(t);
	//t->ref_count = 1;
	//t->source = "constructor";
	
	//printf("%d\n", ti->GetSize());
	
	
	//script_test_type_AddRef(t, );
	
//	return t;
	return NULL;
} 

void script_test_type_Destructor(test_type_t *this_pointer)
{
	//printf("%d\n", this_pointer->ref_count);
//	memory_Free(this_pointer);
}

void script_test_type_AddRef(test_type_t *this_pointer)
{
	
//	asITypeInfo *ti;
//	ti = (asITypeInfo *)this_pointer->type_info;
	
//	ti->AddRef();
//	printf("%s\n", ti->GetName());
	//if(this_pointer->ref_count >= 0)
	//{
	//	this_pointer->ref_count++;
	//}
	
}

void script_test_type_Release(test_type_t *this_pointer)
{
	//if(this_pointer->ref_count > 0)
//	{
//	asITypeInfo *ti;
//	ti = (asITypeInfo *)this_pointer->type_info;
	
//	ti->Release();
//		printf("destructor: %d\n", this_pointer->ref_count);
//		this_pointer->ref_count--;
//		if(this_pointer->ref_count == 0)
//		{
//			//script_test_type_Destructor(this_pointer);
//		}
//	}
}

void script_test_type_OpAssign(test_type_t *other, test_type_t *this_pointer)
{
	//this_pointer->
}

void script_test_type_OpHndlAssign(test_type_t *other, test_type_t *this_pointer)
{
	
}


/* 
===============================================
===============================================
===============================================
*/


void *script_particle_system_Factory()
{
	return NULL;
}






