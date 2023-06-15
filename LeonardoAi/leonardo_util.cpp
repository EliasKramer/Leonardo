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

	for (int i = 0; i < 64; i++)
	{
		Square square = (Square)i;

		if (bitboardsOverlap(square, BB_SQUARE[square]))
		{
			ChessPiece piece = board.getBoardRepresentation().getPieceAt((Square)i);

			//the own pieces are 1 the others are -1
			int multiplier = piece.getColor() == board.getCurrentTurnColor() ? 1 : -1;
			m.set_at(
				i,
				PIECETYPE_VALUE[piece.getType()] * multiplier
			);
		}
		else
		{
			m.set_at_flat(i, 0.0f);
		}
	}
	m.apply_noise(0.5);
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
	throw std::exception("not implemented");
	return 0;
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
	output.set_at_flat(0, game.getGameState() == GameState::WhiteWon ? 1.0f : 0.0f);
	output.set_at_flat(1, game.getGameState() == GameState::BlackWon ? 1.0f : 0.0f);
}
