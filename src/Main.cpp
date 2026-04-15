#include "ChessGUI.h"
#include "ChessUCI.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

void updateFrame()
{
	//updateWindowSize();

	const float scale = std::min( (float)GetScreenWidth()/Window.widthf, (float)GetScreenHeight()/Window.heightf );
	if( IsWindowResized() )
	{
		//Update the Mouse, so it works with resizable screens.
		SetMouseOffset(-(GetScreenWidth() - (int)(Window.widthf*scale))/2, -(GetScreenHeight() - (int)(Window.heightf*scale))/2);
		SetMouseScale(1.0f/scale, 1.0f/scale);
	}

	Level *level = LevelList[LevelID];

	level->input();

	//Draw to the render texture.
	BeginTextureMode(Window.renderTexture);
	ClearBackground(ColorBackground);
	level->draw();
	EndTextureMode();

	//Draw the render texture to the screen.
	BeginDrawing();
	ClearBackground({25,25,25,255});
	const Rectangle rectRenderTexture = { 0.0f, 0.0f, (float)Window.renderTexture.texture.width, (float)-Window.renderTexture.texture.height  };
	const Rectangle rectScreen = { (GetScreenWidth() - (Window.widthf*scale))*0.5f, (GetScreenHeight() - (Window.heightf*scale))*0.5f, 
		Window.widthf*scale, Window.heightf*scale };
	DrawTexturePro(Window.renderTexture.texture, rectRenderTexture, rectScreen, {0,0}, 0.0f, WHITE);
	EndDrawing();

}

int main()
{
	loadGame();

#if defined(PLATFORM_WEB)
	emscripten_set_main_loop(updateFrame, 0, 1);
#else
	std::cout << "\nHINT: You can use the terminal to issue commands to the Chess Engine!\n";
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