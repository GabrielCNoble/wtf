#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "player.h"
#include "camera.h"
#include "r_main.h"
#include "input.h"
#include "physics.h"
#include "sound.h"
#include "bsp.h"

int player_list_size;
int player_count;
player_t *players;
player_t *active_player;


mesh_t *weapon_mesh;
mesh_t *body_mesh;

extern float normalized_mouse_x;
extern float normalized_mouse_y;

int visible_player_count;
int *visible_players_indexes;
mat4_t *visible_players_body_transforms;
mat4_t *visible_players_weapon_transforms;
mat4_t *active_player_transform;


/* from r_main.c */
extern int r_width;
extern int r_height;

/* from world.c */
extern bsp_pnode_t *collision_nodes;

extern int fire_sound;

void player_Init()
{
	player_list_size = 64;
	player_count = 0;
	players = malloc(sizeof(player_t) * player_list_size);
	active_player = NULL;
	
	//mesh_LoadModel("weapon.obj", "weapon");
	//mesh_LoadModel("body.obj", "body");
	
	//weapon_mesh = mesh_GetModel("weapon");
	//body_mesh = mesh_GetModel("body");
	
	
	visible_players_body_transforms = malloc(sizeof(mat4_t ) * player_list_size);
	visible_players_weapon_transforms = malloc(sizeof(mat4_t) * player_list_size);
	visible_players_indexes = malloc(sizeof(int) * player_list_size);
}

void player_Finish()
{
	int i;
	for(i = 0; i < player_count; i++)
	{
		free(players[i].player_name);
	}
	free(players);
	free(visible_players_body_transforms);
	free(visible_players_weapon_transforms);
	free(visible_players_indexes);
}

void player_CreatePlayer(char *name, vec3_t position, mat3_t *orientation)
{
	player_t *player;
	int player_index = player_count++;
	int name_len;
	int camera_index;
	char camera_name[128];
	int w;
	int h;
	
	if(player_index >= player_list_size)
	{
		player = malloc(sizeof(player_t) * (player_list_size + 16));
		memcpy(player, players, sizeof(player_t) * player_list_size);
		free(players);
		players = player;
		player_list_size += 16;
	}
	
	
	player = &players[player_index];
	name_len = strlen(name) + 1;
	name_len = (name_len + 3) & (~3);
	player->player_name = calloc(name_len, 1);
	strcpy(player->player_name, name);
	
	strcpy(camera_name, name);
	strcat(camera_name, "_camera");
	
	renderer_GetWindowSize(&w, &h);
	
	
	player->player_position = position;
	player->collision_box_position = position;
	player->player_orientation = *orientation;
	
	/*printf("[%f %f %f]\n[%f %f %f]\n[%f %f %f]\n\n", player->player_orientation.floats[0][0], player->player_orientation.floats[0][1], player->player_orientation.floats[0][2],
													 player->player_orientation.floats[1][0], player->player_orientation.floats[1][1], player->player_orientation.floats[1][2],
													 player->player_orientation.floats[2][0], player->player_orientation.floats[2][1], player->player_orientation.floats[2][2]);*/
	
	//player->weapon_start = weapon_mesh->start;
	//player->weapon_count = weapon_mesh->vert_count;
	
	//player->body_start = body_mesh->start;
	//player->body_count = body_mesh->vert_count;
	
	player->weapon_x_shift = 0.0;
	player->weapon_y_shift = 0.0;
	player->weapon_z_shift = 0.0;
	player->pitch = 0.0;
	player->yaw = 0.0;
	player->max_slope = PLAYER_MAX_SLOPE_ANGLE;
	
	position.y = PLAYER_CAMERA_HEIGHT;
	
	camera_index = camera_CreateCamera(camera_name, position, orientation, 0.68, r_width, r_height, 0.1, 500.0, CAMERA_UPDATE_ON_RESIZE);
	
	player->player_camera = camera_GetCameraByIndex(camera_index);
	
	player->player_type = PLAYER_NPC;
}

player_t *player_GetPlayer(char *name)
{
	int i;
	
	for(i = 0; i < player_count; i++)
	{
		if(!strcmp(players[i].player_name, name))
		{
			return &players[i];
		}
	}
	
	return NULL;
}

player_t *player_GetActivePlayer()
{
	return active_player;
}

void player_SetPlayerAsActive(player_t *player)
{
	active_player = player;
	camera_SetCamera(player->player_camera);
}

void player_ProcessActivePlayer(float delta_time)
{
	static float pitch;
	static float yaw;
	
	static float x_shift = 0.0;
	static float y_shift = 0.0;
	
	vec3_t forward_vector;
	vec3_t right_vector;
	
	float dx;
	float dy;
	
	static int fire_timer = 0;

	if(active_player)
	{
		//printf("active player\n");
		active_player->bm_movement &= ~(MOVE_FORWARD|MOVE_BACKWARD|MOVE_STRAFE_LEFT|MOVE_STRAFE_RIGHT|MOVE_JUMP);
		
		x_shift *= 0.925;
		y_shift *= 0.925;
		
		dx = normalized_mouse_x * 0.25;
		dy = normalized_mouse_y * 0.25;
		 
		active_player->pitch += dy;		
		if(active_player->pitch > 0.5) active_player->pitch = 0.5;
		else if(active_player->pitch < -0.5) active_player->pitch = -0.5;

		active_player->yaw -= dx;
		
		if(active_player->yaw > 1.0) active_player->yaw = -1.0 + (active_player->yaw - 1.0);
		else if(active_player->yaw < -1.0) active_player->yaw = 1.0 + (active_player->yaw + 1.0);
		
		x_shift -= dx * delta_time * 0.05;
		
		if(x_shift > 1.0) x_shift = -1.0 + (x_shift - 1.0);
		else if(x_shift < -1.0) x_shift = 1.0 + (x_shift + 1.0);
		
		
		//forward_vector = active_player->player_orientation.f_axis;
		
		y_shift += dy * delta_time * 0.05;
		
		if(y_shift < -0.5) y_shift = -0.5;
		else if(y_shift > 0.5) y_shift = 0.5;
		
		active_player->weapon_x_shift = sin(x_shift * 3.14159265) * 0.5;
		active_player->weapon_y_shift = sin(y_shift * 3.14159265) * 0.5;
		active_player->weapon_z_shift = (-cos(x_shift * 3.14159265) - cos(y_shift * 3.14159265)) * 0.5;
				
		if(input_GetKeyPressed(SDL_SCANCODE_W))
		{
			active_player->bm_movement |= MOVE_FORWARD;
		}
		
		if(input_GetKeyPressed(SDL_SCANCODE_S))
		{
			active_player->bm_movement |= MOVE_BACKWARD;
		}
		
		
		if(input_GetKeyPressed(SDL_SCANCODE_A))
		{
			active_player->bm_movement |= MOVE_STRAFE_LEFT;
		}
		
		if(input_GetKeyPressed(SDL_SCANCODE_D))
		{
			active_player->bm_movement |= MOVE_STRAFE_RIGHT;
		}
		
		if(input_GetKeyPressed(SDL_SCANCODE_SPACE) && (active_player->bm_movement & PLAYER_ON_GROUND))
		{
			active_player->bm_movement |= MOVE_JUMP | PLAYER_JUMPED;
		}
		
		/*if(fire_timer)
		{
			active_player->bm_movement &= ~PLAYER_FIRED;
			fire_timer--;	
		}*/
		
		if(input_GetMouseButton(SDL_BUTTON_LEFT) & MOUSE_LEFT_BUTTON_CLICKED)
		{
			
			//if(!fire_timer)
		//	{
		//		fire_timer = 5;
				active_player->bm_movement |= PLAYER_FIRED;
				//sound_PlaySound(fire_sound, active_player->player_camera->world_position, 1.0);	
		//	}
			
			
		}
		else
		{
			active_player->bm_movement &= ~PLAYER_FIRED;
		}
		
		
		
	}
}

void player_ProcessAI(float delta_time)
{
	int i;
	int c = player_count;
	
	vec3_t player_vec;
	
	float yaw;
	float pitch;
	
	if(active_player)
	{
		for(i = 0; i < c; i++)
		{
			if(&players[i] == active_player) continue;
			
			players[i].bm_movement = PLAYER_FIRED;
			
			player_vec = sub3(active_player->player_camera->world_position, players[i].player_camera->world_position);
			player_vec = normalize3(player_vec);
			yaw = dot3(player_vec, players[i].player_camera->world_orientation.r_axis);
			pitch = dot3(player_vec, players[i].player_camera->world_orientation.u_axis);
			
			players[i].yaw += -yaw * 0.05;
			players[i].pitch += pitch * 0.05;
			
			players[i].weapon_x_shift = 0.0;
			players[i].weapon_y_shift = 0.0;
			players[i].weapon_z_shift = 0.0;
			
		}
	}
	
	
}

void player_UpdatePlayers(double delta_time)
{
	int i;
	int j;
	float r;
	vec3_t forward_vector;
	vec3_t right_vector;
	
	float s;
	float c;
	int transform_index;
	
	float increment;
	
	camera_t *active_camera = camera_GetActiveCamera();
	
	mat4_t transform;
	mat4_t weapon_transform;
	mat3_t weapon_rot = mat3_t_id();
	mat3_t rot;
	vec3_t weapon_position = vec3(1.0, -1.0, -2.0);
	vec3_t direction;
	vec3_t r_vector;
	vec3_t u_vector;
	vec3_t r_direction;
	vec3_t position;
	
	//printf("%f\n", delta_time);
	
	visible_player_count = 0;
	
	for(i = 0; i < player_count; i++)
	{	
		s = sin(players[i].yaw * 3.14159265);
		c = cos(players[i].yaw * 3.14159265);
		
		players[i].player_orientation.floats[0][0] = c;
		players[i].player_orientation.floats[0][2] = -s;
		
		
		players[i].player_orientation.floats[2][0] = s;
		players[i].player_orientation.floats[2][2] = c;
		
		vec3_t forward_vector = players[i].player_orientation.f_axis;
		vec3_t right_vector = players[i].player_orientation.r_axis;
		
		if(!(players[i].bm_movement & PLAYER_ON_GROUND))
		{
			increment = AIR_DELTA_INCREMENT;
		}
		else
		{
			increment = GROUND_DELTA_INCREMENT;
		}
		
		
		if(players[i].bm_movement & MOVE_FORWARD)
		{
			players[i].delta.x -= forward_vector.x * increment * delta_time * 0.0025;
			players[i].delta.z -= forward_vector.z * increment * delta_time * 0.0025;
		}
		
		if(players[i].bm_movement & MOVE_BACKWARD)
		{
			players[i].delta.x += forward_vector.x * increment * delta_time * 0.0025;
			players[i].delta.z += forward_vector.z * increment * delta_time * 0.0025;
		}
		
		 
		if(players[i].bm_movement & MOVE_STRAFE_LEFT)
		{
			players[i].delta.x -= right_vector.x * increment * delta_time * 0.0025;
			players[i].delta.z -= right_vector.z * increment * delta_time * 0.0025;
		}
		
		if(players[i].bm_movement & MOVE_STRAFE_RIGHT)
		{
			players[i].delta.x += right_vector.x * increment * delta_time * 0.0025;
			players[i].delta.z += right_vector.z * increment * delta_time * 0.0025;
		}
		
		if(players[i].bm_movement & MOVE_JUMP)
		{
			players[i].delta.y = JUMP_DELTA * 0.15;
		}
		
		player_Move(&players[i]);
		
		players[i].player_position.x = players[i].collision_box_position.x;
		players[i].player_position.z = players[i].collision_box_position.z;
		
		if(players[i].bm_movement & PLAYER_STEPPING_UP)
		{ 
			
			s = fabs(players[i].player_position.y);
			c = fabs(players[i].collision_box_position.y);
					
			players[i].player_position.y += (players[i].collision_box_position.y - players[i].player_position.y) * 0.075;

					
			if(fabs(s - c) <= 0.01)
				players[i].bm_movement &= ~PLAYER_STEPPING_UP;
			
		}
		else
		{
			players[i].player_position.y += (players[i].collision_box_position.y - players[i].player_position.y) * 0.25;
		}
		
		/*if(players[i].fire_timer)
		{
			players[i].fire_timer--;
		}
		
		
		if(players[i].bm_movement & PLAYER_FIRED && players[i].fire_timer == 0)
		{
			
			
			
			players[i].fire_timer = 5;
			
			direction = players[i].player_camera->world_orientation.f_axis;
			r_vector = players[i].player_camera->world_orientation.r_axis;
			u_vector = players[i].player_camera->world_orientation.u_axis;
			
			direction.x = -direction.x;
			direction.y = -direction.y;
			direction.z = -direction.z;
			
			r_direction = direction;
			
			for(j = 0; j < 1; j++)
			{
				r_direction = direction;
				
				r = ((float)((rand()%720) - 360) / 360.0) * 0.02;
				
				r_direction.x += r_vector.x * r;
				r_direction.y += r_vector.y * r;
				r_direction.z += r_vector.z * r;
				
				
				r = ((float)((rand()%720) - 360) / 360.0) * 0.02;
				
				r_direction.x += u_vector.x * r;
				r_direction.y += u_vector.y * r;
				r_direction.z += u_vector.z * r;

				
				r_direction.x *= 10.0;
				r_direction.y *= 10.0;
				r_direction.z *= 10.0;
				
				//projectile_AddProjectile(players[i].player_camera->world_position, r_direction, 1.505, 600, i);
				
				
			}
			
			//sound_PlaySound(fire_sound, players[i].player_camera->world_position, 0.9);		
			/*r_direction.x *= 10.0;
			r_direction.y *= 10.0;
			r_direction.z *= 10.0;
					
			projectile_AddProjectile(players[i].player_camera->world_position, r_direction, 1.505, 600, i);
		}*/
			
	}
	
	/*for(i = 0; i < visible_player_count; i++)
	{
		mat4_t_mult_fast(&visible_players_body_transforms[i], &visible_players_body_transforms[i], &active_camera->world_to_camera_matrix);
	}*/
	
	/*for(i = 0; i < visible_player_count; i++)
	{
		mat4_t_mult_fast(&visible_players_weapon_transforms[i], &visible_players_weapon_transforms[i], &active_camera->world_to_camera_matrix);
	}*/
	
}

#define BUMP_COUNT 5
#define SPEED_THRESHOLD 0.00001
void player_Move(player_t *player)
{
	if(!player)
		return;
	
	int i;
	int c;
	
	trace_t trace;
	
	vec3_t end;
	vec3_t new_velocity = player->delta;
	
	//end.x = position->x + velocity.x;
	//end.y = position->y + velocity.y;
	//end.z = position->z + velocity.z;
	
	//static int b_break = 0;
	
	if(collision_nodes)
	{
	
		player->bm_movement &= ~PLAYER_ON_GROUND;	
	
		end.x = player->collision_box_position.x + player->delta.x;
		end.y = player->collision_box_position.y + player->delta.y;
		end.z = player->collision_box_position.z + player->delta.z;
		
		for(i = 0; i < BUMP_COUNT; i++)
		{
			
			/* still enough to ignore any movement... */
			if(fabs(new_velocity.x) < SPEED_THRESHOLD && 
			   fabs(new_velocity.y) < SPEED_THRESHOLD &&
			   fabs(new_velocity.z) < SPEED_THRESHOLD)
			{
				break;
			}
			
			end.x = player->collision_box_position.x + new_velocity.x;
			end.y = player->collision_box_position.y + new_velocity.y;
			end.z = player->collision_box_position.z + new_velocity.z;
			
			
			bsp_FirstHit(collision_nodes, player->collision_box_position, end, &trace);
			
			//printf("%f\n", trace.frac);
			
			/* covered whole distance, bail out... */
			if(trace.frac == 1.0)
			{
				//printf("ALL DISTANCE!\n");
				break;
			}
			else
			{
				
				
				
				/* hit a vertical-ish surface, test to see whether it's a step or a wall... */
				if(trace.normal.y < 0.2 && trace.normal.y > -0.2)
				{

					if(bsp_TryStepUp(&player->collision_box_position, &new_velocity, &trace))
					{
						
						/* if step-up was successful, do not clip the speed... */
						
						/* TODO: maybe it's a good idea to dampen the speed on 
						staircases a little to avoid the player skyrocketing when walking one
						up... */
						player->bm_movement |= PLAYER_STEPPING_UP;
						continue;
					}
				}
				
				//assert(bsp_SolidPoint(collision_nodes, trace.position) != BSP_SOLID_LEAF);
				
				//printf("%f\n", trace.frac);
				
				/*if(player->bm_movement & PLAYER_ON_GROUND)
				{
					if(bsp_TryStepDown(&player->player_position, &player->delta, &trace))
					{
						continue;
					}
				}*/
				

				player->bm_movement |= PLAYER_ON_GROUND;
				
				
				//printf("before: [%f %f %f]  ", new_velocity.x, new_velocity.y, new_velocity.z);
				
				/* horizontal-ish surface (floor or slope)... */
				bsp_ClipVelocityToPlane(trace.normal, new_velocity, &new_velocity, 1.0);
				
				//printf("after: [%f %f %f]\n", new_velocity.x, new_velocity.y, new_velocity.z);
				
				player->collision_box_position.x += new_velocity.x * trace.frac;
				player->collision_box_position.y += new_velocity.y * trace.frac;
				player->collision_box_position.z += new_velocity.z * trace.frac;
				
				
				
			}
			
		}
	}
	
	
	player->delta = new_velocity;
	
	player->collision_box_position.x += player->delta.x;
	player->collision_box_position.y += player->delta.y;
	player->collision_box_position.z += player->delta.z;
		
	//bsp_Move(&player->player_position, &player->delta);	
		
}



void player_TransformPlayers()
{
	int i;
	
	for(i = 0; i < player_count; i++)
	{
		
	}
}














