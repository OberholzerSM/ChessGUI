#pragma once
#include "ChessEngine.h"
#include "Raylib_GameHeader.h"
using namespace Raylib;

class LevelOptions: public Level
{
public:
    void load() override;
    void input() override;
    void draw() override;
    void unload() override;

private:

    void loadButtons();
    void inputButtons();
    void drawSlider(const Slider &slider) const;
    void drawTitle() const;
    void drawButtons() const;
    void drawInfoSquare(Chess::BotTypes botType) const;

    //Boardsize Variables
    const Vector2 squareSize = {Window.sizef / Window.widthf, Window.sizef / Window.heightf};   //Size of a Square on the board with relative coordinates.

    //Info-Square Variables
    bool infoSquareActive = false;
    Chess::BotTypes infoSquareBotType = Chess::OPTIMUMBOT2;

    //Buttons
    SwitchButton buttonList[2][6]{};
    Button infoButtonList[2][6]{};
    Button buttonExit{ {0.5f - 2.0f*squareSize.x,8.75f*squareSize.x}, {4.0f*squareSize.x, squareSize.y},"Exit"};

    //Sliders
    const Vector2 sliderSize = {2.5f*squareSize.x,0.5f*squareSize.y};
    Slider sliderDepthWhite{1,9,{0.25f - 0.5f*sliderSize.x,1.0f*squareSize.y,sliderSize.x,sliderSize.y}};
    Slider sliderDepthBlack{1,9,{0.75f - 0.5f*sliderSize.x,1.0f*squareSize.y,sliderSize.x,sliderSize.y}};
};
inline LevelOptions levelOptions{};