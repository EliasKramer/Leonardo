#include "Move.h"

Move::Move(Square givenStart, Square givenDest) :
	_start(givenStart),
	_destination(givenDest)
{}

Square Move::getStart() const
{
	return _start;
}

Square Move::getDestination() const
{
	return _destination;
}

void Move::execute(BoardRepresentation& board) const
{
	board.copySquareToPos(_start, _destination);
	board.delAtPos(_start);
}

BitBoard Move::getBBWithMoveDone()
{
	return BB_SQUARE[_start] | BB_SQUARE[_destination];
}

std::string Move::getString() const
{
	return SQUARE_STRING[_start] + SQUARE_STRING[_destination];
}

bool operator==(const Move& first, const Move& second)
{
	return
		first._destination == second._destination &&
		first._start == second._start;
}

bool operator!=(const Move& first, const Move& second)
{
	return !(first == second);
}
