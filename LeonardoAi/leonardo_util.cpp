#include "leonardo_util.hpp"

vector3 leonardo_util::get_input_format()
{
	return vector3(8, 8, 1);
}

vector3 leonardo_util::get_policy_output_format()
{
	//z0 is the start field of the move
	//z1 is the destination field of the move
	//xy dimensions are the board
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
			vector3 coord(x, flipped ? y : 7 - y);
			Square square = (Square)vector3(x, y).get_index(vector3(8, 8, 1));

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

int leonardo_util::square_to_flat_idx(Square s, ChessColor color_to_move)
{
	static const vector3 board_dimensions(8, 8, 1);
	static vector3 coord(0, 0, 0);

	int x = s % 8;
	int y = s / 8;
	if (color_to_move == Black)
	{
		y = 7 - y;
	}
	coord.x = x;
	coord.y = y;

	return coord.get_index(board_dimensions);
}

float leonardo_util::get_move_value(const Move& move, const matrix& policy_output, ChessColor color)
{
	//remove statements for more speed
	if (matrix::equal_format(policy_output.get_format(), get_policy_output_format()) == false)
	{
		throw std::exception("output has wrong format");
	}
	if (policy_output.host_data_is_updated() == false)
	{
		throw std::exception("output is not updated");
	}

	static vector3 coord(0, 0, 0);

	int flat_start_idx = square_to_flat_idx(move.getStart(), color);
	int flat_dest_idx = square_to_flat_idx(move.getDestination(), color);

	coord.x = flat_start_idx;
	coord.y = flat_dest_idx;

	return policy_output.get_at_host(coord);
}

void leonardo_util::set_move_value(const Move& move, matrix& output, float value, const ChessColor color_to_move)
{
	if (matrix::equal_format(output.get_format(), get_policy_output_format()) == false)
	{
		throw std::exception("output has wrong format");
	}
	if (!output.host_data_is_updated())
	{
		throw std::exception("output is not synced");
	}

	static vector3 coord(0, 0, 0);

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
	if (matrix::equal_format(output.get_format(), get_policy_output_format()) == false)
	{
		throw std::exception("output has wrong format");
	}

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

int leonardo_util::get_random_best_move(
	const matrix& output,
	const UniqueMoveList& allowed_moves,
	ChessColor curr_turn_col)
{
	if (matrix::equal_format(output.get_format(), get_policy_output_format()) == false)
	{
		throw std::exception("output has wrong format");
	}
	//the implementation has negative values in mind
	float sum_positive = 0;
	float sum_negative = 0;

	for (const std::unique_ptr<Move>& move : allowed_moves)
	{
		float value = get_move_value(*move, output, curr_turn_col);
		value > 0 ? sum_positive += value : sum_negative += value;
	}
	
	if (sum_positive > 0)
	{
		float random = random_float_excl(0, sum_positive);

		float current_sum = 0;
		int move_idx = 0;
		for (const std::unique_ptr<Move>& move : allowed_moves)
		{
			float value = get_move_value(*move, output, curr_turn_col);
			if (value > 0)
			{
				current_sum += value;
				if (current_sum >= random)
				{
					return move_idx;
				}
				move_idx++;
			}
		}
	}
	else if (sum_negative < 0)
	{
		float random = random_float_excl(sum_negative, 0);
		
		float current_sum = 0;
		int move_idx = 0;

		for (const std::unique_ptr<Move>& move : allowed_moves)
		{
			float value = get_move_value(*move, output, curr_turn_col);
			if (value < 0)
			{
				current_sum += value;
				if (current_sum <= random)
				{
					return move_idx;
				}
				move_idx++;
			}
		}
	}
	else if (sum_positive == 0 && sum_negative == 0)
	{
		std::cout
			<< "all move values are 0 -> moves.size() = "
			<< allowed_moves.size() << std::endl;
		return random_idx(allowed_moves.size());
	}
	else
	{
		throw std::exception("something went wrong\n");
	}
}

void leonardo_util::set_prediction_output(matrix& output, const ChessBoard& game, ChessColor color)
{
	if (matrix::equal_format(output.get_format(), get_prediction_output_format()) == false)
	{
		throw std::exception("output has wrong format");
	}
	if (game.getGameState() == GameState::Ongoing)
	{
		throw std::exception("game is not over");
	}

	//high if color wins high -> low if  color loses - low

	//color wins
	// 1 | 0
	//color loses
	// 0 | 1

	output.sync_device_and_host();
	output.set_at_flat_host(0, 
		(game.getGameState() == GameState::WhiteWon && color == White) ||
		(game.getGameState() == GameState::BlackWon && color == Black) 
		? 1.0f : 0.0f);
	output.set_at_flat_host(1, 
		game.getGameState() == GameState::BlackWon && color == White ||
		game.getGameState() == GameState::WhiteWon && color == Black
		? 1.0f : 0.0f);
	output.sync_device_and_host();
}

float leonardo_util::get_prediction_output(matrix& output)
{
	if (matrix::equal_format(output.get_format(), get_prediction_output_format()) == false)
	{
		throw std::exception("output has wrong format");
	}

	output.sync_device_and_host();

	float own_score = output.get_at_flat_host(0);
	float enemy_score = output.get_at_flat_host(1);

	return own_score - enemy_score;
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