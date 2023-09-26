#pragma once
#include "chess_player.hpp"

class human_player : public chess_player
{
public:
	chess::Move get_move(chess::Board& pos) override;
};

