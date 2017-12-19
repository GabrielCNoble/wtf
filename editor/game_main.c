#include <stdio.h>

#include "game_main.h"
#include "camera.h"
#include "renderer.h"
#include "player.h"
#include "physics.h"
#include "input.h"
#include "world.h"
#include "engine.h"
#include "light.h"
#include "projectile.h"
#include "sound.h"

static int game_state = 0;
extern int engine_state;

int pew_sounds[14];
int bullet_impact[3];
int fire_sound;

void game_Main(float delta_time)
{
	int camera_index;
	mat3_t r;
	camera_t *active_camera;
	player_t *player;
	int w;
	int h;
	static float angle = 0.0;
	vec3_t ray_origin;
	vec3_t ray_direction;
	vec3_t direction;
	
	int i;
	static int sound_handle;
	switch(game_state)
	{
		case 0:
			r = mat3_t_id();
			
			//renderer_GetWindowSize(&w, &h);
			//camera_index = camera_CreateCamera("camera", vec3(0.0, 0.0, 0.0), &r, 0.68, w, h, 0.1, 250.0);
			//camera_SetCameraByIndex(camera_index);
			
			
			world_LoadWorldModel("yeah5.fms");
			
			player_CreatePlayer("player", vec3(0.0, 5.0, 0.0), &r);
			player = player_GetPlayer("player");
			player_SetPlayerAsActive(player);
			
			/*player_CreatePlayer("enemy0", vec3(0.0, 10.0, 0.0), &r);
			player_CreatePlayer("enemy1", vec3(5.0, 0.0, -3.0), &r);
			player_CreatePlayer("enemy2", vec3(0.0, 0.0, 8.0), &r);*/
			
			
			
			//light_CreateLight("light0", &r, vec3(0.0, 6.0, 0.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);
			
			
			/*light_CreateLight("light0", &r, vec3(0.0, 6.0, -18.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);
			light_CreateLight("light1", &r, vec3(0.0, 6.0, 18.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);
			light_CreateLight("light2", &r, vec3(18.0, 6.0, 0.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);
			light_CreateLight("light3", &r, vec3(-18.0, 6.0, 0.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);
			
			light_CreateLight("light4", &r, vec3(-17.0, -18.0, 17.0), vec3(1.0, 1.0, 1.0), 35.0, 45.0);
			light_CreateLight("light5", &r, vec3(17.0, -18.0, -17.0), vec3(1.0, 1.0, 1.0), 35.0, 45.0);
			light_CreateLight("light6", &r, vec3(-17.0, -18.0, -17.0), vec3(1.0, 1.0, 1.0), 35.0, 45.0);
			light_CreateLight("light7", &r, vec3(17.0, -18.0, 17.0), vec3(1.0, 1.0, 1.0), 35.0, 45.0);
			
			light_CreateLight("light8", &r, vec3(0.0, 4.0, 38.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);
			light_CreateLight("light9", &r, vec3(0.0, 4.0, 58.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);
			light_CreateLight("light10", &r, vec3(0.0, 4.0, 78.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);
			light_CreateLight("light11", &r, vec3(0.0, 4.0, 98.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);
			light_CreateLight("light12", &r, vec3(16.0, 4.0, 112.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);
			light_CreateLight("light13", &r, vec3(36.0, 4.0, 112.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);
			light_CreateLight("light14", &r, vec3(56.0, 4.0, 112.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);
			light_CreateLight("light15", &r, vec3(76.0, 4.0, 112.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);
			light_CreateLight("light16", &r, vec3(96.0, 4.0, 96.0), vec3(1.0, 1.0, 1.0), 30.0, 45.0);*/
			//physics_CreateBlock(vec3(0.0, 0.0, 0.0), vec3(50.0, 0.2, 50.0));
			
			//physics_CreateBlock(vec3(0.0, 2.0, 0.0), vec3(1.5, 1.5, 1.5));
			//physics_CreateBlock(vec3(0.0, 2.0, 20.0), vec3(1.0, 1.0, 1.0));
			//physics_CreateBlock(vec3(0.0, -2.0, -10.0), vec3(40.0, 2.0, 30.0));
			input_RegisterKey(SDL_SCANCODE_ESCAPE);
			
			//physics_BuildWorldMeshBVH();
			
			
			//sound_handle = sound_LoadSound("explosion.wav", "explosion");
			//sound_handle = sound_LoadSound("pew_sound.wav", "pew");
			/*pew_sounds[0] = sound_LoadSound("sounds/ric1.wav", "ric0");
			pew_sounds[1] = sound_LoadSound("sounds/ric2.wav", "ric1");
			pew_sounds[2] = sound_LoadSound("sounds/ric3.wav", "ric2");
			pew_sounds[3] = sound_LoadSound("sounds/ric4.wav", "ric3");
			pew_sounds[4] = sound_LoadSound("sounds/ric5.wav", "ric4");
			pew_sounds[5] = sound_LoadSound("sounds/ric5.wav", "ric5");
			
			pew_sounds[6] = sound_LoadSound("sounds/ric6.wav", "ric6");
			pew_sounds[7] = sound_LoadSound("sounds/ric7.wav", "ric7");
			pew_sounds[8] = sound_LoadSound("sounds/ric8.wav", "ric8");
			pew_sounds[9] = sound_LoadSound("sounds/ric9.wav", "ric9");
			pew_sounds[10] = sound_LoadSound("sounds/ric10.wav", "ric10");
			pew_sounds[11] = sound_LoadSound("sounds/ric11.wav", "ric11");
			pew_sounds[12] = sound_LoadSound("sounds/ric12.wav", "ric12");
			pew_sounds[13] = sound_LoadSound("sounds/ric13.wav", "ric13");
			
			fire_sound = sound_LoadSound("sounds/fire1.wav", "fire");


			bullet_impact[0] = sound_LoadSound("sounds/metal_box_impact_bullet1.wav", "imp0");
			bullet_impact[1] = sound_LoadSound("sounds/metal_box_impact_bullet2.wav", "imp1");
			bullet_impact[2] = sound_LoadSound("sounds/metal_box_impact_bullet3.wav", "imp2");*/
			/*pew_sounds[6] = sound_LoadSound("ric6.wav", "ric6");*/
			
			
			/*for(i = 0; i < 50; i++)
			{
				physics_CreateBlock(vec3(0.0, 3.0 + i, -10.0 * i), vec3(5.0, 0.2, 5.0));
			}*/
			
			/*for(i = 0; i < 50; i++)
			{
				physics_CreateBlock(vec3(20.0, 10.0 + i, -10.0 * i), vec3(5.0, 0.2, 5.0));
			}*/
			
			
			
			/*physics_CreateBlock(vec3(0.0, -2.0, -10.0), vec3(5.0, 0.2, 5.0));
			
			physics_CreateBlock(vec3(0.0, -1.0, -20.0), vec3(5.0, 0.2, 5.0));
			
			physics_CreateBlock(vec3(0.0, 0.0, -30.0), vec3(5.0, 0.2, 5.0));*/
			
			game_state = 1;
			
			
			
			
		break;
		
		case 1:
			
			if(input_GetKeyPressed(SDL_SCANCODE_E))
			{
				player = player_GetPlayer("player");
				player->delta.y = 0.2;
				
				//sound_PlaySound(sound_handle, vec3(0.0, 0.0, 0.0), 1.0);
			}
			
			if(input_GetKeyStatus(SDL_SCANCODE_ESCAPE) & KEY_JUST_PRESSED)
			{
				if(engine_state == ENGINE_PLAYING)
				{
					engine_SetEngineState(ENGINE_PAUSED);
				}
				else
				{
					engine_SetEngineState(ENGINE_QUIT);
				}
			}
			
			if(input_GetMouseButton(SDL_BUTTON_LEFT) & MOUSE_LEFT_BUTTON_CLICKED)
			{
				if(engine_state != ENGINE_PLAYING)
				{
					engine_SetEngineState(ENGINE_PLAYING);
				}
				else
				{
					//player = player_GetActivePlayer();
					
					//player->bm_movement |= PLAYER_FIRED;
					/*active_camera = camera_GetActiveCamera();
					direction = active_camera->world_orientation.f_axis;
					
					direction.x = -direction.x * 3.0;
					direction.y = -direction.y * 3.0;
					direction.z = -direction.z * 3.0;
					
					projectile_AddProjectile(active_camera->world_position, direction, 1.505, 600);*/
				}
				
			}
			
			/*if(input_GetMouseButton(SDL_BUTTON_LEFT) & MOUSE_LEFT_BUTTON_CLICKED)
			{
				//active_camera = camera_GetActiveCamera();
				
				//ray_origin = active_camera->world_position;
				//ray_direction = active_camera->world_orientation.f_axis;
				
				ray_origin = vec3(0.0, 1.0, 5.0);
				ray_direction = vec3(sin(3.14159265 * angle), 0.0, cos(3.14159265 * angle));
				
				//ray_direction = vec3(0.0, -1.0, 0.0);
				
				//angle += 0.01;
				
				//printf("click!\n");
				
				//ray_direction.x = -ray_direction.x;
				//ray_direction.y = -ray_direction.y;
				//ray_direction.z = -ray_direction.z;
				
				physics_RayCast(&ray_origin, &ray_direction, 1000.0, NULL);
			}*/
			
			/*if(input_GetKeyPressed(SDL_SCANCODE_K))
			{
				player = player_GetPlayer("enemy0");
				player_SetPlayerAsActive(player);
			}
			else if(input_GetKeyPressed(SDL_SCANCODE_L))
			{
				player = player_GetPlayer("player");
				player_SetPlayerAsActive(player);
			}*/
			
		break;
	}
}
