#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "player.h"
#include "camera.h"
#include "r_main.h"
#include "input.h"
#include "physics.h"
#include "phy_character.h"
#include "sound.h"
#include "bsp.h"
#include "entity.h"
#include "memory.h"
#include "script.h"

//int player_list_size;
//int player_count;
//static int free_positions_stack_top = -1;
//static int *free_positions_stack = NULL;
//player_t *players = NULL;
//player_t *active_player = NULL;

struct entity_handle_t pl_active_player = {0, INVALID_ENTITY_INDEX};


//player_def_t *pl_player_defs = NULL;
//player_def_t *pl_last_player_def = NULL;


//int max_spawn_points = 32;
//int spawn_point_count = 0;
//int spawn_point_free_position_stack_top = -1;
//int *spawn_point_free_position_stack = NULL;
//spawn_point_t *spawn_points = NULL;

//mesh_t *weapon_mesh;
//mesh_t *body_mesh;

extern float normalized_mouse_x;
extern float normalized_mouse_y;

//int visible_player_count;
//int *visible_players_indexes;
//mat4_t *visible_players_body_transforms;
//mat4_t *visible_players_weapon_transforms;
//mat4_t *active_player_transform;


/* from r_main.c */
extern int r_width;
extern int r_height;

/* from world.c */
//extern bsp_pnode_t *collision_nodes;

extern int fire_sound;



#ifdef __cplusplus
extern "C"
{
#endif

int player_Init()
{
	//player_list_size = 64;
	//player_count = 0;
	//players = memory_Malloc(sizeof(player_t) * player_list_size, "player_Init");
	//free_positions_stack = memory_Malloc(sizeof(int) * player_list_size, "player_Init");
	//active_player = NULL;

	//mesh_LoadModel("weapon.obj", "weapon");
	//mesh_LoadModel("body.obj", "body");

	//weapon_mesh = mesh_GetModel("weapon");
	//body_mesh = mesh_GetModel("body");

	//spawn_points = memory_Malloc(sizeof(spawn_point_t) * max_spawn_points, "player_Init");
	//spawn_point_free_position_stack = memory_Malloc(sizeof(int) * max_spawn_points, "player_Init");


	//visible_players_body_transforms = memory_Malloc(sizeof(mat4_t ) * player_list_size, "player_Init");
	//visible_players_weapon_transforms = memory_Malloc(sizeof(mat4_t) * player_list_size, "player_Init");
	//visible_players_indexes = memory_Malloc(sizeof(int) * player_list_size, "player_Init");

	return 1;
}

void player_Finish()
{
	//int i;
	//for(i = 0; i < player_count; i++)
	//{
	//	if(players[i].bm_flags & PLAYER_INVALID)
	//		continue;

	//	memory_Free(players[i].player_name);
	//}
	//memory_Free(players);


	//for(i = 0; i < spawn_point_count; i++)
	//{
	//	if(spawn_points[i].bm_flags & SPAWN_POINT_INVALID)
	//		continue;

	//	memory_Free(spawn_points[i].name);
	//}
	//memory_Free(spawn_points);
	//memory_Free(spawn_point_free_position_stack);


	//memory_Free(free_positions_stack);
	//memory_Free(visible_players_body_transforms);
	//memory_Free(visible_players_weapon_transforms);
	//memory_Free(visible_players_indexes);
}

/*
========================================================================================
========================================================================================
========================================================================================
*/
/*
player_def_t *player_CreatePlayerDef(char *name, collider_def_t *collider_def)
{
	player_def_t *def;


	def = memory_Malloc(sizeof(player_def_t), "player_CreatePlayerDef");
	def->name = memory_Strdup(name, "player_CreatePlayerDef");
	def->collider_def = collider_def;


	if(!pl_player_defs)
	{
		pl_player_defs = def;
	}
	else
	{
		pl_last_player_def->next = def;
		def->prev = pl_last_player_def;
	}

	pl_last_player_def = def;

	return def;
}*/

/*void player_DestroyPlayerDef(char *name)
{

}*/

/*player_def_t *player_GetPlayerDefPointer(char *name)
{
	player_def_t *def;

	def = pl_player_defs;

	while(def)
	{
		if(!strcmp(name, def->name))
		{
			break;
		}
		def = def->next;
	}

	return def;
}*/

/*
========================================================================================
========================================================================================
========================================================================================
*/

int player_SpawnPlayer(mat3_t *orientation, vec3_t position, vec3_t scale, player_def_t *def)
{
	/*player_t *player;
	int player_index;
	int name_len;
	int camera_index;
	char camera_name[128];
	int w;
	int h;

	int gun_entity_def;

	if(free_positions_stack_top > -1)
	{
		player_index = free_positions_stack[free_positions_stack_top];
		free_positions_stack_top--;
	}
	else
	{
		player_index = player_count++;

		if(player_index >= player_list_size)
		{
			player = memory_Malloc(sizeof(player_t) * (player_list_size + 16), "player_CreatePlayer");
			memcpy(player, players, sizeof(player_t) * player_list_size);
			memory_Free(players);
			players = player;
			player_list_size += 16;
		}
	}

	player = &players[player_index];
	name_len = strlen(def->name) + 1;
	name_len = (name_len + 3) & (~3);
	player->player_name = memory_Calloc(name_len, 1, "player_CreatePlayer");
	strcpy(player->player_name, def->name);

	strcpy(camera_name, def->name);
	strcat(camera_name, "_camera");



	//renderer_GetWindowSize(&w, &h);

	if(def->collider_def)
	{
		player->collider_index = physics_CreateCollider(NULL, position, scale, def->collider_def, 0);
	}
	else
	{
		player->collider_index = -1;
	}

	player->player_position = position;
	//player->collision_box_position = position;

	player->player_orientation = *orientation;
	player->bm_flags = 0;


	//gun_entity_def = entity_GetEntityDef("portal gun");
	//player->gun_entity_index = entity_CreateEntity("gun", vec3(position.x + 1.0, position.y - 1.0, position.z - 2.0), vec3(0.035, 0.055, 0.055), orientation, gun_entity_def);

	//entity_SetInvisible(player->gun_entity_index);

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
	player->bm_flags = PLAYER_IN_WORLD;

	position.y = PLAYER_CAMERA_HEIGHT;
	position.x = 0.0;
	position.z = 0.0;

	//camera_index = camera_CreateCamera(camera_name, position, orientation, 0.68, r_width, r_height, 0.1, 500.0, CAMERA_UPDATE_ON_RESIZE);

	player->player_camera = camera_CreateCamera(camera_name, position, orientation, 0.68, r_width, r_height, 0.1, 500.0, CAMERA_UPDATE_ON_RESIZE);


	//player->player_type = PLAYER_NPC;

	return player_index;*/
}

struct ai_script_t *player_LoadAIScript(char *file_name, char *script_name)
{
	//return (struct ai_script_t *)script_LoadScript(file_name, script_name, SCRIPT_TYPE_AI);
}

int player_CreatePlayer(char *name, vec3_t position, mat3_t *orientation)
{
	/*player_t *player;
	int player_index;
	int name_len;
	int camera_index;
	char camera_name[128];
	int w;
	int h;

	int gun_entity_def;

	if(free_positions_stack_top > -1)
	{
		player_index = free_positions_stack[free_positions_stack_top];
		free_positions_stack_top--;
	}
	else
	{
		player_index = player_count++;

		if(player_index >= player_list_size)
		{
			player = memory_Malloc(sizeof(player_t) * (player_list_size + 16), "player_CreatePlayer");
			memcpy(player, players, sizeof(player_t) * player_list_size);
			memory_Free(players);
			players = player;
			player_list_size += 16;
		}
	}

	player = &players[player_index];
	name_len = strlen(name) + 1;
	name_len = (name_len + 3) & (~3);
	player->player_name = memory_Calloc(name_len, 1, "player_CreatePlayer");
	strcpy(player->player_name, name);

	strcpy(camera_name, name);
	strcat(camera_name, "_camera");



	//renderer_GetWindowSize(&w, &h);


	player->player_position = position;
	//player->collision_box_position = position;
	player->player_orientation = *orientation;
	player->bm_flags = 0;


	//gun_entity_def = entity_GetEntityDef("portal gun");
	//player->gun_entity_index = entity_CreateEntity("gun", vec3(position.x + 1.0, position.y - 1.0, position.z - 2.0), vec3(0.035, 0.055, 0.055), orientation, gun_entity_def);

	//entity_SetInvisible(player->gun_entity_index);

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
	position.x = 0.0;
	position.z = 0.0;

	//camera_index = camera_CreateCamera(camera_name, position, orientation, 0.68, r_width, r_height, 0.1, 500.0, CAMERA_UPDATE_ON_RESIZE);

	player->player_camera = camera_CreateCamera(camera_name, position, orientation, 0.68, r_width, r_height, 0.1, 500.0, CAMERA_UPDATE_ON_RESIZE);


	player->player_type = PLAYER_NPC;

	return player_index;*/
}

void player_DestroyPlayer(char *name)
{

}

void player_DestroyPlayerIndex(int player_index)
{

}

/*int player_CreateSpawnPoint(vec3_t position, char *name)
{
	int spawn_point_index;
	spawn_point_t *spawn_point;

	if(spawn_point_free_position_stack_top > -1)
	{
		spawn_point_index = spawn_point_free_position_stack[spawn_point_free_position_stack_top];
		spawn_point_free_position_stack_top--;
	}
	else
	{
		spawn_point_index = spawn_point_count++;

		if(spawn_point_index >= max_spawn_points)
		{
			spawn_point = memory_Malloc(sizeof(spawn_point_t) * (max_spawn_points + 16), "player_CreateSpawnPoint");
			memcpy(spawn_point, spawn_points, sizeof(spawn_point_t) * max_spawn_points);
			memory_Free(spawn_points);
			spawn_points = spawn_point;
			max_spawn_points += 16;
		}

	}

	spawn_point = &spawn_points[spawn_point_index];
	spawn_point->position = position;
	spawn_point->name = strdup(name);
	spawn_point->bm_flags = 0;

	return spawn_point_index;
}*/

/*void player_DestroySpawnPoint(int spawn_point_index)
{
	spawn_point_t *spawn_point;
	if(spawn_point_index >= 0 && spawn_point_index < spawn_point_count)
	{
		if(!(spawn_points[spawn_point_index].bm_flags & SPAWN_POINT_INVALID))
		{
			spawn_point = &spawn_points[spawn_point_index];

			spawn_point->bm_flags |= SPAWN_POINT_INVALID;

			spawn_point_free_position_stack_top++;
			spawn_point_free_position_stack[spawn_point_free_position_stack_top] = spawn_point_index;
		}
	}
}*/

/*void player_DestroyAllSpawnPoints()
{
	int i;

	for(i = 0; i < spawn_point_count; i++)
	{
		if(!(spawn_points[i].bm_flags & SPAWN_POINT_INVALID))
		{
			memory_Free(spawn_points[i].name);
		}
	}

	spawn_point_free_position_stack_top = -1;
	spawn_point_count = 0;
}*/

/*void player_SpawnPlayer(int player_index, int spawn_point_index)
{
	if(player_index >= 0 && player_index < player_count)
	{
		if(!(players[player_index].bm_flags & PLAYER_INVALID))
		{

			if(!spawn_point_count)
				return;

			if(spawn_point_index < 0)
			{
				spawn_point_index = rand() % spawn_point_count;

				while(spawn_points[spawn_point_index].bm_flags & SPAWN_POINT_INVALID)
				{
					spawn_point_index = rand() % spawn_point_count;
				}
			}

			if(!(players[player_index].bm_flags & PLAYER_IN_WORLD))
			{
				players[player_index].player_position = spawn_points[spawn_point_index].position;
				players[player_index].bm_flags |= PLAYER_IN_WORLD;
				players[player_index].bm_movement = 0;
			}

		}
	}
}*/

/*void player_RemovePlayer(int player_index)
{
	if(player_index >= 0 && player_index < player_count)
	{
		if(!(players[player_index].bm_flags & PLAYER_INVALID))
		{
			players[player_index].bm_flags &= ~PLAYER_IN_WORLD;

			if(active_player == &players[player_index])
			{
				active_player = NULL;
			}
		}
	}
}*/

player_t *player_GetPlayer(char *name)
{
	int i;

/*	for(i = 0; i < player_count; i++)
	{
		if(!strcmp(players[i].player_name, name))
		{
			return &players[i];
		}
	}

	return NULL;*/
}

player_t *player_GetActivePlayer()
{
	//return active_player;
}

void player_SetPlayerAsActive(player_t *player)
{
	/*active_player = player;
	camera_SetCamera(player->player_camera);*/
}

void player_SetPlayerAsActiveIndex(struct entity_handle_t player)
{
	/*struct entity_t *entity;
	if(player.def)
	{
		printf("player_SetPlayerAsActive: can't set an entity def as player!\n");
		return;
	}

	entity = entity_GetEntityPointerIndex(player);

	if(entity->components[COMPONENT_INDEX_PLAYER_CONTROLLER].type != COMPONENT_TYPE_PLAYER_CONTROLLER)
	{
		printf("player_SetPlayerAsActive: entity [%s] doesn't have a player controller component\n", entity->name);
		return;
	}

	pl_active_player = player;	*/
}

void player_UpdateActivePlayer(double delta_time)
{
	#if 0
	static float pitch;
	static float yaw;

	static float x_shift = 0.0;
	static float y_shift = 0.0;

	vec3_t forward_vector;
	vec3_t right_vector;
	vec3_t up_vector;

	camera_t *player_camera;

	struct entity_t *player_entity;
	struct player_controller_component_t *player_controller;
	struct transform_component_t *transform_component;

	float dx;
	float dy;
	float r;

	static int fire_timer = 0;

	if(pl_active_player.entity_index != INVALID_ENTITY_INDEX)
	{

		player_entity = entity_GetEntityPointerIndex(pl_active_player);

		transform_component = entity_GetComponentPointer(player_entity->components[COMPONENT_INDEX_TRANSFORM]);
		player_controller = entity_GetComponentPointer(player_entity->components[COMPONENT_INDEX_PLAYER_CONTROLLER]);


		if(!player_controller)
		{
			printf("player_UpdateActivePlayer: active player doesn't have a player controller component!\n");
			return;
		}

		if(player_controller->controller.type != COMPONENT_TYPE_PLAYER_CONTROLLER)
		{
			printf("player_UpdateActivePlayer: active player doesn't has the wrong controller component!\n");
			return;
		}


		//active_player->bm_movement &= ~(MOVE_FORWARD|MOVE_BACKWARD|MOVE_STRAFE_LEFT|MOVE_STRAFE_RIGHT|MOVE_JUMP);

		player_controller->control_flags &= ~(MOVE_FORWARD|MOVE_BACKWARD|MOVE_STRAFE_LEFT|MOVE_STRAFE_RIGHT|MOVE_JUMP);


	//	x_shift *= 0.925;
	//	y_shift *= 0.925;

	//	dx = normalized_mouse_x * 0.25;
	//	dy = normalized_mouse_y * 0.25;

	//	player_camera = active_player->player_camera;


	//	forward_vector = player_camera->world_orientation.f_axis;

	//	r = (forward_vector.x * forward_vector.x + forward_vector.z * forward_vector.z);

	//	active_player->pitch += dy;
	//	if(active_player->pitch > 0.5) active_player->pitch = 0.5;
	//	else if(active_player->pitch < -0.5) active_player->pitch = -0.5;

	//	active_player->yaw -= dx;

	//	if(active_player->yaw > 1.0) active_player->yaw = -1.0 + (active_player->yaw - 1.0);
	//	else if(active_player->yaw < -1.0) active_player->yaw = 1.0 + (active_player->yaw + 1.0);

	//	x_shift -= dx * delta_time * 0.05 * r;

	//	if(x_shift > 1.0) x_shift = -1.0 + (x_shift - 1.0);
	//	else if(x_shift < -1.0) x_shift = 1.0 + (x_shift + 1.0);


	//	if(active_player->pitch < 0.5 && active_player->pitch > -0.5)
	//	{
	//		y_shift += dy * delta_time * 0.05;
	//	}



	//	if(y_shift < -0.5) y_shift = -0.5;
	//	else if(y_shift > 0.5) y_shift = 0.5;

	//	active_player->weapon_x_shift = sin(x_shift * 3.14159265) * 0.5;
	//	active_player->weapon_y_shift = sin(y_shift * 3.14159265) * 0.5;
	//	active_player->weapon_z_shift = (-cos(x_shift * 3.14159265) - cos(y_shift * 3.14159265)) * 0.5;

		//printf("[%f %f %f]\n", active_player->weapon_x_shift, active_player->weapon_y_shift, active_player->weapon_z_shift);

		if(input_GetKeyPressed(SDL_SCANCODE_W))
		{
			//active_player->bm_movement |= MOVE_FORWARD;
			//printf("move forward");

			physics_Move(player_controller->controller.collider.collider_index, vec3(0.0, 0.0, -15.0));

		}

		if(input_GetKeyPressed(SDL_SCANCODE_S))
		{
			//active_player->bm_movement |= MOVE_BACKWARD;
			physics_Move(player_controller->controller.collider.collider_index, vec3(0.0, 0.0, 15.0));
		}


		if(input_GetKeyPressed(SDL_SCANCODE_A))
		{
			//active_player->bm_movement |= MOVE_STRAFE_LEFT;
			physics_Move(player_controller->controller.collider.collider_index, vec3(-15.0, 0.0, 0.0));
		}

		if(input_GetKeyPressed(SDL_SCANCODE_D))
		{
			//active_player->bm_movement |= MOVE_STRAFE_RIGHT;
			physics_Move(player_controller->controller.collider.collider_index, vec3(15.0, 0.0, 0.0));
		}

		if(input_GetKeyPressed(SDL_SCANCODE_SPACE))
		{
			//active_player->bm_movement |= MOVE_JUMP | PLAYER_JUMPED;
			physics_Jump(player_controller->controller.collider.collider_index, 100.0);
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
			//	active_player->bm_movement |= PLAYER_FIRED;
				//sound_PlaySound(fire_sound, active_player->player_camera->world_position, 1.0);
		//	}


		}
		else
		{
			//active_player->bm_movement &= ~PLAYER_FIRED;
		}



	}

	#endif
}

void player_ProcessAI(float delta_time)
{
/*	int i;
	int c = player_count;

	vec3_t player_vec;

	float yaw;
	float pitch;

	if(active_player)
	{
		for(i = 0; i < c; i++)
		{
			if(&players[i] == active_player)
				continue;

			if(!(players[i].bm_flags & PLAYER_IN_WORLD))
				continue;

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
	*/

}

void player_UpdatePlayers(double delta_time)
{
/*	int i;
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
	vec3_t weapon_position = vec3(1.0, -1.0, -1.5);
	vec3_t direction;
	vec3_t r_vector;
	vec3_t u_vector;
	vec3_t r_direction;
	vec3_t position;

	struct entity_t *gun;

	visible_player_count = 0;

	for(i = 0; i < player_count; i++)
	{

		if(!(players[i].bm_flags & PLAYER_IN_WORLD))
			continue;

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
		//	increment = AIR_DELTA_INCREMENT;
		}
		else
		{
		//	increment = GROUND_DELTA_INCREMENT;
		}


		if(players[i].bm_movement & MOVE_FORWARD)
		{
		//	players[i].delta.x -= forward_vector.x * increment * delta_time * 0.0025;
		//	players[i].delta.z -= forward_vector.z * increment * delta_time * 0.0025;

			physics_Move(players[i].collider_index, vec3(-15.0, 0.0, 0.0));

		}

		if(players[i].bm_movement & MOVE_BACKWARD)
		{
		//	players[i].delta.x += forward_vector.x * increment * delta_time * 0.0025;
		//	players[i].delta.z += forward_vector.z * increment * delta_time * 0.0025;
			physics_Move(players[i].collider_index, vec3(15.0, 0.0, 0.0));
		}


		if(players[i].bm_movement & MOVE_STRAFE_LEFT)
		{
		//	players[i].delta.x -= right_vector.x * increment * delta_time * 0.0025;
		//	players[i].delta.z -= right_vector.z * increment * delta_time * 0.0025;
			physics_Move(players[i].collider_index, vec3(0.0, 0.0, 15.0));
		}

		if(players[i].bm_movement & MOVE_STRAFE_RIGHT)
		{
		//	players[i].delta.x += right_vector.x * increment * delta_time * 0.0025;
		//	players[i].delta.z += right_vector.z * increment * delta_time * 0.0025;
			physics_Move(players[i].collider_index, vec3(0.0, 0.0, -15.0));
		}

		if(players[i].bm_movement & MOVE_JUMP)
		{
			physics_Jump(players[i].collider_index, 105.0);
		//	players[i].delta.y = JUMP_DELTA * 0.35;
		}
	}	*/
}

void player_PostUpdatePlayers(double delta_time)
{
	/*int i;
	int j;
	float r;
	vec3_t forward_vector;
	vec3_t right_vector;

	float s;
	float c;
	int transform_index;

	float increment;

	camera_t *active_camera = camera_GetActiveCamera();
	camera_t *player_camera;

	mat4_t transform;
	mat4_t weapon_transform;
	mat3_t weapon_rot;
	mat3_t rot;
	vec3_t weapon_position = vec3(1.0, -1.0, -0.35);
	vec3_t direction;
	vec3_t r_vector;
	vec3_t u_vector;
	vec3_t r_direction;
	vec3_t position;

	struct entity_t *gun;

	//printf("%f\n", delta_time);

	mat3_t_rotate(&weapon_rot, vec3(0.0, 1.0, 0.0), 0.5, 1);

	visible_player_count = 0;

	for(i = 0; i < player_count; i++)
	{

		if(!(players[i].bm_flags & PLAYER_IN_WORLD))
			continue;



	//	rot = weapon_rot;
	//	position = weapon_position;

	//	position.x += players[i].weapon_x_shift;
	//	position.y -= players[i].weapon_y_shift;
	//	position.z += players[i].weapon_z_shift;


	//	player_camera = players[i].player_camera;

	//	mat3_t_mult(&rot, &weapon_rot, &player_camera->world_orientation);
	//	mat3_t_vec3_t_mult(&player_camera->world_orientation, &position);

	//	position.x += player_camera->world_position.x;
	//	position.y += player_camera->world_position.y;
	//	position.z += player_camera->world_position.z;

	//	gun = entity_GetEnttiyPointerIndex(players[i].gun_entity_index);
	//	gun->position = position;
	//	gun->orientation = rot;
	//	gun->flags |= ENTITY_HAS_MOVED;

	}
*/
}

#define BUMP_COUNT 5
#define SPEED_THRESHOLD 0.00001
void player_Move(player_t *player, float delta_time)
{
	#if 0
	if(!player)
		return;

	int i;
	int c;
	int j;
	int k;

	float d;

	trace_t trace;

	vec3_t end;
	vec3_t new_velocity = player->delta;
	vec3_t original_velocity = player->delta;
	vec3_t velocity_vector;

	vec3_t v;

	int plane_count = 0;
	bsp_clipplane_t planes[BUMP_COUNT * 2];

	int b_early_out = 0;


	if(!(player->bm_flags & PLAYER_IN_WORLD))
		return;

	int bm_movement;


	/*new_velocity.x *= delta_time;
	new_velocity.y *= delta_time;
	new_velocity.z *= delta_time;


	original_velocity.x *= delta_time;
	original_velocity.y *= delta_time;
	original_velocity.z *= delta_time;*/
	//end.x = position->x + velocity.x;
	//end.y = position->y + velocity.y;
	//end.z = position->z + velocity.z;

	//static int b_break = 0;

	//printf("---> [%f %f %f]\n", new_velocity.x, new_velocity.y, new_velocity.z);

	if(collision_nodes)
	{

		//player->bm_movement &= ~PLAYER_ON_GROUND;

		//bm_movement = player->bm_movement;

		//if(fabs(new_velocity.x) >= SPEED_THRESHOLD || fabs(new_velocity.y) >= SPEED_THRESHOLD || fabs(new_velocity.z) >= SPEED_THRESHOLD)
		{
			player->bm_movement &= ~PLAYER_ON_GROUND;

			for(i = 0; i < BUMP_COUNT; i++)
			{

				/* still enough to ignore any movement... */
				if(fabs(new_velocity.x) < SPEED_THRESHOLD && fabs(new_velocity.y) < SPEED_THRESHOLD && fabs(new_velocity.z) < SPEED_THRESHOLD)
				{
					//b_early_out = 1;
					break;
				}

				//printf("[%f %f %f]\n", new_velocity.x, new_velocity.y, new_velocity.z);

				end.x = player->collision_box_position.x + new_velocity.x;
				end.y = player->collision_box_position.y + new_velocity.y;
				end.z = player->collision_box_position.z + new_velocity.z;

				trace.bm_flags = 0;

				//while(trace.bm_flags & TRACE_MID_SOLID)

				bsp_FirstHit(collision_nodes, player->collision_box_position, end, &trace);

				/* this is less than ideal, but time is short... */
				//do

				c = 0;



				//printf("[%f %f %f]\n", trace.normal.x, trace.normal.y, trace.normal.z);

				if(trace.bm_flags & TRACE_MID_SOLID)
				{
					velocity_vector = normalize3(original_velocity);

					while(trace.bm_flags & TRACE_MID_SOLID)
					{
						player->collision_box_position.x -= velocity_vector.x * 0.05;
						player->collision_box_position.y -= velocity_vector.y * 0.05;
						player->collision_box_position.z -= velocity_vector.z * 0.05;

						end.x = player->collision_box_position.x + original_velocity.x;
						end.y = player->collision_box_position.y + original_velocity.y;
						end.z = player->collision_box_position.z + original_velocity.z;

						bsp_FirstHit(collision_nodes, player->collision_box_position, end, &trace);

					}
				}

				/*if(trace.frac > 0.0)
				{
					original_velocity = player->delta;
					plane_count = 0;
				}*/


				/* covered whole distance, bail out... */
				if(trace.frac == 1.0)
				{
					//printf("BAIL\n");
					//b_early_out = 1;
					break;
				}
				else
				{
					/* HACK -- if our speed points in the same general direction
					as the wall we just hit, it means it's normal should be flipped
					to avoid 'floating' over surfaces... */
					if(dot3(trace.normal, new_velocity) > 0.0)
					{
						trace.normal.x = -trace.normal.x;
						trace.normal.y = -trace.normal.y;
						trace.normal.z = -trace.normal.z;
					}

				//	printf("[%f %f %f]\n",  trace.normal.x, trace.normal.y, trace.normal.z);

					/* hit a vertical-ish surface, test to see whether it's a step or a wall... */
					//if(trace.normal.y < 0.2 && trace.normal.y > -0.2)

					planes[plane_count++].normal = trace.normal;

					if(trace.normal.y >= 0.0 && trace.normal.y < 0.2)
					{

						if(bsp_TryStepUp(&player->collision_box_position, &new_velocity, &trace))
						{
							/* if step-up was successful, do not clip the speed... */
							player->bm_movement |= PLAYER_STEPPING_UP | PLAYER_ON_GROUND;
							//printf("step up!\n");
							continue;
						}
					}
					else
					{
						if(trace.normal.y >= 0.7)
							player->bm_movement |= PLAYER_ON_GROUND;
					}

					#if 0

					for(j = 0; j < plane_count; j++)
					{
						bsp_ClipVelocityToPlane(planes[j].normal, original_velocity, &new_velocity, 1.0);

						for(k = 0; k < plane_count; k++)
						{
							if(k == j)
								continue;

							if(dot3(planes[k].normal, new_velocity) < 0.0)
							{
								/* no good... */
								break;
							}

						}

						if(k == plane_count)
							break;
					}

					if(j != plane_count)
					{
						player->delta = new_velocity;
					}
					else
					{
						v = cross(planes[0].normal, planes[1].normal);
						d = dot3(v, player->delta);
						player->delta.x = v.x * d;
						player->delta.y = v.y * d;
						player->delta.z = v.z * d;
					}

					#endif

					//printf("---> [%f %f %f]\n", new_velocity.x, new_velocity.y, new_velocity.z);
					//printf("[%f %f %f]\n", trace.normal.x, trace.normal.y, trace.normal.z);
					/* horizontal-ish surface (floor or slope)... */
					bsp_ClipVelocityToPlane(trace.normal, new_velocity, &new_velocity, 1.0);

					//printf("---> [%f %f %f]\n", new_velocity.x, new_velocity.y, new_velocity.z);

					//player->collision_box_position.x += new_velocity.x * trace.frac;
					//player->collision_box_position.y += new_velocity.y * trace.frac;
					//player->collision_box_position.z += new_velocity.z * trace.frac;



				}

			}
		}


	}


	player->delta = new_velocity;

	player->collision_box_position.x += player->delta.x;
	player->collision_box_position.y += player->delta.y;
	player->collision_box_position.z += player->delta.z;

	/*if(bm_movement)
	{
		player->bm_movement = bm_movement;
	}*/

	//printf("done\n");

	//printf("%d\n", player->bm_movement & PLAYER_ON_GROUND);

	//bsp_Move(&player->player_position, &player->delta);

	#endif

}





#ifdef __cplusplus
}
#endif













