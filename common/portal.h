#ifndef PORTAL_H
#define PORTAL_H

#include "vector.h"
#include "matrix.h"

typedef struct
{
	mat3_t orientation;
	vec3_t position;
	vec2_t extents;
	struct portal_t *linked;
}portal_t;



#endif



