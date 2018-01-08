#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <intrin.h>

#include "GL\glew.h"

#include "physics.h"
#include "bsp.h"
#include "gpu.h"

#include "player.h"
#include "camera.h"




extern int player_count;
extern player_t *players;


extern bsp_pnode_t *collision_nodes;

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
 

void physics_ProcessCollisions(double delta_time)
{
	int i;
//	int c = player_count;
/*	int j; */
	
	float l;
	float d;
	
	camera_t *active_camera = camera_GetActiveCamera();
	
	
	/*vec3_t move_start;
	vec3_t move_end;

	
	unsigned long long start;
	unsigned long long end;*/
	
	
	/*if(!world_hull)
		return;*/
	
	//printf("%f\n", (float)delta_time);		
		
	if(!collision_nodes)
		return;	
		
	
	
	for(i = 0; i < player_count; i++)
	{
		
		players[i].delta.y -= GRAVITY * delta_time * 0.01;
		
		player_Move(&players[i]);
			 
		l = players[i].delta.x * players[i].delta.x + players[i].delta.z * players[i].delta.z;
		
		if(l > MAX_HORIZONTAL_DELTA * MAX_HORIZONTAL_DELTA)
		{
			l = sqrt(l);
			l = MAX_HORIZONTAL_DELTA / l;
			
			players[i].delta.x *= l;
			players[i].delta.z *= l;
		}  
		
		//printf("%f\n", players[i].delta.y);
		
		/*if(fabs(players[i].delta.y) > GRAVITY * 2.0)
		{
			players[i].bm_movement |= PLAYER_FLYING;
		} 
		else
		{
			players[i].bm_movement &= ~PLAYER_FLYING;
		}*/
		 
		//printf("%d\n", players[i].bm_movement & PLAYER_ON_GROUND);
		
		if(!(players[i].bm_movement & PLAYER_ON_GROUND))
		{
			players[i].delta.x *= 1.0 - (AIR_FRICTION * delta_time * 0.01);
			players[i].delta.z *= 1.0 - (AIR_FRICTION * delta_time * 0.01);
		}  
		else 
		{
			if(!(players[i].bm_movement & (MOVE_FORWARD | MOVE_BACKWARD | MOVE_STRAFE_LEFT | MOVE_STRAFE_RIGHT)))
			{
				players[i].delta.x *= 1.0 - (GROUND_FRICTION * delta_time * 0.01);
				players[i].delta.z *= 1.0 - (GROUND_FRICTION * delta_time * 0.01);
			}
		}
				  
		
		players[i].player_camera->world_position.x = players[i].player_position.x;
		players[i].player_camera->world_position.y = players[i].player_position.y + PLAYER_CAMERA_HEIGHT;
		players[i].player_camera->world_position.z = players[i].player_position.z;
		
		camera_PitchYawCamera(players[i].player_camera, players[i].yaw, players[i].pitch);
		
		if(players[i].player_camera == active_camera)
		{
			camera_ComputeWorldToCameraMatrix(players[i].player_camera);
		}		
				
	}  
	
	
	//end = _rdtsc();
	//printf("%llu\n", end - start);
}




