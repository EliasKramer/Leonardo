#include "leonardo_overlord.hpp"
#include "../MockChessEngine/AlphaBetaPruningBot.h"
#include "../MockChessEngine/RandomPlayer.h"
int main()
{
	
	leonardo_overlord overlord("helo");
	overlord.debug();

	return 0;
	
	ChessBoard board(STARTING_FEN);
	
	std::vector<std::string> inputs = {
			"d2d3",
			"h7h5",
			"c1g5",
			"g7g6",
			"g1f3",
			"b8a6",
			"h1g1",
			"g8f6",
			"a2a4",
			"f8h6",
			"b1a3",
			"d7d6",
			"g1h1"
	};

	for (int i = 0; i < inputs.size(); i++)
	{
		auto legal_moves = board.getAllLegalMoves();
		bool found_move = false;
		for (int j = 0; j < legal_moves.size(); j++)
		{
			if (legal_moves[j].get()->getString() == inputs[i])
			{
				std::cout << "move: " << inputs[i] << std::endl;
				board.makeMove(*legal_moves[j].get());
				found_move = true;
				break;
			}
		}
		if (!found_move)
		{
			std::cout << "move not found: " << inputs[i] << std::endl;
			break;
		}
	}
	auto moves = board.getAllLegalMoves();
	for (auto& move : moves)
	{
		std::cout << move.get()->getString() << std::endl;
		if (move.get()->getString() == "O-O-O")
		{
			ChessBoard copy = board.getCopyByValue();
			copy.makeMove(*move.get());
			std::cout << copy.getFen();
		}
	}
}