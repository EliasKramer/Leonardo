#pragma once
#include "chess_player.hpp"

class chess_game
{
private:
	chess::Board pos;
	chess_player* white_player;
	chess_player* black_player;
public:
	chess_game(chess_player* white_player, chess_player* black_player);

	int play();
};

