#pragma once
#include "Move.h"
#include "ChessPiece.h"
#include "Constants.h"
class MovePromote : public Move
{
private:
	ChessPiece _promotingPiece;
public:
	MovePromote(Square givenStart, Square givenDest, ChessPiece promotingPiece);

	void execute(BoardRepresentation& board) override;

	std::string getString() const override;
	//no overriding of getBBWithMoveDone, because it acts the same as the normal move
};