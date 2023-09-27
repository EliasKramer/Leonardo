#pragma once
#include "chess.hpp"
#include "NeuroFox/vector3.hpp"
#include "NeuroFox/matrix.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <cassert>

namespace leonardo_util
{
		vector3 get_input_format();
		vector3 get_policy_output_format();
		vector3 get_value_nnet_output_format();

		void set_matrix_from_chessboard(const chess::Board& board, matrix& input);

		std::vector<std::string> split_string(const std::string& input, char separator);

		/*
		
		int square_to_flat_idx(Square s, ChessColor color_to_move);
		float get_move_value(const Move& move, const matrix& policy_output, ChessColor color);
		void set_move_value(const Move& move, matrix& output, float value, const ChessColor color_to_move);

		//has a matrix and picks the one with the highest value
		int get_best_move(const matrix& output, const UniqueMoveList& allowed_moves, ChessColor curr_turn_col);
		//has a matrix and picks the best move at random, but moves with higher value get picked more often
		int get_random_best_move(const matrix& output, const UniqueMoveList& allowed_moves, ChessColor curr_turn_col);

		void set_value_nnet_output(matrix& output, const ChessBoard& game, ChessColor color);

		float get_value_nnet_output(matrix& output);

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

		void update_thread(
			const size_t& progression,
			size_t total,
			size_t tick_in_ms);
			*/
}