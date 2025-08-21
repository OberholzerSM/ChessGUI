# ChessGUI

## Overview

A simple chess program done to further learn C++ and its features such as threads.

## Credit

- Game made using the Raylib library.  
 Raylib is licensed under the zlib/libpng license. Copyright (c) 2013-2025 Ramon Santamaria (@raysan5).

- Chess sprites made by: jurgenwesterhof (adapted from work of Cburnett).  
 File is licensed under the Creative Commons Attribution-Share Alike 3.0 Unported license.  
 https://commons.wikimedia.org/wiki/File:Chess_Pieces_Sprite.svg (20.08.2025)

## Features

- Beginner-friendly features such as showing which squares a chesspiece can move to and the option to display the attack range of both sides.

- The ability to rewind turns.

- Six different Bots, which use various random means to pick their next move (safe for one bot that simply picks the best move).

## Console Commands

It is possible to issue commands to the terminal.

- ‘quit’, ‘q’, ‘exit’ or ‘close’:
	- Close the program.\n
    
- ‘stop’:
	- Abort all ongoing calculations.\n


- ‘go’:
	- Let the engine calculate the next move with the current settings.

- ‘go depth [depth-value]’:
	- Let the engine calculate the next move with a given depth.

- ‘go searchmoves [move1] [move2] ...’:
	- Let the engine find the best move among a list of moves.
	- Warning: This option only works for the master bot!

- ‘go bottype [bot-name]’:
	- Let one of the other bots determine the next move.


- ‘position’:
	- Sets up a given chess position.

- ‘position startpos’:
	- Returns the board to the default start position.

- ‘position testpos [i]’:
	- Sets the board to one of 6 available test positions.

- ‘position fen [fen-String]’:
	- Sets the board according to a custom fen-String.

- ‘position moves [move1] [move2] ...’:
	- Plays out a series of moves from the current chess position. Can be combined with the other position commands.
	- Warning: This allows you to play illegal moves too! Empty fields cannot be moved.

- ‘d’ or ‘display’:
	- Show the current state of the chessboard. Default is displaying the chessboard itself.

- ‘d board’ or ‘display board’:
	- Display the chessboard itself.

- ‘d movementrange’ or ‘display movementrange’:
	- Display the movement range of all chesspieces.

- ‘d attackrange’ or ‘display attackrange’:
	- Display the attack range of all chesspieces.

- ‘perft depth [depth-value]’:
	- Perform a perft-search to find the number of available positions after depth moves.
- ‘perft depth [depth-value] searchmoves [move1] [move2] ...’:
	- Perform a perft-search limited to a list of moves.


## File Structure

- The 'ChessEngine' files provides the calculations for the chess game itself.

- The 'Timer-Header.h' file provides a stopwatch class.

- The 'Random_Header.h' file provides functions to generate random numbers.

- The 'Raylib_GameHeader' files provide basic gamedev functionality that could potentionally be reused for future projects that make use of the raylib library.

- The 'ChessGUI' files are responsible for loading and unloading the game as well as managing global ressources.

- The "ChessUCI" files handle console commands.

- The 'Level...' files all handle player input and the graphical output for the different menus of the game.

  - The 'LevelStartMenu' files manage the start screen that is shown when starting the game.

  - The 'LevelOptions' files manage the option menu.

  - The 'LevelChessboard' files display the chess game based on the internal state of 'ChessEngine' and also manage player input and the UI elements.
 
  - The 'LevelSetupChessboard' files manage the mode where you can setup your own chess positions.
