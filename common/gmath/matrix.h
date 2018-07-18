#ifndef _MATRIX_H_
#define _MATRIX_H_


#include <math.h>

#include "frustum_types.h"
#include "matrix_types.h"
#include "vector.h"
#include <stdlib.h>


#ifdef __cplusplus
extern "C"
{
#endif

#define mat3_t_c0(r0, r1, r2) (mat3_t){r0.x,r0.y,r0.z,r1.x,r1.y,r1.z,r2.x,r2.y,r2.z};

//#define mat3_t_c0(r0, r1, r2) (mat3_t){.r0=r0,.r1=r1,.r2=2}

void CreatePerspectiveMatrix(mat4_t *mat,  float fovY, float aspect, float znear, float zfar, float x_shift, float y_shift, frustum_t *generated_frustum);

void Frustum(mat4_t *mat, float left, float right, float top, float bottom, float znear, float zfar, frustum_t *generated_frustum);

void CreateOrthographicMatrix(mat4_t *mat, float left, float right, float top, float bottom, float znear, float zfar, frustum_t *generated_frustum);

void mat4_t_rotate(mat4_t *mat, vec3_t axis, float angle, int b_set);

void mat3_t_rotate(mat3_t *mat, vec3_t axis, float angle, int b_set);

void mat2_t_rotate(mat2_t *mat, float angle, int b_set);

void mat4_t_scale(mat4_t *mat, vec3_t axis, float scale_factor);

void mat4_t_scale_axis_aligned(mat4_t *mat, vec3_t scale);

void mat4_t_translate(mat4_t *mat, vec3_t position, int b_set);

mat4_t mat4_t_id();

mat3_t mat3_t_id();

mat2_t mat2_t_id();

void mat4_t_mult(mat4_t *result, mat4_t *mat1, mat4_t *mat2);

__fastcall void mat4_t_mult_fast(mat4_t *result, mat4_t *mat1, mat4_t *mat2);

//PEWAPI static inline void amat4_t_mult_fast(amat4_t *result, amat4_t *mat1, amat4_t *mat2);

void mat3_t_mult(mat3_t *result, mat3_t *mat1, mat3_t *mat2);

void mat2_t_mult(mat2_t *result, mat2_t *mat1, mat2_t *mat2);

void mat4_t_mat3_t(mat3_t *result, mat4_t *mat);

void mat2_t_mat3_t(mat2_t *in, mat3_t *out);

void mat3_t_mat2_t(mat3_t *in, mat2_t *out);

void mat4_t_mat2_t(mat4_t *in, mat2_t *out);

void mat4_t_transpose(mat4_t *mat);

void mat4_t_inverse_transform(mat4_t *mat);

void mat4_t_inverse(mat4_t *mat);

void mat3_t_transpose(mat3_t *mat);

void mat2_t_transpose(mat2_t *mat);

void mat2_t_invert(mat2_t *mat);

void mat4_t_compose(mat4_t *result, mat3_t *Orientation, vec3_t Position);

void mat3_t_compose(mat3_t *result, vec3_t vec);

static inline void MultiplyVector4(mat4_t *mat, vec4_t *vec);

__fastcall void mat4_t_vec4_t_mult(mat4_t *mat, vec4_t *vec);

__fastcall void mat3_t_vec3_t_mult(mat3_t *mat, vec3_t *vec);

vec3_t MultiplyVector3(mat3_t *mat, vec3_t vec);

//PEWAPI static inline void mat3_t_vec3_t_mult(mat3_t *mat, avec3_t *vec);

//PEWAPI mat3_t MatrixCopy3(mat3_t *out, mat3_t *in);

//PEWAPI void MatrixCopy4(mat4_t *out, mat4_t *in);

void quat_to_mat3_t(mat3_t *out, quaternion_t *q);


#include "matrix.inl"

#ifdef __cplusplus
}
#endif


#endif //_MATRIX_H_












