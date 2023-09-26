#include "chess_game.hpp"
#include "random_player.hpp"
#include <iostream>
#include <thread>

const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

chess_game::chess_game(chess_player* white_player, chess_player* black_player)
	: white_player(white_player), black_player(black_player)
{
}

const std::string OUTCOME_STR[6] = {
	"CHECKMATE",
	"STALEMATE",
	"INSUFFICIENT_MATERIAL",
	"FIFTY_MOVE_RULE",
	"THREEFOLD_REPETITION",
	"NONE"
};
void chess_game::play()
{
	chess::Board board = chess::Board(STARTING_FEN);
	bool white_to_move = true;

	std::string moves_str = "";

	while (board.isGameOver().first == chess::GameResultReason::NONE)
	{
		std::cout << board << std::endl;

		auto start = std::chrono::high_resolution_clock::now();
		chess::Move move = 
			white_to_move ? white_player->get_move(board) : black_player->get_move(board);


		auto end = std::chrono::high_resolution_clock::now();
		long ms_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		std::cout << chess::uci::moveToUci(move) << " took " << ms_elapsed << "ms" << std::endl;

		moves_str += chess::uci::moveToUci(move) + "\n";

		board.makeMove(move);

		white_to_move = !white_to_move;
	}

	std::cout << "game over" << std::endl;
	std::cout << "color: " << (white_to_move ? "black" : "white") << std::endl;
	std::cout << "result: " << OUTCOME_STR[(int)board.isGameOver().first] << std::endl;

	std::cout << "moves: \n" << moves_str << std::endl;
}
