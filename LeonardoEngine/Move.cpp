#include "Move.h"

Move::Move(pieceType piece, square from, square to) :
	piece(piece),
	from(from), 
	to(to)
{
}

Move::Move(pieceType piece, square from, square to, moveType type) :
	piece(piece),
	from(from), 
	to(to), 
	type(type)
{
}

Move::Move(square from, pieceType promotion) :
	piece(promotion),
	from(from), 
	to(from), 
	type(PROMOTION)
{
}

pieceType Move::getPieceType()
{
	return piece;
}

square Move::getFrom()
{
	return from;
}

square Move::getTo()
{
	return to;
}

moveType Move::getType()
{
	return type;
}

bool Move::operator==(const Move& other) const
{
	return piece == other.piece && from == other.from && to == other.to && type == other.type;
}