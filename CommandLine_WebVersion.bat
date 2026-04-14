cd C:\Users\samue\source\repos\ChessGUI

emcc -o index.html src\ChessEngine.cpp src\ChessGUI.cpp src\ChessUCI.cpp src\LevelChessboard.cpp src\LevelOptions.cpp src\LevelSetupChessboard.cpp src\LevelStartMenu.cpp src\Main.cpp src\Raylib_GameHeader.cpp ^
 -Os -std=c++20 C:\raylib\raylib\src\web\libraylib.a ^
 -I. -I C:\raylib\raylib\src -I C:\raylib\raylib\src\external -I C:\Users\samue\source\repos\ChessGUI\src ^
 -L. -L C:\raylib\raylib\src ^
 -s USE_GLFW=3 ^
 -s TOTAL_MEMORY=67108864 ^
 -s USE_PTHREADS=1 ^
 --preload-file Sprites ^
 --shell-file C:\raylib\raylib\src\shell.html C:\raylib\raylib\src\web\libraylib.a ^
 -DPLATFORM_WEB