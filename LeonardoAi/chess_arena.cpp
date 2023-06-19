#include "chess_arena.hpp"

chess_arena::chess_arena(
	std::string given_name,
	std::unique_ptr<Player>&& player1,
	std::unique_ptr<Player>&& player2
) :
	name(given_name),
	player1(std::move(player1)),
	player2(std::move(player2)),
	board(STARTING_FEN)
{}

void chess_arena::play_game(arena_result& result)
{
	size_t loop_count = 0;

	Player& white_player = player1_plays_white ? *player1.get() : *player2.get();
	Player& black_player = player1_plays_white ? *player2.get() : *player1.get();

	while (true)
	{
		ChessColor currentTurnColor = board.getCurrentTurnColor();
		Player& currPlayer = currentTurnColor == White ? white_player : black_player;

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

		if (currGameState != GameState::Ongoing)
		{
			if (currGameState == GameState::WhiteWon)
			{
				player1_plays_white ?
					result.player_1_won++ :
					result.player_2_won++;
			}
			else if (currGameState == GameState::BlackWon)
			{
				player1_plays_white ?
					result.player_2_won++ :
					result.player_1_won++;
			}
			else
			{
				result.draws++;
			}

			break;
		}

		loop_count++;

		if (loop_count > 500)
		{
			result.draws++;
			break;
		}
	}

	player1_plays_white = !player1_plays_white;
}

arena_result chess_arena::play(size_t number_of_games)
{
	arena_result result = arena_result();

	for (int i = 0; i < number_of_games; i++)
	{
		play_game(result);
		board = ChessBoard(STARTING_FEN);
		//std::cout << "Game " << i + 1 << " finished" << std::endl;
	}

	return result;
}
