#include "human_player.hpp"

chess::Move human_player::get_move(chess::Board& pos)
{
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, pos);

	std::cout << pos;

	std::string input = "";
	while (true)
	{
		std::cout << "Enter move: ";
		std::cin >> input;

		if (input == "list")
		{
			for (int i = 0; i < moves.size(); i++)
			{
				std::cout << chess::uci::moveToUci(moves[i]) << "\n";
			}
			continue;
		}

		for (chess::Move m : moves)
		{
			if (chess::uci::moveToUci(m) == input)
			{
				return m;
			}
		}
		std::cout << "\nInvalid move!\n";
	}
}
