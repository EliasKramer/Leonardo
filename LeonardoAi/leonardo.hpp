#pragma once

#include "chess_player.hpp"
#include "NeuroFox/neural_network.hpp"
#include "stockfish_interface.hpp"
#include "nnet_table.hpp"
#include "leonardo_util.hpp"
#include "chess_eval_constants.hpp"

class leonardo
{
private:
	//time control
	std::chrono::steady_clock::time_point deadline;
	
	//iterative deepening
	uint32_t iterative_deepening_depth = 1;
	uint32_t reached_depth = 0;

	bool search_cancelled();
	
	[[nodiscard]] int32_t corrected_result_score(chess::GameResult result, int32_t ply_from_root);
	[[nodiscard]] int32_t eval(chess::Board& board, chess::Movelist& moves, int ply, bool only_caputes_in_moves);

	int32_t search(
		int32_t ply_remaining,
		int32_t ply_from_root,
		chess::Board& board,
		int32_t alpha,
		int32_t beta,
		chess::Move& best_move);

	void setup_members(int32_t ms_left);
public:
	leonardo(uint32_t hash_table_size_mb);

	chess::Move get_move(chess::Board& board, int32_t ms_left, std::string& info);
};
