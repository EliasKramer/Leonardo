#include <iostream>

#include "Types.h"
#include "Engine.h"
#include "Board.h"

int main()
{
	map pawns = 0x800002010000400;
	map rooks = 0x1000000000;
	map whitePieces = 0x800001010000400;
	map blackPieces = 0x2000000000;
	Board board(pawns, 0, rooks, 0, 0, 0, whitePieces, blackPieces);
	std::vector<Move> moves = getMoves(board, WHITE);
	
	for (Move move : moves)
	{
		std::string from = SQARE_NAMES.at(move.getFrom());
		std::string to = SQARE_NAMES.at(move.getTo());
		std::cout << "from " << from << " to " << to << "\n";
		if (move.getPromotion() != NONE)
		{
			int promotion = move.getPromotion();
			std::cout << "promotion " << promotion << "\n";
		}
	}
}