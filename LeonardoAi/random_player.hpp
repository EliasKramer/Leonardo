#pragma once
#include "chess_player.hpp"

class random_player : public chess_player
{
public:
	Move get_move(Position& pos) override;
};

