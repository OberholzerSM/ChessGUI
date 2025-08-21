#include "ChessGUI.h"
#include "ChessUCI.h"

int main()
{
	loadGame();

	std::cout << "HINT: You can use the terminal to issue commands to the Chess Engine!\n";
	std::cout << "HINT: Press \"help\" or \"h\" to see all available commands.\n";
	std::thread threadUCI(ChessUCI::inputUCI);
	threadUCI.detach();

	while(!WindowShouldClose() && Window.active)
	{
		updateWindowSize();

		Level *level = LevelList[LevelID];

		level->input();

		BeginDrawing();
		ClearBackground(ColorBackground);
		level->draw();
		EndDrawing();
	}

	unloadGame();
	return 0;
}