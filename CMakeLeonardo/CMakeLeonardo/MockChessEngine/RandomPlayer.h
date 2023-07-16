#pragma once
#include "Player.h"
#include <random>
#include <string>
class RandomPlayer : public Player
{
public:
	RandomPlayer();
	RandomPlayer(std::string name);

	int getMove(const ChessBoard& board, const UniqueMoveList& moves) override;
};