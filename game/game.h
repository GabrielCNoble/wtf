#ifndef GAME_H
#define GAME_H


enum GAME_STATE
{
	GAME_STATE_PLAYING,
	GAME_STATE_PAUSED,
	GAME_STATE_QUIT,

	GAME_STATE_SPLASH,


	GAME_STATE_LOAD_MAP,

	GAME_STATE_MAIN_MENU,
	GAME_STATE_GAME_OVER,

	GAME_STATE_NONE,
};


void game_Init(int argc, char *argv[]);

void game_Finish();

void game_Main(float delta_time);


#endif // GAME_H
