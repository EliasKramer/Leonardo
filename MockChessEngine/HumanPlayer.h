#pragma once
#include "Player.h"
#include <iostream>

class HumanPlayer : public Player
{
public:
	HumanPlayer();
	HumanPlayer(std::string name);
	

	int getMove(const ChessBoard& board, const UniqueMoveList& moves) override;
};