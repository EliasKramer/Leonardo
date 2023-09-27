#pragma once
#include "chess_player.hpp"

class abp_player : public chess_player
{
private:
	int start_depth;

	std::vector<std::pair<size_t, chess::Move>> openings;

	void load_openings();

	int get_opening_move(size_t hash);
public:
	abp_player();
	abp_player(int start_depth);

	chess::Move get_move(chess::Board& board) override;
};

