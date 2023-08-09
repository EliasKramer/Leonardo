#include "Move.h"

Move::Move(square from, square to) :
	from(from), 
	to(to)
{
}


Move::Move(square from, pieceType promotion) :
	from(from), 
	to(from), 
	promotion(promotion),
{
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