#include "ChessEngine.h"
using namespace Chess;

//Bitboard Functions

void Chess::bitboardInitializeKeys()
{
    u64 n = 1;
    for(int i=0; i<8; i++)
    {
        for(int j=0; j<8; j++)
        {
            bitboardKey[i+(8*j)] = n;
            n = 2*n;
        }
    }
}

void Chess::bitboardFlip(u64 &bitboard, const BoardPos &pos)
{
    const int k = pos.i + (8*pos.j);
    bitboard = (bitboard ^ bitboardKey[k]);
}

bool Chess::bitboardCheck(u64 bitboard, const BoardPos &pos)
{
    const int k = pos.i + (8*pos.j);
    return (bitboard & bitboardKey[k]);
}

//Enum Identity Functions

std::string Chess::PTypeString(PType type)
{
    switch(type)
    {
    case KING:
        return "King";

    case QUEEN:
        return "Queen";

    case BISHOP:
        return "Bishop";

    case KNIGHT:
        return "Knight";

    case ROOK:
        return "Rook";

    case PAWN:
        return "Pawn";

    }
}

std::string Chess::PColourString(PColour colour)
{
    switch(colour)
    {
    case PWHITE:
        return "white";

    case PBLACK:
        return "black";

    case PNONE:
        return "no colour";
    }
}

//ChessPiece Functions

void ChessPiece::place(const BoardPos &newpos)
{
    pos = newpos;
    moved = ( pos.i == startPos.i && pos.j == startPos.j ) ? false : true;
    enpassant = false;
    alive = true;
    engine.board[pos.i][pos.j] = this;

    if(transformed)
    {
        type = PAWN;
        transformed = false;
    }
}

void ChessPiece::kill()
{
    movementPseudo = movementPseudo & 0;
    engine.board[pos.i][pos.j] = nullptr;
    alive = false;
}

void ChessPiece::move(const ChessMove &move)
{
    const BoardPos start = move.start;
    const BoardPos end = move.end;

    if(type==PAWN)
    {
        engine.turnsUntilDrawCounter = 0;

        //en passant
        if(engine.board[end.i][end.j] == nullptr && abs(start.i - end.i) == 1)
            engine.board[end.i][start.j]->kill();

        //Double Move
        if(abs(start.j - end.j) == 2)
            enpassant = true;
    }

    if(engine.board[end.i][end.j] != nullptr)
    {
        engine.board[end.i][end.j]->kill();
        engine.turnsUntilDrawCounter = 0;
    }

    engine.board[start.i][start.j] = nullptr;
    engine.board[end.i][end.j] = this;
    pos = end;

    moved = true;
}

void ChessPiece::printMovement() const
{
    std::cout << "\nPseudolegal:";
    std::cout << "\n+---+---+---+---+---+---+---+---+\n";
    for(int j=0; j<8; j++)
    {
        for(int i=0; i<8; i++)
        {
            if(i==0)
                std::cout << '|';
            std::cout << ' ' << bitboardCheck(movementPseudo, {i,j}) << " |";
            if(i==7)
                std::cout << ' ' << 8-j;
        }
        std::cout << "\n+---+---+---+---+---+---+---+---+\n";
    }
    for(int i=0; i<8; i++)
    {
        std::cout << "  " << (char)('a' + i) << " ";
    }
    std::cout << "\n\n";

    std::cout << "Legal:";
    std::cout << "\n+---+---+---+---+---+---+---+---+\n";
    for(int j=0; j<8; j++)
    {
        for(int i=0; i<8; i++)
        {
            if(i==0)
                std::cout << '|';
            std::cout << ' ' << bitboardCheck(movement, {i,j}) << " |";
            if(i==7)
                std::cout << ' ' << 8-j;
        }
        std::cout << "\n+---+---+---+---+---+---+---+---+\n";
    }
    for(int i=0; i<8; i++)
    {
        std::cout << "  " << (char)('a' + i) << " ";
    }
    std::cout << "\n\n";
}

void ChessPiece::identify() const
{
    const std::string typeName = PTypeString(type);
    const std::string colourName = PColourString(colour);
    std::cout << "I am a " << colourName << ' ' << typeName << '\n';
}

//ChessPiece Piece-specific

void ChessPiece::teleportRook()
{
    const int end_i = (pos.i==0) ? 3 : 5;

    engine.board[pos.i][pos.j] = nullptr;
    engine.board[end_i][pos.j] = this;
    pos = {end_i,pos.j};
    moved = true;
}

bool ChessPiece::checkCheck() const
{
    return (colour == PWHITE) ? bitboardCheck(engine.dangerZone[1], pos) : bitboardCheck(engine.dangerZone[0], pos);
}

void ChessPiece::transformPawn(PType newtype)
{
    if(type != newtype)
    {
        transformed = true;
        type = newtype;
    }
}

//ChessPiece Pseudo-Legal Move-generation

void ChessPiece::KingPseudoLegal()
{
    //Basic Movement
    bool treason{false};
    bool nomove{false};

    const int start_i = std::max(0, pos.i - 1);
    const int end_i = std::min(7, pos.i + 1);
    const int start_j = std::max(0, pos.j - 1);
    const int end_j = std::min(7, pos.j + 1);

    for(int i = start_i; i <= end_i; i++)
    {
        for(int j = start_j; j <= end_j; j++)
        {
            nomove = (i == pos.i) && (j == pos.j);

            treason = false;
            ChessPiece *piece = engine.board[i][j];
            if(piece != nullptr)
                treason = (piece->colour == colour);

            if(!nomove && !treason)
            {
                bitboardFlip(movementPseudo, {i,j});
                moveListPseudo[nMovesPseudo++]= {pos,{i,j},type};
                engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},type};
            }
        }
    }

    //Casteling
    const int king_j = (colour==PWHITE) ? 7 : 0;
    if(!moved && pos.i == 4 && pos.j == king_j)
    {
        bool freepathLeft{false}, freepathRight{false};
        freepathLeft  = (engine.board[3][pos.j] == nullptr) && (engine.board[2][pos.j] == nullptr) && (engine.board[1][pos.j] == nullptr) && (engine.board[0][pos.j] != nullptr);
        freepathRight = (engine.board[5][pos.j] == nullptr) && (engine.board[6][pos.j] == nullptr) && (engine.board[7][pos.j] != nullptr);

        if(freepathRight)
        {
            if(engine.board[7][pos.j]->type == ROOK && engine.board[7][pos.j]->moved == false)
            {
                bitboardFlip(movementPseudo, {pos.i+2,pos.j});
                moveListPseudo[nMovesPseudo++] = {pos,{pos.i+2,pos.j},type};
                engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{pos.i+2,pos.j},type};
            }
        }

        if(freepathLeft)
        {
            if(engine.board[0][pos.j]->type == ROOK && engine.board[0][pos.j]->moved == false)
            {
                bitboardFlip(movementPseudo, {pos.i-2,pos.j});
                moveListPseudo[nMovesPseudo++] = {pos,{pos.i-2,pos.j},type};
                engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{pos.i-2,pos.j},type};
            }
        }
    }
}

void ChessPiece::checkDirection(int signX, int signY)
{
    int i = pos.i + signX;
    int j = pos.j + signY;

    while(i >= 0 && i < 8 && j >= 0 && j < 8)
    {
        const ChessPiece *piece = engine.board[i][j];
        if(piece == nullptr)
        {
            bitboardFlip(movementPseudo, {i,j});
            moveListPseudo[nMovesPseudo++]= {pos,{i,j},type};
            engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},type};
        }
        else if(piece->colour != colour)
        {
            bitboardFlip(movementPseudo, {i,j});
            moveListPseudo[nMovesPseudo++]= {pos,{i,j},type};
            engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},type};
            break;
        }
        else
            break;

        i += signX;
        j += signY;
    }
}

void ChessPiece::BishopPseudoLegal()
{
    checkDirection(1, 1);
    checkDirection(-1, 1);
    checkDirection(1, -1);
    checkDirection(-1, -1);
}

void ChessPiece::RookPseudoLegal()
{
    checkDirection(1, 0);
    checkDirection(0, 1);
    checkDirection(-1, 0);
    checkDirection(0, -1);
}

void ChessPiece::QueenPseudoLegal()
{
    RookPseudoLegal();
    BishopPseudoLegal();
}

void ChessPiece::checkKnightSquare(int k, int l)
{
    const int i = pos.i+k;
    const int j = pos.j+l;

    if(i >= 0 && i < 8 && j >= 0 && j < 8)
    {
        const ChessPiece *piece = engine.board[i][j];
        if(piece == nullptr || piece->colour != colour)
        {
            bitboardFlip(movementPseudo, {i,j});
            moveListPseudo[nMovesPseudo++]= {pos,{i,j},type};
            engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},type};
        }
    }
}

void ChessPiece::KnightPseudoLegal()
{
    checkKnightSquare(+2,-1);
    checkKnightSquare(+1,-2);
    checkKnightSquare(-1,-2);
    checkKnightSquare(-2,-1);
    checkKnightSquare(-2,+1);
    checkKnightSquare(-1,+2);
    checkKnightSquare(+1,+2);
    checkKnightSquare(+2,+1);
}

void ChessPiece::checkPawnCapture(int signX, int signY)
{
    const int i = pos.i+signX;
    const int j = pos.j+signY;

    if(i >= 0 && i < 8 && j >= 0 && j < 8)
    {
        //Capture
        const ChessPiece *piece = engine.board[i][j];
        if(piece != nullptr && piece->colour != colour)
        {
            //Pawn-Transform
            if(j==0 || j==7)
            {
                bitboardFlip(movementPseudo, {i,j});

                moveListPseudo[nMovesPseudo++] = {pos,{i,j},QUEEN};
                moveListPseudo[nMovesPseudo++] = {pos,{i,j},BISHOP};
                moveListPseudo[nMovesPseudo++] = {pos,{i,j},KNIGHT};
                moveListPseudo[nMovesPseudo++] = {pos,{i,j},ROOK};

                engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},QUEEN};
                engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},BISHOP};
                engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},KNIGHT};
                engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},ROOK};
            }
            else
            {
                bitboardFlip(movementPseudo, {i,j});
                moveListPseudo[nMovesPseudo++]= {pos,{i,j},type};
                engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},type};
            }
        }

        //En passant
        const ChessPiece *enpassantPiece = engine.board[i][pos.j];
        if(enpassantPiece != nullptr && piece == nullptr && enpassantPiece->type == PAWN && enpassantPiece->enpassant)
        {
            bitboardFlip(movementPseudo, {i,j});
            moveListPseudo[nMovesPseudo++]= {pos,{i,j},type};
            engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},type};
        }

    }
}

void ChessPiece::checkPawnTwoForward(int sign)
{
    //Two forward from Start
    const int i = pos.i;
    const int j = (sign==-1) ? 6 : 1;

    if(pos.j == j && engine.board[i][j+sign] == nullptr && engine.board[i][j+2*sign] == nullptr)
    {
        bitboardFlip(movementPseudo, {i,j+2*sign});
        moveListPseudo[nMovesPseudo++]= {pos,{i,j+2*sign},type};
        engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j+2*sign},type};
    }
}

void ChessPiece::PawnPseudoLegal()
{
    //The Direction the Pawn moves in
    const int sign = (colour==PWHITE) ? -1 : +1;

    //Basic Movement
    const int i = pos.i;
    const int j = pos.j+sign;

    if(j > 0 && j < 7 && engine.board[i][j] == nullptr)
    {
        bitboardFlip(movementPseudo, {i,j});
        moveListPseudo[nMovesPseudo++] = {pos,{i,j},type};
        engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},type};
    }

    //Pawn-Transform
    if((j==0 || j==7) && engine.board[i][j] == nullptr)
    {
        bitboardFlip(movementPseudo, {i,j});

        moveListPseudo[nMovesPseudo++] = {pos,{i,j},QUEEN};
        moveListPseudo[nMovesPseudo++] = {pos,{i,j},BISHOP};
        moveListPseudo[nMovesPseudo++] = {pos,{i,j},KNIGHT};
        moveListPseudo[nMovesPseudo++] = {pos,{i,j},ROOK};

        engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},QUEEN};
        engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},BISHOP};
        engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},KNIGHT};
        engine.moveListPseudo[colour][engine.nMovesPseudo[colour]++] = {pos,{i,j},ROOK};
    }

    //Two forward from Start
    checkPawnTwoForward(sign);

    //Capture
    checkPawnCapture(1, sign);
    checkPawnCapture(-1, sign);
}

void ChessPiece::updatePseudolegalMovement()
{
    //Reset Pseudolegal Movement
    nMovesPseudo = 0;
    movementPseudo &= 0;

    if(alive)
    {
        switch(type)
        {
        case KING:
            KingPseudoLegal();
            break;

        case QUEEN:
            QueenPseudoLegal();
            break;

        case BISHOP:
            BishopPseudoLegal();
            break;

        case KNIGHT:
            KnightPseudoLegal();
            break;

        case ROOK:
            RookPseudoLegal();
            break;

        case PAWN:
            PawnPseudoLegal();
            break;
        }
    }
}


//Engine-Functions

//Debug

void Engine::printChessboard() const
{
    std::cout << "\n+---+---+---+---+---+---+---+---+\n";
    for(int j=0; j<8; j++)
    {
        for(int i=0; i<8; i++)
        {
            if(i==0)
                std::cout << '|';
            std::cout << ' ' << getPieceLetter(board[i][j]) << " |";
            if(i==7)
                std::cout << ' ' << 8-j;
        }
        std::cout << "\n+---+---+---+---+---+---+---+---+\n";
    }
    for(int i=0; i<8; i++)
    {
        std::cout << "  " << (char)('a' + i) << " ";
    }
    std::cout << "\n\n";
    std::cout << "FEN: " << getFEN() << '\n';

    std::cout << '\n';
}

void Engine::printDangerZones() const
{
    std::cout << "\nWhite Dangerzone:";
    std::cout << "\n+---+---+---+---+---+---+---+---+\n";
    for(int j=0; j<8; j++)
    {
        for(int i=0; i<8; i++)
        {
            if(i==0)
                std::cout << '|';
            std::cout << ' ' << bitboardCheck(dangerZone[0], {i,j}) << " |";
            if(i==7)
                std::cout << ' ' << 8-j;
        }
        std::cout << "\n+---+---+---+---+---+---+---+---+\n";
    }
    for(int i=0; i<8; i++)
    {
        std::cout << "  " << (char)('a' + i) << " ";
    }
    std::cout << "\n\n";

    std::cout << "Black Dangerzone:";
    std::cout << "\n+---+---+---+---+---+---+---+---+\n";
    for(int j=0; j<8; j++)
    {
        for(int i=0; i<8; i++)
        {
            if(i==0)
                std::cout << '|';
            std::cout << ' ' << bitboardCheck(dangerZone[1], {i,j}) << " |";
            if(i==7)
                std::cout << ' ' << 8-j;
        }
        std::cout << "\n+---+---+---+---+---+---+---+---+\n";
    }
    for(int i=0; i<8; i++)
    {
        std::cout << "  " << (char)('a' + i) << " ";
    }
    std::cout << "\n\n";
}

char Engine::getPTypeLetter(PType type) const
{
    switch(type)
    {
    case KING:
        return 'K';

    case QUEEN:
        return 'Q';

    case BISHOP:
        return 'B';

    case KNIGHT:
        return 'N';

    case ROOK:
        return 'R';

    case PAWN:
        return 'P';

    default:
        return 'X';
    }
}

void Engine::printMove(const ChessMove &move) const
{
    const BoardPos start = move.start, end = move.end;
    PType startType;
    const PType endType = move.endType;

    if(board[start.i][start.j] != nullptr)
        startType = board[start.i][start.j]->type;
    else if(board[end.i][end.j] != nullptr)
        startType = board[end.i][end.j]->type;
    else
        startType = endType;

    const char startLetter = start.i + 'a', endLetter = move.end.i + 'a';

    if(startType != endType)
        std::cout << startLetter << 8-start.j << endLetter << 8-end.j << getPTypeLetter(endType);
    else
        std::cout << startLetter << 8-start.j << endLetter << 8-end.j;
    
}

//Saving and Loading the Boardstate

void Engine::saveBoardState(std::size_t position)
{
    //Increase size if necessary
    if(position >= boardStateListSize)
    {
        boardStateListSize += 200;
        boardStateList.resize(boardStateListSize);
    }

    BoardState &boardState = boardStateList[position];

    //Save General Game Variables
    boardState.turnCounter = turnCounter;
    boardState.turnsUntilDrawCounter = turnsUntilDrawCounter;
    boardState.isdraw = isdraw;
    boardState.requestDraw = requestDraw;
    boardState.lateGame = lateGame;
    boardState.turnColour = turnColour;
    boardState.checkmateText = checkmateText;
    boardState.drawText = drawText;

    //Saving Lists
    for(int l=0; l<2; l++)
    {
        boardState.checkmate[l] = checkmate[l];
        boardState.nAlive[l] = nAlive[l];
        boardState.nAvailable[l] = nAvailable[l];
        boardState.dangerZone[l] = dangerZone[l];

        for(int k=0; k<nPieces; k++)
        {
            boardState.piecesListAlive[l][k] = piecesListAlive[l][k];
            boardState.piecesListAvailable[l][k] = piecesListAvailable[l][k];

            //Saving Board
            const BoardPos posOld = boardState.pos[l][k];
            const BoardPos posNew = piecesList[l][k]->pos;

            boardState.board[posOld.i][posOld.j] = board[posOld.i][posOld.j];
            boardState.board[posNew.i][posNew.j] = board[posNew.i][posNew.j];

            //Saving ChessPiece Variables
            const ChessPiece *piece = piecesList[l][k];

            boardState.type[l][k] = piece->type;
            boardState.alive[l][k] = piece->alive;
            boardState.moved[l][k] = piece->moved;
            boardState.enpassant[l][k] = piece->enpassant;
            boardState.transformed[l][k] = piece->transformed;

            boardState.pos[l][k] = piece->pos;
            boardState.movement[l][k] = piece->movement;
            boardState.movementPseudo[l][k] = piece->movementPseudo;
            boardState.nMoves[l][k] = piece->nMoves;
            boardState.nMovesPseudo[l][k] = piece->nMovesPseudo;

            for(int n=0; n<(piece->nMovesPseudo); n++)
            {
                boardState.moveList[l][k][n] = piece->moveList[n];
                boardState.moveListPseudo[l][k][n] = piece->moveListPseudo[n];
            }
        }
    }
}

void Engine::loadBoardState(std::size_t position)
{
    const BoardState &boardState = boardStateList[position];

    //Save General Game Variables
    turnCounter = boardState.turnCounter;
    turnsUntilDrawCounter = boardState.turnsUntilDrawCounter;
    isdraw = boardState.isdraw;
    requestDraw = boardState.requestDraw;
    lateGame = boardState.lateGame;
    turnColour = boardState.turnColour;
    turnColourOld = turnColour;
    checkmateText = boardState.checkmateText;
    drawText = boardState.drawText;

    //Saving Lists
    for(int l=0; l<2; l++)
    {
        checkmate[l] = boardState.checkmate[l];
        nAlive[l] = boardState.nAlive[l];
        nAvailable[l] = boardState.nAvailable[l];
        dangerZone[l] = boardState.dangerZone[l];

        nMovesLegal[l] = 0;
        nMovesPseudo[l] = 0;

        for(int k=0; k<nPieces; k++)
        {
            piecesListAlive[l][k] = boardState.piecesListAlive[l][k];
            piecesListAvailable[l][k] = boardState.piecesListAvailable[l][k];

            //Saving Board
            const BoardPos posOld = piecesList[l][k]->pos;
            const BoardPos posNew = boardState.pos[l][k];

            board[posOld.i][posOld.j] = boardState.board[posOld.i][posOld.j];
            board[posNew.i][posNew.j] = boardState.board[posNew.i][posNew.j];

            //Saving ChessPiece Variables
            ChessPiece *piece = piecesList[l][k];

            piece->type = boardState.type[l][k];
            piece->alive = boardState.alive[l][k];
            piece->moved = boardState.moved[l][k];
            piece->enpassant = boardState.enpassant[l][k];
            piece->transformed = boardState.transformed[l][k];

            piece->pos = boardState.pos[l][k];
            piece->movement = boardState.movement[l][k];
            piece->movementPseudo = boardState.movementPseudo[l][k];
            piece->nMoves = boardState.nMoves[l][k];
            piece->nMovesPseudo = boardState.nMovesPseudo[l][k];

            if(piece->alive)
            {
                for(int n=0; n<(piece->nMovesPseudo); n++)
                {
                    piece->moveListPseudo[n] = boardState.moveListPseudo[l][k][n];
                    moveListPseudo[l][nMovesPseudo[l]++] = boardState.moveListPseudo[l][k][n];
                }

                for(int n=0; n<(piece->nMoves); n++)
                {
                    piece->moveList[n] = boardState.moveList[l][k][n];
                    moveListLegal[l][nMovesLegal[l]++] = boardState.moveList[l][k][n];
                }
            }
        }
    }
}

void Engine::loadTestBoardState(std::size_t position)
{
    const BoardState &boardState = boardStateList[position];

    //Loading Game Variables
    turnColour = boardState.turnColour;
    turnsUntilDrawCounter = boardState.turnsUntilDrawCounter;
    isdraw = boardState.isdraw;
    lateGame = boardState.lateGame;

    //Loading Lists.
    for(int l=0; l<2; l++)
    {
        checkmate[l] = boardState.checkmate[l];
        dangerZone[l] = boardState.dangerZone[l];
        nMovesPseudo[l] = 0;

        for(int k=0; k<nPieces; k++)
        {
            //Loading Board
            const BoardPos posOld = piecesList[l][k]->pos;
            const BoardPos posNew = boardState.pos[l][k];

            board[posOld.i][posOld.j] = boardState.board[posOld.i][posOld.j];
            board[posNew.i][posNew.j] = boardState.board[posNew.i][posNew.j];

            //Loading ChessPiece Variables
            ChessPiece *piece = piecesList[l][k];

            piece->type = boardState.type[l][k];
            piece->alive = boardState.alive[l][k];
            piece->moved = boardState.moved[l][k];
            piece->enpassant = boardState.enpassant[l][k];
            piece->transformed = boardState.transformed[l][k];

            piece->pos = boardState.pos[l][k];
            piece->movementPseudo = boardState.movementPseudo[l][k];
            piece->nMovesPseudo = boardState.nMovesPseudo[l][k];

            for(int n=0; n<boardState.nMovesPseudo[l][k]; n++)
            {
                piece->moveListPseudo[n] = boardState.moveListPseudo[l][k][n];
                moveListPseudo[l][nMovesPseudo[l]++] = boardState.moveListPseudo[l][k][n];
            }
        }
    }
}

bool Engine::compareBoardStates(const BoardState &state1, const BoardState &state2)
{
    for(int i=0; i<8; i++)
    {
        for(int j=0; j<8; j++)
        {
            ChessPiece *piece1 = state1.board[i][j];
            ChessPiece *piece2 = state2.board[i][j];

            if((piece1==nullptr && piece2 != nullptr) || (piece1!=nullptr && piece2 == nullptr))
                return false;
            else if(piece1!=nullptr && piece2 != nullptr)
            {
                if(piece1->movement != piece2->movement)
                    return false;

                if(piece1->type != piece2->type)
                    return false;

                if(piece1->colour != piece2->colour)
                    return false;

                if(piece1->enpassant != piece2->enpassant)
                    return false;

                if(piece1->moved != piece2->moved)
                    return false;
            }
        }
    }

    return true;
}

//Engine Update BoardState

void Engine::initialize()
{
    bitboardInitializeKeys();

    for(int l=0; l<2; l++)
    {
        for(int k=0; k<nPieces; k++)
        {
            ChessPiece *piece = piecesList[l][k];
            piece->place( piece->startPos );
        }
    }

    updateMovement();
    saveBoardState(0);
}

void Engine::updatePseudoLegalMovement()
{
    for(int l=0; l<2; l++)
    {
        nMovesPseudo[l] = 0;
        dangerZone[l] &= 0;
        for(int k=0; k<nPieces; k++)
        {
            piecesList[l][k]->updatePseudolegalMovement();
            dangerZone[l] |= piecesList[l][k]->movementPseudo;
        }
    }
}

void Engine::updateLegalMovement()
{
    saveBoardState(turnCounter+1);

    for(int l=0; l<2; l++)
    {
        nAlive[l] = 0;
        nAvailable[l] = 0;
        nMovesLegal[l] = 0;

        for(int k=0; k<nPieces; k++)
        {
            ChessPiece *piece = piecesList[l][k];
            if(!piece->alive)
                continue;
            piecesListAlive[l][nAlive[l]++] = piece;

            piece->movement = piece->movementPseudo;
            piece->nMoves = 0;

            for(int n=0; n<piece->nMovesPseudo; n++)
            {
                const ChessMove move = piece->moveListPseudo[n];
                const bool isLegal = makeMove(move);

                if( !isLegal )
                    bitboardFlip(piece->movement, move.end);
                else
                {
                    piece->moveList[piece->nMoves++] = move;
                    moveListLegal[l][nMovesLegal[l]++] = move;
                }

                loadTestBoardState(turnCounter+1);
            }

            if(piece->nMoves > 0)
                piecesListAvailable[l][nAvailable[l]++] = piece;
        }
    }
}

void Engine::checkLateGame()
{
    lateGame = false;

    //Check manually how many pieces are left alive, since piecesListAlive won't be updated.
    int nAliveTest[2] = {0,0};
    for(int l=0; l<2; l++)
    {
        for(int k=0; k<nPieces; k++)
        {
            ChessPiece *piece = piecesList[l][k];
            if(piece->alive)
                nAliveTest[l]++;
        }
    }

    if(!queenWhite.alive && !queenBlack.alive)
        lateGame = true;

    bool WhiteMinorPieceAlive = bishopWhiteL.alive || bishopWhiteR.alive || knightWhiteL.alive || knightWhiteR.alive;
    if( nAliveTest[PWHITE] <=2 || (nAliveTest[PWHITE]==3 && WhiteMinorPieceAlive) )
        lateGame = true;

    bool BlackMinorPieceAlive = bishopBlackL.alive || bishopBlackR.alive || knightBlackL.alive || knightBlackR.alive;
    if( nAliveTest[PBLACK] <=2 || (nAliveTest[PBLACK]==3 && BlackMinorPieceAlive) )
        lateGame = true;
}

void Engine::checkGameOver()
{
    isdraw = false;
    for(int l=0; l<2; l++)
    {
        if(nAvailable[l] == 0)
        {
            checkmate[l] =  piecesList[l][KING]->checkCheck();
            if(checkmate[l])
            {
                turnColour = PNONE;
                checkmateText = ( (l==PWHITE) ? "Black has won!" : "White has won!");
            }
            else
            {
                isdraw = true;
                turnColour = PNONE;
                drawText = ((l==PWHITE) ? "White is out of moves." : "Black is out of moves.");
            }
        }
    }

    if(!checkmate[0] && !checkmate[1])
    {
        //Check if enough pieces are still left on the board.
        bool kingvsking = (nAlive[PWHITE] == 1) && (nAlive[PBLACK] == 1);
        bool kingbishopvsking = ((nAlive[PWHITE] == 2) && (nAlive[PBLACK] == 1)) && (bishopWhiteL.alive || bishopWhiteR.alive);
        bool kingvskingbishop = ((nAlive[PWHITE] == 1) && (nAlive[PBLACK] == 2)) && (bishopBlackL.alive || bishopBlackR.alive);
        bool kingknightvsking = ((nAlive[PWHITE] == 2) && (nAlive[PBLACK] == 1)) && (knightWhiteL.alive || knightWhiteR.alive);
        bool kingvskingknight = ((nAlive[PWHITE] == 1) && (nAlive[PBLACK] == 2)) && (knightBlackL.alive || knightBlackR.alive);
        bool kingbishopvskingbishop = ((nAlive[PWHITE] == 2) && (nAlive[PBLACK] == 2)) &&
            ((bishopWhiteL.alive && bishopBlackR.alive) || (bishopWhiteR.alive && bishopBlackL.alive));

        bool notEnoughPieces = kingvsking || kingbishopvsking || kingvskingbishop || kingknightvsking || kingvskingknight || kingbishopvskingbishop;

        //Check how many times this board configutation has occured.
        int nRepetitions{0};
        for(std::size_t t=turnCounterStart; t<turnCounter; t++)
        {
            if(compareBoardStates(boardStateList[t], boardStateList[turnCounter]))
                ++nRepetitions;
            if(nRepetitions >= 5)
                break;
        }

        //Draw-States
        if(notEnoughPieces)
        {
            isdraw = true;
            drawText = "Not enough pieces left on the board.";
        }

        if(turnsUntilDrawCounter == 100)
        {
            requestDraw = true;
            drawText = "This has been going nowhere for 50 turns.";
        }
        else if(turnsUntilDrawCounter >= 150)
        {
            isdraw = true;
            drawText = "This has been going nowhere for 75 turns.";
        }

        if(nRepetitions == 3)
        {
            requestDraw = true;
            drawText = "We had this situation three times already.";
        }
        else if(nRepetitions >= 5)
        {
            isdraw = true;
            drawText = "We had this situation five times already.";
        }

        if(isdraw)
        {
            turnColour = PNONE;
        }
    }
}

void Engine::updateMovement()
{
    updatePseudoLegalMovement();
    updateLegalMovement();
}

void Engine::updateAttackZone()
{
    for(int l=0; l<2; l++)
    {
        attackZone[l] &= 0;
        //Update via all the normal Pieces.
        for(int k=0; k<8; k++)
        {
            const ChessPiece *piece = piecesList[l][k];
            if(!piece->alive)
                continue;
            attackZone[l] |= piecesList[l][k]->movementPseudo;
        }

        //Update via the Pawns.
        for(int k=8; k<16; k++)
        {
            const ChessPiece *pawn = piecesList[l][k];
            if(!pawn->alive)
                continue;

            const int sign = ( (pawn->colour == PWHITE) ? -1 : +1 );
            const int j = pawn->pos.j+sign;
            if(j < 0 || j >= 8)
                continue;

            const BoardPos left  = {pawn->pos.i-1, j};
            if(left.i >= 0)
            {
                if( board[left.i][left.j] == nullptr || board[left.i][left.j]->colour != pawn->colour )
                {
                    if(!bitboardCheck(attackZone[l], left))
                        bitboardFlip(attackZone[l],left);
                }
            }

            const BoardPos right = {pawn->pos.i+1, j};
            if(right.i < 8)
            {
                if(board[right.i][right.j] == nullptr || board[right.i][right.j]->colour != pawn->colour)
                {
                    if(!bitboardCheck(attackZone[l], right))
                        bitboardFlip(attackZone[l], right);
                }
            }
        }
    }
}

//To execute a ChessMove

void Engine::updateRooks(const BoardPos &Start, const BoardPos &End)
{
    if(End.i == Start.i-2)
        board[0][End.j]->teleportRook();
    else if(End.i == Start.i+2)
        board[7][End.j]->teleportRook();
}

void Engine::resetEnPassantFlags()
{
    const int l = (turnColour == PWHITE) ? PBLACK : PWHITE;
    for(int k=7; k<nPieces; k++)
    {
        piecesList[l][k]->enpassant = false;
    }
}

bool Engine::checkCastelingRights(const ChessMove &move) const
{
    //If the King is in check, one cannot castle.
    if(piecesList[turnColour][KING]->checkCheck())
        return false;

    const BoardPos start = move.start;
    const BoardPos end = move.end;

    bool freepathLeft{true}, freepathRight{true};
    const int jPawn = (turnColour==PWHITE) ? 6 : 1;

    //Check if the central Pawn threatens the Path.
    const ChessPiece *pawnC = board[4][jPawn];
    if(pawnC != nullptr && pawnC->type == PAWN && pawnC->colour != turnColour)
        return false;

    //Check if any other Pawns threaten the Path.
    for(int i=1; i<4; i++)
    {
        const ChessPiece *pawnL = board[i][jPawn];
        if(pawnL != nullptr && pawnL->type == PAWN && pawnL->colour != turnColour)
            freepathLeft = false;

        const ChessPiece *pawnR = board[8-i][jPawn];
        if(pawnR != nullptr && pawnR->type == PAWN && pawnR->colour != turnColour)
            freepathRight = false;
    }

    //Check if the Path is threatened by any other Piece.
    const PColour c = (turnColour==PWHITE) ? PBLACK : PWHITE;

    if(freepathLeft)
        freepathLeft  = !(bitboardCheck(dangerZone[c], {3,start.j})) && !(bitboardCheck(dangerZone[c], {2,start.j}));

    if(freepathRight)
        freepathRight = !(bitboardCheck(dangerZone[c], {5,start.j})) && !(bitboardCheck(dangerZone[c], {6,start.j}));

    //Return false if the King wants to castle alongside an unsafe path.
    if(!freepathLeft && (start.i > end.i))
        return false;

    if(!freepathRight && (start.i < end.i))
        return false;

    return true;
}

bool Engine::makeMove(const ChessMove &move)
{
    const BoardPos start = move.start;
    const BoardPos end = move.end;
    ChessPiece *piece = board[start.i][start.j];

    if(piece == nullptr)
    {
        std::cerr << "ERROR makeMove: Tried to move nullptr!\n";
        return false;
    }

    const bool castelingMove = (move.endType == KING && abs(move.end.i-move.start.i)==2);
    bool canCastle;
    if(castelingMove)
    {
        canCastle = checkCastelingRights(move);
        updateRooks(start,end);
    }

    resetEnPassantFlags();
    piece->move(move);
    piece->transformPawn(move.endType);
    updatePseudoLegalMovement();

    const bool inCheck = piecesList[piece->colour][KING]->checkCheck();

    if(inCheck)
        return false;

    if(castelingMove)
        return canCastle;

    return true;
}

void Engine::advanceTurn()
{
    if(turnColour == PWHITE)
        turnColour = PBLACK;
    else if(turnColour == PBLACK)
        turnColour = PWHITE;
    else
        return;

    updateMovement();
    checkLateGame();
    checkGameOver();

    if(turnCounter == maxTurns)
        maxTurns++;
    turnCounter++;
    turnsUntilDrawCounter++;

    saveBoardState(turnCounter);
}

//FEN-Loading

void Engine::placeTransformedPawn(const BoardPos &pos, PType type, PColour colour) const
{
    for(int k = 8; k < 16; k++)
    {
        ChessPiece *pawn = piecesList[colour][k];
        if(!pawn->alive)
        {
            pawn->place(pos);
            pawn->type = type;
            pawn->transformed = true;
            break;
        }
    }
}

char Engine::getPieceLetter(const ChessPiece *piece) const
{
    if(piece == nullptr)
        return ' ';

    const PType type = piece->type;
    const PColour colour = piece->colour;

    switch(type)
    {
    case KING:
        return (colour==PWHITE) ? 'K' : 'k';

    case QUEEN:
        return (colour==PWHITE) ? 'Q' : 'q';

    case BISHOP:
        return (colour==PWHITE) ? 'B' : 'b';

    case KNIGHT:
        return (colour==PWHITE) ? 'N' : 'n';

    case ROOK:
        return (colour==PWHITE) ? 'R' : 'r';

    case PAWN:
        return (colour==PWHITE) ? 'P' : 'p';

    default:
        return 'X';
    }
}

void Engine::loadFEN(const char *FEN)
{
    //Mark Pieces as dead until placed on their new position.
    for(int l=0; l<2; l++)
    {
        for(int k=0; k<nPieces; k++)
        {
            piecesList[l][k]->kill();
        }
    }

    int i=0, j=0, n=0;
    bool loop = true, eof = false;
    turnCounter = 0;
    turnsUntilDrawCounter = 0;
    
    //First Loop sets the Pieces at the correct position.
    while(loop)
    {
        const char cFEN = FEN[n];

        bool isNumber = (cFEN - '0' > 0 && cFEN - '0' <= 8);

        if(isNumber)
        {
            if(cFEN-'0' == 8 || i >= 8)
                i = -1;
            else
                i += cFEN - '0' - 1;
        }
        else
        {
            switch(cFEN)
            {
            case ' ':
                loop = false;
                break;

            case '\0':
                loop = false;
                eof = true;
                break;

            case '/':
                i = -1;
                j = ( (j < 8) ? j+1 : 7 );
                break;

            case 'r':
                if(!rookBlackL.alive)
                    rookBlackL.place({i,j});
                else if(!rookBlackR.alive)
                    rookBlackR.place({i,j});
                else
                    placeTransformedPawn({i,j}, ROOK, PBLACK);
                break;

            case 'n':
                if(!knightBlackL.alive)
                    knightBlackL.place({i,j});
                else if(!knightBlackR.alive)
                    knightBlackR.place({i,j});
                else
                    placeTransformedPawn({i,j}, KNIGHT, PBLACK);
                break;

            case 'b':
                if(!bishopBlackL.alive)
                    bishopBlackL.place({i,j});
                else if(!bishopBlackR.alive)
                    bishopBlackR.place({i,j});
                else
                    placeTransformedPawn({i,j}, BISHOP, PBLACK);
                break;

            case 'q':
                if(!queenBlack.alive)
                    queenBlack.place({i,j});
                else
                    placeTransformedPawn({i,j}, QUEEN, PBLACK);
                break;

            case 'k':
                kingBlack.place({i,j});
                break;

            case 'p':
                for(int k=8; k<nPieces; k++)
                {
                    if(!piecesList[PBLACK][k]->alive)
                    {
                        piecesList[PBLACK][k]->place({i,j});
                        break;
                    }
                }
                break;

            case 'R':
                if(!rookWhiteL.alive)
                    rookWhiteL.place({i,j});
                else if(!rookWhiteR.alive)
                    rookWhiteR.place({i,j});
                else
                    placeTransformedPawn({i,j}, ROOK, PWHITE);
                break;

            case 'N':
                if(!knightWhiteL.alive)
                    knightWhiteL.place({i,j});
                else if(!knightWhiteR.alive)
                    knightWhiteR.place({i,j});
                else
                    placeTransformedPawn({i,j}, KNIGHT, PWHITE);
                break;

            case 'B':
                if(!bishopWhiteL.alive)
                    bishopWhiteL.place({i,j});
                else if(!bishopWhiteR.alive)
                    bishopWhiteR.place({i,j});
                else
                    placeTransformedPawn({i,j}, BISHOP, PWHITE);
                break;

            case 'Q':
                if(!queenWhite.alive)
                    queenWhite.place({i,j});
                else
                    placeTransformedPawn({i,j}, QUEEN, PWHITE);
                break;

            case 'K':
                kingWhite.place({i,j});
                break;

            case 'P':
                for(int k=8; k<nPieces; k++)
                {
                    if(!piecesList[PWHITE][k]->alive)
                    {
                        piecesList[PWHITE][k]->place({i,j});
                        break;
                    }
                }
                break;
            }
        }
        i++;
        n++;
    }

    //Set the Casteling to false.
    kingWhite.moved  = true;
    rookWhiteL.moved = true;
    rookWhiteR.moved = true;
    kingBlack.moved  = true;
    rookBlackL.moved = true;
    rookBlackR.moved = true;

    //Second Loop sets the Flags and TurnCounter.
    if(!eof)
    {
        bool loop2=true;
        bool plyCounterSet = false;
        bool turnCounterSet = false;

        while(loop2)
        {
            const char cFEN = FEN[n];
            bool isNumber = (cFEN - '0' >= 0 && cFEN - '0' <= 9);

            const int cFEN2 = FEN[n-1];
            bool isSquareLetter = (cFEN2 >= 'a' && cFEN2 <= 'h');

            //en passant
            if(isNumber && isSquareLetter)
            {
                int i = cFEN2 - 'a';
                int j = '8' - cFEN;

                if(j==2)
                    board[i][j+1]->enpassant = true;
                else if(j==5)
                    board[i][j-1]->enpassant = true;

            }
            //Turn-Counter
            else if(isNumber)
            {
                int nDigits=1;
                int digits[4]{};

                digits[0] = cFEN - '0';
                digits[1] = FEN[n+1] - '0';

                if(digits[1] >= 0 && digits[1] <= 9)
                {
                    nDigits++;
                    digits[2] = FEN[n+2] - '0';

                    if(digits[2] >= 0 && digits[2] <= 9)
                    {
                        nDigits++;
                        digits[3] = FEN[n+3] - '0';

                        if(digits[3] >= 0 && digits[3] <= 9)
                            nDigits++;
                    }
                }

                if(!plyCounterSet)
                {
                    switch(nDigits)
                    {
                    case 1:
                        turnsUntilDrawCounter = static_cast<std::size_t>(digits[0]);
                        break;

                    case 2:
                        turnsUntilDrawCounter = static_cast<std::size_t>(10*digits[0] + digits[1]);
                        break;

                    case 3:
                        turnsUntilDrawCounter = static_cast<std::size_t>(100*digits[0] + 10*digits[1] + digits[2]);
                        break;

                    case 4:
                        turnsUntilDrawCounter = static_cast<std::size_t>(1000*digits[0] + 100*digits[1] + 10*digits[2] + digits[3]);
                        break;
                    }

                    plyCounterSet = true;
                }
                else if(!turnCounterSet)
                {
                    switch(nDigits)
                    {
                    case 1:
                        turnCounter = static_cast<std::size_t>(digits[0]);
                        break;

                    case 2:
                        turnCounter = static_cast<std::size_t>(10*digits[0] + digits[1]);
                        break;

                    case 3:
                        turnCounter = static_cast<std::size_t>(100*digits[0] + 10*digits[1] + digits[2]);
                        break;

                    case 4:
                        turnCounter = static_cast<std::size_t>(1000*digits[0] + 100*digits[1] + 10*digits[2] + digits[3]);
                        break;
                    }

                    turnCounterSet = true;
                }
            }
            else
            {
                switch(cFEN)
                {
                case '\0':
                    loop2 = false;
                    eof = true;
                    break;

                case 'w':
                    turnColour = PWHITE;
                    break;

                case 'b':
                    turnColour = PBLACK;
                    break;

                case 'K':
                    kingWhite.moved = false;
                    rookWhiteR.moved = false;
                    break;

                case 'Q':
                    kingWhite.moved = false;
                    rookWhiteL.moved = false;
                    break;

                case 'k':
                    kingBlack.moved = false;
                    rookBlackL.moved = false;
                    break;

                case 'q':
                    kingBlack.moved = false;
                    rookBlackR.moved = false;
                    break;
                }
            }
            n++;
        }

        //Adjust the turn-Counter to represent the number of moves.
        if(turnCounter > 0)
        {
            if(turnColour == PWHITE)
                turnCounter = 2*turnCounter - 2;
            else
                turnCounter = 2*turnCounter - 1;
        }
        turnCounterStart = turnCounter;
        maxTurns = turnCounter;
    }

    updateMovement();
    saveBoardState(turnCounter);
}

std::string Engine::getFEN() const
{
    std::stringstream FEN{};

    int nEmpty = 0;

    for(int j=0; j<8; j++)
    {
        for(int i=0; i<8; i++)
        {
            const ChessPiece *piece = board[i][j];
            const ChessPiece *previousPiece = (i==0) ? nullptr : board[i-1][j];

            if(piece == nullptr)
            {
                nEmpty++;

                if(i==7 && nEmpty > 0)
                    FEN << nEmpty;

                continue;
            }
            else if(previousPiece == nullptr && nEmpty > 0)
                FEN << nEmpty;

            nEmpty = 0;
            FEN << getPieceLetter(piece);

        }

        if(j != 7)
            FEN << '/';
        nEmpty = 0;
    }

    //Turn-Colour
    FEN << ' ' << ( (turnColour == PWHITE) ? 'w' : 'b' ) << ' ';

    //Casteling
    bool canCastle = false;
    if(kingWhite.moved == false && rookWhiteR.moved == false)
    {
        FEN << 'K';
        canCastle = true;
    }

    if(kingWhite.moved == false && rookWhiteL.moved == false)
    {
        FEN << 'Q';
        canCastle = true;
    }

    if(kingBlack.moved == false && rookBlackR.moved == false)
    {
        FEN << 'k';
        canCastle = true;
    }

    if(kingBlack.moved == false && rookBlackL.moved == false)
    {
        FEN << 'q';
        canCastle = true;
    }

    if(!canCastle)
        FEN << '-';

    FEN << ' ';

    //en-passant
    const int l = (turnColour == PWHITE) ? PBLACK : PWHITE;
    for(int k=7; k<nPieces; k++)
    {
        if(piecesList[l][k]->enpassant)
        {
            const int sign = (l==PWHITE) ? +1 : -1;
            const BoardPos pos = piecesList[l][k]->pos;

            FEN << (char)(pos.i + 'a') << (char)('8' - pos.j - sign);

            break;
        }

        if(k == 15)
            FEN << '-';
    }

    //Turn Counter
    FEN << ' ' << turnsUntilDrawCounter << ' ' << (turnCounter/2) + 1;

    return FEN.str();
}

//Weight-Calculations

int Engine::getPositionWeight(const BoardPos &pos, PType type, PColour colour) const
{
    //For Black, the PST-Matrix must be mirrored.
    const int i = (colour==PWHITE) ? pos.j : 7-pos.j;
    const int j = pos.i;

    switch(type)
    {
    case KING:
        if(lateGame)
            return KingEndPST[i][j];
        else
            return KingMiddlePST[i][j];
        break;

    case QUEEN:
        return QueenPST[i][j];
        break;

    case BISHOP:
        return BishopPST[i][j];
        break;

    case KNIGHT:
        return KnightPST[i][j];
        break;

    case ROOK:
        return RookPST[i][j];
        break;

    case PAWN:
        return PawnPST[i][j];
        break;
    }
    return 0;
}

int Engine::getBoardWeight() const
{
    if(isdraw)
        return 0;

    int weight = 0;

    for(int l=0; l<2; l++)
    {
        const int sign = ((l==PWHITE) ? +1 : -1);

        //Add the PST- and piece-values.
        for(int k=0; k<nPieces; k++)
        {
            const ChessPiece *piece = piecesList[l][k];
            if(piece->alive)
                weight += sign * ( getPositionWeight(piece->pos, piece->type, piece->colour) + pieceValue[piece->type] );
        }

        //Add the Kings value if there is a checkmate.
        if(checkmate[l])
        {
            weight -= sign*pieceValue[KING];
        }
    }

    return weight;
}

int Engine::getUpdatedBoardWeight(const ChessMove &move) const
{
    const BoardPos start = move.start, end = move.end;
    if(start.i==10)
        return INT_MAX;

    const PType endType = move.endType;
    const ChessPiece *piece = board[start.i][start.j], *capturedPiece = board[end.i][end.j];
    if(piece==nullptr)
    {
        std::cout << "ERROR getUpdatedBoardWeight: Tried to move nullptr!\n";
        printMove(move);
        printChessboard();
        exit(-1);
        return INT_MAX;
    }

    const int sign = (piece->colour == PWHITE) ? +1 : -1;
    int newBoardWeight = getBoardWeight();

    //Update new Position
    newBoardWeight += sign * (getPositionWeight(end, move.endType, piece->colour) - getPositionWeight(start, piece->type, piece->colour));

    //Update Capture
    if(capturedPiece != nullptr)
    {
        newBoardWeight += sign * (pieceValue[capturedPiece->type] + getPositionWeight(end, capturedPiece->type, capturedPiece->colour));
    }
    else if(piece->type == PAWN) //en-passant
    {
        if(end.i - start.i == 1)
        {
            const ChessPiece *enpassantPiece = board[start.i+1][start.j];
            newBoardWeight += sign * (pieceValue[enpassantPiece->type] + getPositionWeight({start.i+1,start.j}, enpassantPiece->type, enpassantPiece->colour));
        }

        if(end.i - start.i == -1)
        {
            const ChessPiece *enpassantPiece = board[start.i-1][start.j];
            newBoardWeight += sign * (pieceValue[enpassantPiece->type] + getPositionWeight({start.i-1,start.j}, enpassantPiece->type, enpassantPiece->colour));
        }
    }

    //Casteling
    if(piece->type == KING)
    {
        if(end.i - start.i == 2)
        {
            newBoardWeight += sign * (getPositionWeight({end.i-1,end.j}, ROOK, piece->colour) - getPositionWeight({7,start.j}, ROOK, piece->colour));
        }

        if(end.i - start.i == -2)
        {
            newBoardWeight += sign * (getPositionWeight({end.i+1,end.j}, ROOK, piece->colour) - getPositionWeight({0,start.j}, ROOK, piece->colour));
        }
    }

    //Pawn-Transform
    if(piece->type == PAWN && (end.j == 0 || end.j == 7))
    {
        newBoardWeight += sign * (pieceValue[endType] - pieceValue[PAWN]);
    }

    return newBoardWeight;
}

void Engine::advancePseudoTurn()
{
    //Check for lateGame
    checkLateGame();

    if(turnColour == PWHITE)
        turnColour = PBLACK;
    else if(turnColour == PBLACK)
        turnColour = PWHITE;
}

//External Functions that makes use of the ChessEngine

void Chess::initialize()
{
    mainEngine.initialize();
    std::cout << "Chess Engine loaded.\n";
}

//Threads

void Chess::cleanUpThreads()
{
    const int nThreads = (int)threadList.size();
    if(nThreads <= 0)
        return;

    for(int i = nThreads-1; i>=0; i--)
    {
        if(threadDone[i])
        {
            //Join the thread if necessary.
            if(threadList[i].joinable())
                threadList[i].join();

            //Erase them from the list.
            threadList.erase(threadList.begin() + i);
            threadDone.erase(threadDone.begin() + i);
        }
    }
}

void Chess::stopAllThreads()
{
    stopThread.store(true);
    for(auto &thread : threadList)
    {
        if(thread.joinable())
            thread.join();
    }
    threadList.clear();
    stopThread.store(false);
}

//Weight Calculations

void Chess::sortMovesMaxtoMin(std::vector<ChessMove> &moveList, std::unique_ptr<Engine> &engine)
{
    //Sorting-Lambda for Weights
    auto sortByWeight
    {
        [&engine](ChessMove move1, ChessMove move2)
        {
            int weight1 = engine->getUpdatedBoardWeight(move1);
            int weight2 = engine->getUpdatedBoardWeight(move2);

            if(weight1==INT_MAX)
                weight1 = -INT_MAX;

            if(weight2==INT_MAX)
                weight2 = -INT_MAX;

            return weight1 > weight2;
        }
    };

    std::sort(moveList.begin(), moveList.end(), sortByWeight);
}

void Chess::sortMovesMintoMax(std::vector<ChessMove> &moveList, std::unique_ptr<Engine> &engine)
{
    //Sorting-Lambda for Weights
    auto sortByWeight
    {
        [&engine](ChessMove move1, ChessMove move2)
        {
            int weight1 = engine->getUpdatedBoardWeight(move1);
            int weight2 = engine->getUpdatedBoardWeight(move2);

            return weight1 < weight2;
        }
    };

    std::sort(moveList.begin(), moveList.end(), sortByWeight);
}

int Chess::maxAlphaBetaWeight(int depth, const int maxDepth, int alpha, int beta, const std::vector<ChessMove> &moveList, std::unique_ptr<Engine> &engine)
{
    if(depth<=0 || stopThread.load() || engine->turnColour == PNONE)
        return engine->getBoardWeight();

    engine->saveBoardState(engine->turnCounter + maxDepth - depth);

    int max = -INT_MAX;

    for( const ChessMove &move : moveList )
    {
        if(stopThread.load())
            return max;

        const bool isLegal = engine->makeMove(move);

        if(isLegal)
        {
            engine->advancePseudoTurn();

            std::vector<ChessMove> newMoveList{};
            newMoveList.assign(engine->moveListPseudo[PBLACK], engine->moveListPseudo[PBLACK] + engine->nMovesPseudo[PBLACK]);

            sortMovesMintoMax(newMoveList,engine);
          
            const int weight = minAlphaBetaWeight(depth-1, maxDepth, alpha, beta, newMoveList, engine);

            if(weight > max)
            {
                max = weight;
                if(weight > alpha)
                    alpha = weight;
            }

            if(weight >= beta)
                return weight;
        }

        engine->loadTestBoardState(engine->turnCounter + maxDepth - depth);
    }

    return max;
}

int Chess::minAlphaBetaWeight(int depth, const int maxDepth, int alpha, int beta, const std::vector<ChessMove> &moveList, std::unique_ptr<Engine> &engine)
{
    if(depth<=0 || stopThread.load() || engine->turnColour == PNONE)
        return engine->getBoardWeight();

    engine->saveBoardState(engine->turnCounter + maxDepth - depth);

    int min = INT_MAX;

    for(const ChessMove &move : moveList)
    {
        if(stopThread.load())
            return min;

        const bool isLegal = engine->makeMove(move);

        if(isLegal)
        {
            engine->advancePseudoTurn();

            std::vector<ChessMove> newMoveList{};
            newMoveList.assign(engine->moveListPseudo[PWHITE], engine->moveListPseudo[PWHITE] + engine->nMovesPseudo[PWHITE]);
            sortMovesMaxtoMin(newMoveList,engine);

            const int weight = maxAlphaBetaWeight(depth-1, maxDepth, alpha, beta, newMoveList, engine);

            if(weight < min)
            {
                min = weight;
                if(weight < beta)
                    beta = weight;
            }

            if(weight <= alpha)
                return weight;
        }

        engine->loadTestBoardState(engine->turnCounter + maxDepth - depth);
    }

    return min;
}

int Chess::getMoveWeight(int depth, const ChessMove &move, std::unique_ptr<Engine> &engine)
{
    if(mainEngine.turnColour == PWHITE)
        return maxAlphaBetaWeight(depth, depth, -INT_MAX, +INT_MAX, {move}, engine);
    else if(mainEngine.turnColour == PBLACK)
        return minAlphaBetaWeight(depth, depth, -INT_MAX, +INT_MAX, {move}, engine);
    else
        return mainEngine.getBoardWeight();
}

//Bot-Functions

void Bot::executeMove() const
{
    bool isLegal = mainEngine.makeMove(nextMove);
    if(isLegal)
        mainEngine.advanceTurn();
    else
        std::cerr << "ERROR Bot: Generated illegal move!";
}

void Bot::reset()
{
    spriteMoved = false;
    spriteArrived = false;
    searching.store(false);
    moveChosen.store(false);
}

int Bot::chooseRandomPiece() const
{
    return Random::randInt(0, mainEngine.nAvailable[botColour]-1);
}

void Bot::generateMove()
{
    if(!searching.load())
    {
        searching.store(true);

        //Lambda to create a thread
        auto createThread
        {
            [this]()
            {
                std::size_t threadID = threadList.size()-1;

                switch(botType)
                {
                case RANDOMBOT:
                    generateMoveRandomBot();
                    break;

                case METROPOLISBOT:
                    generateMoveMetropolisBot();
                    break;

                case WEIGHTEDRANDOMBOT1:
                    generateMoveWeightedRandomBot1();
                    break;

                case WEIGHTEDRANDOMBOT2:
                    generateMoveWeightedRandomBot2();
                    break;

                case OPTIMUMBOT1:
                    generateMoveOptimumBot1();
                    break;

                case OPTIMUMBOT2:
                    generateMoveOptimumBot2();
                    break;

                default:
                    generateMoveRandomBot();
                    break;
                }

                if(bitboardCheck(mainEngine.board[nextMove.start.i][nextMove.start.j]->movement, nextMove.end))
                    moveChosen.store(true);
                else
                {
                    std::cerr << "ERROR generateMove: Generated illegal Move! ";
                    mainEngine.printMove(nextMove);
                    std::cout << '\n';
                }

                searching.store(false);
                threadDone[threadID] = true;
            }
        };

        threadDone.emplace_back(false);
        threadList.emplace_back(createThread);
    }
}

void Bot::generateMoveRandomBot()
{
    const int r = Random::randInt( 0, mainEngine.nMovesLegal[botColour]-1 );
    nextMove = mainEngine.moveListLegal[botColour][r];
}

void Bot::generateMoveMetropolisBot()
{
    const int sign = (botColour == PWHITE) ? +1 : -1;
    const int weightOld = sign * mainEngine.getBoardWeight();

    int test = 0;
    const int nMoves = mainEngine.nMovesLegal[botColour];

    while(1)
    {
        //Initalize the board to perform the search on.
        testEngine->loadFEN(mainEngine.getFEN().c_str());
        testEngine->turnCounter = 0;
        testEngine->saveBoardState(0);

        //Get a random move.
        generateMoveRandomBot();

        //Get the weight and switch the sign such that its from the bots perspective
        int weightNew = sign * getMoveWeight(depth, nextMove, testEngine);

        //Ensure Weights are positive and nonzero.
        const int minValue = abs(std::min(weightOld, weightNew));
        const int weightNorm = weightOld + minValue + 1;
        weightNew += minValue + 1;

        //Determine acceptance probability.
        double prob = (double)weightNew / (double)weightNorm;

        if(prob >= 1.0)
            break;
        else
        {
            double u = Random::randDouble();
            if(u <= prob)
                break;
        }

        test++;
        if(test >= nMoves)
            break;
    }
}

void Bot::generateMoveWeightedRandomBot1()
{
    const int sign = (botColour == PWHITE) ? +1 : -1;
    const ChessPiece *piece = mainEngine.piecesListAvailable[botColour][chooseRandomPiece()];
    const int nMoves = piece->nMoves;
    std::vector<int> weightList(nMoves);
    int minWeight = INT_MAX;
    int maxWeight = -INT_MAX;
    int maxWeightID = 0;
    for(int m=0; m<nMoves; m++)
    {
        testEngine->loadFEN(mainEngine.getFEN().c_str());
        testEngine->turnCounter = 0;
        testEngine->saveBoardState(0);
        weightList[m] = sign*getMoveWeight(depth, piece->moveList[m], testEngine);
        if(weightList[m] < minWeight)
            minWeight = weightList[m];

        if(weightList[m] > maxWeight)
        {
            maxWeight = weightList[m];
            maxWeightID = m;
        }
    }
    minWeight = abs(minWeight);

    //Ensure Weights are positive and nonzero
    int Z = 0;
    for(int m=0; m<nMoves; m++)
    {
        weightList[m] += minWeight + 1;
        Z += weightList[m];
    }

    int test = 0;
    while(1)
    {
        const int ID = Random::randInt(0, nMoves-1);
        nextMove = piece->moveList[ID];
        const double prob = (double)weightList[ID] / (double)Z;
        const double u = Random::randDouble();

        if(u <= prob)
            break;

        test++;
        if(test >= nMoves)
        {
            nextMove = piece->moveList[maxWeightID];
            break;
        }
    }
}

void Bot::generateMoveWeightedRandomBot2()
{
    const int sign = (botColour == PWHITE) ? +1 : -1;
    const int nMoves = mainEngine.nMovesLegal[botColour];
    std::vector<int> weightList(nMoves);
    int minWeight = INT_MAX;
    int maxWeight = -INT_MAX;
    int maxWeightID = 0;
    for(int m=0; m<nMoves; m++)
    {
        testEngine->loadFEN(mainEngine.getFEN().c_str());
        testEngine->turnCounter = 0;
        testEngine->saveBoardState(0);
        weightList[m] = sign*getMoveWeight(depth,mainEngine.moveListLegal[botColour][m],testEngine);
        if(weightList[m] < minWeight)
            minWeight = weightList[m];

        if(weightList[m] > maxWeight)
        {
            maxWeight = weightList[m];
            maxWeightID = m;
        }
    }
    minWeight = abs(minWeight);

    //Ensure Weights are positive and nonzero
    int Z = 0;
    for(int m=0; m<nMoves; m++)
    {
        weightList[m] += minWeight + 1;
        Z += weightList[m];
    }

    int test = 0;
    while(1)
    {
        const int ID = Random::randInt(0,nMoves-1);
        nextMove = mainEngine.moveListLegal[botColour][ID];
        const double prob = (double)weightList[ID] / (double)Z;
        const double u = Random::randDouble();

        if(u <= prob)
            break;

        test++;
        if(test >= nMoves)
        {
            nextMove = mainEngine.moveListLegal[botColour][maxWeightID];
            break;
        }
    }
}

void Bot::generateMoveOptimumBot1()
{
    //Pick a random Piece and get its moves.
    const int ID = chooseRandomPiece();
    const ChessPiece *piece = mainEngine.piecesListAvailable[botColour][ID];
    const int nMoves = piece->nMoves;
    std::vector<ChessMove> moveList(nMoves);
    for(int m=0; m<nMoves; m++)
    {
        moveList[m] = piece->moveList[m];
    }

    //Find the best Move.
    if(botColour == PWHITE)
        findMaxMove(moveList);
    else
        findMinMove(moveList);
}

void Bot::generateMoveOptimumBot2()
{
    //Create the Movelist
    const int nMoves = mainEngine.nMovesLegal[botColour];
    std::vector<ChessMove> moveList(nMoves);
    for(int m=0; m<nMoves; m++)
    {
        moveList[m] = mainEngine.moveListLegal[botColour][m];
    }

    //Find the best Move.
    if(botColour == PWHITE)
        findMaxMove(moveList);
    else
        findMinMove(moveList);
}

void Bot::findMaxMove(std::vector<ChessMove> &moveList)
{
    //Initalize the board to perform the search on (necessary when using threads).
    testEngine->loadFEN(mainEngine.getFEN().c_str());
    testEngine->turnCounter = 0;
    testEngine->saveBoardState(0);

    int alpha = -INT_MAX;
    constexpr int beta = +INT_MAX;
    int maxWeight = -INT_MAX;
    sortMovesMaxtoMin(moveList, testEngine);
    nextMove = moveList[0];

    if(depth <= 0)
        return;

    for(const ChessMove &move : moveList)
    {
        if(stopThread.load())
            break;

        testEngine->makeMove(move);
        testEngine->advancePseudoTurn();

        std::vector<ChessMove> newMoveList{};
        newMoveList.assign(testEngine->moveListPseudo[PBLACK], testEngine->moveListPseudo[PBLACK] + testEngine->nMovesPseudo[PBLACK]);
        sortMovesMintoMax(newMoveList, testEngine);

        const int weight = minAlphaBetaWeight(depth-1, depth, alpha, beta, newMoveList, testEngine);

        if(weight > maxWeight)
        {
            nextMove = move;
            maxWeight = weight;
            if(weight > alpha)
                alpha = weight;
        }

        testEngine->loadTestBoardState(testEngine->turnCounter);
    }
}

void Bot::findMinMove(std::vector<ChessMove> &moveList)
{
    //Initalize the board to perform the search on (necessary when using threads).
    testEngine->loadFEN(mainEngine.getFEN().c_str());
    testEngine->turnCounter = 0;
    testEngine->saveBoardState(0);

    constexpr int alpha = -INT_MAX;
    int beta = +INT_MAX;
    int minWeight = +INT_MAX;
    sortMovesMintoMax(moveList, testEngine);
    nextMove = moveList[0];

    if(depth <= 0)
        return;

    for(const ChessMove &move : moveList)
    {
        if(stopThread.load())
            break;

        testEngine->makeMove(move);
        testEngine->advancePseudoTurn();

        std::vector<ChessMove> newMoveList{};
        newMoveList.assign(testEngine->moveListPseudo[PWHITE], testEngine->moveListPseudo[PWHITE] + testEngine->nMovesPseudo[PWHITE]);
        sortMovesMaxtoMin(newMoveList, testEngine);

        const int weight = maxAlphaBetaWeight(depth-1, depth, alpha, beta, newMoveList, testEngine);

        if(weight < minWeight)
        {
            nextMove = move;
            minWeight = weight;
            if(weight < beta)
                beta = weight;
        }

        testEngine->loadTestBoardState(testEngine->turnCounter);
    }
}