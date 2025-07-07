#include "LevelStartMenu.h"
#include "LevelChessboard.h"
#include "LevelOptions.h"
#include "ChessGUI.h"
using namespace Chess;
using namespace Raylib;

//Public Level Functions.

void LevelOptions::load()
{
	//Switch the currently active level.
	LevelID = this->ID();
	ColorBackground = MIDDLEBROWN;

	if(!loaded)
	{
		constexpr float fontSize = 35.0f;

		buttonExit.textInsideButton = true;
		buttonExit.fontSize = fontSize;

		loaded = true;
	}
}

void LevelOptions::input()
{
	sliderDepthWhite.input();
	Chess::botList[PWHITE].depth = sliderDepthWhite.value;

	sliderDepthBlack.input();
	Chess::botList[PBLACK].depth = sliderDepthBlack.value;

	buttonExit.checkInput();
	if( IsKeyPressed(KEY_ESCAPE) || (buttonExit.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) )
	{
		levelOptions.unload();
		levelStartMenu.load();
	}
}

void LevelOptions::draw()
{
	if(loaded)
	{
		drawUI();
		buttonExit.draw(DARKGRAY, DARKGRAY, RAYWHITE);
	}
}

void LevelOptions::unload()
{
	if(loaded)
	{
		loaded = false;
	}
}

//Drawing Functions

void LevelOptions::drawSlider(const Slider &slider) const
{
	slider.draw();

	const std::string sliderValue = "Depth: " + std::to_string(slider.value);
	const Vector2 sliderPos = slider.getPos();
	const Vector2 sliderSize = slider.getSize();
	drawText(sliderValue, {sliderPos.x, sliderPos.y + sliderSize.y + 0.01f},30.0f);
}

void LevelOptions::drawUI() const
{
	drawRectangle({0.5f,0.0f,0.5f,1.0f},DARKBROWN);
	drawLine({0.5f,0.0f},{0.5f,1.0f},3.0f);

	constexpr float fontTitle = 50.0f;
	static const Vector2 sliderWhitePos = sliderDepthWhite.getPos();
	static const Vector2 sliderBlackPos = sliderDepthBlack.getPos();

	drawText("White Bot", {sliderWhitePos.x, sliderWhitePos.y - 0.1f}, fontTitle);

	drawSlider(sliderDepthWhite);

	drawText("Black Bot", {sliderBlackPos.x, sliderBlackPos.y - 0.1f}, fontTitle);

	drawSlider(sliderDepthBlack);
}