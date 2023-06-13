#pragma once

#include "../MockChessEngine/Player.h"
#include "NeuroFox/neural_network.hpp"
class leonardo_bot : public Player
{
private:
	vector3 input_format = vector3(8, 8, 1);
	vector3 output_format = vector3(64, 64, 1);
	
	neural_network& nn;
	matrix input_board;

	void set_nn_input(const ChessBoard& board, float random_range);

	int get_matrix_idx_for_move(const Move& move);
	int get_move_idx(const matrix& output, const UniqueMoveList& allowed_moves);
public:
	leonardo_bot(neural_network& given_nn);

	int getMove(const ChessBoard& board, const UniqueMoveList& moves) override;

	void train_on_game(const std::vector<std::string>& move_strings);
};

