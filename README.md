# ChessGUI

## Overview

A simple chess program done to further learn C++ and its features such as threads.

Includes beginner-friendly features such as the option to display the attack range of a player or the movement range of individual pieces.

## File Structure

- The 'ChessEngine' files provides the calculations for the chess game itself. They do not require the raylib library and could be reused for a potential UCI-compliant CLI version.

- The 'Timer-Header.h' file provides a stopwatch class.

- The 'Random_Header.h' file provides functions to generate random numbers.

- The 'Raylib_GameHeader' files provide basic gamedev functionality that could potentionally be reused for future projects that make use of the raylib library.

- The 'ChessGUI' files are responsible for loading and unloading the game as well as managing global ressources.

- The 'Level...' files all handle player input and the graphical output for the different menus of the game.

  - The 'LevelStartMenu' files manage the start screen that is shown when starting the game.

  - The 'LevelOptions' files manage the option menu.

  - The 'LevelChessboard' files display the chess game based on the internal state of 'ChessEngine' and also manage player input and the UI elements.
