#include "main.h"

int main(int argc, char *argv[])
{
	engine_Init(800, 600, INIT_WINDOWED, argc, argv);
	engine_SetGameStartupFunction(editor_Init);
	engine_SetGameMainFunction(editor_Main);
	engine_SetGameShutdownFunction(editor_Finish);
	engine_MainLoop();
	engine_Finish();
	return 0;
}
