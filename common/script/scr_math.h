#ifndef SCR_MATH_H
#define SCR_MATH_H


#include "scr_common.h"
#include "vector_types.h"
#include "matrix_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

float randfloat();


/*
================================================================
================================================================
================================================================
*/

float vec_get_op_index(void *this_pointer, int index);

void vec_set_op_index(void *this_pointer, int index, float value);

vec3_t *vec3_normalize(vec3_t *vec);

/*
================================================================
================================================================
================================================================
*/

void vec2_constructor(void *this_pointer);

void vec2_constructor_init(void *this_pointer, float x, float y);

void vec2_destructor(void *this_pointer);

void *vec2_op_assign(void *this_pointer, void *other);

/*
================================================================
================================================================
================================================================
*/

void vec3_constructor(void *this_pointer);

void vec3_constructor_init(void *this_pointer, float x, float y, float z);

void vec3_destructor(void *this_pointer);

void *vec3_op_assign(void *this_pointer, void *other);

void *vec3_op_add(void *this_pointer, void *other);

void *vec3_op_sub(void *this_pointer, void *other);

void *vec3_op_mul(void *this_pointer, float factor);

float vec3_dot(void *vec_a, void *vec_b);

/*
================================================================
================================================================
================================================================
*/

void vec4_constructor(void *this_pointer);

void vec4_constructor_init(void *this_pointer, float x, float y, float z, float w);

void vec4_destructor(void *this_pointer);

void *vec4_op_assign(void *this_pointer, void *other);

/*
================================================================
================================================================
================================================================
*/

void mat3_constructor(void *this_pointer);

void mat3_constructor_init(void *this_pointer, void *r0, void *r1, void *r2);

void mat3_destructor(void *this_pointer);

void *mat3_op_assign(void *this_pointer, void *other);

void *mat3_get_op_index(void *this_pointer, int index);

void mat3_set_op_index(void *this_pointer, int index, void *row);

void *mat3_op_mul(void *this_pointer, void *vec);

void mat3_identity(void *this_pointer);



/*
================================================================
================================================================
================================================================
*/




#ifdef __cplusplus
}
#endif



#endif
