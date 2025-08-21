#include "ChessUCI.h"
#include "Raylib_GameHeader.h"
#include "LevelChessboard.h"
#include "LevelSetupChessboard.h"

using namespace Chess;

void ChessUCI::inputUCI()
{
    while( !stopMainThread.load() )
    {
        std::getline( std::cin, inputString );

        cleanUpThreads();

        //Commands with no Parameters or Options
        if(stopMainThread.load() || inputString == "exit" || inputString == "quit" || inputString == "q" || inputString == "close")
        {
            stopAllThreads();
            stopMainThread.store(true);
            Raylib::Window.active = false;
        }

        else if(inputString == "stop")
            stopAllThreads();

        else if(inputString == "isready")
            std::cout << "readyok\n";

        else if(inputString == "help" || inputString == "h")
            executeHelpCommand();

        else if(UCIsetCommandParameters("d", {"board","movementrange","attackrange"}) || UCIsetCommandParameters("display", {"board","movementrange","attackrange"}))
            executeDisplayCommand();

        else if(UCIsetCommandParameters("position", {"startpos","testpos","fen","moves"}))
            executePositionCommand();

        else if(UCIsetCommandParameters("perft", {"depth", "searchmoves"}))
            executePerftCommand();

        else if(UCIsetCommandParameters("go", {"depth", "searchmoves", "bottype"}))
            executeGoCommand();

        else
            std::cout << "Unknown command.\n";
    }
}

bool ChessUCI::UCIsetCommandParameters(std::string_view command, std::vector<std::string_view> options)
{
    if(inputString.substr(0, command.length()) != command)
        return false;

    const std::size_t nOptions = options.size();
    inputParameters.resize(nOptions);
    if(nOptions == 0)
        return true;

    //Stores the Location of the Options within inputString.
    struct posOptionsStruct
    {
        int i;                      //Which Option from the options-Vector this refers to.
        std::size_t posOption;      //The position of the Option in inputString.
        std::size_t posParameter;   //The position of the first Parameter in inputString.
    };
    std::vector<posOptionsStruct> posOptions(nOptions);

    //Find the location of the Options.
    for(int i=0; i<nOptions; i++)
    {
        posOptions[i].i = i;
        posOptions[i].posOption = inputString.find(options[i]);
        if(posOptions[i].posOption < std::string::npos)
            posOptions[i].posParameter = posOptions[i].posOption + options[i].size() + 1;
        else
            posOptions[i].posParameter = std::string::npos;
    }

    //Sort based on the order they appear in inputString.
    std::sort(posOptions.begin(), posOptions.end(), [](const posOptionsStruct &a, const posOptionsStruct &b) {return a.posOption < b.posOption; });

    //Get the Parameters.
    for(std::size_t j=0; j<nOptions; j++)
    {
        const std::size_t start = posOptions[j].posParameter;
        std::size_t end = (j+1 < nOptions) ? posOptions[j+1].posOption : inputString.size();

        if(end == std::string::npos)
            end = inputString.size();

        //If the Option was not used
        if(start == std::string::npos)
            inputParameters[posOptions[j].i] = "NO_OPTION";

        //If the Option has no Parameters
        else if(start >= end)
            inputParameters[posOptions[j].i] = "NO_PARAMETER";

        //Store Parameter
        else
            inputParameters[posOptions[j].i] = inputString.substr(start, end-start );
    }

    return true;
}

void ChessUCI::executeHelpCommand()
{
    std::cout << "\'quit\', \'q\', \'exit\' or \'close\':\n";
    std::cout << "\tClose the program.\n\n";
    
    std::cout << "\'stop\':\n";
    std::cout << "\tAbort all ongoing calculations.\n\n";


    std::cout << "\'go\':\n";
    std::cout << "\tLet the engine calculate the next move with the current settings.\n";

    std::cout << "\'go depth [depth-value]\':\n";
    std::cout << "\tLet the engine calculate the next move with a given depth.\n";

    std::cout << "\'go searchmoves [move1] [move2] ...\':\n";
    std::cout << "\tLet the engine find the best move among a list of moves.\n";
    std::cout << "\tWarning: This option only works for the master bot!\n";

    std::cout << "\'go bottype [bot-name]\':\n";
    std::cout << "\tLet one of the other bots determine the next move.\n";
    std::cout << '\n';


    std::cout << "\'position\':\n";
    std::cout << "\tSets up a given chess position.\n";

    std::cout << "\'position startpos\':\n";
    std::cout << "\tReturns the board to the default start position.\n";

    std::cout << "\'position testpos [i]\':\n";
    std::cout << "\tSets the board to one of 6 available test positions.\n";

    std::cout << "\'position fen [fen-String]\':\n";
    std::cout << "\tSets the board according to a custom fen-String.\n";

    std::cout << "\'position moves [move1] [move2] ...\':\n";
    std::cout << "\tPlays out a series of moves from the current chess position. Can be combined with the other position commands.\n";
    std::cout << "\tWarning: This allows you to play illegal moves too! Empty fields cannot be moved.\n";
    std::cout << '\n';


    std::cout << "\'d\' or \'display\':\n";
    std::cout << "\tShow the current state of the chessboard. Default is displaying the chessboard itself.\n";

    std::cout << "\'d board\' or \'display board\':\n";
    std::cout << "\tDisplay the chessboard itself.\n";

    std::cout << "\'d movementrange\' or \'display movementrange\':\n";
    std::cout << "\tDisplay the movement range of all chesspieces.\n";

    std::cout << "\'d attackrange\' or \'display attackrange\':\n";
    std::cout << "\tDisplay the attack range of all chesspieces.\n";
    std::cout << '\n';


    std::cout << "\'perft depth [depth-value]\':\n";
    std::cout << "\tPerform a perft-search to find the number of available positions after depth moves.\n";
    std::cout << "\'perft depth [depth-value] searchmoves [move1] [move2] ...\':\n";
    std::cout << "\tPerform a perft-search limited to a list of moves.\n";
    std::cout << '\n';
}

void ChessUCI::executeDisplayCommand()
{
    enum DisplayOptions
    {
        BOARD,
        DANGERZONES,
        ATTACKZONES,
    };

    if(inputParameters[BOARD] != "NO_OPTION" || (inputParameters[DANGERZONES] == "NO_OPTION" && inputParameters[ATTACKZONES] == "NO_OPTION") )
        mainEngine.printChessboard();

    if(inputParameters[DANGERZONES] != "NO_OPTION")
    {
        mainEngine.printDangerZones();
    }

    if(inputParameters[ATTACKZONES] != "NO_OPTION")
    {
        mainEngine.updateAttackZone();
        mainEngine.printAttackZones();
    }
}

void ChessUCI::executePositionCommand()
{
    enum PositionOptions
    {
        STARTPOS,
        TESTPOS,
        FEN,
        MOVES,
    };

    if(inputParameters[STARTPOS] != "NO_OPTION")
    {
        mainEngine.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ");
    }
    else if(inputParameters[TESTPOS] != "NO_OPTION" && inputParameters[TESTPOS] != "NO_PARAMETER")
    {
        try
        {
            const int testpos = std::stoi(inputParameters[TESTPOS]);

            switch(testpos)
            {
            case 1:
                mainEngine.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ");                    //Position 1, Standard.
                break;

            case 2:
                mainEngine.loadFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ");           //Position 2, Kiwipete, for testing casteling.
                break;

            case 3:
                mainEngine.loadFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");                                   //Position 3, for testing en-passant.
                break;

            case 4:
                mainEngine.loadFEN("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");            //Position 4, for testing promotions.
                break;

            case 5:
                mainEngine.loadFEN("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");                   //Position 5.
                break;

            case 6:
                mainEngine.loadFEN("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");    //Position 6.
                break;

            default:
                std::cout << "ERROR: There are only 6 Testpositions available!\n";
                return;
            }
        }

        catch(const std::invalid_argument &e)
        {
            e;
            std::cout << "ERROR: Invalid Parameter for testpos! Use Numbers 1-6 instead.\n";
        }

        catch(const std::out_of_range &e)
        {
            e;
            std::cout << "ERROR: There are only 6 Testpositions available!\n";
        }
    }
    else if(inputParameters[FEN] != "NO_OPTION" && inputParameters[FEN] != "NO_PARAMETER")
    {
        mainEngine.loadFEN(inputParameters[FEN].c_str());
    }

    mainEngine.updateLegalMovement();
    mainEngine.checkLateGame();
    mainEngine.checkGameOver();
    mainEngine.turnCounter = mainEngine.turnCounterStart;
    mainEngine.saveBoardState(mainEngine.turnCounter);

    if(inputParameters[MOVES] != "NO_OPTION" && inputParameters[MOVES] != "NO_PARAMETER")
    {
        std::vector<ChessMove> moveList;
        const int nMoves = extractChessMoves( inputParameters[MOVES].c_str(), moveList );

        for(int m=0; m<nMoves; m++)
        {
            mainEngine.makeMove(moveList[m]);
            mainEngine.turnColour = (mainEngine.turnColour == PWHITE) ? PBLACK : PWHITE;
            mainEngine.turnCounter++;
            mainEngine.turnsUntilDrawCounter++;
            mainEngine.saveBoardState(mainEngine.turnCounter);
        }

        mainEngine.updateLegalMovement();
        mainEngine.checkLateGame();
        mainEngine.checkGameOver();
    }

    if(mainEngine.turnColour == PNONE)
    {
        if(mainEngine.isdraw)
            std::cout << "Draw! " << mainEngine.drawText << '\n';
        else
            std::cout << "Checkmate! " << mainEngine.checkmateText << '\n';
    }
    else if(mainEngine.requestDraw)
        std::cout << "Note: A draw could be requested.\n";

    if(levelChessboard.isLoaded())
    {
        levelChessboard.setChessPieceSpriteDestination();
        levelChessboard.moveChessPieces();
    }
    else if(levelSetupChessboard.isLoaded())
    {
        levelChessboard.setChessPieceSpriteDestination();
        levelSetupChessboard.moveChessPieces();
    }
}

int ChessUCI::extractChessMoves(const char *moveString, std::vector<ChessMove> &moveList)
{
    int m = 0, n = 0;
    moveList.clear();

    while(1)
    {
        const char c = moveString[n];
        if(c == '\0')
            break;

        //Check if c is a square Letter
        if(c >= 'a' && c <= 'h')
        {
            //Get the following 3 letters
            const char c1 = moveString[n+1];

            char c2 = '\0';
            if(c1 != '\0')
                c2 = moveString[n+2];

            char c3 = '\0';
            if(c2 != '\0')
                c3 = moveString[n+3];

            //Check if the following Letters denote a position
            const bool isSquareNumber1 = (c1 - '0' > 0 && c1 - '0' <= 8);
            const bool isSquareLetter2 = (c2 >= 'a' && c2 <= 'h');
            const bool isSquareNumber3 = (c3 - '0' > 0 && c3 - '0' <= 8);

            if( isSquareNumber1 && isSquareLetter2 && isSquareNumber3 )
            {
                ChessMove move;

                move.start.i = c - 'a';
                move.start.j = '8' - c1;

                move.end.i = c2 - 'a';
                move.end.j = '8' - c3;

                //Check if there is a ChessPiece to be moved
                const ChessPiece *piece = mainEngine.board[move.start.i][move.start.j];
                if(piece != nullptr)
                {
                    //Check for Pawn-Promotion
                    char c4 = '\0';
                    if(c3 != '\0')
                        c4 = moveString[n+4];

                    const bool pawnTransformQueen  = (c4 == 'q' || c4 == 'Q');
                    const bool pawnTransformBishop = (c4 == 'b' || c4 == 'B');
                    const bool pawnTransformKnight = (c4 == 'n' || c4 == 'N');
                    const bool pawnTransformRook   = (c4 == 'r' || c4 == 'R');

                    if(pawnTransformQueen)
                        move.endType = QUEEN;
                    else if(pawnTransformBishop)
                        move.endType = BISHOP;
                    else if(pawnTransformKnight)
                        move.endType = KNIGHT;
                    else if(pawnTransformRook)
                        move.endType = ROOK;
                    else
                        move.endType = piece->type;

                    moveList.push_back(move);
                    m++;
                }
                else
                {
                    std::cerr << "ERROR: The move " << c << c1 << c2 << c3 << " is invalid: No ChessPiece at start position!\n";
                    return m;
                }
            }

            n += 4;
        }

        n++;
    }

    return m;
}

void ChessUCI::executePerftCommand()
{
    //Perft-Test
    try
    {
        //Get the requested Depth.
        const int depth = std::stoi(inputParameters[0]);

        //Get the requested MoveList.
        std::vector<ChessMove> moveList{};
        int nMoves = extractChessMoves(inputParameters[1].c_str(),moveList);

        //If no Movelist was provided, use moveListLegal.
        if(nMoves <= 0)
        {
            nMoves = mainEngine.nMovesLegal[mainEngine.turnColour];
            moveList.resize(nMoves);
            for(int m=0; m<nMoves; m++)
            {
                moveList[m] = mainEngine.moveListLegal[mainEngine.turnColour][m];
            }
        }

        //Lambda to create a thread
        auto createThread
        {
            [depth, moveList]() mutable
            {
                perftMainThread(depth, Chess::nThreads, moveList, threadList.size()-1);
            }
        };

        //Initalize the perft-Search.
        threadDone.emplace_back(false);
        threadList.emplace_back(createThread);
    }

    catch(const std::invalid_argument &e)
    {
        e;
        std::cerr << "ERROR: Invalid Parameter for perft!\n";
    }

    catch(const std::out_of_range &e)
    {
        e;
        std::cerr << "ERROR: Invalid Parameter for perft! int Overflow.\n";
    }
}

void ChessUCI::perftMainThread(const int depth, int nCores, const std::vector<ChessMove> &moveList, const std::size_t threadID)
{
    threadDone[threadID] = false;

    if(nCores <= 0)
        nCores = 1;

    mtx.lock();
    const double tStart = Chess::CLOCK.getTime();
    std::cout << "info string Threads used: " << nCores << '\n';
    mtx.unlock();

    //Vector to store the results in.
    std::vector<PerftResult> perftResults;

    //If only one Core is used, creating a new thread is unnecessary.
    const int nMoves = (int)moveList.size();
    if(nCores == 1 || nMoves < nCores)
    {
        perftTest(depth, moveList, perftResults, 0);

        //Print the Results
        std::lock_guard<std::mutex> lock(mtx);
        if(stopThread.load())
            std::cout << "Search aborted! Nodes found so far:\n";

        u64 sum = 0;
        const std::size_t nMovesFound = perftResults.size();
        for(std::size_t m=0; m<nMovesFound; m++)
        {
            sum += perftResults[m].nodes;
        }

        const double time = Chess::CLOCK.getTime() - tStart;
        std::cout << "\nNodes:\t" << sum << '\n';
        std::cout << "Time:\t" << time << "s\n";
        std::cout << "Speed:\t" << sum/time << " nodes/s\n\n";

        threadDone[threadID] = true;
    }

    //New ThreadList to do the perft-test with.
    std::vector<std::thread> perftThreadList;

    //Distribute the Moves among the Cores.
    const int movesPerCore = nMoves/nCores;
    int movesLeft = nMoves - (movesPerCore * nCores);
    int offset = 0;

    for(std::size_t n=0; n<nCores; n++)
    {
        //Determine how many Moves this Core should handle.
        int nMovesCore = movesPerCore;
        if(movesLeft > 0)
        {
            ++nMovesCore;
            --movesLeft;
        }

        //Create the Move-List.
        std::vector<ChessMove> moveListCore(nMovesCore);
        for(std::size_t m=0; m<nMovesCore; m++)
        {
            moveListCore[m] = moveList[m + offset];
        }
        offset += nMovesCore;

        //Lambda to create a thread
        auto createThread
        {
            [depth, moveListCore, &perftResults, n]()
            {
                perftTest(depth, moveListCore, perftResults, n);
            }
        };

        //Create the Thread
        perftThreadList.emplace_back(createThread);
    }

    //Wait for all threads to finish
    for(int n=0; n<nCores; n++)
    {
        perftThreadList[n].join();
    }

    //Print the Results
    std::lock_guard<std::mutex> lock(mtx);
    if(stopThread.load())
        std::cout << "Search aborted! Nodes found so far:\n";

    u64 sum = 0;
    const std::size_t nMovesFound = perftResults.size();
    for(std::size_t m=0; m<nMovesFound; m++)
    {
        sum += perftResults[m].nodes;
    }

    const double time = Chess::CLOCK.getTime() - tStart;
    std::cout << "\nNodes:\t" << sum << '\n';
    std::cout << "Time:\t" << time << "s\n";
    std::cout << "Speed:\t" << sum/time << " nodes/s\n\n";

    threadDone[threadID] = true;
}

void ChessUCI::perftTest(const int depth, std::vector<ChessMove> moveList, std::vector<PerftResult> &perftResults, const std::size_t threadID)
{
    auto testEngine{ std::make_unique<Engine>() };  //Board on which the perft-Test will be made. Needed for multiple threads.
    testEngine->initialize();
    testEngine->loadFEN(mainEngine.getFEN().c_str());
    const std::size_t startTurn = testEngine->turnCounter;

    const std::size_t nMoves = moveList.size();
    for(std::size_t m=0; m<nMoves; m++)
    {
        if(stopThread.load())
            break;

        std::vector<ChessMove> move{ moveList[m] };
        u64 result = perftDepthTest(depth, depth, move, testEngine);

        mtx2.lock();
        perftResults.push_back( { result, moveList[m] } );
        mainEngine.printMove(moveList[m]);
        std::cout << ": " << result << '\n';
        mtx2.unlock();

        testEngine->loadBoardState(startTurn);
    }
}

u64 ChessUCI::perftDepthTest(int depth, const int maxDepth, const std::vector<ChessMove> &moveList, std::unique_ptr<Engine> &engine)
{
    if(depth<=0 || stopThread.load())
        return 1;

    engine->saveBoardState(engine->turnCounter + maxDepth - depth);

    u64 nodes =  0;

    for( const ChessMove &move : moveList )
    {
        if(stopThread.load())
            return nodes;

        const bool isLegal = engine->makeMove(move);

        if(isLegal)
        {
            engine->turnColour = (engine->turnColour == PWHITE) ? PBLACK : PWHITE;

            std::vector<ChessMove> newMoveList{};
            const PColour c = engine->turnColour;
            newMoveList.assign(engine->moveListPseudo[c], engine->moveListPseudo[c] + engine->nMovesPseudo[c]);

            u64 newNodes = perftDepthTest(depth-1, maxDepth, newMoveList, engine);
            nodes += newNodes;
        }

        engine->loadTestBoardState(engine->turnCounter + maxDepth - depth);
    }

    return nodes;
}

void ChessUCI::executeGoCommand()
{
    //Find the next Move
    try
    {
        //Get the requestes Depth.
        int depth = 5;
        if(inputParameters[0] != "NO_OPTION" && inputParameters[0] != "NO_PARAMETER")
            depth = std::stoi(inputParameters[0]);
        else
            depth = Chess::botList[mainEngine.turnColour].depth;

        //Get the Bottype
        BotTypes bottype = OPTIMUMBOT2;
        if(inputParameters[2] != "NO_OPTION" && inputParameters[2] != "NO_PARAMETER")
        {
            if(inputParameters[2] == "random")
                bottype = RANDOMBOT;
            else if(inputParameters[2] == "metro")
                bottype = METROPOLISBOT;
            else if(inputParameters[2] == "fool")
                bottype = WEIGHTEDRANDOMBOT1;
            else if(inputParameters[2] == "jester")
                bottype = WEIGHTEDRANDOMBOT2;
            else if(inputParameters[2] == "novice")
                bottype = OPTIMUMBOT1;
            else if(inputParameters[2] == "master")
                bottype = OPTIMUMBOT1;
            else
            {
                std::cout << "info Warning: Unknown Bottype! Master-Bot will be used.\n";
                bottype = OPTIMUMBOT2;
            }
        }
        else
            bottype = Chess::botList[mainEngine.turnColour].botType;

        //Only OptimumBot2 can make use of a custom movelist.
        if(bottype == OPTIMUMBOT2)
        {
            //Get the requested MoveList.
            std::vector<ChessMove> moveList{};
            int nMoves = extractChessMoves(inputParameters[1].c_str(), moveList);

            //If no Movelist was provided, use moveListLegal.
            if(nMoves <= 0)
            {
                const PColour l = mainEngine.turnColour;
                nMoves = mainEngine.nMovesLegal[l];
                moveList.resize(nMoves);
                for(int m=0; m<nMoves; m++)
                {
                    moveList[m] = mainEngine.moveListLegal[l][m];
                }
            }

            //Lambda to create a thread
            auto createThread
            {
                [depth,moveList]()
                {
                    findBestMove(depth,moveList, threadList.size()-1);
                }
            };

            //Create the Thread
            threadDone.emplace_back(false);
            threadList.emplace_back(createThread);
        }
        else
        {
            //Warn the User if they requested a movelist.
            if(inputParameters[1] != "NO_OPTION" && inputParameters[1] != "NO_PARAMETER")
                std::cout << "info Warning: Only bottype optimum2 can make use of the searchmoves command!\n";

            //Lambda to create a thread
            auto createThread
            {
                [depth,bottype]()
                {
                    findNextMove(depth,bottype, threadList.size()-1);
                }
            };

            //Create the Thread
            threadDone.emplace_back(false);
            threadList.emplace_back(createThread);
        }
    }

    catch(const std::invalid_argument &e)
    {
        e;
        std::cerr << "ERROR: Invalid Parameter for go depth! Specify the depth with int.\n";
    }

    catch(const std::out_of_range &e)
    {
        e;
        std::cerr << "ERROR: Invalid Parameter for go depth! int Overflow.\n";
    }
}

void ChessUCI::findNextMove(int depth, BotTypes bottype, const std::size_t threadID)
{
    threadDone[threadID] = false;
    auto bot = std::make_unique<Bot>(mainEngine.turnColour);
    bot->depth = depth;
    bot->botType = bottype;
    bot->generateMove();
    while(bot->searching);
    std::cout << "bestmove ";
    mainEngine.printMove(bot->nextMove);
    std::cout << '\n' << '\n';
    threadDone[threadID] = true;
}

void ChessUCI::findBestMove(int depth, std::vector<ChessMove> moveList, const std::size_t threadID)
{
    auto bot = std::make_unique<Bot>(mainEngine.turnColour);
    bot->depth = depth;

    //Find the best Move.
    bot->findOptimumMove(moveList);

    std::cout << "bestmove ";
    mainEngine.printMove(bot->nextMove);
    std::cout << '\n' << '\n';
    threadDone[threadID] = true;
}