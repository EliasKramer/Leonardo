#include "leonardo_bot.hpp"

void leonardo_bot::set_nn_input(const ChessBoard& board, float random_range)
{
	if (matrix::equal_format(input_board, input_format) == false)
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
			input_board.set_at(
				i,
				PIECETYPE_VALUE[piece.getType()] * multiplier
				+ random_range //TODO IMPROVE NOISE
			);
		}
		else
		{
			input_board.set_at_flat(i, 0.0f + random_range);
		}
	}
	input_board.apply_noise(0.5);
}

int leonardo_bot::get_matrix_idx_for_move(const Move& move)
{
	size_t x = move.getStart();
	size_t y = move.getDestination();

	return vector3(x, y).get_index(output_format);
}

int leonardo_bot::get_move_idx(
	const matrix& output,
	const UniqueMoveList& allowed_moves)
{
	if (matrix::equal_format(output.get_format(), output_format) == false)
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

leonardo_bot::leonardo_bot(
	neural_network& given_nn
) :
	Player("LeonardoBot"),
	nn(given_nn),
	input_board(input_format)
{
	if (nn.is_in_gpu_mode())
	{
		input_board.enable_gpu_mode();
	}
}

int leonardo_bot::getMove(const ChessBoard& board, const UniqueMoveList& moves)
{
	set_nn_input(board, 0.5f);
	input_board.sync_device_and_host();

	nn.forward_propagation(input_board);

	nn.get_output().sync_device_and_host();

	return get_move_idx(nn.get_output(), moves);
}

void leonardo_bot::train_on_game(const std::vector<std::string>& move_strings)
{
	if (nn.is_in_gpu_mode())
	{
		throw std::exception("gpu backprop not implemented yet");
	}

	ChessBoard board(STARTING_FEN);
	matrix expected = matrix(output_format);
	expected.set_all(0.0f);

	for (int i = 0; i < move_strings.size(); i++)
	{
		UniqueMoveList possible_moves = board.getAllLegalMoves();
		
		const std::string& curr_expected_move = move_strings[i];
		
		int best_move_idx = -1;

		int move_idx = 0;

		bool found = false;
		for (const std::unique_ptr<Move>& move : possible_moves)
		{
			if (move->getString() == curr_expected_move)
			{
				expected.set_at_flat(get_matrix_idx_for_move(*move), 1.0f);
				best_move_idx = move_idx;
				found = true;
				break;
			}
			move_idx++;
		}
		if (found == false)
		{
			std::cout << "invalid game. returning " << std::endl;
			break;
			//return would be better, but i have to write a "clear delta" function first
		}
		
		set_nn_input(board, 0);
		nn.back_propagation(input_board, expected);

		board.makeMove(*possible_moves[best_move_idx].get());
	}
	nn.apply_deltas(move_strings.size(), 1);
}
