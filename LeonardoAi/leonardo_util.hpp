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

	//pawn
	vector3 get_pawn_input_format();
	void set_board_matrix(matrix& m, int z_idx, float val, chess::Bitboard bb, bool add, bool flip);

	vector3 get_sparse_input_format();
	void encode_m_to_sparse_matrix(const chess::Board& board, matrix& input, chess::Color col);
	void encode_m_to_sparse_matrix(const chess::Board& board, matrix& input);
	void set_value_nnet_output(matrix& output, float value);

	chess::Bitboard attack_bb(const chess::Board& board, chess::Color col);
}