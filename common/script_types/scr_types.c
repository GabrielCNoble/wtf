#include "scr_types.h"
#include "angelscript.h"
#include "c_memory.h"

#include <stdio.h>

extern asIScriptEngine *scr_virtual_machine;


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
	ref = (script_generic_reference_t *)memory_Malloc(sizeof(script_generic_reference_t));
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


void script_string_function(void *param)
{

}



