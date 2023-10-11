#include "Move.h"

Move::Move(Piece *piece, uint8_t pieceIndex, square from, square to) :
	pieceIndex(pieceIndex),
	from(from), 
	to(to)
{
}

Move::Move(Piece *piece, uint8_t pieceIndex, square from, square to, moveType type) :
	pieceIndex(pieceIndex),
	from(from), 
	to(to), 
	type(type)
{
}

Move::Move(Piece *piece, uint8_t pieceIndex, square from, square to, pieceType promotion) :
	pieceIndex(pieceIndex),
	from(from), 
	to(to), 
	promotion(promotion),
	type(PROMOTION)
{
}


uint8_t Move::getPieceIndex() 
{
	return pieceIndex;
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
	return pieceIndex == other.pieceIndex && from == other.from && to == other.to && promotion == other.promotion && type == other.type;
}