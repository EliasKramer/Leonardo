#include "leonardo_bot.hpp"

leonardo_bot::leonardo_bot(
	neural_network& given_nn
) :
	Player("LeonardoBot"),
	nn(given_nn),
	input_board(leonardo_util::get_input_format())
{
	if (nn.is_in_gpu_mode())
	{
		input_board.enable_gpu_mode();
	}
}

int leonardo_bot::getMove(const ChessBoard& board, const UniqueMoveList& legal_moves)
{
	leonardo_util::set_matrix_from_chessboard(board, input_board);
	input_board.sync_device_and_host();

	nn.forward_propagation(input_board);

	nn.get_output().sync_device_and_host();

	return leonardo_util::get_random_best_move(nn.get_output(), legal_moves);
}