#include "main.h"
#include "engine.h"






int main(int argc, char *argv[])
{
    engine_Init(1366, 768, INIT_WINDOWED, argc, argv);
    engine_SetGameStartupFunction(game_Init);
    engine_SetGameShutdownFunction(game_Finish);
    engine_SetGameMainFunction(game_Main);
    engine_MainLoop();
    engine_Finish();
}
