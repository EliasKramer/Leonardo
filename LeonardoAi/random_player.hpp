#pragma once
#include "chess_player.hpp"

class random_player : public chess_player
{
public:
	Stockfish::Move get_move(Stockfish::Position& pos) override;
};

