#include "Game.h"

ChessGame::ChessGame(
	std::unique_ptr<Player>&& player1,
	std::unique_ptr<Player>&& player2,
	std::string fen)
	: 
	_whitePlayer(std::move(player1)),
	_blackPlayer(std::move(player2)),
	_board(fen)
{}

void ChessGame::start()
{
	//this variable stores all the moves of the players as the game progresses
	std::string movesMade = "";
	
	//this loop is the game loop
	//it will only end if the game is done
	while (true)
	{
		ChessColor currentTurnColor = _board.getCurrentTurnColor();
		Player& currPlayer = currentTurnColor == White ? *_whitePlayer.get() : *_blackPlayer.get();
		

		//this number increases every time black makes a move
		int currentTurnNumber = _board.getNumberOfMovesPlayed();
		
		//print board and the move number
		std::cout 
			<< "\n\n" 
			<< COLOR_STRING[currentTurnColor] << " " << currPlayer.getName() 
			<< "'s turn " << currentTurnNumber;
		std::cout << "\n" << (_board.getGameDurationState() == MidGame ? "Mid Game" : "End Game");
		std::cout << "\n" << _board.getFen();
		std::cout << _board.getString();
		
		//get the move from the current player
		UniqueMoveList legalMoves = _board.getAllLegalMoves();
		//to make sure the move the player wants to play is legal, 
		//the return value is an index of the list of all legal moves
		int choosenMoveIdx = currPlayer.getMove(_board, legalMoves);
		//throws an error if the returned value is not a valid index
		if (choosenMoveIdx < 0 || choosenMoveIdx >= legalMoves.size())
		{
			throw "move cannot be executed. returned index is invalid!";
		}

		//get the move from the list
		Move& moveToMake = *legalMoves.at(choosenMoveIdx).get();
		
		//store the current move and print it
		std::string currMoveStr = moveToMake.getString();
		std::cout << "\n" << "move: " << currMoveStr << "\n";
		movesMade += currMoveStr + "\n";

		//execute the move
		_board.makeMove(moveToMake);

		//get the gamestate and evaluate if the game has ended
		GameState currGameState = _board.getGameState();
		if(currGameState == WhiteWon || currGameState == BlackWon)
		{
			std::cout << _board.getString();
			std::cout << "Checkmate! " << COLOR_STRING[currentTurnColor] << " wins!" << std::endl;
			break;
		}
		else if(currGameState == Draw)
		{
			std::cout << "Draw!" << std::endl;
			break;
		}
		else if(currGameState == Stalemate)
		{
			std::cout << "Stalemate!" << std::endl;
			break;
		}
	}

	//print the history of moves
	std::cout << "\n\nPress any key to show move log\n";
	//wait for 2s
	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::cout << "Moves made:\n" << movesMade;
}