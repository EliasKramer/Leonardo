#pragma once
#include "chess.hpp"

class chess_player
{
public:
	virtual chess::Move get_move(chess::Board& pos) = 0;
};