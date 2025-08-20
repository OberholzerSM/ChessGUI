#include "LevelStartMenu.h"
#include "LevelChessboard.h"
#include "LevelOptions.h"
#include "ChessGUI.h"
using namespace Chess;
using namespace Raylib;

//Public Functions

void LevelStartMenu::load()
{
	//Switch the currently active level.
	LevelID = this->ID();
	ColorBackground = MIDDLEBROWN;

    //Resize sprite
    Resources::textureSprites.setSize({2.0f*squareSize.x,2.0f*squareSize.y});

    choiceBoxActive = false;

    if(!loaded)
    {
        constexpr float fontSize = 35.0f;

        buttonPlayervsPlayer.textInsideButton = true;
        buttonPlayervsPlayer.fontSize = fontSize;

        buttonPlayervsBot.textInsideButton = true;
        buttonPlayervsBot.fontSize = fontSize;

        buttonBotvsBot.textInsideButton = true;
        buttonBotvsBot.fontSize = fontSize;

        buttonCustom.textInsideButton = true;
        buttonCustom.fontSize = fontSize;

        buttonOptions.textInsideButton = true;
        buttonOptions.fontSize = fontSize;

        buttonExit.textInsideButton = true;
        buttonExit.fontSize = fontSize;

        loaded = true;
    }
}

void LevelStartMenu::input()
{
    if(!choiceBoxActive)
    {
        inputButtonPlayervsPlayer();
        inputButtonPlayervsBot();
        inputButtonBotvsBot();
        inputButtonCustom();
        inputButtonOption();
        inputButtonExit();
    }
    else
        inputChoiceBox();

    if(IsKeyPressed(KEY_ESCAPE))
        choiceBoxActive = false;
}

void LevelStartMenu::draw()
{
    if(loaded)
    {
        drawBoardSquares();
        drawBoardLines();
        drawTitle();
        drawButtons();
        if(choiceBoxActive)
            drawChoiceBox();
    }
}

void LevelStartMenu::unload()
{
    if(loaded)
    {
        loaded = false;
    }
}

//Input Functions

void LevelStartMenu::inputButtonPlayervsPlayer()
{
    buttonPlayervsPlayer.checkInput();
    if(buttonPlayervsPlayer.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        levelStartMenu.unload();
        levelChessboard.load();
        levelChessboard.playerControll[PWHITE] = true;
        levelChessboard.playerControll[PBLACK] = true;
    }
}

void LevelStartMenu::inputButtonPlayervsBot()
{
    buttonPlayervsBot.checkInput();
    if(buttonPlayervsBot.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        choiceBoxActive = true;
}

void LevelStartMenu::inputButtonBotvsBot()
{
    buttonBotvsBot.checkInput();
    if(buttonBotvsBot.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        levelStartMenu.unload();
        levelChessboard.load();
        levelChessboard.playerControll[PWHITE] = false;
        levelChessboard.playerControll[PBLACK] = false;
    }
}

void LevelStartMenu::inputButtonCustom()
{
    buttonCustom.checkInput();
    if( buttonCustom.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        levelStartMenu.unload();
        levelSetupChessboard.load();
    }
}

void LevelStartMenu::inputButtonOption()
{
    buttonOptions.checkInput();
    if(buttonOptions.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        levelStartMenu.unload();
        levelOptions.load();
    }
}

void LevelStartMenu::inputButtonExit()
{
    buttonExit.checkInput();
    if( IsKeyPressed(KEY_ESCAPE) || (buttonExit.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) )
    {
        Window.active = false;
    }
}

void LevelStartMenu::inputChoiceBox()
{
    buttonWhite.checkInput();
    if(buttonWhite.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        levelStartMenu.unload();
        levelChessboard.load();
        levelChessboard.playerControll[PWHITE] = true;
        levelChessboard.playerControll[PBLACK] = false;
        choiceBoxActive = false;

        return;
    }

    buttonBlack.checkInput();
    if(buttonBlack.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        levelStartMenu.unload();
        levelChessboard.load();
        levelChessboard.playerControll[PWHITE] = false;
        levelChessboard.playerControll[PBLACK] = true;
        choiceBoxActive = false;

        const bool flipBoardOld = levelChessboard.flipBoard;
        levelChessboard.buttonFlipBoard.pressed = true;
        levelChessboard.flipBoard = true;

        if(levelChessboard.flipBoard && !flipBoardOld)
        {
            levelChessboard.setChessPieceSpritePos();
            levelChessboard.buttonShowDangerZoneWhite.move({8.75f*squareSize.x,3.25f*squareSize.y});
            levelChessboard.buttonShowDangerZoneBlack.move({8.75f*squareSize.x,4.25f*squareSize.y});
        }

        return;
    }
}

//Drawing Functions

void LevelStartMenu::drawBoardSquares() const
{
    for(int i = 0; i < nSquaresHorizontal; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            Rectangle rect1 = {boardOrigin.x + (float)(i)*squareSize.x, boardOrigin.y + (float)(j)*squareSize.y, squareSize.x, squareSize.y};
            Rectangle rect2 = {boardOrigin.x + (float)(i+7)*squareSize.x, boardOrigin.y + (float)(j)*squareSize.y, squareSize.x, squareSize.y};

            if((i + j) % 2 == 0)
            {
                drawRectangle(rect1, BROWN);
                drawRectangle(rect2, BROWN);
            }
            else
            {
                drawRectangle(rect1, DARKBROWN);
                drawRectangle(rect2, DARKBROWN);
            }
        }
    }
}

void LevelStartMenu::drawBoardLines() const
{
    const float thickness = 5.0f;
    Vector2 startpoint{}, endpoint{};
    for(int i = 0; i <= nSquaresHorizontal; i++)
    {
        startpoint.x = boardOrigin.x + (float)(i) * squareSize.x;
        startpoint.y = boardOrigin.y;
        endpoint.x = boardOrigin.x + (float)(i) * squareSize.x;
        endpoint.y = boardOrigin.y + 8.0f * squareSize.y;
        drawLine(startpoint,endpoint,thickness);

        startpoint.x = boardOrigin.x + (float)(i+7) * squareSize.x;
        startpoint.y = boardOrigin.y;
        endpoint.x = boardOrigin.x + (float)(i+7) * squareSize.x;
        endpoint.y = boardOrigin.y + 8.0f * squareSize.y;
        drawLine(startpoint, endpoint, thickness);
    }

    for(int j=0; j < 6; j++)
    {
        startpoint.x = boardOrigin.x;
        startpoint.y = boardOrigin.y + (float)(j) * squareSize.y;
        endpoint.x = boardOrigin.x + 10.0f * squareSize.x;
        endpoint.y = boardOrigin.y + (float)(j) * squareSize.y;
        drawLine(startpoint, endpoint, thickness);
    }
}

void LevelStartMenu::drawTitle() const
{
    const char text[] = "Wombator";
    const char text2[] = "A simple Chess Engine";
    const float fontSize = 80.0f, fontSize2 = fontSize-20.0f, spacing = 0.1f;
    const float textSize = getTextSize(text,fontSize,spacing).x;
    const float textSize2 = getTextSize(text2, fontSize2, spacing).x;

    drawText(text,{ 0.5f - textSize/2.0f, 0.25f*squareSize.y }, fontSize, RAYWHITE, spacing);
    drawText(text2, {0.5f - textSize2/2.0f, 1.25f*squareSize.y}, fontSize2, RAYWHITE, spacing);

    //White King
    Resources::textureSprites.setSpritePosition({0.0f,0.0f});
    Resources::textureSprites.setPosition({0.0f,0.0f});
    Resources::textureSprites.draw();

    //Black King
    Resources::textureSprites.setSpritePosition({0.0f,Resources::textureSize.y/2.0f});
    Resources::textureSprites.setPosition({8.0f*squareSize.x,0.0f});
    Resources::textureSprites.draw();
}

void LevelStartMenu::drawButtons() const
{
    buttonPlayervsPlayer.draw(DARKGRAY, DARKGRAY, RAYWHITE);
    buttonPlayervsBot.draw(DARKGRAY, DARKGRAY, RAYWHITE);
    buttonBotvsBot.draw(DARKGRAY, DARKGRAY, RAYWHITE);
    buttonCustom.draw(DARKGRAY, DARKGRAY, RAYWHITE);
    buttonOptions.draw(DARKGRAY, DARKGRAY, RAYWHITE);
    buttonExit.draw(DARKGRAY, DARKGRAY, RAYWHITE);
}

void LevelStartMenu::drawChoiceBox() const
{
    drawRectangle({0.0f,0.0f,1.0f,1.0f}, {125,125,125,75});
    drawRectangle({3.0f*squareSize.x,3.0f*squareSize.y, 4.0f*squareSize.x, 2.0f*squareSize.y}, DARKBLUE);
    buttonWhite.draw();
    buttonBlack.draw(GRAY,DARKGRAY);

    constexpr int fontsize = 42;

    const float textsize  = getTextSize("Pick a Colour.", fontsize, 0.1f).x;
    const float xText = std::max(0.5f - 0.5f*textsize, 0.0f);
    drawText("Pick a Colour.", {xText, 3.25f*squareSize.y}, fontsize, RAYWHITE);
}