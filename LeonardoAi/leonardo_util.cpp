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
/*
int leonardo_util::square_to_flat_idx(Square s, ChessColor color_to_move)
{
	const vector3 board_dimensions(8, 8, 1);

	int x = s % 8;
	int y = s / 8;

	if (color_to_move == Black)
	{
		y = 7 - y;
	}

	return vector3(x, y, 0).get_index(board_dimensions);
}

float leonardo_util::get_move_value(const Move& move, const matrix& policy_output, ChessColor color)
{
	smart_assert(vector3::are_equal(policy_output.get_format(), get_policy_output_format()));
	smart_assert(policy_output.host_data_is_updated());

	vector3 coord(0, 0, 0);

	int flat_start_idx = square_to_flat_idx(move.getStart(), color);
	int flat_dest_idx = square_to_flat_idx(move.getDestination(), color);

	coord.x = flat_start_idx;
	coord.y = flat_dest_idx;

	return policy_output.get_at_host(coord);
}

void leonardo_util::set_move_value(const Move& move, matrix& output, float value, const ChessColor color_to_move)
{
	smart_assert(vector3::are_equal(output.get_format(), get_policy_output_format()));
	smart_assert(output.host_data_is_updated());

	vector3 coord(0, 0, 0);

	int flat_start_idx = square_to_flat_idx(move.getStart(), color_to_move);
	int flat_dest_idx = square_to_flat_idx(move.getDestination(), color_to_move);

	coord.x = flat_start_idx;
	coord.y = flat_dest_idx;

	output.set_at_host(coord, value);
}

int leonardo_util::get_best_move(
	const matrix& output,
	const UniqueMoveList& allowed_moves,
	ChessColor curr_turn_col)
{
	smart_assert(vector3::are_equal(output.get_format(), get_policy_output_format()));

	int max_idx = 0;
	float max_value = FLT_MIN;

	int move_idx = 0;
	for (const std::unique_ptr<Move>& move : allowed_moves)
	{
		float value = get_move_value(*move, output, curr_turn_col);

		if (value > max_value)
		{
			max_value = value;
			max_idx = move_idx;
		}

		move_idx++;
	}

	return max_idx;
}

static double output_to_exponent(float value)
{
	//would be a nice implementation, if the compiler would support it
	//std::clamp((double)value / 10,-7097,7097);

	//these values got found by writing a little test script.
	//exp((double)x/10) is invalid if x is smaller than -7097 or larger than 7097
	if (value < -7097)
	{
		std::cout << "WARNING: value too small for exponentiation. Clamping to -7097.\n";
		return -7097;
	}
	else if (value > 7097)
	{
		std::cout << "WARNING: value too large for exponentiation. Clamping to 7097.\n";
		return 7097;
	}
	else
	{
		return (double)value / 10;
	}
}

int leonardo_util::get_random_best_move(
	const matrix& output,
	const UniqueMoveList& allowed_moves,
	ChessColor curr_turn_col)
{
	smart_assert(matrix::equal_format(output.get_format(), get_policy_output_format()));

	//we are implementing softmax here instead of the nnet, in order to save some time.
	//a layer that does softmax would be cleaner

	double e_sum = 0;
	for (const std::unique_ptr<Move>& move : allowed_moves)
	{
		float value = get_move_value(*move, output, curr_turn_col);
		double e = exp(output_to_exponent(value));
		e_sum += e;
	}

	float random = random_float_excl(0, 1);
	//std::cout << "random: " << random << std::endl;

	double current_sum = 0;
	int move_idx = 0;
	for (const std::unique_ptr<Move>& move : allowed_moves)
	{
		float value = get_move_value(*move, output, curr_turn_col);
		double e = exp((output_to_exponent(value)));

		double prob = e / e_sum;

		//std::cout << "m_idx " << move_idx << " curr sum: " << current_sum << " value " << value << " prob: " << prob << " e " << e << " esum " << e_sum << std::endl;

		current_sum += prob;
		if (current_sum >= random)
		{
			//std::cout << "move found: " << move_idx << std::endl;
			return move_idx;
		}
		move_idx++;
	}

	//this could occur due to rounding errors
	//in very few cases
	std::cout << "no move found\n";
	return 0;
}

void leonardo_util::set_value_nnet_output(matrix& output, const ChessBoard& game, ChessColor color)
{
	GameState state = game.getGameState();
	smart_assert(vector3::are_equal(output.get_format(), get_value_nnet_output_format()));
	smart_assert(output.host_data_is_updated());
	smart_assert(state != GameState::Ongoing);

	//color wins
	// 1 | 0
	//color loses
	// 0 | 1
	//draw or stalemate
	// 0.5 | 0.5
	/*
	output.set_at_flat_host(0,
		(state == GameState::WhiteWon && color == White) ||
		(state == GameState::BlackWon && color == Black)
		? 1.0f : 0.0f);
	output.set_at_flat_host(1,
		state == GameState::BlackWon && color == White ||
		state == GameState::WhiteWon && color == Black
		? 1.0f : 0.0f);

	if (state == GameState::Draw || state == GameState::Stalemate)
	{
		output.set_at_flat_host(0, 0.5f);
		output.set_at_flat_host(1, 0.5f);
	}
	output.set_at_flat_host(0,
		(state == GameState::WhiteWon && color == White) ||
		(state == GameState::BlackWon && color == Black) ? 50.0f :
		(state == GameState::BlackWon && color == White) ||
		(state == GameState::WhiteWon && color == Black) ? -50.0f : 0.0f);

	output.sync_device_and_host();
}


matrix& leonardo_util::matrix_map_get(
	std::unordered_map<ChessBoard, matrix, chess_board_hasher>& map,
	const ChessBoard& game)
{
	//insert it into map if it does not exist
	if (map.find(game) == map.end())
	{
		map.insert(std::make_pair(game, matrix(get_policy_output_format())));
	}

	matrix& m = map[game];

	smart_assert(vector3::are_equal(m.get_format(), get_policy_output_format()));

	return m;
}

float leonardo_util::matrix_map_get_float(
	std::unordered_map<ChessBoard, matrix, chess_board_hasher>& map,
	const ChessBoard& game,
	const Move& move)
{
	matrix& m = matrix_map_get(map, game);
	return get_move_value(move, m, game.getCurrentTurnColor());
}

void leonardo_util::matrix_map_set_float(
	std::unordered_map<ChessBoard, matrix, chess_board_hasher>& map,
	const ChessBoard& game,
	const Move& move,
	float value)
{
	matrix& m = matrix_map_get(map, game);
	set_move_value(move, m, value, game.getCurrentTurnColor());
}

float leonardo_util::matrix_map_sum(
	std::unordered_map<ChessBoard, matrix, chess_board_hasher>& map,
	const ChessBoard& game,
	std::vector<std::unique_ptr<Move>>& legal_moves)
{
	matrix& m = matrix_map_get(map, game);
	float sum = 0;
	for (const std::unique_ptr<Move>& move : legal_moves)
	{
		sum += get_move_value(*move, m, game.getCurrentTurnColor());
	}
	return sum;
}

void leonardo_util::update_thread(
	const size_t& progression,
	size_t total,
	size_t tick_in_ms)
{
	long long start = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();

	while (true)
	{
		long long now = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
		long long passed_ms = now - start;

		std::cout
			<< "progress: " << progression << "/" << total
			<< " (" << (float)progression / (float)total * 100.0f << "%)"
			<< " passed: " << ms_to_str(passed_ms)
			<< " remaining: " << ms_to_str((long long)((float)passed_ms / (float)progression * (float)(total - progression)))
			<< std::endl;

		if (progression >= total)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(tick_in_ms));
	}
}
*/