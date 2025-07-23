#include "ChessGUI.h"
using namespace Chess;
using namespace Raylib;

//Loading and unloading global ressources

void Resources::load()
{
	if(!loaded)
	{
		textureSprites.load();
		textureSprites.getTextureSize(textureSize);
		textureSprites.setSpriteSize({textureSize.x / 6.0f, textureSize.y / 2.0f});

		arrowTexture.load();
		cursorTexture.load();

		loaded = true;
	}
}

void Resources::unload()
{
	if(loaded)
	{
		textureSprites.unload();
		arrowTexture.unload();
		cursorTexture.unload();
		loaded = false;
	}
}

//Loading and unloading the Game.

void loadGame()
{
	Window.load();
	Resources::load();
	speechbubble.load();

	mainEngine.initialize();
	LevelID = levelStartMenu.ID(); //ID of the Startscreen
	levelStartMenu.load(); //Load the Startscreen last.
}

void unloadGame()
{
	for(std::size_t i=0; i<nLevels; i++)
		LevelList[i]->unload();
	speechbubble.unload();
	Resources::unload();
	Window.unload();
	Chess::stopAllThreads();
}