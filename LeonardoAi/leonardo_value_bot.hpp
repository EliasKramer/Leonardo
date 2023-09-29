#pragma once

#include "chess_player.hpp"
#include "NeuroFox/neural_network.hpp"

class leonardo_value_bot : public chess_player
{
private:
	int start_depth;

	std::vector<std::pair<size_t, chess::Move>> openings;
	
	neural_network value_nnet;
	matrix input_matrix;

	void load_openings();

	float eval(chess::Board& board, int depth);
	float eval(chess::Board& board);

	float recursive_eval(
		int depth,
		chess::Board& board,
		bool maximizing,
		float alpha,
		float beta);

	int get_opening_move(size_t hash);

public:
	leonardo_value_bot();
	leonardo_value_bot(int start_depth);

	chess::Move get_move(chess::Board& board) override;
};

