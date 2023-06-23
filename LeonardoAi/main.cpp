#include "leonardo_overlord.hpp"
#include "../MockChessEngine/AlphaBetaPruningBot.h"
#include "../MockChessEngine/RandomPlayer.h"
#include <iomanip>
void test_board(
	const std::string& first_fen,
	const std::string& second_fen)
{
	ChessBoard w_board(first_fen);
	ChessBoard b_board(second_fen);

	matrix w_matrix(leonardo_util::get_input_format());
	matrix b_matrix(leonardo_util::get_input_format());

	leonardo_util::set_matrix_from_chessboard(w_board, w_matrix);
	leonardo_util::set_matrix_from_chessboard(b_board, b_matrix);
	
	std::cout << "----------------------\n";
	w_board.getCurrentTurnColor() == White ? std::cout << "White to move\n" : std::cout << "Black to move\n";

	for (int y = 0; y < 8; y++)
	{
		std::cout << "|";
		for (int x = 0; x < 8; x++)
		{
			std::cout << std::setw(5) << w_matrix.get_at_host(vector3(x, y)) << "|";
		}
		std::cout << std::endl;
	}
	std::cout << "++++++++++++++++++++++\n";
	std::cout << w_board.getString();
	std::cout << "----------------------\n";

	std::cout << "----------------------\n";
	b_board.getCurrentTurnColor() == White ? std::cout << "White to move\n" : std::cout << "Black to move\n";

	for (int y = 0; y < 8; y++)
	{
		std::cout << "|";
		for (int x = 0; x < 8; x++)
		{
			std::cout << std::setw(5) << b_matrix.get_at_host(vector3(x, y)) << "|";
		}
		std::cout << std::endl;
	}
	std::cout << "++++++++++++++++++++++\n";
	std::cout << b_board.getString();
	std::cout << "----------------------\n";
}

int main()
{
	//leonardo_overlord overlord("debugless");
	//overlord.train();

	test_board(
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"
	);
	return 0;
	ChessBoard board(STARTING_FEN);
	RandomPlayer random_player;


	while (board.getGameState() == Ongoing)
	{
		matrix m(leonardo_util::get_input_format());
		leonardo_util::set_matrix_from_chessboard(board, m);
		std::cout << "----------------------\n";
		board.getCurrentTurnColor() == White ? std::cout << "White to move\n" : std::cout << "Black to move\n";

		for (int y = 0; y < 8; y++)
		{
			std::cout << "|";
			for (int x = 0; x < 8; x++)
			{
				std::cout << std::setw(5) << m.get_at_host(vector3(x, y)) << "|";
			}
			std::cout << std::endl;
		}
		std::cout << "++++++++++++++++++++++\n";
		std::cout << board.getString();
		std::cout << "----------------------\n";

		auto moves = board.getAllLegalMoves();
		int move_idx = random_player.getMove(board, moves);
		board.makeMove(*moves[move_idx].get());
	}
}