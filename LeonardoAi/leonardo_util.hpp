#pragma once
#include "chess.hpp"
#include "NeuroFox/vector3.hpp"
#include "NeuroFox/matrix.hpp"
#include "NeuroFox/neural_network.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <cassert>
#include "nnet_table.hpp"

namespace leonardo_util
{
	vector3 get_input_format();
	vector3 get_input_format_one_hot();
	vector3 get_input_format_duration_nnet();

	vector3 get_policy_output_format();
	vector3 get_value_nnet_output_format();

	void set_matrix_from_chessboard(const chess::Board& board, matrix& input, chess::Color col);
	void set_matrix_from_chessboard(const chess::Board& board, matrix& input);

	void set_matrix_from_chessboard_one_hot(const chess::Board& board, matrix& input, chess::Color col);
	void set_matrix_from_chessboard_one_hot(const chess::Board& board, matrix& input);
	void set_matrix_from_chessboard_duration_nnet(const chess::Board& board, matrix& input);

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
	vector3 get_pawn_white_turn_pos();
	vector3 get_pawn_black_turn_pos();
	void encode_pawn_matrix(const chess::Board& board, matrix& input);
	void encode_pawn_matrix(chess::Bitboard w_pawns, chess::Bitboard b_pawns, matrix& input, bool white_to_move);
	void set_pawn_matrix_value(matrix& output, float value, chess::Color side_to_move);
	void set_pawn_matrix_value(matrix& output, float value);
	float get_pawn_matrix_value(matrix& output, chess::Color side_to_move);
	float get_pawn_matrix_value(matrix& output);

	//executes the move on the board and the pawn board
	void make_move(chess::Board& board, matrix& pawn_board, const chess::Move& move, neural_network& pawn_nnet);
	//unmakes the move on the board and the pawn board
	void unmake_move(chess::Board& board, matrix& pawn_board, const chess::Move& move, neural_network& pawn_nnet);

	bool use_position(chess::Board& board);
	std::string get_pawn_structure_str(chess::Board& board);

	chess::Square get_random_square(chess::Bitboard bb);
	void remove_random_pawns(chess::Board& board);

	std::string pawn_board_to_str(const matrix& pawn_board);

	int get_board_val(
		chess::Bitboard curr_white_bb, 
		chess::Bitboard curr_black_bb, 
		chess::Bitboard& prev_white_bb, 
		chess::Bitboard& prev_black_bb, 
		neural_network& pawn_nnet,
		matrix& curr_input,
		nnet_table& table,
		bool white_to_move
	);

	int get_ms_to_think(
		neural_network& duration_nnet,
		matrix& input,
		chess::Board& board,
		int time_to_move,
		int time_remaining,
		int time_increment);

	chess::Move get_move_from_uci(const chess::Board& board, const std::string uci_move);
}