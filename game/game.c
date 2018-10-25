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
#include "..\common\r_main.h"
#include "..\common\r_debug.h"
#include "..\common\script.h"

int game_state = GAME_STATE_NONE;

extern int r_window_width;
extern int r_window_height;
struct world_script_t *world_script;


void game_ScriptSetGameState(int state)
{
    game_state = state;
}



void game_Init(int argc, char *argv[])
{
	game_state = GAME_STATE_MAIN_MENU;
	gui_ImGuiAddFontFromFileTTF("fixedsys.ttf", 32);
	engine_SetEngineState(ENGINE_PAUSED);

	renderer_Enable(R_Z_PRE_PASS);
	renderer_Disable(R_DEBUG);
	//renderer_Enable(R_DEBUG);
	//renderer_Enable(R_VERBOSE_DEBUG);
	//renderer_Enable(R_WIREFRAME);


	//renderer_Enable(R_WIREFRAME);
	//renderer_Fullscreen(1);


	script_RegisterGlobalFunction("void game_SetGameState(int state)", game_ScriptSetGameState);
    script_RegisterEnum("GAME_STATE");
    script_RegisterEnumValue("GAME_STATE", "GAME_STATE_PLAYING", GAME_STATE_PLAYING);
    script_RegisterEnumValue("GAME_STATE", "GAME_STATE_GAME_OVER", GAME_STATE_GAME_OVER);
    script_RegisterEnumValue("GAME_STATE", "GAME_STATE_QUIT", GAME_STATE_QUIT);



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
	//sound_LoadSound("pokey_intro.ogg", "pokey_intro");
	//sound_LoadSound("pokey_loop.ogg", "pokey_loop");

	//sound_LoadSound("wilhelm.ogg", "death");

	/*sound_LoadSound("explode3.wav", "explosion0");
	sound_LoadSound("explode4.wav", "explosion1");
	sound_LoadSound("explode5.wav", "explosion2");

	sound_LoadSound("SCREAM_4.ogg", "scream");

	sound_LoadSound("pain0.ogg", "pain");

	sound_LoadSound("laser4.wav", "laser");*/

	/*sound_LoadSound("doh0.ogg", "doh0");
	sound_LoadSound("doh1.ogg", "doh1");
	sound_LoadSound("doh2.ogg", "doh2");
	sound_LoadSound("doh3.ogg", "doh3");
	sound_LoadSound("doh4.ogg", "doh4");
	sound_LoadSound("doh5.ogg", "doh5");
	sound_LoadSound("doh6.ogg", "doh6");
	sound_LoadSound("doh7.ogg", "doh7");
	sound_LoadSound("doh8.ogg", "doh8");
	sound_LoadSound("doh9.ogg", "doh9");
	sound_LoadSound("doh10.ogg", "doh10");
	sound_LoadSound("doh11.ogg", "doh11");
	sound_LoadSound("doh12.ogg", "doh12");
	sound_LoadSound("doh13.ogg", "doh13");
	sound_LoadSound("doh14.ogg", "doh14");
	sound_LoadSound("doh15.ogg", "doh15");
	sound_LoadSound("doh16.ogg", "doh16");
	sound_LoadSound("doh17.ogg", "doh17");
	sound_LoadSound("doh18.ogg", "doh18");
	sound_LoadSound("doh19.ogg", "doh19");
	sound_LoadSound("doh20.ogg", "doh20");*/


	//sound_LoadSound("giygas_lair.ogg", "text");
	//sound_LoadSound("prayer_for_safety.ogg", "won");

	int explosion_texture = texture_LoadTexture("explosion2.ptx", "explosion", 0);
	struct particle_system_script_t *ps_script = particle_LoadParticleSystemScript("explosion.pas", "explosion");
	particle_CreateParticleSystemDef("explosion", 1, 60, 1, 0, explosion_texture, ps_script);


    view_def_t *active_view;

    active_view = renderer_GetActiveView();

    active_view->world_position.x = -350.0;
    active_view->world_position.y = -350.0;
    active_view->world_position.z = -350.0;

    renderer_ComputeViewMatrix(active_view);

	/*camera_t *camera;


	camera = camera_GetActiveCamera();
    camera->world_position.x = -350.0;
    camera->world_position.y = -350.0;
    camera->world_position.z = -350.0;
    camera_ComputeWorldToCameraMatrix(camera);*/
}

void game_Finish()
{

}

void game_Options()
{

	int current_shadow_maps_resolution;
	int current_frame_rate_clamping;
	//char current_shadow_maps_resolution_text[512];

	char *current_shadow_maps_resolution_text;
	char *current_frame_rate_clamping_text;

	gui_ImGuiPushItemWidth(230.0);

	if(!renderer_GetFullscreen())
	{
		if(gui_ImGuiButton("Fullscreen", vec2(230.0, 32.0)))
		{
			renderer_Fullscreen(1);
		}
	}
	else
	{
		if(gui_ImGuiButton("Windowed", vec2(230.0, 32.0)))
		{
            renderer_Fullscreen(0);
		}
	}

	current_shadow_maps_resolution = renderer_GetShadowMapResolution();

	switch(current_shadow_maps_resolution)
	{
		case 1024:
			current_shadow_maps_resolution_text = "High";
		break;

		case 512:
			current_shadow_maps_resolution_text = "Medium";
		break;

		case 256:
			current_shadow_maps_resolution_text = "Low";
		break;

		case 128:
			current_shadow_maps_resolution_text = "Very low";
		break;

		default:
			current_shadow_maps_resolution_text = "Off";
		break;
	}




	current_frame_rate_clamping = renderer_GetFrameRateClamping();

	switch(current_frame_rate_clamping)
	{
		case 0:
            current_frame_rate_clamping_text = "Uncapped";
		break;

		case 1:
			current_frame_rate_clamping_text = "60 fps";
		break;

		case 2:
			current_frame_rate_clamping_text = "30 fps";
		break;
	}



	//sprintf(current_shadow_maps_resolution_text, "%d", current_shadow_maps_resolution);

	if(gui_ImGuiBeginCombo("Shadows", current_shadow_maps_resolution_text, 0))
	{
		if(gui_ImGuiMenuItem("High", NULL, NULL, 1))
		{
			renderer_SetShadowMapResolution(1024);
			gui_ImGuiCloseCurrentPopup();
		}
		if(gui_ImGuiMenuItem("Medium", NULL, NULL, 1))
		{
			renderer_SetShadowMapResolution(512);
			gui_ImGuiCloseCurrentPopup();
		}
		if(gui_ImGuiMenuItem("Low", NULL, NULL, 1))
		{
			renderer_SetShadowMapResolution(256);
			gui_ImGuiCloseCurrentPopup();
		}
		if(gui_ImGuiMenuItem("Very low", NULL, NULL, 1))
		{
			renderer_SetShadowMapResolution(128);
			gui_ImGuiCloseCurrentPopup();
		}
		if(gui_ImGuiMenuItem("Off", NULL, NULL, 1))
		{
			renderer_SetShadowMapResolution(0);
			gui_ImGuiCloseCurrentPopup();
		}

		gui_ImGuiEndCombo();
	}


	if(gui_ImGuiBeginCombo("Frame rate capping", current_frame_rate_clamping_text, 0))
	{
        if(gui_ImGuiMenuItem("Uncapped", NULL, NULL, 1))
		{
			renderer_SetFrameRateClamping(0);
		}
		if(gui_ImGuiMenuItem("60 fps", NULL, NULL, 1))
		{
			renderer_SetFrameRateClamping(1);
		}
		if(gui_ImGuiMenuItem("30 fps", NULL, NULL, 1))
		{
			renderer_SetFrameRateClamping(2);
		}

		gui_ImGuiEndCombo();
	}

	gui_ImGuiPopItemWidth();
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

	static int main_menu_options_open = 0;
	static int game_menu_options_open = 0;



	switch(game_state)
	{
		case GAME_STATE_MAIN_MENU:
			gui_ImGuiPushFont(gui_ImGuiGetFontIndex(1));
			gui_ImGuiSetNextWindowPos(vec2(r_window_width / 2 - 115.0, r_window_height / 2), 0, vec2(0.0, 0.0));

			gui_ImGuiBegin("Main menu", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

			if(!main_menu_options_open)
			{
				if(gui_ImGuiButton("Start", vec2(230.0, 32.0)))
				{
					game_state = GAME_STATE_PLAYING;
					world_SetWorldScript(world_script);
					bsp_LoadBsp("map.bsp");
				}

				if(gui_ImGuiButton("Options", vec2(230.0, 32.0)))
				{
                    main_menu_options_open = 1;
				}

				if(gui_ImGuiButton("Quit", vec2(230.0, 32.0)))
				{
					game_state = GAME_STATE_QUIT;
				}
			}
			else
			{

				game_Options();

				if(gui_ImGuiButton("Back", vec2(230.0, 32.0)))
				{
					main_menu_options_open = 0;
				}
			}

			gui_ImGuiEnd();

			gui_ImGuiPopFont();

			return;

		break;

		case GAME_STATE_GAME_OVER:
			death_camera = camera_GetCamera("default camera");
            //camera_SetCamera(death_camera);
            renderer_SetActiveView((view_def_t *)death_camera);

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

			if(!game_menu_options_open)
			{
				if(gui_ImGuiButton("Resume", vec2(230.0, 32.0)))
				{
					game_state = GAME_STATE_PLAYING;
				}

				if(gui_ImGuiButton("Options", vec2(230.0, 32.0)))
				{
                    game_menu_options_open = 1;
				}

				if(gui_ImGuiButton("Quit", vec2(230.0, 32.0)))
				{
					game_state = GAME_STATE_QUIT;
				}
			}
			else
			{
				game_Options();

				if(gui_ImGuiButton("Back", vec2(230.0, 32.0)))
				{
					game_menu_options_open = 0;
				}
			}

			gui_ImGuiEnd();
			gui_ImGuiPopFont();

			return;
		break;

		case GAME_STATE_QUIT:
			sound_StopAllSounds();
			engine_SetEngineState(ENGINE_QUIT);
		break;
	}

	main_menu_options_open = 0;
	game_menu_options_open = 0;
}









