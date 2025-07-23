#include "ChessGUI.h"

int main()
{
	loadGame();

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