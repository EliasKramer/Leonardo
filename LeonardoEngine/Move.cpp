#include "Move.h"

Move::Move(Piece *piece, square from, square to) :
	piece(piece),
	from(from), 
	to(to)
{
}

Move::Move(Piece *piece, square from, square to, moveType type) :
	piece(piece),
	from(from), 
	to(to), 
	type(type)
{
}

Move::Move(Piece *piece, square from, square to, pieceType promotion) :
	piece(piece),
	from(from), 
	to(to), 
	promotion(promotion),
	type(PROMOTION)
{
}

Piece *Move::getPiece()
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

pieceType Move::getPromotion()
{
	return promotion;
}

moveType Move::getType()
{
	return type;
}

bool Move::operator==(const Move& other) const
{
	return piece == other.piece && from == other.from && to == other.to && promotion == other.promotion && type == other.type;
}