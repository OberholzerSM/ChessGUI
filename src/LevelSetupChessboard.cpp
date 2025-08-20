#include "LevelStartMenu.h"
#include "LevelSetupChessboard.h"
#include "ChessGUI.h"
using namespace Chess;
using namespace Raylib;

//Public Level Functions

void LevelSetupChessboard::load()
{
	LevelID = this->ID();
	ColorBackground = DARKGRAY;

	if(!loaded)
	{
		//Reset the board.
		mainEngine.loadBoardState(0);

		//Resize the Sprites.
		Resources::textureSprites.setSize(squareSize);
		Resources::arrowTexture.setSize({0.5f*squareSize.x, 0.5f*squareSize.y});
		Resources::cursorTexture.setSize(squareSize);

		//Set the Sprite Positions.
		setChessPieceSpritePos();

		//Set the Buttons
		if( mainEngine.turnColour == PBLACK )
		{
			buttonTCBlack.pressed = true;
			buttonTCWhite.pressed = false;
		}
		else
		{
			buttonTCWhite.pressed = true;
			buttonTCBlack.pressed = false;
		}

		loaded = true;
	}
}

void LevelSetupChessboard::input()
{
	if(loaded)
	{
		const bool active = !pptActive && !castlingMode && !enpassantMode;

		if(active)
		{
			inputMouse();
			inputHomeButton();
			inputResetButton();
			inputTurnColourButtons();
			inputCastlingStartButton();
			inputEnPassantStartButton();
		}

		if(pptActive)
			inputPPT();
		else if(castlingMode)
			inputCastlingMode();
		else if(enpassantMode)
			inputEnPassantMode();

		if(IsKeyPressed(KEY_ESCAPE))
		{
			castlingMode = false;
			enpassantMode = false;
			buttonCastling.pressed = false;
			buttonEnPassant.pressed = false;

			for(int l=0; l<2; l++)
			{
				for(int k=8; k<16; k++)
				{
					ChessPiece *pawn = mainEngine.piecesList[l][k];
					pawn->enpassant = false;
				}
			}
		}

		moveChessPieces();
	}
}

void LevelSetupChessboard::draw()
{
	if(loaded)
	{
		const Rectangle greyRect = {.x=8.0f*squareSize.x, .y=0.0f, .width = 2.0f*squareSize.x, .height = 8.0f*squareSize.y};

		drawBoardSquares();
		drawBoardLines();
		drawScore();
		drawButtons();
		if(!pptActive && !castlingMode)
			drawCursor();
		drawChessPieces();

		if(pptActive)
		{
			drawPPT();
			drawCursor();
		}
		else if(castlingMode)
		{
			drawRectangle(greyRect,greyOutColor);
			drawCastlingMode();
		}
		else if(enpassantMode)
		{
			drawRectangle(greyRect, greyOutColor);
			drawEnPassantMode();
		}
	}
}

void LevelSetupChessboard::unload()
{
	if(loaded)
	{
		loaded = false;
	}
}

//Actions

Chess::BoardPos LevelSetupChessboard::getMouseSquare(bool flip) const
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

void LevelSetupChessboard::setChessPieceSpritePos() const
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

void LevelSetupChessboard::setChessPieceSpriteDestination() const
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

void LevelSetupChessboard::pickupPiece(const BoardPos &start) const
{
	ChessPiece *piece = mainEngine.board[start.i][start.j];

	if(piece != nullptr && piece->alive && !pptActive)
	{
		piece->selected = true;
	}
}

void LevelSetupChessboard::placedownPiece(const Chess::BoardPos &start, const Chess::BoardPos &end)
{
	ChessPiece *piece = mainEngine.board[start.i][start.j];
	if(piece != nullptr && piece->selected)
	{
		piece->selected = false;

		if(start.i == end.i && start.j == end.j)
		{
			setChessPieceSpritePos();
			return;
		}

		const int jEnd = (piece->colour == PWHITE) ? 0 : 7;
		bool pawnTransform = (piece->type == PAWN && jEnd == end.j);

		if(!pawnTransform)
		{
			if(mainEngine.board[end.i][end.j] != nullptr)
			{
				mainEngine.board[end.i][end.j]->kill();
				mainEngine.turnsUntilDrawCounter = 0;
			}
			mainEngine.board[start.i][start.j] = nullptr;
			mainEngine.board[end.i][end.j] = piece;
			piece->pos = end;
		}
		else
			pptAssignPawn(start, end);

		setChessPieceSpritePos();
	}
}

Vector2 LevelSetupChessboard::flipVector2(Vector2 pos) const
{
	pos.x = 2.0f*boardOrigin.x + 7.0f*squareSize.x - pos.x;
	pos.y = 2.0f*boardOrigin.y + 7.0f*squareSize.y - pos.y;
	return pos;
}

void LevelSetupChessboard::reset()
{
	for(int i=0; i<8; i++)
	{
		for(int j=0; j<8; j++)
		{
			mainEngine.board[i][j] = nullptr;
		}
	}

	mainEngine.initialize();

	for(int l=0; l<2; l++)
	{
		for(int k=0; k<nPieces; k++)
		{
			ChessPiece *piece = mainEngine.piecesList[l][k];
			piece->selected = false;
		}
	}

	setChessPieceSpriteDestination();
	mainEngine.turnColour = PWHITE;
	buttonTCWhite.pressed = true;
	buttonTCBlack.pressed = false;

	pptActive = false;

	tReset = Raylib::CLOCK.getTime();
}

void LevelSetupChessboard::moveChessPiece(Chess::ChessPiece *piece) const
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
	else
	{
		if(!xArrived)
			piece->drawX += speedX;

		if(!yArrived)
			piece->drawY += speedY;
	}
}

void LevelSetupChessboard::moveChessPieces()
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

//Pawn-Transform-Panel

void LevelSetupChessboard::pptAssignPawn(const Chess::BoardPos &start, const Chess::BoardPos &end)
{
	pptPawn = mainEngine.board[start.i][start.j];

	if(pptPawn!=nullptr)
	{
		pptActive = true;
		cursorActive = true;

		pptStart = start;
		pptEnd = end;

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

void LevelSetupChessboard::pptTransformPawn(Chess::PType type)
{

	if(mainEngine.board[pptEnd.i][pptEnd.j] != nullptr)
	{
		mainEngine.board[pptEnd.i][pptEnd.j]->kill();
		mainEngine.turnsUntilDrawCounter = 0;
	}
	mainEngine.board[pptStart.i][pptStart.j] = nullptr;
	mainEngine.board[pptEnd.i][pptEnd.j] = pptPawn;
	pptPawn->pos = pptEnd;
	pptPawn->transformPawn(type);

	setChessPieceSpritePos();

	pptPawn = nullptr;
	pptActive = false;
	cursorActive = false;
}


//Input Functions

void LevelSetupChessboard::inputMouse()
{
	static BoardPos start = {0,0};
	static ChessPiece *piece = nullptr;

	if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		start = getMouseSquare(flipBoard);
		if(start.i >= 0 && start.i < 8 && start.j >= 0 && start.j < 8)
		{
			piece = mainEngine.board[start.i][start.j];
			if(piece != nullptr)
				pickupPiece(start);
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
				piece->kill();

			piece = nullptr;
		}
	}
}

void LevelSetupChessboard::inputPPT()
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

void LevelSetupChessboard::inputTurnColourButtons()
{
	buttonTCWhite.checkInput();

	if(!buttonTCWhite.pressed && buttonTCWhite.pressedOld)
		buttonTCWhite.pressed = true;

	if(buttonTCWhite.pressed)
	{
		mainEngine.turnColour = PWHITE;
		if(buttonTCBlack.pressed)
			buttonTCBlack.pressed = false;
	}

	buttonTCBlack.checkInput();

	if(!buttonTCBlack.pressed && buttonTCBlack.pressedOld)
		buttonTCBlack.pressed = true;

	if(buttonTCBlack.pressed)
	{
		mainEngine.turnColour = PBLACK;
		if(buttonTCWhite.pressed)
			buttonTCWhite.pressed = false;
	}
}

void LevelSetupChessboard::inputCastlingStartButton()
{
	buttonCastling.checkInput();
	if(buttonCastling.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
	{
		castlingMode = true;

		if( mainEngine.kingWhite.pos.i != mainEngine.kingWhite.startPos.i || mainEngine.kingWhite.pos.j != mainEngine.kingWhite.startPos.j )
		{
			buttonCastleWhiteLeft.pressed = false;
			buttonCastleWhiteRight.pressed = false;
		}
		else
		{
			buttonCastleWhiteLeft.pressed = true;
			buttonCastleWhiteRight.pressed = true;

			if(mainEngine.rookWhiteL.pos.i != mainEngine.rookWhiteL.startPos.i || mainEngine.rookWhiteL.pos.j != mainEngine.rookWhiteL.startPos.j)
				buttonCastleWhiteLeft.pressed = false;
			else
				buttonCastleWhiteLeft.pressed = true;

			if(mainEngine.rookWhiteR.pos.i != mainEngine.rookWhiteR.startPos.i || mainEngine.rookWhiteR.pos.j != mainEngine.rookWhiteR.startPos.j)
				buttonCastleWhiteRight.pressed = false;
			else
				buttonCastleWhiteRight.pressed = true;
		}

		if(mainEngine.kingBlack.pos.i != mainEngine.kingBlack.startPos.i || mainEngine.kingBlack.pos.j != mainEngine.kingBlack.startPos.j)
		{
			buttonCastleBlackLeft.pressed = false;
			buttonCastleBlackRight.pressed = false;
		}
		else
		{
			buttonCastleBlackLeft.pressed = true;
			buttonCastleBlackRight.pressed = true;

			if(mainEngine.rookBlackL.pos.i != mainEngine.rookBlackL.startPos.i || mainEngine.rookBlackL.pos.j != mainEngine.rookBlackL.startPos.j)
				buttonCastleBlackLeft.pressed = false;
			else
				buttonCastleBlackLeft.pressed = true;

			if(mainEngine.rookBlackR.pos.i != mainEngine.rookBlackR.startPos.i || mainEngine.rookBlackR.pos.j != mainEngine.rookBlackR.startPos.j)
				buttonCastleBlackRight.pressed = false;
			else
				buttonCastleBlackRight.pressed = true;
		}
	}
}

void LevelSetupChessboard::inputCastlingMode()
{
	const bool whiteKingMoved = ( mainEngine.kingWhite.pos.i != mainEngine.kingWhite.startPos.i || mainEngine.kingWhite.pos.j != mainEngine.kingWhite.startPos.j);

	const bool whiteRookLMoved = (mainEngine.rookWhiteL.pos.i != mainEngine.rookWhiteL.startPos.i || mainEngine.rookWhiteL.pos.j != mainEngine.rookWhiteL.startPos.j);
	if( !whiteKingMoved && !whiteRookLMoved )
	{
		buttonCastleWhiteLeft.checkInput();
		mainEngine.rookWhiteL.moved = !buttonCastleWhiteLeft.pressed;
	}

	const bool whiteRookRMoved = (mainEngine.rookWhiteR.pos.i != mainEngine.rookWhiteR.startPos.i || mainEngine.rookWhiteR.pos.j != mainEngine.rookWhiteR.startPos.j);
	if(!whiteKingMoved && !whiteRookRMoved)
	{
		buttonCastleWhiteRight.checkInput();
		mainEngine.rookWhiteR.moved = !buttonCastleWhiteRight.pressed;
	}


	const bool blackKingMoved = (mainEngine.kingBlack.pos.i != mainEngine.kingBlack.startPos.i || mainEngine.kingBlack.pos.j != mainEngine.kingBlack.startPos.j);

	const bool blackRookLMoved = (mainEngine.rookBlackL.pos.i != mainEngine.rookBlackL.startPos.i || mainEngine.rookBlackL.pos.j != mainEngine.rookBlackL.startPos.j);
	if(!blackKingMoved && !blackRookLMoved)
	{
		buttonCastleBlackLeft.checkInput();
		mainEngine.rookBlackL.moved = !buttonCastleBlackLeft.pressed;
	}

	const bool blackRookRMoved = (mainEngine.rookBlackR.pos.i != mainEngine.rookBlackR.startPos.i || mainEngine.rookBlackR.pos.j != mainEngine.rookBlackR.startPos.j);
	if(!blackKingMoved && !blackRookRMoved)
	{
		buttonCastleBlackRight.checkInput();
		mainEngine.rookBlackR.moved = !buttonCastleBlackRight.pressed;
	}

	buttonCastleDone.checkInput();
	if(buttonCastleDone.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		castlingMode = false;
}

void LevelSetupChessboard::inputEnPassantStartButton()
{
	buttonEnPassant.checkInput();
	if(buttonEnPassant.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
	{
		//Find all the Pieces elligable for en-passant.
		for(int i=0; i<7; i++)
		{
			const int j = (mainEngine.turnColour == PWHITE ? 3 : 4);
			ChessPiece *pieceLeft = mainEngine.board[i][j];
			ChessPiece *pieceRight = mainEngine.board[i+1][j];
			
			if(pieceLeft == nullptr || pieceLeft->type != PAWN || pieceRight == nullptr || pieceRight->type != PAWN)
				continue;

			if(pieceLeft->colour != pieceRight->colour)
			{
				if(pieceLeft->colour == mainEngine.turnColour)
				{
					pieceLeft->enpassant = false;
					pieceRight->enpassant = true;
				}
				else
				{
					pieceLeft->enpassant = true;
					pieceRight->enpassant = false;
				}
				enpassantMode = true;
			}
		}
	}
}

void LevelSetupChessboard::inputEnPassantMode()
{
	if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		const BoardPos mousePos = getMouseSquare(flipBoard);
		if(mousePos.i >= 0 && mousePos.i < 8 && mousePos.j >= 0 && mousePos.j < 8)
		{
			ChessPiece *pawn = mainEngine.board[mousePos.i][mousePos.j];
			if( pawn!=nullptr && pawn->enpassant )
			{
				pawn->selected = true;

				//Deactivate all en-passant flags for the other pawns.
				for(int l=0; l<2; l++)
				{
					for(int k=8; k<16; k++)
					{
						ChessPiece *pawn2 = mainEngine.piecesList[l][k];
						pawn2->enpassant = pawn2->selected;
					}
				}

				pawn->selected = false;
				enpassantMode = false;
			}
		}
	}
}

void LevelSetupChessboard::inputHomeButton()
{
	buttonHome.checkInput();
	if(IsKeyPressed(KEY_ESCAPE) || (buttonHome.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
	{
		buttonHome.pressed = false;

		//Save the Boardstate
		mainEngine.turnCounter = 0;
		mainEngine.turnCounterStart = 0;
		mainEngine.updateMovement();
		mainEngine.checkLateGame();
		mainEngine.checkGameOver();
		mainEngine.updateAttackZone();
		mainEngine.saveBoardState(0);

		levelSetupChessboard.unload();
		levelStartMenu.load();
	}
}

void LevelSetupChessboard::inputResetButton()
{
	buttonReset.checkInput();
	if(buttonReset.pressed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		reset();
}


//Drawing Functions

void LevelSetupChessboard::drawBoardSquares() const
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

void LevelSetupChessboard::drawBoardLines() const
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

void LevelSetupChessboard::drawScore() const
{
	//Draw the Board-Score.
	const int score = mainEngine.getBoardWeight();
	std::string text = "Score: ";
	text += std::to_string(score);
	static const float textHeight = Raylib::getTextSize(text, 35.0f, 1.0f).y;
	const Vector2 textPos = {(8.0f + 1.0f/16.0f)*squareSize.x, 0.5f*squareSize.y - 0.5f*textHeight};
	drawText(text, textPos, 35.0f);
}

void LevelSetupChessboard::drawPPT() const
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

void LevelSetupChessboard::drawCursor() const
{
	if(cursorActive)
	{
		const BoardPos posMouse = getMouseSquare();
		const Vector2 pos = {(float)(posMouse.i)*squareSize.x, (float)(posMouse.j)*squareSize.y};
		Resources::cursorTexture.setPosition(pos);
		Resources::cursorTexture.draw();
	}
}

void LevelSetupChessboard::drawButtons() const
{
	Vector2 textSize = Raylib::getTextSize("Next to Move",25.0f,0.1f);
	Raylib::drawText("Next to Move", {8.75f*squareSize.x - 0.3f*textSize.x,2.2f*squareSize.y - textSize.y}, 25.0f);
	buttonTCWhite.draw(DARKGREEN,DARKGRAY);
	buttonTCBlack.draw(MAROON,DARKGRAY);

	textSize = Raylib::getTextSize("Castling", 25.0f, 0.1f);
	Raylib::drawText("Castling", {8.75f*squareSize.x - 0.25f*textSize.x,3.25f*squareSize.y - textSize.y}, 25.0f);
	buttonCastling.draw();

	textSize = Raylib::getTextSize("En-Passant", 25.0f, 0.1f);
	Raylib::drawText("En-Passant", {8.75f*squareSize.x - 0.32f*textSize.x,4.25f*squareSize.y - textSize.y}, 25.0f);
	buttonEnPassant.draw();

	buttonHome.draw();
	buttonReset.draw();
}

void LevelSetupChessboard::drawCastlingMode() const
{
	//Draw the Options-Box.
	const Rectangle greyRect = { .x = 0.0f, .y = 0.0f, .width = 8.0f*squareSize.x, .height = 8.0f*squareSize.y };
	drawRectangle(greyRect,greyOutColor);

	const Rectangle blueRect = { .x = 1.0f*squareSize.x, .y = 0.5f*squareSize.y, .width = 6.0f*squareSize.x, .height = 7.0f*squareSize.y };
	drawRectangle(blueRect, DARKBLUE);
	drawRectangleLines(blueRect, 4.0f, BLACK);

	Vector2 textSize = getTextSize("Castling",50.0f,0.1f);
	drawText( "Castling", { 4.0f*squareSize.x - 0.5f*textSize.x, 0.6f*squareSize.y  }, 50.0f );

	//Draw the Arrows.

	Resources::arrowTexture.setPosition({2.8f*squareSize.x, 5.1f*squareSize.y});
	Resources::arrowTexture.draw(-90.0f);

	Resources::arrowTexture.setPosition({2.8f*squareSize.x, 2.1f*squareSize.y});
	Resources::arrowTexture.draw(-90.0f);

	Resources::arrowTexture.setPosition({4.65f*squareSize.x, 5.1f*squareSize.y});
	Resources::arrowTexture.draw(+90.0f);

	Resources::arrowTexture.setPosition({4.65f*squareSize.x, 2.1f*squareSize.y});
	Resources::arrowTexture.draw(+90.0f);

	//Draw the White King Sprite.
	Resources::textureSprites.setSpritePosition({(float)(KING)*(Resources::textureSize.x / 6.0f),(float)(PWHITE)*(Resources::textureSize.y/2.0f)});
	Resources::textureSprites.setPosition({3.5f*squareSize.x, 4.85f*squareSize.y});
	Resources::textureSprites.draw();

	//Draw the white Rook Sprites.
	Resources::textureSprites.setSpritePosition({(float)(ROOK)*(Resources::textureSize.x / 6.0f),(float)(PWHITE)*(Resources::textureSize.y/2.0f)});
	Resources::textureSprites.setPosition({1.75f*squareSize.x, 4.85f*squareSize.y });
	Resources::textureSprites.draw();
	Resources::textureSprites.setPosition({5.25f*squareSize.x, 4.85f*squareSize.y});
	Resources::textureSprites.draw();

	//Draw the black King Sprite.
	Resources::textureSprites.setSpritePosition({(float)(KING)*(Resources::textureSize.x / 6.0f),(float)(PBLACK)*(Resources::textureSize.y/2.0f)});
	Resources::textureSprites.setPosition({3.5f*squareSize.x, 1.85f*squareSize.y});
	Resources::textureSprites.draw();

	//Draw the black Rook Sprites.
	Resources::textureSprites.setSpritePosition({(float)(ROOK)*(Resources::textureSize.x / 6.0f),(float)(PBLACK)*(Resources::textureSize.y/2.0f)});
	Resources::textureSprites.setPosition({1.75f*squareSize.x, 1.85f*squareSize.y});
	Resources::textureSprites.draw();
	Resources::textureSprites.setPosition({5.25f*squareSize.x, 1.85f*squareSize.y});
	Resources::textureSprites.draw();

	//Draw the Buttons

	drawText( "Queenside White", { 1.1f*squareSize.x, 4.5f*squareSize.y }, 35.0f );
	buttonCastleWhiteLeft.draw(DARKGREEN, GRAY);

	textSize = getTextSize("Kingside White",35.0f,0.1f);
	drawText("Kingside White", {6.9f*squareSize.x - textSize.x, 4.5f*squareSize.y}, 35.0f);
	buttonCastleWhiteRight.draw(DARKGREEN, GRAY);

	drawText("Queenside Black", {1.1f*squareSize.x, 1.5f*squareSize.y}, 35.0f);
	buttonCastleBlackLeft.draw(MAROON, GRAY);

	textSize = getTextSize("Kingside Black", 35.0f, 0.1f);
	drawText("Kingside Black", {6.9f*squareSize.x - textSize.x, 1.5f*squareSize.y}, 35.0f);
	buttonCastleBlackRight.draw(MAROON, GRAY);

	buttonCastleDone.draw();
}

void LevelSetupChessboard::drawEnPassantMode() const
{
	static const Rectangle greyRectTop = { .x = 0.0f, .y = 0.0f, .width = 8.0f*squareSize.x, .height = 3.0f*squareSize.y };
	static const Rectangle greyRectBottom = {.x = 0.0f, .y = 5.0f*squareSize.y, .width = 8.0f*squareSize.x, .height = 3.0f*squareSize.y};

	drawRectangle(greyRectTop,greyOutColor);
	drawRectangle(greyRectBottom, greyOutColor);

	for(int i=0; i<8; i++)
	{
		const int j = ( mainEngine.turnColour == PWHITE ? 3 : 4 );
		const ChessPiece *piece = mainEngine.board[i][j];
		const Rectangle square = {.x = (float)i * squareSize.x, .y = (float)j*squareSize.y, .width = squareSize.x, .height = squareSize.y};

		if(piece == nullptr || !piece->enpassant)
			drawRectangle(square, greyOutColor);

		const float j2 = (mainEngine.turnColour == PWHITE ? 4.0f : 3.0f);
		const Rectangle square2 = {.x = (float)i * squareSize.x, .y = j2*squareSize.y, .width = squareSize.x, .height = squareSize.y};
		drawRectangle(square2, greyOutColor);
	}
}

void LevelSetupChessboard::drawChessPiece(const Chess::ChessPiece *piece) const
{
	//Pick the correct Sprite from the spritesheet.
	Resources::textureSprites.setSpritePosition({(float)(piece->type)*(Resources::textureSize.x / 6.0f),(float)(piece->colour)*(Resources::textureSize.y/2.0f)});

	//Draw the Sprite.
	const Vector2 pos = {piece->drawX,piece->drawY};
	Resources::textureSprites.setPosition(pos);
	Resources::textureSprites.draw();
}

void LevelSetupChessboard::drawChessPieces() const
{
	const ChessPiece *pieceSelected = nullptr;

	for(int l=0; l<2; l++)
	{
		for(int k=0; k < nPieces; k++)
		{
			const ChessPiece *piece = mainEngine.piecesList[l][k];

			if(piece->alive)
			{
				if(piece->selected)
					pieceSelected = piece;
				else if(piece != pptPawn)
					drawChessPiece(piece);
			}
		}
	}

	if(pieceSelected != nullptr)
	{
		drawChessPiece(pieceSelected);
	}
}