#pragma once

#include "chess_player.hpp"
#include "NeuroFox/neural_network.hpp"

class leonardo_value_bot_1 : public chess_player
{
private:
	int end_depth;

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
		float beta);

	int get_opening_move(size_t hash);

	void sort_move_list(chess::Movelist& moves, chess::Board& board);
public:
	leonardo_value_bot_1();
	leonardo_value_bot_1(int end_depth);

	chess::Move get_move(chess::Board& board) override;
};

