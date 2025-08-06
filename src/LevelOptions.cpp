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

		loadButtons();

		buttonExit.textInsideButton = true;
		buttonExit.fontSize = fontSize;

		loaded = true;
	}
}

void LevelOptions::input()
{
	if(!infoSquareActive)
	{
		sliderDepthWhite.input();
		Chess::botList[PWHITE].depth = sliderDepthWhite.value;

		sliderDepthBlack.input();
		Chess::botList[PBLACK].depth = sliderDepthBlack.value;

		inputButtons();

		buttonExit.checkInput();

		if(IsKeyPressed(KEY_ESCAPE) || (buttonExit.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
		{
			levelOptions.unload();
			levelStartMenu.load();
		}
	}
	else if( IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_SPACE) )
	{
		for(int l=0; l<2; l++)
		{
			for(int i=0; i<6; i++)
			{
				infoButtonList[l][i].pressed = false;
			}
		}
		infoSquareActive = false;
	}
}

void LevelOptions::draw()
{
	if(loaded)
	{
		drawRectangle({0.5f,0.0f,0.5f,1.0f}, DARKBROWN);
		drawLine({0.5f,0.0f}, {0.5f,1.0f}, 3.0f);
		drawTitle();
		drawSlider(sliderDepthWhite);
		drawSlider(sliderDepthBlack);
		drawButtons();
		drawInfoSquare(infoSquareBotType);
	}
}

void LevelOptions::unload()
{
	if(loaded)
	{
		loaded = false;
	}
}

//Loading Functions

void LevelOptions::loadButtons()
{
	std::vector<std::string> buttonNames = {"RANDOM","METRO","FOOL","JESTER","NOVICE","MASTER"};

	for(int i=0; i<6; i++)
	{
		const Vector2 buttonSize = {1.5f*squareSize.x,0.5f*squareSize.y};
		const Vector2 circleSize = {0.3f*squareSize.x,0.3f*squareSize.y};

		Vector2 buttonPos = { 0.0f, (2.5f + (float)(i/2)) * squareSize.y };
		if(i%2 == 0)
			buttonPos.x = 0.5f*squareSize.x;
		else
			buttonPos.x = 0.25f;

		Vector2 circlePos = {buttonPos.x + 1.025f*buttonSize.x, buttonPos.y + 0.5f*(buttonSize.y - circleSize.y)};

		buttonList[PWHITE][i].initialize(buttonPos,buttonSize,buttonNames[i].c_str());
		infoButtonList[PWHITE][i].initialize(circlePos, circleSize, "?");

		//Right side
		buttonPos.x += 0.5f;
		circlePos.x += 0.5f;

		buttonList[PBLACK][i].initialize(buttonPos, buttonSize, buttonNames[i].c_str());
		infoButtonList[PBLACK][i].initialize(circlePos, circleSize, "?");

		for(int l=0; l<2; l++)
		{
			buttonList[l][i].textInsideButton = true;
			buttonList[l][i].fontSize = 18.0f;

			infoButtonList[l][i].drawCircle = true;
			infoButtonList[l][i].textInsideButton = true;
			infoButtonList[l][i].fontSize = 18.0f;

			if(Chess::botList[l].botType == (Chess::BotTypes)i)
				buttonList[l][i].pressed = true;
		}
	}
}

//Input Functions

void LevelOptions::inputButtons()
{
	for(int l=0; l<2; l++)
	{
		for(int i=0; i<6; i++)
		{
			buttonList[l][i].checkInput();
			if(buttonList[l][i].pressed)
			{
				Chess::botList[l].botType = (Chess::BotTypes)i;
				//Deactivate all the other Switch Buttons
				for(int j=0; j<6; j++)
				{
					if(j!=i)
						buttonList[l][j].pressed = false;
				}
			}

			//Prevent the user from deactivating the active button.
			if(Chess::botList[l].botType == (Chess::BotTypes)i)
				buttonList[l][i].pressed = true;

			infoButtonList[l][i].checkInput();
			if( infoButtonList[l][i].pressed )
			{
				infoSquareBotType = (Chess::BotTypes)i;
				infoSquareActive = true;
			}
		}
	}
}

//Drawing Functions

void LevelOptions::drawSlider(const Slider &slider) const
{
	slider.draw();

	const std::string sliderValue = "Depth: " + std::to_string(slider.value);
	const Vector2 sliderPos = slider.getPos();
	const Vector2 sliderSize = slider.getSize();
	Raylib::drawText(sliderValue, {sliderPos.x, sliderPos.y + sliderSize.y + 0.01f},30.0f);
}

void LevelOptions::drawTitle() const
{
	constexpr float fontTitle = 50.0f;
	static const Vector2 sliderWhitePos = sliderDepthWhite.getPos();
	static const Vector2 sliderBlackPos = sliderDepthBlack.getPos();

	Raylib::drawText("White Bot", {sliderWhitePos.x, sliderWhitePos.y - 0.1f}, fontTitle);
	Raylib::drawText("Black Bot", {sliderBlackPos.x, sliderBlackPos.y - 0.1f}, fontTitle);
}

void LevelOptions::drawButtons() const
{
	for(int l=0; l<2; l++)
	{
		for(int i=0; i<6; i++)
		{
			buttonList[l][i].draw(GRAY, DARKGRAY, RAYWHITE);
			infoButtonList[l][i].draw(BLUE, DARKBLUE, RAYWHITE);
		}
	}
	buttonExit.draw(DARKGRAY, DARKGRAY, RAYWHITE);
}

void LevelOptions::drawInfoSquare(Chess::BotTypes botType) const
{
	if(infoSquareActive)
	{
		const Vector2 rectSize = {0.6f, 0.3f};
		const Vector2 rectPos = {0.5f*(1.0f - rectSize.x), 0.5f*(1.0f - rectSize.y)};
		const Rectangle rect = { rectPos.x, rectPos.y, rectSize.x, rectSize.y };
		drawRectangle( rect, DARKBLUE );
		drawRectangleLines(rect, 4.0f, BLACK);

		std::string title, text;
		switch(botType)
		{
		case RANDOMBOT:
			title = "RANDOM BOT";
			text = "Chooses moves at complete random.";
			break;

		case METROPOLISBOT:
			title = "METROPOLIS BOT";
			text = "Chooses moves at complete random,\nbut rejects moves that lead to\nworse positions.";
			break;

		case WEIGHTEDRANDOMBOT1:
			title = "FOOL BOT";
			text = "Chooses a chesspiece at random,\nthen chooses a random move based\non their weights.";
			break;

		case WEIGHTEDRANDOMBOT2:
			title = "JESTER BOT";
			text = "Chooses a random chesspiece and\nmove based on their weights.";
			break;

		case OPTIMUMBOT1:
			title = "NOVICE BOT";
			text = "Chooses a chesspiece at random,\nthen picks the best move.";
			break;

		case OPTIMUMBOT2:
			title = "MASTER BOT";
			text = "Chooses the best move available.";
			break;
		}

		const Vector2 titleSize = Raylib::getTextSize(title,50.0f,0.1f);
		constexpr float d = 0.01f;

		Raylib::drawText(title,{0.5f - 0.5f*titleSize.x, rect.y + d},50.0f);
		Raylib::drawText(text,{rect.x + d, rect.y + titleSize.y + 5.0f*d},35.0f);
	}
}