#include "chess_arena.hpp"

chess_arena::chess_arena(
	std::unique_ptr<Player>&& player1,
	std::unique_ptr<Player>&& player2
) :
	_whitePlayer(std::move(player1))
	, _blackPlayer(std::move(player2))
	, board(STARTING_FEN)
{}

int chess_arena::play_game()
{
	size_t loop_count = 0;

	while (true)
	{
		ChessColor currentTurnColor = board.getCurrentTurnColor();
		Player& currPlayer = currentTurnColor == White ? *_whitePlayer.get() : *_blackPlayer.get();

		//get the move from the current player
		UniqueMoveList legalMoves = board.getAllLegalMoves();
		//to make sure the move the player wants to play is legal, 
		//the return value is an index of the list of all legal moves
		int choosenMoveIdx = currPlayer.getMove(board, legalMoves);
		//throws an error if the returned value is not a valid index
		if (choosenMoveIdx < 0 || choosenMoveIdx >= legalMoves.size())
		{
			throw std::exception("move cannot be executed. returned index is invalid!");
		}

		//get the move from the list
		Move& moveToMake = *legalMoves.at(choosenMoveIdx).get();

		//execute the move
		board.makeMove(moveToMake);

		//get the gamestate and evaluate if the game has ended
		GameState currGameState = board.getGameState();
		if (currGameState == GameState::WhiteWon )
		{
			return 1;
		}
		else if (currGameState == GameState::BlackWon)
		{
			return -1;
		}
		else if (currGameState != GameState::Ongoing)
		{
			return 0;
		}
		loop_count++;

		if (loop_count > 1000)
		{
			//just in case
			std::cout << "loop count exceeded 1000. returning" << std::endl;
			return 0;
		}
	}
	throw std::exception("game should return inside the gameloop");
}

int chess_arena::play(size_t number_of_games)
{
	return play(number_of_games, false);
}

int chess_arena::play(size_t number_of_games, bool print)
{
	int white_wins = 0;

	for (int i = 0; i < number_of_games; i++)
	{
		white_wins += play_game();
		//reset the board - not tested
		board = ChessBoard(STARTING_FEN);

		if (print)
		{
			std::cout << "game " << i << " finished. score: " << white_wins << std::endl;
		}
	}

	return white_wins;
}
