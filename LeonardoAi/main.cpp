#include "leonardo_overlord.hpp"
#include "../MockChessEngine/AlphaBetaPruningBot.h"
#include "../MockChessEngine/RandomPlayer.h"
int main()
{
	//leonardo_overlord overlord("helo");
	//overlord.train();

	int hashset_sum = 0;
	int move_sum = 0;
	for (int i = 0; i < 10000000000; i++)
	{
		ChessBoard board(STARTING_FEN);
		RandomPlayer bot;
		std::unordered_set<ChessBoard, chess_board_hasher> hashset;

		while (board.getGameState() == GameState::Ongoing)
		{
			hashset.insert(board);
			auto moves = board.getAllLegalMoves();
			int move_idx = bot.getMove(board, moves);
			board.makeMove(*moves[move_idx].get());
		}
		hashset_sum += hashset.size();
		move_sum += (board.getNumberOfMovesPlayed() * 2);

		std::cout 
			<< i << " " 
			<< "hashset_sum: " << hashset_sum << " "
			<< "move_sum: " << move_sum 
			<< " percent: " << (((float)hashset_sum/ (float)move_sum)*100)
			<< std::endl;
	}
}