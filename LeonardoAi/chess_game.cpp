#include "chess_game.hpp"
#include "random_player.hpp"
#include <iostream>
#include <thread>

chess_game::chess_game(chess_player* white_player, chess_player* black_player)
	: white_player(white_player), black_player(black_player)
{
	initialise_all_databases();
	zobrist::initialise_zobrist_keys();
}

void chess_game::play()
{
	std::string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	Position::set(starting_fen, pos);

	random_player* random_playr = new random_player;

	std::string moves_str = "";
	int i = 0;
	while (true)
	{
		i++;
		std::cout << pos << "\n";

		Color turn = pos.turn();


		if (turn == WHITE)
		{
			MoveList<WHITE> moves(pos);
			if (moves.size() == 0)
			{
				if (pos.in_check<WHITE>())
				{
					std::cout << "WHITE wins!\n";
				}

				std::cout << "draw\n";
				break;
			}
			Move move = random_playr->get_move(pos);
			pos.play<WHITE>(move);
			moves_str += to_string(move) + "\n";
			std::cout << to_string(move) + "\n";
		}
		else
		{
			MoveList<BLACK> moves(pos);
			if (moves.size() == 0)
			{
				if (pos.in_check<BLACK>())
				{
					std::cout << "BLACK wins!\n";
				}

				std::cout << "draw\n";

				break;
			}
			std::cout << black_player << "\n";
			std::cout << i << "\n";
			Move move = random_playr->get_move(pos);
			
			pos.play<BLACK>(move);
			moves_str += to_string(move) + "\n";
			std::cout << to_string(move) + "\n";
		}
	}
	std::cout << "\n" << moves_str << "\n";
}
