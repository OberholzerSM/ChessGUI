#pragma once
#include "ChessEngine.h"
#include "Raylib_GameHeader.h"
using namespace Raylib;

class LevelStartMenu: public Raylib::Level
{
public:
    void load() override;
    void input() override;
    void draw() override;
    void unload() override;

private:

    void inputButtonPlayervsPlayer();
    void inputButtonPlayervsBot();
    void inputButtonBotvsBot();
    void inputButtonCustom();
    void inputButtonOption();
    void inputButtonExit();
    void inputChoiceBox();

    void drawBoardSquares() const;
    void drawBoardLines() const;
    void drawTitle() const;
    void drawButtons() const;
    void drawChoiceBox() const;

    //Boardsize Variables
    const Vector2 squareSize = {Window.sizef / Window.widthf, Window.sizef / Window.heightf};   //Size of a Square on the board with relative coordinates.
    const Vector2 boardOrigin = {0.0f,2.0f*squareSize.y};                                       //Where on the screen the chessboard starts.
    const int nSquaresHorizontal = 3;                                                           //How many squares should be drawn next to the Buttons.

    //Buttons
    const Vector2 buttonSize = {4.0f*squareSize.x, squareSize.y};
    const float buttonX = 0.5f - buttonSize.x/2.0f;
    const float startpos = 2.5f;
    const float distance = 1.25f;
    bool choiceBoxActive = true;
    Button buttonPlayervsPlayer{{buttonX,startpos*squareSize.x }, buttonSize, "Player vs. Player"};
    Button buttonPlayervsBot{{buttonX,(startpos+distance)*squareSize.x},buttonSize,"Player vs. Bot"};
    Button buttonBotvsBot{{buttonX,(startpos+2.0f*distance)*squareSize.x},buttonSize,"Bot vs. Bot"};
    Button buttonCustom{{buttonX,(startpos+3.0f*distance)*squareSize.x},buttonSize,"Setup Board"};
    Button buttonOptions{{buttonX,(startpos+4.0f*distance)*squareSize.x},buttonSize,"Options"};
    Button buttonExit{{buttonX,(startpos+5.0f*distance)*squareSize.x},buttonSize,"Exit"};

    //To accept a yes or no answer.
    const float choiceButtonSize = 0.525f;
    Button buttonWhite{{0.5f - 1.0f*squareSize.x,4.0f*squareSize.y},{choiceButtonSize*squareSize.x,choiceButtonSize*squareSize.y},"White"};
    Button buttonBlack{{0.5f + 0.5f*squareSize.x,4.0f*squareSize.y},{choiceButtonSize*squareSize.x,choiceButtonSize*squareSize.y},"Black"};

};
inline LevelStartMenu levelStartMenu{};