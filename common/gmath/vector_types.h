#ifndef _VECTOR_TYPES_H_
#define _VECTOR_TYPES_H_


#include <stdint.h>
//#include "GL/glew.h"


typedef union
{
	struct
	{
		float x;
		float y;
	};
	
	struct
	{
		float floats[2];	
	};
	
	struct
	{
		float a0;
		float a1;
	};
}vec2_t;

typedef __attribute((aligned(16)))vec2_t avec2_t;

typedef union 
{
	struct
	{
		float x;
		float y;
		float z;
	};
	
	struct
	{
		float floats[3];	
	};
	
	struct
	{
		float r;
		float g;
		float b;
	};
	
	struct
	{
		float a0;
		float a1;
		float a2;
	};
	
	struct
	{
		vec2_t vec2;
		float e;
	};
	
}vec3_t;


typedef __attribute((aligned(16)))vec3_t avec3_t;

typedef struct
{
	struct
	{
		long double x;
		long double y;
		long double z;
	};
	
	struct
	{
		long double floats[3];	
	};
	
	struct
	{
		long double r;
		long double g;
		long double b;
	};
	
	struct
	{
		long double a0;
		long double a1;
		long double a2;
	};
}ldvec3_t;


typedef union
{
	struct
	{
		int x, y, z;
	};
	
	struct
	{
		int ints[3];
	};
	
}ivec3_t;

typedef union
{
	struct
	{
		uint64_t x, y, z; 
	};
	
	struct 
	{
		uint64_t ints[3];
	};
}luivec3_t;


typedef union 
{
	struct 
	{
		float x;
		float y;
		float z;
		float w;
	};
	
	struct
	{
		float floats[4];	
	};
	
	struct 
	{
		float r;
		float g;
		float b;
		float a;
	};
	
	struct
	{
		float a0;
		float a1;
		float a2;
		float a3;
	};
	struct
	{
		vec3_t vec3;
		float pad;
	};
	
}vec4_t;

typedef __attribute((aligned(16)))vec4_t avec4_t;


typedef union quaternion_t 
{
	struct
	{
		float x, y, z, w;
	};
	struct
	{
		vec3_t v;
		float s;
	};
	struct 
	{
		float floats[4];
	};
}quaternion_t;

//typedef vec4_t quaternion_t;




#endif //_VECTOR_TYPES_H_
