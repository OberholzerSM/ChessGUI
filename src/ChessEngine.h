#pragma once
#include "Random_Header.h"  //For generating random numbers. Includes <random>.
#include "Timer_Header.h"   //For Timer-Class. Includes <chrono>.
#include <iostream>         //For Console input and output.
#include <array>            //Static Array Class.
#include <vector>           //Dynamic Array Class.
#include <string>           //String Class.
#include <sstream>          //String-Stream, to more easily add to a string.
#include <algorithm>        //For the sorting-Function.
#include <thread>	        //For having multiple threads to do parallel processing.
#include <atomic>           //For having shared variables that threads can access in a safe manner.
#include <mutex>            //For freezing Threads.

namespace Chess
{
    //Counts how many seconds have passed since the Program started.
    inline Timer CLOCK{};

    //Enum ChessPiece Attributes
    enum PType
    {
        KING,
        QUEEN,
        BISHOP,
        KNIGHT,
        ROOK,
        PAWN,
    };
    enum PColour
    {
        PWHITE,
        PBLACK,
        PNONE,
    };

    //Functions to transform an enum to string
    std::string PTypeString(PType type);

    std::string PColourString(PColour colour);

    //Enum possible Bottypes
    enum BotTypes
    {
        RANDOMBOT,          //Choose at complete random.
        METROPOLISBOT,      //Choose at random, (probably) reject moves with a worse weight.
        WEIGHTEDRANDOMBOT1, //Random Piece, weighted random move.
        WEIGHTEDRANDOMBOT2, //Weighted random piece and move.
        OPTIMUMBOT1,        //Random Piece, best Move.
        OPTIMUMBOT2,        //Best Piece and Move.
    };

    //Position on the board. i and j take values 0-7. i goes left to right, j goes up to down (matrix representation).
    struct BoardPos { int i{}, j{}; };

    //A ChessMove consits of a start- and end-position and a possible pawn-transformation. Castling and en-passant can be checked manually.
    struct ChessMove
    {
        BoardPos start{}, end{};
        PType endType{};
    };

    //constexpr Variables
    constexpr int   nPieces=16,         //Number of Pieces per side.
                    nMovesMax=27,       //Max number of moves a single piece can have.
                    nMovesMaxTotal=321; //Max number of moves in a single halfturn.

    //Bitboard Functions
    typedef uint64_t u64;
    inline u64 bitboardKey[64]{};

    //Get the Bitboard keys, defined as the u64 bitboards with a single bit set to 1.
    void bitboardInitializeKeys();

    //Take a bitboard and flip the value at pos.
    void bitboardFlip(u64 &bitboard, const BoardPos &pos);

    //Returns the value of the bitboard at pos.
    bool bitboardCheck(u64 bitboard, const BoardPos &pos);

    //Forward-declare all Classes
    class ChessPiece;
    class Engine;
    class Bot;

    //Piece Square Tables, numerical Data from https://www.chessprogramming.org/Simplified_Evaluation_Function (14.03.2025)
    constexpr int PawnPST[2][8][8] =
    {
        {
            {0,0,0,0,0,0,0,0},
            {50,50,50,50,50,50,50,50},
            {10,10,20,30,30,20,10,10},
            {5,5,10,25,25,10,5,5},
            {0,0,0,0,0,0,0,0},
            {5,-5,-10,0,0,-10,-5,5},
            {5,10,10,-20,-20,10,10,5},
            {0,0,0,0,0,0,0,0}
        },
        {
            {0,0,0,0,0,0,0,0},
            {50,50,50,50,50,50,50,50},
            {10,10,20,30,30,20,10,10},
            {5,5,10,25,25,10,5,5},
            {0,0,0,0,0,0,0,0},
            {5,-5,-10,0,0,-10,-5,5},
            {5,10,10,-20,-20,10,10,5},
            {0,0,0,0,0,0,0,0}
        }
    };

    constexpr int KnightPST[2][8][8] =
    {
        {
            {-50,-40,-30,-30,-30,-30,-40,-50},
            {-40,-20,0,0,0,0,-20,-40},
            {-30,0,10,15,15,10,0,-30},
            {-30,5,15,20,20,15,5,-30},
            {-30,0,15,20,20,15,0,-30},
            {-30,5,10,15,15,10,5,-30},
            {-40,-20,0,5,5,0,-20,-40},
            {-50,-40,-30,-30,-30,-30,-40,-50}
        },
        {
            {-50,-40,-30,-30,-30,-30,-40,-50},
            {-40,-20,0,0,0,0,-20,-40},
            {-30,0,10,15,15,10,0,-30},
            {-30,5,15,20,20,15,5,-30},
            {-30,0,15,20,20,15,0,-30},
            {-30,5,10,15,15,10,5,-30},
            {-40,-20,0,5,5,0,-20,-40},
            {-50,-40,-30,-30,-30,-30,-40,-50}
        }
    };

    constexpr int BishopPST[2][8][8] =
    {
        {
            {-20,-10,-10,-10,-10,-10,-10,-20},
            {-10,0,0,0,0,0,0,-10},
            {-10,0,5,10,10,5,0,-10},
            {-10,5,5,10,10,5,5,-10},
            {-10,0,10,10,10,10,0,-10},
            {-10,10,10,10,10,10,10,-10},
            {-10,5,0,0,0,0,5,-10},
            {-20,-10,-10,-10,-10,-10,-10,-20}
        },
        {
            {-20,-10,-10,-10,-10,-10,-10,-20},
            {-10,0,0,0,0,0,0,-10},
            {-10,0,5,10,10,5,0,-10},
            {-10,5,5,10,10,5,5,-10},
            {-10,0,10,10,10,10,0,-10},
            {-10,10,10,10,10,10,10,-10},
            {-10,5,0,0,0,0,5,-10},
            {-20,-10,-10,-10,-10,-10,-10,-20}
        }
    };

    constexpr int RookPST[2][8][8] =
    {
        {
            {0,0,0,0,0,0,0,0},
            {5,10,10,10,10,10,10,5},
            {-5,0,0,0,0,0,0,-5},
            {-5,0,0,0,0,0,0,-5},
            {-5,0,0,0,0,0,0,-5},
            {-5,0,0,0,0,0,0,-5},
            {-5,0,0,0,0,0,0,-5},
            {0,0,0,5,5,0,0,0}
        },
        {
            {0,0,0,0,0,0,0,0},
            {5,10,10,10,10,10,10,5},
            {-5,0,0,0,0,0,0,-5},
            {-5,0,0,0,0,0,0,-5},
            {-5,0,0,0,0,0,0,-5},
            {-5,0,0,0,0,0,0,-5},
            {-5,0,0,0,0,0,0,-5},
            {0,0,0,5,5,0,0,0}
        }
    };

    constexpr int QueenPST[2][8][8] =
    {
        {
            {-20,-10,-10,-5,-5,-10,-10,-20},
            {-10,0,0,0,0,0,0,-10},
            {-10,0,5,5,5,5,0,-10},
            {-5,0,5,5,5,5,0,-5},
            {0,0,5,5,5,5,0,-5},
            {-10,5,5,5,5,5,0,-10},
            {-10,0,5,0,0,0,0,-10},
            {-20,-10,-10,-5,-5,-10,-10,-20}
        },
        {
            {-20,-10,-10,-5,-5,-10,-10,-20},
            {-10,0,0,0,0,0,0,-10},
            {-10,0,5,5,5,5,0,-10},
            {-5,0,5,5,5,5,0,-5},
            {0,0,5,5,5,5,0,-5},
            {-10,5,5,5,5,5,0,-10},
            {-10,0,5,0,0,0,0,-10},
            {-20,-10,-10,-5,-5,-10,-10,-20}
        }
    };

    constexpr int KingPST[2][8][8] =
    {
        {
            {-20,-10,-10,-5,-5,-10,-10,-20},
            {-10,0,0,0,0,0,0,-10},
            {-10,0,5,5,5,5,0,-10},
            {-5,0,5,5,5,5,0,-5},
            {0,0,5,5,5,5,0,-5},
            {-10,5,5,5,5,5,0,-10},
            {-10,0,5,0,0,0,0,-10},
            {-20,-10,-10,-5,-5,-10,-10,-20}
        },
        {
            {-50,-40,-30,-20,-20,-30,-40,-50},
            {-30,-20,-10,0,0,-10,-20,-30},
            {-30,-10,20,30,30,20,-10,-30},
            {-30,-10,30,40,40,30,-10,-30},
            {-30,-10,30,40,40,30,-10,-30},
            {-30,-10,20,30,30,20,-10,-30},
            {-30,-30,0,0,0,0,-30,-30},
            {-50,-30,-30,-30,-30,-30,-30,-50}
        }
    };

    //Piece-Values KING,QUEEN,BISHOP,KNIGHT,ROOK,PAWN
    constexpr int pieceValue[2][6] = {{20000,900,330,320,500,100},{20000,900,330,320,500,100}};

    //Mobility Score.

    constexpr int RookMobility[2][15] = 
    {
        {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140}, 
        {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140}
    };
    
    constexpr int KnightMobility[2][9] = 
    {
        {0,10,20,30,40,50,60,70,80},
        {0,10,20,30,40,50,60,70,80}
    };

    constexpr int BishopMobility[2][14] = 
    {
        {0,10,20,30,40,50,60,70,80,90,100,110,120,130},
        {0,10,20,30,40,50,60,70,80,90,100,110,120,130}
    };

    constexpr int QueenMobility[2][28] = 
    {
        {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,210,220,230,240,250,260,270},
        {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,210,220,230,240,250,260,270}
    };

    constexpr int KingMobility[2][9] =
    {
        {0,10,20,30,40,50,60,70,80},
        {0,10,20,30,40,50,60,70,80}
    };

    //Pawn Score.

    constexpr int PawnDoubled[2] = {-50,-50};   //Doubled Pawn:  Two friendly Pawns on the same file.
    constexpr int PawnIsolated[2] = {-50,-50};  //Isolated Pawn: No friendly Pawns on the neighbouring files.
    constexpr int PawnBlocked[2] = {-50,-50};   //Blocked Pawn:  Pawn cannot move.

    //King Safety
    constexpr int KingCheck[2] = {-2000,-2000};
}

//Class representing a single Chesspiece.
class Chess::ChessPiece
{
public:

    ChessPiece(const BoardPos &start, PType type, PColour colour, Engine &engine):
        startPos{start}, type{type}, colour{colour}, engine{engine}{}

    //Game-related Functions

    //Places the ChessPiece at newpos. Automatically transforms Pawns back.
    void place(const BoardPos &newpos);

    //Removes itself from the board.
    void kill();

    //Execute a Chessmove.
    void move(const ChessMove &move);

    //Updates the movementPseudo bitboard and stores all available pseudolegal Moves in the movesPseudo List.
    void updatePseudolegalMovement();

    //Debug: Print the Movement of this Chesspiece.
    void printMovement() const;

    //Debug: Identify a chesspiece.

    void identify() const;

    //Piece-specific
    
    //Teleports the Rook to its castling position.
    void teleportRook();

    //Checks if the Piece is threatened by another piece through pseudolegal moves.
    bool checkCheck() const;

    //Transforms the ChessPiece to newtype and sets the corresponding flag.
    void transformPawn(PType newtype);

protected:
    //Test for pseudolegal moves.

    //Checks which squares are pseudolegal for the King.
    void KingPseudoLegal();

    /*Checks along a straight-line defined by signX and signY to what squares a ChessPiece can move.
    Directions are defined via Matrix-Representation: Right and Bottom are positive.
    Example: checkDirection(+1, 0) checks all the squares right of the ChessPiece.
    Example: checkDirection(-1,+1) checks all the squares alongside the leftbottom diagonal.*/
    void checkDirection(int signX, int signY);

    //Checks the four diagonal directions.
    void BishopPseudoLegal();

    //Checks the four horizontal direction.
    void RookPseudoLegal();

    //Checks the four horizontal and diagonal directions.
    void QueenPseudoLegal();

    //Checks if a Knight can jump to the Position at (k,l).
    void checkKnightSquare(int k, int l);

    //Checks all available Knight Positions.
    void KnightPseudoLegal();

    //Checks if a Pawn can Capture in the Direction signX. signY is determined by the Colour of the Pawn.
    void checkPawnCapture(int signX, int signY);

    //Checks if the Pawn can move two squares forward. sign is determined by its colour.
    void checkPawnTwoForward(int sign);

    //Checks all the squares a Pawn could move to.
    void PawnPseudoLegal();

public:
    //Game Variables
    PType type;                                                                 //Type of ChessPiece. May change through Pawn-Transform.
    const PColour colour;                                                       //Game-colour of the Chesspiece. Does not change.
    bool alive{false}, moved{false}, enpassant{false}, transformed{false};      //Chesspiece game flags.
    BoardPos pos;                                                               //Position on the board.
    const BoardPos startPos;                                                    //Initial Position.
    u64 movement=0, movementPseudo=0;                                           //Movement-Bitboard. If 1, one can move there.
    int nMoves=0, nMovesPseudo=0;                                               //Number of available ChessMoves.
    ChessMove moveList[nMovesMax]{}, moveListPseudo[nMovesMax]{};               //List of available ChessMoves.
    Engine &engine;                                                             //Reference to the engine the chesspiece is tied to.

    //GUI Variables
    bool selected = false;                                                      //If the chesspiece has been selected by the player.
    bool showMovement = false;                                                  //Highlight the squares the Chesspiece can move to.
    float drawX = 0.0f, drawY = 0.0f;                                           //The position of the sprite on the screen.
    float drawDestinationX = 0.0f, drawDestinationY = 0.0f;                     //The position the sprite is moving towards.
};

//Class to handle all the Board-logic. Represents a single Chessboard.
class Chess::Engine
{
public:
    friend class ChessPiece;
    friend class Bot;

    void initialize();

    //Debug

    //Print the Chessboard with all the ChessPieces.
    void printChessboard() const;

    //Print the Dangerzones (pseudolegal Movement reach) for both colours.
    void printDangerZones() const;

    //Print the Attackzones (which squares are threatened) for both colours.
    void printAttackZones() const;

    //Print a given Chessmove.
    void printMove(const ChessMove &move) const;

    //Prints the letter accociated with the ChessType
    char getPTypeLetter(PType type) const;


    //Saving and Loading the Boardstate
    struct BoardState;

    //Saves the current BoardState at position.
    void saveBoardState(std::size_t position);

    //Loads the BoardState saved at position.
    void loadBoardState(std::size_t position);

    //Only load what's necessary to unmake a pseudolegal move for generating legal moves. Does not affect listAlive and listAvailable.
    void loadTestBoardState(std::size_t position);

    //Tests if two Boardstate represent the same Gamestate, ignoring the TurnCounter.
    bool compareBoardStates(const BoardState &state1, const BoardState &state2);


    //Engine update BoardState

    //Updates the movementPseudo bitboard for all Chesspieces and also updates the Dangerzones.
    void updatePseudoLegalMovement();

    //Checks for every move if it's legal and stores it in moveListLegal if that is the case.
    //Also updates piecesListAvailable and piecesListAlive.
    void updateLegalMovement();

    //Checks for late-game conditions and sets the corresponding Flag. Important for Bots.
    void checkLateGame();

    //Checks for game-ending conditions and sets the corresponding Flags.
    void checkGameOver();

    //Updates the boardstate.
    void updateMovement();

    //Update the Attack-Zone. Only used for the GUI.
    void updateAttackZone();

    //To execute a ChessMove
private:
    //Check if a Move is a valid Castling-Move. Should be called before one executes the move.
    bool checkCastlingRights(const ChessMove &move) const;

    //Teleport the appropriate Rook if a King castles from Start to End.
    void updateRooks(const BoardPos &Start, const BoardPos &End);

    //Reset all en-passant Flags. Must be done before one executes a move.
    void resetEnPassantFlags();
public:
    //Execute a pseudolegal Chessmove. Returns true if the Move was legal.
    bool makeMove(const ChessMove &move);

    //Advance the turn after a Move has been made.
    void advanceTurn();

    //FEN-loading

    //Searches for the next available dead Pawn with colour, transforms it to type and places it at pos.
    void placeTransformedPawn(const BoardPos &pos, PType type, PColour colour) const;

    //Takes a ChessPiece and returns its accociated letter.
    char getPieceLetter(const ChessPiece *piece) const;

    //Takes a FEN-String, loads the Boardstate and saves it at the current turnCounter.
    void loadFEN(const char *FEN);

    //Turns the current Boardstate into a FEN.
    std::string getFEN() const;
    

    //Board Weight

    //Get the correct Piece-Square-Table value for a given position and chesspiece.
    int getPositionWeight(const BoardPos &pos, PType type, PColour colour) const;

    //Get the Weight of the current Board.
    int getBoardWeight() const;

    //Get the estimated weight for a move with a given depth.
    int getMoveWeight(int depth, const ChessMove &move) const;

    //Advance the Turn for Weight calculations without calculating legal moves.
    void advancePseudoTurn();

public:

    //General Game Variables
    std::size_t turnCounter = 0, turnCounterStart = 0, turnsUntilDrawCounter = 0, maxTurns = 0;
    bool isdraw = false, checkmate[2] = {false,false}, lateGame = false, requestDraw = false;
    PColour turnColour = PWHITE, turnColourOld = PWHITE;
    std::string checkmateText{}, drawText{};

    //ChessPiece-Variables
    int nAlive[2]{}, nAvailable[2]{}, nMovesLegal[2]{}, nMovesPseudo[2]{};              //nAvailable: How many pieces have legal moves left.
    ChessPiece *piecesListAvailable[2][nPieces]{}, *piecesListAlive[2][nPieces]{};      //piecesListAvailable: Pieces that have legal moves left.
    ChessMove moveListLegal[2][nMovesMaxTotal]{}, moveListPseudo[2][nMovesMaxTotal]{};  //List of all available moves.

    //Chessboard holds a pointer to every Chesspiece at BoardPos {i,j}. Stores a nullptr if no ChessPiece is present there.
    ChessPiece *board[8][8]{};

    //Dangerzone Bitboard: The (pseudolegal) movement-reach of each Player for all Pieces.
    u64 dangerZone[2]{};

    //The squares that are within the attack-range. Different from dangerZone because of Pawns.
    u64 attackZone[2]{};

    //Declaration of all Chesspieces
    ChessPiece  
        kingWhite{{4,7},KING,PWHITE,*this},      kingBlack{{4,0},KING,PBLACK,*this},
        queenWhite{{3,7},QUEEN,PWHITE,*this},    queenBlack{{3,0},QUEEN,PBLACK,*this},

        bishopWhiteL{{2,7},BISHOP,PWHITE,*this}, bishopBlackL{{2,0},BISHOP,PBLACK,*this},
        bishopWhiteR{{5,7},BISHOP,PWHITE,*this}, bishopBlackR{{5,0},BISHOP,PBLACK,*this},

        knightWhiteL{{1,7},KNIGHT,PWHITE,*this}, knightBlackL{{1,0},KNIGHT,PBLACK,*this},
        knightWhiteR{{6,7},KNIGHT,PWHITE,*this}, knightBlackR{{6,0},KNIGHT,PBLACK,*this},

        rookWhiteL{{0,7},ROOK,PWHITE,*this},     rookBlackL{{0,0},ROOK,PBLACK,*this},
        rookWhiteR{{7,7},ROOK,PWHITE,*this},     rookBlackR{{7,0},ROOK,PBLACK,*this},

        pawnWhite0{{0,6},PAWN,PWHITE,*this},     pawnWhite1{{1,6},PAWN,PWHITE,*this}, pawnWhite2{{2,6},PAWN,PWHITE,*this}, pawnWhite3{{3,6},PAWN,PWHITE,*this},
        pawnWhite4{{4,6},PAWN,PWHITE,*this},     pawnWhite5{{5,6},PAWN,PWHITE,*this}, pawnWhite6{{6,6},PAWN,PWHITE,*this}, pawnWhite7{{7,6},PAWN,PWHITE,*this},

        pawnBlack0{{0,1},PAWN,PBLACK,*this},     pawnBlack1{{1,1},PAWN,PBLACK,*this}, pawnBlack2{{2,1},PAWN,PBLACK,*this}, pawnBlack3{{3,1},PAWN,PBLACK,*this},
        pawnBlack4{{4,1},PAWN,PBLACK,*this},     pawnBlack5{{5,1},PAWN,PBLACK,*this}, pawnBlack6{{6,1},PAWN,PBLACK,*this}, pawnBlack7{{7,1},PAWN,PBLACK,*this};

    //List with a Pointer to every ChessPiece.
    ChessPiece *const piecesList[2][nPieces]
        {
            {
                &kingWhite,
                &queenWhite,
                &bishopWhiteL,
                &bishopWhiteR,
                &knightWhiteL,
                &knightWhiteR,
                &rookWhiteL,
                &rookWhiteR,
                &pawnWhite0,
                &pawnWhite1,
                &pawnWhite2,
                &pawnWhite3,
                &pawnWhite4,
                &pawnWhite5,
                &pawnWhite6,
                &pawnWhite7,
            },

            {
                &kingBlack,
                &queenBlack,
                &bishopBlackL,
                &bishopBlackR,
                &knightBlackL,
                &knightBlackR,
                &rookBlackL,
                &rookBlackR,
                &pawnBlack0,
                &pawnBlack1,
                &pawnBlack2,
                &pawnBlack3,
                &pawnBlack4,
                &pawnBlack5,
                &pawnBlack6,
                &pawnBlack7,
            },
        };

    //BoardState Struct.
    struct BoardState
    {
        //General Game Variables
        std::size_t turnCounter = 0, turnsUntilDrawCounter = 0;
        bool isdraw = false, checkmate[2] = {false,false}, requestDraw = false, lateGame = false;
        PColour turnColour = PWHITE;
        std::string checkmateText{}, drawText{};

        //Engine ChessPiece-Variables
        int nAlive[2]{}, nAvailable[2]{};                                               //nAvailable: How many pieces have legal moves left.
        ChessPiece *piecesListAvailable[2][nPieces]{}, *piecesListAlive[2][nPieces]{};  //piecesListAvailable: Pieces that have legal moves left.

        //Chessboard Variables
        ChessPiece *board[8][8]{};
        u64 dangerZone[2]{};

        //Chesspiece Variables
        PType type[2][nPieces]{};
        bool alive[2][nPieces]{false}, moved[2][nPieces]{false}, enpassant[2][nPieces]{false}, transformed[2][nPieces]{false};
        BoardPos pos[2][nPieces]{};
        u64 movement[2][nPieces]{}, movementPseudo[2][nPieces]{};
        int nMoves[2][nPieces]{}, nMovesPseudo[2][nPieces]{};
        ChessMove moveList[2][nPieces][nMovesMax]{}, moveListPseudo[2][nPieces][nMovesMax]{};                         
    };

    //List to Save the Boardstates in.
    std::vector<BoardState> boardStateList{std::vector<BoardState>(10)};
    std::size_t boardStateListSize{10};
};

//Bot-Class: Chooses what moves to play for a given colour.
class Chess::Bot
{
public:
    Bot(PColour colour): botColour{colour}{ updateThreadSize(); }

    const PColour botColour;
    BotTypes botType = OPTIMUMBOT2;
    int depth = 5, nThreadsBot = 1;
    ChessMove nextMove{};
    std::vector<ChessMove> nextMoveList{  };
    std::vector<int> nextMoveWeightList{};
    bool spriteMoved = false, spriteArrived = false;
    std::atomic<bool> searching{false}, moveChosen{false};

    //Set the move depending on the bottype and store it in nextMove.
    void generateMove();

    //Executes the move stored in nextMove and advance the turn.
    void executeMove() const;

    //Reset all Variables.
    void reset();

    //Adjust ThreadSize.
    void updateThreadSize();

    //Distribute the moves among Threads and find the best move.
    void findOptimumMove(const std::vector<ChessMove> &moveList);

private:

    //Return the index of a random piece from piecesListAvailable.
    int chooseRandomPiece() const;

    //Generate a random legal move.
    void generateMoveRandomBot();

    //Pick a move and reject worse moves.
    void generateMoveMetropolisBot();

    //Pick a random piece and a weighted random move.
    void generateMoveWeightedRandomBot1();

    //Pick a weighted random piece and move.
    void generateMoveWeightedRandomBot2();

    //Get the weight of a move via a thread.
    void setMoveWeightThread(const ChessMove &move, std::size_t threadID);

    //Pick a random piece and its best move.
    void generateMoveOptimumBot1();

    //Pick the best piece and move.
    void generateMoveOptimumBot2();

    //Find the best Move according to negamax.
    void findNegamaxMove(std::vector<ChessMove> moveList, std::size_t threadID);
};

namespace Chess
{
    //The Main Engine that runs the game
    inline Engine mainEngine;

    void initialize();

    //Weight Calculations

    //Negamax search.
    int negamax(int depth, int saveCounter, int alpha, int beta, const std::vector<ChessMove> &moveList, std::unique_ptr<Engine> &engine);

    //Thread Functions

    //Delete Threads from threadlist that have set threadDone = true.
    void cleanUpThreads();

    //Stops all active threads.
    void stopAllThreads();

    //Thread Variables
    inline int nThreads = std::thread::hardware_concurrency();  //How many Threads should be used.
    inline std::atomic<bool> stopThread{false};		            //Shared flag to signal all threads to stop.
    inline std::vector<std::thread> threadList{};		        //List of all threads currently running.
    inline std::vector<bool> threadDone{};				        //List of bools to signal if a Thread has finished.
    inline std::mutex mtx, mtx2, mtx3;		                    //Mutex-Locks to safely access thread-Variables.

    //Bots
    inline Bot botList[2]{ PWHITE,PBLACK };
}