#ifndef _VECTOR_H_
#define _VECTOR_H_


#include <math.h>

#include "matrix_types.h"
#include "vector_types.h"





#ifdef __cplusplus
extern "C"
{
#endif


#define vec3_t_c(x,y,z) ((vec3_t){x,y,z})


vec2_t vec2(float x, float y);

vec2_t add2(vec2_t vec1, vec2_t vec2);

vec2_t sub2(vec2_t vec1, vec2_t vec2);

vec2_t mul2(vec2_t vec1, float value);

vec2_t getmiddle2(vec2_t A, vec2_t B);

vec2_t Vec3ToVec2(vec3_t vec_3);

//static inline vec3_t vec3(float x, float y, float z);

static inline vec4_t vec4(float x, float y, float z, float w);

vec3_t GetVec3To(vec3_t From, vec3_t To);

vec3_t GetMiddlePoint(vec3_t A, vec3_t B);

vec3_t project3(vec3_t vec1, vec3_t vec2);

float get_angle(vec3_t vec0, vec3_t vec1, vec3_t plane_normal);

vec3_t project3_NORMALIZED(vec3_t vec1, vec3_t vec2);

static inline vec3_t mul3(vec3_t vec, float value);

static inline vec3_t add3(vec3_t vec1, vec3_t vec2);

__forceinline void add3_fast(vec3_t *dst, vec3_t *src);

static inline vec3_t sub3(vec3_t vec1, vec3_t vec2);

__forceinline void sub3_fast(vec3_t *dst, vec3_t *src);

static inline vec3_t vec4vec3(vec4_t vec);

static inline vec4_t vec3vec4(vec3_t vec);

static inline vec3_t normalize3(vec3_t vec);

static inline vec3_t lerp3(vec3_t a, vec3_t b, float t);

vec3_t normalize32(vec3_t vec);

vec2_t normalize2(vec2_t vec);

static inline vec3_t cross(vec3_t vec1, vec3_t vec2);

vec3_t invert3(vec3_t vec);

vec4_t invert4(vec4_t vec);

vec4_t mul4mat(mat4_t *mat, vec4_t vec);

static inline quaternion_t lerp4(quaternion_t *a, quaternion_t *b, float t);

static inline quaternion_t slerp(quaternion_t *a, quaternion_t *b, float t);

//PEWAPI static inline quaternion_t squad(quaternion_t *a, quaternion_t *b, quaternion_t *c, float t);

static inline quaternion_t qlog(quaternion_t *q);

static inline quaternion_t qexp(quaternion_t *q);

static inline float dot4(vec4_t *a, vec4_t *b);

static inline quaternion_t qinverse(quaternion_t *q);

static inline quaternion_t qmult(quaternion_t *a, quaternion_t *b);

static inline float dot3(vec3_t vec1, vec3_t vec2);

float angle3(vec3_t vec1, vec3_t vec2);

float angle3_NORMALIZED(vec3_t vec1, vec3_t vec2);

static inline float length3(vec3_t vec);

float length2(vec2_t vec);

float dot2(vec2_t vec1, vec2_t vec2);

void memcpy_vec2_t(void *dst, void *src, int count);

static inline vec3_t gs_orthg(vec3_t ref, vec3_t src);

#include "vector.inl"

#ifdef __cplusplus
}
#endif

 


#endif // _VECTOR_H_


