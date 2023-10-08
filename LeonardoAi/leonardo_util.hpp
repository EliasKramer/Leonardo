#pragma once
#include "chess.hpp"
#include "NeuroFox/vector3.hpp"
#include "NeuroFox/matrix.hpp"
#include "NeuroFox/neural_network.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <cassert>

namespace leonardo_util
{
	vector3 get_input_format();
	vector3 get_input_format_one_hot();

	vector3 get_policy_output_format();
	vector3 get_value_nnet_output_format();

	void set_matrix_from_chessboard(const chess::Board& board, matrix& input, chess::Color col);
	void set_matrix_from_chessboard(const chess::Board& board, matrix& input);

	void set_matrix_from_chessboard_one_hot(const chess::Board& board, matrix& input, chess::Color col);
	void set_matrix_from_chessboard_one_hot(const chess::Board& board, matrix& input);

	std::vector<std::string> split_string(const std::string& input, char separator);

	float get_value_nnet_output(matrix& output);

	float get_value_nnet_eval(
		neural_network& value_nnet,
		matrix& input,
		chess::Board& board,
		bool double_eval);

	vector3 sq_to_pawn_matrix_pos(const chess::Square sq, const int z_idx);
	void set_board_matrix(matrix& m, int z_idx, float val, chess::Bitboard bb);

	vector3 get_pawn_input_format();
	void encode_pawn_matrix(const chess::Board& board, matrix& input);
	void set_pawn_matrix_value(matrix& output, float value, chess::Color side_to_move);
	float get_pawn_matrix_value(matrix& output, float value, chess::Color side_to_move);

	//executes the move on the board and the pawn board
	void make_move(chess::Board& board, matrix& pawn_board, const chess::Move& move);
	//unmakes the move on the board and the pawn board
	void unmake_move(chess::Board& board, matrix& pawn_board, const chess::Move& move);
}