#include "ChessGUI.h"
#include "ChessUCI.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

void updateFrame()
{
	updateWindowSize();

	Level *level = LevelList[LevelID];

	level->input();

	BeginDrawing();
	ClearBackground(ColorBackground);
	level->draw();
	EndDrawing();
}

int main()
{
	loadGame();

#if defined(PLATFORM_WEB)
	emscripten_set_main_loop(updateFrame, 0, 1);
#else
	std::cout << "HINT: You can use the terminal to issue commands to the Chess Engine!\n";
	std::cout << "HINT: Press \"help\" or \"h\" to see all available commands.\n";
	std::thread threadUCI(ChessUCI::inputUCI);
	threadUCI.detach();

	while(!WindowShouldClose() && Window.active)
	{
		updateFrame();
	}

#endif

	unloadGame();
	return 0;
}