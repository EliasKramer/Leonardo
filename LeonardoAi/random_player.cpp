#include "random_player.hpp"

Move random_player::get_move(Position& pos)
{
	if (pos.turn() == WHITE)
	{
		MoveList<WHITE> moves(pos);
		return *moves.begin();// *(moves.begin() + rand() % moves.size());
	}
	else {

		MoveList<BLACK> moves(pos);
		return *moves.begin();// *(moves.begin() + rand() % moves.size());
	}
}