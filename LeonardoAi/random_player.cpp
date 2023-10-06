#include "random_player.hpp"

chess::Move random_player::get_move(chess::Board& pos)
{
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, pos);

	int random_idx = rand() % moves.size();

	return moves[random_idx];
}