#include "chess_game.hpp"
#include "random_player.hpp"
#include <iostream>
#include <thread>

const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

chess_game::chess_game(chess_player* white_player, chess_player* black_player)
	: white_player(white_player), black_player(black_player)
{
}

void chess_game::play()
{
	chess::Board board = chess::Board(STARTING_FEN);
	bool white_to_move = true;

	std::string moves_str = "";

	while (board.isGameOver().first == chess::GameResultReason::NONE)
	{
		chess::Move move = 
			white_to_move ? white_player->get_move(board) : black_player->get_move(board);

		moves_str += chess::uci::moveToUci(move) + "\n";

		board.makeMove(move);

		std::cout << board << std::endl;

		white_to_move = !white_to_move;
	}

	std::cout << "game over" << std::endl;

	std::cout << "moves: \n" << moves_str << std::endl;
}
