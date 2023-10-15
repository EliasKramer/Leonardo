#pragma once

#include "chess_player.hpp"
#include "NeuroFox/neural_network.hpp"
#include "stockfish_interface.hpp"

class leonardo_value_bot_3 : public chess_player
{
private:
	int end_depth;

	int pruned = 0;
	int nodes_visited = 0;

	int leaf_nodes = 0;
	int leaf_nodes_evaluated_nnet = 0;

	int print_count = 0;

	std::vector<std::pair<size_t, chess::Move>> openings;

	neural_network value_nnet;
	matrix input_matrix;

	void load_openings();

	float eval(chess::Board& board, int depth);

	float recursive_eval(
		int depth,
		int depth_addition,
		chess::Board& board,
		float alpha,
		float beta,
		chess::Move& best_move);

	int get_opening_move(size_t hash);

	void sort_move_list(chess::Movelist& moves, chess::Board& board);
public:
	leonardo_value_bot_3();
	leonardo_value_bot_3(int end_depth);

	chess::Move get_move(chess::Board& board) override;
};
