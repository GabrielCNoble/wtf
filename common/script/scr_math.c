#include "scr_math.h"
#include "vector.h"
#include "matrix.h"
#include <stdlib.h>




#ifdef __cplusplus
extern "C" 
{
#endif


static vec3_t vec3_ret;


float randfloat()
{
	short x;
	
	x = rand() % RAND_MAX;
	
	return (float)x / (float)RAND_MAX;
} 

/*
================================================================
================================================================
================================================================
*/

float vec_get_op_index(void *this_pointer, int index)
{
	vec4_t *v = (vec4_t *)this_pointer;
	return v->floats[index];
}

void vec_set_op_index(void *this_pointer, int index, float value)
{
	vec4_t *v = (vec4_t *)this_pointer;
	v->floats[index] = value;
}



/*
================================================================
================================================================
================================================================
*/

void vec2_constructor(void *this_pointer)
{
	
}

void vec2_constructor_init(void *this_pointer, float x, float y)
{
	vec2_t *v;
	v = (vec2_t *)this_pointer;
	v->x = x;
	v->y = y;
}

void vec2_destructor(void *this_pointer)
{
	
}

void *vec2_op_assign(void *this_pointer, void *other)
{
	*(vec2_t *)this_pointer = *(vec2_t *)other;
	return this_pointer;
}

/*
================================================================
================================================================
================================================================
*/

void vec3_constructor(void *this_pointer)
{
	
}

void vec3_constructor_init(void *this_pointer, float x, float y, float z)
{
	vec3_t *v = (vec3_t *) this_pointer;
	v->x = x;
	v->y = y;
	v->z = z;
}

void vec3_destructor(void *this_pointer)
{
	
}

void *vec3_op_assign(void *this_pointer, void *other)
{
	*(vec3_t *)this_pointer = *(vec3_t *)other;
	return this_pointer;
}

void *vec3_op_add(void *this_pointer, void *other)
{
	vec3_ret.x = ((vec3_t *)this_pointer)->x + ((vec3_t *)other)->x;
	vec3_ret.y = ((vec3_t *)this_pointer)->y + ((vec3_t *)other)->y;
	vec3_ret.z = ((vec3_t *)this_pointer)->z + ((vec3_t *)other)->z;
	return &vec3_ret;
}

void *vec3_op_sub(void *this_pointer, void *other)
{
	vec3_ret.x = ((vec3_t *)this_pointer)->x - ((vec3_t *)other)->x;
	vec3_ret.y = ((vec3_t *)this_pointer)->y - ((vec3_t *)other)->y;
	vec3_ret.z = ((vec3_t *)this_pointer)->z - ((vec3_t *)other)->z;
	return &vec3_ret;
}

void *vec3_op_mul(void *this_pointer, float factor)
{
	vec3_ret.x = ((vec3_t *)this_pointer)->x * factor;
	vec3_ret.y = ((vec3_t *)this_pointer)->y * factor;
	vec3_ret.z = ((vec3_t *)this_pointer)->z * factor;
	return &vec3_ret;
}

float vec3_dot(void *vec_a, void *vec_b)
{
	return ((vec3_t *)vec_a)->x * ((vec3_t *)vec_b)->x + ((vec3_t *)vec_a)->y * ((vec3_t *)vec_b)->y + ((vec3_t *)vec_a)->z * ((vec3_t *)vec_b)->z;
}

/*
================================================================
================================================================
================================================================
*/

void vec4_constructor(void *this_pointer)
{
	
}

void vec4_constructor_init(void *this_pointer, float x, float y, float z, float w)
{
	vec4_t *v = (vec4_t *)this_pointer;
	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;
}

void vec4_destructor(void *this_pointer)
{
	
}

void *vec4_op_assign(void *this_pointer, void *other)
{
	*(vec4_t *)this_pointer = *(vec4_t *)other;
	return this_pointer;
}

/*
================================================================
================================================================
================================================================
*/

void mat3_constructor(void *this_pointer)
{
	
}

void mat3_constructor_init(void *this_pointer, void *r0, void *r1, void *r2)
{
	((mat3_t *)this_pointer)->r0 = *(vec3_t *)r0;
	((mat3_t *)this_pointer)->r1 = *(vec3_t *)r1;
	((mat3_t *)this_pointer)->r2 = *(vec3_t *)r2;
}

void mat3_destructor(void *this_pointer)
{
	
}

void *mat3_op_assign(void *this_pointer, void *other)
{
	*(mat3_t *)this_pointer = *(mat3_t *)other;
	return this_pointer;
}

void *mat3_get_op_index(void *this_pointer, int index)
{
	mat3_t *mat;
	mat = (mat3_t *)this_pointer;
	return &mat->floats[index][0];
}

void mat3_set_op_index(void *this_pointer, int index, void *row)
{
	mat3_t *mat;
	vec3_t *v;
	
	mat = (mat3_t *)this_pointer;
	v = (vec3_t *)row;
	
	mat->floats[index][0] = v->x;
	mat->floats[index][1] = v->y;
	mat->floats[index][2] = v->z;
}

void *mat3_op_mul(void *this_pointer, void *vec)
{
	vec3_ret = *(vec3_t *)vec;
	mat3_t_vec3_t_mult(this_pointer, &vec3_ret);
	return &vec3_ret;
}

void mat3_identity(void *this_pointer)
{
	((mat3_t *)this_pointer)->floats[0][0] = 1.0;
	((mat3_t *)this_pointer)->floats[0][1] = 0.0;
	((mat3_t *)this_pointer)->floats[0][2] = 0.0;
	
	((mat3_t *)this_pointer)->floats[1][0] = 0.0;
	((mat3_t *)this_pointer)->floats[1][1] = 1.0;
	((mat3_t *)this_pointer)->floats[1][2] = 0.0;
	
	((mat3_t *)this_pointer)->floats[2][0] = 0.0;
	((mat3_t *)this_pointer)->floats[2][1] = 0.0;
	((mat3_t *)this_pointer)->floats[2][2] = 1.0;
}



/*
================================================================
================================================================
================================================================
*/

#ifdef __cplusplus
}
#endif







