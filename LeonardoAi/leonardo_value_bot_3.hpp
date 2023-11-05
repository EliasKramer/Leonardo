#pragma once
//#define DEBUG_PRINT

#include "chess_player.hpp"
#include "NeuroFox/neural_network.hpp"
#include "stockfish_interface.hpp"

class tt_item
{
public:
	int depth;
	float value;
	float alpha;
	float beta;
};

class leonardo_value_bot_3 : public chess_player
{
private:
	size_t ms_per_move = 1000;

	float ms_per_forward_feed_sum = 0;
	float forward_feed_count = 1;
	//std::chrono::time_point start_point = 0;

	int end_depth;

	int pruned = 0;
	int nodes_visited = 0;

	int leaf_nodes = 0;
	int leaf_nodes_evaluated_nnet = 0;

	bool use_nnet = false;

	int print_count = 0;

	int transpositions_count = 0;

	//transposition table
	std::unordered_map<size_t, tt_item> tt;

	std::vector<std::pair<size_t, chess::Move>> openings;

	neural_network value_nnet;
	matrix input_matrix;

	tt_item* tt_get(chess::Board& board, int depth);
	void tt_store(chess::Board& board, int depth, float value, float alpha, float beta);

	void load_openings();

	float eval(chess::Board& board, chess::Movelist& moves, int depth);

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
