#include "leonardo_bot.hpp"

leonardo_bot::leonardo_bot(
	neural_network& given_nn,
	e_playstyle_t given_playstyle
) :
	Player("LeonardoBot"),
	nn(given_nn),
	playstyle(given_playstyle),
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

	return 
		playstyle == e_playstyle_t::max ? 
			leonardo_util::get_best_move(nn.get_output(), legal_moves, board.getCurrentTurnColor()) :
		playstyle == e_playstyle_t::distributed_random ? 
			leonardo_util::get_random_best_move(nn.get_output(), legal_moves, board.getCurrentTurnColor()) :
		throw std::exception("unknown playstyle");
}