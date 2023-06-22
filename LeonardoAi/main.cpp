#include "leonardo_overlord.hpp"
#include "../MockChessEngine/AlphaBetaPruningBot.h"
#include "../MockChessEngine/RandomPlayer.h"
#include <iomanip>
int main()
{
	//leonardo_overlord overlord("debugless");
	//overlord.train();
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