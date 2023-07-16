#include "MovePromote.h"

MovePromote::MovePromote(Square givenStart, Square givenDest, ChessPiece promotingPiece)
	:Move(givenStart, givenDest),
	_promotingPiece(promotingPiece)
{}

void MovePromote::execute(BoardRepresentation& board) const
{
	board.delAtPos(_start);
	board.delAtPos(_destination);
	board.setAtPosition(_promotingPiece, _destination);
}

 std::string MovePromote::getString() const
{
	std::string moveStr = Move::getString();
	moveStr += charToLower(PIECETYPE_CHAR[_promotingPiece.getType()]);

	return moveStr;
}
