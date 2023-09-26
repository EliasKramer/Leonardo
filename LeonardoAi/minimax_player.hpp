#pragma once
#include "chess_player.hpp"

class minimax_player : public chess_player
{
private:
	int start_depth;
public:
	minimax_player();
	minimax_player(int start_depth);

	chess::Move get_move(chess::Board& board) override;
};