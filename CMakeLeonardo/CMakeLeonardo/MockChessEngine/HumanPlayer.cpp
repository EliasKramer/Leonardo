#include "HumanPlayer.h"

HumanPlayer::HumanPlayer()
	:Player("Human Player")
{}

HumanPlayer::HumanPlayer(std::string name)
	:Player(name)
{}

int HumanPlayer::getMove(const ChessBoard& board, const UniqueMoveList& moves)
{
	while (true)
	{
		std::string input;
		std::cout << "Please enter your move: \n";
		std::cin >> input;

		int i = 0;
		for (const std::unique_ptr<Move>& curr : moves)
		{
			if (input == curr.get()->getString())
			{
				return i;
			}
			i++;
		}
		std::cout << "That was not a valid move\n";
	}
}