#ifndef SCR_TYPES_H
#define SCR_TYPES_H


#include "scr_common.h"
#include "particle.h"


typedef struct
{
	void *object;
	void *type_info;
	int ref_count;
}script_generic_reference_t;


typedef struct
{
	int field_a;
	int field_b;
	void *type_info;
	//char *source;
	//int ref_count;
}test_type_t;


struct script_string_t
{
	char *string;
};




/*
====================================================================
====================================================================
====================================================================
*/


script_generic_reference_t *script_generic_reference_Constructor();

void script_generic_reference_Destructor(script_generic_reference_t *this_pointer);

void script_generic_reference_AddRef(script_generic_reference_t *this_pointer);

void script_generic_reference_Release(script_generic_reference_t *this_pointer);

void script_generic_reference_OpHndlAssign(script_generic_reference_t *other, script_generic_reference_t *this_pointer);

char script_generic_reference_OpEquals(script_generic_reference_t *object);

void script_generic_reference_OpCast(script_generic_reference_t *object);


/*
====================================================================
====================================================================
====================================================================
*/

void script_vec2_t_Constructor();

void script_vec2_t_Destructor(vec2_t *this_pointer);

void script_vec2_t_OpAssign(vec2_t *other, vec2_t *this_pointer);

void script_vec2_t_OpHndlAssign(vec2_t *other, vec2_t *this_pointer);


/*
====================================================================
====================================================================
====================================================================
*/


void *script_DummyDefaultConstructor();

void *script_DummyConstructor(void *type_info, void *this_pointer);

void script_DummyDestructor(void *this_pointer);


/*
====================================================================
====================================================================
====================================================================
*/


test_type_t *script_test_type_Constructor(void *type_info);

void script_test_type_Destructor(test_type_t *this_pointer);

void script_test_type_AddRef(test_type_t *this_pointer);

void script_test_type_Release(test_type_t *this_pointer);

void script_test_type_OpAssign(test_type_t *other, test_type_t *this_pointer);

void script_test_type_OpHndlAssign(test_type_t *other, test_type_t *this_pointer);



/*
====================================================================
====================================================================
====================================================================
*/

void *script_particle_system_Factory();



void script_string_function(void *param);


#endif










