#pragma once
#include "chess_player.hpp"

class chess_game
{
private:
	Stockfish::Position pos;
	chess_player& white_player;
	chess_player& black_player;
public:
	chess_game(chess_player& white_player, chess_player& black_player);
	void play();
};

