#pragma once
#include "ChessEngine.h"

namespace ChessUCI
{
	using namespace Chess;

	//Stores User-Input in inputString and translates it into a UCI-command.
	void inputUCI();

	//Takes inputString, searches for the Options set by command and stores any parameters in inputParameters.
	bool UCIsetCommandParameters(std::string_view command, std::vector<std::string_view> options); //Sets inputCommand and inputParameters. Returns false if command != inputCommand.

	//Display all available commands.
	void executeHelpCommand();

	//Display Information such as the current board or Dangerzones.
	void executeDisplayCommand();

	//Executes the position-Command and loads the appropriate Boardstate.
	void executePositionCommand();

	//Takes a String with ChessMoves and converts it to an Array of ChessMoves. Returns the number of moves found.
	int extractChessMoves(const char *moveString, std::vector<ChessMove> &moveList);

	//Executes the go perft Command.
	void executePerftCommand();

	void findNextMove(int depth, BotTypes bottype, const std::size_t threadID);

	void findBestMove(int depth, std::vector<ChessMove> moveList, const std::size_t threadID);

	//Perft-Test Functions.

	//To store the perft results.
	struct PerftResult
	{
		u64 nodes;
		ChessMove move;
	};

	//A main-thread that handles several perft-Test threads.
	void perftMainThread(const int depth, int nCores, const std::vector<ChessMove> &moveList, const std::size_t threadID);

	//Call to perform a Perft-Test with depth and store the results.
	void perftTest(const int depth, std::vector<ChessMove> moveList, std::vector<PerftResult> &perftResults, const std::size_t threadID);

	//Perform the actual Perft-Test iteratively.
	u64 perftDepthTest(int depth, const int maxDepth, const std::vector<ChessMove> &moveList, std::unique_ptr<Engine> &engine);

	//Execute the regular go Command.
	void executeGoCommand();

	//UCI Variables
	inline std::string inputString;						//Stores the User-Input.
	inline std::vector<std::string> inputParameters;	//A Vector containing the Parameters from the User-Input.
	inline std::atomic<bool> stopMainThread{false};		//To stop the main UCI thread.
}