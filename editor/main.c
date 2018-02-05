#include "main.h"

int main(int argc, char *argv[]) 
{	
	engine_Init(1366, 768, INIT_WINDOWED);
	engine_SetGameStartupFunction(editor_Init);
	engine_SetGameMainFunction(editor_Main);
	engine_MainLoop();
	engine_Finish();
	return 0;
}
