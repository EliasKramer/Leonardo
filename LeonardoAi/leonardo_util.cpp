#include "leonardo_util.hpp"
#include <float.h>
#include <algorithm>

vector3 leonardo_util::get_input_format()
{
	//pawn
	//knight
	//bishop
	//rook
	//queen
	//king
	return vector3(8, 8, 6);
}

vector3 leonardo_util::get_input_format_one_hot()
{
	return vector3(8, 8, 12);
}

vector3 leonardo_util::get_policy_output_format()
{
	//z0 is the start field of the move
	//z1 is the destination field of the move
	//xy dimensions are the board
	return vector3(64, 64, 1);
}

vector3 leonardo_util::get_value_nnet_output_format()
{
	return vector3(1, 1, 1);
}

void leonardo_util::set_matrix_from_chessboard(const chess::Board& board, matrix& m, chess::Color col)
{
	smart_assert(m.host_data_is_updated());
	smart_assert(matrix::equal_format(m.get_format(), leonardo_util::get_input_format()));

	//BitBoard all_pieces = board.getBoardRepresentation().AllPieces;

	bool flipped = col == chess::Color::BLACK;

	chess::Bitboard our_pieces = board.us(col);

	m.set_all(0);

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			int square = vector3(x, y).get_index(vector3(8, 8, 1));
			chess::Piece curr_piece = board.at<chess::Piece>(chess::Square(square));
			if (curr_piece != chess::Piece::NONE)
			{
				chess::PieceType curr_type = board.at<chess::PieceType>(chess::Square(square));
				vector3 coord(x, flipped ? y : 7 - y, (int)curr_type);


				m.set_at_host(
					coord,
					//curr pos overlaps with curr pieces
					(our_pieces & (chess::Bitboard(1) << square)) != 0 ? 1 : -1
				);
			}
		}
	}
}

void leonardo_util::set_matrix_from_chessboard(const chess::Board& board, matrix& m)
{
	set_matrix_from_chessboard(board, m, board.sideToMove());
}

void leonardo_util::set_matrix_from_chessboard_one_hot(const chess::Board& board, matrix& m, chess::Color col)
{
	smart_assert(m.host_data_is_updated());
	smart_assert(matrix::equal_format(m.get_format(), leonardo_util::get_input_format_one_hot()));

	//BitBoard all_pieces = board.getBoardRepresentation().AllPieces;

	bool flipped = col == chess::Color::BLACK;

	chess::Bitboard our_pieces = board.us(col);

	m.set_all(0);

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			int square = vector3(x, y).get_index(vector3(8, 8, 1));
			chess::Piece curr_piece = board.at<chess::Piece>(chess::Square(square));
			if (curr_piece != chess::Piece::NONE)
			{
				chess::PieceType curr_type = board.at<chess::PieceType>(chess::Square(square));

				bool is_us = (our_pieces & (chess::Bitboard(1) << square)) != 0;

				vector3 coord(
					x,
					flipped ? y : 7 - y,
					is_us ? (int)curr_type : (int)curr_type + 6
				);


				m.set_at_host(
					coord,
					1
				);
			}
		}
	}
}

void leonardo_util::set_matrix_from_chessboard_one_hot(const chess::Board& board, matrix& input)
{
	set_matrix_from_chessboard_one_hot(board, input, board.sideToMove());
}


std::vector<std::string> leonardo_util::split_string(const std::string& input, char separator)
{
	std::vector<std::string> result;
	std::string current;

	for (char c : input) {
		if (c == separator) {
			if (!current.empty()) {
				result.push_back(current);
				current.clear();
			}
		}
		else {
			current += c;
		}
	}

	if (!current.empty()) {
		result.push_back(current);
	}

	return result;
}

float leonardo_util::get_value_nnet_output(matrix& output)
{
	if (!output.host_data_is_updated())
	{
		int x = 0;
	}
	smart_assert(output.host_data_is_updated());
	smart_assert(vector3::are_equal(output.get_format(), get_value_nnet_output_format()));

	//not necessary?
	//output.sync_device_and_host();

	//float own_score = output.get_at_flat_host(0);
	//float enemy_score = output.get_at_flat_host(1);

	return output.get_at_flat_host(0);// own_score - enemy_score;
}

float leonardo_util::get_value_nnet_eval(
	neural_network& value_nnet,
	matrix& input,
	chess::Board& board,
	bool double_eval)
{
	smart_assert(vector3::are_equal(input.get_format(), get_input_format()));

	set_matrix_from_chessboard(board, input, board.sideToMove());
	value_nnet.forward_propagation(input);
	matrix& output = value_nnet.get_output();
	float our_value = get_value_nnet_output(output);

	float their_value = 0;

	if (double_eval)
	{
		set_matrix_from_chessboard(board, input, ~board.sideToMove());
		value_nnet.forward_propagation(input);
		output = value_nnet.get_output();
		their_value = get_value_nnet_output(output);
	}

	return our_value - their_value;
}

vector3 leonardo_util::sq_to_pawn_matrix_pos(const chess::Square sq, const int z_idx)
{
	const chess::Bitboard curr_bb = chess::Bitboard(1) << sq;

	const int x = sq % 8;
	int y = sq / 8;

	y -= 1;

	return vector3(x, y, z_idx);
}

void leonardo_util::set_board_matrix(matrix& m, int z_idx, float val, chess::Bitboard bb)
{
	smart_assert(m.host_data_is_updated());

	while (bb)
	{
		const unsigned int sq = chess::builtin::poplsb(bb);

		m.set_at_host(
			sq_to_pawn_matrix_pos(chess::Square(sq), z_idx),
			val);
	}
}

vector3 leonardo_util::get_pawn_input_format()
{
	//0 white pawns
	//1 black pawns

	return vector3(8, 6, 2);
}

void leonardo_util::encode_pawn_matrix(const chess::Board& board, matrix& input)
{
	smart_assert(vector3::are_equal(input.get_format(), get_pawn_input_format()));

	input.set_all(0);

	chess::Bitboard w_pawns = board.pieces(chess::PieceType::PAWN, chess::Color::WHITE);
	chess::Bitboard b_pawns = board.pieces(chess::PieceType::PAWN, chess::Color::BLACK);

	set_board_matrix(input, 0, 1, w_pawns);
	set_board_matrix(input, 1, 1, b_pawns);
}

void leonardo_util::set_pawn_matrix_value(matrix& output, float value, chess::Color side_to_move)
{
	smart_assert(vector3::are_equal(output.get_format(), get_value_nnet_output_format()));

	float col_mult = side_to_move == chess::Color::WHITE ? 1 : -1;
	output.set_at_flat_host(0, value * col_mult);
}

float leonardo_util::get_pawn_matrix_value(matrix& output, chess::Color side_to_move)
{
	smart_assert(vector3::are_equal(output.get_format(), get_value_nnet_output_format()));

	float col_mult = side_to_move == chess::Color::WHITE ? 1 : -1;
	return output.get_at_flat_host(0) * col_mult;
}
float leonardo_util::get_pawn_matrix_value(matrix& output)
{
	smart_assert(vector3::are_equal(output.get_format(), get_value_nnet_output_format()));

	return output.get_at_flat_host(0);
}

void leonardo_util::make_move(chess::Board& board, matrix& pawn_board, const chess::Move& move)
{
	bool is_pawn_move = board.at<chess::PieceType>(move.from()) == chess::PieceType::PAWN;
	bool white_to_move = board.sideToMove() == chess::Color::WHITE;

	if (is_pawn_move)
	{
		bool is_promotion = move.typeOf() == chess::Move::PROMOTION;
		bool is_en_passant = move.typeOf() == chess::Move::ENPASSANT;

		if (is_en_passant) //just recalculate the pawn matrix if en passant
		{
			board.makeMove(move);
			encode_pawn_matrix(board, pawn_board);
			return;
		}
		int our_idx = white_to_move ? 0 : 1;
		int their_idx = white_to_move ? 1 : 0;

		pawn_board.set_at_host(sq_to_pawn_matrix_pos(move.from(), our_idx), 0);

		if (!is_promotion) // on promotions the .to() square gets out of bounds
		{
			pawn_board.set_at_host(sq_to_pawn_matrix_pos(move.to(), our_idx), 1);
			pawn_board.set_at_host(sq_to_pawn_matrix_pos(move.to(), their_idx), 0); // to handle captures
		}
	}

	bool is_capturing_pawn =
		((1ULL << move.to()) & board.pieces(chess::PieceType::PAWN)) != 0;

	if (is_capturing_pawn)
	{
		int their_idx = white_to_move ? 1 : 0;
		pawn_board.set_at_host(sq_to_pawn_matrix_pos(move.to(), their_idx), 0);
	}

	board.makeMove(move);
}

void leonardo_util::unmake_move(chess::Board& board, matrix& pawn_board, const chess::Move& move)
{
	bool is_promotion = move.typeOf() == chess::Move::PROMOTION;
	bool is_en_passant = move.typeOf() == chess::Move::ENPASSANT;
	if (is_en_passant || is_promotion) //just recalculate the pawn matrix if en passant or promotion
	{
		board.unmakeMove(move);
		encode_pawn_matrix(board, pawn_board);
		return;
	}

	bool is_pawn_move = board.at<chess::PieceType>(move.to()) == chess::PieceType::PAWN;
	bool white_to_move = board.sideToMove() == chess::Color::BLACK;
	int their_idx = white_to_move ? 1 : 0;

	if (is_pawn_move)
	{
		int our_idx = white_to_move ? 0 : 1;
		//if white is to unmake a move, then black has currently their turn

		pawn_board.set_at_host(sq_to_pawn_matrix_pos(move.from(), our_idx), 1);
		pawn_board.set_at_host(sq_to_pawn_matrix_pos(move.to(), our_idx), 0);

	}

	chess::Piece captured_piece = board.prevStates().back().captured_piece;

	if (captured_piece == chess::Piece::WHITEPAWN ||
		captured_piece == chess::Piece::BLACKPAWN)
	{
		pawn_board.set_at_host(sq_to_pawn_matrix_pos(move.to(), their_idx), 1); // to handle captures
	}


	board.unmakeMove(move);
}

bool leonardo_util::board_material_equal(chess::Board& board)
{
	static const float PIECE_EVAL[5] = { 100.0f, 300.0f, 300.0f, 500.0f, 900.0f };
	//only consider pieces, not pawns

	chess::Bitboard black_bb = board.us(chess::Color::BLACK);
	chess::Bitboard white_bb = board.us(chess::Color::WHITE);
	float score = 0;
	for (int i = 1; i < 5; i++)
	{
		chess::Bitboard curr_bb = board.pieces(chess::PieceType(i));

		while (curr_bb)
		{
			unsigned int sq = chess::builtin::poplsb(curr_bb);
			chess::Bitboard curr_bb = chess::Bitboard(1) << sq;
			if ((curr_bb & black_bb) != 0)
			{
				score -= PIECE_EVAL[i];
			}
			else
			{
				score += PIECE_EVAL[i];
			}
		}
	}

	return score == 0;
}
