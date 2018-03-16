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


int physics_Init()
{
	/*block_list_size = 64;
	block_count = 0;
	block_list = malloc(sizeof(block_t) * block_list_size);
	handles = malloc(sizeof(int) * block_list_size);
	
	world_mesh_bvh = NULL;
	
	
	max_static_world_mesh_triangles = 16000;
	static_world_mesh_triangle_count = 0;
	static_world_mesh = malloc(sizeof(collision_triangle_t) * max_static_world_mesh_triangles);*/
	
	return 1;
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


#define STEP_DELTA 16.666 

void physics_ProcessCollisions(double delta_time)
{
	int i;
	int step;
//	int c = player_count;
/*	int j; */
	
	float l;
	float d;
	float s;
	float c;
	
	int steps;
	float remaining_delta = delta_time;
	
	camera_t *active_camera = camera_GetActiveCamera();
			
	if(!collision_nodes)
		return;	
		
	
	steps = (int)(delta_time / STEP_DELTA);
	
	
	//for(step = 0; step < steps; step++)
	while(remaining_delta > 0.0)
	{
		for(i = 0; i < player_count; i++)
		{
			l = players[i].delta.x * players[i].delta.x + players[i].delta.z * players[i].delta.z;
			
			if(l > MAX_HORIZONTAL_DELTA * MAX_HORIZONTAL_DELTA)
			{
				l = sqrt(l);
				l = MAX_HORIZONTAL_DELTA / l;
				
				players[i].delta.x *= l;
				players[i].delta.z *= l;
			} 
			
			players[i].delta.y -= GRAVITY * STEP_DELTA * 0.05;
						
			if(!(players[i].bm_movement & PLAYER_ON_GROUND))
			{
				players[i].delta.x *= 1.0 - (AIR_FRICTION * STEP_DELTA * 0.01);
				players[i].delta.z *= 1.0 - (AIR_FRICTION * STEP_DELTA * 0.01);
			}  
			else 
			{
				if(!(players[i].bm_movement & (MOVE_FORWARD | MOVE_BACKWARD | MOVE_STRAFE_LEFT | MOVE_STRAFE_RIGHT)))
				{
					players[i].delta.x /= 1.0 + (GROUND_FRICTION * STEP_DELTA * 0.01);
					players[i].delta.z /= 1.0 + (GROUND_FRICTION * STEP_DELTA * 0.01);
				}
			}
			
			player_Move(&players[i], STEP_DELTA);
			
			players[i].player_position.x = players[i].collision_box_position.x;
			players[i].player_position.z = players[i].collision_box_position.z;
			
			if(players[i].bm_movement & PLAYER_STEPPING_UP)
			{ 
				
				s = fabs(players[i].player_position.y);
				c = fabs(players[i].collision_box_position.y);
						
				players[i].player_position.y += (players[i].collision_box_position.y - players[i].player_position.y) * 0.1 * STEP_DELTA * 0.075;
				
				//printf("smooth...\n");
						
				if(fabs(s - c) <= 0.01)
					players[i].bm_movement &= ~PLAYER_STEPPING_UP;
				
			}
			else
			{
				players[i].player_position.y += (players[i].collision_box_position.y - players[i].player_position.y) * 0.25 * STEP_DELTA * 0.075;
				players[i].bm_movement &= ~PLAYER_STEPPING_UP;
			}
		}
		
		remaining_delta -= STEP_DELTA;
		
	}
		
	for(i = 0; i < player_count; i++)
	{
		
		//printf("%d\n", players[i].bm_movement & (PLAYER_ON_GROUND | PLAYER_STEPPING_UP));
		
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




