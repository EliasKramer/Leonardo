#pragma once
#include "NeuroFox/vector3.hpp"
#include "../MockChessEngine/ChessBoard.h"
#include "NeuroFox/matrix.hpp"
namespace leonardo_util 
{
	vector3 get_input_format();
	vector3 get_policy_output_format();
	vector3 get_prediction_output_format();

	void set_matrix_from_chessboard(const ChessBoard& board, matrix& input);
	int get_matrix_idx_for_move(const Move& move);
	
	//has a matrix and picks the one with the highest value
	int get_best_move(const matrix& output, const UniqueMoveList& allowed_moves);
	//has a matrix and picks the best move at random, but moves with higher value get picked more often
	int get_random_best_move(const matrix& output, const UniqueMoveList& allowed_moves);

	void set_prediction_output(matrix& output, const ChessBoard& game);
}