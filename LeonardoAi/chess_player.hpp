#pragma once
#include "../StockFish/position.h"

class chess_player
{
public:
	virtual Stockfish::Move get_move(Stockfish::Position& pos) = 0;
};