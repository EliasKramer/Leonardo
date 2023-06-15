#pragma once
#include "NeuroFox/vector3.hpp"
#include "../MockChessEngine/ChessBoard.h"
#include "NeuroFox/matrix.hpp"
namespace leonardo_util 
{
	vector3 get_input_format();
	vector3 get_output_format();
	void chessboard_to_matrix(const ChessBoard& board, matrix& input);
	int get_matrix_idx_for_move(const Move& move);
	int get_move_idx(const matrix& output, const UniqueMoveList& allowed_moves);
}