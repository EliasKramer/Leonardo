#pragma once
#include "Move.h"
#include <string>
#include "ChessBoard.h"
#include <iostream>
class Player
{
protected:
	std::string _name;

public:
	Player(std::string name);

	std::string getName();

	//returns the index of the move in the list
	virtual int getMove(const ChessBoard& board, const UniqueMoveList& moves);
};