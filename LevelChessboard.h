#pragma once
#include "ChessEngine.h"
#include "Raylib_GameHeader.h"
using namespace Raylib;

class LevelChessboard: public Raylib::Level
{
public:
    void load() override;
    void input() override;
    void draw() override;
    void unload() override;

    friend class LevelStartMenu;
    friend class LevelOptions;

private:

    //Actions

     //Getting the Square the Mouse is hovering over.
    Chess::BoardPos getMouseSquare(bool flip = false) const;

    //Set the Sprite-Position off all Chesspieces according to their Board-Position.
    void setChessPieceSpritePos() const;

    //Set the Sprite-Destination off all Chesspieces according to their Board-Position.
    void setChessPieceSpriteDestination() const;

    //Pick up a Chesspiece at the start location
    void pickupPiece(const Chess::BoardPos &start) const;

    //Take a Chesspiece that was picked up at start and place it down at end.
    void placedownPiece(const Chess::BoardPos &start, const Chess::BoardPos &end);

    //Flip the Screenposition of a Vector2.
    Vector2 flipVector2(Vector2 pos) const;

    //Reset the Board.
    void reset();

    //Move a Chesspiece slowly to its destination.
    void moveChessPiece(Chess::ChessPiece *piece) const;

    //Move all the Chesspieces slowly to their destination.
    void moveChessPieces();

    //Make a Chesspiece speak a line.
    void speak(const std::string &text, Chess::ChessPiece *piece);

    //Move the Bubblesprite to its chesspiece.
    void moveBubbleSprite();

    //Have the Chesspiece comment on an invalid move.
    void commentInvalidMoves(Chess::ChessPiece *piece, const Chess::BoardPos &start, const Chess::BoardPos &end);

    //Have the Kings comment if they are in check.
    void commentCheck();

    //Pawn-Transform-Panel (ppt)

    //Assign the Pawn to be transformed.
    void pptAssignPawn(const Chess::BoardPos &start, const Chess::BoardPos &end);

    //Transform the Pawn, execute the move and advance the turn.
    void pptTransformPawn(Chess::PType type);

    //Bot Functions

    //Reset all the Bot-Variables, such that they start their search anew.
    void resetBots();

    //Make the Bots display a thinking animation while searching.
    void botThink(Chess::Bot &bot, double tStartBotSearch);

    //Set the Sprite Destination for the Bot.
    void botSetSpriteDest(Chess::Bot &bot, double tStartBotSearch);

    //Test if the Sprite of the Bot has arrived.
    void botTestSpriteDest(Chess::Bot &bot);


    //Input

    //Handles all the Mouse-Inputs.
    void inputMouse();

    //Pausing the Game via the Space-Key.
    void inputPause();

    //Input Flip Board Button.
    void inputFlipButton();

    //Input for the Dangerzone Buttons.
    void inputDangerZoneButtons();

    //Input Home Button.
    void inputHomeButton();

    //Input Reset Button.
    void inputResetButton();

    //Input for a single Arrow Button.
    void inputSingleArrowButton(Button &button, int sign);

    //Input for both Arrow buttons.
    void inputArrowButtons();

    //Check for Inputs.
    void inputPPT();

    //Yes or No Buttons for the Draw-Request.
    void inputDrawRequest();

    //Input for the Bots.
    void inputBots();


    //Drawing

    //Draw the Squares of the Board.
    void drawBoardSquares() const;

    //Draw the Lines between the squares.
    void drawBoardLines() const;

    //Draw the turnCounter in the top rigth corner.
    void drawTurnCounter() const;

    //Draw a single Chesspiece.
    void drawChessPiece(const Chess::ChessPiece *piece) const;

    //Draw every Chesspiece still alive.
    void drawChessPieces() const;

    //Draw the Movement-Radius of a single chesspiece.
    void drawMovement(const Chess::ChessPiece *piece) const;

    //Draw the Movement-Radius of all chesspieces that got selected.
    void drawChessPieceMovements() const;

    //Draw all the Buttons.
    void drawButtons() const;

    //Draw the Arrow Buttons.
    void drawArrowButtons() const;

    //Draw the Dangerzone if requested.
    void drawDangerZone() const;

    //Draw the Cursor if active
    void drawCursor() const;

    //Draw the Panel if active.
    void drawPPT() const;

    //Draw the Game-Over Banner.
    void drawGameOver() const;

    //Draw the Draw-Request.
    void drawDrawRequest() const;

    //Draw the Speechbubble
    void drawSpeechBubble();

    //Boardsize Variables
    const Vector2 squareSize = {Window.sizef / Window.widthf, Window.sizef / Window.heightf};   //Size of a Square on the board with relative coordinates.
    const Vector2 boardOrigin = {0.0f,0.0f};                                                    //Where on the screen the chessboard starts.

    //Game Variables
    bool paused = false, flipBoard = false, cursorActive = false, drawGameOverIsActive = false; //Gamestate Variable.
    bool playerControll[2] = {true,true};                                                       //Which side can be manually controlled by the player.
    const Color highlightColor1{0, 228, 48, 125},                                               //Green for white movement.
        highlightColor2{190, 33, 55, 125},                                              //Maroon for black movement.
        highlightColor3{255, 161, 0, 125},                                              //Orange for invalid movement.
        greyOutColor{125,125,125,75};                                                   //Transparent Grey to grey out the board.
    const float speed = 10.0f;                                                                  //Speed with which the chesspieces move.
    int checkCounter[2] = {0,0};                                                                //How often a King was in check. Used only for speeches.                       
    double tReset = -1.5;                                                                       //The bot should wait for some time when reset is called.

    //Pawn-Transform-Panel Variables
    bool pptActive = false;
    float pptSign = 1.0f;
    Chess::ChessPiece *pptPawn = nullptr;
    Chess::BoardPos pptStart{}, pptEnd{};
    Vector2 pptxDraw{};

    //Speechbubble
    const double duration = 1.0;                                                                //How long the speechbubble will be shown.
    double tStart = 0.0;                                                                        //To start the countdown for the speechbubble.
    bool speechBubbleActive = false;                                                            //Flag to draw the speechbubble.
    Chess::ChessPiece *speechBubbleCP = nullptr;                                                //Chesspiece currently tied to the Speechbubble.

    //Buttons

    const float d{0.35f}; //Horizontal distance parameter, so you can have two buttons next to each other.

    //Button to flip the Board
    SwitchButton buttonFlipBoard{{8.75f*squareSize.x,2.25f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f},"Flip Board"};

    //Buttons to show the Dangerzones
    SwitchButton buttonShowDangerZoneWhite{{8.75f*squareSize.x,4.25f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f},"Show Attack Range of White"};
    SwitchButton buttonShowDangerZoneBlack{{8.75f*squareSize.x,3.25f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f},"Show Attack Range of Black"};

    //To go back to the start menu,
    Button buttonHome{{(8.75f-d)*squareSize.x,6.25f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f},"HOME"};

    //To reset the board.
    Button buttonReset{{(8.75f+d)*squareSize.x,6.25f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f},"RESET"};

    //To move between previous gamestates.
    Button buttonArrowBack{{(8.75f-d)*squareSize.x,7.25f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f}},
        buttonArrowForward{{(8.75f+d)*squareSize.x,7.25f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f}};

    //To accept a yes or no answer.
    Button buttonYes{{3.5f*squareSize.x,4.25f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f},"Yes"},
        buttonNo{{4.5f*squareSize.x,4.25f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f},"No"};

    //Pawn-Transform-Panel Buttons
    SwitchButton queenButton{}, bishopButton{}, knightButton{}, rookButton{};

};
inline LevelChessboard levelChessboard{};