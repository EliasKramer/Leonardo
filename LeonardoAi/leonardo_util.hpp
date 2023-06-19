#pragma once
#include "NeuroFox/vector3.hpp"
#include "../MockChessEngine/ChessBoard.h"
#include "NeuroFox/matrix.hpp"
#include <unordered_map>
#include <vector>
#include <memory>

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
	//returns between -1 and 1 (1 means white won, -1 means black won)
	float get_prediction_output(matrix& output);

	matrix& matrix_map_get(
		std::unordered_map<ChessBoard, matrix, chess_board_hasher>& map,
		const ChessBoard& game);

	float matrix_map_get_float(
		std::unordered_map<ChessBoard, matrix, chess_board_hasher>& map,
		const ChessBoard& game,
		const Move& move
	);
	void matrix_map_set_float(
		std::unordered_map<ChessBoard, matrix, chess_board_hasher>& map,
		const ChessBoard& game,
		const Move& move,
		float value
	);

	float matrix_map_sum(
		std::unordered_map<ChessBoard, matrix, chess_board_hasher>& map,
		const ChessBoard& game,
		std::vector<std::unique_ptr<Move>>& legal_moves);
}