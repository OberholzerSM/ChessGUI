#pragma once
#include "ChessEngine.h"
#include "Raylib_GameHeader.h"
using namespace Raylib;

class LevelSetupChessboard: public Raylib::Level
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

    //Pawn-Transform-Panel (ppt)

    //Assign the Pawn to be transformed.
    void pptAssignPawn(const Chess::BoardPos &start, const Chess::BoardPos &end);

    //Transform the Pawn, execute the move and advance the turn.
    void pptTransformPawn(Chess::PType type);


    //Input Functions

    //Handles all the Mouse-Inputs.
    void inputMouse();

    //Check for Inputs.
    void inputPPT();

    //Input Turn Colour.
    void inputTurnColourButtons();

    //Input Button to start Castling mode.
    void inputCastlingStartButton();

    //Inputs when in Castling Mode.
    void inputCastlingMode();

    //Input Button to start en-passant mode.
    void inputEnPassantStartButton();

    //Inputs when in en-passant mode.
    void inputEnPassantMode();

    //Input Home Button.
    void inputHomeButton();

    //Input Reset Button.
    void inputResetButton();


    //Drawing

    //Draw the Squares of the Board.
    void drawBoardSquares() const;

    //Draw the Lines between the squares.
    void drawBoardLines() const;

    //Draw the Score in the top rigth corner.
    void drawScore() const;

    //Draw the Panel if active.
    void drawPPT() const;

    //Draw the Cursor if active
    void drawCursor() const;

    //Draw all the Buttons.
    void drawButtons() const;

    //Draw the Castling options.
    void drawCastlingMode() const;

    //Draw en-passant options.
    void drawEnPassantMode() const;

    //Draw a single Chesspiece.
    void drawChessPiece(const Chess::ChessPiece *piece) const;

    //Draw every Chesspiece still alive.
    void drawChessPieces() const;

    //Boardsize Variables
    const Vector2 squareSize = {Window.sizef / Window.widthf, Window.sizef / Window.heightf};   //Size of a Square on the board with relative coordinates.
    const Vector2 boardOrigin = {0.0f,0.0f};                                                    //Where on the screen the chessboard starts.

    //Game Variables
    bool flipBoard = false, cursorActive = false;                                               //Gamestate Variable.
    bool castlingMode = false, enpassantMode = false;                                          //To Select Castling and enpassant Options.
    const Color highlightColor1{0, 228, 48, 125},                                               //Green for white movement.
        highlightColor2{190, 33, 55, 125},                                                      //Maroon for black movement.
        highlightColor3{255, 161, 0, 125},                                                      //Orange for invalid movement.
        greyOutColor{125,125,125,75};                                                           //Transparent Grey to grey out the board.
    const float speed = 10.0f;                                                                  //Speed with which the chesspieces move.                       
    double tReset = -1.5;                                                                       //The bot should wait for some time when reset is called.

    //Pawn-Transform-Panel Variables
    bool pptActive = false;
    float pptSign = 1.0f;
    Chess::ChessPiece *pptPawn = nullptr;
    Chess::BoardPos pptStart{}, pptEnd{};
    Vector2 pptxDraw{};

    //Buttons

    const float d{0.35f}; //Horizontal distance parameter, so you can have two buttons next to each other.

    //Buttons to Switch the TurnColour.
    SwitchButton buttonTCWhite{{(8.75f-d)*squareSize.x,2.25f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f},"White"};
    SwitchButton buttonTCBlack{{(8.75f+d)*squareSize.x,2.25f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f},"Black"};

    //Button to turn on Castling Options.
    Button buttonCastling{{8.75f*squareSize.x,3.3f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f}};
    SwitchButton buttonCastleWhiteLeft{{2.0f*squareSize.x,6.0f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f}};
    SwitchButton buttonCastleWhiteRight{{5.5f*squareSize.x,6.0f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f}};
    SwitchButton buttonCastleBlackLeft{{2.0f*squareSize.x,3.0f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f}};
    SwitchButton buttonCastleBlackRight{{5.5f*squareSize.x,3.0f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f}};
    Button buttonCastleDone{{3.75f*squareSize.x,6.75f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f}, "Done"};

    //Button to turn on en-passant mode.
    Button buttonEnPassant{{8.75f*squareSize.x,4.3f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f}};

    //To go back to the start menu,
    Button buttonHome{{(8.75f-d)*squareSize.x,6.25f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f},"HOME"};

    //To reset the board.
    Button buttonReset{{(8.75f+d)*squareSize.x,6.25f*squareSize.y},{squareSize.x/2.0f,squareSize.y/2.0f},"RESET"};

    //Pawn-Transform-Panel Buttons
    SwitchButton queenButton{}, bishopButton{}, knightButton{}, rookButton{};
};
inline LevelSetupChessboard levelSetupChessboard{};