#include "game.h"
#include "vector.h"
#include "engine.h"

#include "..\common\input.h"
#include "..\common\gui_imgui.h"
#include "..\common\world.h"
#include "..\common\sound.h"
#include "..\common\l_main.h"
#include "..\common\camera.h"
#include "..\common\entity.h"

int game_state = GAME_STATE_NONE;

extern int r_window_width;
extern int r_window_height;
struct world_script_t *world_script;

void game_Init(int argc, char *argv[])
{
	game_state = GAME_STATE_MAIN_MENU;
	gui_ImGuiAddFontFromFileTTF("fixedsys.ttf", 32);
	engine_SetEngineState(ENGINE_PAUSED);

	renderer_Debug(0, 0);


	input_RegisterKey(SDL_SCANCODE_ESCAPE);
	input_RegisterKey(SDL_SCANCODE_W);
	input_RegisterKey(SDL_SCANCODE_S);
	input_RegisterKey(SDL_SCANCODE_A);
	input_RegisterKey(SDL_SCANCODE_D);
	input_RegisterKey(SDL_SCANCODE_K);
	input_RegisterKey(SDL_SCANCODE_C);
	input_RegisterKey(SDL_SCANCODE_SPACE);
	input_RegisterKey(SDL_SCANCODE_LSHIFT);




	input_RegisterKey(SDL_SCANCODE_H);
	input_RegisterKey(SDL_SCANCODE_J);
	input_RegisterKey(SDL_SCANCODE_L);
	input_RegisterKey(SDL_SCANCODE_P);
	input_RegisterKey(SDL_SCANCODE_R);
	input_RegisterKey(SDL_SCANCODE_G);
	input_RegisterKey(SDL_SCANCODE_M);
	input_RegisterKey(SDL_SCANCODE_T);
	input_RegisterKey(SDL_SCANCODE_V);

	input_RegisterKey(SDL_SCANCODE_X);
	input_RegisterKey(SDL_SCANCODE_Y);


	input_RegisterKey(SDL_SCANCODE_DELETE);
	input_RegisterKey(SDL_SCANCODE_TAB);


	world_script = world_LoadScript("map.was", "level");
	sound_LoadSound("pokey_intro.ogg", "pokey_intro");
	sound_LoadSound("pokey_loop.ogg", "pokey_loop");

	sound_LoadSound("wilhelm.ogg", "death");

	sound_LoadSound("explode3.wav", "explosion0");
	sound_LoadSound("explode4.wav", "explosion1");
	sound_LoadSound("explode5.wav", "explosion2");

	int explosion_texture = texture_LoadTexture("explosion2.ptx", "explosion", 0);
	struct particle_system_script_t *ps_script = particle_LoadParticleSystemScript("explosion.pas", "explosion");
	particle_CreateParticleSystemDef("explosion", 1, 60, 1, 0, explosion_texture, ps_script);
}

void game_Finish()
{

}

void game_Main(float delta_time)
{
	struct entity_handle_t player;
	struct entity_t *player_ptr;
	struct entity_prop_t *score;
	struct entity_prop_t *life;
	struct entity_transform_t *world_transform;

	camera_t *death_camera;

	int life_value;

	switch(game_state)
	{
		case GAME_STATE_MAIN_MENU:
			gui_ImGuiPushFont(gui_ImGuiGetFontIndex(1));
			gui_ImGuiSetNextWindowPos(vec2(r_window_width / 2 - 115.0, r_window_height / 2), 0, vec2(0.0, 0.0));

			gui_ImGuiBegin("Main menu", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

			if(gui_ImGuiButton("Start", vec2(230.0, 32.0)))
			{
				game_state = GAME_STATE_PLAYING;
				world_SetWorldScript(world_script);
				bsp_LoadBsp("map.bsp");
			}

			if(gui_ImGuiButton("Quit", vec2(230.0, 32.0)))
			{
				game_state = GAME_STATE_QUIT;
			}
			gui_ImGuiEnd();

			gui_ImGuiPopFont();
		break;

		case GAME_STATE_GAME_OVER:
			death_camera = camera_GetCamera("default camera");
            camera_SetCamera(death_camera);

            player = entity_GetEntityHandle("player entity", 0);

            player_ptr = entity_GetEntityPointerHandle(player);

            if(player_ptr)
			{
                world_transform = entity_GetWorldTransformPointer(player_ptr->components[COMPONENT_TYPE_TRANSFORM]);

                death_camera->world_position.x = world_transform->transform.floats[3][0];
                death_camera->world_position.y = world_transform->transform.floats[3][1];
                death_camera->world_position.z = world_transform->transform.floats[3][2];

                camera_ComputeWorldToCameraMatrix(death_camera);

				entity_MarkForRemoval(player);
			}
			else
			{
				engine_SetEngineState(ENGINE_PAUSED);

				gui_ImGuiPushFont(gui_ImGuiGetFontIndex(1));
				gui_ImGuiSetNextWindowSize(vec2(650.0, 100.0), 0);
				gui_ImGuiSetNextWindowPos(vec2(r_window_width / 2 - 325.0, r_window_height / 2), 0, vec2(0.0, 0.0));



				gui_ImGuiBegin("Game over", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

				gui_ImGuiText("You died. Yeah, dead. As in, defunct.");

				if(gui_ImGuiButton("Quit", vec2(630.0, 32.0)))
				{
					game_state = GAME_STATE_QUIT;
				}

				gui_ImGuiEnd();
				gui_ImGuiPopFont();
			}

		break;

		case GAME_STATE_PLAYING:

			engine_SetEngineState(ENGINE_PLAYING);

			if(input_GetKeyStatus(SDL_SCANCODE_ESCAPE) & KEY_JUST_PRESSED)
			{
                game_state = GAME_STATE_PAUSED;
			}

			player = entity_GetEntityHandle("player entity", 0);

			player_ptr = entity_GetEntityPointerHandle(player);

			if(player_ptr)
			{
				score = entity_GetPropPointer(player, "score");

				if(score)
				{
					gui_ImGuiPushFont(gui_ImGuiGetFontIndex(1));

					gui_ImGuiSetNextWindowPos(vec2(r_window_width / 2 - 150.0, 0.0), 0, vec2(0.0, 0.0));
					gui_ImGuiSetNextWindowSize(vec2(300.0, 40.0), 0);
					gui_ImGuiBegin("Score", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
					gui_ImGuiText("Score: %d", *(int *)score->memory);
					gui_ImGuiEnd();

					gui_ImGuiPopFont();
				}

				life = entity_GetPropPointer(player, "life");

				if(life)
				{

					/* What is the life's value? Cannot know
					if not living. And during life, it's always
					changing... */
					life_value = *(int *)life->memory;

					gui_ImGuiPushFont(gui_ImGuiGetFontIndex(1));

					gui_ImGuiSetNextWindowPos(vec2(0.0, r_window_height - 40.0), 0, vec2(0.0, 0.0));
					gui_ImGuiSetNextWindowSize(vec2(200.0, 40.0), 0);
					gui_ImGuiBegin("Health", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
					gui_ImGuiText("Health: %d", *(int *)life->memory);
					gui_ImGuiEnd();

					gui_ImGuiPopFont();


                    if(life_value <= 0)
					{
                        game_state = GAME_STATE_GAME_OVER;
					}
				}
			}
		break;

		case GAME_STATE_PAUSED:

			 engine_SetEngineState(ENGINE_PAUSED);

			gui_ImGuiPushFont(gui_ImGuiGetFontIndex(1));
			gui_ImGuiSetNextWindowPos(vec2(r_window_width / 2 - 115.0, r_window_height / 2), 0, vec2(0.0, 0.0));

			gui_ImGuiBegin("Main menu", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

			if(gui_ImGuiButton("Resume", vec2(230.0, 32.0)))
			{
				game_state = GAME_STATE_PLAYING;
			}

			if(gui_ImGuiButton("Quit", vec2(230.0, 32.0)))
			{
                game_state = GAME_STATE_QUIT;
			}
			gui_ImGuiEnd();
			gui_ImGuiPopFont();
		break;

		case GAME_STATE_QUIT:
			sound_StopAllSounds();
			engine_SetEngineState(ENGINE_QUIT);
		break;
	}
}









