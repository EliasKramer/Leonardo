#include "random_player.hpp"

Stockfish::Move random_player::get_move(Stockfish::Position& pos)
{
	return Stockfish::Move::MOVE_NONE;
	/*
	if (pos.turn() == WHITE)
	{
		MoveList<WHITE> list(pos);
		return list.begin()[rand() % list.size()];
	}
	else
	{
		MoveList<BLACK> list(pos);
		return list.begin()[rand() % list.size()];
	}
	*/
}