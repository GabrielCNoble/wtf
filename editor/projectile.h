#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "vector_types.h"

#define MIN_PROJECTILE_RADIUS 0.1
#define MAX_PROJECTILE_RADIUS 5.0

#define MIN_PROJECTILE_LIFE 1
#define MAX_PROJECTILE_LIFE 32000


typedef struct
{
	vec3_t delta;
	vec3_t position;
	float radius;
	int index;
	int player_index;
	short life;
	
}projectile_t;




void projectile_Init();

void projectile_Finish();

int projectile_AddProjectile(vec3_t position, vec3_t delta, float radius, int life, int player_index);

void projectile_RemoveProjectile(int projectile_index);

void projectile_UpdateProjectiles();



#endif
