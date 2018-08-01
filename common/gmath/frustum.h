#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <math.h>
#include "frustum_types.h"
#include "vector_types.h"

int PointInsideFrustum(frustum_t *frustum, vec3_t point);

int SphereInsideFrustum(frustum_t *frustum, vec3_t center, float radius);

//float BoxInsideFrustum(frustum_t *frustum, vec4_t *box);


#endif /* FRUSTUM_H */
