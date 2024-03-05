#pragma once

#include "chess_player.hpp"
#include "NeuroFox/neural_network.hpp"
#include "stockfish_interface.hpp"
#include "nnet_table.hpp"
#include "leonardo_util.hpp"
#include "static_eval.hpp"
#include "chess_constants.hpp"
#include "transposition_table.hpp"

class leonardo
{
private:
	//time control
	std::chrono::steady_clock::time_point deadline;
	
	//iterative deepening
	uint32_t iterative_deepening_depth = 1;
	uint32_t reached_depth = 0;

	//transposition table
	transposition_table tt;

	bool search_cancelled();
	
	[[nodiscard]] int32_t corrected_result_score(chess::GameResult result, int32_t ply_from_root);
	[[nodiscard]] bool move_causes_draw(chess::Board& board, chess::Move& move);

	[[nodiscard]] int32_t eval(chess::Board& board, chess::Movelist& moves, int ply, bool only_caputes_in_moves);

	int32_t search(
		int32_t ply_remaining,
		int32_t ply_from_root,
		chess::Board& board,
		int32_t alpha,
		int32_t beta,
		chess::Move& best_move);

	void sort_move_list(chess::Movelist& moves, chess::Board& board, int32_t ply_from_root, int32_t depth);

	void setup_members(int32_t ms_left);
public:
	leonardo(uint32_t hash_table_size_mb);

	chess::Move get_move(chess::Board& board, int32_t ms_left, std::string& info);
	
	void resize_tt(uint32_t hash_table_size_mb);
};
