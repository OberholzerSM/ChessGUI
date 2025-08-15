#include "LevelStartMenu.h"
#include "LevelChessboard.h"
#include "LevelOptions.h"
#include "ChessGUI.h"
using namespace Chess;
using namespace Raylib;

//Public Level Functions

void LevelChessboard::load()
{
	//Switch the currently active level.
	LevelID = this->ID();
	ColorBackground = DARKGRAY;

	if(!loaded)
	{
		//Reset the board.
		reset();
		tReset = Raylib::CLOCK.getTime() + 100.0; //Do not wait.

		//Resize the Sprites.
		Resources::textureSprites.setSize(squareSize);
		Resources::arrowTexture.setSize(squareSize);
		Resources::cursorTexture.setSize(squareSize);

		//Set the Sprite Positions.
		setChessPieceSpritePos();

		loaded = true;
	}
}

void LevelChessboard::input()
{
	const bool active = !pptActive && !drawGameOverIsActive && !mainEngine.requestDraw;

	inputMouse();
	inputPause();
	if(active || paused)
	{
		inputFlipButton();
		inputDangerZoneButtons();
		inputHomeButton();
		inputResetButton();
		inputArrowButtons();
	}
	inputDrawRequest();
	inputPPT();
	inputBots();
	moveChessPieces();
}

void LevelChessboard::draw()
{
	if(loaded)
	{
		const bool active = !pptActive && !drawGameOverIsActive && !mainEngine.requestDraw;

		drawBoardSquares();
		drawBoardLines();
		drawTurnCounter();
		drawButtons();
		if(active || paused)
		{
			drawChessPieceMovements();
			drawDangerZone();
			drawCursor();
		}
		drawChessPieces();
		drawSpeechBubble();
		if(paused)
			drawRectangle({boardOrigin.x,boardOrigin.y, 8.0f*squareSize.x, 8.0f*squareSize.y}, greyOutColor);
		drawGameOver();
		drawDrawRequest();
		drawPPT();
		if(pptActive)
			drawCursor();
	}
}

void LevelChessboard::unload()
{
	if(loaded)
	{
		resetBots();
		loaded = false;
	}
}

//Actions

Chess::BoardPos LevelChessboard::getMouseSquare(bool flip) const
{
	const Vector2 MousePos = {(float)GetMouseX() / Window.widthf, (float)GetMouseY()  / Window.heightf};
	BoardPos MouseBoardPos{};

	if(MousePos.x <= boardOrigin.x || MousePos.x >= boardOrigin.x + 8.0f * squareSize.x)
		MouseBoardPos.i = 10;
	else
		MouseBoardPos.i = (int)((MousePos.x - boardOrigin.x) / squareSize.x);

	if(MousePos.y <= boardOrigin.y || MousePos.y >= boardOrigin.y + 8.0f * squareSize.y)
		MouseBoardPos.j = 10;
	else
		MouseBoardPos.j = (int)((MousePos.y - boardOrigin.y) / squareSize.y);

	if(flip && MouseBoardPos.i != 10 && MouseBoardPos.j != 10)
	{
		MouseBoardPos.i = 7 - MouseBoardPos.i;
		MouseBoardPos.j = 7 - MouseBoardPos.j;
	}

	return MouseBoardPos;
}

void LevelChessboard::setChessPieceSpritePos() const
{
	for(int l=0; l<2; l++)
	{
		for(int k=0; k< mainEngine.nAlive[l]; k++)
		{
			ChessPiece *piece = mainEngine.piecesListAlive[l][k];
			Vector2 pos = {(float)(piece->pos.i)*squareSize.x + boardOrigin.x, (float)(piece->pos.j)*squareSize.y + boardOrigin.y};
			if(flipBoard)
				pos = flipVector2(pos);

			piece->drawDestinationX = pos.x;
			piece->drawDestinationY = pos.y;
			piece->drawX = pos.x;
			piece->drawY = pos.y;
		}
	}
}

void LevelChessboard::setChessPieceSpriteDestination() const
{
	for(int l=0; l<2; l++)
	{
		for(int k=0; k< mainEngine.nAlive[l]; k++)
		{
			ChessPiece *piece = mainEngine.piecesListAlive[l][k];
			Vector2 pos = {(float)(piece->pos.i)*squareSize.x + boardOrigin.x, (float)(piece->pos.j)*squareSize.y + boardOrigin.y};
			if(flipBoard)
				pos = flipVector2(pos);

			piece->drawDestinationX = pos.x;
			piece->drawDestinationY = pos.y;
		}
	}
}

void LevelChessboard::pickupPiece(const BoardPos &start) const
{
	ChessPiece *piece = mainEngine.board[start.i][start.j];

	if(piece != nullptr && piece->colour == mainEngine.turnColour && piece->alive)
	{
		piece->selected = true;
	}
}

void LevelChessboard::placedownPiece(const Chess::BoardPos &start, const Chess::BoardPos &end)
{
	ChessPiece *piece = mainEngine.board[start.i][start.j];
	if(piece != nullptr && piece->selected)
	{
		piece->selected = false;

		if(bitboardCheck(piece->movement, end))
		{
			const int jEnd = (piece->colour == PWHITE) ? 0 : 7;
			bool pawnTransform = (piece->type == PAWN && jEnd == end.j);

			if(!pawnTransform)
			{
				const int weightOld = mainEngine.getBoardWeight();
				const Chess::ChessMove move = {start, end, piece->type};
				mainEngine.makeMove(move);
				mainEngine.advanceTurn();
				mainEngine.updateAttackZone();
				commentCheck();
				commentValidMoves(weightOld);

				if(mainEngine.checkmate[0] || mainEngine.checkmate[1] || mainEngine.isdraw)
					drawGameOverIsActive = true;
			}
			else if(playerControll[mainEngine.turnColour])
			{
				pptAssignPawn(start, end);
			}

		}
		else
		{
			commentInvalidMoves(piece, start, end);
		}

		setChessPieceSpritePos();
	}
}

Vector2 LevelChessboard::flipVector2(Vector2 pos) const
{
	pos.x = 2.0f*boardOrigin.x + 7.0f*squareSize.x - pos.x;
	pos.y = 2.0f*boardOrigin.y + 7.0f*squareSize.y - pos.y;
	return pos;
}

void LevelChessboard::reset()
{
	resetBots();

	mainEngine.maxTurns = mainEngine.turnCounterStart;
	mainEngine.loadBoardState(mainEngine.turnCounterStart);
	mainEngine.updateAttackZone();

	for(int l=0; l<2; l++)
	{
		for(int k=0; k<nPieces; k++)
		{
			ChessPiece *piece = mainEngine.piecesList[l][k];
			piece->showMovement = false;
			piece->selected = false;
		}
	}

	setChessPieceSpriteDestination();

	paused = false;
	pptActive = false;
	speechBubbleActive = false;
	dialogueCounter = 1;

	tReset = Raylib::CLOCK.getTime();
}

void LevelChessboard::moveChessPiece(Chess::ChessPiece *piece) const
{
	const Vector2 v = {speed / Window.widthf, speed / Window.heightf};
	const Vector2 xDraw = {piece->drawX, piece->drawY};
	const Vector2 xDest = {piece->drawDestinationX, piece->drawDestinationY};

	const bool xArrived = (xDest.x - v.x <= xDraw.x) && (xDraw.x <= xDest.x + v.x);
	const bool yArrived = (xDest.y - v.y <= xDraw.y) && (xDraw.y <= xDest.y + v.y);

	const float dx = xDest.x - xDraw.x, dy = xDest.y - xDraw.y;
	const float r = sqrtf(powf(dx, 2.0f) + powf(dy, 2.0f));
	const float speedX = dx*v.x/r, speedY = dy*v.y/r;

	if(xArrived && yArrived)
	{
		piece->drawX = piece->drawDestinationX;
		piece->drawY = piece->drawDestinationY;
	}
	else if(!paused)
	{
		if(!xArrived)
			piece->drawX += speedX;

		if(!yArrived)
			piece->drawY += speedY;
	}
}

void LevelChessboard::moveChessPieces()
{
	for(int l=0; l<2; l++)
	{
		for(int k=0; k< mainEngine.nAlive[l]; k++)
		{
			ChessPiece *piece = mainEngine.piecesListAlive[l][k];
			moveChessPiece(piece);
		}
	}
}

void LevelChessboard::speak(const std::string &text, Chess::ChessPiece *piece)
{
	speechBubbleCP = piece;
	speechbubble.changeText(text);
	speechBubbleActive = true;
	tStart = Raylib::CLOCK.getTime();
}

void LevelChessboard::moveBubbleSprite()
{
	const Vector2 d = {0.1f*squareSize.x, 0.15f*squareSize.y}; //Offset.
	Vector2 xBubbleDraw = {speechBubbleCP->drawX + 0.5f*squareSize.x, speechBubbleCP->drawY + d.y};

	//Flip the Speechbubble depending on which half of the board the chesspiece is on.
	if(!speechbubble.fliped)
	{
		xBubbleDraw.x += d.x;

		if(speechBubbleCP->drawX > 3.0f*squareSize.x)
			speechbubble.flip();
	}
	else if(speechbubble.fliped)
	{
		xBubbleDraw.x -= d.x;
		if(speechBubbleCP->drawX <= 3.0f*squareSize.x)
			speechbubble.flip();
	}

	speechbubble.moveTail(xBubbleDraw);

	//Adjust the Position if it is outside the board.
	Vector2 pos = speechbubble.getBubblePos();
	const Vector2 size = speechbubble.getBubbleSize();

	if(pos.x < boardOrigin.x)
		pos.x = 0.0f;
	else if(pos.x + size.x > boardOrigin.x + 8.0f*squareSize.x)
		pos.x = boardOrigin.y + 8.0f*squareSize.x - size.x;

	if(pos.y < boardOrigin.y)
		pos.y = 0.001f;
	else if(pos.y + size.y > boardOrigin.y + 8.0f*squareSize.y)
		pos.y = boardOrigin.y + 8.0f*squareSize.y - size.y;

	speechbubble.move(pos);
}

void LevelChessboard::commentInvalidMoves(Chess::ChessPiece *piece, const Chess::BoardPos &start, const Chess::BoardPos &end)
{
	if(bitboardCheck(piece->movementPseudo, end))
	{
		if(piece->type == KING && piece->colour == PWHITE)
		{
			speak("That would be foolish!", piece);
		}
		else if(piece->type == KING && piece->colour == PBLACK)
		{
			speak("Do you want to get me killed?!", piece);
		}
		else if(piece->colour == PWHITE && mainEngine.kingWhite.checkCheck())
		{
			speak("We must aid the king!", piece);
		}
		else if(piece->colour == PBLACK && mainEngine.kingBlack.checkCheck())
		{
			speak("The king needs us!", piece);
		}
		else
		{
			speak("That would leave the king unguarded!", piece);
		}
	}
	else if(mainEngine.board[end.i][end.j] != nullptr && !(start.i == end.i && start.j == end.j) && (piece->colour == mainEngine.board[end.i][end.j]->colour))
	{
		speak("This is treason!", piece);
	}
	else if(piece->type == PAWN && mainEngine.board[end.i][end.j]==nullptr && piece->colour == PWHITE && end.j == start.j+1)
	{
		speak("That would be cowardice!", piece);
	}
	else if(piece->type == PAWN && mainEngine.board[end.i][end.j]==nullptr && piece->colour == PBLACK && end.j == start.j-1)
	{
		speak("No step back!", piece);
	}
}

void LevelChessboard::commentCheck()
{
	constexpr int n_lines{3};

	if(mainEngine.checkmate[PWHITE])
		speak("I give up!", &mainEngine.kingWhite);
	else if(mainEngine.checkmate[PBLACK])
		speak("I surrender!", &mainEngine.kingBlack);
	else if(mainEngine.isdraw)
	{
		if(mainEngine.kingWhite.nMoves == 0)
			speak("I am stuck!", &mainEngine.kingWhite);
		else if(mainEngine.kingBlack.nMoves == 0)
			speak("I am trapped!", &mainEngine.kingBlack);
	}
	else
	{
		if(mainEngine.kingWhite.checkCheck())
		{
			checkCounter[PWHITE]++;
			switch(checkCounter[PWHITE]%n_lines)
			{
			case(1):
				speak("Help!", &mainEngine.kingWhite);
				break;

			case(2):
				speak("I am in grave danger!", &mainEngine.kingWhite);
				break;

			case(0):
				speak("Where are my troops?", &mainEngine.kingWhite);
				break;

			default:
				break;
			}
		}

		if(mainEngine.kingBlack.checkCheck())
		{
			checkCounter[PBLACK]++;
			switch(checkCounter[PBLACK]%n_lines)
			{
			case(1):
				speak("Hmpf!", &mainEngine.kingBlack);
				break;

			case(2):
				speak("You have yet to catch me!", &mainEngine.kingBlack);
				break;

			case(0):
				speak("Rally to your king!", &mainEngine.kingBlack);
				break;

			default:
				break;
			}
		}
	}
}

void LevelChessboard::commentValidMoves(int weightOld)
{
	//Determine which King should speak.
	const PColour turnColour = mainEngine.turnColour;
	ChessPiece *king = mainEngine.piecesList[turnColour][KING];

	//Increase Dialogue Counter.
	constexpr int nLines = 3;
	if( (dialogueCounter % 2) != (int)turnColour )
		++dialogueCounter;
	dialogueCounter = ++dialogueCounter % (2*nLines);

	//Determine the Weight difference.
	const int sign = (turnColour == PWHITE ? -1 : +1 );
	const int dw = sign*(mainEngine.getBoardWeight() - weightOld);

	if(dw >= pieceValue[mainEngine.lateGame][QUEEN])
	{
		if(turnColour==PWHITE)
			speak("Gasp!", king);
		else if(turnColour==PBLACK)
			speak("Impossible!", king);
	}
	else if(turnColour < PNONE && mainEngine.turnCounter > 0 && mainEngine.nAlive[mainEngine.turnColour] < mainEngine.boardStateList[mainEngine.turnCounter-1].nAlive[mainEngine.turnColour])
	{
		switch(dialogueCounter)
		{
		case 1:
			speak("Oh no!", king);
			break;

		case 2:
			speak("Curses!", king);
			break;

		case 3:
			speak("Not good...", king);
			break;

		case 4:
			speak("You will pay for this!", king);
			break;

		case 5:
			speak("That was one of my bravest warriors!", king);
			break;

		case 0:
			speak("Not my mighty warrior!", king);
			break;
		}
	}
	else if(dw < -pieceValue[mainEngine.lateGame][PAWN]/2)
	{
		switch(dialogueCounter)
		{
		case 1:
			speak("You think that wise?", king);
			break;

		case 2:
			speak("How foolish!", king);
			break;

		case 3:
			speak("Huh?", king);
			break;

		case 4:
			speak("I got you now!", king);
			break;

		case 5:
			speak("Peculiar choice.", king);
			break;

		case 0:
			speak("Victory is as good as mine!", king);
			break;
		}
	}
}

//Pawn-Transform-Panel

void LevelChessboard::pptAssignPawn(const Chess::BoardPos &start, const Chess::BoardPos &end)
{
	pptPawn = mainEngine.board[start.i][start.j];

	if(pptPawn!=nullptr)
	{
		pptActive = true;
		cursorActive = true;

		pptStart = start;
		pptEnd = end;

		mainEngine.turnColourOld = mainEngine.turnColour;
		mainEngine.turnColour = PNONE;

		pptxDraw.x = (float)(end.i)*squareSize.x + boardOrigin.x;
		pptxDraw.y = (float)(end.j)*squareSize.y + boardOrigin.y;

		if(end.j < 4)
			pptSign = (flipBoard ? -1.0f : 1.0f);
		else
			pptSign = (flipBoard ? 1.0f : -1.0f);

		const Vector2 drawPos = flipBoard ? flipVector2(pptxDraw) : pptxDraw;

		queenButton.initialize(drawPos, squareSize);
		bishopButton.initialize({drawPos.x,drawPos.y + pptSign*squareSize.y}, squareSize);
		knightButton.initialize({drawPos.x,drawPos.y + 2.0f*pptSign*squareSize.y}, squareSize);
		rookButton.initialize({drawPos.x,drawPos.y + 3.0f*pptSign*squareSize.y}, squareSize);
	}
	else
		std::cerr << "ERROR PawnTransformPanel::assignPawn: Tried to assign Null-Pointer!\n";
}

void LevelChessboard::pptTransformPawn(Chess::PType type)
{
	mainEngine.turnColour = mainEngine.turnColourOld;
	mainEngine.makeMove({pptStart,pptEnd,type});
	mainEngine.advanceTurn();
	setChessPieceSpritePos();

	pptPawn = nullptr;
	pptActive = false;
	cursorActive = false;
}

//Bot-Functions

void LevelChessboard::resetBots()
{
	stopAllThreads();
	botList[PWHITE].reset();
	botList[PBLACK].reset();
}

void LevelChessboard::botThink(Chess::Bot &bot, double tStartBotSearch)
{
	//Don't display the thinking animation unless it takes too long.
	const double t = Raylib::CLOCK.getTime() - tStartBotSearch;
	if(t <= duration)
		return;

	//Cycle through the dots.
	const int nDots = (int)t%3;
	switch(nDots)
	{
	case 0:
		speak(". ", mainEngine.piecesList[bot.botColour][KING]);
		break;

	case 1:
		speak(".. ", mainEngine.piecesList[bot.botColour][KING]);
		break;

	case 2:
		speak("... ", mainEngine.piecesList[bot.botColour][KING]);
		break;
	}
}

void LevelChessboard::botSetSpriteDest(Chess::Bot &bot, double tStartBotSearch)
{
	//Deactivate the speechbubble, if the bot was displaying the thinking animation.
	const double t = Raylib::CLOCK.getTime() - tStartBotSearch;
	if(t > duration)
		speechBubbleActive = false;

	const BoardPos start = bot.nextMove.start;
	ChessPiece *piece = mainEngine.board[start.i][start.j];
	if(piece == nullptr)
	{
		std::cerr << "ERROR inputBots: Generated move for nullptr! ";
		mainEngine.printMove(bot.nextMove);
		std::cout << '\n';
		return;
	}

	const BoardPos end = bot.nextMove.end;
	Vector2 pos = {(float)(end.i)*squareSize.x + boardOrigin.x, (float)(end.j)*squareSize.y + boardOrigin.y};
	if(flipBoard)
		pos = flipVector2(pos);
	piece->drawDestinationX = pos.x;
	piece->drawDestinationY = pos.y;

	bot.spriteMoved = true;
}

void LevelChessboard::botTestSpriteDest(Chess::Bot &bot)
{
	const BoardPos start = bot.nextMove.start;
	ChessPiece *piece = mainEngine.board[start.i][start.j];
	if(piece == nullptr)
	{
		std::cerr << "ERROR inputBots: Tried to move Sprite of a nullptr!\n";
		return;
	}
	const Vector2 v = {speed / Window.widthf, speed / Window.heightf};
	const Vector2 xDraw = {piece->drawX, piece->drawY};
	const Vector2 xDest = {piece->drawDestinationX, piece->drawDestinationY};

	const bool xArrived = (xDest.x - v.x <= xDraw.x) && (xDraw.x <= xDest.x + v.x);
	const bool yArrived = (xDest.y - v.y <= xDraw.y) && (xDraw.y <= xDest.y + v.y);

	bot.spriteArrived = xArrived && yArrived;
}

//Input-Functions

void LevelChessboard::inputPause()
{
	//Pausing the Game
	if(IsKeyReleased(KEY_SPACE))
	{
		//Pausing and unpausing.
		const bool active = !pptActive && !drawGameOverIsActive && !mainEngine.requestDraw;

		if(!paused && active)
		{
			paused = true;
			mainEngine.turnColourOld = mainEngine.turnColour;
			mainEngine.turnColour = PNONE;
		}
		else if(paused)
		{
			paused = false;
			mainEngine.turnColour = mainEngine.turnColourOld;
			mainEngine.maxTurns = mainEngine.turnCounter;

			if(mainEngine.checkmate[0] || mainEngine.checkmate[1] || mainEngine.isdraw)
				drawGameOverIsActive = true;
		}
		
		if(active)
			drawGameOverIsActive = false;
	}
}

void LevelChessboard::inputMouse()
{
	static BoardPos start = {0,0};
	static ChessPiece *piece = nullptr;

	if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		drawGameOverIsActive = false;
		start = getMouseSquare(flipBoard);
		if(start.i >= 0 && start.i < 8 && start.j >= 0 && start.j < 8)
		{
			piece = mainEngine.board[start.i][start.j];
			if(piece != nullptr)
			{
				if(playerControll[piece->colour])
					pickupPiece(start);

				//Draw movement when you click on an enemy chesspiece.
				if(!piece->showMovement && !pptActive)
					piece->showMovement = true;
				else
					piece->showMovement = false;
			}
		}
	}
	else if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		if(piece != nullptr && piece->selected)
		{
			//Draw the cursor.
			cursorActive = true;

			//Move the Sprite with the Mouse
			piece->drawX = ((float)(GetMouseX()) / Window.widthf) - 0.5f*squareSize.x;
			piece->drawY = ((float)(GetMouseY()) / Window.heightf)  - 0.5f*squareSize.y;
			piece->drawDestinationX = piece->drawX;
			piece->drawDestinationY = piece->drawY;
		}
	}
	else if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
	{
		if(piece != nullptr && piece->selected)
		{
			//Deactivate the cursor.
			cursorActive = false;

			const BoardPos end = getMouseSquare(flipBoard);
			if(end.i >= 0 && end.i < 8 && end.j >= 0 && end.j < 8)
				placedownPiece(start, end);
			else
				placedownPiece(start, start);

			piece = nullptr;
		}
	}
}

void LevelChessboard::inputFlipButton()
{
	const bool flipBoardOld = flipBoard;
	buttonFlipBoard.checkInput();
	flipBoard = buttonFlipBoard.pressed;

	if(flipBoard && !flipBoardOld)
	{
		setChessPieceSpritePos();
		buttonShowDangerZoneWhite.move({8.75f*squareSize.x,3.25f*squareSize.y});
		buttonShowDangerZoneBlack.move({8.75f*squareSize.x,4.25f*squareSize.y});
	}

	if(!flipBoard && flipBoardOld)
	{
		setChessPieceSpritePos();
		buttonShowDangerZoneWhite.move({8.75f*squareSize.x,4.25f*squareSize.y});
		buttonShowDangerZoneBlack.move({8.75f*squareSize.x,3.25f*squareSize.y});
	}
}

void LevelChessboard::inputDangerZoneButtons()
{
	//The two Show Dangerzone Buttons shouldn't be both active at the same time.
	buttonShowDangerZoneWhite.checkInput();
	if(buttonShowDangerZoneWhite.pressed && buttonShowDangerZoneBlack.pressed)
		buttonShowDangerZoneBlack.pressed = false;

	buttonShowDangerZoneBlack.checkInput();
	if(buttonShowDangerZoneBlack.pressed && buttonShowDangerZoneWhite.pressed)
		buttonShowDangerZoneWhite.pressed = false;

	//Deactive the show Movement if you press the buttons.
	const bool whiteButton = (buttonShowDangerZoneWhite.pressed && !buttonShowDangerZoneWhite.pressedOld) || (!buttonShowDangerZoneWhite.pressed && buttonShowDangerZoneWhite.pressedOld);
	const bool blackButton = (buttonShowDangerZoneBlack.pressed && !buttonShowDangerZoneBlack.pressedOld) || (!buttonShowDangerZoneBlack.pressed && buttonShowDangerZoneBlack.pressedOld);

	if(whiteButton || blackButton)
	{
		for(int l=0; l<2; l++)
		{
			for(int k=0; k< mainEngine.nAlive[l]; k++)
			{
				ChessPiece *piece = mainEngine.piecesListAlive[l][k];
				piece->showMovement = false;
			}
		}
	}
}

void LevelChessboard::inputHomeButton()
{
	buttonHome.checkInput();
	if( IsKeyPressed(KEY_ESCAPE) || (buttonHome.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) )
	{
		buttonHome.pressed = false;
		buttonShowDangerZoneWhite.pressed = false;
		buttonShowDangerZoneBlack.pressed = false;
		buttonFlipBoard.pressed = false;
		resetBots();
		levelChessboard.unload();
		levelStartMenu.load();
	}
}

void LevelChessboard::inputResetButton()
{
	buttonReset.checkInput();
	if(buttonReset.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		reset();
}

void LevelChessboard::inputSingleArrowButton(Button &button, int sign)
{
	static double tStart[3]{};
	constexpr double tDuration = 1.0;
	button.checkInput();

	const KeyboardKey KEY = (sign==1) ? KEY_RIGHT : KEY_LEFT;

	if(IsKeyPressed(KEY))
		button.pressed = true;
	else if(IsKeyReleased(KEY))
		button.pressed = false;

	if(button.pressed)
	{
		if(!button.pressedOld)
		{
			tStart[sign+1] = Raylib::CLOCK.getTime();

			paused = true;
			speechBubbleActive = false;
			mainEngine.loadBoardState(mainEngine.turnCounter+sign);
			mainEngine.updateAttackZone();
			setChessPieceSpritePos();
			resetBots();
		}
		else if(Raylib::CLOCK.getTime() - tStart[sign+1] > tDuration)
		{
			paused = true;
			speechBubbleActive = false;
			mainEngine.loadBoardState(mainEngine.turnCounter+sign);
			mainEngine.updateAttackZone();
			setChessPieceSpritePos();
			resetBots();
		}
	}
	else
		tStart[sign+1] = DBL_MAX;
}

void LevelChessboard::inputArrowButtons()
{
	if(mainEngine.turnCounter > mainEngine.turnCounterStart)
		inputSingleArrowButton(buttonArrowBack, -1);
	else
		buttonArrowBack.pressed = false;

	if(mainEngine.turnCounter < mainEngine.maxTurns)
		inputSingleArrowButton(buttonArrowForward, +1);
	else
		buttonArrowForward.pressed = false;
}

void LevelChessboard::inputPPT()
{
	if(pptActive)
	{
		queenButton.checkInput();
		bishopButton.checkInput();
		knightButton.checkInput();
		rookButton.checkInput();

		if(queenButton.pressed)
		{
			pptTransformPawn(QUEEN);
			queenButton.pressed = false;
		}
		else if(bishopButton.pressed)
		{
			pptTransformPawn(BISHOP);
			bishopButton.pressed = false;
		}
		else if(knightButton.pressed)
		{
			pptTransformPawn(KNIGHT);
			knightButton.pressed = false;
		}
		else if(rookButton.pressed)
		{
			pptTransformPawn(ROOK);
			rookButton.pressed = false;
		}
	}
}

void LevelChessboard::inputDrawRequest()
{
	if(!paused && mainEngine.requestDraw)
	{
		buttonYes.checkInput();
		if(buttonYes.pressed)
		{
			buttonYes.pressed = false;
			mainEngine.requestDraw = false;
			drawGameOverIsActive = true;
			mainEngine.isdraw = true;
			mainEngine.turnColour = PNONE;
		}

		buttonNo.checkInput();
		if(buttonNo.pressed)
		{
			buttonNo.pressed = false;
			mainEngine.requestDraw = false;
		}
	}
}

void LevelChessboard::inputBots()
{
	//Bot does not act if one of the following conditions is true.
	if(mainEngine.turnColour == PNONE || playerControll[mainEngine.turnColour] || abs(Raylib::CLOCK.getTime() - tReset) < 1.5 || paused || mainEngine.requestDraw)
		return;

	Bot &bot{botList[mainEngine.turnColour]};
	static double tStartBotSearch = 0.0;

	if(!bot.searching && !bot.moveChosen)
	{
		tStartBotSearch = Raylib::CLOCK.getTime();
		bot.generateMove();
	}
	else if(bot.searching)
	{
		botThink(bot, tStartBotSearch);
	}
	else if(bot.moveChosen && !bot.spriteMoved)
	{
		cleanUpThreads();
		botSetSpriteDest(bot, tStartBotSearch);
	}
	else if(bot.spriteMoved && !bot.spriteArrived)
	{
		botTestSpriteDest(bot);
	}
	else if(bot.moveChosen && bot.spriteArrived)
	{
		const int weightOld = mainEngine.getBoardWeight();
		bot.executeMove();
		mainEngine.updateAttackZone();

		commentValidMoves(weightOld);
		commentCheck();

		if(mainEngine.checkmate[0] || mainEngine.checkmate[1] || mainEngine.isdraw)
			drawGameOverIsActive = true;

		setChessPieceSpritePos();
		bot.reset();
	}
}

//Drawing Functions

void LevelChessboard::drawBoardSquares() const
{
	for(int i = 0; i < 8; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			if((i + j) % 2 == 0)
				drawRectangle({boardOrigin.x + (float)(i)*squareSize.x, boardOrigin.y + (float)(j)*squareSize.y, squareSize.x, squareSize.y}, BROWN);
			else
				drawRectangle({boardOrigin.x + (float)(i)*squareSize.x, boardOrigin.y + (float)(j)*squareSize.y, squareSize.x, squareSize.y}, DARKBROWN);
		}
	}
}

void LevelChessboard::drawBoardLines() const
{
	Vector2 startpoint{}, endpoint{};
	for(int i = 0; i < 9; i++)
	{
		startpoint.x =	boardOrigin.x + (float)(i)*squareSize.x;
		startpoint.y =	boardOrigin.y;
		endpoint.x	 =	startpoint.x;
		endpoint.y	 =	boardOrigin.y + 8.0f*squareSize.y;
		drawLine(startpoint, endpoint);

		startpoint.x =	boardOrigin.x;
		startpoint.y =	boardOrigin.y + (float)(i)*squareSize.y;
		endpoint.x	 =	boardOrigin.x + 8.0f*squareSize.x;
		endpoint.y	 =	startpoint.y;
		drawLine(startpoint, endpoint);
	}
}

void LevelChessboard::drawTurnCounter() const
{
	std::string text{};

	if(!paused)
	{
		text = "Turn ";
		text += std::to_string((mainEngine.turnCounter/2)+1);
	}
	else
	{
		text = "Move ";
		text += std::to_string(mainEngine.turnCounter+1);
	}

	Vector2 textPos = {(8.0f + 1.0f/16.0f)*squareSize.x, 0.25f*squareSize.y};
	drawText(text, textPos, 50.0f);

	if(paused)
	{
		drawText("Game paused.\nPress Space to continue.", {textPos.x, 0.875f*squareSize.y}, 18.0f);
	}

	//Draw the Board-Score.
	const int score = mainEngine.getBoardWeight();
	text = "Score: ";
	text += std::to_string(score);
	static const float textHeight = Raylib::getTextSize(text, 35.0f, 1.0f).y;
	textPos = {(8.0f + 1.0f/16.0f)*squareSize.x, 1.5f*squareSize.y - 0.5f*textHeight};
	drawText(text, textPos, 35.0f);
}

void LevelChessboard::drawChessPiece(const Chess::ChessPiece *piece) const
{
	//Pick the correct Sprite from the spritesheet.
	Resources::textureSprites.setSpritePosition({(float)(piece->type)*(Resources::textureSize.x / 6.0f),(float)(piece->colour)*(Resources::textureSize.y/2.0f)});

	//Draw the Sprite.
	const Vector2 pos = {piece->drawX,piece->drawY};
	Resources::textureSprites.setPosition(pos);
	Resources::textureSprites.draw();
}

void LevelChessboard::drawChessPieces() const
{
	const ChessPiece *pieceSelected = nullptr;

	for(int l=0; l<2; l++)
	{
		for(int k=0; k< mainEngine.nAlive[l]; k++)
		{
			const ChessPiece *piece = mainEngine.piecesListAlive[l][k];

			if(piece->selected)
				pieceSelected = piece;
			else if(piece != pptPawn)
				drawChessPiece(piece);
		}
	}

	if(pieceSelected != nullptr)
	{
		drawChessPiece(pieceSelected);
	}
}

void LevelChessboard::drawMovement(const Chess::ChessPiece *piece) const
{
	const Color color1 = ((piece->colour == PWHITE) ? highlightColor1 : highlightColor2);
	const Color color2 = highlightColor3;

	for(int i=0; i<8; i++)
	{
		for(int j=0; j<8; j++)
		{
			BoardPos pos = {i,j};
			if(flipBoard)
				pos = {7-i,7-j};

			const Rectangle squareRectangle = {.x = (float)(pos.i) * squareSize.x, .y = (float)(pos.j) * squareSize.y, .width = squareSize.x, .height = squareSize.y};

			if(bitboardCheck(piece->movement, {i,j}))
				drawRectangle(squareRectangle, color1);
			else if(bitboardCheck((piece->movement^piece->movementPseudo), {i,j}))//If a square is pseudolegal but not legal, draw it with color2.
				drawRectangle(squareRectangle, color2);
		}
	}
}

void LevelChessboard::drawChessPieceMovements() const
{
	for(int l=0; l<2; l++)
	{
		for(int k=0; k< mainEngine.nAlive[l]; k++)
		{
			ChessPiece *piece = mainEngine.piecesListAlive[l][k];

			//Check for conditions if the movement of CPU pieces should be displayed.
			if(piece->showMovement && playerControll[piece->colour] && piece->colour == mainEngine.turnColour)
				piece->showMovement = false;

			if(piece->selected || piece->showMovement)
				drawMovement(piece);
		}
	}
}

void LevelChessboard::drawArrowButtons() const
{
	if(mainEngine.turnCounter == mainEngine.turnCounterStart)
		buttonArrowBack.drawTextured(Resources::arrowTexture, GRAY, GRAY, WHITE, -90.0f);
	else
		buttonArrowBack.drawTextured(Resources::arrowTexture, GRAY, LIGHTGRAY, WHITE, -90.0f);

	if(mainEngine.turnCounter >= mainEngine.maxTurns)
		buttonArrowForward.drawTextured(Resources::arrowTexture, GRAY, GRAY, WHITE, 90.0f);
	else
		buttonArrowForward.drawTextured(Resources::arrowTexture, GRAY, LIGHTGRAY, WHITE, 90.0f);
}

void LevelChessboard::drawButtons() const
{
	buttonFlipBoard.draw();
	buttonShowDangerZoneWhite.draw(DARKGREEN);
	buttonShowDangerZoneBlack.draw(MAROON);
	buttonHome.draw();
	buttonReset.draw();
	drawArrowButtons();
}

void LevelChessboard::drawDangerZone() const
{
	if(buttonShowDangerZoneWhite.pressed || buttonShowDangerZoneBlack.pressed)
	{
		for(int i=0; i<8; i++)
		{
			for(int j=0; j<8; j++)
			{
				BoardPos pos = {i,j};
				if(flipBoard)
					pos = {7-i,7-j};
				const Rectangle squareRectangle = {.x = (float)(pos.i) * squareSize.x, .y = (float)(pos.j) * squareSize.y, .width = squareSize.x, .height = squareSize.y};

				if(buttonShowDangerZoneWhite.pressed && bitboardCheck(mainEngine.attackZone[PWHITE], {i,j}))
					drawRectangle(squareRectangle, highlightColor1);
				else if(buttonShowDangerZoneBlack.pressed && bitboardCheck(mainEngine.attackZone[PBLACK], {i,j}))
					drawRectangle(squareRectangle, highlightColor2);
			}
		}
	}
}

void LevelChessboard::drawCursor() const
{
	if(cursorActive)
	{
		const BoardPos posMouse = getMouseSquare();
		const Vector2 pos = {(float)(posMouse.i)*squareSize.x, (float)(posMouse.j)*squareSize.y};
		Resources::cursorTexture.setPosition(pos);
		Resources::cursorTexture.draw();
	}
}

void LevelChessboard::drawPPT() const
{
	if(pptActive)
	{
		drawRectangle({0.0f,0.0f,1.0f,1.0f}, greyOutColor);

		const Vector2 drawPos = flipBoard ? flipVector2(pptxDraw) : pptxDraw;

		queenButton.draw();
		Resources::textureSprites.setSpritePosition({(float)(QUEEN)*(Resources::textureSize.x / 6.0f),(float)(pptPawn->colour)*(Resources::textureSize.y/2.0f)});
		Resources::textureSprites.setPosition(drawPos);
		Resources::textureSprites.draw();

		bishopButton.draw();
		Resources::textureSprites.setSpritePosition({(float)(BISHOP)*(Resources::textureSize.x / 6.0f),(float)(pptPawn->colour)*(Resources::textureSize.y/2.0f)});
		Resources::textureSprites.setPosition({drawPos.x,drawPos.y + pptSign*squareSize.y});
		Resources::textureSprites.draw();

		knightButton.draw();
		Resources::textureSprites.setSpritePosition({(float)(KNIGHT)*(Resources::textureSize.x / 6.0f),(float)(pptPawn->colour)*(Resources::textureSize.y/2.0f)});
		Resources::textureSprites.setPosition({drawPos.x,drawPos.y + 2.0f*pptSign*squareSize.y});
		Resources::textureSprites.draw();

		rookButton.draw();
		Resources::textureSprites.setSpritePosition({(float)(ROOK)*(Resources::textureSize.x / 6.0f),(float)(pptPawn->colour)*(Resources::textureSize.y/2.0f)});
		Resources::textureSprites.setPosition({drawPos.x,drawPos.y + 3.0f*pptSign*squareSize.y});
		Resources::textureSprites.draw();
	}
}

void LevelChessboard::drawGameOver() const
{
	if(drawGameOverIsActive)
	{
		drawRectangle({0.0f,0.0f,1.0f,1.0f}, greyOutColor);
		drawRectangle({boardOrigin.x, boardOrigin.y + 3.0f*squareSize.y, 8.0f*squareSize.x, 2.0f*squareSize.y}, DARKERBLUE);

		constexpr int fontsize = 60;
		const int fontsize2 = fontsize - 16;
		if(mainEngine.checkmate[0] || mainEngine.checkmate[1])
		{
			const float textsize  = getTextSize("CHECKMATE!", fontsize, 0.1f).x;
			drawText("CHECKMATE!", {4.0f*squareSize.x - 0.5f*textsize, 3.25f*squareSize.y}, fontsize);

			const float textsize2  = getTextSize(mainEngine.checkmateText, fontsize2, 0.1f).x;
			drawText(mainEngine.checkmateText, {4.0f*squareSize.x - 0.5f*textsize2, 4.25f*squareSize.y}, fontsize2);
		}
		else if(mainEngine.isdraw)
		{
			const float textsize  = getTextSize("DRAW!", fontsize, 0.1f).x;
			drawText("DRAW!", {4.0f*squareSize.x - 0.5f*textsize, 3.25f*squareSize.y}, fontsize);

			const float textsize2  = getTextSize(mainEngine.drawText, fontsize2, 0.1f).x;
			drawText(mainEngine.drawText, {4.0f*squareSize.x - 0.5f*textsize2, 4.25f*squareSize.y}, fontsize2);
		}
	}
}

void LevelChessboard::drawDrawRequest() const
{
	if(!paused && mainEngine.requestDraw)
	{
		drawRectangle({0.0f,0.0f,1.0f,1.0f}, greyOutColor);
		drawRectangle({boardOrigin.x,boardOrigin.y + 3.0f*squareSize.y, 8.0f*squareSize.x, 2.0f*squareSize.y}, DARKBLUE);
		buttonYes.draw();
		buttonNo.draw();

		constexpr int fontsize = 42;

		const float textsize  = getTextSize(mainEngine.drawText, fontsize, 0.1f).x;
		float xText = std::max(4.0f*squareSize.x - 0.5f*textsize, 0.0f);
		drawText(mainEngine.drawText, {xText, 3.25f*squareSize.y}, fontsize, RAYWHITE);

		const float textsize2  = getTextSize("Would you like to call a draw?", fontsize, 0.1f).x;
		xText = std::max(4.0f*squareSize.x - 0.5f*textsize2, 0.0f);
		drawText("Would you like to call a draw?", {xText, 3.75f*squareSize.y}, fontsize, RAYWHITE);
	}
}

void LevelChessboard::drawSpeechBubble()
{
	if(speechBubbleActive && speechBubbleCP != nullptr && abs(Raylib::CLOCK.getTime() - tStart) < duration)
	{
		moveBubbleSprite();
		speechbubble.draw();
	}
	else
		speechBubbleActive = false;
}