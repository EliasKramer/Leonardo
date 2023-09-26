#pragma once
#include "chess_player.hpp"

class abp_player : public chess_player
{
private:
	int start_depth;
public:
	abp_player();
	abp_player(int start_depth);

	chess::Move get_move(chess::Board& board) override;
};

