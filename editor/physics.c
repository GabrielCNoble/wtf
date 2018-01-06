#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <intrin.h>

#include "GL\glew.h"

#include "physics.h"
#include "bsp.h"
#include "gpu.h"

#include "player.h"




extern int player_count;
extern player_t *players;

#define EXTRA_MARGIN 0.01




static float cube_bmodel_verts[] = 
{
	-1.0, 1.0, 1.0,
	-1.0,-1.0, 1.0,
	 1.0,-1.0, 1.0,
	 1.0,-1.0, 1.0,
	 1.0, 1.0, 1.0,
	-1.0, 1.0, 1.0,
	
	 1.0, 1.0, 1.0,
	 1.0,-1.0, 1.0,
	 1.0,-1.0,-1.0,
	 1.0,-1.0,-1.0,
	 1.0, 1.0,-1.0,
	 1.0, 1.0, 1.0,
	 
	 1.0, 1.0,-1.0,
	 1.0,-1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0, 1.0,-1.0,
	 1.0, 1.0,-1.0,
	 
	-1.0, 1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0,-1.0, 1.0,
	-1.0,-1.0, 1.0,
	-1.0, 1.0, 1.0,
	-1.0, 1.0,-1.0,
	
	-1.0, 1.0,-1.0,
	-1.0, 1.0, 1.0,
	 1.0, 1.0, 1.0,
	 1.0, 1.0, 1.0,
	 1.0, 1.0,-1.0,
	-1.0, 1.0,-1.0,
	
	-1.0,-1.0, 1.0,
	-1.0,-1.0,-1.0,
	 1.0,-1.0,-1.0,
	 1.0,-1.0,-1.0,
	 1.0,-1.0, 1.0,
	-1.0,-1.0, 1.0,         
};


extern int world_hull_node_count;
extern bsp_pnode_t *world_hull;


void physics_Init()
{
	/*block_list_size = 64;
	block_count = 0;
	block_list = malloc(sizeof(block_t) * block_list_size);
	handles = malloc(sizeof(int) * block_list_size);
	
	world_mesh_bvh = NULL;
	
	
	max_static_world_mesh_triangles = 16000;
	static_world_mesh_triangle_count = 0;
	static_world_mesh = malloc(sizeof(collision_triangle_t) * max_static_world_mesh_triangles);*/
}

void physics_Finish()
{
	int i;
	
	/*physics_DeleteWorldMeshBVH();
	
	for(i = 0; i < block_count; i++)
	{
		free(block_list[i].verts);
	}
	
	free(block_list);
	free(handles);
	free(static_world_mesh);*/
	
	
}


void physics_ProcessCollisions(float delta_time)
{
	int i;
	int c = player_count;
	int j; 
	
	float l;
	float d;
	
	
	vec3_t move_start;
	vec3_t move_end;

	
	unsigned long long start;
	unsigned long long end;
	
	
	if(!world_hull)
		return;
	
	for(i = 0; i < c; i++)
	{
			 
		l = players[i].delta.x * players[i].delta.x + players[i].delta.z * players[i].delta.z;
		
		if(l > MAX_HORIZONTAL_DELTA * MAX_HORIZONTAL_DELTA)
		{
			l = sqrt(l);
			l = MAX_HORIZONTAL_DELTA / l;
			
			players[i].delta.x *= l;
			players[i].delta.z *= l;
		}
		
		if(fabs(players[i].delta.y) > GRAVITY * 2.0)
		{
			players[i].bm_movement |= PLAYER_FLYING;
		}
		
		if(players[i].bm_movement & PLAYER_FLYING)
		{
			players[i].delta.x *= AIR_FRICTION;
			players[i].delta.z *= AIR_FRICTION;
		}
		else
		{
			players[i].delta.x *= GROUND_FRICTION;
			players[i].delta.z *= GROUND_FRICTION;
		}
		
		//players[i].delta.y -= GRAVITY;
		//bsp_Move(&players[i].player_position, &players[i].delta);
		
				
	}
	
	
	//end = _rdtsc();
	//printf("%llu\n", end - start);
}




