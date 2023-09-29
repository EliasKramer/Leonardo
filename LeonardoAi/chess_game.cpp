#include "chess_game.hpp"
#include "random_player.hpp"
#include <iostream>
#include <thread>

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
	chess::Board board = chess::Board(DEFAULT_FEN);
	bool white_to_move = true;

	std::string moves_str = "";

	while (board.isGameOver().first == chess::GameResultReason::NONE)
	{
		std::cout << (board.sideToMove() == chess::Color::WHITE ? "white" : "black") << " to move\n";
		std::cout << board << std::endl;

		auto start = std::chrono::high_resolution_clock::now();
		chess::Move move = white_to_move ? 
			white_player->get_move(board) : 
			black_player->get_move(board);
		auto end = std::chrono::high_resolution_clock::now();

		chess::Movelist moves;
		chess::movegen::legalmoves(moves, board);

		if (move == chess::Move::NULL_MOVE || move == chess::Move::NO_MOVE)
		{
			std::cout << "no move\n";
			break;
		}
		bool move_found = false;
		for (chess::Move curr : moves)
		{
			if (curr == move)
			{
				move_found = true;
				break;
			}
		}
		if (!move_found)
		{
			std::cout << "illegal move " << chess::uci::moveToUci(move) << std::endl;
			break;
		}

		long long ms_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		std::cout << chess::uci::moveToUci(move) << " took " << ms_elapsed << "ms" << std::endl;

		moves_str += chess::uci::moveToUci(move) + "\n";

		board.makeMove(move);

		white_to_move = !white_to_move;
		std::cout << "------------------------\n";
	}

	std::cout << "game over" << std::endl;
	std::cout << "color: " << (white_to_move ? "black" : "white") << std::endl;
	std::cout << "result: " << OUTCOME_STR[(int)board.isGameOver().first] << std::endl;

	std::cout << "moves: \n" << moves_str << std::endl;
}
