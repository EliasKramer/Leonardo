#include "MoveEnPassant.h"

MoveEnPassant::MoveEnPassant(Square givenStart, Square givenDest, Square posToDel)
	: Move(givenStart, givenDest),
	_posToDel(posToDel)
{}

void MoveEnPassant::execute(BoardRepresentation & board) const
{
	Move::execute(board);
	board.delAtPos(_posToDel);
}

BitBoard MoveEnPassant::getBBWithMoveDone()
{
	return BB_SQUARE[_posToDel] | BB_SQUARE[_destination] | BB_SQUARE[_start];
}
