#include "leonardo_util.hpp"

vector3 leonardo_util::get_input_format()
{
	return vector3(8, 8, 1);
}

vector3 leonardo_util::get_policy_output_format()
{
	//y0 is the start field of the move
	//y1 is the destination field of the move
	//x dimension is the square_idx
	return vector3(64, 2, 1);
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
	if (m.item_count() != 64)
	{
		throw std::exception("input format not supported");
	}

	BitBoard all_pieces = board.getBoardRepresentation().AllPieces;

	bool flipped = board.getCurrentTurnColor() == Black;

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			vector3 coord(x, flipped ? 7 - y : y);
			Square square = (Square)coord.get_index(vector3(8, 8, 1));

			if (bitboardsOverlap(all_pieces, BB_SQUARE[square]))
			{
				ChessPiece piece = board.getBoardRepresentation().getPieceAt(square);

				//the own pieces are 1 the others are -1
				int multiplier = piece.getColor() == board.getCurrentTurnColor() ? 1 : -1;

				float value = piece.getType() == PieceType::King ? 1000 : PIECETYPE_VALUE[piece.getType()];
				m.set_at_host(
					coord,
					value * multiplier
				);
			}
			else
			{
				m.set_at_host(coord, 0.0f);
			}
		}
	}
}

float leonardo_util::get_move_value(const Move& move, const matrix& policy_output)
{
	if (matrix::equal_format(policy_output.get_format(), get_policy_output_format()) == false)
	{
		throw std::exception("output has wrong format");
	}
	if (policy_output.host_data_is_updated() == false)
	{
		throw std::exception("output is not updated");
	}

	Square start = move.getStart();
	Square dest = move.getDestination();

	int flat_start_idx = start;
	int flat_dest_idx = 64 + dest;

	float start_value = policy_output.get_at_flat_host(flat_start_idx);
	float dest_value = policy_output.get_at_flat_host(flat_dest_idx);

	return start_value * dest_value;
}

void leonardo_util::set_move_value(const Move& move, matrix& output, float value)
{
	if (matrix::equal_format(output.get_format(), get_policy_output_format()) == false)
	{
		throw std::exception("output has wrong format");
	}
	if (!output.host_data_is_updated())
	{
		throw std::exception("output is not synced");
	}

	vector3 start_coord = vector3(move.getStart(), 0, 0);
	vector3 dest_coord = vector3(move.getDestination(), 1, 0);

	float start_value = output.get_at_host(start_coord);
	float dest_value = output.get_at_host(dest_coord);

	if (start_value < 0 || dest_value < 0 || value < 0)
	{
		throw std::exception("negative value");
	}
	if (start_value != 0 && dest_value != 0)
	{
		throw std::exception("move already set");
	}

	if (start_value != 0.0f)
	{
		output.set_at_host(dest_coord, value / start_value);
	}
	else if (dest_value != 0.0f)
	{
		output.set_at_host(start_coord, value / dest_value);
	}
	else
	{
		//coud be set at the start aswell
		output.set_at_host(dest_coord, value);
	}
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
		float value = get_move_value(*move, output);

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
		float value = get_move_value(*move, output);
		sum += value;
	}

	float random = random_float_excl(0, sum);

	float current_sum = 0;
	int move_idx = 0;
	for (const std::unique_ptr<Move>& move : allowed_moves)
	{
		float value = get_move_value(*move, output);
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
	output.set_at_flat_host(0, game.getGameState() == GameState::WhiteWon ? 1.0f : 0.0f);
	output.set_at_flat_host(1, game.getGameState() == GameState::BlackWon ? 1.0f : 0.0f);
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
	//insert it into map if it does not exist
	if (map.find(game) == map.end())
	{
		map.insert(std::make_pair(game, matrix(get_policy_output_format())));
	}
	//find it
	matrix& m = map[game];
	//format has to be correct
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
	return get_move_value(move, m);
}

void leonardo_util::matrix_map_set_float(
	std::unordered_map<ChessBoard, matrix, chess_board_hasher>& map,
	const ChessBoard& game,
	const Move& move,
	float value)
{
	matrix& m = matrix_map_get(map, game);
	set_move_value(move, m, value);
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
		sum += get_move_value(*move, m);
	}
	return sum;
}
