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

    void drawSlider(const Slider &slider) const;
    void drawUI() const;

    //Boardsize Variables
    const Vector2 squareSize = {Window.sizef / Window.widthf, Window.sizef / Window.heightf};   //Size of a Square on the board with relative coordinates.
    const Vector2 boardOrigin = {0.0f,0.0f};                                                    //Where on the screen the chessboard starts.

    //Buttons
    const Vector2 buttonSize = {4.0f*squareSize.x, squareSize.y};
    const float buttonX = 0.5f - buttonSize.x/2.0f;
    const float startpos = 2.5f;
    const float distance = 1.25f;

    Button buttonExit{{buttonX,(startpos+5.0f*distance)*squareSize.x},buttonSize,"Exit"};

    //Sliders
    const Vector2 sliderSize = {2.5f*squareSize.x,0.5f*squareSize.y};
    Slider sliderDepthWhite{1,9,{0.25f - 0.5f*sliderSize.x,2.0f*squareSize.y,sliderSize.x,sliderSize.y}};
    Slider sliderDepthBlack{1,9,{0.75f - 0.5f*sliderSize.x,2.0f*squareSize.y,sliderSize.x,sliderSize.y}};
};
inline LevelOptions levelOptions{};