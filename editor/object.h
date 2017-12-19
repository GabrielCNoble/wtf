#ifndef OBJECT_H
#define OBJECT_H

#include "matrix_types.h"
#include "mesh.h"

typedef struct
{
	mat3_t orientation;
	vec3_t position;
	mesh_t *mesh;
	char *name;
}object_t;



#endif
