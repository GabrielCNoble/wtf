#ifndef PHYSICS_H
#define PHYSICS_H

#include "vector_types.h"
#include "mesh.h"
#include "player.h"
#include "projectile.h"

#define GRAVITY 0.0098

#define GROUND_FRICTION 0.85
#define AIR_FRICTION 0.99

#define GROUND_DELTA_INCREMENT 0.5
#define AIR_DELTA_INCREMENT 0.025

#define MAX_HORIZONTAL_DELTA 0.5

#define JUMP_DELTA 0.3 

#define PLAYER_CAPSULE_HEIGHT 4.0
#define PLAYER_CAPSULE_RADIUS 1.5

typedef struct
{
	vec3_t position;
	vec3_t half_extents;
	vertex_t *verts;
	int start;
}block_t;

typedef struct
{
	vec3_t a;
	vec3_t b;
	float radius;
}capsule_t;

typedef struct
{
	vec3_t a;
	vec3_t b;
	vec3_t c;
	unsigned int packed_normal;
	
	//vec3_t normal;
}collision_triangle_t;

typedef struct bvh_node_t
{
	vec3_t position;						/* if this node is not a leaf, it represents a volume within the hierarchy... */
	vec3_t half_extents;
	
	int start;								/* this is an offset within the array of vertices that compose the static world geometry... */
	
	struct bvh_node_t *left;
	struct bvh_node_t *right;
	
	union
	{
		struct bvh_node_t *parent;
		struct bvh_node_t *next;
	};
	
}bvh_node_t;

typedef struct
{
	vec3_t hit_point;
	vec3_t hit_normal;
}ray_cast_result_t;


void physics_Init();

void physics_Finish();

void physics_ProcessCollisions(float delta_time);





#endif








