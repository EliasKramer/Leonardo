#include "leonardo_util.hpp"

vector3 leonardo_util::get_input_format()
{
	return vector3(8, 8, 1);
}

vector3 leonardo_util::get_policy_output_format()
{
	return vector3(64, 64, 1);
}

vector3 leonardo_util::get_prediction_output_format()
{
	return vector3(2, 1, 1);
}

void leonardo_util::set_matrix_from_chessboard(const ChessBoard& board, matrix& m)
{
	if (matrix::equal_format(m.get_format(), leonardo_util::get_input_format()) == false)
	{
		throw std::exception("input_board has wrong format");
	}

	BitBoard all_pieces = board.getBoardRepresentation().AllPieces;

	bool flipped = board.getCurrentTurnColor() == Black;

	//we do this in order to have the same order as the input format - no matter black or white's turn
	for (int i = 0; i < 64; i++)
	{
		Square square = (Square)i;

		int flat_idx = flipped ? 63 - i : i;
		if (bitboardsOverlap(all_pieces, BB_SQUARE[square]))
		{
			ChessPiece piece = board.getBoardRepresentation().getPieceAt((Square)i);

			//the own pieces are 1 the others are -1
			int multiplier = piece.getColor() == board.getCurrentTurnColor() ? 1 : -1;

			float value = piece.getType() == PieceType::King ? 1000 : PIECETYPE_VALUE[piece.getType()];
			m.set_at_flat(
				flat_idx,
				value * multiplier
			);
		}
		else
		{
			m.set_at_flat(flat_idx, 0.0f);
		}
	}
}

int leonardo_util::get_matrix_idx_for_move(const Move& move)
{
	size_t x = move.getStart();
	size_t y = move.getDestination();

	return vector3(x, y).get_index(get_policy_output_format());
}

int leonardo_util::get_best_move(const matrix& output, const UniqueMoveList& allowed_moves)
{
	if (matrix::equal_format(output.get_format(), get_policy_output_format()) == false)
	{
		throw std::exception("output has wrong format");
	}

	int max_idx = 0;
	float max_value = FLT_MIN;

	int move_idx = 0;
	for (const std::unique_ptr<Move>& move : allowed_moves)
	{
		size_t matrix_idx = get_matrix_idx_for_move(*move);
		float value = output.get_at_flat_host(matrix_idx);

		if (value > max_value)
		{
			max_value = value;
			max_idx = move_idx;
		}

		move_idx++;
	}

	return max_idx;
}

int leonardo_util::get_random_best_move(
	const matrix& output,
	const UniqueMoveList& allowed_moves)
{
	if (matrix::equal_format(output.get_format(), get_policy_output_format()) == false)
	{
		throw std::exception("output has wrong format");
	}

	float sum = 0;
	for (const std::unique_ptr<Move>& move : allowed_moves)
	{
		size_t matrix_idx = get_matrix_idx_for_move(*move);
		float value = output.get_at_flat_host(matrix_idx);
		sum += value;
	}

	float random = random_float_excl(0, sum);

	float current_sum = 0;
	int move_idx = 0;
	for (const std::unique_ptr<Move>& move : allowed_moves)
	{
		size_t matrix_idx = get_matrix_idx_for_move(*move);
		float value = output.get_at_flat_host(matrix_idx);
		current_sum += value;
		if (current_sum >= random)
		{
			return move_idx;
		}
		move_idx++;
	}

	throw std::exception("no move found");
}

void leonardo_util::set_prediction_output(matrix& output, const ChessBoard& game)
{
	if (matrix::equal_format(output.get_format(), get_prediction_output_format()) == false)
	{
		throw std::exception("output has wrong format");
	}
	if (game.getGameState() == GameState::Ongoing)
	{
		throw std::exception("game is not over");
	}
	output.sync_device_and_host();
	output.set_at_flat(0, game.getGameState() == GameState::WhiteWon ? 1.0f : 0.0f);
	output.set_at_flat(1, game.getGameState() == GameState::BlackWon ? 1.0f : 0.0f);
	output.sync_device_and_host();
}

float leonardo_util::get_prediction_output(matrix& output)
{
	if (matrix::equal_format(output.get_format(), get_prediction_output_format()) == false)
	{
		throw std::exception("output has wrong format");
	}

	output.sync_device_and_host();
	float white_score = output.get_at_flat_host(0);
	float black_score = output.get_at_flat_host(1);

	return white_score - black_score;
}

matrix& leonardo_util::matrix_map_get(
	std::unordered_map<ChessBoard, matrix, chess_board_hasher>& map,
	const ChessBoard& game)
{
	if (map.find(game) == map.end())
	{
		map.insert(std::make_pair(game, matrix(get_policy_output_format())));
	}

	matrix& m = map[game];
	if (matrix::equal_format(m.get_format(), get_policy_output_format()) == false)
	{
		throw std::exception("output has wrong format");
	}

	return m;
}

float leonardo_util::matrix_map_get_float(
	std::unordered_map<ChessBoard, matrix, chess_board_hasher>& map,
	const ChessBoard& game,
	const Move& move)
{
	matrix& m = matrix_map_get(map, game);
	int idx = get_matrix_idx_for_move(move);
	return m.get_at_flat_host(idx);
}

void leonardo_util::matrix_map_set_float(
	std::unordered_map<ChessBoard, matrix, chess_board_hasher>& map,
	const ChessBoard& game,
	const Move& move,
	float value)
{
	matrix& m = matrix_map_get(map, game);
	int idx = get_matrix_idx_for_move(move);
	m.set_at_flat(idx, value);
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
		int idx = get_matrix_idx_for_move(*move);
		sum += m.get_at_flat_host(idx);
	}
	return sum;
}
