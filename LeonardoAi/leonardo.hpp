#pragma once
//#define DEBUG_PRINT

#include "chess_player.hpp"
#include "NeuroFox/neural_network.hpp"
#include "stockfish_interface.hpp"
#include "nnet_table.hpp"
#include "leonardo_util.hpp"
//Credit to https://web.archive.org/web/20071031100051/http://www.brucemo.com/compchess/programming/hashing.htm

class leonardo
{
private:
	int ms_per_move = -1;
	bool searched_at_least_one_move = false;
	int iterative_deepening_depth = 1;
	std::chrono::steady_clock::time_point start_time;

	bool search_cancelled();

	int eval(chess::Board& board, chess::Movelist& moves, int ply, bool only_caputes_in_moves);

	int search(
		int ply_remaining,
		int ply_from_root,
		chess::Board& board,
		int alpha,
		int beta,
		chess::Move& best_move);
public:
	leonardo(uint32_t hash_table_size_mb);

	chess::Move get_move(chess::Board& board, int ms_left, std::string& info);
};
