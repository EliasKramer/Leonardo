#pragma once
#include "chess_player.hpp"
#include "../TmpEngine/position_surge.h"
#include "../TmpEngine/tables_surge.h"
#include "../TmpEngine/types_surge.h"

class chess_game
{
private:
	Position pos;
	chess_player* white_player;
	chess_player* black_player;
public:
	chess_game(chess_player* white_player, chess_player* black_player);
	void play();
};

