#pragma once
#include "LevelStartMenu.h"
#include "LevelChessboard.h"
#include "LevelOptions.h"
#include "LevelSetupChessboard.h"
using namespace Raylib;

//Put all the Ressources here, so every level has access to them.
namespace Resources
{
    inline RaylibTexture textureSprites{"Chess_Pieces_Sprite.png"};
    inline RaylibTexture arrowTexture{"Arrow.png"};
    inline RaylibTexture cursorTexture{"Cursor.png"};
    inline RaylibTexture starEmptyTexture{"StarEmpty.png"};
    inline RaylibTexture starFullTexture{"StarFull.png"};
    inline Vector2 textureSize{}; //Size of textureSprites

    inline bool loaded = false;

    void load();
    void unload();
}

inline SpeechBubble speechbubble{"Hello!",20.0f};

inline const std::size_t nLevels = LevelList.size();

//Load the Game.
void loadGame();

//Unload the Game.
void unloadGame();