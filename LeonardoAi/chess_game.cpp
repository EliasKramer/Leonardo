#include "chess_game.hpp"

chess_game::chess_game(chess_player& white_player, chess_player& black_player)
	: white_player(white_player), black_player(black_player)
{
	//Position::set(DEFAULT_FEN, pos);
}

void chess_game::play()
{
	while (true)
	{
		Stockfish::Move move;
		//std::cout << pos << std::endl;
		/*if (pos.turn() == WHITE)
		{
			move = white_player.get_move(pos);
			pos.play<WHITE>(move);
		}
		else
		{
			move = black_player.get_move(pos);
			pos.play<BLACK>(move);
		}*/
		break;
	}
}
