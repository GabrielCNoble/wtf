#ifndef INDIRECT_H
#define INDIRECT_H

#include "vector.h"

typedef struct
{
	vec3_t origin;
	short lod;
	short frame;
	float accum;
	unsigned int next_lod;
}light_volume_t;




enum VOLUME_LOD
{
	VOLUME_LOD3 = 0,			/* 4 x 4 x 4 */
	VOLUME_LOD2,				/* 2 x 2 x 2 */
	VOLUME_LOD1,				/* 1 x 1 x 1 */
	VOLUME_LOD0,				/* 0.5 x 0.5 x 0.5 */
};



void indirect_BuildVolumes();

void indirect_GetAffectedVolumes();

void indirect_Propagate();

void indirect_DrawVolumes();




#endif
