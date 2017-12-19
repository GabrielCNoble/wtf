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

void physics_CreateBlock(vec3_t position, vec3_t half_extents);

void physics_ProcessCollisions(float delta_time);

void physics_UpdateWorldMesh(vertex_t *vertices, int vertex_count);

void physics_BuildWorldMeshBVH();

void physics_DeleteWorldMeshBVH();

void physics_WalkBVH(bvh_node_t *root, vec3_t *capsule_a, vec3_t *capsule_b, vec3_t *capsule_direction, vec3_t *capsule_normalized_direction, player_t *player);

void physics_RayCastBVH(bvh_node_t *root, vec3_t *origin, vec3_t *normalized_direction, float max_distance, ray_cast_result_t *result);

void physics_RayCast(vec3_t *origin, vec3_t *normalized_direction, float max_distance, ray_cast_result_t *result);

int physics_CheckProjectileCollisionBVH(bvh_node_t *root, projectile_t *projectile, vec3_t *position, vec3_t *normal, int *collided);

int physics_CheckProjectileCollisionPlayers(projectile_t *projectile, player_t **hit);

int physics_CheckProjectileCollision(projectile_t *projectile, vec3_t *position, vec3_t *normal, player_t **hit);

float physics_ClipEdge(vec3_t plane_point, vec3_t plane_normal, vec3_t a, vec3_t b);

float physics_IntersectPlane(vec3_t plane_point, vec3_t plane_normal, vec3_t ray_origin, vec3_t ray_direction);

vec3_t physics_ProjectPointOnPlane(vec3_t a, vec3_t plane_point, vec3_t plane_normal);



#endif








