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
int chess_game::play()
{
	chess::Board board = chess::Board(DEFAULT_FEN);
	bool white_to_move = true;

	std::string moves_str = "";

	long ms_sum[2] = { 0, 0 };
	int moves_calculated[2] = { 0, 0 };
	int moves_played[2] = { 0, 0 };

	while (board.isGameOver().second == chess::GameResult::NONE)
	{
		white_to_move = board.sideToMove() == chess::Color::WHITE;
#ifdef DEBUG_PRINT
		std::cout << (white_to_move ? "white" : "black") << " to move\n";
		std::cout << board.getFen() << "\n";
		std::cout << board << std::endl;
#endif

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
			return 0;
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
			return 0;
		}
		long long ms_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		int side_idx = white_to_move ? 0 : 1;
		ms_sum[side_idx] += ms_elapsed;
		moves_calculated[side_idx] += moves.size();
		moves_played[side_idx]++;
		
		#ifdef DEBUG_PRINT
		std::cout << chess::uci::moveToUci(move) << " took " << ms_elapsed << "ms" << std::endl;
		std::cout << "------------------------\n";
		#endif // DEBUG_PRINT
		moves_str += chess::uci::moveToUci(move) + "\n";

		board.makeMove(move);
	}


	std::cout << "game over" << std::endl;
	std::cout << "color: " << (white_to_move ? "black" : "white") << std::endl;
	std::cout << "result: " << OUTCOME_STR[(int)board.isGameOver().first] << std::endl;
	std::cout << "time: \n"
		<< "white\tblack\n"
		<< ms_sum[0] << "\t" << ms_sum[1] << "\t ms_sum\n"
		<< moves_calculated[0] << "\t" << moves_calculated[1] << "\t moves_calculated\n"
		<< moves_played[0] << "\t" << moves_played[1] << "\t moves_played\n"
		<< ms_sum[0] / std::max(1, moves_calculated[0]) << "\t" << ms_sum[1] / std::max(1, moves_calculated[1]) << "\t ms_per_move_calculated	\n"
		<< ms_sum[0] / std::max(1, moves_played[0]) << "\t" << ms_sum[1] / std::max(1, moves_played[1]) << "\t ms_per_move_played\n";
	std::cout << "moves: \n" << moves_str << std::endl;
	std::cout << board.getFen() << "\n";
	return board.isGameOver().first != chess::GameResultReason::CHECKMATE ? 0 :
		white_to_move ? 1 : -1;
}
